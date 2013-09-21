// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
