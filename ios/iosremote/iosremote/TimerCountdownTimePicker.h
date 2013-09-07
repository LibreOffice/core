//
//  TimerCountdownTimePicker.h
//  iosremote
//
//  Created by Siqi Liu on 9/7/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface TimerCountdownTimePicker : UIView

@property (nonatomic, assign, readonly) UIDatePicker *picker;

- (void) setMode: (UIDatePickerMode) mode;
- (void) setHidden: (BOOL) hidden animated: (BOOL) animated;
- (void) addTargetForDoneButton: (id) target action: (SEL) action;

@end
