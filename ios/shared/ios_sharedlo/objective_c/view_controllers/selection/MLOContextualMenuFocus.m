// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOCommon.h"
#import "MLOContextualMenuFocus.h"
#import "MLOSelectionViewController_Impl.h"
#import "mlo_uno.h"

@interface MLOContextualMenuFocus ()
@property MLOSelectionViewController * selectionViewController;
@end
@implementation MLOContextualMenuFocus

-(id)initWithSelectionViewController:(MLOSelectionViewController *) selectionViewController{
    self = [self init];
    if(self){
        self.selectionViewController = selectionViewController;
        self.frame = CGRECT_ZERO;
        self.backgroundColor = [UIColor clearColor];
    }
    return self;
}

-(void)loCopyToClipboard:(id) sender{
    [UIPasteboard generalPasteboard].string =[_selectionViewController.buffer copy];

}

-(void)loSelectAll:(id) sender{

    NSLog(@"Calling mlo_select_all()");
    mlo_select_all();
    NSLog(@"mlo_select_all() returned. reshowing contextualMenu");
    [_selectionViewController showPostSelectAll];
}


-(void)defineBuffer:(id) sender{
    [_selectionViewController defineBuffer];
}

-(NSString *) description{
    return @"MLO contextual menu focus UIView";
}
- (BOOL) canPerformAction:(SEL)selector withSender:(id) sender {
    if ((selector == @selector(loCopyToClipboard:)) ||
        ((selector == @selector(loSelectAll:)) ||
         (selector == @selector(defineBuffer:)))) {
        return YES;
    }
    return NO;
}
- (BOOL) canBecomeFirstResponder {
    return YES;
}
@end
