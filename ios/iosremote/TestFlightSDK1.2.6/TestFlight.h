//
//  TestFlight.h
//  libTestFlight
//
//  Created by Jonathan Janzen on 06/11/11.
//  Copyright 2011 TestFlight. All rights reserved.

#import <Foundation/Foundation.h>
#define TESTFLIGHT_SDK_VERSION @"1.2.6"
#undef TFLog

#if __cplusplus
extern "C" { 
#endif
    void TFLog(NSString *format, ...);
    void TFLogv(NSString *format, va_list arg_list);
#if __cplusplus
}
#endif

/**
 * TestFlight object
 * All methods are class level
 */
@interface TestFlight : NSObject {

}

/**
 * Add custom environment information
 * If you want to track custom information such as a user name from your application you can add it here
 * 
 * @param information A string containing the environment you are storing
 * @param key The key to store the information with
 */
+ (void)addCustomEnvironmentInformation:(NSString *)information forKey:(NSString*)key;


/**
 * Starts a TestFlight session using the Application Token for this Application
 *
 * @param applicationToken Will be the application token for the current application.
 *                         The token for this application can be retrieved by going to https://testflightapp.com/dashboard/applications/
 *                         selecting this application from the list then selecting SDK.
 */

+ (void)takeOff:(NSString *)applicationToken;

/**
 * Sets custom options
 *
 * @param options NSDictionary containing the options you want to set. Available options are described below at "TestFlight Option Keys"
 *
 */
+ (void)setOptions:(NSDictionary*)options;

/**
 * Track when a user has passed a checkpoint after the flight has taken off. Eg. passed level 1, posted high score
 *
 * @param checkpointName The name of the checkpoint, this should be a static string
 */
+ (void)passCheckpoint:(NSString *)checkpointName;

/**
 * Opens a feedback window that is not attached to a checkpoint
 */
+ (void)openFeedbackView;

/**
 * Submits custom feedback to the site. Sends the data in feedback to the site. This is to be used as the method to submit
 * feedback from custom feedback forms.
 *
 * @param feedback Your users feedback, method does nothing if feedback is nil
 */
+ (void)submitFeedback:(NSString*)feedback;

/**
 * Sets the Device Identifier.
 *
 * !! DO NOT CALL IN SUBMITTED APP STORE APP.
 *
 * !! MUST BE CALLED BEFORE +takeOff:
 *
 * This method should only be used during testing so that you can identify a testers test data with them.
 * If you do not provide the identifier you will still see all session data, with checkpoints
 * and logs, but the data will be anonymized.
 * 
 * It is recommended that you only use this method during testing.
 * Apple may reject your app if left in a submitted app.
 *
 * Use:
 * Only use this with the Apple device UDID. DO NOT use Open ID or your own identifier.
 * [TestFlight setDeviceIdentifier:[[UIDevice currentDevice] uniqueIdentifier]];
 *
 * @param deviceIdentifer The current devices device identifier
 */
+ (void)setDeviceIdentifier:(NSString*)deviceIdentifer;

@end


/**
 * TestFlight Option Keys
 *
 * Pass these as keys to the dictionary you pass to +`[TestFlight setOptions:]`.
 * The values should be NSNumber BOOLs (`[NSNumber numberWithBool:YES]` or `@YES`)
 */
extern NSString *const TFOptionAttachBacktraceToFeedback; // Defaults to @NO. Setting to @YES attaches the current backtrace, with symbols, to the feedback.
extern NSString *const TFOptionDisableInAppUpdates; // Defaults to @NO. Setting to @YES, disables the in app update screen shown in BETA apps when there is a new version available on TestFlight.
extern NSString *const TFOptionLogToConsole; // Defaults to @YES. Prints remote logs to Apple System Log.
extern NSString *const TFOptionLogToSTDERR; // Defaults to @YES. Sends remote logs to STDERR when debugger is attached.
extern NSString *const TFOptionReinstallCrashHandlers; // If set to @YES: Reinstalls crash handlers, to be used if a third party library installs crash handlers overtop of the TestFlight Crash Handlers.
extern NSString *const TFOptionSendLogOnlyOnCrash; // Defaults to @NO. Setting to @YES stops remote logs from being sent when sessions end. They would only be sent in the event of a crash.

