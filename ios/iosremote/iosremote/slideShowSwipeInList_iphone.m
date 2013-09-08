/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "slideShowSwipeInList_iphone.h"
#import "CommunicationManager.h"
#import "CommandInterpreter.h"
#import "SlideShow.h"
#import "CommandTransmitter.h"
#import "SWRevealViewController.h"
#import "slideShowPreviewTable_vc.h"
#import "ControlVariables.h"
#import "stopWatch.h"
#import "Timer.h"
#import "UIImageView+setImageAnimated.h"
#import "UIView+Shadowing.h"
#import "UIViewController+LibOStyling.h"
#import <QuartzCore/CALayer.h>
#import <QuartzCore/QuartzCore.h>

@interface slideShowSwipeInList () <UIScrollViewDelegate>

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) SlideShow *slideshow;
@property NSInteger currentPage;

@end

@implementation slideShowSwipeInList

@synthesize comManager = _comManager;
@synthesize currentPage = _currentPage;
@synthesize slideshow = _slideshow;
@synthesize timer = _timer;
@synthesize stopWatch = _stopWatch;

dispatch_queue_t backgroundQueue;


- (void) viewDidLoad
{
    [super viewDidLoad];

    self.comManager = [CommunicationManager sharedComManager];
    self.slideshow = self.comManager.interpreter.slideShow;
    self.slideshow.secondaryDelegate = self;
    
    self.clearsSelectionOnViewWillAppear = NO;
    // set stopwatch as default, users may swipe for a timer
    self.currentPage = 0;
    [self.revealViewController setOwner:STOPWATCH];
    
    self.stopWatch = [[stopWatch alloc] init];
    if ([[NSUserDefaults standardUserDefaults] boolForKey:KEY_TIMER]) {
        [self.stopWatch start];
    }
    
    self.timer = [[Timer alloc] init];
    
    self.tableView.backgroundColor = [UIColor colorWithRed:.674509804 green:.729411765 blue:.760784314 alpha:1.0];
}

- (void) viewDidAppear:(BOOL)animated
{
    if (!self.stopWatch.set) {
        [self.stopWatch setupWithTableViewCell:[self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:0]]];
    }
    if (!self.timer.set) {
        [self.timer setupWithTableViewCell:[self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:0]]];
    }
    [self.stopWatch updateStartButtonIcon];
    [self.timer updateStartButtonIcon];
    if ([self.comManager.interpreter.slideShow size] > 0) {
        [self.stopWatch updateStartButtonIcon];
        [self.timer updateStartButtonIcon];
        NSIndexPath *indexPath = [NSIndexPath indexPathForRow:self.slideshow.currentSlide
                                                    inSection:1];
        [self.tableView selectRowAtIndexPath:indexPath animated:NO scrollPosition:UITableViewScrollPositionNone];
        [self.tableView scrollToRowAtIndexPath:indexPath atScrollPosition:UITableViewScrollPositionMiddle animated:NO];
    }
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

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    // Section one used for stopwatch
    if (section == 0)
        return 1;
    else
        return [self.slideshow size];
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
    return 23;
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
    
    NSString *sectionTitle;
    switch (section) {
        case 0:
            switch (self.currentPage) {
                case 0:
                    sectionTitle = NSLocalizedString(@"Stop Watch", @"Sidebar section header");
                    break;
                case 1:
                    sectionTitle = NSLocalizedString(@"Timer", @"Sidebar section header");
                    break;
                default:
                    break;
            }
            break;
        case 1:
            sectionTitle = NSLocalizedString(@"Slides", @"Sidebar section header");
        default:
            break;
    }
    
    UILabel *label = [[UILabel alloc] init];
    label.frame = CGRectMake(10, 0, 284, 23);
    label.textColor = [UIColor whiteColor];
    label.font = [UIFont boldSystemFontOfSize:14.0];
    label.text = sectionTitle;
    label.backgroundColor = [UIColor clearColor];
    
    // Create header view and add label as a subview
    UIView *view = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 320, 23)];
    view.backgroundColor = [UIColor colorWithRed:1.0 green:0.662745098 blue:0.074509804 alpha:0.9];

    [view addSubview:label];
    
    if (section == 0)
    {
        UIPageControl * pageControl = [[UIPageControl alloc] initWithFrame:CGRectMake(self.revealViewController.rearViewRevealWidth - 20, view.frame.origin.y + 3, 40, 20)];
        pageControl.numberOfPages = 2;
        pageControl.currentPage = self.currentPage;
        
        if ([[[UIDevice currentDevice] systemVersion] floatValue] >= 6.0)
        {
            pageControl.currentPageIndicatorTintColor = [UIColor whiteColor];
            pageControl.pageIndicatorTintColor = [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:0.7];
        }
        pageControl.tag = -100;
        view.tag = -99;
        label.tag = -98;
        [view addSubview:pageControl];
    }
    
    return view;
}

- (void) tableView:(UITableView *)tableView accessoryButtonTappedForRowWithIndexPath:(NSIndexPath *)indexPath
{
    [self performSegueWithIdentifier: @"sw_customized_segue" sender: [tableView cellForRowAtIndexPath: indexPath]];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section == 0) {
        static NSString *CellIdentifier = @"stopWatch";

        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: CellIdentifier];
        [cell setSelectionStyle:UITableViewCellSelectionStyleNone];
        UIView * view = [cell viewWithTag:8];
        [view.layer setCornerRadius:5.0f];
        view.layer.masksToBounds = YES;
        view.clipsToBounds = YES;
        [view setShadow];
        
        view = [cell viewWithTag:9];
        [view.layer setCornerRadius:5.0f];
        view.layer.masksToBounds = YES;
        view.clipsToBounds = YES;
        [view setShadow];
        
        UIScrollView * scroll = (UIScrollView *) [cell viewWithTag:7];
        scroll.contentSize = CGSizeMake(412, 120);
        return cell;
    } else {
        static NSString *CellIdentifier = @"slide";

        UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier: CellIdentifier];
        UILabel * slideNumber = (UILabel *)[cell viewWithTag:2];
        UIImageView * image = (UIImageView *)[cell viewWithTag:1];

        // Starting 20, all tags are used for thumbnails in this sidebar
        [cell setTag:20+indexPath.row];
        [self.slideshow getContentAtIndex:indexPath.row forView:cell];
        [slideNumber setText:[NSString stringWithFormat:@"%u", indexPath.row+1]];
        
        [image setShadow];
        
        cell.contentView.backgroundColor = [UIColor colorWithRed:.674509804-0.02 * indexPath.row green:.729411765-0.02 * indexPath.row blue:.760784314-0.02 * indexPath.row alpha:1.0];
        
        return cell;
    }
}

- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath{
    // Otherwise selection will disable background color and make slide number unreadable
    if(indexPath.section == 1 && cell.selected){
        UILabel *label = (UILabel *)[cell viewWithTag:2];
        if ([label backgroundColor]!=[UIColor lightGrayColor]) {
            [label setBackgroundColor:[UIColor lightGrayColor]];
        }
    }
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section == 0)
        return;
    [self.comManager.transmitter gotoSlide:indexPath.row];
    [[[self.tableView cellForRowAtIndexPath:indexPath] viewWithTag:2] setBackgroundColor:[UIColor lightGrayColor]];
    [self.revealViewController revealToggle: self];
}

- (void)viewDidUnload {
    [self setStopWatch:nil];
    [self setTimer:nil];
    [super viewDidUnload];
}

#pragma mark scrollview delegate

- (void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView
{
    if (scrollView.tag == 7) {
        CGFloat pageWidth = scrollView.frame.size.width;
        float fractionalPage = scrollView.contentOffset.x / pageWidth;
        NSInteger page = lround(fractionalPage);
        if (self.currentPage != page) {
            UIPageControl * pageControl = (UIPageControl *) ([[self.tableView viewWithTag:-99] viewWithTag:-100]);
            UILabel * label = (UILabel *) ([[self.tableView viewWithTag:-99] viewWithTag:-98]);
            
            CATransition *animation = [CATransition animation];
            animation.duration = 0.5;
            animation.type = kCATransitionMoveIn;
            animation.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault];
            [label.layer addAnimation:animation forKey:@"changeTextTransition"];

            switch (page) {
                case 0:
                    [label setText:NSLocalizedString(@"Stop Watch", @"Sidebar section header")];
                    [self.revealViewController setTitle:@""];
                    [self.revealViewController setOwner:STOPWATCH];
                    break;
                case 1:
                    [label setText:NSLocalizedString(@"Timer", @"Sidebar section header")];
                    [self.revealViewController setTitle:@""];
                    [self.revealViewController setOwner:TIMER];
                    break;
                default:
                    break;
            }
            [pageControl setCurrentPage:page];
            self.currentPage = page;
        }
    }
}

@end
