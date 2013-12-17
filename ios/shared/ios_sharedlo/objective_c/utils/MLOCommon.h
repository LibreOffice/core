// -*- Mode: ObjC; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef __Mobile_LibreOffice_MLOCommon_h__
#define __Mobile_LibreOffice_MLOCommon_h__

typedef enum { LO_APP, TILE_TESTER} MLOAppRole;
#define MLOAppRoleString(enum) [@[@"LO_APP",@"RENDER_TILE_TESTER"] objectAtIndex:enum]
static const MLOAppRole APP_ROLE = MLO_APP_ROLE;
static const BOOL ENABLE_LO_DESKTOP = APP_ROLE == LO_APP;

static const BOOL
    LOG_DRAW_RECT = NO,
    LOG_GET_VIEW_DATA = YES,
    LOG_FLICK_FRAMES=NO,
    LOG_PAN=NO,
    LOG_PINCH=YES,
    LOG_DOUBLE_TAP=YES,
    LOG_GESTURE_LIMITING = NO;

#define LOG_RECT(RECT,NAME)  NSLog(@"%@: w:%d, h:%d, origin:(%d,%d)",NAME, (int) RECT.size.width, (int) RECT.size.height, (int) RECT.origin.x, (int) RECT.origin.y)
#define IGNORE_ARG(X) ((void) X)

#define CPPU_ENV gcc3
#define OSL_DEBUG_LEVEL 1
#define RTL_USING
#define HAVE_GCC_VISIBILITY_FEATURE

#import "time.h"

typedef enum {IN,OUT} MLOFadeType;
#define MLOFadeTypeString(enum) [@[@"IN",@"OUT"] objectAtIndex:enum]

#define CGRECT_ZERO CGRectMake(0,0,0,0)
#define CGRECT_ONE CGRectMake(1,1,1,1)
static const CGFloat    MLO_IPAD_HEIGHT_IN_PIXELS   = 1024,
                        MLO_IPAD_WIDTH_IN_PIXELS    = 768;


#endif
