/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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
