
// you can enable debug logging to Serial at 115200
//#define REMOTEXY__DEBUGLOG

// RemoteXY select connection mode and include library 
#define REMOTEXY_MODE__ESP32CORE_WIFI_POINT
#include "EmonLib.h"
#include <TFT_eSPI.h>
#include <SPI.h>

#include <WiFi.h>
// #include <FreeRTOS.h>

// RemoteXY connection settings 
#define REMOTEXY_WIFI_SSID "Energy Monitoring"
#define REMOTEXY_WIFI_PASSWORD "fluctuation"
#define REMOTEXY_SERVER_PORT 6377

/* TFT */
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

#include <RemoteXY.h>

/* EMon */
EnergyMonitor emon;
#define vCalibration (106.8 - 65)
#define currCalibration 0.13186

float kWh = 0;
unsigned long lastmillis = millis();
unsigned long interval = 100; // interval at which to run event (milliseconds)
unsigned long startTime = 0;


// RemoteXY GUI configuration

// uint8_t RemoteXY_CONF[] = {   // 210 bytes
// 	255,1,0,56,0,203,0,17,0,0,0,16,1,106,200,1,1,15,0,130,
// 	3,6,100,76,0,31,68,4,7,99,73,34,31,26,134,86,111,108,116,115,
// 	0,65,109,112,115,0,67,50,86,26,10,5,2,27,11,129,8,89,38,5,
// 	116,86,111,108,116,97,103,101,32,82,101,97,100,105,110,103,58,0,129,78,
// 	88,16,7,17,86,111,108,116,115,0,69,7,2,14,14,1,2,20,183,70,
// 	12,0,2,26,31,31,79,78,0,79,70,70,0,67,50,101,26,10,5,2,
// 	27,11,129,8,104,37,5,116,67,117,114,114,101,110,116,32,82,101,97,100,
// 	105,110,103,58,0,129,78,103,19,7,17,65,109,112,115,0,130,6,123,95,
// 	54,0,31,67,27,134,53,9,5,35,31,11,129,43,149,22,5,25,67,111,
// 	110,100,105,116,105,111,110,0,67,10,156,88,18,5,2,27,13,129,46,127,
// 	15,5,25,83,116,97,116,117,115,0
// };
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =   // 293 bytes
  { 255,1,0,90,0,30,1,17,0,0,0,31,1,106,200,1,1,24,0,130,
  4,4,98,93,0,31,130,4,102,98,75,0,31,68,6,5,95,45,33,31,
  26,86,111,108,116,115,0,129,14,116,16,5,116,86,114,109,115,32,61,0,
  69,7,2,14,14,1,2,15,182,78,13,0,2,26,31,31,79,78,0,79,
  70,70,0,129,14,126,16,5,116,73,114,109,115,32,61,32,0,67,43,155,
  53,9,4,49,31,11,129,15,158,16,5,25,83,116,97,116,117,115,58,0,
  129,14,136,20,5,116,80,111,119,101,114,32,61,32,0,129,14,167,23,5,
  116,67,111,110,100,105,116,105,111,110,58,0,67,38,114,42,9,4,8,31,
  11,67,38,124,42,9,4,8,31,11,67,38,134,42,9,4,8,31,11,129,
  71,116,14,6,8,86,111,108,116,115,0,129,71,126,16,6,8,65,109,112,
  115,0,129,70,136,16,6,8,87,97,116,116,115,0,67,43,165,53,9,4,
  49,31,21,129,44,105,15,7,116,68,97,116,97,0,129,14,146,15,5,116,
  107,87,104,32,61,32,0,67,38,144,42,9,4,8,31,11,129,72,146,12,
  6,8,107,87,104,0,130,4,52,98,48,0,31,68,6,53,95,45,34,31,
  33,135,65,109,112,115,0,87,97,116,116,115,0 };

// this structure defines all the variables and events of your control interface 
struct {

    // input variables
  uint8_t sys_switch; // =1 if switch ON and =0 if OFF

    // output variables
  float voltmeter_graph;
  int16_t alarm; // =0 no sound, else ID of sound, =1001 for example, look sound list in app
  char status[11]; // string UTF8 end zero
  char voltmeter[11]; // string UTF8 end zero
  char ammeter[11]; // string UTF8 end zero
  char watt[11]; // string UTF8 end zero
  char condition[21]; // string UTF8 end zero
  char kWh[11]; // string UTF8 end zero
  float ammeter_graph;
  float wattmeter_graph;

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

QueueHandle_t queue;

void myTimerEvent() {
	emon.calcVI(20, 2000);

	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 0);

	Serial.print("Vrms: ");
	Serial.print(emon.Vrms, 2);
	Serial.print("V");

	tft.setCursor(25, 30);
	tft.print("Vrms = ");
	tft.print(emon.Vrms, 2);
	tft.println("V");

	Serial.print("\tIrms: ");
	Serial.print(emon.Irms, 4);
	Serial.print("A");

	tft.setCursor(25, 50);
	tft.print("Irms = ");
	tft.print(emon.Irms, 4);
	tft.println("A");

	Serial.print("\tPower: ");
	Serial.print(emon.apparentPower, 4);
	Serial.print("W");

	tft.setCursor(25, 70);
	tft.print("Power = ");
	tft.print(emon.apparentPower, 4);
	tft.println("W");

	Serial.print("\tkWh: ");
	kWh = kWh + emon.apparentPower*(millis()-lastmillis)/3600000000.0;
	Serial.print(kWh, 4);
	Serial.println("kWh");

	tft.setCursor(25, 90);
	tft.print("kWh = ");
	tft.print(kWh, 4);
	tft.println("kWh");

	lastmillis = millis();

	RemoteXY.voltmeter_graph = emon.Vrms;
	RemoteXY.ammeter_graph = emon.Irms;
	RemoteXY.wattmeter_graph = emon.apparentPower;
	sprintf(RemoteXY.voltmeter, "%.2f", emon.Vrms);
	sprintf(RemoteXY.ammeter, "%.4f", emon.Irms);
	sprintf(RemoteXY.watt, "%.4f", emon.apparentPower);
	sprintf(RemoteXY.kWh, "%.4f", kWh);
}

void check_switch(void * parameter)
{
	int lastState = RemoteXY.sys_switch;
	while (1)
	{
		if (RemoteXY.sys_switch != lastState)
		{
			lastState = RemoteXY.sys_switch;
			if (RemoteXY.sys_switch == 0)
			{
				// Send a message to the main task to stop the meter reading
				int msg = 0;
				xQueueSend(queue, &msg, portMAX_DELAY);
			}
		}
		vTaskDelay(1);
	}
}

void setup()
{
	Serial.begin(115200);

	/* digital pin for relay */
	pinMode(16, OUTPUT);

	/* relay OFF */
	digitalWrite(16, LOW);

	RemoteXY_Init();

	/* system switch off initial state */
	RemoteXY.sys_switch = 0;

	/* initial condition print Waiting */
	sprintf(RemoteXY.condition, "Waiting");

	// TODO you setup code
	/* create queue */
	queue = xQueueCreate(10, sizeof(int));

	// Create a task that checks the switch
	xTaskCreate(check_switch, "Check switch", 1000, NULL, 2, NULL);


	tft.init();
	tft.setRotation(1);

	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE,TFT_BLACK);
	tft.setTextFont(2);

	emon.voltage(34, vCalibration, 1.7); // Voltage: input pin, calibration, phase_shift
	emon.current(35, currCalibration); // Current: input pin, calibration.

	tft.fillScreen(TFT_BLACK);
	tft.setCursor(25, 35);
	tft.println("Energy Monitoring");
	tft.setCursor(55, 55);
	tft.println("System");
	tft.setCursor(35, 85);
	tft.print("Initializing");
	delay(1000);
	tft.print(".");
	delay(1000);
	tft.print(".");
	delay(1000);
	tft.print(".");
	delay(1000);
	tft.print(".");
	delay(1000);
	tft.print(".");

	tft.fillScreen(TFT_BLACK);
	tft.setCursor(10, 35);
	tft.println("Awaiting Connection...");

	/* print connected can now start */
	tft.setCursor(25, 65);
	tft.println("Systems Ready!");

	/* initialize 0 reading */
	sprintf(RemoteXY.voltmeter, "0.00");
	sprintf(RemoteXY.ammeter, "0.0000");
	sprintf(RemoteXY.watt, "0.0000");
	sprintf(RemoteXY.kWh, "0.0000");
}

void loop()
{
	RemoteXY_Handler ();

	// TODO you loop code
	// use the RemoteXY structure for data transfer
	// do not call delay(), use instead RemoteXY_delay()

	if (RemoteXY.sys_switch == 1)
	{
		unsigned long elapsedTime = millis() - startTime;

		if (startTime == 0)
			startTime = millis();

		/* print tft elapsed time */
		tft.setCursor(0, 0);
		tft.print("Elapsed Time: ");
		tft.print(elapsedTime / 1000);
		tft.println("s");

		/* print status -> running */
		sprintf(RemoteXY.status, "Running");

		/* relay ON */
		digitalWrite(16, HIGH);

		// if (millis() - lastmillis >= interval)
		myTimerEvent();
	} else {
		startTime = 0;
		/* print status -> stopped */
		sprintf(RemoteXY.status, "Stopped");

		/* relay OFF */
		digitalWrite(16, LOW);

		/* tft print OFF */
		tft.setCursor(130, 0);
		tft.println("OFF");
	}

	 // Check if a message has been received from the switch checking task
	int msg;
	if (xQueueReceive(queue, &msg, 0) == pdTRUE) {
	// Stop the meter reading
	// TODO: Add your code here to stop the meter reading
		RemoteXY.sys_switch = 0;

		sprintf(RemoteXY.voltmeter, "0.00");
		sprintf(RemoteXY.ammeter, "0.0000");
		sprintf(RemoteXY.watt, "0.0000");
		sprintf(RemoteXY.kWh, "0.0000");
	}

	/*
	 * switch case for condition based on the value of the graph
	 * if the value of the voltmeter graph is greater than 245, the condition is "Overvoltage"
	 * if the value of the voltmeter graph is between than 215 and 240 and , the condition is "Normal"
	 * if the value of the voltmeter graph is less than 215, the condition is "Undervoltage"
	 */
	switch ((int)RemoteXY.voltmeter_graph) {
		case 245 ... 1000:
			tft.setCursor(0, 110);
			tft.println("Overvoltage");
			sprintf(RemoteXY.condition, "Overvoltage");
			break;
		case 215 ... 244:
			tft.setCursor(0, 110);
			tft.println("Normal");
			sprintf(RemoteXY.condition, "Normal");
			break;
		case 101 ... 214:
			tft.setCursor(0, 110);
			tft.println("Undervoltage");
			sprintf(RemoteXY.condition, "Undervoltage");
			break;
		case 10 ... 100:
			tft.setCursor(0, 110);
			tft.println("Not connected");
			sprintf(RemoteXY.condition, "Not connected");
			break;
		default:
			tft.setCursor(0, 110);
			tft.println("Waiting");
			sprintf(RemoteXY.condition, "Waiting");
			break;
	}

}