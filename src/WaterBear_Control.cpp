#include "WaterBear_Control.h"

#define MAX_REQUEST_LENGTH 50

int WaterBear_Control::state = 0;
void * lastCommandPayload;
bool lastCommandPayloadAllocated = false;

bool WaterBear_Control::ready(HardwareSerial &port) {
    Stream *myStream = &port;
    return WaterBear_Control::ready(myStream);
}

bool WaterBear_Control::ready(Adafruit_BluefruitLE_UART &ble){
    Stream *myStream = &ble;
    return WaterBear_Control::ready(myStream);
}

bool WaterBear_Control::ready(Adafruit_BluefruitLE_SPI &ble){
    Stream *myStream = &ble;
    return WaterBear_Control::ready(myStream);
}

bool WaterBear_Control::ready(Stream * myStream) {
    if( (myStream->peek() == '>' && WaterBear_Control::state == 0)
        || WaterBear_Control::state == 1){
      return true;
    } else {
      return false;
    }
}

int WaterBear_Control::processControlCommands(HardwareSerial &port) {
  Stream *myStream = &port;
  return WaterBear_Control::processControlCommands(myStream);
}

int WaterBear_Control::processControlCommands(Adafruit_BluefruitLE_UART &ble) {
  Stream *myStream = &ble;
  return WaterBear_Control::processControlCommands(myStream);
}

int WaterBear_Control::processControlCommands(Adafruit_BluefruitLE_SPI &ble) {
  Stream *myStream = &ble;
  return WaterBear_Control::processControlCommands(myStream);
}

void * WaterBear_Control::getLastPayload() {
  return lastCommandPayload;
}

void WaterBear_Control::blink(int times, int duration){
  pinMode(PA5, OUTPUT);
  for (int i = times; i > 0; i--)
  {
    digitalWrite(PA5, HIGH);
    delay(duration);
    digitalWrite(PA5, LOW);
    delay(duration);
  }
}

  DS3231 ClockWBC;
  struct tm ts;

time_t WaterBear_Control::timestamp(){
  bool century = false;
	bool h24Flag;
	bool pmFlag;

  ts.tm_year = ClockWBC.getYear();
  ts.tm_mon = ClockWBC.getMonth(century);
  ts.tm_mday = ClockWBC.getDate();
  ts.tm_wday = ClockWBC.getDoW();
  ts.tm_hour = ClockWBC.getHour(h24Flag, pmFlag);
  ts.tm_min = ClockWBC.getMinute();
  ts.tm_sec = ClockWBC.getSecond();
  ts.tm_isdst = -1; // Is DST on? 1 = yes, 0 = no, -1 = unknown
  return (mktime(&ts)); // turn tm struct into time_t value
}

void WaterBear_Control::setTime(time_t toSet){
  ts = *localtime(&toSet); // convert UTC epoch timestamp time_t value to tm structure
  ClockWBC.setClockMode(false); //true for 12h false for 24h
  ClockWBC.setYear(ts.tm_year);
  ClockWBC.setMonth(ts.tm_mon);
  ClockWBC.setDate(ts.tm_mday);
  ClockWBC.setDoW(ts.tm_wday);
  ClockWBC.setHour(ts.tm_hour);
  ClockWBC.setMinute(ts.tm_min);
  ClockWBC.setSecond(ts.tm_sec);
}

void	WaterBear_Control::t_t2ts(time_t epochTS){
  char testTime[23];

  ts = *localtime(&epochTS); // convert unix to tm structure
  // Format time, "yyyy/mm/dd hh:mm:ss zzz"(23) = "%Y/%m/%d %H:%M:%S %Z"
  strftime(testTime, sizeof(testTime), "%Y/%m/%d %w %H:%M:%S %Z", &ts); // converts a tm into custom date structure stored in string
  Serial2.print("Custom Timestamp: ");
  Serial2.println(testTime);
  Serial2.flush();
}

int WaterBear_Control::processControlCommands(Stream * myStream) {

  char lastDownloadDate[15] = "NOTIMPLEMENTED"; // placeholder

  if(WaterBear_Control::state == 0){

    if(lastCommandPayloadAllocated == true){
      free(lastCommandPayload);
      lastCommandPayloadAllocated = false;
    }
    // awakeTime = RTC.now().unixtime(); // Keep us awake once we are talking to the phone

    char request[MAX_REQUEST_LENGTH] = "";
    myStream->readBytesUntil('<', request, MAX_REQUEST_LENGTH);
    myStream->write(">COMMAND RECIEVED: ");
    myStream->write(&request[1]);
    myStream->write("<");
    myStream->flush();
    delay(100);

    if(strncmp(request, ">WT_OPEN", 19) == 0) {
      // TODO:  Need to pass firmware version to control somehow
      /*
      myStream->write(">VERSION:");
      myStream->write(version);
      myStream->write("<");
      myStream->flush();
      delay(100);
      */
      // DateTime now = RTC.now();
      char dateString[11];
      // sprintf(dateString, "%lu", now.unixtime());
      myStream->print(">Datalogger Time: ");
      //myStream->print(now.year());
      myStream->print("-");
      //myStream->print(now.month());
      myStream->print("-");
      //myStream->print(now.day());
      myStream->print(" ");
      //myStream->print(now.hour());
      myStream->print(":");
      //myStream->print(now.minute());
      myStream->print(":");
      //myStream->print(now.second());
      myStream->print("<");
      delay(100);


      myStream->write(">WT_IDENTIFY:");
      // TODO: create and pass a device info object
      myStream->print("NOTIMPLEMENTED");
      for(int i=0; i<8; i++){
//        myStream->print((unsigned int) uuid[2*i], HEX);
      }
      myStream->write("<");
      myStream->flush();

      myStream->write(">WT_TIMESTAMP:");
      //myStream->print(RTC.now().unixtime());
      myStream->write("<");
      myStream->flush();
      delay(100);


    }
    else if(strncmp(request, ">WT_DOWNLOAD",12) == 0) {
      // Flush the input, would be better to use a delimiter
      // May not be necessary now
      unsigned long now = millis ();
      while (millis () - now < 1000)
      myStream->read ();  // read and discard any input

      if(request[20] == ':'){
        // we have a reference date
        // TODO: create and pass a device info object
        strncpy(lastDownloadDate, &request[21], 10);
      }

      myStream->print(">WT_READY<");
      myStream->flush();

      WaterBear_Control::state = 1;
      return WT_CONTROL_NONE;
    }
    else if(strncmp(request, ">WT_SET_RTC:", 12) == 0){
      myStream->println("GOT SET_RTC<");
      char UTCTime[11] = "0000000000";
      strncpy(UTCTime, &request[12], 10);
      time_t value;
      int found = sscanf(&UTCTime[0], "%lld", &value);
      if(found == 1){
        time_t * commandPayloadPointer = (time_t *) malloc(sizeof(time_t));
        *commandPayloadPointer = value;
        lastCommandPayloadAllocated = true;
        lastCommandPayload = commandPayloadPointer;
      }
      return WT_SET_RTC;

      //UTCTime[10] = '\0';
      //long time = atol(UTCTime);
      //delay(100);

      //myStream->println(">RTC not enabled<");
      //RTC.adjust(DateTime(time));

      //myStream->write( (char *) F(">RECV UTC: "));
      //myStream->print(UTCTime);
      //myStream->write( (char *) F("--"));
      //myStream->print(time);
      //myStream->write( (char *) F("--"));
      //myStream->print(RTC.now().unixtime());
      //myStream->write( (char *) F("<"));
      //myStream->flush();

      //myStream->print(">Received UTC time: ");
      //myStream->print(UTCTime);
      //myStream->print("---");
      //myStream->print(time);
      //myStream->print("---");
      // myStream->print(RTC.now().unixtime());
      //myStream->print("<");
      //myStream->flush();

      // TODO: create and pass a data file writer class
      // setNewDataFile();

    } else if(strncmp(request, ">WT_DEPLOY:", 11) == 0){
      char deploymentIdentifier[29];
      strncpy(deploymentIdentifier, &request[11], 28);
      myStream->println(">DEPLOYMENT IDENTIFER NOT WRITTEN<");
      // writeDeploymentIdentifier(deploymentIdentifier); // How will we hand the deployment identifier back ??
                                                         // This is just part of the device object I guess
      myStream->write(">Wrote: ");
      myStream->write(deploymentIdentifier);
      myStream->write("<");
      myStream->flush();

      // TODO: create and pass a data file writer class
      // setNewDataFile();

    } else if(strncmp(request, ">WT_CONFIG", 10) == 0){
      myStream->println(">CONFIG<");
      return WT_CONTROL_CONFIG;
      // go into config mode

    } else if(strncmp(request, ">CAL_DRY", 8) == 0){
      myStream->println(">GOT CAL_DRY<");
      return WT_CONTROL_CAL_DRY;

    } else if(strncmp(request, ">CAL_LOW:", 9) == 0){
      myStream->println(">GOT CAL_LOW<");
      char calibrationPointStringValue[10];
      strncpy(calibrationPointStringValue, &request[9], 9);
      int value;
      int found = sscanf(&calibrationPointStringValue[0], "%d", &value);
      if(found == 1){
        int * commandPayloadPointer = (int *) malloc(sizeof(int));
        *commandPayloadPointer = value;
        lastCommandPayloadAllocated = true;
        lastCommandPayload = commandPayloadPointer;
      }

      return WT_CONTROL_CAL_LOW;

    } else if(strncmp(request, ">CAL_HIGH:", 10) == 0){
      myStream->println(">GOT CAL_HIGH<");
      char calibrationPointStringValue[10];
      strncpy(calibrationPointStringValue, &request[10], 9);
      int value;
      myStream->println(calibrationPointStringValue);
      int found = sscanf(&calibrationPointStringValue[0], "%d", &value);
      if(found == 1){
        int * commandPayloadPointer = (int *) malloc(sizeof(int));
        *commandPayloadPointer = value;
        lastCommandPayloadAllocated = true;
        lastCommandPayload = commandPayloadPointer;
      }

      return WT_CONTROL_CAL_H;

    } else {
      char lastDownloadDateEmpty[11] = "0000000000";
      strcpy(lastDownloadDate, lastDownloadDateEmpty);
    }

  } else if(WaterBear_Control::state == 1){

    char ack[7] = "";
    myStream->readBytesUntil('<', ack, 7);
    if(strcmp(ack, ">WT_OK") != 0) {
      char message[30] = "";
      sprintf(message, "ERROR #%s#", ack);
      myStream->print(message);

      //Flush
      unsigned long now = millis ();
      while (millis () - now < 1000)
      myStream->read ();  // read and discard any input

      WaterBear_Control::state = 0;
      return WT_CONTROL_NONE;
    }

    char lastFileNameSent[10];
    //bool success = WaterBear_FileSystem::dumpLoggedDataToStream(myStream, &lastFileNameSent); //also needs lastDownloadDate

    if(true){
      // Send last download date to phone for book keeeping
      char transferCompleteMessage[34] = ">WT_COMPLETE:0000000000<";
      strncpy(&transferCompleteMessage[22], lastFileNameSent, 10); // Send timestamp of last file sent
      myStream->write(transferCompleteMessage);
      // TODO: create and pass a data file writer class
      // setNewDataFile();
    } else {
      // There was some kind of error
    }
    WaterBear_Control::state = 0;
  }

  return WT_CONTROL_NONE;
}
