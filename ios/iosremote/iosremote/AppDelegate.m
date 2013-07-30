// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "AppDelegate.h"

@implementation AppDelegate

@synthesize window = _window;

- (NSUInteger)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(UIWindow *)window
{
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
        return UIInterfaceOrientationMaskAll;
    else
        return UIInterfaceOrientationMaskPortrait;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    [TestFlight takeOff:@"cb645cf2-a557-481e-993a-6ad3aaeb06d0"];
    
    NSString *defaultsPath = [[NSBundle mainBundle] pathForResource:@"UserDefaults"
                                                             ofType:@"plist"];
    NSDictionary *appDefaults = [NSDictionary dictionaryWithContentsOfFile:defaultsPath];
    [[NSUserDefaults standardUserDefaults] registerDefaults:appDefaults];
    
    // Override point for customization after application launch.
    /**
     * If ever we need some iOS6-only storyboard based features and we want to keep backward compatibility, we should uncomment these code to pick the right storyboard based on the existence on certains classes.
     */
//    UIStoryboard *mainStoryboard = nil;
//    if (NSClassFromString(@"NSLayoutConstraint")) {
//        mainStoryboard = [UIStoryboard storyboardWithName:@"iPhone_autolayout" bundle:nil];
//        NSLog(@"loading autolayout storyboard");
//    } else {
//        mainStoryboard = [UIStoryboard storyboardWithName:@"iPhone_autosize" bundle:nil];
//        NSLog(@"Doesn't support autolayout, loading autosize");
//    }
//
//    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
//    self.window.rootViewController = [mainStoryboard instantiateInitialViewController];
//    [self.window makeKeyAndVisible];
    
    return YES;
}

- (CMMotionManager *)motionManager
{
    if (!motionManager) motionManager = [[CMMotionManager alloc] init];
    return motionManager;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
