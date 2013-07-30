//
//  slideShowPreview_vc~ipad.m
//  iosremote
//
//  Created by Siqi Liu on 7/26/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowPreviewTable_vc_ipad.h"
#import "slideShowPreviewTable_vc.h"
#import "CommunicationManager.h"
#import "CommandTransmitter.h"
#import "CommandInterpreter.h"
#import "SlideShow.h"
#import "MainSplitViewController.h"

@interface slideShowPreviewTable_vc_ipad ()

@end

@implementation slideShowPreviewTable_vc_ipad

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
    NSLog(@"%@", [self.delegate class]);
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        self.optionsArray = [NSArray arrayWithObjects:OPTION_TIMER, nil];
    } else 
        self.optionsArray = [NSArray arrayWithObjects:OPTION_TIMER, OPTION_POINTER, nil];
    self.comManager = [CommunicationManager sharedComManager];
    self.comManager.delegate = self;
}

- (void) viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    if ([self.comManager.interpreter.slideShow size] > 0){
        NSLog(@"3");
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


@end
