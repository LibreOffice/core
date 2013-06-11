//
//  slideShowViewController.m
//  iosremote
//
//  Created by Liu Siqi on 6/9/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowViewController.h"
#import "SlideShow.h"

@interface slideShowViewController ()

@end

@implementation slideShowViewController

@synthesize slideshow = _slideshow;
@synthesize slideShowImageReadyObserver = _slideShowImageReadyObserver;
@synthesize slideShowNoteReadyObserver = _slideShowNoteReadyObserver;

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
    [self.image setImage:[self.slideshow getImageAtIndex:0]];
    [self.lecturer_notes loadHTMLString: [self.slideshow getNotesAtIndex:0]baseURL:nil];
    self.slideShowImageReadyObserver = [center addObserverForName:@"IMAGE_READY" object:nil
                                                              queue:mainQueue usingBlock:^(NSNotification *note) {
                                                                  NSLog(@"Getting image to display: %@", [self.slideshow getImageAtIndex:0]);
                                                                  [self.image setImage:[self.slideshow getImageAtIndex:0]];
                                                              }];
    self.slideShowNoteReadyObserver = [center addObserverForName:@"NOTE_READY" object:nil
                                                              queue:mainQueue usingBlock:^(NSNotification *note) {
                                                                  NSLog(@"Getting note to display: %@", [self.slideshow getNotesAtIndex:0]);
                                                                  [self.lecturer_notes loadHTMLString: [self.slideshow getNotesAtIndex:0]baseURL:nil];
                                                              }];
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
