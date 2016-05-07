// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <CoreText/CoreText.h>

#include <LibreOfficeKit/LibreOfficeKit.h>

#import "View.h"
#import "TiledView.h"

@interface TiledView ()

@property CGFloat scale;

@end

@implementation TiledView

static const int NTIMESTAMPS = 100;
static const CFTimeInterval AVERAGINGTIME = 5;

static struct {
    CFTimeInterval timestamp;
    int count;
} tileTimestamps[NTIMESTAMPS];
static int oldestTimestampIndex = 0;
static int nextTimestampIndex = 0;

static LibreOfficeKitDocument* loDocument;

static void dropOldTimestamps(CFTimeInterval now)
{
    // Drop too old timestamps
    while (oldestTimestampIndex != nextTimestampIndex && now - tileTimestamps[oldestTimestampIndex].timestamp >= AVERAGINGTIME)
        oldestTimestampIndex = (oldestTimestampIndex + 1) % NTIMESTAMPS;
}

static void updateTilesPerSecond(UILabel *label)
{
    int n = 0;

    for (int k = oldestTimestampIndex; k != nextTimestampIndex; k = (k + 1) % NTIMESTAMPS)
        n += tileTimestamps[k].count;

    // NSLog(@"oldest:%d next:%d n:%d", oldestTimestampIndex, nextTimestampIndex, n);

    double tps = n/AVERAGINGTIME;

    [label setText:[NSString stringWithFormat:@"%.0f tiles/second", tps]];
}

- (void)didRenderTile
{
    CFTimeInterval now = CACurrentMediaTime();

    @synchronized(self) {
        dropOldTimestamps(now);

        // Add new timestamp
        tileTimestamps[nextTimestampIndex].timestamp = now;
        tileTimestamps[nextTimestampIndex].count++;
        // Let next added replace newest if array full
        if (oldestTimestampIndex != (nextTimestampIndex + 1) % NTIMESTAMPS) {
            nextTimestampIndex = (nextTimestampIndex + 1) % NTIMESTAMPS;
            tileTimestamps[nextTimestampIndex].count = 0;
        }

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

static int DBG_DRAW_DELAY = 10;
int DBG_DRAW_ROUNDS = -1;
int DBG_DRAW_COUNTER = 0;
int DBG_DRAW_ROUNDS_MAX = INT_MAX;

- (void)redraw
{
    DBG_DRAW_ROUNDS++;
    DBG_DRAW_COUNTER = 0;
    [self setNeedsDisplay];
    if (DBG_DRAW_ROUNDS < DBG_DRAW_ROUNDS_MAX)
        [NSTimer scheduledTimerWithTimeInterval:DBG_DRAW_DELAY target:self selector:@selector(redraw) userInfo:nil repeats:NO];
}

- (id)initWithFrame:(CGRect)frame scale:(CGFloat)scale maxZoom:(int)maxZoom document:(LibreOfficeKitDocument*)document
{
    loDocument = document;

    self = [super initWithFrame:frame];
    if (self) {
        self.scale = scale;
        CATiledLayer *catl = (CATiledLayer*) [self layer];
        catl.tileSize = CGSizeMake(512, 512);
        catl.levelsOfDetail = log2(maxZoom) + 1;
        catl.levelsOfDetailBias = catl.levelsOfDetail - 1;

        [NSTimer scheduledTimerWithTimeInterval:1 target:self selector:@selector(updateTilesPerSecond) userInfo:nil repeats:YES];
        if (getenv("DRAW_INCREMENTALLY_FROM")) {
            DBG_DRAW_ROUNDS = atoi(getenv("DRAW_INCREMENTALLY_FROM"));
            if (getenv("DRAW_INCREMENTALLY_DELAY") &&
                atoi(getenv("DRAW_INCREMENTALLY_DELAY")) > 1)
                DBG_DRAW_DELAY = atoi(getenv("DRAW_INCREMENTALLY_DELAY"));
            [NSTimer scheduledTimerWithTimeInterval:DBG_DRAW_DELAY target:self selector:@selector(redraw) userInfo:nil repeats:NO];
        }
    }
    return self;
}

+ (Class)layerClass
{
    return [CATiledLayer class];
}

static bool tileMatches(const char *spec, CGRect bb)
{
    int x, y;

    return (sscanf(spec, "%d,%d", &x, &y) == 2 &&
            x == (int) (bb.origin.x / bb.size.width) &&
            y == (int) (bb.origin.y / bb.size.height));
}

extern void PtylTestEncryptionAndExport(const char *pathname);
extern const char *ptyl_test_encryption_pathname;

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)ctx
{
    // Even if I set the CATL's tileSize to 512x512 above, this is
    // called initially with a clip bbox of 128x128. Odd, I would have
    // expected it to be called with a bbox of 256x256.

    static bool once = false;
    if (!once) {
        once = true;
        if (getenv("PTYL_TEST_ENCRYPTION_AND_EXPORT"))
            PtylTestEncryptionAndExport(ptyl_test_encryption_pathname);
    }

    CGRect bb = CGContextGetClipBoundingBox(ctx);

    CGContextSaveGState(ctx);

    CGContextTranslateCTM(ctx, bb.origin.x, bb.origin.y);

    // CGSize tileSize = [catl tileSize];
    CGSize tileSize = bb.size;

    NSLog(@"bb:%.0fx%.0f@(%.0f,%.0f) zoomScale:%.0f tile:%.0fx%.0f at:(%.0f,%.0f) size:%.0fx%.0f", bb.size.width, bb.size.height, bb.origin.x, bb.origin.y, [((View *) [self superview]) zoomScale], tileSize.width, tileSize.height, bb.origin.x/self.scale, bb.origin.y/self.scale, bb.size.width/self.scale, bb.size.height/self.scale);

    if (!getenv("DRAW_ONLY_TILE") || tileMatches(getenv("DRAW_ONLY_TILE"), bb)) {
        fprintf(stderr, "+++ rendering to context %p\n", ctx);
        loDocument->pClass->paintTile(loDocument, (unsigned char *)ctx,
                                      tileSize.width, tileSize.height,
                                      bb.origin.x/self.scale, bb.origin.y/self.scale,
                                      bb.size.width/self.scale, bb.size.height/self.scale);
    } else {
        CGContextSetRGBFillColor(ctx, 1, 1, 1, 1);
        CGContextFillRect(ctx, CGRectMake(0, 0, bb.size.width, bb.size.height));
    }

    [self didRenderTile];

    CGContextRestoreGState(ctx);

    if (getenv("DRAW_TILE_BORDERS")) {
        // I am a bit confused about what tiles exactly I am drawing, so
        // make it perfectly obvious by drawing borders around the tiles
        CGContextSaveGState(ctx);
        CGContextSetStrokeColorWithColor(ctx, [[UIColor colorWithRed:1 green:0 blue:0 alpha:0.5] CGColor]);
        CGContextSetLineWidth(ctx, 1);
        CGContextStrokeRect(ctx, bb);
        CGContextRestoreGState(ctx);
    }

    if (getenv("DRAW_TILE_NUMBERS")) {
        // Also draw the coordinates of the tile;)
        CGContextSaveGState(ctx);
        float scale = 1/[((View *) [self superview]) zoomScale];
        NSString *s = [NSString stringWithFormat:@"%d,%d", (int) (bb.origin.x / bb.size.width), (int) (bb.origin.y / bb.size.height)];
        CFAttributedStringRef as = CFAttributedStringCreate(NULL, (__bridge CFStringRef)(s), NULL);
        CTLineRef l = CTLineCreateWithAttributedString(as);
        CGContextTranslateCTM(ctx, bb.origin.x, bb.origin.y);
        CGContextScaleCTM(ctx, scale, scale);
        CGContextSetTextPosition(ctx, 2, 12);
        CGContextSetTextMatrix(ctx, CGAffineTransformScale(CGContextGetTextMatrix(ctx), 1, -1));
        CTLineDraw(l, ctx);
        CGContextRestoreGState(ctx);
    }
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
