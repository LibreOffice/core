// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOKeyboardManager.h"
#import "MLOMainViewController_Friend.h"
#import "MLOManager.h"
#import "MLOGestureEngine_Impl.h"
#include <touch/touch.h>

@interface MLOKeyboardManager ()
@property MLOMainViewController * mainViewController;
@property UITextView * textView;
@property BOOL allowLoToinvokdeKeyboard;
@property BOOL isShown;
@end

@implementation MLOKeyboardManager

-(id)initWithMainViewController:(MLOMainViewController *) mainViewController{
    self = [self init];
    if(self){
        
        self.mainViewController = mainViewController;
        
        [self initTextView];
        
        self.isShown = NO;
        
        id defaultCenter = [NSNotificationCenter defaultCenter];
        
        [defaultCenter addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
        [defaultCenter addObserver:self selector:@selector(keyboardDidHide:) name:UIKeyboardDidHideNotification object:nil];
    }
    return self;
}
-(void)addToMainViewController{
    [self.mainViewController.canvas addSubview:self.textView];
}
-(void)hideLibreOffice{
    self.allowLoToinvokdeKeyboard =NO;
    [self hide];
}
-(void)showLibreOffice{
    self.allowLoToinvokdeKeyboard =NO;
}
-(void)initTextView{
    self.textView = [[UITextView alloc] initWithFrame:CGRECT_ONE];
    self.textView.alpha = 0.0f;
    self.textView.autocapitalizationType = UITextAutocapitalizationTypeNone;
    self.textView.delegate = self;
}

-(void)show{
    self.isShown =YES;
    NSLog(@"MLOKeyboardManager : show");
    [self.textView becomeFirstResponder];
}

-(void)hide{
    if(self.isShown){
       self.isShown =NO;
        NSLog(@"MLOKeyboardManager : hide");
        [self.textView resignFirstResponder];
    }
}

-(BOOL)textView:(UITextView *)textView shouldChangeTextInRange:(NSRange)range replacementText:(NSString *)text
{
    NSLog(@"textView: %@ shouldChangeTextInRange:[%u,%u] replacementText:%@", textView, range.location, range.length, text);

    NSInteger length = [text length];
    
    for (NSUInteger i = 0; i < length; i++){
        touch_lo_keyboard_input([text characterAtIndex: i]);
    }

    if(length>0){
        [self.mainViewController onTextEdit];
    }
    return NO;
}

-(void)loInvokeKeyboard{
    if(self.allowLoToinvokdeKeyboard){
        [self show];
    }else{
        self.allowLoToinvokdeKeyboard = YES;
    }
}

-(BOOL)canBecomeFirstResponder{
    return YES;
}

-(void)keyboardWillShow:(NSNotification *)note{
    IGNORE_ARG(note);
    [self.mainViewController.gestureEngine onKeyboardShow];
}

-(void)keyboardDidHide:(NSNotification *)note{
    IGNORE_ARG(note);
    [self.mainViewController.gestureEngine onKeyboardHide];
}


// C functions
// ===========
//
// Functions called in the LO thread, which thus need to dispatch any
// CocoaTouch activity to happen on the GUI thread. Use
// dispatch_async() consistently.

void touch_ui_show_keyboard()
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [[MLOManager getInstance].mainViewController.keyboard loInvokeKeyboard];
    });
}

void touch_ui_hide_keyboard()
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [[MLOManager getInstance].mainViewController.keyboard hide];
    });
}

bool touch_ui_keyboard_visible()
{
    // Should return info whether the soft keyboard is currently displayed,
    // or a hardware keyboard is attached/paired.
    return [MLOManager getInstance].mainViewController.keyboard.isShown;
}

@end
