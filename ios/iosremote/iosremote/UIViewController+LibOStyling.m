//
//  UIViewController+LibOStyling.m
//  iosremote
//
//  Created by Siqi Liu on 8/6/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "UIViewController+LibOStyling.h"
#import "ControlVariables.h"

@implementation UIViewController (LibOStyling)

- (void)setTitle:(NSString *)title
{
    UILabel *titleView = (UILabel *)self.navigationItem.titleView;
    if (!titleView) {
        titleView = [[UILabel alloc] initWithFrame:CGRectZero];
        titleView.backgroundColor = [UIColor clearColor];
        titleView.font = kAppTitleFont;
        titleView.shadowColor = nil;
        
        titleView.textColor = [UIColor blackColor];
        
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
