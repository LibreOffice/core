//
//  WalkThroughContainerViewController.m
//  iosremote
//
//  Created by Siqi Liu on 8/27/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "WalkThroughContainerViewController.h"
#import "WalkThroughPageViewController.h"
#import <QuartzCore/QuartzCore.h>

@interface WalkThroughContainerViewController ()

@end

@implementation WalkThroughContainerViewController

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
    self.pageController = [[UIPageViewController alloc] initWithTransitionStyle:UIPageViewControllerTransitionStyleScroll navigationOrientation:UIPageViewControllerNavigationOrientationHorizontal options:nil];
    
    self.pageController.dataSource = self;
    [[self.pageController view] setFrame:[[self view] bounds]];
    WalkThroughPageViewController *initialPageViewController = [self viewControllerAtIndex:0];
    NSArray *viewControllers = [NSArray arrayWithObject:initialPageViewController];
    
    [self.pageController setViewControllers:viewControllers direction:UIPageViewControllerNavigationDirectionForward animated:NO completion:nil];
    [self addChildViewController:self.pageController];
    [[self view] addSubview:self.pageController.view];
    [self.pageController didMoveToParentViewController:self];
    
    UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"Ok, thanks", nil) style:UIBarButtonItemStyleBordered target:self action:@selector(handleBack)];
    [backButton setBackgroundImage:[UIImage imageNamed:@"backButton"] forState:UIControlStateNormal barMetrics:UIBarMetricsDefault];
    self.navigationItem.leftBarButtonItem = backButton;
    
    [self setTitle:NSLocalizedString(@"How-to", nil)];
    
    
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark UIPageViewControllerDataSource methods

- (UIViewController *)pageViewController:(UIPageViewController *)pageViewController viewControllerBeforeViewController:(UIViewController *)viewController
{
    NSUInteger index = [(WalkThroughPageViewController *)viewController index];
    
    if (index == 0) {
        return nil;
    }
    
    index--;
    
    return [self viewControllerAtIndex:index];
}

- (UIViewController *)pageViewController:(UIPageViewController *)pageViewController viewControllerAfterViewController:(UIViewController *)viewController
{
    NSUInteger index = [(WalkThroughPageViewController *)viewController index];
    
    index++;
    
    if (index == 3) {
        return nil;
    }
    
    return [self viewControllerAtIndex:index];
}

- (WalkThroughPageViewController *)viewControllerAtIndex:(NSUInteger)index {
    
    WalkThroughPageViewController *childViewController;
    if (index == 2) {
        childViewController = [[WalkThroughPageViewController alloc] initWithNibName:@"WalkThroughPageViewMainImageController" bundle:nil];
    } else {
        childViewController = [[WalkThroughPageViewController alloc] initWithNibName:@"WalkThroughPageViewController" bundle:nil];
    }
    childViewController.index = index;
    
    return childViewController;
}

- (NSInteger)presentationCountForPageViewController:(UIPageViewController *)pageViewController {
    // The number of items reflected in the page indicator.
    return 3;
}

- (NSInteger)presentationIndexForPageViewController:(UIPageViewController *)pageViewController {
    // The selected item reflected in the page indicator.
    return 0;
}

@end
