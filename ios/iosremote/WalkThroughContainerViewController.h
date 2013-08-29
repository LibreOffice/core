//
//  WalkThroughContainerViewController.h
//  iosremote
//
//  Created by Siqi Liu on 8/27/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface WalkThroughContainerViewController : UIViewController <UIPageViewControllerDataSource>

@property (strong, nonatomic) UIPageViewController *pageController;

@end
