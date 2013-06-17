//
//  slideShowViewController.m
//  iosremote
//
//  Created by Liu Siqi on 6/9/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowViewController.h"
#import "SlideShow.h"
#import "CommandTransmitter.h"
#import "CommunicationManager.h"

@interface slideShowViewController ()

@property (nonatomic, strong) CommunicationManager* comManager;

@end

@implementation slideShowViewController

@synthesize slideshow = _slideshow;
@synthesize slideShowImageReadyObserver = _slideShowImageReadyObserver;
@synthesize slideShowNoteReadyObserver = _slideShowNoteReadyObserver;
@synthesize comManager = _comManager;

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
    
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];

    self.slideShowImageReadyObserver =[center addObserverForName:MSG_SLIDE_CHANGED
                                                          object:nil
                                                           queue:mainQueue
                                                      usingBlock:^(NSNotification *note) {
                                                                  NSLog(@"Getting slide: %u image to display: %@", self.slideshow.currentSlide, [self.slideshow getImageAtIndex:self.slideshow.currentSlide]);
                                                          
                                                                  [self.image setImage:[self.slideshow getImageAtIndex:self.slideshow.currentSlide]];
                                                                  [self.lecturer_notes loadHTMLString: [self.slideshow getNotesAtIndex:self.slideshow.currentSlide]baseURL:nil];
                                                              }];
    
    self.slideShowNoteReadyObserver = [center addObserverForName:STATUS_CONNECTED_NOSLIDESHOW
                                                          object:nil
                                                           queue:mainQueue
                                                      usingBlock:^(NSNotification *note) {
                                                                  [self.navigationController popViewControllerAnimated:YES];
                                                              }];
    
    self.comManager = [CommunicationManager sharedComManager];
}

- (IBAction)nextSlide:(id)sender {
    [self.comManager.transmitter nextTransition];
}

- (IBAction)previousSlide:(id)sender {
    [self.comManager.transmitter previousTransition];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidUnload {
    [self setImage:nil];
    [self setLecturer_notes:nil];
    [super viewDidUnload];
}
@end
