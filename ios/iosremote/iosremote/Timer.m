// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "Timer.h"
#import <AudioToolbox/AudioServices.h>

@interface Timer ()

@property int state;
@property (weak, nonatomic) UIButton * startButton;
@property (weak, nonatomic) UIButton * clearButton;
@property (weak, nonatomic) UIButton * setTimeButton;
@property (weak, nonatomic) UILabel * timeLabel;

@end

@implementation Timer

@synthesize startButton = _startButton;
@synthesize clearButton = _clearButton;
@synthesize timeLabel = _timeLabel;
@synthesize setTimeButton = _setTimeButton;

int hours, minutes, seconds;
int secondsLeft;
int initSecondsLeft;
int vibrationCount;

- (Timer *) init
{
    self = [super init];
    self.state = TIMER_STATE_CLEARED;
    self.set = NO;
    secondsLeft = kCountDownTimerDefaultDuration * 60;
    
    return self;
}

- (Timer *) initWithStartButton:(UIButton *)startButton
                        ClearButton:(UIButton *)clearButton
                      SetTimeButton:(UIButton *)setTimeButton
                          TimeLabel:(UILabel *)timeLabel
{
    self = [self init];
    
    self.startButton = startButton;
    self.clearButton = clearButton;
    self.setTimeButton = setTimeButton;
    self.timeLabel = timeLabel;
    
    [self setupActions];
    return self;
}

- (void) setupWithTableViewCell:(UITableViewCell *)cell
{
    self.startButton = (UIButton *)[[cell viewWithTag:9] viewWithTag:2];
    self.clearButton = (UIButton *)[[cell viewWithTag:9] viewWithTag:3];
    self.setTimeButton = (UIButton *)[[cell viewWithTag:9] viewWithTag:4];
    self.timeLabel = (UILabel *)[[cell viewWithTag:9] viewWithTag:1];
    
    [self setupActions];
}

- (void) setupActions
{
    [self.startButton addTarget:self action:@selector(start) forControlEvents:UIControlEventTouchUpInside];
    [self.clearButton addTarget:self action:@selector(clear) forControlEvents:UIControlEventTouchUpInside];
    // Sending the sender as well, so that we get a handle on the Timer itself ---> allow us to update seconds left
    [self.setTimeButton addTarget:self action:@selector(startTimePickerwithTimer) forControlEvents:UIControlEventTouchUpInside];
    self.set = YES;
}

- (void)startTimePickerwithTimer
{
    if (self.state == TIMER_STATE_RUNNING) {
        // If running, we switch it to pause before setting a new duration
        [self start];
    }
    [self.delegate startTimePickerwithTimer:self];
}

- (void)updateTimer
{
    // Create date from the elapsed time
    if (secondsLeft > 0) {
        secondsLeft--;
        hours = secondsLeft / 3600;
        minutes = (secondsLeft % 3600) / 60;
        seconds = (secondsLeft %3600) % 60;
        self.timeLabel.text = [NSString stringWithFormat:@"%02d:%02d:%02d", hours, minutes, seconds];
        [self.delegate setTitle:[NSString stringWithFormat:@"%02d:%02d:%02d", hours, minutes, seconds] sender:self];
    } else {
        // flash timer label in red
        static BOOL disappear = NO;
        if (!disappear) {
            disappear = YES;
            self.timeLabel.text = @"";
            [self.delegate setTitle:@"" sender:self];
        } else {
            disappear = NO;
            self.timeLabel.text = @"00:00:00";
            [self.delegate setTitle:@"00:00:00" sender:self];
            // Vibrate the phone if supported (i.e. this works on iPhone but not iPad)
            // On ipad this just get ignored
            if (kCountDownTimerVibration && vibrationCount < 3){
                vibrationCount++;
                NSLog(@"Vibrating...");
                AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
            }
        }
    }
}


- (void) start
{
    // Reset vibration count to 0 on each start so that we vibrate 3 times maxi.
    vibrationCount = 0;
    switch (self.state) {
        case TIMER_STATE_RUNNING:
            self.state = TIMER_STATE_PAUSED;
            [self.timerTimer invalidate];
            if (secondsLeft == 0) {
                [self.timeLabel setText:@"00:00:00"];
            }
            [self.delegate setTitle:@"" sender:self];
            break;
        case TIMER_STATE_PAUSED:
            self.state = TIMER_STATE_RUNNING;
            self.timerTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                                   target:self
                                                                 selector:@selector(updateTimer)
                                                                 userInfo:nil
                                                                  repeats:YES];
            [[NSRunLoop currentRunLoop] addTimer:self.timerTimer forMode:NSRunLoopCommonModes];
            break;
        case TIMER_STATE_CLEARED:
            self.state = TIMER_STATE_RUNNING;
            secondsLeft++;
            [self updateTimer];
            // Create the stop watch timer that fires every 100 ms
            [self.timerTimer invalidate];
            self.timerTimer = nil;
            self.timerTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                                   target:self
                                                                 selector:@selector(updateTimer)
                                                                 userInfo:nil
                                                                  repeats:YES];
            [[NSRunLoop currentRunLoop] addTimer:self.timerTimer forMode:NSRunLoopCommonModes];
            initSecondsLeft = secondsLeft;
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
    [self.timerTimer invalidate];
    self.timerTimer = nil;
    self.state = TIMER_STATE_CLEARED;
    
    [self updateStartButtonIcon];
    [self.delegate setTitle:@"" sender:self];
    
    secondsLeft = initSecondsLeft;
    
    hours = secondsLeft / 3600;
    minutes = (secondsLeft % 3600) / 60;
    seconds = (secondsLeft %3600) % 60;
    self.timeLabel.text = [NSString stringWithFormat:@"%02d:%02d:%02d", hours, minutes, seconds];
}

- (void) setSecondsLeft:(NSTimeInterval)duration
{
    secondsLeft = (int) duration;
    initSecondsLeft = secondsLeft;
    hours = secondsLeft / 3600;
    minutes = (secondsLeft % 3600) / 60;
    seconds = (secondsLeft %3600) % 60;
    self.timeLabel.text = [NSString stringWithFormat:@"%02d:%02d:%02d", hours, minutes, seconds];
}

@end

