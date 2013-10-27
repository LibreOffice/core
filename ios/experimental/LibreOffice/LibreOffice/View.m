// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "View.h"

#include <touch/touch.h>

@interface View ()
@property const void *documentHandle;
@property CGRect *selectionRectangles;
@property int selectionRectangleCount;
@end

#define HANDLE_BLOB 20
#define DRAG_RADIUS (HANDLE_BLOB + 20)
#define HANDLE_STEM_WIDTH 4
#define HANDLE_STEM_HEIGHT 10

#define SQUARE(n) ((n)*(n))

@implementation View

- (CGRect) topLeftResizeHandle
{
    if (self.selectionRectangleCount == 0)
        return CGRectNull;

    return CGRectMake(self.selectionRectangles[0].origin.x - HANDLE_STEM_WIDTH/2 - HANDLE_BLOB/2,
                      self.selectionRectangles[0].origin.y - HANDLE_STEM_HEIGHT - HANDLE_BLOB,
                      HANDLE_BLOB, HANDLE_BLOB);
}

- (CGRect) bottomRightResizeHandle
{
    const int N = self.selectionRectangleCount;

    if (N == 0)
        return CGRectNull;

    return CGRectMake(self.selectionRectangles[N-1].origin.x +
                      self.selectionRectangles[N-1].size.width + HANDLE_STEM_WIDTH/2 - HANDLE_BLOB/2,
                      self.selectionRectangles[N-1].origin.y +
                      self.selectionRectangles[N-1].size.height + HANDLE_STEM_HEIGHT,
                      HANDLE_BLOB, HANDLE_BLOB);
}

- (bool) topLeftResizeHandleIsCloseTo:(CGPoint)position
{
    return ((SQUARE((self.selectionRectangles[0].origin.x - HANDLE_STEM_WIDTH/2) - position.x) +
             SQUARE((self.selectionRectangles[0].origin.y - HANDLE_STEM_HEIGHT/2 - HANDLE_BLOB/2) - position.y)) <
            SQUARE(DRAG_RADIUS));
}

- (bool) bottomRightResizeHandleIsCloseTo:(CGPoint)position
{
    const int N = self.selectionRectangleCount;

    return ((SQUARE((self.selectionRectangles[N-1].origin.x +
                     self.selectionRectangles[N-1].size.width + HANDLE_STEM_WIDTH/2) - position.x) +
             SQUARE((self.selectionRectangles[N-1].origin.y +
                     self.selectionRectangles[N-1].size.height + HANDLE_STEM_HEIGHT/2 + HANDLE_BLOB/2) - position.y)) <
            SQUARE(DRAG_RADIUS));
}

- (void) requestSelectionRedisplay
{
    if (self.selectionRectangleCount == 0)
        return;

    CGRect r = CGRectNull;
    for (int i = 0; i < self.selectionRectangleCount; i++) {
        r = CGRectUnion(r, self.selectionRectangles[i]);
    }
    r = CGRectUnion(r, [self topLeftResizeHandle]);
    r = CGRectUnion(r, [self bottomRightResizeHandle]);

    [self setNeedsDisplayInRect:r];
}

- (void) drawSelectionIntoContext:(CGContextRef)context
{
    if (self.selectionRectangleCount == 0)
        return;

    const int N = self.selectionRectangleCount;

    CGContextSetFillColorWithColor(context, [[UIColor colorWithRed:0 green:0 blue:1 alpha:0.5] CGColor]);

    CGContextSetBlendMode(context, kCGBlendModeNormal);
    CGContextFillRects(context, self.selectionRectangles, self.selectionRectangleCount);

    CGContextFillRect(context,
                      CGRectMake(self.selectionRectangles[0].origin.x - HANDLE_STEM_WIDTH,
                                 self.selectionRectangles[0].origin.y - HANDLE_STEM_HEIGHT,
                                 HANDLE_STEM_WIDTH, self.selectionRectangles[0].size.height + HANDLE_STEM_HEIGHT));

    CGContextFillRect(context,
                      CGRectMake(self.selectionRectangles[N-1].origin.x +
                                 self.selectionRectangles[N-1].size.width,
                                 self.selectionRectangles[N-1].origin.y,
                                 HANDLE_STEM_WIDTH, self.selectionRectangles[N-1].size.height + HANDLE_STEM_HEIGHT));

    CGContextSetFillColorWithColor(context, [[UIColor colorWithRed:0 green:0 blue:1 alpha:0.8] CGColor]);

    CGContextFillEllipseInRect(context, [self topLeftResizeHandle]);
    CGContextFillEllipseInRect(context, [self bottomRightResizeHandle]);
}

- (void)drawRect:(CGRect)rect
{
    // NSLog(@"View drawRect: %dx%d@(%d,%d)", (int) rect.size.width, (int) rect.size.height, (int) rect.origin.x, (int) rect.origin.y);
    // NSLog(@"statusBarOrientation: %ld", (long)[[UIApplication sharedApplication] statusBarOrientation]);

    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSaveGState(context);

    switch ([[UIApplication sharedApplication] statusBarOrientation]) {
    case UIInterfaceOrientationPortrait:
        CGContextTranslateCTM(context, 0, self.frame.size.height);
        CGContextScaleCTM(context, 1, -1);
        break;
    case UIInterfaceOrientationLandscapeLeft:
        CGContextTranslateCTM(context, 0, self.frame.size.width);
        CGContextScaleCTM(context, 1, -1);
        break;
    case UIInterfaceOrientationLandscapeRight:
        CGContextTranslateCTM(context, 0, self.frame.size.width);
        CGContextScaleCTM(context, 1, -1);
        break;
    case UIInterfaceOrientationPortraitUpsideDown:
        CGContextTranslateCTM(context, 0, self.frame.size.height);
        CGContextScaleCTM(context, 1, -1);
        break;
    }
    touch_lo_render_windows(context, rect.origin.y, rect.origin.y, rect.size.width, rect.size.height);

    CGContextRestoreGState(context);

    [self drawSelectionIntoContext:context];
}

#if 0
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    NSLog(@"===> View touchesBegan!");
}
#endif

- (void)tapGesture:(UITapGestureRecognizer *)gestureRecognizer
{
    if ([gestureRecognizer state] == UIGestureRecognizerStateEnded) {
        CGPoint location = [gestureRecognizer locationInView:self];

        // NSLog(@"tapGesture: at: (%d,%d)", (int)location.x, (int)location.y);

        touch_lo_tap(location.x, location.y);
    } else {
        // NSLog(@"tapGesture: %@", gestureRecognizer);
    }
}

- (void)panGesture:(UIPanGestureRecognizer *)gestureRecognizer
{
    const int N = self.selectionRectangleCount;

    static enum { NONE, TOPLEFT, BOTTOMRIGHT } draggedHandle = NONE;
    static CGPoint previous;
    static CGPoint dragOffset;

    CGPoint location = [gestureRecognizer locationInView:self];
    CGPoint translation = [gestureRecognizer translationInView:self];

    if (gestureRecognizer.state == UIGestureRecognizerStateBegan) {
        previous = CGPointMake(0, 0);
    }

    CGPoint delta;
    delta.x = translation.x - previous.x;
    delta.y = translation.y - previous.y;

    // NSLog(@"location: (%f,%f) , drag: (%f,%f)", location.x, location.y, delta.x, delta.y);

    previous = translation;

    if (gestureRecognizer.state == UIGestureRecognizerStateBegan &&
        gestureRecognizer.numberOfTouches == 1) {
        if ([self topLeftResizeHandleIsCloseTo:location]) {
            NSLog(@"===> dragging TOPLEFT handle");
            draggedHandle = TOPLEFT;
            dragOffset.x = location.x - self.selectionRectangles[0].origin.x;
            dragOffset.y = location.y - self.selectionRectangles[0].origin.y;
        } else if ([self bottomRightResizeHandleIsCloseTo:location]) {
            NSLog(@"===> dragging BOTTOMRIGHT handle");
            draggedHandle = BOTTOMRIGHT;
            dragOffset.x = location.x - self.selectionRectangles[N-1].origin.x;
            dragOffset.y = location.y - self.selectionRectangles[N-1].origin.y;
        }
    }

    if (draggedHandle == TOPLEFT) {

        touch_lo_selection_start_move(self.documentHandle,
                                      location.x - dragOffset.x, location.y - dragOffset.y);

        if (gestureRecognizer.state == UIGestureRecognizerStateEnded)
            draggedHandle = NONE;

        return;
    } else if (draggedHandle == BOTTOMRIGHT) {

        touch_lo_selection_end_move(self.documentHandle,
                                    location.x - dragOffset.x, location.y - dragOffset.y);

        if (gestureRecognizer.state == UIGestureRecognizerStateEnded)
            draggedHandle = NONE;

        return;
    }

    if (gestureRecognizer.state != UIGestureRecognizerStateBegan) {
        touch_lo_pan(delta.x, delta.y);
    }
}

- (void)pinchGesture:(UIPinchGestureRecognizer *)gestureRecognizer
{
    CGPoint location = [gestureRecognizer locationInView:self];
    CGFloat scale = gestureRecognizer.scale;

    // NSLog(@"pinchGesture: pinch: (%f) cords (%d,%d)", (float)scale, (int)location.x, (int)location.y );

    touch_lo_zoom((int)location.x, (int)location.y, (float)scale);

    // to reset the gesture scaling
    if (gestureRecognizer.state == UIGestureRecognizerStateEnded) {
        touch_lo_zoom(1, 1, 0.0f);
    }
}

- (void)longPressGesture:(UILongPressGestureRecognizer *)gestureRecognizer
{
    CGPoint point = [gestureRecognizer locationInView:self];

    if (gestureRecognizer.state == UIGestureRecognizerStateEnded) {
        touch_lo_tap(point.x, point.y);
        touch_lo_tap(point.x, point.y);
    }
}

static int compare_rects(const void *a, const void *b)
{
    const CGRect *ra = a;
    const CGRect *rb = b;

    if (ra->origin.y != rb->origin.y)
        return ra->origin.y - rb->origin.y;
    else
        return ra->origin.x - rb->origin.x;
}

- (void)startSelectionOfType:(MLOSelectionKind)kind withNumber:(int)number ofRectangles:(CGRect *)rects forDocument:(const void *)document
{
    (void) kind;

    // First request the old selection area to be redisplayed
    [self requestSelectionRedisplay];

    free(self.selectionRectangles);
    self.selectionRectangles = NULL;
    self.selectionRectangleCount = 0;
    self.documentHandle = NULL;

    if (number == 0)
        return;

    self.selectionRectangles = rects;
    self.selectionRectangleCount = number;
    self.documentHandle = document;

    // The selection rectangle provided by LO are not sorted in any sane way
    qsort(self.selectionRectangles, self.selectionRectangleCount, sizeof(self.selectionRectangles[0]), compare_rects);

    [self requestSelectionRedisplay];
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
