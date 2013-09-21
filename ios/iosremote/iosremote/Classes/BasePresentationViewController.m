// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "BasePresentationViewController.h"
#import "SlideShow.h"
#import "CommunicationManager.h"
#import "CommandTransmitter.h"
#import "CommandInterpreter.h"
#import "HorizontalSlideCell.h"
#import "ServerListViewController.h"
#import "MainSplitViewController.h"
#import "stopWatch.h"
#import "Timer.h"
#import "PopoverView.h"
#import "UIView+Shadowing.h"
#import "LibONavigationController.h"
#import "AppDelegate.h"
#import "ControlVariables.h"

#import <QuartzCore/QuartzCore.h>
#import <CoreText/CoreText.h>
#import <CoreMotion/CoreMotion.h>

#define CURRENT_SLIDE_IMAGEVIEW 19
#define CURRENT_SLIDE_NOTES 18

@interface BasePresentationViewController () <PopoverViewDelegate, TimerDelegate, UIPopoverControllerDelegate>

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) id slideShowImageNoteReadyObserver;
@property (nonatomic, strong) id slideShowFinishedObserver;
@property (nonatomic, strong) id slideChangedObserver;

@property (nonatomic, strong) StopWatch * stopWatch;
@property (nonatomic, strong) Timer * timer;
@property (nonatomic, strong) UIPopoverController * popoverTimePickerController;

@end

@implementation BasePresentationViewController
@synthesize masterIsHiddenWhenLandscape = _masterIsHiddenWhenLandscape;

@synthesize comManager = _comManager;
@synthesize slideShowImageNoteReadyObserver = _slideShowImageNoteReadyObserver;
@synthesize slideShowFinishedObserver = _slideShowFinishedObserver;
@synthesize slideChangedObserver = _slideChangedObserver;
@synthesize popoverTimePickerController = _popoverTimePickerController;

#pragma mark - UITableView delegate
- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.comManager.interpreter.slideShow size];
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *cellIdentifier = @"HorizontalTableSlideCell";   
    
    HorizontalSlideCell *cell = [tableView dequeueReusableCellWithIdentifier:cellIdentifier];
    
    if (cell == nil)
    {
        cell = [[HorizontalSlideCell alloc] initWithFrame:CGRectMake(0, 0, kCellWidth, kCellHeight)];
    }
    
    cell.numberLabel.text =
        [NSString stringWithFormat:@"%u", indexPath.row+1];
    
    [cell setTag:-indexPath.row - 1];
    [cell.thumbnail setTag:1];
    
    [cell.thumbnail setShadow];
    
    [self.comManager.interpreter.slideShow getContentAtIndex:indexPath.row forView:cell];
    
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (self.horizontalTableView.isHidden)
        return;
    [[(HorizontalSlideCell *)[self.horizontalTableView cellForRowAtIndexPath:indexPath] numberLabel] setBackgroundColor:kHorizontalTableCellHighlightedBackgroundColor];
    [self.comManager.transmitter gotoSlide:indexPath.row];
}

#pragma mark -
#pragma iOS5 autorotate

- (BOOL) shouldAutorotate
{
    return YES;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

- (NSUInteger)supportedInterfaceOrientations {
    return UIInterfaceOrientationMaskAll;
}

#pragma mark -
#pragma SplitView hide master view tweak

// FIXME This is a hack, consider replace it when a more proper method is available
-(void)hideMaster:(BOOL)hideState
{
    NSLog(@"hideMasterNow");
    _masterIsHiddenWhenLandscape = hideState;
    
    [self.splitViewController.view setNeedsLayout];
    self.splitViewController.delegate = nil;
    self.splitViewController.delegate = self;
    
    [self.splitViewController willRotateToInterfaceOrientation:[UIApplication sharedApplication].statusBarOrientation duration:0];
}

- (BOOL) splitViewController:(UISplitViewController *)svc shouldHideViewController:(UIViewController *)vc inOrientation:(UIInterfaceOrientation)orientation
{
    if (UIInterfaceOrientationIsLandscape(orientation)){
        return self.masterIsHiddenWhenLandscape;
    }
    return UIInterfaceOrientationIsPortrait(orientation);
}

#pragma mark - Pointer
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    
    UITouch *touch = [[event allTouches] anyObject];
        CGPoint loc = [touch locationInView:self.currentSlideImageView];
        if (loc.x >= 0 && loc.x <= self.currentSlideImageView.frame.size.width
            && loc.y >= 0 && loc.y <= self.currentSlideImageView.frame.size.height){
            
            [UIView animateWithDuration:0.2 animations:^{
                [self.previousButton setAlpha:0];
                [self.nextButton setAlpha:0];
            }];
            
            CGPoint pointerInPercentage;
            pointerInPercentage.x = loc.x / self.currentSlideImageView.frame.size.width;
            pointerInPercentage.y = loc.y / self.currentSlideImageView.frame.size.height;
            [self.comManager.transmitter setPointerVisibleAt:pointerInPercentage];
            
            CGPoint p;
            p.x = loc.x + self.currentSlideImageView.frame.origin.x;
            p.y = loc.y + self.currentSlideImageView.frame.origin.y;
            self.movingPointer.center = p;
            [self.movingPointer setHidden:NO];
        }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    static int count = 0;
    if (count < 2){
        ++count;
        return;
    }
    count = 0;
    
    UITouch *touch = [[event allTouches] anyObject];
    CGPoint loc = [touch locationInView:self.currentSlideImageView];
    if (loc.x >= 0 && loc.x <= self.currentSlideImageView.frame.size.width
        && loc.y >= self.movingPointer.frame.size.height && loc.y <= self.currentSlideImageView.frame.size.height - self.movingPointer.frame.size.height)
    {
        [UIView animateWithDuration:0.2 animations:^{
            [self.previousButton setAlpha:0];
            [self.nextButton setAlpha:0];
        }];
        
        CGPoint pointerInPercentage;
        pointerInPercentage.x = loc.x / self.currentSlideImageView.frame.size.width;
        pointerInPercentage.y = loc.y / self.currentSlideImageView.frame.size.height;
        [self.comManager.transmitter pointerCoordination:pointerInPercentage];
        
        CGPoint p;
        p.x = loc.x + self.currentSlideImageView.frame.origin.x;
        p.y = loc.y + self.currentSlideImageView.frame.origin.y;
        self.movingPointer.center = p;
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self.movingPointer setHidden:YES];
    [self.comManager.transmitter setPointerDismissed];
    
    [UIView animateWithDuration:0.2 animations:^{
        [self.previousButton setAlpha:0.2];
        [self.nextButton setAlpha:0.2];
    }];
}

# pragma mark - welcome page

-(void)startConnectionModal:(id)sender
{
    UIStoryboard *mainStoryboard = [UIStoryboard storyboardWithName:@"iPad_autosize"
                                                             bundle: nil];
    ServerListViewController * slvc = [mainStoryboard instantiateViewControllerWithIdentifier:@"serverList"];
    self.initialNavController = slvc;
    slvc.modalPresentationStyle = UIModalPresentationFormSheet;
    slvc.modalTransitionStyle = UIModalTransitionStyleFlipHorizontal;
    [self presentViewController:slvc animated:YES completion:^{}];
}

# pragma mark - system defaults

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toOrientation
                                duration:(NSTimeInterval)duration
{
    if (toOrientation == UIInterfaceOrientationPortrait ||
        toOrientation == UIInterfaceOrientationPortraitUpsideDown){
        if (self.horizontalTableView.isHidden!=NO) {
            [self.horizontalTableView setHidden:NO];
        }
        [self.horizontalTableView reloadData];
    }
    else if (toOrientation == UIInterfaceOrientationLandscapeLeft ||
        toOrientation == UIInterfaceOrientationLandscapeRight)
        [self.horizontalTableView setHidden:YES];
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
    if (UIInterfaceOrientationIsLandscape(self.interfaceOrientation)) {
        [UIView animateWithDuration:0.25 animations:^{
            self.NotesView.frame = CGRectMake(-15.0,
                                              /*self.NotesView.frame.origin.y - self.horizontalTableView.frame.size.height*/ 466.0,
                                              self.NotesView.frame.size.width,
                                              /*self.NotesView.frame.size.height + self.horizontalTableView.frame.size.height*/ 176.0);
            self.horizontalTableView.bounds = CGRectMake(0, 0, 768, 0);
        }];
    } else {
        [UIView animateWithDuration:0.25 animations:^{
            self.NotesView.frame = CGRectMake(-13.0,
                                              /*self.NotesView.frame.origin.y - self.horizontalTableView.frame.size.height*/ 595.0,
                                              794.0,
                                              /*self.NotesView.frame.size.height + self.horizontalTableView.frame.size.height*/ 303.0);
            self.horizontalTableView.bounds = CGRectMake(0, 0, 129, 768);
        } completion:nil];
    }
    [self.popoverTimePickerController dismissPopoverAnimated:YES];
}

- (SlideShow *)slideshow
{
    return self.comManager.interpreter.slideShow;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.welcome_blocking_page = [[UIView alloc] initWithFrame:self.view.bounds];
    self.welcome_blocking_page.backgroundColor = [UIColor orangeColor];
    self.welcome_blocking_page.autoresizingMask = UIViewAutoresizingFlexibleHeight |
    UIViewAutoresizingFlexibleWidth;
    [self.view addSubview:self.welcome_blocking_page];
    
    self.masterIsHiddenWhenLandscape = YES;
    [self.currentSlideImageView setTag:CURRENT_SLIDE_IMAGEVIEW];
    [self.NoteWebView setTag:CURRENT_SLIDE_NOTES];
    self.slideshow.delegate = self;
    self.comManager = [CommunicationManager sharedComManager];
    
    if (UIInterfaceOrientationIsLandscape(self.interfaceOrientation)) {
        [self.horizontalTableView setHidden:YES];
    }
    self.comManager.interpreter.slideShow.delegate = self;    
    self.movingPointer.layer.cornerRadius = 3;
    
    self.horizontalTableView.showsVerticalScrollIndicator = NO;
    self.horizontalTableView.showsHorizontalScrollIndicator = NO;
    self.horizontalTableView.transform = CGAffineTransformMakeRotation(-M_PI * 0.5);
    [self.horizontalTableView setFrame:CGRectMake(0, 462, kTableLength - kRowHorizontalPadding, kCellHeight)];
    
    self.horizontalTableView.rowHeight = kCellWidth;
    
    self.horizontalTableView.backgroundColor = [UIColor colorWithRed:.674509804 green:.729411765 blue:.760784314 alpha:1.0];
    
    self.horizontalTableView.separatorStyle = UITableViewCellSeparatorStyleNone;
    
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    
    self.slideChangedObserver =[[NSNotificationCenter defaultCenter] addObserverForName:MSG_SLIDE_CHANGED
                                                   object:nil
                                                    queue:mainQueue
                                               usingBlock:^(NSNotification *note) {
                                                   if ([self.horizontalTableView numberOfRowsInSection:0] > 0){
                                                       NSIndexPath *indexPath = [NSIndexPath indexPathForRow:self.slideshow.currentSlide inSection:0];
                                                       [self.horizontalTableView selectRowAtIndexPath:indexPath animated:YES scrollPosition:UITableViewScrollPositionNone];
                                                       [[(HorizontalSlideCell *)[self.horizontalTableView cellForRowAtIndexPath:indexPath] numberLabel] setBackgroundColor:kHorizontalTableCellHighlightedBackgroundColor];
                                                       [self.horizontalTableView scrollToRowAtIndexPath:indexPath atScrollPosition:UITableViewScrollPositionMiddle animated:YES];
                                                   }
                                               }];
    self.stopWatch = [[StopWatch alloc] initWithStartButton:self.startButton ClearButton:self.clearButton TimeLabel:self.timeLabel];
    self.timer = [[Timer alloc] initWithStartButton:self.timerStartButton ClearButton:self.timerClearButton SetTimeButton:self.timerSetTimeButton TimeLabel:self.timerTimeLabel];
    [self.stopWatchView setShadowLight];
    [self.timerView setShadowLight];
    self.timer.delegate = self;
    self.stopWatchTimerScrollView.contentSize = CGSizeMake(1240, 62);
    [self.gearButton setShadowLight];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    
    [UIView animateWithDuration:0.2 delay:1 options:(UIViewAnimationOptionCurveEaseInOut|UIViewAnimationOptionAllowUserInteraction) animations:^{
        [self.previousButton setAlpha:0.2];
        [self.nextButton setAlpha:0.2];
    } completion:nil];
    
    [UIView animateWithDuration:0.4 delay:0 options:(UIViewAnimationOptionCurveEaseIn) animations:^{
        [self.welcome_blocking_page setAlpha:1];
    } completion:nil];

    if (UIInterfaceOrientationIsLandscape(self.interfaceOrientation)) {
        [UIView animateWithDuration:0.25 animations:^{
            self.NotesView.frame = CGRectMake(-15.0,
                                              /*self.NotesView.frame.origin.y - self.horizontalTableView.frame.size.height*/ 466.0,
                                              self.NotesView.frame.size.width,
                                              /*self.NotesView.frame.size.height + self.horizontalTableView.frame.size.height*/ 176.0);
        }];
    } else {
        [UIView animateWithDuration:0.25 animations:^{
            self.NotesView.frame = CGRectMake(-13.0,
                                              /*self.NotesView.frame.origin.y - self.horizontalTableView.frame.size.height*/ 595.0,
                                              794.0,
                                              /*self.NotesView.frame.size.height + self.horizontalTableView.frame.size.height*/ 303.0);
        }];
    }
    
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    
    self.slideShowImageNoteReadyObserver =[center addObserverForName:MSG_SLIDE_CHANGED
                                                              object:nil
                                                               queue:mainQueue
                                                          usingBlock:^(NSNotification *note) {
                                                              [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.currentSlideImageView];
                                                              [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.NoteWebView];
                                                              NSLog(@"slideShowImageNoteReadyObserver:%u/%u", [self.slideshow currentSlide]+1, [self.slideshow size]);
                                                              [self.slideNumber setText:[NSString stringWithFormat:@"%u/%u", [self.slideshow currentSlide]+1, [self.slideshow size]]];
                                                          }];
    
    self.slideShowFinishedObserver = [center addObserverForName:STATUS_CONNECTED_NOSLIDESHOW
                                                         object:nil
                                                          queue:mainQueue
                                                     usingBlock:^(NSNotification *note) {
                                                         [self.stopWatch clear];
                                                         [self hideMaster:YES];
                                                         if ([self.comManager.interpreter.slideShow size] == 0)
                                                             [self setWelcomePageVisible:YES];
                                                         if (!self.presentedViewController) {
                                                             // We fire up the retained initialNavController which will bring back the preview view controller page.
                                                             self.initialNavController.modalPresentationStyle = UIModalPresentationFormSheet;
                                                             self.initialNavController.modalTransitionStyle = UIModalTransitionStyleCoverVertical;
                                                             
                                                             [self presentViewController:self.initialNavController animated:YES completion:^{}];
                                                             [self.timer clear];
                                                             [self.stopWatch clear];
                                                         }
                                                     }];
    [self startConnectionModal:nil];
}

- (void) setWelcomePageVisible:(BOOL)visible
{
    if (!visible) {
        [self.currentSlideImageView setUserInteractionEnabled:NO];
        
        [UIView animateWithDuration:0.15 delay:0 options:UIViewAnimationOptionCurveLinear animations:^{
            self.welcome_blocking_page.alpha = 0.0;
        } completion:^(BOOL finished) {
            if (finished) {
                self.welcome_blocking_page.hidden = YES;
                [self.currentSlideImageView setUserInteractionEnabled:YES];
            } else {
                [self setWelcomePageVisible:visible];
                [self.currentSlideImageView setUserInteractionEnabled:YES];
            }
            if (kStopwatchAutoStart)
                [self.stopWatch start];
            if (kCountDownTimerAutoStart)
                [self.timer start];
            CGRect frame = self.stopWatchTimerScrollView.frame;
            frame.origin.x = frame.size.width * kDefaultTimerWidget;
            frame.origin.y = 0;
            [self.stopWatchTimerScrollView scrollRectToVisible:frame animated:YES];
            [self.timer setSecondsLeft:kCountDownTimerDefaultDuration * 60];
            
        }];
        self.slideshow.delegate = self;
    }
    else {
        [self.currentSlideImageView setUserInteractionEnabled:NO];
        self.welcome_blocking_page.hidden = NO;
        
        [UIView animateWithDuration:0.15 delay:0 options:UIViewAnimationOptionCurveLinear animations:^{
            self.welcome_blocking_page.alpha = 1.0;
        } completion:nil];
    }
}

- (void) viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
    [self.popoverTimePickerController dismissPopoverAnimated:YES];
}


- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidUnload {
    [self.popoverTimePickerController dismissPopoverAnimated:YES];
    [self setHorizontalTableView:nil];
    [self setNotesView:nil];
    [self setNoteWebView:nil];
    [self setMovingPointer:nil];
    [self setCurrentSlideImageView:nil];
    [self setPreviousButton:nil];
    [self setNextButton:nil];
    [self setSlideNumber:nil];
    [self setSlideNumber:nil];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowFinishedObserver];
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowImageNoteReadyObserver];
    
    [self setStartButton:nil];
    [self setClearButton:nil];
    [self setTimeLabel:nil];
    [self setGearButton:nil];
    [self setStopWatchView:nil];
    [self setTimerView:nil];
    [self setStopWatchTimerScrollView:nil];
    [super viewDidUnload];
}


- (IBAction)nextSlideAction:(id)sender {
    
    [[self.comManager transmitter] nextTransition];
    
    [UIView animateWithDuration:0.4 delay:0 options:(UIViewAnimationOptionCurveEaseInOut|UIViewAnimationOptionAllowUserInteraction) animations:^{
        [self.previousButton setAlpha:0.5];
        [self.nextButton setAlpha:0.5];
    } completion:^(BOOL finished) {
        [UIView animateWithDuration:0.4 delay:1 options:(UIViewAnimationOptionCurveEaseInOut|UIViewAnimationOptionAllowUserInteraction) animations:^{
            [self.previousButton setAlpha:0.2];
            [self.nextButton setAlpha:0.2];
        } completion:nil];
    }];
}

- (IBAction)previousSlideAction:(id)sender {
    
    [[self.comManager transmitter] previousTransition];
    
    [UIView animateWithDuration:0.4 delay:0 options:(UIViewAnimationOptionCurveEaseInOut|UIViewAnimationOptionAllowUserInteraction) animations:^{
        [self.previousButton setAlpha:0.5];
        [self.nextButton setAlpha:0.5];
    } completion:^(BOOL finished) {
        [UIView animateWithDuration:0.4 delay:1 options:(UIViewAnimationOptionCurveEaseInOut|UIViewAnimationOptionAllowUserInteraction) animations:^{
            [self.previousButton setAlpha:0.2];
            [self.nextButton setAlpha:0.2];
        } completion:nil];
    }];
}

static BOOL isBlank = NO;
#pragma mark - Popover toggle
- (IBAction)popOverUp:(id)sender {
    if (!isBlank) {
        [PopoverView showPopoverAtPoint:self.gearButton.frame.origin inView:self.gearButton.superview withStringArray:
         [NSArray arrayWithObjects:NSLocalizedString(@"Stop Presentation", @"Popover option"), NSLocalizedString(@"Restart", @"Popover option"), NSLocalizedString(@"Blank Screen", @"Popover option"), nil]
                               delegate:self];
    } else {
        [PopoverView showPopoverAtPoint:self.gearButton.frame.origin inView:self.gearButton.superview withStringArray:
         [NSArray arrayWithObjects:NSLocalizedString(@"Stop Presentation", @"Popover option"), NSLocalizedString(@"Restart", @"Popover option"), NSLocalizedString(@"Resume from blank Screen", @"Popover option"), nil]
                               delegate:self];
    }
    [UIView animateWithDuration:0.24 animations:^{
        [self.gearButton setCenter:CGPointMake(self.gearButton.center.x - 20, self.gearButton.center.y)];
    }];
}

- (void)popoverView:(PopoverView *)popoverView didSelectItemAtIndex:(NSInteger)index
{
    [popoverView dismiss];
    switch (index) {
        case 0:
            [self.comManager.transmitter stopPresentation];
            break;
        case 1:
            [self.comManager.transmitter gotoSlide:0];
            break;
        case 2:
            if (!isBlank){
                [self.comManager.transmitter blankScreen];
            } else {
                [self.comManager.transmitter resume];
            }
            isBlank = !isBlank;
            break;
        default:
            break;
    }
}

- (void) popoverViewDidDismiss:(PopoverView *)popoverView
{
    [UIView animateWithDuration:0.24 animations:^{
        [self.gearButton setCenter:CGPointMake(self.gearButton.center.x + 20, self.gearButton.center.y)];
    }];
}

#pragma mark - Reconnection
- (void) didReceiveDisconnection
{
    //Dismiss time picker in case that users didn't have the time to tap elsewhere to dismiss it.
    [self.popoverTimePickerController dismissPopoverAnimated:YES];
    
    [self dismissViewControllerAnimated:YES completion:^{
        [self startConnectionModal:nil];
    }];
    
    if (self.welcome_blocking_page.isHidden == YES) {
        [self setWelcomePageVisible:YES];
    }
}

#pragma mark TimerDelegate

- (void)changeDate:(UIDatePicker *)sender {
    [self.timer setSecondsLeft: sender.countDownDuration];
}

- (void)startTimePickerwithTimer:(Timer *) timer
{
    UIViewController* popoverContent = [[UIViewController alloc] init]; //ViewController
    
    UIView *popoverView = [[UIView alloc] init];   //view
    popoverView.backgroundColor = [UIColor blackColor];
    
    UIDatePicker *datePicker=[[UIDatePicker alloc]init];//Date picker
    datePicker.frame=CGRectMake(0,44,320, 216);
    datePicker.datePickerMode = UIDatePickerModeCountDownTimer;
    [datePicker setTag:10];
    [datePicker addTarget:self action:@selector(changeDate:) forControlEvents:UIControlEventValueChanged];
    [popoverView addSubview:datePicker];
    
    popoverContent.view = popoverView;
    self.popoverTimePickerController = [[UIPopoverController alloc] initWithContentViewController:popoverContent];
    self.popoverTimePickerController.delegate = self;
    
    [self.popoverTimePickerController setPopoverContentSize:CGSizeMake(320, 264) animated:NO];
    [self.popoverTimePickerController presentPopoverFromRect:self.timerSetTimeButton.frame inView:self.timerView permittedArrowDirections:UIPopoverArrowDirectionDown animated:YES];
}

@end
