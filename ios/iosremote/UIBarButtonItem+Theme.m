/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#import "UIBarButtonItem+Theme.h"

@implementation UIBarButtonItem (Theme)
+ (UIBarButtonItem *)themedDoneButtonWithTarget:(id)target andSelector:(SEL)selector
{
    UIBarButtonItem *doneButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"BUTTON_DONE", @"")
                                                                      style:UIBarButtonItemStyleBordered
                                                                     target:target
                                                                     action:selector];
    [doneButton setBackgroundImage:[UIImage imageNamed:@"doneButton"]
                          forState:UIControlStateNormal
                        barMetrics:UIBarMetricsDefault];
    [doneButton setBackgroundImage:[UIImage imageNamed:@"doneButtonHighlight"]
                          forState:UIControlStateHighlighted
                        barMetrics:UIBarMetricsDefault];
    [doneButton setTitleTextAttributes:@{UITextAttributeTextShadowColor : [UIColor whiteColor], UITextAttributeTextColor : [UIColor blackColor]}
                              forState:UIControlStateNormal];
    return doneButton;
}

+ (UIBarButtonItem *)themedBackButtonWithTarget:(id)target andSelector:(SEL)selector
{
    UIBarButtonItem *backButton = [[UIBarButtonItem alloc] initWithTitle:NSLocalizedString(@"BUTTON_BACK", @"")
                                                                   style:UIBarButtonItemStyleBordered
                                                                  target:target
                                                                  action:selector];
    [backButton setBackgroundImage:[[UIImage imageNamed:@"backButton"] resizableImageWithCapInsets:UIEdgeInsetsMake(0, 12, 0, 6)]
                          forState:UIControlStateNormal
                        barMetrics:UIBarMetricsDefault];
    [backButton setBackgroundImage:[[UIImage imageNamed:@"backButtonHighlight"] resizableImageWithCapInsets:UIEdgeInsetsMake(0, 12, 0, 6)]
                          forState:UIControlStateHighlighted
                        barMetrics:UIBarMetricsDefault];
    [backButton setTitleTextAttributes:@{UITextAttributeTextShadowColor : [UIColor colorWithWhite:0. alpha:.37], UITextAttributeTextColor : [UIColor whiteColor]} forState:UIControlStateNormal];
    [backButton setTitlePositionAdjustment:UIOffsetMake(3, 0) forBarMetrics:UIBarMetricsDefault];
    return backButton;
}

@end
