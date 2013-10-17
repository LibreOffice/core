// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOButton.h"

typedef void (^MLOToolbarButtonCallback)();
#define MLO_TOOLBAR_BUTTON_STUB_CALLBACK ^{ NSLog(@"stub toolbar button callback");}


@class  MLOToolbarViewController,MLOResourceImage;

typedef enum { AUTOMATIC, RETAP_OR_OTHER_TAPPED,RETAP_ONLY} MLOToolbarButtonTapReleaseType;
#define MLOToolbarButtonTapReleaseTypeString(enum) [@[@"AUTOMATIC",@"RETAP_OR_OTHER_TAPPED",@"RETAP_ONLY"] objectAtIndex:enum]

@interface MLOToolbarButton : MLOButton
@property MLOResourceImage * alternateImage;

+(MLOToolbarButton*)buttonWithImage:(MLOResourceImage *) image onTap:(MLOToolbarButtonCallback) onTap tapRelease:(MLOToolbarButtonTapReleaseType)type;
+(MLOToolbarButton*)buttonWithImage:(MLOResourceImage *) image onTap:(MLOToolbarButtonCallback) onTap tapRelease:(MLOToolbarButtonTapReleaseType)type onTapRelease:(MLOToolbarButtonCallback)onTapRelease;

-(void)addToToolbarControler:(MLOToolbarViewController *) toolbar;
-(void)showLibreOffice;
-(void)hideLibreOffice;
-(void)onOtherButtonTapped;
-(void)switchImage;
@end
