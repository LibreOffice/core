//
//  slideShow_vc.m
//  iosremote
//
//  Created by Liu Siqi on 7/5/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShow_vc.h"
#import "SlideShow.h"
#import "CommunicationManager.h"
#import "SWRevealViewController.h"
#import "CommandInterpreter.h"
#import "CommandTransmitter.h"
#import <QuartzCore/QuartzCore.h>


#define CURRENT_SLIDE_IMAGEVIEW 1
#define NEXT_SLIDE_IMAGEVIEW 2
#define CURRENT_SLIDE_NOTES 3

@interface slideShow_vc ()

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) id slideShowImageNoteReadyObserver;
@property (nonatomic, strong) id slideShowFinishedObserver;
@property (nonatomic, strong) SlideShow* slideshow;

// SWReveal Controller
@property (readwrite) IBOutlet UIBarButtonItem* revealButtonItem;

@end

@implementation slideShow_vc

@synthesize comManager = _comManager;
@synthesize slideShowImageNoteReadyObserver = _slideShowImageNoteReadyObserver;
@synthesize slideShowFinishedObserver = _slideShowFinishedObserver;
@synthesize slideshow = _slideshow;

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
    
    self.comManager = [CommunicationManager sharedComManager];
    self.slideshow = [self.comManager.interpreter slideShow];
    self.slideshow.delegate = self;
    
    [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.slideView];
    [self.slideshow getContentAtIndex:self.slideshow.currentSlide+1 forView:self.secondarySlideView];
    [self.slideshow getContentAtIndex:self.slideshow.currentSlide forView:self.lecturer_notes];
    [self.slideNumber setText:[NSString stringWithFormat:@"%u/%u", [self.slideshow currentSlide]+1, [self.slideshow size]]];
    
    
    UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:@"End"
                                                                   style:UIBarButtonItemStyleBordered
                                                                  target:self
                                                                  action:@selector(handleBack:)];
    self.revealViewController.navigationItem.leftBarButtonItem = backButton;
    
    self.revealButtonItem = [[UIBarButtonItem alloc] initWithImage: [UIImage imageNamed:@"more_icon.png"]
                                                             style:UIBarButtonItemStyleBordered
                                                            target:self.revealViewController
                                                            action:@selector( revealToggle: )];
    self.revealViewController.navigationItem.rightBarButtonItem = self.revealButtonItem;
    [self.navigationController.navigationBar addGestureRecognizer: self.revealViewController.panGestureRecognizer];
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
    
    [super viewDidAppear:animated];
}

- (void) viewDidDisappear:(BOOL)animated
{
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
    [super viewDidUnload];
}

- (IBAction)nextSlideAction:(id)sender {
    [[self.comManager transmitter] nextTransition];
}

- (IBAction)previousSlideAction:(id)sender {
    [[self.comManager transmitter] previousTransition];
}

- (IBAction)pointerAction:(id)sender {
}
@end
