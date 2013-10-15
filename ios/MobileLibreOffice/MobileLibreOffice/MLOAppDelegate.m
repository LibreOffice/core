// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOAppDelegate.h"
#import "MLOAppViewController.h"
#import "MLOFileManagerViewController.h"
#import "MLOManager.h"
#import "NSObject+MLOUtils.h"


@implementation MLOAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    
    [[UIApplication sharedApplication]setStatusBarHidden:YES];
    
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    
    self.viewController = [[MLOAppViewController alloc] initWithAppDelegate:self];
    
    self.window.rootViewController = self.viewController;

    [self.window makeKeyAndVisible];
    
    [[UIApplication sharedApplication]setStatusBarHidden:YES];
    
    [[MLOManager getInstance] application:application didFinishLaunchingWithOptions:launchOptions];
    
    [[MLOManager getInstance] start];
    
    [self.viewController.fileManager show];


    return YES;
}


- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
    if (url != nil && [url isFileURL]) {
        
        for (NSString * extension in @[@"docx",@"odt"]) {
            if([[[url pathExtension]lowercaseString] isEqualToString:extension]){
                
                [self.viewController.fileManager openFilePath:[url path]];
            }
        }
        
    }
    
    return YES;
}

-(void)willShowLibreOffice{
    
}
-(void)didShowLibreOffice{
    
}
-(void)willHideLibreOffice{
    
}
-(void)didHideLibreOffice{
    
    [self.viewController.fileManager didHideLibreOffice];
}
-(CGRect) bounds{
    return self.window.bounds;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
    
    [[MLOManager getInstance] applicationWillResignActive:application];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    
    [[MLOManager getInstance] applicationDidEnterBackground:application];
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
    
    [[MLOManager getInstance] applicationWillEnterForeground:application];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    
    [[MLOManager getInstance] applicationWillTerminate:application];
}


@end
