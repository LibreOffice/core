// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "stopWatch.h"

@interface StopWatch ()

@property NSTimeInterval lastInterval;
@property int state;
@property (weak, nonatomic) UIButton * startButton;
@property (weak, nonatomic) UIButton * clearButton;
@property (weak, nonatomic) UILabel * timeLabel;

@end

@implementation StopWatch
@synthesize startButton = _startButton;
@synthesize clearButton = _clearButton;
@synthesize timeLabel = _timeLabel;

@synthesize lastInterval = _lastInterval;


- (StopWatch *) init
{
    self = [super init];
    self.state = TIMER_STATE_CLEARED;
    self.set = NO;
    
    return self;
}

- (StopWatch *) initWithStartButton:(UIButton *)startButton
                        ClearButton:(UIButton *)clearButton
                          TimeLabel:(UILabel *)timeLabel
{
    self = [self init];
    
    self.startButton = startButton;
    self.clearButton = clearButton;
    self.timeLabel = timeLabel;
    
    [self setupActions];
    return self;
}

- (void) setupWithTableViewCell:(UITableViewCell *)cell
{
    self.startButton = (UIButton *)[[cell viewWithTag:8] viewWithTag:2];
    self.clearButton = (UIButton *)[[cell viewWithTag:8] viewWithTag:3];
    self.timeLabel = (UILabel *)[[cell viewWithTag:8] viewWithTag:1];
    
    [self setupActions];
}

- (void) setupActions
{
    [self.startButton addTarget:self action:@selector(start) forControlEvents:UIControlEventTouchUpInside];
    [self.clearButton addTarget:self action:@selector(clear) forControlEvents:UIControlEventTouchUpInside];
    self.set = YES;
}

- (void)updateTimer
{
        // Create date from the elapsed time
        NSDate *currentDate = [NSDate date];
        NSTimeInterval timeInterval = [currentDate timeIntervalSinceDate:self.startDate] + self.lastInterval;
        NSDate *timerDate = [NSDate dateWithTimeIntervalSince1970:timeInterval];
        
        // Create a date formatter
        NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateFormat:@"HH:mm:ss"];
        [dateFormatter setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0.0]];
        
        // Format the elapsed time and set it to the label
        NSString *timeString = [dateFormatter stringFromDate:timerDate];
        self.timeLabel.text = timeString;
        [self.delegate setTitle:timeString sender:self];
}


- (void) start
{
    switch (self.state) {
        case TIMER_STATE_RUNNING:
            self.state = TIMER_STATE_PAUSED;
            [self.stopWatchTimer invalidate];
            [self.delegate setTitle:@"" sender:self];
            self.lastInterval += [[NSDate date] timeIntervalSinceDate:self.startDate];
            break;
        case TIMER_STATE_PAUSED:
            self.state = TIMER_STATE_RUNNING;
            self.startDate = [NSDate date];
            self.stopWatchTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/10.0
                                                                   target:self
                                                                 selector:@selector(updateTimer)
                                                                 userInfo:nil
                                                                  repeats:YES];
            [[NSRunLoop currentRunLoop] addTimer:self.stopWatchTimer forMode:NSRunLoopCommonModes];
            break;
        case TIMER_STATE_CLEARED:
            self.state = TIMER_STATE_RUNNING;
            self.startDate = [NSDate date];
            // Create the stop watch timer that fires every 100 ms
            self.stopWatchTimer = [NSTimer scheduledTimerWithTimeInterval:1.0/10.0
                                                                   target:self
                                                                 selector:@selector(updateTimer)
                                                                 userInfo:nil
                                                                  repeats:YES];
            [[NSRunLoop currentRunLoop] addTimer:self.stopWatchTimer forMode:NSRunLoopCommonModes];
            break;
        default:
            break;
    }
    
    [self updateStartButtonIcon];
}

- (void) updateStartButtonIcon
{
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        switch (self.state) {
            case TIMER_STATE_RUNNING:
                [self.startButton setImage:[UIImage imageNamed:@"timer_pause_btn_dark"] forState:UIControlStateNormal];
                [self.startButton setTitle:NSLocalizedString(@"Pause", @"Update button text in diffrent states") forState:UIControlStateNormal];
                break;
            case TIMER_STATE_PAUSED:
                [self.startButton setImage:[UIImage imageNamed:@"timer_start_btn_dark"] forState:UIControlStateNormal];
                [self.startButton setTitle:NSLocalizedString(@"Start", @"Update button text in diffrent states") forState:UIControlStateNormal];
                break;
            case TIMER_STATE_CLEARED:
                [self.startButton setImage:[UIImage imageNamed:@"timer_start_btn_dark"] forState:UIControlStateNormal];
                [self.startButton setTitle:NSLocalizedString(@"Start", @"Update button text in diffrent states") forState:UIControlStateNormal];
                break;
            default:
                break;
        }
    } else {
        switch (self.state) {
            case TIMER_STATE_RUNNING:
                [self.startButton setImage:[UIImage imageNamed:@"timer_pause_btn"] forState:UIControlStateNormal];
                break;
            case TIMER_STATE_PAUSED:
                [self.startButton setImage:[UIImage imageNamed:@"timer_start_btn"] forState:UIControlStateNormal];
                break;
            case TIMER_STATE_CLEARED:
                [self.startButton setImage:[UIImage imageNamed:@"timer_start_btn"] forState:UIControlStateNormal];
                break;
            default:
                break;
        }
    }
}

- (void) clear
{
    [self.stopWatchTimer invalidate];
    self.stopWatchTimer = nil;
    self.startDate = [NSDate date];
    self.lastInterval = 0;
    self.state = TIMER_STATE_CLEARED;
    
    [self updateStartButtonIcon];
    
    [self updateTimer];
    [self.delegate setTitle:@"" sender:self];
}

@end

