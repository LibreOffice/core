/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#import "SlideShowPreviewViewController~iphone.h"
#import "PopoverView.h"
#import "SlideShow.h"
#import "UIView+FadeInFadeOut.h"
#import "CommunicationManager.h"
#import "SWRevealViewController.h"
#import "SlideShowViewController~iphone.h"
#import "CommandInterpreter.h"
#import "stopWatch.h"
#import "CommandTransmitter.h"
#import "SlideShowPreviewViewController.h"
#import "SlideShowSwipeInList~iphone.h"
#import <QuartzCore/QuartzCore.h>
#import "UIImageView+setImageAnimated.h"
#import "UIView+Shadowing.h"
#import <CoreMotion/CoreMotion.h>


#define CURRENT_SLIDE_IMAGEVIEW 1
#define NEXT_SLIDE_IMAGEVIEW 2
#define TOUCH_POINTER_VIEW 3
#define CURRENT_SLIDE_NOTES 4

@interface SlideShowViewController_iphone () <PopoverViewDelegate>

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) id slideShowImageNoteReadyObserver;
@property (nonatomic, strong) id slideShowFinishedObserver;
@property (nonatomic, strong) SlideShow* slideshow;

@property CGPoint refLeftUpperGravity;
@property CGPoint refRightUpperGravity;
@property CGPoint refRightLowerGravity;

// SWReveal Controller
@property (readwrite) IBOutlet UIBarButtonItem* revealButtonItem;

@end

@implementation SlideShowViewController_iphone

@synthesize comManager = _comManager;
@synthesize slideShowImageNoteReadyObserver = _slideShowImageNoteReadyObserver;
@synthesize slideShowFinishedObserver = _slideShowFinishedObserver;
@synthesize slideshow = _slideshow;

#pragma mark - Pointer

- (IBAction)pointerAction:(id)sender {
    if ([self.touchPointerImage isHidden]){
        [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.touchPointerImage];
        CGPoint p = self.view.center;
        p.y -= 50;
        self.touchPointerImage.center = p;
        [self.view removeGestureRecognizer:self.revealViewController.panGestureRecognizer];
    } else {
        [self.view addGestureRecognizer:self.revealViewController.panGestureRecognizer];
    }
    [self.touchPointerImage fadeInfadeOutwithDuration:0.0 maxAlpha:1.0];
    [self.blockingView fadeInfadeOutwithDuration:0.0 maxAlpha:0.7];
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

    self.movingPointer.layer.cornerRadius = 3;

    [self.view addGestureRecognizer:self.revealViewController.panGestureRecognizer];
}

- (void) popOverStart:(id)sender
{
    if (!isBlank) {
        [PopoverView showPopoverAtPoint: CGPointMake([[sender view] center].x, [[sender view] center].y + [[sender view] frame].size.height * 0.5)
                                 inView:[sender view].superview
                        withStringArray:[NSArray arrayWithObjects:NSLocalizedString(@"Stop Presentation", nil), NSLocalizedString(@"Restart", nil), NSLocalizedString(@"Blank Screen", nil), nil]
                               delegate:self];
    } else {
        [PopoverView showPopoverAtPoint: CGPointMake([[sender view] center].x, [[sender view] center].y + [[sender view] frame].size.height * 0.5)
                                 inView:[sender view].superview
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
    
    [[(SlideShowSwipeInList_iphone *)self.revealViewController.rearViewController stopWatch] setDelegate:self.revealViewController];
    
    [[(SlideShowSwipeInList_iphone *)self.revealViewController.rearViewController timer] setDelegate:self.revealViewController];

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
    [self.slideView setShadow];
    [self.secondarySlideView setShadow];

    [super viewDidAppear:animated];
}

- (void) viewDidDisappear:(BOOL)animated
{
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowFinishedObserver];
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideShowImageNoteReadyObserver];
    [[(SlideShowSwipeInList_iphone *)self.revealViewController.rearViewController timer] clear];
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
