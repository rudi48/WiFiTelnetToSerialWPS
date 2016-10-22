# WiFiTelnetToSerialWPS
WPS example with application WiFiTelnetToSerial


WPS (WiFi Protected Setup), see [how-does-wi-fi-protected-setup-work](http://www.wi-fi.org/knowledge-center/faq/how-does-wi-fi-protected-setup-work) allows to connect to a new **WiFi Access Point** via **push button switch**, instead of entering the **credentials** manually. 

By combining 3 pieces of existing software I made it to allow an **Access Point change** for the **Serial Transparent Bridge** (application program) just by using a **Push Button switch** together with a WPS functionality.

For the **wiring** of the **Push Button switch** and a **LED** for indication see the schematic: 
````
 *  Connection of ESP8266 GPIO0 and GPIO2 for WPS push button and indicator LED:
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
 ```
 If you power up the ESP8266 module, and the red LED (WPS indicator) lits (ON), it signals, that the WiFi module has lost the connection to the known Access Point, or the credentials of the Access Point had changed.

In case of **changed credentials**, it is possible to connect to the **Access Point** via **WPS**. The procedure is:

 * Push the **WLAN/WPS button** at your **Access Point**, until it is signallsing the WPS mode. You have a **2 minute timeout**.

 * Push the **WPS button switch** at the **ESP8266 module**. The ESP8266 module is now trying to connect to the **Access Point**.
    If the connect was successful, the LED goes OFF.

 * Check the WiFi connection via **ping** command in a terminal, or the application program, or the WiFi **Access Point** status web page. 
 
