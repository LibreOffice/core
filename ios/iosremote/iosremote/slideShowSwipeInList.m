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
        [self stopWatchStart:nil];
    }
    self.clearsSelectionOnViewWillAppear = NO;
}

- (void) viewDidAppear:(BOOL)animated
{
    [self changeStartButtonIconForButton:nil];
    NSIndexPath *indexPath = [NSIndexPath indexPathForRow:self.slideshow.currentSlide
                                                inSection:1];
    [self.tableView selectRowAtIndexPath:indexPath animated:NO scrollPosition:UITableViewScrollPositionNone];
    [self.tableView scrollToRowAtIndexPath:indexPath atScrollPosition:UITableViewScrollPositionMiddle animated:NO];
//    [[[self.tableView cellForRowAtIndexPath:indexPath] viewWithTag:2] setBackgroundColor:[UIColor lightGrayColor]];
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

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Section one used for stopwatch
    if (section == 0)
        return 1;
    else
        return [self.slideshow size];
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    switch (section) {
        case 0:
            return @"Stop Watch";
            break;
        case 1:
            return @"Slides";
        default:
            break;
    }
    return nil;
}

- (void) tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath
{
    [self performSegueWithIdentifier: @"sw_customized_segue" sender: [tableView cellForRowAtIndexPath: indexPath]];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section == 0) {
        static NSString *CellIdentifier = @"stopWatch";

        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: CellIdentifier];
        [cell setSelectionStyle:UITableViewCellSelectionStyleNone];
        cell.contentView.backgroundColor = [UIColor whiteColor];
        return cell;
    } else {
        static NSString *CellIdentifier = @"slide";

        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: CellIdentifier];
        UILabel * slideNumber = (UILabel *)[cell viewWithTag:2];

        // Starting 20, all tags are used for thumbnails in this sidebar
        [cell setTag:20+indexPath.row];
        [self.slideshow getContentAtIndex:indexPath.row forView:cell];
        [slideNumber setText:[NSString stringWithFormat:@"%u", indexPath.row+1]];
        return cell;
    }
}

- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath{
    // Otherwise selection will disable background color and make slide number unreadable
    if(indexPath.section == 1 && cell.selected){
        UILabel *label = (UILabel *)[cell viewWithTag:2];
        if ([label backgroundColor]!=[UIColor lightGrayColor]) {
            [label setBackgroundColor:[UIColor lightGrayColor]];
        }
    }
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section == 0)
        return;
    [self.comManager.transmitter gotoSlide:indexPath.row];
    [[[self.tableView cellForRowAtIndexPath:indexPath] viewWithTag:2] setBackgroundColor:[UIColor lightGrayColor]];
    [self.revealViewController revealToggle: self];
}


- (void)viewDidUnload {
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
    UILabel *l = (UILabel *)[[self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:0]] viewWithTag:1];
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

    [self changeStartButtonIconForButton:sender];
}

- (void) changeStartButtonIconForButton:(UIButton *)sender
{
    UIButton * btn = sender;
    if (!btn) {
        btn = (UIButton *)[[self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:0]] viewWithTag:2];
    }
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

    UIButton *l = (UIButton *)[[self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:0]] viewWithTag:2];
    [l setImage:[UIImage imageNamed:@"timer_start_btn"] forState:UIControlStateNormal];
    [self updateTimer];
}
@end
