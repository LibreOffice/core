// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "LOViewerWindow.h"

#include <osl/detail/ios-bootstrap.h>

@implementation LOViewerWindow

- (void)drawRect:(CGRect)rect
{
    (void) rect;

    lo_render_windows([self pixelBuffer], [self bounds].size.width, [self bounds].size.height);

    CGContextRef context = UIGraphicsGetCurrentContext();

    CGContextDrawImage(context, [self bounds], [self image]);
}

@end
