//
//  slideShowSwipeInList.m
//  iosremote
//
//  Created by Liu Siqi on 7/8/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "slideShowSwipeInList.h"
#import "CommunicationManager.h"
#import "CommandInterpreter.h"
#import "SlideShow.h"
#import "CommandTransmitter.h"
#import "SWRevealViewController.h"

@interface slideShowSwipeInList ()

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) SlideShow *slideshow;
@end

@implementation slideShowSwipeInList

@synthesize comManager = _comManager;
@synthesize slideshow = _slideshow;

- (void) viewDidLoad
{
    [super viewDidLoad];
    
    self.comManager = [CommunicationManager sharedComManager];
    self.slideshow = self.comManager.interpreter.slideShow;
    self.slideshow.secondaryDelegate = self;
}

- (void) prepareForSegue: (UIStoryboardSegue *) segue sender: (id) sender
{
    // configure the segue.
    // in this case we dont swap out the front view controller, which is a UINavigationController.
    // but we could..
    if ( [segue isKindOfClass: [SWRevealViewControllerSegue class]] )
    {
        SWRevealViewControllerSegue* rvcs = (SWRevealViewControllerSegue*) segue;
        
        SWRevealViewController* rvc = self.revealViewController;
        NSAssert( rvc != nil, @"oops! must have a revealViewController" );
        
        NSAssert( [rvc.frontViewController isKindOfClass: [UINavigationController class]], @"oops!  for this segue we want a permanent navigation controller in the front!" );
        
        rvcs.performBlock = ^(SWRevealViewControllerSegue* rvc_segue, UIViewController* svc, UIViewController* dvc) {
            
            UINavigationController* nc = (UINavigationController*)rvc.frontViewController;
            [nc setViewControllers: @[ dvc ] animated: YES ];
            
            [rvc setFrontViewPosition: FrontViewPositionLeft animated: YES];
        };
    }
}


#pragma mark - Table view data source

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.slideshow size];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.row == 0) {
        static NSString *CellIdentifier = @"stopWatch";
        
        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: CellIdentifier];
        [cell setSelectionStyle:UITableViewCellSelectionStyleNone];
        cell.contentView.backgroundColor = [UIColor whiteColor];
        return cell;
    } else {
        static NSString *CellIdentifier = @"slide";
        
        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: CellIdentifier];
        UILabel * slideNumber = (UILabel *)[cell viewWithTag:2];
        
        [cell setTag:20+indexPath.row];
        [self.slideshow getContentAtIndex:indexPath.row-1 forView:cell];
        [slideNumber setText:[NSString stringWithFormat:@"%u", indexPath.row]];
        return cell;
    }
}


- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [self.comManager.transmitter gotoSlide:indexPath.row - 1];
    [self.revealViewController revealToggle: self];
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}


- (void)viewDidUnload {
    [self setSlidesTable:nil];
    [super viewDidUnload];
}
@end
