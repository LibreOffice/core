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
#import <QuartzCore/CALayer.h>

@interface slideShowSwipeInList ()

@property (nonatomic, strong) CommunicationManager *comManager;
@property (nonatomic, strong) SlideShow *slideshow;

@end

@implementation slideShowSwipeInList

@synthesize comManager = _comManager;
@synthesize slideshow = _slideshow;

dispatch_queue_t backgroundQueue;

- (void) viewDidLoad
{
    [super viewDidLoad];

    self.comManager = [CommunicationManager sharedComManager];
    self.slideshow = self.comManager.interpreter.slideShow;
    self.slideshow.secondaryDelegate = self;
    
    self.clearsSelectionOnViewWillAppear = NO;
    
    self.stopWatch = [[stopWatch alloc] init];
    if ([[NSUserDefaults standardUserDefaults] boolForKey:KEY_TIMER]) {
        [self.stopWatch start];
    }
    
    self.tableView.backgroundColor = [UIColor colorWithRed:.674509804 green:.729411765 blue:.760784314 alpha:1.0];
}

- (void) viewDidAppear:(BOOL)animated
{
    if (!self.stopWatch.set) {
        [self.stopWatch setupWithTableViewCell:[self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:0]]];
    }
    [self.stopWatch updateStartButtonIcon];
    if ([self.comManager.interpreter.slideShow size] > 0) {
        [self.stopWatch updateStartButtonIcon];
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
            sectionTitle = NSLocalizedString(@"Stop Watch", @"Sidebar section header");
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
        cell.contentView.backgroundColor = [UIColor whiteColor];
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
        
        image.layer.shadowColor = [[UIColor blackColor] CGColor];
        image.layer.shadowOpacity = 0.5;
        image.layer.shadowRadius = 4.0;
        image.layer.shadowOffset = CGSizeMake(3.0f, 3.0f);
        image.layer.shadowPath = [UIBezierPath bezierPathWithRect:image.bounds].CGPath;
        image.clipsToBounds = NO;
        
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
    [super viewDidUnload];
}

@end
