// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <touch/touch.h>

#import "AppDelegate.h"
#import "DocumentTableViewController.h"
#import "View.h"
#import "ViewController.h"
#import "lo.h"

@interface AppDelegate ()

- (void)showDocumentList:(NSArray*)documents inFolder:(NSString*)folder;
- (void)threadMainMethod:(NSString *)documentPath;

@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.backgroundColor = [UIColor whiteColor];
    [self.window makeKeyAndVisible];

    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];

    NSDirectoryEnumerator *dirEnumerator = [fileManager enumeratorAtPath:documentsDirectory];
    int nDocs = 0;
    NSString *document;
    NSMutableArray *documents = [[NSMutableArray alloc] init];
    NSString *test1 = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent: @"test1.odt"];
    [documents addObject:test1];

    while ((document = [dirEnumerator nextObject])) {
        nDocs++;
        [documents addObject:[documentsDirectory stringByAppendingPathComponent:document]];
    }

    if (nDocs == 0) {
        [self startDisplaying:test1];
    } else {
        [documents sortUsingSelector:@selector(localizedStandardCompare:)];
        [self showDocumentList:documents inFolder:documentsDirectory];
    }

    return YES;
}

- (void)startDisplaying:(NSString*)documentPath;
{
    ViewController *vc = [[ViewController alloc] init];
    self.window.rootViewController = vc;

    [[[NSThread alloc] initWithTarget:self selector:@selector(threadMainMethod:) object:documentPath] start];
    vc.view = [[View alloc] initWithFrame:[self.window frame]];
}

- (void)threadMainMethod:(NSString *)documentPath
{
    @autoreleasepool {
        lo_initialize(documentPath);
        touch_lo_runMain();
    }
}

- (void)showDocumentList:(NSArray*)documents inFolder:(NSString*)folder
{
    UITableViewController *vc = [DocumentTableViewController createForDocuments:documents forAppDelegate:self];
    self.window.rootViewController = vc;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to
    // inactive state. This can occur for certain types of temporary
    // interruptions (such as an incoming phone call or SMS message)
    // or when the user quits the application and it begins the
    // transition to the background state. Use this method to pause
    // ongoing tasks, disable timers, and throttle down OpenGL ES
    // frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data,
    // invalidate timers, and store enough application state
    // information to restore your application to its current state in
    // case it is terminated later. If your application supports
    // background execution, this method is called instead of
    // applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the
    // inactive state; here you can undo many of the changes made on
    // entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while
    // the application was inactive. If the application was previously
    // in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if
    // appropriate. See also applicationDidEnterBackground:.
}

@end

// dummies

void touch_ui_selection_start(MLOSelectionKind kind,
                              const void *documentHandle,
                              MLORect *rectangles,
                              int rectangleCount,
                              void *preview)
{
}

void touch_ui_selection_none()
{
}

MLODialogResult touch_ui_dialog_modal(MLODialogKind kind, const char *message)
{
    return MLODialogCancel;
}

void touch_ui_show_keyboard()
{
}

void touch_ui_hide_keyboard()
{
}

void touch_ui_damaged(int minX, int minY, int width, int height)
{
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
