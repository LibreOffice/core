// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "UIView+Shadowing.h"
#import <QuartzCore/CALayer.h>

@implementation UIView (Shadowing)

- (void) setShadow
{
    self.layer.shadowColor = [[UIColor blackColor] CGColor];
    self.layer.shadowOpacity = 0.5;
    self.layer.shadowRadius = 4.0;
    self.layer.shadowOffset = CGSizeMake(3.0f, 3.0f);
    self.layer.shadowPath = [UIBezierPath bezierPathWithRect:self.bounds].CGPath;
    self.clipsToBounds = NO;
}

- (void) setShadowLight
{
    self.layer.shadowColor = [[UIColor blackColor] CGColor];
    self.layer.shadowOpacity = 0.3;
    self.layer.shadowRadius = 3.0;
    self.layer.shadowOffset = CGSizeMake(4.0f, 3.0f);
    self.layer.shadowPath = [UIBezierPath bezierPathWithRect:self.bounds].CGPath;
    self.clipsToBounds = NO;
}

@end
