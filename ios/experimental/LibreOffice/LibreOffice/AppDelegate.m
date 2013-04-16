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

static View *theView;

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    (void) application;
    (void) launchOptions;

    CGRect bounds = [[UIScreen mainScreen] bounds];

    NSLog(@"mainScreen bounds: %dx%d@(%d,%d)",
          (int) bounds.size.width, (int) bounds.size.height,
          (int) bounds.origin.x, (int) bounds.origin.y);

    CGRect applicationFrame = [[UIScreen mainScreen] applicationFrame];

    NSLog(@"mainScreen applicationFrame: %dx%d@(%d,%d)",
          (int) applicationFrame.size.width, (int) applicationFrame.size.height,
          (int) applicationFrame.origin.x, (int) applicationFrame.origin.y);

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

    self.view->textView = [[UITextView alloc] initWithFrame: r];
    self.view->textView.autocapitalizationType = UITextAutocapitalizationTypeNone;
    self.view->textView.alpha = 0;
    [self.view addSubview: self.view->textView];
    self.view->textView.delegate = self;

    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self.view action:@selector(tapGesture:)];

    [self.window addGestureRecognizer: tapRecognizer];

    NSLog(@"statusBarOrientation: %d", [[UIApplication sharedApplication] statusBarOrientation]);

    if (UIInterfaceOrientationIsLandscape([[UIApplication sharedApplication] statusBarOrientation]))
        lo_set_view_size(applicationFrame.size.height, applicationFrame.size.width);
    else
        lo_set_view_size(applicationFrame.size.width, applicationFrame.size.height);

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardDidHide:) name:UIKeyboardDidHideNotification object:nil];

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

- (BOOL)textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text
{
    NSLog(@"textView: %@ shouldChangeTextInRange:[%u,%u] replacementText:%@", textView, range.location, range.length, text);
    assert(textView == theView->textView);

    for (NSUInteger i = 0; i < [text length]; i++)
        lo_keyboard_input([text characterAtIndex: i]);

    return NO;
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

- (void)application:(UIApplication *)application didChangeStatusBarFrame:(CGRect)oldStatusBarFrame
{
    (void) application;
    (void) oldStatusBarFrame;

    CGRect applicationFrame = [[UIScreen mainScreen] applicationFrame];
    NSLog(@"New applicationFrame: %dx%d@(%d,%d)",
          (int) applicationFrame.size.width, (int) applicationFrame.size.height,
          (int) applicationFrame.origin.x, (int) applicationFrame.origin.y);
    NSLog(@"statusBarOrientation: %d", [[UIApplication sharedApplication] statusBarOrientation]);

    if (UIInterfaceOrientationIsLandscape([[UIApplication sharedApplication] statusBarOrientation]))
        lo_set_view_size(applicationFrame.size.height, applicationFrame.size.width);
    else
        lo_set_view_size(applicationFrame.size.width, applicationFrame.size.height);
}

- (void)keyboardWillShow:(NSNotification *)note
{
    NSDictionary *info = [note userInfo];
    CGRect frameBegin;
    CGRect frameEnd;

    [[info objectForKey:UIKeyboardFrameBeginUserInfoKey] getValue:&frameBegin];
    [[info objectForKey:UIKeyboardFrameEndUserInfoKey] getValue:&frameEnd];

    NSLog(@"keyboardWillShow: frame:%dx%d@(%d,%d)",
          (int) frameEnd.size.width, (int) frameEnd.size.height,
          (int) frameEnd.origin.x, (int) frameEnd.origin.y);
}

- (void)keyboardDidHide:(NSNotification *)note
{
    (void) note;

    NSLog(@"keyboardDidHide");

    lo_keyboard_did_hide();
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
            [theView->textView becomeFirstResponder];
        });
}

void lo_hide_keyboard()
{
    dispatch_async(dispatch_get_main_queue(), ^{
            [theView->textView resignFirstResponder];
        });
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
