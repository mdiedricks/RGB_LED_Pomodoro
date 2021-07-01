#include <RTClib.h>

/*
   VARIABLE NAMING LEGEND
   Uppercase letters denote hardware - B_RESET = button
   Lowercase letters denote data-types - b_ActionPressed = boolean

*/

#define B_ACTION1 12
#define B_RESET2 13
#define L_RED 2
#define L_GREEN 3
#define L_BLUE 4
#define FOCUS 25
#define BREAK 5

RTC_DS3231 rtc;

struct state {
  bool startSession;
  bool inSession;
  bool inFocus;
  bool inBreak;
  bool inPause;
  bool pauseTriggered;
  bool unpauseTriggered;
  bool reset;
};

struct session {
  uint8_t i_sessionCount;
  uint8_t i_startTimeHour;
  uint8_t i_startTimeMin;
  uint8_t i_endTimeHour;
  uint8_t i_endTimeMin;
  uint8_t i_timeRemaining;
};

struct colour {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

long i_ActionLen_millis = 0;
long i_ResetLen_millis = 0;

uint8_t currentMinute;
uint8_t currentSecond;
uint8_t lastMinute;
uint8_t lastSecond;

state currentState = {false, false, false, false, false, false, false, false};
session currentSession = {4, 0, 0, 0, 0, 0};

void setup() {
  Serial.begin(9600);
  Serial.println(":: Setting up");

  DateTime currentTime = rtc.now();

  pinMode(B_ACTION1, INPUT_PULLUP);
  pinMode(B_RESET2, INPUT_PULLUP);
  digitalWrite(B_ACTION1, HIGH);
  digitalWrite(B_RESET2, HIGH);
  toggleLights({255, 255, 255});
  currentMinute = currentTime.minute();
  currentSecond = currentTime.second();

  Serial.println(":: Initializing RTC");
  if (! rtc.begin()) {
    Serial.println(":: Error. Couldn't find RTC");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println(":: Error. RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  toggleLights({244, 255, 255});
  Serial.println(":: Ready to go");
}

void loop() {
  DateTime currentTime = rtc.now();

  // CHECK BUTTONS PRESSED
  while (digitalRead(B_ACTION1) == LOW && i_ActionLen_millis < 1000) {
    delay(100);
    i_ActionLen_millis = i_ActionLen_millis + 100;
  }
  while (digitalRead(B_RESET2) == LOW && i_ResetLen_millis < 1000) {
    delay(100);
    i_ResetLen_millis = i_ResetLen_millis + 100;
  }

  // ACTION BUTTON DECISION
  if (i_ActionLen_millis >= 1000) {
    Serial.print("...Action pressed...");
    if (!currentState.inSession && !currentState.startSession) {
      currentState.startSession = true; // start session
    } else if (currentState.inSession && !currentState.inPause) {
      currentState.pauseTriggered = true; // pause session
    } else if (currentState.inSession && currentState.inPause) {
      currentState.unpauseTriggered = true; //unpause session
    }
    i_ActionLen_millis = 0;
  }

  // RESET BUTTON DECISION
  if (i_ResetLen_millis >= 1000) {
    Serial.println("...Reset Pressed... ");
    if (currentState.inPause && !currentState.reset) {
      currentState.reset = true; // end Session
    }
    i_ResetLen_millis = 0;
  }

  // Start session
  if (!currentState.inSession && currentState.startSession) {
    currentState.inSession = true;
    currentState.startSession = false;
    Serial.println(":: Starting new session");
    startPeriod(FOCUS); // start focus session
  }

  // Pause/Unpause Session
  if (currentState.inSession && currentState.pauseTriggered) {
    pauseSession();
    currentState.inPause = true;
    currentState.pauseTriggered = false;
  }
  if (currentState.inSession && currentState.unpauseTriggered) {
    unpauseSession();
    currentState.inPause = false;
    currentState.unpauseTriggered = false;
  }

  // End Session
  if (currentState.inSession && currentState.reset) {
    endSession(); //end session
  }

  if (!currentState.inPause) {
    // Track focus progress
    if (currentState.inSession && currentState.inFocus) {
      //printTime(currentTime);
      if (currentSession.i_endTimeHour == currentTime.hour() && currentSession.i_endTimeMin == currentTime.minute()) {
        if (currentSession.i_sessionCount != 0) {
          currentSession.i_sessionCount --; //decrement focus sessions
          startPeriod(BREAK); //start new break
        } else {
          endSession(); //end session
        }
      }
    }

    // Track break progress
    if (currentState.inSession && currentState.inBreak) {
      if (currentSession.i_endTimeHour == currentTime.hour() && currentSession.i_endTimeMin == currentTime.minute()) {
        startPeriod(FOCUS); // start focus session
      }
    }
  }

}

void startPeriod(uint8_t sessionLength) {
  if (sessionLength == FOCUS) {
    toggleLights({244, 149, 66});
    Serial.println(">> Starting focus");
    currentState.inFocus = true;
    currentState.inBreak = false;
  }
  if(sessionLength == BREAK){
    toggleLights({166, 244, 66});
    Serial.println(">> Starting break");
    currentState.inFocus = false;
    currentState.inBreak = true;
  }
  DateTime startTime = rtc.now();
  DateTime endTime = startTime + TimeSpan(0, 0, sessionLength, 0);
  currentSession.i_startTimeHour = startTime.hour();
  currentSession.i_startTimeMin = startTime.minute();
  currentSession.i_endTimeHour = endTime.hour();
  currentSession.i_endTimeMin = endTime.minute();

  Serial.print("Start time: ");
  Serial.print(startTime.hour());
  Serial.print(":");
  Serial.println(startTime.minute());
  Serial.print("End time: ");
  Serial.print(endTime.hour());
  Serial.print(":");
  Serial.println(endTime.minute());
}

void printTime(DateTime curTime) {
  uint8_t thisSecond = curTime.second();
  uint8_t thisMinute = curTime.minute();
  if (thisMinute != currentMinute) {
    currentMinute = curTime.minute();
  }
  if (thisSecond != currentSecond) {
    currentSecond = curTime.second();
    Serial.print(currentMinute);
    Serial.print(":");
    Serial.println(currentSecond);
  }
}

void endSession() {
  toggleLights({255, 255, 255});
  Serial.println(">> Quitting session");
  currentState.inSession = false;
  currentState.startSession = false;
  currentState.inFocus = false;
  currentState.inBreak = false;
  currentState.reset = false;
  currentSession.i_sessionCount = 4;
}

void pauseSession() {
  toggleLights({215,125,233});
  Serial.println(">> Pausing session");
  DateTime pauseTime = rtc.now();
  if (currentSession.i_endTimeMin < pauseTime.minute()) {
    currentSession.i_timeRemaining = (60 - pauseTime.minute()) + currentSession.i_endTimeMin;
  } else {
    currentSession.i_timeRemaining = currentSession.i_endTimeMin - pauseTime.minute();
  }
};

void unpauseSession() {
  currentState.inFocus ? toggleLights({244, 149, 66}): toggleLights({244, 244, 66});;
  Serial.println(">> Unpausing session");
  DateTime unpauseTime = rtc.now();
  DateTime newEndTime = unpauseTime + TimeSpan(0, 0, currentSession.i_timeRemaining, 0);
  currentSession.i_endTimeHour = newEndTime.hour();
  currentSession.i_endTimeMin = newEndTime.minute();

  Serial.print("End time: ");
  Serial.print(newEndTime.hour());
  Serial.print(":");
  Serial.println(newEndTime.minute());
}

void toggleLights(colour col) {
  analogWrite(L_RED, col.r);
  analogWrite(L_GREEN, col.g);
  analogWrite(L_BLUE, col.b);
};
