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

@interface libreoffice_sdremoteViewController ()

// For debug use, will use a manager to manage server and client instead
@property (nonatomic, strong) Server* server;
@property (nonatomic, strong) Client* client;

@end

@implementation libreoffice_sdremoteViewController

@synthesize server = _server;
@synthesize client = _client;

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


- (IBAction)connectToServer:(id)sender {
    NSString * address = [self.ipAddressTextEdit text];
    self.server = [[Server alloc] initWithProtocol:NETWORK atAddress:address ofName:@"Server"];
    self.client = [[Client alloc] initWithServer:self.server managedBy:nil interpretedBy:nil];
    [self.client connect];
}


- (void)viewDidUnload {
    [self setIpAddressTextEdit:nil];
    [super viewDidUnload];
}
@end
