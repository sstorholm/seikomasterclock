# seikomasterclock
ESP32 Seiko Master Clock Emulator

## Introduction

This project is intended for driving Seiko marine slave clocks with forward and reversing coils. The clocks expect a 24VDC pulse every 30 seconds of alternating polarity. This is achieved by using an ESP32 for the clock functions, which drives an L298N H-bridge that handles the 24VDC power switching.

## Features

- Button inputs for forwarding and reversing the clock in hours and half minutes
- NTP time synchronisation over WiFi
- Can drive hundreds of Seiko slave clocks

## TODO

- DST detection and automatic forwarding/reversing of the clocks
- General clean-up of code base

## Disclaimer
This project is in the early alpha stage. The driver works, but long-term stability hasn't been tested (rudimentary drift compensation has been done, but it's probably not even close to perfect). Use the included code at your own risk. 
