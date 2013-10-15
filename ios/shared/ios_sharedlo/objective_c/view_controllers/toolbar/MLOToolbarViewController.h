// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOViewController.h"

@class  MLOMainViewController,MLOToolbarButton;
@interface MLOToolbarViewController : MLOViewController
-(id)initWithMainViewController:(MLOMainViewController *) mainViewController;
-(void)addToMainViewController;
-(void)showLibreOffice;
-(void)hideLibreOffice;
-(void) expandDidToggle;
-(BOOL) isTappable;
-(void) onRotate;
-(void)hideAllButtonsBut:(MLOToolbarButton *) showButton;
@end
