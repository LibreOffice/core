// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
    
    
    if (self.comManager.interpreter.slideShow)
        [self.titleLabel setText:[self.comManager.interpreter.slideShow title]];
    
    [[NSNotificationCenter defaultCenter] addObserverForName:SLIDESHOW_INFO_RECEIVED object:self.titleObserver queue:[NSOperationQueue mainQueue] usingBlock:^(NSNotification *note) {
        [self.titleLabel setText:self.comManager.interpreter.slideShow.title];
        if (!self.titleLabel) {
            NSLog(@"TitleLabel nil");
        }
        NSLog(@"Received: %@", self.comManager.interpreter.slideShow.title);
    }];
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

- (void)viewDidUnload {
    [self setTitleLabel:nil];
    [super viewDidUnload];
}
@end
