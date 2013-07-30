//
//  slideShowPreview_vc~iphone.m
//  iosremote
//
//  Created by Siqi Liu on 7/26/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowPreviewTable_vc_iphone.h"
#import "CommandInterpreter.h"
#import "SlideShow.h"

@implementation slideShowPreviewTable_vc_iphone

- (void) viewDidLoad
{
    [super viewDidLoad];
    self.optionsArray = [NSArray arrayWithObjects:OPTION_TIMER, OPTION_POINTER, nil];
    self.comManager = [CommunicationManager sharedComManager];
    self.comManager.delegate = self;
}

- (void) viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];

    if ([self.comManager.interpreter.slideShow size] > 0){
        [self performSegueWithIdentifier:@"slideShowSegue" sender:self];
    }
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.slideShowStartObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_CONNECTED_SLIDESHOW_RUNNING
                                                                                    object:nil
                                                                                     queue:mainQueue
                                                                                usingBlock:^(NSNotification *note) {
                                                                                    [self performSegueWithIdentifier:@"slideShowSegue" sender:self];
                                                                                }];
}

@end
