/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#import "slideShow_vc.h"
#import "SlideShow.h"
#import "setRecursiveUserInteraction.h"
#import "CommunicationManager.h"
#import "SWRevealViewController.h"
#import "CommandInterpreter.h"
#import "CommandTransmitter.h"
#import "slideShowPreviewTable_vc.h"
#import <QuartzCore/QuartzCore.h>
#import <CoreMotion/CoreMotion.h>


#define CURRENT_SLIDE_IMAGEVIEW 1
#define NEXT_SLIDE_IMAGEVIEW 2
#define TOUCH_POINTER_VIEW 3
#define CURRENT_SLIDE_NOTES 4

@interface slideShow_vc ()

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
        [self.touchPointerScrollView setHidden:![self.touchPointerScrollView isHidden]];
    }
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    UITouch *touch = [[event allTouches] anyObject];
    CGPoint loc = [touch locationInView:self.slideView];
    if (loc.x >= 0 && loc.x <= self.slideView.frame.origin.x+self.slideView.frame.size.width
        && loc.y >= 0 && loc.y <= self.slideView.frame.origin.y + self.slideView.frame.size.height)
        NSLog(@"Touch begins at: %f, %f", loc.x, loc.y);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [[event allTouches] anyObject];
    CGPoint loc = [touch locationInView:self.slideView];
    if (loc.x >= 0 && loc.x <= self.slideView.frame.origin.x+self.slideView.frame.size.width
        && loc.y >= 0 && loc.y <= self.slideView.frame.origin.y + self.slideView.frame.size.height)
        NSLog(@"Touch at: %f, %f", loc.x, loc.y);
}

#pragma mark - System defaults

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
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
    self.slideshow = [self.comManager.interpreter slideShow];
    self.slideshow.delegate = self;

    [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.slideView];
    [self.slideshow getContentAtIndex:self.slideshow.currentSlide+1 forView:self.secondarySlideView];
    [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.lecturer_notes];
    [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.touchPointerImage];
    [self.slideNumber setText:[NSString stringWithFormat:@"%u/%u", [self.slideshow currentSlide]+1, [self.slideshow size]]];


    UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:@"Stop Presentation"
                                                                   style:UIBarButtonItemStyleBordered
                                                                  target:self
                                                                  action:@selector(handleBack:)];
    [backButton setTintColor:[UIColor redColor]];
    self.revealViewController.navigationItem.leftBarButtonItem = backButton;

    self.revealButtonItem = [[UIBarButtonItem alloc] initWithImage: [UIImage imageNamed:@"more_icon.png"]
                                                             style:UIBarButtonItemStyleBordered
                                                            target:self.revealViewController
                                                            action:@selector( revealToggle: )];
    self.revealViewController.navigationItem.rightBarButtonItem = self.revealButtonItem;
    [self.navigationController.navigationBar addGestureRecognizer: self.revealViewController.panGestureRecognizer];

    self.pointerCalibrationOn = NO;
    self.movingPointer.layer.cornerRadius = 3;

    BOOL acc = [[NSUserDefaults standardUserDefaults] boolForKey:KEY_POINTER];
    if (!acc) {
        [self.pointerBtn addTarget:self action:@selector(pointerAction:) forControlEvents:UIControlEventTouchUpOutside];
        [self.pointerBtn addTarget:self action:@selector(pointerAction:) forControlEvents:UIControlEventTouchUpInside];
    }
}

- (void) handleBack:(id)sender
{
    [self.comManager.transmitter stopPresentation];
    [self.navigationController popViewControllerAnimated:YES];
}


- (void) viewDidAppear:(BOOL)animated
{
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];

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
    [self setTouchPointerScrollView:nil];
    [self setPointerBtn:nil];
    [super viewDidUnload];
}

- (IBAction)nextSlideAction:(id)sender {
    [[self.comManager transmitter] nextTransition];
}

- (IBAction)previousSlideAction:(id)sender {
    [[self.comManager transmitter] previousTransition];
}

@end
