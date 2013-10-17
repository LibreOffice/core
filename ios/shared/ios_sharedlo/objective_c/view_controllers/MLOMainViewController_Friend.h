// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOMainViewController.h"

static const CGFloat TOP_BAR_HEIGHT =40.0f;

@class MLOGestureEngine,MLORenderManager;
@interface MLOMainViewController ()
@property MLOGestureEngine * gestureEngine;
@property MLORenderManager * renderManager;
-(void) toggleExpand;
@end
