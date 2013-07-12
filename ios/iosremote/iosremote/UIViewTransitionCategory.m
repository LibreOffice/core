/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#import "UIViewTransitionCategory.h"

@implementation UIView(Transition)
-(void)fadeInfadeOutwithDuration:(double)duration maxAlpha:(double)maxAlpha
{
    if (self.isHidden){
        [self setHidden:NO];
        self.alpha = 0.0;
        [UIView animateWithDuration:duration animations:^{
            self.alpha = maxAlpha;
        } completion:^(BOOL finished) {
            if (!finished) {
                [self setHidden:NO];
            }
        }];
    } else {
        self.alpha = maxAlpha;
        [UIView animateWithDuration:duration animations:^{
            self.alpha = 0.0f;
        } completion:^(BOOL finished) {
            [self setHidden:YES];
        }];
    }
}
@end