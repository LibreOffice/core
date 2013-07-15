##Introduction

The TestFlight SDK allows you to track how beta testers are testing your application. Out of the box we track simple usage information, such as which tester is using your application, their device model/OS, how long they used the application, logs of their test session, and automatic recording of any crashes they encounter.

To get the most out of the SDK we have provided the Checkpoint API.

The Checkpoint API is used to help you track exactly how your testers are using your application. Curious about which users passed level 5 in your game, or posted their high score to Twitter, or found that obscure feature? With a single line of code you can finally gather all this information. Wondering how many times your app has crashed? Wondering who your power testers are? We've got you covered. See more information on the Checkpoint API in section 4.

Alongside the Checkpoint API is the Questions interface. The Questions interface is managed on a per build basis on the TestFlight website. Find out more about the Questions Interface in section 6.

For more detailed debugging we have a remote logging solution. Find out more about our logging system with TFLog in the Remote Logging section.


##Considerations
       
Information gathered by the SDK is sent to the website in real time. When an application is put into the background (iOS 4.x) or terminated (iOS 3.x) we try to send the finalizing information for the session during the time allowed for finalizing the application. Should all of the data not get sent the remaining data will be sent the next time the application is launched. As such, to get the most out of the SDK we recommend your application support iOS 4.0 and higher.

This SDK can be run from both the iPhone Simulator and Device and has been tested using Xcode 4.0.

The AdSupport.framework is required for iOS 6.0+ in order to estimate the number of unique users your app has. You may weak link the framework in you app. If your app does not link with the AdSupport.framework, the TestFlight SDK will automatically load it for apps running on iOS 6.0+.

                
##Integration

1. Add the files to your project: File -&gt; Add Files to " "
    1. Find and select the folder that contains the SDK
    2. Make sure that "Copy items into destination folder (if needed)" is checked
    3. Set Folders to "Create groups for any added folders"
    4. Select all targets that you want to add the SDK to
2. Verify that libTestFlight.a has been added to the Link Binary With Libraries Build Phase for the targets you want to use the SDK with     
    1. Select your Project in the Project Navigator
    2. Select the target you want to enable the SDK for
    3. Select the Build Phases tab
    4. Open the Link Binary With Libraries Phase
    5. If libTestFlight.a is not listed, drag and drop the library from your Project Navigator to the Link Binary With Libraries area
    6. Repeat Steps 2 - 5 until all targets you want to use the SDK with have the SDK linked
3. Add libz to your Link Binary With Libraries Build Phase
    1. Select your Project in the Project Navigator
    2. Select the target you want to enable the SDK for
    3. Select the Build Phases tab
    4. Open the Link Binary With Libraries Phase
    5. Click the + to add a new library
    6. Find libz.dylib in the list and add it
    7. Repeat Steps 2 - 6 until all targets you want to use the SDK with have libz.dylib
    
4. In your Application Delegate:
    1. Import TestFlight: `#import "TestFlight.h"`                      

        ***NOTE:*** Rather than importing `TestFlight.h` in every file you may add the above line into you pre-compiled header (`<projectname>_Prefix.pch`) file inside of the


            #ifdef __OBJC__ 


        section. This will give you access to the SDK across all files.

    2. Get your Application Token which you can find at [http://testflightapp.com/dashboard/applications/](http://testflightapp.com/dashboard/applications/) select the application you are using from the list choose the SDK option and the application token for this application will be there. To ensure that your testers do not show up as anonymous place the call to setDeviceIdentifer before calling takeOff. Remove #define TESTING 1 before building your release build for the App Store.

    3. Launch TestFlight with your Application Token

            -(BOOL)application:(UIApplication *)application 
                didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
            // start of your application:didFinishLaunchingWithOptions 

            // !!!: Use the next line only during beta
            // [TestFlight setDeviceIdentifier:[[UIDevice currentDevice] uniqueIdentifier]];
            
            [TestFlight takeOff:@"Insert your Application Token here"];
            // The rest of your application:didFinishLaunchingWithOptions method
            // ...
            }

    4. To report crashes to you we install our own uncaught exception handler. If you are not currently using an exception handler of your own then all you need to do is go to the next step. If you currently use an Exception Handler, or you use another framework that does please go to the section on advanced exception handling.


##Beta Testing and Release Differentiation

In order to provide more information about your testers while beta testing you will need to provide the device's unique identifier. This identifier is not something that the SDK will collect from the device and we do not recommend using this in production. To send the device identifier to us put the following code **before your call to takeOff**.

    [TestFlight setDeviceIdentifier:[[UIDevice currentDevice] uniqueIdentifier]];
    [TestFlight takeOff:@"Insert your Application Token here"];

This will allow you to have the best possible information during testing. **When it is time to submit to the App Store comment this line out**. Apple may reject your app if you leave this line in. If you decide to not include the device's unique identifier during your testing phase TestFlight will still collect all of the information that you send but it may be anonymized.

    
##Checkpoint API

When a tester does something you care about in your app you can pass a checkpoint. For example completing a level, adding a todo item, etc. The checkpoint progress is used to provide insight into how your testers are testing your apps. The passed checkpoints are also attached to crashes, which can help when creating steps to replicate.

`[TestFlight passCheckpoint:@"CHECKPOINT_NAME"];` Use `passCheckpoint:` to track when a user performs certain tasks in your application. This can be useful for making sure testers are hitting all parts of your application, as well as tracking which testers are being thorough.

##Feedback API
                
To launch unguided feedback call the openFeedbackView method. We recommend that you call this from a GUI element.

    -(IBAction)launchFeedback {
        [TestFlight openFeedbackView];
    }

If you want to create your own feedback form you can use the submitCustomFeedback method to submit the feedback that the user has entered.

    -(IBAction)submitFeedbackPressed:(id)sender {
        NSString *feedback = [self getUserFeedback];
        [TestFlight submitFeedback:feedback];
    }

The above sample assumes that [self getUserFeedback] is implemented such that it obtains the users feedback from the GUI element you have created and that submitFeedbackPressed is the action for your submit button.

Once users have submitted feedback from inside of the application you can view it in the feedback area of your build page.


##Upload your build
    
After you have integrated the SDK into your application you need to upload your build to TestFlight. You can upload from your dashboard or or using the Upload API, full documentation at [https://testflightapp.com/api/doc/](https://testflightapp.com/api/doc/)

##Questions Interface

In order to ask a question, you'll need to associate it with a checkpoint. Make sure your checkpoints are initialized by running your app and hitting them all yourself before you start adding questions.

There are three question types available: Yes/No, Multiple Choice, and Long Answer.

To create questions, visit your builds Questions page and click on 'Add Question'. If you choose Multiple Choice, you'll need to enter a list of possible answers for your testers to choose from — otherwise, you'll only need to enter your question's, well, question. If your build has no questions, you can also choose to migrate questions from another build (because seriously — who wants to do all that typing again)?

After restarting your application on an approved device, when you pass the checkpoint associated with your questions a TestFlight modal question form will appear on the screen asking the beta tester to answer your question.

After you upload a new build to TestFlight you will need to associate questions once again. However if your checkpoints and questions have remained the same you can choose "copy questions from an older build" and choose which build to copy the questions from.

##View the results
                
As testers install your build and start to test it you will see their session data on the web on the build report page for the build you've uploaded.

##Advanced Exception Handling

An uncaught exception means that your application is in an unknown state and there is not much that you can do but try and exit gracefully. Our SDK does its best to get the data we collect in this situation to you while it is crashing, but it is designed in such a way that the important act of saving the data occurs in as safe way a way as possible before trying to send anything. If you do use uncaught exception or signal handlers install your handlers before calling `takeOff`. Our SDK will then call your handler while ours is running. For example:

      /*
       My Apps Custom uncaught exception catcher, we do special stuff here, and TestFlight takes care of the rest
      */
      void HandleExceptions(NSException *exception) {
        NSLog(@"This is where we save the application data during a exception");
        // Save application data on crash
      }
      /*
       My Apps Custom signal catcher, we do special stuff here, and TestFlight takes care of the rest
      */
      void SignalHandler(int sig) {
        NSLog(@"This is where we save the application data during a signal");
        // Save application data on crash
      }

      -(BOOL)application:(UIApplication *)application 
      didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
        // installs HandleExceptions as the Uncaught Exception Handler
        NSSetUncaughtExceptionHandler(&HandleExceptions);
        // create the signal action structure 
        struct sigaction newSignalAction;
        // initialize the signal action structure
        memset(&newSignalAction, 0, sizeof(newSignalAction));
        // set SignalHandler as the handler in the signal action structure
        newSignalAction.sa_handler = &SignalHandler;
        // set SignalHandler as the handlers for SIGABRT, SIGILL and SIGBUS
        sigaction(SIGABRT, &newSignalAction, NULL);
        sigaction(SIGILL, &newSignalAction, NULL);
        sigaction(SIGBUS, &newSignalAction, NULL);
        // Call takeOff after install your own unhandled exception and signal handlers
        [TestFlight takeOff:@"Insert your Application Token here"];
        // continue with your application initialization
      }

You do not need to add the above code if your application does not use exception handling already.

##Remote Logging
       
To perform remote logging you can use the TFLog method which logs in a few different methods described below. In order to make the transition from NSLog to TFLog easy we have used the same method signature for TFLog as NSLog. You can easily switch over to TFLog by adding the following macro to your header

    #define NSLog TFLog

That will do a switch from NSLog to TFLog, if you want more information, such as file name and line number you can use a macro like

    #define NSLog(__FORMAT__, ...) TFLog((@"%s [Line %d] " __FORMAT__), __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)

Which will produce output that looks like

    -[HTFCheckpointsController showYesNoQuestion:] [Line 45] Pressed YES/NO

We have implemented three different loggers.

    1. TestFlight logger
    2. Apple System Log logger
    3. STDERR logger

Each of the loggers log asynchronously and all TFLog calls are non blocking. The TestFlight logger writes its data to a file which is then sent to our servers on Session End events. The Apple System Logger sends its messages to the Apple System Log and are viewable using the Organizer in Xcode when the device is attached to your computer. The ASL logger can be disabled by turning it off in your TestFlight options

    [TestFlight setOptions:{ TFOptionLogToConsole : @NO }];

The default option is YES.

The STDERR logger sends log messages to STDERR so that you can see your log statements while debugging. The STDERR logger is only active when a debugger is attached to your application. If you do not wish to use the STDERR logger you can disable it by turning it off in your TestFlight options

    [TestFlight setOptions:{ TFOptionLogToSTDERR : @NO }];

The default option is YES.

## Advanced Remote Logging

For most users we expect using TFLog to provide all of the logging functionality that they need. For the occasion where you need to provide a wrapper around TFLog we provide 

    void TFLogv(NSString *format, va_list arg_list);

Using TFLogv you can have your method that accepts a variable number of arguments that then passes that format and argument list to TFLog.


##iOS3
        
We now require that anyone who is writing an application that supports iOS3 add the System.framework as an optional link. In order to provide a better shutdown experience we send any large log files to our servers in the background. To add System.framework as an optional link: 
    
1. Select your Project in the Project Navigator 
2. Select the target you want to enable the SDK for 
3. Select the Build Phases tab 
4. Open the Link Binary With Libraries Phase 
5. Click the + to add a new library 
6. Find libSystem.dylib in the list and add it 
7. To the right of libSystem.dylib in the Link Binary With Libraries pane change "Required" to "Optional"
