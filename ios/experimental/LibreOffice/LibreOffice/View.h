// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//

#import <UIKit/UIKit.h>

#import <touch/touch.h>

@interface View : UIView
{
@public
    UITextView* textView;
}
- (void)drawRect:(CGRect)rect;
- (void)tapGesture:(UITapGestureRecognizer *)gestureRecognizer;
- (void)panGesture:(UIPanGestureRecognizer *)gestureRecognizer;
- (void)longPressGesture:(UILongPressGestureRecognizer *)gestureRecognizer;
- (void)startSelectionOfType:(MLOSelectionKind)kind withNumber:(int)number ofRectangles:(CGRect *)rects forDocument:(const void *)document;

@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
