//
//  LibONavigationController.m
//  iosremote
//
//  Created by Siqi Liu on 8/5/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "LibONavigationController.h"
#import "UINavigationController+Theme.h"

@interface LibONavigationController ()

@end

@implementation LibONavigationController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self loadTheme];
    
	// Do any additional setup after loading the view.
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
