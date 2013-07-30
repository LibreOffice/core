//
//  server_list_vc_ipad.m
//  iosremote
//
//  Created by Siqi Liu on 7/26/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "server_list_vc_ipad.h"

@interface server_list_vc_ipad ()

@end

@implementation server_list_vc_ipad

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
	// Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidUnload {
    [super viewDidUnload];
}

#pragma mark - Actions
- (IBAction)cancelModalView:(id)sender {
    [self.presentingViewController dismissModalViewControllerAnimated:YES];
}
@end
