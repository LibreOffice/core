// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "View.h"

#include <osl/detail/ios-bootstrap.h>

@implementation View

- (void)drawRect:(CGRect)rect
{
    NSLog(@"drawRect: %dx%d@(%d,%d)", (int) rect.size.width, (int) rect.size.height, (int) rect.origin.x, (int) rect.origin.y);

    NSDate *startDate = [NSDate date];

    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSaveGState(context);
    CGContextTranslateCTM(context, 0, self.frame.size.height);
    CGContextScaleCTM(context, 1, -1);
    lo_render_windows(context, rect);
    CGContextRestoreGState(context);

    NSLog(@"drawRect: lo_render_windows took %f s", [[NSDate date] timeIntervalSinceDate: startDate]);
}

- (void) tapGesture:(UIGestureRecognizer *)gestureRecognizer
{
    if ([gestureRecognizer state] == UIGestureRecognizerStateEnded) {
        CGPoint location = [gestureRecognizer locationInView: self];
        NSLog(@"tapGesture: at: (%d,%d)", (int)location.x, (int)location.y);
        lo_tap(location.x, location.y);
    } else
        NSLog(@"tapGesture: %@", gestureRecognizer);
}

- (void)insertText:(NSString *)text
{
    (void) text;
    // Do something with the typed character
}

- (void)deleteBackward
{
    // Handle the delete key
}

- (BOOL)hasText
{
    // Return whether there's any text present
    return YES;
}

- (BOOL)canBecomeFirstResponder
{
    return YES;
}


@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
