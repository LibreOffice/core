//
//  NSObject+UIImageView_setImageAnimated.m
//  iosremote
//
//  Created by Siqi Liu on 7/31/13.
//  Copyright (c) 2013 libreoffice. All rights reserved.
//

#import "UIImageView+setImageAnimated.h"

@implementation UIImageView (setImageAnimated)

- (void) setImage:(UIImage *)image animated:(BOOL) animated Duration:(double)duration
{
    if (animated) {
        [UIView transitionWithView:self
                          duration:duration
                           options:UIViewAnimationOptionTransitionCrossDissolve
                        animations:^{
                            self.image = image;
                        } completion:nil];
    } else
        [self setImage:image];
}

- (void) setImage:(UIImage *)image animated:(BOOL) animated
{
    [self setImage:image animated:animated Duration:0.5];
}
@end
