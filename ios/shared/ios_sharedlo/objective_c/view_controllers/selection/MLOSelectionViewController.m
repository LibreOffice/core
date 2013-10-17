// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOMainViewController_Friend.h"
#import "MLOSelectionViewController_Impl.h"
#import "MLOContextualMenuFocus.h"
#import "MLOSelectionHandle.h"
#import "MLOMagnifier.h"
#import "MLORenderManager.h"
#import "MLOThesaurusViewController.h"
#import "mlo_utils.h"
#import "mlo_uno.h"
#include <touch/touch.h>

@interface MLOSelectionViewController ()
@property MLOContextualMenuFocus * focus;
@property MLOSelectionHandle * topLeft, * bottomRight;
@property MLOMagnifier * magnifier;
@property BOOL menuShown,magnifierShown;
@end

static const CGFloat FOCUS_DIM=20.0f;

@implementation MLOSelectionViewController

-(id) initWithMainViewController:(MLOMainViewController *)mainViewController{
    self = [super init];
    if(self){
        self.mainViewController = mainViewController;
        self.buffer = [NSMutableString stringWithString:@""];
        self.focus = [[MLOContextualMenuFocus alloc] initWithSelectionViewController:self];
        self.topLeft = [[MLOSelectionHandle alloc] initWithType: TOP_LEFT selection:self];
        self.bottomRight = [[MLOSelectionHandle alloc] initWithType: BOTTOM_RIGHT selection:self];
        self.magnifier = [[MLOMagnifier alloc] initWithMainViewController:mainViewController];
        self.menuShown =NO;
        self.magnifierShown =NO;
        self.finger = CGPointMake(0,0);
    }
    
    return self;
}
-(void) addToMainViewController{
    [_mainViewController.canvas addSubview:_focus];
    [_topLeft addToMainViewController];
    [_bottomRight addToMainViewController];
    [_magnifier addToMainViewController];
}

-(void) reset{
    [self hide:NO];
}
-(UIMenuItem *)copyItem{
    return [[UIMenuItem alloc]
            initWithTitle:  @"Copy"
            action:         @selector(loCopyToClipboard:)];
}
-(UIMenuItem *)selectAllItem{
    return [[UIMenuItem alloc]
            initWithTitle: @"Select All"
            action:        @selector(loSelectAll:)];
}

-(UIMenuItem *)defineItem{
    return [[UIMenuItem alloc]
            initWithTitle:  @"Define"
            action:         @selector(defineBuffer:)];
}

-(void) showMenuAtCenter:(CGPoint)center{
    [self showAtCenter:center selectAll:YES];
}

-(void) showPostSelectAll{

    [self hideTouchTrackers];
    [self showAtCenter:CGPointMake(_mainViewController.canvas.frame.size.width / 2.0f, FOCUS_DIM) selectAll:NO];
}

-(void) showAtTopLeft:(CGPoint) topLeft bottomRight:(CGPoint) bottomRight showMenu:(BOOL) showMenu{
    [_topLeft showAt:topLeft];
    [_bottomRight showAt:bottomRight];
    if(showMenu){
        [self hideMagnifier];
        [self showMenuAtCenter:CGPointMake( (bottomRight.x + bottomRight.x) /2.0f,
                                    (bottomRight.y + bottomRight.y) /2.0f)];
        self.menuShown = YES;
    }else{
        [self hideMenu:YES];
        [_magnifier showAt:self.finger];
        self.magnifierShown=YES;
    }
    
    [_mainViewController.renderManager renderNow];
}

-(void) showBetweenFinger:(CGPoint) finger andPreviousPoint:(CGPoint) previousPoint showMenu:(BOOL) showMenu{
    self.finger = finger;
    [self showBetweenPointA:finger andPointB:previousPoint showMenu:showMenu];
}

-(void) showBetweenPointA:(CGPoint) pointA andPointB:(CGPoint) pointB showMenu:(BOOL) showMenu{

    if((pointA.y < pointB.y) ||
       ((pointA.y == pointB.y) && (pointA.x < pointB.x))){
        
        [self showAtTopLeft:pointA bottomRight:pointB showMenu:showMenu];
    }else{
        [self showAtTopLeft:pointB bottomRight:pointA showMenu:showMenu];
    }
}

-(void)showAtCenter:(CGPoint) center selectAll:(BOOL) isSelectAll{
    
    CGFloat width = _mainViewController.canvas.frame.size.width;
    CGFloat height = _mainViewController.canvas.frame.size.height;

    self.focus.frame = CGRectMake( min(max(0,center.x - FOCUS_DIM),width - FOCUS_DIM),
                             min(max(0,center.y - FOCUS_DIM),height - FOCUS_DIM),
                             FOCUS_DIM,
                             FOCUS_DIM);

    [self.focus becomeFirstResponder];

    UIMenuController *menuController = [UIMenuController sharedMenuController];

    [menuController setMenuItems:[self getMenuItems:isSelectAll]];
    
    [menuController setTargetRect:self.focus.frame
                           inView:_mainViewController.canvas];
    [menuController setMenuVisible:YES animated:YES];
}

-(NSArray *)getMenuItems:(BOOL) isSelectAll{

    mlo_get_selection(_buffer);

    NSLog(@"Copied LO selection to keyboard: %@",_buffer);

    if(isSelectAll){

        if([UIReferenceLibraryViewController dictionaryHasDefinitionForTerm:_buffer]){
            NSLog(@"Creating contextual menu with Copy, Select All, and Define");

            return @[[self copyItem], [self selectAllItem], [self defineItem]];
            
        }
         NSLog(@"Creating contextual menu with Copy and Select All");
        
         return @[[self copyItem],[self selectAllItem]];
    }
    
    NSLog(@"Creating contextual menu with Copy only");

    return @[[self copyItem]];
 }

-(NSString *)description{
    return @"MLO contextual menu view controller";
}

-(void) hide{
    [self hide:YES];
}

-(void) hideTouchTrackers{
    [_topLeft hide];
    [_bottomRight hide];
    [self hideMagnifier];
}

-(void) hideMenu:(BOOL) animate{
    if(self.menuShown){
        self.menuShown =NO;
        [[UIMenuController sharedMenuController] setMenuVisible:NO animated:animate];
    }
}

-(void) hideMagnifier{
    if(self.magnifierShown){
        self.magnifierShown =NO;
        [_magnifier hide];
    }
}

-(void) hide:(BOOL) animate{

    [self hideTouchTrackers];

    [self hideMenu:animate];
    
    [self.focus resignFirstResponder];
    self.focus.frame =CGRECT_ZERO;
}

-(void)reselect:(BOOL) showMenu{

    CGPoint pointA = [_topLeft getPivot];
    CGPoint pointB = [_bottomRight getPivot];

    touch_lo_mouse_drag(pointA.x, pointA.y, DOWN);

    touch_lo_mouse_drag(pointB.x, pointB.y, MOVE);

    touch_lo_mouse_drag(pointB.x, pointB.y, UP);

    [self showBetweenPointA:pointA andPointB:pointB showMenu:showMenu];
}

-(void) onRotate{
    [self reset];

}

-(BOOL)handleMoveAtPoint:(CGPoint) center gestureState:(UIGestureRecognizerState) state{
    return  (self.focus.frame.origin.x != 0) &&
            ([_topLeft handleMoveAtPoint:center gestureState:state] ||
            [_bottomRight handleMoveAtPoint:center gestureState:state]);
}


-(void)defineBuffer{
    [[[MLOThesaurusViewController alloc] initWithSelectionViewController:self] show];
    
}

@end
