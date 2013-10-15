// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOObject.h"

@class MLOMainViewController;

@interface MLOGestureEngine : MLOObject
-(id)initWithMainViewController:(MLOMainViewController *) mainViewController;
-(void)showLibreOffice:(UIWindow *) window;
-(void)hideLibreOffice;
-(void)reset;
-(void)onRotate;
@end
