// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "slideShowSwipeInList_ipad.h"
#import "CommunicationManager.h"
#import "CommandInterpreter.h"
#import "SlideShow.h"
#import "CommandTransmitter.h"
#import "SWRevealViewController.h"
#import "slideShowPreviewTable_vc.h"
#import "UIView+Shadowing.h"
#import <QuartzCore/CALayer.h>

@interface slideShowSwipeInList_ipad ()

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) SlideShow *slideshow;
@property (nonatomic, strong) id slideChangedObserver;

@end

@implementation slideShowSwipeInList_ipad

@synthesize comManager = _comManager;
@synthesize slideshow = _slideshow;
@synthesize slideChangedObserver = _slideChangedObserver;

dispatch_queue_t backgroundQueue;

- (void) handleBack:(id)sender
{
    [self.comManager.transmitter stopPresentation];
    [self.tableView reloadData];
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (SlideShow *)slideshow
{
    return self.comManager.interpreter.slideShow;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	self.comManager = [CommunicationManager sharedComManager];
    self.slideshow.secondaryDelegate = self;
    
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    NSOperationQueue *mainQueue = [NSOperationQueue mainQueue];
    self.slideChangedObserver =[center addObserverForName:MSG_SLIDE_CHANGED
                                                   object:nil
                                                    queue:mainQueue
                                               usingBlock:^(NSNotification *note) {
                                                   if ([self.tableView numberOfRowsInSection:0] > 0){
                                                       NSIndexPath *indexPath = [NSIndexPath indexPathForRow:self.slideshow.currentSlide inSection:0];
                                                       [self.tableView selectRowAtIndexPath:indexPath animated:YES scrollPosition:UITableViewScrollPositionNone];
                                                       [self.tableView scrollToRowAtIndexPath:indexPath atScrollPosition:UITableViewScrollPositionMiddle animated:YES];
                                                   }
                                               }];
    self.tableView.backgroundColor = [UIColor colorWithRed:.674509804 green:.729411765 blue:.760784314 alpha:1.0];
}

- (void)viewDidUnload
{
    [[NSNotificationCenter defaultCenter] removeObserver:self.slideChangedObserver];
    [super viewDidUnload];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void) viewWillAppear:(BOOL)animated
{
    [self.tableView reloadData];
    [super viewWillAppear:animated];
}

- (void) viewDidAppear:(BOOL)animated
{
    if ([self.comManager.interpreter.slideShow size] > 0) {
        NSIndexPath *indexPath = [NSIndexPath indexPathForRow:self.slideshow.currentSlide
                                                    inSection:0];
        [self.tableView selectRowAtIndexPath:indexPath animated:NO scrollPosition:UITableViewScrollPositionNone];
        [self.tableView scrollToRowAtIndexPath:indexPath atScrollPosition:UITableViewScrollPositionMiddle animated:NO];
    }
}

- (void) didReceivePresentationStarted
{
    self.slideshow.secondaryDelegate = self;
    [self.tableView reloadData];
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.slideshow size];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"slide";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: CellIdentifier];
    UILabel * slideNumber = (UILabel *)[cell viewWithTag:2];
    UIImageView * thumbnail = (UIImageView *)[cell viewWithTag:1];
    
    [thumbnail setShadow];
    // Starting 20, all tags are used for thumbnails in this sidebar
    [cell setTag:20+indexPath.row];
    [self.slideshow getContentAtIndex:indexPath.row forView:cell];
    [slideNumber setText:[NSString stringWithFormat:@"%u", indexPath.row+1]];
    return cell;
}

- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath{
    // Otherwise selection will disable background color and make slide number unreadable
    if(cell.selected){
        UILabel *label = (UILabel *)[cell viewWithTag:2];
        if ([label backgroundColor]!=[UIColor lightGrayColor]) {
            [label setBackgroundColor:[UIColor lightGrayColor]];
        }
    }
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [self.comManager.transmitter gotoSlide:indexPath.row];
    [[[self.tableView cellForRowAtIndexPath:indexPath] viewWithTag:2] setBackgroundColor:[UIColor lightGrayColor]];
}

@end
