// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOPostRenderManager.h"
#import "MLOMainViewController.h"

@interface MLOPostRenderManager ()
@property UITextView * invisibleSelection;
@end

@implementation MLOPostRenderManager

-(id)init{
    self = [super init];
    if(self){
        self.invisibleSelection = [UITextView new];

        _invisibleSelection.backgroundColor = [UIColor clearColor];
        _invisibleSelection.textColor = [UIColor clearColor];
        
    }
    return self;
}

-(void)addToMainViewController:(MLOMainViewController *) mainViewController{
    [mainViewController.canvas addSubview:_invisibleSelection];
}

@end
