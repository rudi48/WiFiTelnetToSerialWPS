// program WiFiTelnetToSerialWPS.ino, Arduino 1.6.12
/* 
  WiFiTelnetToSerial - Example Transparent UART to Telnet Server for esp8266

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP8266WiFi library for Arduino environment.
 
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/* 2016-10-17 Rudolf Reuter, push button switch for WPS function added.
 *  http://www.wi-fi.org/knowledge-center/faq/how-does-wi-fi-protected-setup-work
 *  
 *  Once the WiFi credentials are WPS fetched, they are stored,
 *  for following use (firmware function).
 *  
 *  Works on ESP-01 module - Arduino Tools:
 *      Board: "Generic ESP8266 Module"
 *      Flash Mode: "DIO"
 *      Flash Fequency: "40 MHz"
 *      CPU Frequency: "80 MHz"
 *      Flash Size: "512K (64K SPIFFS)"
 *      Debug port: "Disabled"
 *      Reset Method: "ck"
 *      Upload Speed: "115200"
 *  
 *  Connection of ESP8266 GPIO0 and GPIO2 for WPS button:
 *         Vcc  Vcc       Vcc = 3.3 V
 *          |    |         |
 *         4K7  4K7       1k0  pull up resistors (3K3 to 10K Ohm)
 *          |    |   +-|<|-+   LED red
 *          |    |   |
 * GPIO0  - +--------+---o |
 *               |         | -> | push button switch for WPS function
 * GPIO2  -------+-------o |
 *
 * from http://www.forward.com.au/pfod/ESP8266/GPIOpins/index.html
 *
 * /!\ There is a '''5 minute''' timeout in the ESP8266 '''!WiFi stack'''. 
 * That means, if you have after the first WLAN connection a data traffic pause
 * longer than '''5 minutes''', you to have to resync the connection 
 * by sending some dummy characters.
 */
 
#include <ESP8266WiFi.h>

bool debug = false;  // enable either one
//bool debug = true:

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 1

WiFiServer server(23);  //  port 23 = telnet
WiFiClient serverClients[MAX_SRV_CLIENTS];


bool startWPSPBC() {
// from https://gist.github.com/copa2/fcc718c6549721c210d614a325271389
// wpstest.ino
  Serial.println("WPS config start");
  bool wpsSuccess = WiFi.beginWPSConfig();
  if(wpsSuccess) {
      // Well this means not always success :-/ in case of a timeout we have an empty ssid
      String newSSID = WiFi.SSID();
      if(newSSID.length() > 0) {
        // WPSConfig has already connected in STA mode successfully to the new station. 
        Serial.printf("WPS finished. Connected successfull to SSID '%s'\n", newSSID.c_str());
      } else {
        wpsSuccess = false;
      }
  }
  return wpsSuccess; 
}

void setup() {
  Serial.begin(9600);        // adopt baud rate to your needs
  //Serial.begin(115200); 
  delay(1000);
  if (debug) {
    Serial.println("\n WPS with push button on GPIO2 input, LOW = active");
    Serial.printf("\nTry connecting to WiFi with SSID '%s'\n", WiFi.SSID().c_str());
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str()); // reading data from EPROM, 
  while (WiFi.status() == WL_DISCONNECTED) {          // last saved credentials
    delay(500);
    if (debug) Serial.print(".");
  }

  wl_status_t status = WiFi.status();
  if(status == WL_CONNECTED) {
    if (debug) Serial.printf("\nConnected successful to SSID '%s'\n", WiFi.SSID().c_str());
  } else {
    // WPS button I/O setup
    pinMode(0,OUTPUT);         // Use GPIO0
    digitalWrite(0,LOW);       // for hardware safe operation.
    pinMode(2, INPUT_PULLUP);  // Push Button for GPIO2 active LOW
    Serial.printf("\nCould not connect to WiFi. state='%d'\n", status);
    Serial.println("Please press WPS button on your router, until mode is indicated.");
    Serial.println("next press the ESP module WPS button, router WPS timeout = 2 minutes");
    
    while(digitalRead(2) == HIGH)  // wait for WPS Button active
      yield(); // do nothing, allow background work (WiFi) in while loops
    Serial.println("WPS button pressed");
    
    if(!startWPSPBC()) {
       Serial.println("Failed to connect with WPS :-(");  
    } else {
      WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str()); // reading data from EPROM, 
      while (WiFi.status() == WL_DISCONNECTED) {          // last saved credentials
        delay(500);
        Serial.print("."); // show wait for connect to AP
      }
      pinMode(0,INPUT);    // GPIO0, LED OFF, show WPS & connect OK
    }
  } 

  server.begin();  // telnet server
  server.setNoDelay(true);
  if (debug) {
    Serial.print("\nReady! Use 'telnet ");
    Serial.print(WiFi.localIP());
    Serial.println(" port 23' to connect");
  }
}

// from Arduino/hardware/esp8266com/esp8266/libraries/ESP8266WiFi/examples/
// (2015-11-15)

void loop() {
  uint8_t i;
  //check if there are any new clients
  if (server.hasClient()) {
    for(i = 0; i < MAX_SRV_CLIENTS; i++) {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected()) {
        if(serverClients[i]) serverClients[i].stop();
        serverClients[i] = server.available();
        if (debug) {
          Serial.print("New client: "); 
          Serial.println(i);
        }
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if(serverClients[i].available()) {
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()) Serial.write(serverClients[i].read());
      }
    }
  }
  //check UART for data
  if(Serial.available()) {
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    //push UART data to all connected telnet clients
    for(i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (serverClients[i] && serverClients[i].connected()) {
        serverClients[i].write(sbuf, len);
        delay(1);  // for multitasking
      }
    }
  }
}

