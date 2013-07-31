// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MainSplitViewController.h"
#import "server_list_vc_ipad.h"
#import "CommunicationManager.h"
#import "BasePresentationViewController.h"
#import "slideShowSwipeInList_ipad.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface MainSplitViewController ()

@property UINavigationController * masterViewController;
@property BasePresentationViewController * detailViewController;

@end

@implementation MainSplitViewController

# pragma mark - System defaults

- (BOOL) shouldAutorotate
{
    return YES;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}


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
    [[[[UIApplication sharedApplication] delegate] window] setRootViewController:self];
    // Do any additional setup after loading the view.
    
    self.detailViewController = [self.viewControllers objectAtIndex:1];
    self.masterViewController = [self.viewControllers objectAtIndex:0];
    
    self.delegate = self.detailViewController;
}

-(void) viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void) didReceivePresentationStarted
{
    [self.detailViewController hideMaster:NO];
    [self dismissViewControllerAnimated:YES completion:nil];
//    [(UITableView *)[(slideShowSwipeInList_ipad *)[[self viewControllers] objectAtIndex:2] view] reloadData];
    [(slideShowSwipeInList_ipad *)[(UINavigationController *)[[self viewControllers] objectAtIndex:0] topViewController] didReceivePresentationStarted];
    [(BasePresentationViewController *)[[self viewControllers] objectAtIndex:1] setWelcomePageVisible:NO];
}

@end
