// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOMainViewController_Friend.h"

@class MLOAppRoleBase,MLOToolbarButton,MLOTopbarViewController,MLOToolbarViewController;
@interface MLOMainViewController ()
@property BOOL focused;
@property CGFloat topBarHeight;
@property UIView * flasher;
@property MLOTopbarViewController * topbar;
@property MLOToolbarViewController * toolbar;
@property MLOAppRoleBase * role;

-(void)resize;
-(void)resetSubviews;
@end
