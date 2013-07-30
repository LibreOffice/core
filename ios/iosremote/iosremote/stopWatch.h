//
//  stopWatch.h
//  iosremote
//
//  Created by Siqi Liu on 7/29/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <Foundation/Foundation.h>

#define TIMER_STATE_RUNNING 0
#define TIMER_STATE_PAUSED 1
#define TIMER_STATE_CLEARED 2

@interface stopWatch : NSObject

// StopWatch
@property (strong, nonatomic) NSTimer *stopWatchTimer;
@property (strong, nonatomic) NSDate *startDate;
@property BOOL set;

- (stopWatch *) initWithStartButton:(UIButton *)startButton
                        ClearButton:(UIButton *)clearButton
                          TimeLabel:(UILabel *)timeLabel;
- (void) setupWithTableViewCell:(UITableViewCell *)cell;

- (void) start;
- (void) clear;
- (void) updateStartButtonIcon;

@end
