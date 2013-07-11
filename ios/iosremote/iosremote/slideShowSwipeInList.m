//
//  slideShowSwipeInList.m
//  iosremote
//
//  Created by Liu Siqi on 7/8/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowSwipeInList.h"
#import "CommunicationManager.h"
#import "CommandInterpreter.h"
#import "SlideShow.h"
#import "CommandTransmitter.h"
#import "SWRevealViewController.h"
#import "slideShowPreviewTable_vc.h"
#import <QuartzCore/CALayer.h>

#define TIMER_STATE_RUNNING 0
#define TIMER_STATE_PAUSED 1
#define TIMER_STATE_CLEARED 2

@interface slideShowSwipeInList ()

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) SlideShow *slideshow;
@property NSTimeInterval lastInterval;
@property int state;
@end

@implementation slideShowSwipeInList

@synthesize comManager = _comManager;
@synthesize slideshow = _slideshow;
@synthesize lastInterval = _lastInterval;

dispatch_queue_t backgroundQueue;

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.comManager = [CommunicationManager sharedComManager];
    self.slideshow = self.comManager.interpreter.slideShow;
    self.slideshow.secondaryDelegate = self;
    self.state = TIMER_STATE_CLEARED;

    if ([[NSUserDefaults standardUserDefaults] boolForKey:KEY_TIMER]) {
        UIButton *l = (UIButton *)[[self.slidesTable cellForRowAtIndexPath:[NSIndexPath indexPathForItem:0 inSection:0]] viewWithTag:2];
        [self stopWatchStart:l];
    }
}

- (void) prepareForSegue: (UIStoryboardSegue *) segue sender: (id) sender
{
    // configure the segue.
    // in this case we dont swap out the front view controller, which is a UINavigationController.
    // but we could..
    if ( [segue isKindOfClass: [SWRevealViewControllerSegue class]] )
    {
        SWRevealViewControllerSegue* rvcs = (SWRevealViewControllerSegue*) segue;

        SWRevealViewController* rvc = self.revealViewController;
        NSAssert( rvc != nil, @"oops! must have a revealViewController" );

        NSAssert( [rvc.frontViewController isKindOfClass: [UINavigationController class]], @"oops!  for this segue we want a permanent navigation controller in the front!" );

        rvcs.performBlock = ^(SWRevealViewControllerSegue* rvc_segue, UIViewController* svc, UIViewController* dvc) {

            UINavigationController* nc = (UINavigationController*)rvc.frontViewController;
            [nc setViewControllers: @[ dvc ] animated: YES ];

            [rvc setFrontViewPosition: FrontViewPositionLeft animated: YES];
        };
    }
}


#pragma mark - Table view data source

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // +1 for stopwatch
    return [self.slideshow size]+1;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.row == 0) {
        static NSString *CellIdentifier = @"stopWatch";

        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: CellIdentifier];
        [cell setSelectionStyle:UITableViewCellSelectionStyleNone];
        cell.contentView.backgroundColor = [UIColor whiteColor];
        return cell;
    } else {
        static NSString *CellIdentifier = @"slide";

        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: CellIdentifier];
        UILabel * slideNumber = (UILabel *)[cell viewWithTag:2];

        [cell setTag:20+indexPath.row];
        [self.slideshow getContentAtIndex:indexPath.row-1 forView:cell];
        [slideNumber setText:[NSString stringWithFormat:@"%u", indexPath.row]];
        return cell;
    }
}


- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.row == 0)
        return;
    [self.comManager.transmitter gotoSlide:indexPath.row - 1];
    [self.revealViewController revealToggle: self];
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}


- (void)viewDidUnload {
    [self setSlidesTable:nil];
    [super viewDidUnload];
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
    UILabel *l = (UILabel *)[[self.slidesTable cellForRowAtIndexPath:[NSIndexPath indexPathForItem:0 inSection:0]] viewWithTag:1];
    l.text = timeString;
}


- (IBAction)stopWatchStart:(id)sender {
    switch (self.state) {
        case TIMER_STATE_RUNNING:
            self.state = TIMER_STATE_PAUSED;
            [self.stopWatchTimer invalidate];
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
            break;
        default:
            break;
    }

    UIButton * btn = (UIButton *)sender;

    switch (self.state) {
        case TIMER_STATE_RUNNING:
            [btn setImage:[UIImage imageNamed:@"timer_pause_btn"] forState:UIControlStateNormal];
            break;
        case TIMER_STATE_PAUSED:
            [btn setImage:[UIImage imageNamed:@"timer_resume_btn"] forState:UIControlStateNormal];
            break;
        case TIMER_STATE_CLEARED:
            [btn setImage:[UIImage imageNamed:@"timer_start_btn"] forState:UIControlStateNormal];
            break;
        default:
            break;
    }
}

- (IBAction)stopWatchClear:(id)sender {
    [self.stopWatchTimer invalidate];
    self.stopWatchTimer = nil;
    self.startDate = [NSDate date];
    self.lastInterval = 0;
    self.state = TIMER_STATE_CLEARED;

    UIButton *l = (UIButton *)[[self.slidesTable cellForRowAtIndexPath:[NSIndexPath indexPathForItem:0 inSection:0]] viewWithTag:2];
    [l setImage:[UIImage imageNamed:@"timer_start_btn"] forState:UIControlStateNormal];
    [self updateTimer];
}
@end
