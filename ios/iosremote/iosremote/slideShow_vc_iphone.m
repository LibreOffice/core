/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#import "slideShow_vc_iphone.h"
#import "PopoverView.h"
#import "SlideShow.h"
#import "UIViewTransitionCategory.h"
#import "CommunicationManager.h"
#import "SWRevealViewController.h"
#import "CommandInterpreter.h"
#import "stopWatch.h"
#import "CommandTransmitter.h"
#import "slideShowPreviewTable_vc.h"
#import "slideShowSwipeInList_iphone.h"
#import <QuartzCore/QuartzCore.h>
#import <CoreMotion/CoreMotion.h>


#define CURRENT_SLIDE_IMAGEVIEW 1
#define NEXT_SLIDE_IMAGEVIEW 2
#define TOUCH_POINTER_VIEW 3
#define CURRENT_SLIDE_NOTES 4

@interface slideShow_vc () <PopoverViewDelegate>

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) id slideShowImageNoteReadyObserver;
@property (nonatomic, strong) id slideShowFinishedObserver;
@property (nonatomic, strong) SlideShow* slideshow;

@property BOOL pointerCalibrationOn;
@property CGPoint refLeftUpperGravity;
@property CGPoint refRightUpperGravity;
@property CGPoint refRightLowerGravity;

// SWReveal Controller
@property (readwrite) IBOutlet UIBarButtonItem* revealButtonItem;

@property int count;

@end

@implementation slideShow_vc

@synthesize comManager = _comManager;
@synthesize slideShowImageNoteReadyObserver = _slideShowImageNoteReadyObserver;
@synthesize slideShowFinishedObserver = _slideShowFinishedObserver;
@synthesize slideshow = _slideshow;

#pragma mark - Pointer

- (CMMotionManager *)motionManager
{
    CMMotionManager *motionManager = nil;

    id appDelegate = [UIApplication sharedApplication].delegate;

    if ([appDelegate respondsToSelector:@selector(motionManager)]) {
        motionManager = [appDelegate motionManager];
    }

    return motionManager;
}

- (void)startMotionDetect
{
    //    __block float stepMoveFactorX = 5;
    //    __block float stepMoveFactorY = 5;
    //    __block double refX, refY;

    [self.motionManager
     startAccelerometerUpdatesToQueue:[[NSOperationQueue alloc] init]
     withHandler:^(CMAccelerometerData *data, NSError *error)
     {

         dispatch_async(dispatch_get_main_queue(),
                        ^{
                            CGRect rect = self.movingPointer.frame;
                            NSLog(@"x:%f y:%f z:%f", data.acceleration.x, data.acceleration.y, data.acceleration.z);
                            // Used to calibrate pointer based on initial position
                            //                            if (self.pointerCalibrationOn){
                            //                                refX = data.acceleration.x;
                            //                                refY = data.acceleration.y;
                            //                                self.pointerCalibrationOn = NO;
                            //                            }
                            //                                float movetoX = rect.origin.x + ((data.acceleration.x - refX) * stepMoveFactorX);

                            float movetoX = self.touchPointerImage.frame.origin.x + self.touchPointerImage.frame.size.width * ABS(data.acceleration.x - self.refLeftUpperGravity.x) / ABS(self.refRightUpperGravity.x - self.refLeftUpperGravity.x);
                            float maxX = self.touchPointerImage.frame.origin.x + self.touchPointerImage.frame.size.width - rect.size.width;

                            //                                float movetoY = (rect.origin.y) + ((data.acceleration.y - refY) * stepMoveFactorY);
                            float movetoY = self.touchPointerImage.frame.origin.y + self.touchPointerImage.frame.size.height * ABS(data.acceleration.y - self.refRightUpperGravity.y) / ABS(self.refRightLowerGravity.y - self.refRightUpperGravity.y);
                            float maxY = self.touchPointerImage.frame.origin.y + self.touchPointerImage.frame.size.height;

                            if ( movetoX > self.touchPointerImage.frame.origin.x && movetoX < maxX ) {
                                rect.origin.x = movetoX;
                            };

                            if ( movetoY > self.touchPointerImage.frame.origin.y && movetoY < maxY ) {
                                rect.origin.y = movetoY;
                            };

                            [UIView animateWithDuration:0 delay:0
                                                options:UIViewAnimationOptionCurveEaseIn
                                             animations:^{
                                                 self.movingPointer.frame = rect;
                                             }
                                             completion:nil
                             ];
                        });
     }];
}

- (IBAction)accPointerAction:(id)sender{
    BOOL acc = [[NSUserDefaults standardUserDefaults] boolForKey:KEY_POINTER];
    if (!acc) {
        static BOOL pointer = NO;
        if (!pointer){
            [self startMotionDetect];
            [self.movingPointer setHidden:NO];
        }
        else {
            [self.motionManager stopAccelerometerUpdates];
            self.pointerCalibrationOn = NO;
            [self.movingPointer setHidden:YES];
        }
        pointer = !pointer;
    }
}

// Not localized for now since this is subject to fundemental changes
- (IBAction)pointerAction:(id)sender {
    if (self.count == 0 || self.count == 1){
        CGPoint p;
        p.x = [self.motionManager accelerometerData].acceleration.x;
        p.y = [self.motionManager accelerometerData].acceleration.y;
        self.refLeftUpperGravity = p;
        if (self.count == 1) {
            UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"Calibration"
                                                              message:@"Upper left corner calibrated, now point your device to the upper right corner of the screen and click Pointer button again"
                                                             delegate:nil
                                                    cancelButtonTitle:@"OK"
                                                    otherButtonTitles:@"Help", nil];
            [message show];
        }
        ++self.count;
    } else if (self.count == 2 || self.count == 3) {
        CGPoint p;
        p.x = [self.motionManager accelerometerData].acceleration.x;
        p.y = [self.motionManager accelerometerData].acceleration.y;
        self.refRightUpperGravity = p;
        if (self.count == 3) {
            UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"Calibration"
                                                              message:@"Upper right corner calibrated, now point your device to the lower right corner of the screen and click Pointer button again!"
                                                             delegate:nil
                                                    cancelButtonTitle:@"OK"
                                                    otherButtonTitles:@"Help", nil];
            [message show];
        }
        ++self.count;
    } else if (self.count == 4 || self.count == 5) {
        CGPoint p;
        p.x = [self.motionManager accelerometerData].acceleration.x;
        p.y = [self.motionManager accelerometerData].acceleration.y;
        self.refRightLowerGravity = p;
        if (self.count == 5) {
            UIAlertView *message = [[UIAlertView alloc] initWithTitle:@"Calibration"
                                                              message:@"Lower right corner calibrated, enjoy your pointer!"
                                                             delegate:nil
                                                    cancelButtonTitle:@"OK"
                                                    otherButtonTitles:@"Help", nil];
            [message show];
        }
        ++self.count;
    } else {
        if ([self.touchPointerImage isHidden]){
            [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.touchPointerImage];
            CGPoint p = self.view.center;
            p.y -= 50;
            self.touchPointerImage.center = p;
        }
        [self.touchPointerImage fadeInfadeOutwithDuration:0.0 maxAlpha:1.0];
        [self.blockingView fadeInfadeOutwithDuration:0.0 maxAlpha:0.7];
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[event allTouches] anyObject];
    if (!self.touchPointerImage.isHidden){
        CGPoint loc = [touch locationInView:self.touchPointerImage];
        if (loc.x >= 0 && loc.x <= self.touchPointerImage.frame.size.width
            && loc.y >= 0 && loc.y <= self.touchPointerImage.frame.size.height){
            CGPoint pointerInPercentage;
            pointerInPercentage.x = loc.x / self.touchPointerImage.frame.size.width;
            pointerInPercentage.y = loc.y / self.touchPointerImage.frame.size.height;
            [self.comManager.transmitter setPointerVisibleAt:pointerInPercentage];
            
            CGPoint p;
            p.x = loc.x + self.touchPointerImage.frame.origin.x;
            p.y = loc.y + self.touchPointerImage.frame.origin.y;
            self.movingPointer.center = p;
            [self.movingPointer setHidden:NO];
        }
    }
    
    if ([touch view] == self.secondarySlideView)
    {
        // Change to the next slide when secondary slide is clicked
        [self.comManager.transmitter gotoSlide:self.slideshow.currentSlide + 1];
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    static int count = 0;
    if (count < 1){
        ++count;
        return;
    }
    count = 0;
    if (!self.touchPointerImage.isHidden){
        UITouch *touch = [[event allTouches] anyObject];
        CGPoint loc = [touch locationInView:self.touchPointerImage];
        if (loc.x >= 0 && loc.x <= self.touchPointerImage.frame.size.width
            && loc.y >= self.movingPointer.frame.size.height && loc.y <= self.touchPointerImage.frame.size.height - self.movingPointer.frame.size.height)
        {
            CGPoint pointerInPercentage;
            pointerInPercentage.x = loc.x / self.touchPointerImage.frame.size.width;
            pointerInPercentage.y = loc.y / self.touchPointerImage.frame.size.height;
            [self.comManager.transmitter pointerCoordination:pointerInPercentage];
            
            CGPoint p;
            p.x = loc.x + self.touchPointerImage.frame.origin.x;
            p.y = loc.y + self.touchPointerImage.frame.origin.y;
            self.movingPointer.center = p;
        }
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self.movingPointer setHidden:YES];
    [self.comManager.transmitter setPointerDismissed];
}

#pragma mark - System defaults

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return (interfaceOrientation == UIInterfaceOrientationPortrait);
}

- (NSUInteger) supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskPortrait;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (SlideShow *)slideshow
{
    return self.comManager.interpreter.slideShow;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // Unique tag assignment. Don't use 0 as it's default. 0-10 for central VC
    [self.slideView setTag:CURRENT_SLIDE_IMAGEVIEW];
    [self.secondarySlideView setTag:NEXT_SLIDE_IMAGEVIEW];
    [self.lecturer_notes setTag:CURRENT_SLIDE_NOTES];
    [self.touchPointerImage setTag:TOUCH_POINTER_VIEW];

    self.comManager = [CommunicationManager sharedComManager];
    self.slideshow.delegate = self;

    [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.slideView];
    [self.slideshow getContentAtIndex:self.slideshow.currentSlide+1 forView:self.secondarySlideView];
    [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.lecturer_notes];
    [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.touchPointerImage];
    [self.slideNumber setText:[NSString stringWithFormat:@"%u/%u", [self.slideshow currentSlide]+1, [self.slideshow size]]];

    UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithImage:[UIImage imageNamed:@"gear_transparent_bg"]
                                                                   style:UIBarButtonItemStyleBordered
                                                                  target:self
                                                                  action:@selector(popOverStart:)];
    
    self.revealViewController.navigationItem.rightBarButtonItem = backButton;

    self.revealButtonItem = [[UIBarButtonItem alloc] initWithImage: [UIImage imageNamed:@"more_icon"]
                                                             style:UIBarButtonItemStyleBordered
                                                            target:self.revealViewController
                                                            action:@selector( revealToggle: )];
    self.revealViewController.navigationItem.leftBarButtonItem = self.revealButtonItem;
    [self.navigationController.navigationBar addGestureRecognizer: self.revealViewController.panGestureRecognizer];

    self.pointerCalibrationOn = NO;
    self.movingPointer.layer.cornerRadius = 3;

    BOOL acc = [[NSUserDefaults standardUserDefaults] boolForKey:KEY_POINTER];
    if (!acc) {
        // Hook up acc detection
        [self.pointerBtn addTarget:self action:@selector(pointerAction:) forControlEvents:UIControlEventTouchUpOutside];
        [self.pointerBtn addTarget:self action:@selector(pointerAction:) forControlEvents:UIControlEventTouchUpInside];
    } else {
        // Disable all calibration functions for acc based pointer
        self.count = INT_MAX;
    }
}

- (void) popOverStart:(id)sender
{
    if (!isBlank) {
        [PopoverView showPopoverAtPoint: CGPointMake(self.navigationController.view.frame.size.width - 20, 0)
                                 inView:self.view
                              withTitle:NSLocalizedString(@"More", @"Popover title")
                        withStringArray:[NSArray arrayWithObjects:NSLocalizedString(@"Stop Presentation", nil), NSLocalizedString(@"Restart", nil), NSLocalizedString(@"Blank Screen", nil), nil]
                               delegate:self];
    } else {
        [PopoverView showPopoverAtPoint: CGPointMake(self.navigationController.view.frame.size.width - 20, 0)
                                 inView:self.view
                              withTitle:@"More"
                        withStringArray:[NSArray arrayWithObjects:NSLocalizedString(@"Stop Presentation", nil), NSLocalizedString(@"Restart", nil), NSLocalizedString(@"Resume from blank screen", nil), nil]
                               delegate:self];
    }
}

- (void) viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}


- (void) viewDidAppear:(BOOL)animated
{
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    
    [[(slideShowSwipeInList *)self.revealViewController.rearViewController stopWatch] setDelegate:self.revealViewController];

    self.slideShowImageNoteReadyObserver =[center addObserverForName:MSG_SLIDE_CHANGED
                                                              object:nil
                                                               queue:mainQueue
                                                          usingBlock:^(NSNotification *note) {
                                                              [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.slideView];
                                                              [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.touchPointerImage];
                                                              [self.slideshow getContentAtIndex:self.slideshow.currentSlide+1 forView:self.secondarySlideView];
                                                              [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.lecturer_notes];
                                                              [self.slideNumber setText:[NSString stringWithFormat:@"%u/%u", [self.slideshow currentSlide]+1, [self.slideshow size]]];

                                                          }];

    self.slideShowFinishedObserver = [center addObserverForName:STATUS_CONNECTED_NOSLIDESHOW
                                                         object:nil
                                                          queue:mainQueue
                                                     usingBlock:^(NSNotification *note) {
                                                         [self.navigationController popViewControllerAnimated:YES];
                                                     }];
    self.slideView.layer.shadowColor = [[UIColor blackColor] CGColor];
    self.slideView.layer.shadowOpacity = 0.5;
    self.slideView.layer.shadowRadius = 4.0;
    self.slideView.layer.shadowOffset = CGSizeMake(3.0f, 3.0f);
    self.slideView.layer.shadowPath = [UIBezierPath bezierPathWithRect:self.slideView.bounds].CGPath;
    self.slideView.clipsToBounds = NO;

    self.secondarySlideView.layer.shadowColor = [[UIColor blackColor] CGColor];
    self.secondarySlideView.layer.shadowOpacity = 0.5;
    self.secondarySlideView.layer.shadowRadius = 4.0;
    self.secondarySlideView.layer.shadowOffset = CGSizeMake(3.0f, 3.0f);
    self.secondarySlideView.layer.shadowPath = [UIBezierPath bezierPathWithRect:self.secondarySlideView.bounds].CGPath;
    self.secondarySlideView.clipsToBounds = NO;

    // We calibrate once when presentation starts. needs a users alert to inform users to point at the center of the screen at the beginning
    self.pointerCalibrationOn = YES;

    [super viewDidAppear:animated];
}

- (void) viewDidDisappear:(BOOL)animated
{
    self.count = 0;
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowFinishedObserver];
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowImageNoteReadyObserver];
    [super viewDidDisappear:animated];
}



- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidUnload {
    [self setLecturer_notes:nil];
    [self setSlideView:nil];
    [self setSlideNumber:nil];
    [self setSecondarySlideView:nil];
    [self setNotesView:nil];
    [self setMovingPointer:nil];
    [self setBlockingView:nil];
    [self setBottomView:nil];
    [self setPointerBtn:nil];
    [super viewDidUnload];
}

#pragma mark - Slides Control
- (IBAction)nextSlideAction:(id)sender {
    [[self.comManager transmitter] nextTransition];
}

- (IBAction)previousSlideAction:(id)sender {
    [[self.comManager transmitter] previousTransition];
}

static BOOL isBlank = NO;
#pragma mark - PopOver delegates
- (void) popoverView:(PopoverView *)popoverView didSelectItemAtIndex:(NSInteger)index
{
    [popoverView dismiss];
    switch (index) {
        case 0:
            // Stop Presentation
            [self.comManager.transmitter stopPresentation];
            [self.navigationController popViewControllerAnimated:YES];
            break;
        case 1:
            [self.comManager.transmitter gotoSlide:0];
            break;
        case 2:
            if (!isBlank) {
                [self.comManager.transmitter blankScreen];
            } else {
                [self.comManager.transmitter resume];
            }
            isBlank = !isBlank;
            break;
        default:
            NSLog(@"Pop over didSelectItemAtIndex out of bound, should not happen");
            break;
    }
}


@end
