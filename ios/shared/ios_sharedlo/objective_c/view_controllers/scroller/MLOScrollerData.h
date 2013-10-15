// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOObject.h"

static const CGFloat
    PIXEL_TO_LOGIC_RATIO = 14.978,//513885/30/MLO_IPAD_HEIGHT_IN_PIXELS,
    CANVAS_WIDTH_IN_LOGIC = 13000,
    // NOTE:
    // value keeps shifting between 14.975 and 14.984,
    // though mostly it is on the 14.975 area

    LO_PAGE_SEPARATOR_HEIGHT_IN_LOGIC = 18 * PIXEL_TO_LOGIC_RATIO,
    LO_HORIZONAL_BORDER_WIDTH_IN_LOGIC = 18 * PIXEL_TO_LOGIC_RATIO,
    PORTRAIT_PAGE_HEIGHT_IN_LOGIC_WITH_SEPARATOR = 513885 /30,
    PORTRAIT_PAGE_HEIGHT_IN_LOGIC_WITHOUT_SEPARATOR = PORTRAIT_PAGE_HEIGHT_IN_LOGIC_WITH_SEPARATOR - LO_PAGE_SEPARATOR_HEIGHT_IN_LOGIC;

@class MLOMainViewController, MLOScrollerTooltip,MLOScrollerGridViewController;
@interface MLOScrollerData : MLOObject
-(id)initWithMainViewController:(MLOMainViewController *) mainViewController;
-(void) onRotateWithGrid:(MLOScrollerGridViewController *) grid;
-(NSInteger) getTotalPages;
-(CGRect) getNewScrollerFrame;
-(CGRect) getShiftedScrollerFrame:(CGFloat) pixelDeltaX;
-(void) updateTooltip:(MLOScrollerTooltip *) tooltip withGrid:(MLOScrollerGridViewController *) grid;
-(void) showLibreOffice;
-(void) hideLibreOffice;
@end
