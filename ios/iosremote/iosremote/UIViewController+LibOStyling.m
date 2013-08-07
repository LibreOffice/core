//
//  UIViewController+LibOStyling.m
//  iosremote
//
//  Created by Siqi Liu on 8/6/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "UIViewController+LibOStyling.h"

@implementation UIViewController (LibOStyling)

- (void)setTitle:(NSString *)title
{
    UILabel *titleView = (UILabel *)self.navigationItem.titleView;
    if (!titleView) {
        titleView = [[UILabel alloc] initWithFrame:CGRectZero];
        titleView.backgroundColor = [UIColor clearColor];
        titleView.font = [UIFont boldSystemFontOfSize:20.0];
        titleView.shadowColor = nil;
        
        titleView.textColor = [UIColor colorWithRed:1.0 green:0.231372549 blue:0.188235294 alpha:1.0];
        
        self.navigationItem.titleView = titleView;
    }
    titleView.text = title;
    [titleView sizeToFit];
}

- (void) handleBack
{
    [self.navigationController popViewControllerAnimated:YES];
}

@end
