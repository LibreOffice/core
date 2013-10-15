// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOThesaurusViewController.h"
#import "MLOSelectionViewController_Impl.h"
#import "MLOMainViewController_Impl.h"
#import "MLOGestureEngine_Impl.h"

static const CGFloat THESAURUS_PADDING = 100.0f;
@interface MLOThesaurusViewController ()
@property MLOSelectionViewController * selection;
@end

@implementation MLOThesaurusViewController


-(id) initWithSelectionViewController:(MLOSelectionViewController *) selectionViewController{
    self = [self initWithTerm:selectionViewController.buffer];
    if(self){
        self.selection = selectionViewController;
        self.modalTransitionStyle = UIModalTransitionStyleCrossDissolve;
        self.modalPresentationStyle = UIModalPresentationFormSheet;
    }
    return self;
}

-(void)show{
    static const CGFloat TWO_PADDINGS = THESAURUS_PADDING*2.0;
    self.view.frame = CGRectMake(THESAURUS_PADDING,
                                       THESAURUS_PADDING,
                                       _selection.mainViewController.canvas.frame.size.width - TWO_PADDINGS ,
                                       _selection.mainViewController.canvas.frame.size.height - TWO_PADDINGS);

    _selection.mainViewController.gestureEngine.focused = NO;
    [_selection.mainViewController presentViewController:self animated:YES completion:nil];

}

-(void)viewDidDisappear:(BOOL)animated{
    _selection.mainViewController.gestureEngine.focused = YES;
    [_selection reselect:YES];
}
@end
