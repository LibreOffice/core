## 1.2.6 - June 11, 2013

- Fixed a bug with crash reporting on iOS 7

## 1.2.5 - May 1, 2013

- Removed any access to mac address
- Added AdSupport.framework requirement
    - AdSupport.framework is automatically loaded on iOS 6.0+ if not weak linked with app

## 1.2.4 - February 19, 2013

- Fixed bug that caused crash reports to sometimes not send immediately (they would be resent later)

## 1.2.3 - January 8, 2013

- Fixed typos in readme
- Fixed bug where logs not sent on crash
- Fixed bug where empty crash files were created (but not sent)
- Cache cache path
- Use consts for `setOptions:`
- Updated `setDeviceIdentifier:` comments to make them clearer
- Remove potentially conflicting function name `UIColorFromRGB`
- Fixed crash on bad in app update data

## 1.2.2 - December 26, 2012

- Fix typo in app token error message

## 1.2.1 - December 26, 2012

- The max number of concurrent network connections has been reduced from 4 to 2.

##1.2 - November 12, 2012

* Removed Team Token support. As of version 1.2 takeOff must be called with the Application Token, https://testflightapp.com/dashboard/applications/, choose your application, select SDK, get the Token for this Application.

##1.2 BETA 3 - October 11, 2012

* Added application token support. Application Tokens are currently optional if you do not have one you do not need one

##1.2 BETA 2 - October 9, 2012

* Resolved an instance of close_file being called on a bad file descriptor

##1.2 BETA 1 - October 1, 2012

* Removed support for armv6
* Exception handler now returns instead of raising a SIGTRAP

##1.1 - September 13, 2012

* armv7s and iOS 6 support
* Updated for general release

##1.1 BETA 3 - September 12, 2012

* armv7s slice added to library
* fixed typo for in application updates, inAppUdates changed to inAppUpdates

##1.1 BETA 2 - September 6, 2012

* Re-enabled armv6 support
* Added option to disable in application updates

##1.1 BETA 1 - July 13, 2012

* Added TFLogv to allow for log customizations. Check the README or online docs for more information.
* Added option attachBacktraceToFeedback, which attaches a backtrace to feedback sent from the SDK. For users who use feedback in more than one location in the application.
* Resolved issue where other exception handlers would not be called during an exception.
* SDK now sends the device language for a session.
* Documentation fixes.
* Stability fixes.

###1.0 - March 29, 2012

* Resolved occurrences of exceptions with the message "No background task exists with identifier 0"

###1.0 BETA 1 - March 23, 2012

* Privacy Updates
* UDID is no longer collected by the SDK. During testing please use `[TestFlight setDeviceIdentifier:[[UIDevice currentDevice] uniqueIdentifier]];` to send the UDID so you can identify your testers. For release do not set `+setDeviceIdentifier`. See Beta Testing and Release Differentiation in the README or online at [https://testflightapp.com/sdk/doc/1.0beta1/](http://testflightapp.com/sdk/doc/1.0beta1/)

###0.8.3 - February 14, 2012

* Rolled previous beta code into release builds
* No longer allow in application updates to occur in applications that were obtained from the app store.

**Tested compiled library with:**

* Xcode 4.3
* Xcode 4.2
* Xcode 4.1
* Xcode 3.2.6

###0.8.3 BETA 5 - February 10, 2012

* Changed logging from asynchronous to synchronous.
* Resolved crash when looking for a log path failed.
* Added submitFeedback to the TestFlight class to allow for custom feedback forms.

###0.8.3 BETA 4 - January 20, 2012

* Resolved an issue that occured when an application was upgraded from 0.8.3 BETA 1 to 0.8.3 BETA 3+ with unsent data from 0.8.3 BETA 1

###0.8.3 BETA 3 - January 19, 2012

* On crash log files over 64k will not be sent until next launch.

**Known Issues:**

* Logging massive amounts of data at the end of a session may prevent the application from launching in time on next launch

###0.8.3 BETA 2 - January 13, 2012

* libz.dylib is now required to be added to your "Link Binary with Libraries" build phase
* Log file compression, The compression is done on an as needed basis rather than before sending
* Changed all outgoing data from JSON to MessagePack
* Added option `logToSTDERR` to disable the `STDERR` logger

###0.8.3 BETA 1 - December 29, 2011

* In rare occurrences old session data that had not been sent to our server may have been discarded or attached to the wrong build. It is now no longer discarded
* Made sending of Session End events more robust
* Network queuing system does better bursting of unsent data
* Log files that are larger than 64K are now sent sometime after the next launch
* Log files that are larger than 16MB are no longer supported and will be replaced with a message indicating the log file was too large
* Fixed crashes while resuming from background

###0.8.2 - December 20, 2011

* Promoted 0.8.2 BETA 4 to stable

**Known Issues:**

* Under some circumstances Session End events may not be sent until the next launch.
* With large log files Session End events may take a long time to show up.

**Tested compiled library with:**

* Xcode 4.3
* Xcode 4.2
* Xcode 4.1
* Xcode 3.2.6

###0.8.2 BETA 4 - December 12, 2011

* Prevented "The string argument is NULL" from occuring during finishedHandshake in rare cases
* Resolved issue where data recorded while offline may not be sent

###0.8.2 BETA 3 - December 8, 2011

* Added auto-release pools to background setup and tear down

###0.8.2 BETA 2 - December 5, 2011

* Fixed the "pointer being freed was not allocated" bug

###0.8.1 - November 18, 2011

* Implemented TFLog logging system, see README for more information
* Fixed an issue where Session End events may not be sent until next launch
* Fixed an issue where duplicate events could be sent
* Fixed an issue with Session End events not being sent from some iPod touch models

**Tested compiled library with:**

* Xcode 4.2
* Xcode 4.1
* Xcode 3.2.6

###0.8 - November 8, 2011

* Added `SIGTRAP` as a signal type that we catch
* Removed all Objective-c from crash reporting
* Removed the use of non signal safe functions from signal handling
* Created a signal safe way to get symbols from a stack trace
* Changed the keyboardType for Long Answer Questions and Feedback to allow for international character input
* Changed `TESTFLIGHT_SDK_VERSION` string to be an `NSString`
* Changed cache folder from Library/Caches/TestFlight to Library/Caches/com.testflight.testflightsdk
* Fixed issue with saving data when device is offline
* Fixed compability issues with iOS 3
* Added calling into the rootViewController shouldAutorotateToInterfaceOrientation if a rootViewController is set
* Made the comments in TestFlight.h compatible with Appledoc

Tested compiled library with:

* Xcode 4.2
* Xcode 4.1
* Xcode 3.2

###0.7.2 - September 29, 2011

* Changed `TESTFLIGHT_SDK_VERSION` string to be an `NSString`
* Fixed an issue where exiting an application while the SDK is active caused modal views to be dismissed

###0.7.1 - September 22, 2011

* Internal release
* Refactoring

###0.7 - September 21, 2011

* Moved TestFlight images and data to the Library/Caches folder
* Resolved an issue where sometimes the rootViewController could not be found and feedback, questions and upgrade views would not be displayed
* In application upgrade changed to allow skipping until the next version is installed and allows upgrades to be forced
* Fixed a memory leak when launching questions

###0.6 - September 2, 2011

* Renamed base64_encode to testflight_base64_encode to remove a conflict with other third party libraries
* Added ability to reinstall crash handlers when they are overwritten using the setOptions API
* Fixed an issue where crash reports might not get sent under certain circumstances
* Fixed a deadlock when the application is put in the background and then resumed before all information can be sent
* Fixed an issue when attempting to un-install all signal handlers during a signal
* Added support for landscape mode on the iPad to the Questions and Feedback views
* Crash reporting now works in versions of Xcode earlier than 4.2
* Fixed a memory leak during handshake

###0.5 - August 19, 2011

* Feedback that is not attached to a checkpoint [TestFlight openFeedbackView]
* Usability changes to question views
* Removed pause and resume sessions, replaced with sessions being stopped and started
* Added text auto correction to the Long Answer question type
* Crash reports now send on crash instead of next launch

###0.4 - August 15, 2011

* In Application Feedback with Questions
* In application updates
* Custom Environment Information added
* Networking stack reimplementation
* Exception handling fixes

###0.3 - June 15, 2011

* Removed all mention of JSONKit from the README
* Added support for using both the Bundle Version and the Bundle Short Version string

###0.2 - June 14, 2011

* Removed all categories this allows users to use the SDK without having to set -ObjC and -load_all
* Prefixed JSONKit for use in TestFlight to remove reported issues where some users were already using JSONKit
* Added support for armv6 again

###0.1 - June 11, 2011

* Initial Version
