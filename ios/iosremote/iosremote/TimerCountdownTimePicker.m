//
//  TimerCountdownTimePicker.m
//  iosremote
//
//  Created by Siqi Liu on 9/7/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "TimerCountdownTimePicker.h"
#import <UIKit/UIKit.h>

#define MyDateTimePickerHeight 260
#define TimerCountdownTimePickerPickerHeight 216
#define TimerCountdownTimePickerToolbarHeight 44

@interface TimerCountdownTimePicker()

@property (nonatomic, assign, readwrite) UIDatePicker *picker;
@property (nonatomic, assign) CGRect originalFrame;

@property (nonatomic, assign) id doneTarget;
@property (nonatomic, assign) SEL doneSelector;

- (void) donePressed;

@end


@implementation TimerCountdownTimePicker

@synthesize picker = _picker;
@synthesize originalFrame = _originalFrame;

@synthesize doneTarget = _doneTarget;
@synthesize doneSelector = _doneSelector;

- (id) initWithFrame: (CGRect) frame {
    if ((self = [super initWithFrame: frame])) {
        self.originalFrame = frame;
        self.backgroundColor = [UIColor clearColor];
        
        CGFloat width = self.bounds.size.width;
        UIDatePicker *picker = [[UIDatePicker alloc] initWithFrame: CGRectMake(0, 0, width, TimerCountdownTimePickerPickerHeight)];
        [self addSubview: picker];
        
        UIToolbar *toolbar = [[UIToolbar alloc] initWithFrame: CGRectMake(0, TimerCountdownTimePickerPickerHeight, width, TimerCountdownTimePickerToolbarHeight)];
        toolbar.barStyle = UIBarStyleBlackOpaque;
        
        UIBarButtonItem *doneButton = [[UIBarButtonItem alloc] initWithTitle: NSLocalizedString(@"Done", @"Done button in the time picker for count down time picker") style: UIBarButtonItemStyleBordered target: self action: @selector(donePressed)];
        doneButton.width = width - 20;
        toolbar.items = [NSArray arrayWithObject: doneButton];
        [self addSubview: toolbar];
        
        self.picker = picker;
    }
    return self;
}

- (void) setMode: (UIDatePickerMode) mode {
    self.picker.datePickerMode = mode;
}

- (void) donePressed {
    if (self.doneTarget) {
        [self.doneTarget performSelector: self.doneSelector];
    }
}

- (void) addTargetForDoneButton: (id) target action: (SEL) action {
    self.doneTarget = target;
    self.doneSelector = action;
}

- (void) setHidden: (BOOL) hidden animated: (BOOL) animated {
    CGRect newFrame = self.originalFrame;
    newFrame.origin.y += hidden ? MyDateTimePickerHeight : 0;
    if (animated) {
        [UIView beginAnimations: @"animateDateTimePicker" context: nil];
        [UIView setAnimationDuration: 0.3];
        [UIView setAnimationCurve: UIViewAnimationCurveEaseOut];
        
        self.frame = newFrame;
        
        [UIView commitAnimations];
    } else {
        self.frame = newFrame;
    }
}

@end
