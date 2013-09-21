// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "SlideShowPreviewViewController~ipad.h"
#import "SlideShowPreviewViewController.h"
#import "CommunicationManager.h"
#import "CommandTransmitter.h"
#import "CommandInterpreter.h"
#import "SlideShow.h"
#import "MainSplitViewController.h"
#import "UIViewController+LibOStyling.h"

@interface SlideShowPreviewViewController_ipad ()

@end

@implementation SlideShowPreviewViewController_ipad

#pragma mark - System defaults

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void) viewDidLoad
{
   [super viewDidLoad];
    if (self.navigationController)
        self.delegate = (MainSplitViewController *) self.navigationController.presentingViewController;
    else
        self.delegate = (MainSplitViewController *) self.presentingViewController;
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
    
    UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Connect", nil) style:UIBarButtonItemStyleBordered target:self action:@selector(handleBack)];
    [backButton setBackgroundImage:[UIImage imageNamed:@"backButton"] forState:UIControlStateNormal barMetrics:UIBarMetricsDefault];
    self.navigationItem.leftBarButtonItem = backButton;
}

- (void) viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    if ([self.comManager.interpreter.slideShow size] > 0){
        [self.delegate didReceivePresentationStarted];
    }
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.slideShowStartObserver = [[NSNotificationCenter defaultCenter] addObserverForName:STATUS_CONNECTED_SLIDESHOW_RUNNING
                                                                                    object:nil
                                                                                     queue:mainQueue
                                                                                usingBlock:^(NSNotification *note) {
                                                                                    [self.delegate didReceivePresentationStarted];
                                                                                }];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (void)viewDidUnload {
    [super viewDidUnload];
}
@end
