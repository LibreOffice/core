//
//  UINavigationController+Theme.m
//  VLC for iOS
//
//  Created by Romain Goyet on 14/06/13.
//  Copyright (c) 2013 Applidium. All rights reserved.
//
//  Refer to the COPYING file of the official project for license.
//

#import "UINavigationController+Theme.h"

@implementation UINavigationController (Theme)
- (void)loadTheme
{
    UINavigationBar *navBar = self.navigationBar;
    [navBar setBackgroundImage:[UIImage imageNamed:@"navBarBackground"]
                 forBarMetrics:UIBarMetricsDefault];
    [navBar setBackgroundImage:[UIImage imageNamed:@"navBarBackgroundPhoneLandscape"]
                 forBarMetrics:UIBarMetricsLandscapePhone];
    navBar.barStyle = UIBarStyleBlack;
}
@end
