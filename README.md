# RGB LED Pomodoro - Arduino Prototype
> Prototype codebase for an LED-based desktop pomodoro device

## Table of Contents
* [General Information](#general-information)
* [Technology Used](#technology-used)
* [Project Status](#project-status)
* [Acknowledgements](#acknowledgements)

## General Information
This project is part of a series of explorations I made into the realm of embedded devices for the home. 

In essence it is a pomodoro device that utilises coloured light to indicate time intervals, creating a more subdued experience of productivity.

The coloured light is shone down onto the keyboard/workspace to indicate session cycles. The sessions are configured, started and stopped with capacitive touch buttons. 

## Technology Used
The prototype is built on the Arduino framework implementing various modules for functionality.
* DS3231 Real-Time Clock for tracking time during meditation sessions
* RGB LED light strip
* 3 x TIP122 NPN transistors for powering the LED strip
* 3 x capacitive touch buttons for control and power

## Project Status
* Unit-testing
* User-testing
* Transfer to PCB
* Device design and prototyping

## Acknowledgements
[Arduino](https://www.arduino.cc/)

[Adafruit RTClib](https://adafruit.github.io/RTClib/html/class_time_span.html#details)
