// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "slideShowPreviewTable_vc.h"

@protocol MainSplitViewControllerProtocol <NSObject>

- (void) didReceivePresentationStarted;

@end

@interface slideShowPreviewTable_vc_ipad : slideShowPreviewTable_vc

@property (strong, nonatomic) id <MainSplitViewControllerProtocol> delegate;

@end
