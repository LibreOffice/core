// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOViewController.h"

@class MLORenderManager,MLOSelectionViewController,MLOScrollerViewController,MLOKeyboardManager;
@interface MLOMainViewController : MLOViewController<UITextViewDelegate>

@property MLOSelectionViewController * selection;
@property MLOScrollerViewController * scroller;
@property MLOKeyboardManager * keyboard;
@property UIView * canvas;

-(void) rotate;
-(void) showLibreOffice:(UIWindow *) window;
-(void) onTextEdit;
-(void) hideLibreOffice;
-(BOOL) isTappable;
-(void) flash;
-(CGFloat) getZoom;
@end

// vim:set shiftwidth=4 softtabstop=4 expandtab:
