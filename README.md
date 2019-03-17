# Zapper
Hobby Electronics demo using ESP8266 and relay module. The idea is to have a relay module interrupting the flow of power on one lead of a muscle stimulator, or TENS unit. This relay is controlled by an ESP826, which allows user interaction over wifi.

## Setup
1. Install the ESP8266 Board in the Arduino IDE, as instructed: https://github.com/esp8266/Arduino#installing-with-boards-manager
2. Within the Arduino IDE, select the ESP board being used
3. Load the Arduino source code and upload it to the ESP8266

## Use
When the ESP8266 is booted, it will set up an unsecure WIFI Access Point. For anyone connected, all DNS requests direct the user back to the internal webserver. For most devices, this is recognized as a 'captive portal', much like hotels use for WIFI login. The internal webserver serves up a single page which allows the user to turn on the relay for a short time, resulting in a zap.
