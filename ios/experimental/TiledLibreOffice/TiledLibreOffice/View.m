// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <touch/touch.h>

#import "View.h"
#import "TiledView.h"

@interface View ()

@property UIView *subView;

@end

@implementation View

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        [self setMaximumZoomScale:4];
        [self setDelegate:self];

        MLODpxSize docSize = touch_lo_get_content_size();

        double widthScale = frame.size.width / docSize.width;
        double docAspectRatio = docSize.height / docSize.width;

        NSLog(@"View frame=%.0fx%.0f docSize=%.0fx%.0f scale=%.3f aspectRatio=%.3f", frame.size.width, frame.size.height, docSize.width, docSize.height, widthScale, docAspectRatio);

        self.subView = [[TiledView alloc] initWithFrame:CGRectMake(0, 0, frame.size.width, frame.size.width*docAspectRatio) andScale:widthScale];
        [self addSubview:self.subView];
    }
    return self;
}

- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView
{
    return self.subView;
}

- (void)scrollViewWillBeginZooming:(UIScrollView *)scrollView withView:(UIView *)view
{
}

- (void)scrollViewDidEndZooming:(UIScrollView *)scrollView withView:(UIView *)view atScale:(CGFloat)scale
{
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
