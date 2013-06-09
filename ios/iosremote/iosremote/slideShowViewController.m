//
//  slideShowViewController.m
//  iosremote
//
//  Created by Liu Siqi on 6/9/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowViewController.h"

@interface slideShowViewController ()

@end

@implementation slideShowViewController

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
    [self setImage:nil];
    [super viewDidUnload];
}
@end
