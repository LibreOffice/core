// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOSelectionHandle.h"
#import "MLOMainViewController_Friend.h"
#import "MLOSelectionViewController_Impl.h"
#import "MLOGestureEngine_Impl.h"
#import "MLOResourceImage.h"

@interface MLOSelectionHandle ()
@property MLOSelectionViewController * selection;
@property CGPoint lineTop, pivot;
@property BOOL tracking;
@end

static const CGFloat LINE_HEIGHT = 20.0f,
    LINE_WIDTH = 2.0f,
    LINE_TO_HANDLE_SPACING=7.0f,
    LINE_THICKNESS_PADDING = 3.0f,
    HANDLE_THICKNESS_PADDING = 15.0f,
    EXTERNAL_PADDING = 3.0f,
    SIDE_PADDING = 15.0f,
    VERTICAL_PADDING = HANDLE_THICKNESS_PADDING + LINE_THICKNESS_PADDING,
    HORIZONAL_PADDING = SIDE_PADDING * 2.0f;

@implementation MLOSelectionHandle

-(id)initWithType:(MLOSelectionHandleType) type selection:(MLOSelectionViewController *) selectionViewController{
    self = [self init];
    if(self){

        self.selection = selectionViewController;
        self.backgroundColor = [UIColor clearColor];

        MLOResourceImage * handleImage = [MLOResourceImage selectionHandle];

        CGFloat handleWidth = handleImage.image.size.width;
        CGFloat handleHeight = handleImage.image.size.height;
        CGFloat handleHeightWithSpacingToLine = LINE_TO_HANDLE_SPACING + handleHeight;
        CGFloat frameWidth = HORIZONAL_PADDING + handleWidth;

        CGFloat lineYOrigin;
        CGFloat handleYOrigin;
        CGFloat pivotY;

        switch (type) {
            case TOP_LEFT:
                handleYOrigin = HANDLE_THICKNESS_PADDING;
                pivotY = handleYOrigin + LINE_HEIGHT;
                lineYOrigin = pivotY + LINE_TO_HANDLE_SPACING;
                break;

            case BOTTOM_RIGHT:
                lineYOrigin = LINE_THICKNESS_PADDING;
                pivotY = lineYOrigin + LINE_HEIGHT;
                handleYOrigin = pivotY+ LINE_TO_HANDLE_SPACING;
                break;
        }

        self.frame = CGRectMake(0,
                                0,
                                frameWidth,
                                VERTICAL_PADDING + LINE_HEIGHT + handleHeightWithSpacingToLine);
        
        self.lineTop = CGPointMake(SIDE_PADDING + ((handleWidth - LINE_WIDTH) / 2.0f),
                                   lineYOrigin);
        
        self.pivot = CGPointMake(frameWidth/2.0f,
                                 pivotY);
        
        UIImageView * handle = [[UIImageView alloc] initWithImage:handleImage.image];
        
        [self addSubview: handle];
        handle.frame = CGRectMake(SIDE_PADDING,
                                  handleYOrigin,
                                  handleWidth,
                                  handleHeight);
        self.alpha = 0.0f;
        self.tracking =NO;
    }
    return self;
}
-(void)addToMainViewController{
    [_selection.mainViewController.canvas addSubview:self];


}
-(CGPoint) getPivot{
    return CGPointMake(self.frame.origin.x + _pivot.x, self.frame.origin.y + _pivot.y);
}
-(void)showAt:(CGPoint) edge{

    [self putAt:edge withAlpha:1.0f];
}

-(void)putAt:(CGPoint) center withAlpha:(CGFloat) alpha{
    self.frame = CGRectMake(center.x - _pivot.x,
                            center.y - _pivot.y,
                            self.frame.size.width,
                            self.frame.size.height);
    self.alpha = alpha;
}

-(void)hide{
    [self putAt:CGPointMake(0,0) withAlpha:0.0f];
}

-(void)onRotate{
    [self hide];
}
-(void)drawRect:(CGRect)rect{
    [super drawRect:rect];

    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextSetStrokeColorWithColor(context, [UIColor blueColor].CGColor);

    CGContextSetLineWidth(context, LINE_WIDTH);

    CGContextMoveToPoint(context, _lineTop.x,_lineTop.y); //start at this point

    CGContextAddLineToPoint(context, _lineTop.x, _lineTop.y + LINE_HEIGHT); //draw to this point

    CGContextStrokePath(context); // draw the path
}

-(BOOL)handleMoveAtPoint:(CGPoint) center gestureState:(UIGestureRecognizerState) state{
    if((self.alpha==0.0f)||
       !CGRectContainsPoint(self.frame, center)){

        [self endTracking];

        return NO;
    }

    if (state == UIGestureRecognizerStateChanged) {
        self.tracking = YES;
        [self showAt:center];

        _selection.finger = center;

        [_selection reselect:NO];

    }else if((state == UIGestureRecognizerStateEnded) ||
             (state == UIGestureRecognizerStateCancelled) ||
             (state == UIGestureRecognizerStateFailed)){

        _selection.finger = center;

        [self endTracking];
    }
    return YES;
}

-(void)endTracking{
    if(self.tracking){
        self.tracking=NO;

        [_selection reselect:YES];

    }
}
@end

