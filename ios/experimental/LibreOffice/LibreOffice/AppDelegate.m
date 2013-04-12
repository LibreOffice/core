// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <UIKit/UIKit.h>

#include <osl/detail/ios-bootstrap.h>
#include <touch/touch.h>

#import "AppDelegate.h"
#import "ViewController.h"

#import "lo.h"

static UIView *theView;

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    (void) application;
    (void) launchOptions;

    CGRect bounds = [[UIScreen mainScreen] bounds];
    self.window = [[UIWindow alloc] initWithFrame:bounds];
    self.window.backgroundColor = [UIColor whiteColor];

    ViewController *vc = [[ViewController alloc] init];
    self.window.rootViewController = vc;

    [self.window makeKeyAndVisible];
    
    CGRect r = [self.window frame];
    r.origin = CGPointMake(0, 0);

    self.view = [[View alloc] initWithFrame: r];
    vc.view = self.view;
    theView = self.view;

    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self.view action:@selector(tapGesture:)];

    [self.window addGestureRecognizer: tapRecognizer];

    lo_set_view_size(bounds.size.width, bounds.size.height);

    NSThread* thread = [[NSThread alloc] initWithTarget:self
                                               selector:@selector(threadMainMethod:)
                                                 object:nil];
    [thread start];

    return YES;
}

- (void)threadMainMethod:(id)argument
{
    (void) argument;

    @autoreleasepool {
        lo_initialize();
        lo_runMain();
    }
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    (void) application;
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    (void) application;
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    (void) application;
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    (void) application;
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    (void) application;
}

@end

void lo_damaged(CGRect rect)
{
    (void) rect;
    dispatch_async(dispatch_get_main_queue(), ^{
            [theView setNeedsDisplayInRect:rect];
        });
    // NSLog(@"lo_damaged: %dx%d@(%d,%d)", (int)rect.size.width, (int)rect.size.height, (int)rect.origin.x, (int)rect.origin.y);
}

void lo_show_keyboard()
{
    dispatch_async(dispatch_get_main_queue(), ^{
            [theView becomeFirstResponder];
        });
}

void lo_hide_keyboard()
{
    dispatch_async(dispatch_get_main_queue(), ^{
            [theView resignFirstResponder];
        });
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
