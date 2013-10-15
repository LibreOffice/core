// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import <UIKit/UIKit.h>

@class MLOFileManagerViewController;
@class MLOAppDelegate;

@interface MLOAppViewController : UIViewController
@property MLOFileManagerViewController * fileManager;
@property MLOAppDelegate * appDelegate;

-(id)initWithAppDelegate:(MLOAppDelegate *)appDelegate;
-(CGRect)currentFullscreenFrame;

@end
