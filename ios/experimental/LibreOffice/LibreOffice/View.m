// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "View.h"

#include <touch/touch.h>

@implementation View

- (void)drawRect:(CGRect)rect
{
    // NSLog(@"drawRect: %dx%d@(%d,%d)", (int) rect.size.width, (int) rect.size.height, (int) rect.origin.x, (int) rect.origin.y);
    // NSLog(@"statusBarOrientation: %ld", (long)[[UIApplication sharedApplication] statusBarOrientation]);

    // NSDate *startDate = [NSDate date];

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

    // NSLog(@"drawRect: touch_lo_render_windows took %f s", [[NSDate date] timeIntervalSinceDate: startDate]);
}

- (void)tapGesture:(UITapGestureRecognizer *)gestureRecognizer
{
    if ([gestureRecognizer state] == UIGestureRecognizerStateEnded) {
        CGPoint location = [gestureRecognizer locationInView: self];

        // NSLog(@"tapGesture: at: (%d,%d)", (int)location.x, (int)location.y);

        touch_lo_tap(location.x, location.y);

        [self->textView becomeFirstResponder];
    } else {
        // NSLog(@"tapGesture: %@", gestureRecognizer);
    }
}

- (void)panGesture:(UIPanGestureRecognizer *)gestureRecognizer
{
    static CGFloat previousX = 0.0f, previousY = 0.0f;

    CGPoint translation = [gestureRecognizer translationInView: self];

    if (gestureRecognizer.state != UIGestureRecognizerStateBegan) {
        int deltaX = translation.x - previousX;
        int deltaY = translation.y - previousY;

        // NSLog(@"panGesture: pan (delta): (%d,%d)", deltaX, deltaY);

        touch_lo_pan(deltaX, deltaY);
    }

    previousX = translation.x;
    previousY = translation.y;
}

- (void)pinchGesture:(UIPinchGestureRecognizer *)gestureRecognizer
{
    CGPoint location = [gestureRecognizer locationInView: self];
    CGFloat scale = gestureRecognizer.scale;

    // NSLog(@"pinchGesture: pinch: (%f) cords (%d,%d)", (float)scale, (int)location.x, (int)location.y );

    touch_lo_zoom((int)location.x, (int)location.y, (float)scale);

    // to reset the gesture scaling
    if (gestureRecognizer.state==UIGestureRecognizerStateEnded) {
        touch_lo_zoom(1, 1, 0.0f);
    }
}

- (void)longPressGesture:(UILongPressGestureRecognizer *)gestureRecognizer
{
    CGPoint point = [gestureRecognizer locationInView:self];

    UIGestureRecognizerState state = gestureRecognizer.state;

    // NSLog(@"longPressGesture: state %d cords (%d,%d)",state ,(int)point.x,(int)point.y);

    if (state == UIGestureRecognizerStateEnded) {
        touch_lo_tap(point.x, point.y);
        touch_lo_tap(point.x, point.y);
    }
}

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
