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

@interface stopWatch : NSObject

// StopWatch
@property (strong, nonatomic) NSTimer *stopWatchTimer;
@property (strong, nonatomic) NSDate *startDate;
@property BOOL set;
@property (weak, nonatomic) UINavigationItem * barItem;

- (stopWatch *) initWithStartButton:(UIButton *)startButton
                        ClearButton:(UIButton *)clearButton
                          TimeLabel:(UILabel *)timeLabel;
- (void) setupWithTableViewCell:(UITableViewCell *)cell;

- (void) start;
- (void) clear;
- (void) updateStartButtonIcon;

@end
