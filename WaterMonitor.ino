/*********************************************************************


 * PH  : A2
 * DO  : Serial port Rx(0),Tx(1)
 * temperature:D5
 *
 * SD card attached to SPI bus as follows:
 * M0:   Onboard SPI pin,CS - pin 4 (CS pin can be changed)
 *
 * author  :  Jason(jason.ling@dfrobot.com)
 * version :  V1.0
 * date    :  2017-04-06
 **********************************************************************/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "GravitySensorHub.h"
#include "GravityRtc.h"
#include "OneWire.h"
#include "SdService.h"
#include "Debug.h"
#include <SoftwareSerial.h>

String myAPIkey = "key"; 
SoftwareSerial ESP8266(2, 3); // Rx,  Tx

long writingTimer = 17; 
long startTime = 0;
long waitTime = 0;

boolean relay1_st = false; 
boolean relay2_st = false; 
unsigned char check_connection=0;
unsigned char times_check=0;
boolean error;

// clock module
GravityRtc rtc;

// sensor monitor
GravitySensorHub sensorHub;
//SdService sdService = SdService(sensorHub.sensors);
void setup() {
	Serial.begin(9600);
	ESP8266.begin(9600); 
  startTime = millis(); 
  ESP8266.println("AT+RST");
  delay(2000);
  Serial.println("Connecting to Wifi");
   while(check_connection==0)
  {
    Serial.print(".");
  ESP8266.print("AT+CWJAP=\"ssid\",\"pass\"\r\n");
  ESP8266.setTimeout(5000);
 if(ESP8266.find("WIFI CONNECTED\r\n")==1)
 {
 Serial.println("WIFI CONNECTED");
 break;
 }
 times_check++;
 if(times_check>3) 
 {
  times_check=0;
   Serial.println("Trying to Reconnect..");
  }
  }
	//rtc.setup();
	sensorHub.setup();
	//sdService.setup();

}


//********************************************************************************************
// function name: sensorHub.getValueBySensorNumber (0)
// Function Description: Get the sensor's values, and the different parameters represent the acquisition of different sensor data     
// Parameters: 0 ph value  
// Parameters: 1 temperature value    
// Parameters: 2 Dissolved Oxygen
// Parameters: 3 Conductivity
// Parameters: 4 Redox potential
// return value: returns a double type of data
//********************************************************************************************

unsigned long updateTime = 0;

void loop() {
	//rtc.update();
	sensorHub.update();
	//sdService.update();

  waitTime = millis()-startTime;   
  if (waitTime > (writingTimer*1000)) 
  {
    writeThingSpeak();
    startTime = millis();   
  }

}

void writeThingSpeak(void)
{
  startThingSpeakCmd();
  String getStr = "GET /update?api_key=";
  getStr += myAPIkey;
  getStr +="&field1=";
  getStr += String(sensorHub.getValueBySensorNumber(0));
  getStr +="&field2=";
  getStr += String(sensorHub.getValueBySensorNumber(1));
  getStr += "\r\n\r\n";
  GetThingspeakcmd(getStr); 
}

void startThingSpeakCmd(void)
{
  ESP8266.flush();
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149";                      // api.thingspeak.com IP address
  cmd += "\",80";
  ESP8266.println(cmd);
  Serial.print("Start Commands: ");
  Serial.println(cmd);

  if(ESP8266.find("Error"))
  {
    Serial.println("AT+CIPSTART error");
    return;
  }
}

String GetThingspeakcmd(String getStr)
{
  String cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ESP8266.println(cmd);
  Serial.println(cmd);

  if(ESP8266.find(">"))
  {
    ESP8266.print(getStr);
    Serial.println(getStr);
    delay(500);
    String messageBody = "";
    while (ESP8266.available()) 
    {
      String line = ESP8266.readStringUntil('\n');
      if (line.length() == 1) 
      { 
        messageBody = ESP8266.readStringUntil('\n');
      }
    }
    Serial.print("MessageBody received: ");
    Serial.println(messageBody);
    return messageBody;
  }
  else
  {
    ESP8266.println("AT+CIPCLOSE");     
    Serial.println("AT+CIPCLOSE"); 
  } 
}
