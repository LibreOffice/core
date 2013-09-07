// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <Foundation/Foundation.h>

#define TIMER_STATE_RUNNING 0
#define TIMER_STATE_PAUSED 1
#define TIMER_STATE_CLEARED 2

@class Timer;

@protocol TimerDelegate <NSObject>

- (void) setTitle:(NSString *) title sender:(id)sender;
- (void) startTimePickerwithTimer:(Timer *) timer;

@end

@interface Timer : NSObject

// Timer
@property (strong, nonatomic) NSTimer *timerTimer;
@property BOOL set;
@property (weak, nonatomic) id<TimerDelegate> delegate;

- (Timer *) initWithStartButton:(UIButton *)startButton
                        ClearButton:(UIButton *)clearButton
                      SetTimeButton:(UIButton *)setTimeButton
                          TimeLabel:(UILabel *)timeLabel;
- (void) setupWithTableViewCell:(UITableViewCell *)cell;

- (void) start;
- (void) clear;
- (void) updateStartButtonIcon;

- (void) setSecondsLeft:(NSTimeInterval)duration;

@end

