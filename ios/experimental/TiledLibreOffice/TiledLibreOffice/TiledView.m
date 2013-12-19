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

static const int NTIMESTAMPS = 100;
static const CFTimeInterval AVERAGINGTIME = 5;

static CFTimeInterval tileTimestamps[NTIMESTAMPS];
static int curFirstTimestamp = 0;
static int curNextTimestamp = 0;

static void dropOldTimestamps(CFTimeInterval now)
{
    // Drop too old timestamps
    while (curFirstTimestamp != curNextTimestamp && now - tileTimestamps[curFirstTimestamp] >= AVERAGINGTIME)
        curFirstTimestamp = (curFirstTimestamp + 1) % NTIMESTAMPS;
}

static void updateTilesPerSecond(UILabel *label)
{
    int n = (curNextTimestamp < curFirstTimestamp) ?
        (NTIMESTAMPS - (curFirstTimestamp - curNextTimestamp))
        : ((curNextTimestamp - curFirstTimestamp));

    // NSLog(@"first:%d next:%d n:%d", curFirstTimestamp, curNextTimestamp, n);

    double tps = n/AVERAGINGTIME;

    [label setText:[NSString stringWithFormat:@"%.0f tiles/second", tps]];
}

- (void)didRenderTile
{
    CFTimeInterval now = CACurrentMediaTime();

    @synchronized(self) {
        dropOldTimestamps(now);

        // Add new timestamp
        tileTimestamps[curNextTimestamp] = now;
        // Let next added replace newest if array full
        if (curFirstTimestamp != (curNextTimestamp + 1) % NTIMESTAMPS)
            curNextTimestamp = (curNextTimestamp + 1) % NTIMESTAMPS;

        updateTilesPerSecond(((View *) [self superview]).tpsLabel);
    }
}

- (void)updateTilesPerSecond
{
    CFTimeInterval now = CACurrentMediaTime();

    @synchronized(self) {
        dropOldTimestamps(now);
        updateTilesPerSecond(((View *) [self superview]).tpsLabel);
    }
}

- (id)initWithFrame:(CGRect)frame scale:(CGFloat)scale maxZoom:(int)maxZoom
{
    self = [super initWithFrame:frame];
    if (self) {
        self.scale = scale;
        CATiledLayer *catl = (CATiledLayer*) [self layer];
        catl.tileSize = CGSizeMake(512, 512);
        catl.levelsOfDetail = log2(maxZoom) + 1;
        catl.levelsOfDetailBias = catl.levelsOfDetail - 1;

        [NSTimer scheduledTimerWithTimeInterval:1 target:self selector:@selector(updateTilesPerSecond) userInfo:nil repeats:YES];
    }
    return self;
}

+ (Class)layerClass
{
    return [CATiledLayer class];
}

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)ctx
{
    // Even if I set the CATL's tileSize to 512x512 above, this is
    // called initially with a clip bbox of 128x128. Odd, I would have
    // expected it to be called with a bbox of 256x256.

    CGRect bb = CGContextGetClipBoundingBox(ctx);

    // double zoomScale = [(View *) [self superview] zoomScale];
    // CATiledLayer *catl = (CATiledLayer*) [self layer];

    CGContextSaveGState(ctx);

    CGContextTranslateCTM(ctx, bb.origin.x, bb.origin.y);
    // CGContextScaleCTM(ctx, 1/zoomScale, 1/zoomScale);

    // CGSize tileSize = [catl tileSize];
    CGSize tileSize = bb.size;

    // NSLog(@"bb:%.0fx%.0f@(%.0f,%.0f) zoomScale:%.0f tile:%.0fx%.0f at:(%.0f,%.0f) size:%.0fx%.0f", bb.size.width, bb.size.height, bb.origin.x, bb.origin.y, zoomScale, tileSize.width, tileSize.height, bb.origin.x/self.scale, bb.origin.y/self.scale, bb.size.width/self.scale, bb.size.height/self.scale);

    // I don't really claim to fully understand all this. It does seem
    // a bit weird to be passing in a "context width x height" (in the
    // terminology of touch_lo_draw_tile) of 64x64, for instance, even
    // if that tile is actually going to be rendered to 128x128 actual
    // pixels. But this seems to work. Other combinations, applying
    // scaling to the CTM, etc, don't. But maybe I haven't tried hard
    // enough.

    touch_lo_draw_tile(ctx,
                       tileSize.width, tileSize.height,
                       CGPointMake(bb.origin.x/self.scale, bb.origin.y/self.scale),
                       CGSizeMake(bb.size.width/self.scale, bb.size.height/self.scale));

    [self didRenderTile];

    CGContextRestoreGState(ctx);

    // I am a bit confused about what tiles exactly I am drawing, so
    // make it perfectly obvious by drawing borders around the tiles
    CGContextSaveGState(ctx);
    CGContextSetStrokeColorWithColor(ctx, [[UIColor colorWithRed:1 green:0 blue:0 alpha:0.5] CGColor]);
    CGContextSetLineWidth(ctx, 1);
    CGContextStrokeRect(ctx, bb);
    CGContextRestoreGState(ctx);
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
