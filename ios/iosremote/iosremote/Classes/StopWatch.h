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

@protocol StopWatchDelegate <NSObject>

- (void) setTitle:(NSString *) title sender:(id)sender;

@end

/**
 This class controls the behavior of stopwatch timing widget.
 **/

@interface StopWatch : NSObject

// StopWatch
@property (strong, nonatomic) NSTimer *stopWatchTimer;
@property (strong, nonatomic) NSDate *startDate;
@property BOOL set;
@property (weak, nonatomic) id<StopWatchDelegate> delegate;

// initialzie Stopwatch with relative UIButton/UILabel elements
- (StopWatch *) initWithStartButton:(UIButton *)startButton
                        ClearButton:(UIButton *)clearButton
                          TimeLabel:(UILabel *)timeLabel;
// initialzie Stopwatch with a tableviewCell which contains relative UIButton/UILabel elements
- (void) setupWithTableViewCell:(UITableViewCell *)cell;

// Start/Pause/Resume the stopwatch
- (void) start;
// Stop the stopwatch and update seconds left to its original value
- (void) clear;
// Keep start button icon up-to-date with respect to its current state
- (void) updateStartButtonIcon;

@end
