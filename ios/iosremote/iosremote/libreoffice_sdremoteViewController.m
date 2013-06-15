// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import "libreoffice_sdremoteViewController.h"
#import "Server.h"
#import "slideShowViewController.h"
#import "CommunicationManager.h"

@interface libreoffice_sdremoteViewController ()

// For debug use, will use a manager to manage server and client instead
@property (nonatomic, strong) Server* server;
@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, weak) NSNotificationCenter* center;
@property (nonatomic, strong) id slideShowPreviewStartObserver;

@end

@implementation libreoffice_sdremoteViewController

@synthesize server = _server;
@synthesize center = _center;
@synthesize comManager = _comManager;
@synthesize slideShowPreviewStartObserver = _slideShowPreviewStartObserver;

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    self.center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.slideShowPreviewStartObserver = [self.center addObserverForName:STATUS_CONNECTED_SLIDESHOW_RUNNING object:nil
                                                     queue:mainQueue usingBlock:^(NSNotification *note) {
                                                         NSLog(@"Received performSegue!");
                                                         [self performSegueWithIdentifier:@"slidesPreviewSegue" sender:self];
                                                     }];

}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    if ([segue.identifier isEqualToString:@"slidesPreviewSegue"]) {
        slideShowViewController *destViewController = segue.destinationViewController;
        [destViewController.slideshow setDelegate:destViewController];
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (IBAction)connectToServer:(id)sender {
    NSString * address = [self.ipAddressTextEdit text];
    self.comManager = [[CommunicationManager alloc] init];
    self.server = [[Server alloc] initWithProtocol:NETWORK atAddress:address ofName:@"Macbook Pro Retina"];
    [self.comManager setDelegate:self];
    [self.comManager connectToServer:self.server];
}

- (void)viewDidUnload {
    [self setIpAddressTextEdit:nil];
    [self setPinLabel:nil];
    [super viewDidUnload];
}

- (void)setPinLabelText:(NSString *)text{
    [self.pinLabel setText:text];
}
@end
