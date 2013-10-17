// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLORenderingUIView.h"

@class MLORenderManager;
@interface MLORenderBuffer : MLORenderingUIView
@property NSInteger index;
@property MLORenderBuffer * previous;
-(id)initWithArrayIndex:(NSInteger) index renderManager:(MLORenderManager *) manager;
+(CGFloat) getAverageFps;
+(CGFloat) getMaxFps;
-(void)hide;
@end
