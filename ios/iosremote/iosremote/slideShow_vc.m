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
#import "CommandInterpreter.h"
#import "CommandTransmitter.h"

@interface slideShow_vc ()

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) id slideShowImageNoteReadyObserver;
@property (nonatomic, strong) id slideShowFinishedObserver;
@property (nonatomic, strong) SlideShow* slideshow;

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
    self.comManager = [CommunicationManager sharedComManager];
    self.slideshow = [self.comManager.interpreter slideShow];
    self.slideshow.delegate = self;
    
    [self.slideView setImage:[self.slideshow getImageAtIndex:self.slideshow.currentSlide]];
    [self.lecturer_notes loadHTMLString: [self.slideshow getNotesAtIndex:self.slideshow.currentSlide]baseURL:nil];
    
    [self.lecturer_notes setBackgroundColor:[UIColor colorWithPatternImage:[UIImage imageNamed:@"lines.png"]]];
}


- (void) viewDidAppear:(BOOL)animated
{
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    
    self.slideShowImageNoteReadyObserver =[center addObserverForName:MSG_SLIDE_CHANGED
                                                              object:nil
                                                               queue:mainQueue
                                                          usingBlock:^(NSNotification *note) {
                                                              NSLog(@"Getting slide: %u image to display: %@", self.slideshow.currentSlide, [self.slideshow getImageAtIndex:self.slideshow.currentSlide]);
                                                              
                                                              [self.slideView setImage:[self.slideshow getImageAtIndex:self.slideshow.currentSlide]];
                                                              [self.lecturer_notes loadHTMLString: [self.slideshow getNotesAtIndex:self.slideshow.currentSlide]baseURL:nil];
                                                          }];
    
    self.slideShowFinishedObserver = [center addObserverForName:STATUS_CONNECTED_NOSLIDESHOW
                                                         object:nil
                                                          queue:mainQueue
                                                     usingBlock:^(NSNotification *note) {
                                                         [self.navigationController popViewControllerAnimated:YES];
                                                     }];
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
    [super viewDidUnload];
}

- (IBAction)nextSlideAction:(id)sender {
    [[self.comManager transmitter] nextTransition];
}

- (IBAction)previousSlideAction:(id)sender {
    [[self.comManager transmitter] previousTransition];
}
@end
