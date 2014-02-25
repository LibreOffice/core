// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// This file is part of the LibreOffice project.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <UIKit/UIKit.h>

#include <touch/touch.h>

#import "AppDelegate.h"
#import "ViewController.h"

#import "lo.h"

static View *theView;
static BOOL keyboardShows;

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

    self.view = [[View alloc] initWithFrame: r];
    vc.view = self.view;
    theView = self.view;

    self.view->textView = [[UITextView alloc] initWithFrame: r];
    self.view->textView.autocapitalizationType = UITextAutocapitalizationTypeNone;
    self.view->textView.alpha = 0;
    [self.view addSubview: self.view->textView];
    self.view->textView.delegate = self;

    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self.view action:@selector(tapGesture:)];
    UIPanGestureRecognizer *panRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:self.view action:@selector(panGesture:)];
    UILongPressGestureRecognizer * longPressRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self.view action:@selector(longPressGesture:)];

    [self.window addGestureRecognizer: tapRecognizer];
    [self.window addGestureRecognizer: panRecognizer];
    [self.window addGestureRecognizer: longPressRecognizer];

    NSLog(@"statusBarOrientation: %ld", (long) [[UIApplication sharedApplication] statusBarOrientation]);

    if (UIInterfaceOrientationIsLandscape([[UIApplication sharedApplication] statusBarOrientation]))
        touch_lo_set_view_size(r.size.height, r.size.width);
    else
        touch_lo_set_view_size(r.size.width, r.size.height);

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardDidHide:) name:UIKeyboardDidHideNotification object:nil];

    NSThread* thread = [[NSThread alloc] initWithTarget:self
                                               selector:@selector(threadMainMethod:)
                                                 object:nil];
    keyboardShows = NO;

    [thread start];

    return YES;
}

- (void)threadMainMethod:(id)argument
{
    (void) argument;

    @autoreleasepool {
        lo_initialize();
        touch_lo_runMain();
    }
}

- (BOOL)textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text
{
    NSLog(@"textView: %@ shouldChangeTextInRange:[%lu,%lu] replacementText:%@", textView, (unsigned long) range.location, (unsigned long) range.length, text);
    assert(textView == theView->textView);

    for (NSUInteger i = 0; i < [text length]; i++)
        touch_lo_keyboard_input([text characterAtIndex: i]);

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
    NSLog(@"statusBarOrientation: %ld", (long) [[UIApplication sharedApplication] statusBarOrientation]);

    if (UIInterfaceOrientationIsLandscape([[UIApplication sharedApplication] statusBarOrientation]))
        touch_lo_set_view_size(applicationFrame.size.height, applicationFrame.size.width);
    else
        touch_lo_set_view_size(applicationFrame.size.width, applicationFrame.size.height);
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

    keyboardShows = YES;
}

- (void)keyboardDidHide:(NSNotification *)note
{
    (void) note;

    NSLog(@"keyboardDidHide");

    keyboardShows = NO;

    touch_lo_keyboard_did_hide();
}

@end

// Functions called in the LO thread, which thus need to dispatch any
// CocoaTouch activity to happen on the GUI thread. Use
// dispatch_async() consistently.

void touch_ui_damaged(int minX, int minY, int width, int height)
{
    CGRect rect = CGRectMake(minX, minY, width, height);
    dispatch_async(dispatch_get_main_queue(), ^{
            [theView setNeedsDisplayInRect:rect];
        });
    // NSLog(@"lo_damaged: %dx%d@(%d,%d)", width, height, minX, minY);
}

void touch_ui_show_keyboard()
{
#if 0
    // Horrible hack
    static bool beenHere = false;
    if (!beenHere) {
        beenHere = true;
        touch_lo_keyboard_did_hide();
        return;
    }
#endif
    dispatch_async(dispatch_get_main_queue(), ^{
            [theView->textView becomeFirstResponder];
        });
}

void touch_ui_hide_keyboard()
{
    dispatch_async(dispatch_get_main_queue(), ^{
            [theView->textView resignFirstResponder];
        });
}

bool touch_ui_keyboard_visible()
{
    return keyboardShows;
}

static const char *
dialog_kind_to_string(MLODialogKind kind)
{
    switch (kind) {
    case MLODialogMessage:
        return "MSG";
    case MLODialogInformation:
        return "INF";
    case MLODialogWarning:
        return "WRN";
    case MLODialogError:
        return "ERR";
    case MLODialogQuery:
        return "QRY";
    default:
        return "WTF";
    }
}

MLODialogResult touch_ui_dialog_modal(MLODialogKind kind, const char *message)
{
    NSLog(@"===>  %s: %s", dialog_kind_to_string(kind), message);
    return MLODialogOK;
}

void touch_ui_selection_start(MLOSelectionKind kind,
                              const void *documentHandle,
                              MLORect *rectangles,
                              int rectangleCount,
                              void *preview)
{
    (void) preview;

    // Note that this is called on the LO thread
    dispatch_async(dispatch_get_main_queue(), ^{
            [theView startSelectionOfType:kind withNumber:rectangleCount ofRectangles:rectangles forDocument:documentHandle];
        });
}

void touch_ui_selection_none()
{
    dispatch_async(dispatch_get_main_queue(), ^{
            [theView startSelectionOfType:MLOSelectionNone withNumber:0 ofRectangles:NULL forDocument:NULL];
        });
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
