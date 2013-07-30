//
//  autoDismissKeyboardNavigationViewController.m
//  iosremote
//
//  Created by Siqi Liu on 7/28/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "autoDismissKeyboardNavigationViewController.h"

@interface autoDismissKeyboardNavigationViewController ()

@end

@implementation autoDismissKeyboardNavigationViewController

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

- (BOOL)disablesAutomaticKeyboardDismissal
{
    return NO;
}

@end
