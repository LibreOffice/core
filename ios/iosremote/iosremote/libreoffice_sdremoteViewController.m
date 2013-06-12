// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.


#import "libreoffice_sdremoteViewController.h"
#import "Server.h"
#import "Client.h"
#import "slideShowViewController.h"

@interface libreoffice_sdremoteViewController ()

// For debug use, will use a manager to manage server and client instead
@property (nonatomic, strong) Server* server;
@property (nonatomic, strong) Client* client;
@property (nonatomic, strong) CommandInterpreter * interpreter;
@property (nonatomic, weak) NSNotificationCenter* center;
@property (nonatomic, strong) id slideShowPreviewStartObserver;

@end

@implementation libreoffice_sdremoteViewController

@synthesize server = _server;
@synthesize client = _client;
@synthesize center = _center;
@synthesize interpreter = _interpreter;
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
        destViewController.slideshow = [self.interpreter slideShow];
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
    self.interpreter = [[CommandInterpreter alloc] init];
    self.server = [[Server alloc] initWithProtocol:NETWORK atAddress:address ofName:@"Server"];
    self.client = [[Client alloc] initWithServer:self.server managedBy:nil interpretedBy:self.interpreter];
    [self.client connect];
    
    if([self.client connected])
    {
        [self.pinLabel setText:[NSString stringWithFormat:@"%@", self.client.pin]];
    }
}


- (void)viewDidUnload {
    [self setIpAddressTextEdit:nil];
    [self setPinLabel:nil];
    [self setPinLabel:nil];
    [super viewDidUnload];
}
@end
