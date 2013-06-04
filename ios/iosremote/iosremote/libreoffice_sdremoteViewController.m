//
//  libreoffice_sdremoteViewController.m
//  iosremote
//
//  Created by Liu Siqi on 6/4/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "libreoffice_sdremoteViewController.h"
#import "Server.h"
#import "Client.h"

@interface libreoffice_sdremoteViewController ()

@end

@implementation libreoffice_sdremoteViewController

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
    Server * server = [[Server alloc] initWithProtocol:NETWORK atAddress:address ofName:@"Server"];
    Client * client = [[Client alloc] initWithServer:server managedBy:nil interpretedBy:nil];
    [client connect];
}


- (void)viewDidUnload {
    [self setIpAddressTextEdit:nil];
    [super viewDidUnload];
}
@end
