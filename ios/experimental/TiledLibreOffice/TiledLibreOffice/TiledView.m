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

@interface TiledView ()

@property CGFloat scale;

@end

@implementation TiledView

- (id)initWithFrame:(CGRect)frame andScale:(CGFloat)scale
{
    self = [super initWithFrame:frame];
    if (self) {
        self.scale = scale;
        CATiledLayer *catl = (CATiledLayer*) [self layer];
        catl.tileSize = CGSizeMake(512, 512);
        catl.levelsOfDetail = 4;
        catl.levelsOfDetailBias = 4;
    }
    return self;
}

+ (Class)layerClass
{
    return [CATiledLayer class];
}

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)ctx
{
    CGContextSaveGState(ctx);

    CGRect bb = CGContextGetClipBoundingBox(ctx);

    // NSLog(@"%.0fx%.0f@(%.0f,%.0f) %f", bb.size.width, bb.size.height, bb.origin.x, bb.origin.y, 1/[(View *) [self superview] zoomScale]);

    CGContextTranslateCTM(ctx, bb.origin.x, bb.origin.y);

    NSLog(@"tile:%.0fx%.0f at:(%.0f,%.0f) size:%.0fx%.0f", bb.size.width, bb.size.height, bb.origin.x/self.scale, bb.origin.y/self.scale, bb.size.width/self.scale, bb.size.height/self.scale);

    touch_lo_draw_tile(ctx,
                       bb.size.width, bb.size.height,
                       CGPointMake(bb.origin.x/self.scale, bb.origin.y/self.scale),
                       CGSizeMake(bb.size.width/self.scale, bb.size.height/self.scale));

    CGContextRestoreGState(ctx);
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
