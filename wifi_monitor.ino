/*
  This is an initial sketch to be used as a "blueprint" to create apps which can be used with IOTappstory.com infrastructure
  Your code can be filled wherever it is marked.

  Copyright (c) [2016] [Andreas Spiess]

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  virginSoilFull V2.2.2
*/

#define COMPDATE __DATE__ __TIME__
#define MODEBUTTON 0                                        // Button pin on the esp for selecting modes. D3 for the Wemos!


#include <IOTAppStory.h>                                    // IotAppStory.com library
IOTAppStory IAS(COMPDATE, MODEBUTTON);                      // Initialize IotAppStory

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#include "time.h"

#include "DHT.h"

#define DHTPIN 26     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);



// ================================================ EXAMPLE VARS =========================================
// used in this example to print variables every 10 seconds
unsigned long printEntry;
String deviceName = "Gwakwani Wifi Monitor";
String chipId;

// We want to be able to edit these example variables below from the wifi config manager
// Currently only char arrays are supported.
// Use functions like atoi() and atof() to transform the char array to integers or floats
// Use IAS.dPinConv() to convert Dpin numbers to integers (D6 > 14)

char* lbl         = "Light Show";
char* exampleURL  = "http://someapi.com/getdata.php?userid=1234&key=7890abc";
char* nrOf        = "6";

char* doSomething = "1";
char* chosen      = "0";

char* updInt      = "3600";
char* ledPin      = "2";
char* timeZone    = "Pretoria";

unsigned long flashtimer = 0;
int i = 0;
int temp = 0;
int hum = 0;
int failCount = 0;
const char* serverName = "http://gwakwani.rf.gd/post-esp-data.php?";
// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
String apiKeyValue = "tPmAT5Ab3j7F9";

String sensorName = "DHT11";
String sensorLocation = "Gwakwani";

unsigned long CHANNEL_ID = 2108346;
const char * CHANNEL_API_KEY = "M9JMDXC9C41N37QX";

//Set time parameters
int tmz = 1;           // Set timezone
const char* ntpServer = "pool.ntp.org";
long  gmtOffset_sec = tmz*3600;            //timezone setting
const int   daylightOffset_sec = 3600;
bool res;
bool timeConfigured = false;
 int c =0;
int hours =0;
int mins = 0;
int secs = 0;

float bat_sample[60];
float voltage;
int sensorValue;
 int s_max = 120;
float volts,vol;
float ave_volts;
int s = 0;
int bat_percentage = 0;

WiFiClient client;
HTTPClient http;

// ================================================ SETUP ================================================
void setup() {
  
  // create a unique deviceName for classroom situations (deviceName-123)
  chipId      = String(ESP_GETCHIPID);
  chipId      = "-"+chipId.substring(chipId.length()-3);
  deviceName += chipId;
	
  /* TIP! delete lines below when not used */
  IAS.preSetDeviceName(deviceName);                       	// preset deviceName this is also your MDNS responder: http://virginsoil-123.local
  //IAS.preSetAutoUpdate(false);                            // automaticUpdate (true, false)
  //IAS.preSetAutoConfig(false);                            // automaticConfig (true, false)
  //IAS.preSetWifi("ssid","password");                      // preset Wifi
  /* TIP! Delete Wifi cred. when you publish your App. */
	

  // IAS.addField(lbl, "textLine", 16);                        // These fields are added to the "App Settings" page in config mode and saved to eeprom. Updated values are returned to the original variable.
  // IAS.addField(exampleURL, "Textarea", 80, 'T');            // reference to org variable | field label value | max char return | Optional "special field" char
  // IAS.addField(nrOf, "Number", 8, 'N');                     // Find out more about the optional "special fields" at https://iotappstory.com/wiki
  
  // IAS.addField(doSomething, "Checkbox:Check me", 1, 'C');
  // IAS.addField(chosen, "Selectbox:Red,Green,Blue", 1, 'S');

  // IAS.addField(updInt, "Interval", 8, 'I');
  // IAS.addField(ledPin, "ledPin", 2, 'P');
  // IAS.addField(timeZone, "Timezone", 48, 'Z');
  


  // You can configure callback functions that can give feedback to the app user about the current state of the application.
  // In this example we use serial print to demonstrate the call backs. But you could use leds etc.

  IAS.onModeButtonShortPress([]() {
    Serial.println(F(" If mode button is released, I will enter in firmware update mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonLongPress([]() {
    Serial.println(F(" If mode button is released, I will enter in configuration mode."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onModeButtonVeryLongPress([]() {
    Serial.println(F(" If mode button is released, I won't do anything unless you program me to."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
    /* TIP! You can use this callback to put your app on it's own configuration mode */
  });

  IAS.onFirmwareUpdateProgress([](int written, int total){
   
      Serial.print(".");
      
      
      if(written%5==0){
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        Serial.print(F("\n Written "));
        Serial.print(written);
        Serial.print(F(" of "));
        Serial.print(total);
      }
      
  });
  
  
  IAS.onModeButtonNoPress([]() {
    Serial.println(F(" Mode Button is not pressed."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });
  
  IAS.onFirstBoot([]() {                              
    Serial.println(F(" Run or display something on the first time this app boots"));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateCheck([]() {
    Serial.println(F(" Checking if there is a firmware update available."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateDownload([]() {
    Serial.println(F(" Downloading and Installing firmware update."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onFirmwareUpdateError([](String statusMessage) {
    Serial.println(F(" Update failed...Check your logs"));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });

  IAS.onConfigMode([]() {
    Serial.println(F(" Starting configuration mode. Search for my WiFi and connect to 192.168.4.1."));
    Serial.println(F("*-------------------------------------------------------------------------*"));
  });
  
  IAS.onFirstBoot([]() {
    IAS.eraseEEPROM('P');                   // Optional! What to do with EEPROM on First boot of the app? 'F' Fully erase | 'P' Partial erase
  });
  

	/* TIP! delete the lines above when not used */
 
  IAS.begin();                                            // Run IOTAppStory
  IAS.setCallHomeInterval(atoi(updInt));                  // Call home interval in seconds(disabled by default), 0 = off, use 60s only for development. Please change it to at least 2 hours in production


  //-------- Your Setup starts from here ---------------

        pinMode(35, INPUT); //It is necessary to declare the input pin
        //start the dht sensor
        dht.begin();
        //Set LED pin as output
        pinMode(LED_BUILTIN, OUTPUT);

        //NTC Time
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      timeConfigured = true;

     struct tm timeinfo;
     if(!getLocalTime(&timeinfo)){
      Serial.println("Time Update Failed! ");
    }
    else{
      Serial.println(&timeinfo, "%H:%M:%S ");//
      Serial.println(&timeinfo, "%A ");
      Serial.println(&timeinfo, "%d %B %Y ");
    }


        //Start ThingSpeak   
       ThingSpeak.begin(client);
        
        ThingSpeak.setField(5, 1);
      int x = ThingSpeak.writeFields(CHANNEL_ID, CHANNEL_API_KEY);
      if(x == 200){
        Serial.println("Reboot Recovery... \nPing successfuly sent to dashboard!");
        
      }
      else{
         Serial.println("Reboot Recovery... \nPing failed to be sent to dashboard!");
      }
      delay(1000);
       
}



// ================================================ LOOP =================================================
void loop() {
  IAS.loop();   // this routine handles the calling home functionality,
                // reaction of the MODEBUTTON pin. If short press (<4 sec): update of sketch, long press (>7 sec): Configuration
                // reconnecting WiFi when the connection is lost,
                // and setting the internal clock (ESP8266 for BearSSL)


  //-------- Your Sketch starts from here ---------------
   struct tm timeinfo;
//
  if (millis()>86400000){ //reset after 24 hours
      ThingSpeak.setField(5, -1);

      int x = ThingSpeak.writeFields(CHANNEL_ID, CHANNEL_API_KEY);
      if(x == 200){
        Serial.println("Regular Restart in progress... ");
        
      }
      else{
         Serial.println("Regular Restart in progress... ");
      }
      delay(1000);
      ESP.restart();
  }

 if (millis() - flashtimer > 500) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

     

      /*  c = c+1;
      if(c%2==0){
        int sensorValue = analogRead(35);
        float voltage = (((sensorValue * 3.3) / 4096)); 
        int bat_percentage = mapfloat(voltage, 1.9, 3.3, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage
        Serial.println("\nBattery Status:");
        Serial.println("ADC Value: " +String(sensorValue));
        Serial.println("Voltage: "+ String(voltage) + " V");
        Serial.println("Battery percentage:" + String(bat_percentage)+ " %\n");
      
    }*/
  flashtimer = millis();
 }  

  if (millis() - printEntry > 60000) {  

    Serial.println("------------------------Update Start---------------------------------");        
    
     i = i + 1;
    Serial.println("Update number: "+String(i));
    
     if(!getLocalTime(&timeinfo)){
      Serial.println("Time Update Failed! ");
    }
    else{
    hours = timeinfo.tm_hour;
     mins = timeinfo.tm_min;
    secs = timeinfo.tm_sec;
    Serial.println("Update Time:");
    Serial.println(String(hours)+":"+String(mins)+":"+String(secs));

    // Serial.println(&timeinfo, "%H:%M:%S");//
     Serial.println(&timeinfo, "%A        ");
     Serial.println(&timeinfo, "%d %B %Y");
    }
     

    float hum = dht.readHumidity();
    float temp = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
  if (isnan(hum) || isnan(temp) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    //return;
  }
  else {

    Serial.println("\nSensor Data:");
      ThingSpeak.setField(1, temp);
      ThingSpeak.setField(2, hum);

      Serial.println(String(temp)+" °C");
      Serial.println(String(hum)+" %");


      sensorValue = analogRead(35);
      voltage = (((sensorValue * 3.3) / 4096)+0.9);
      bat_percentage = mapfloat(voltage, 2.8, 4.2, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage
      
        Serial.println("\nBattery Status:");
        ThingSpeak.setField(3, voltage);
        ThingSpeak.setField(4, bat_percentage);
        Serial.println("Voltage: "+ String(voltage) + " V");
        Serial.println("Battery percentage:" + String(bat_percentage)+ " %\n");

      int x = ThingSpeak.writeFields(CHANNEL_ID, CHANNEL_API_KEY);
      if(x == 200){
        Serial.println("Channel update successful.");
        failCount = 0;
      }
      else{
        Serial.println("Problem updating channel. HTTP error code " + String(x));
        failCount = failCount + 1;

        if (failCount > 3){
              ThingSpeak.setField(5, -1);

                int x = ThingSpeak.writeFields(CHANNEL_ID, CHANNEL_API_KEY);
                if(x == 200){
                  Serial.println("Update failure Restart in progress... ");
                  
                }
                else{
                  Serial.println("Update failure Restart in progress... ");
                }
                delay(1000);
              ESP.restart();
        }
      }
   }
      /*  int sensorValue = analogRead(35);
        float voltage = (((sensorValue * 3.3) / 4096)); 
        int bat_percentage = mapfloat(voltage, 1.9, 3.3, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage
        Serial.println("\nBattery Status:");
        Serial.println("ADC Value: " +String(sensorValue));
        Serial.println("Voltage: "+ String(voltage) + " V");
        Serial.println("Battery percentage:" + String(bat_percentage)+ " %\n");
      */
      
   Serial.println("-------------------------Update End--------------------------------");
    printEntry = millis();
  }
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
