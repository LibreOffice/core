// -*- Mode: Objective-C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#import "MLOObject.h"

typedef enum {WIDTH_IS_HEIGHT,WIDTH_IS_NOT_HEIGHT} MLOTestingTileParametersMode;
#define MLOTestingTileParametersModeString(enum) [@[@"WIDTH_IS_HEIGHT",@"WIDTH_IS_NOT_HEIGHT"] objectAtIndex:enum]

typedef void (^MLOTestingTileParameterExtractor)(CGFloat value);

@class MLOTestingTileParametersViewController;
@interface MLOTestingTileParameter : MLOObject
-(MLOTestingTileParameter *)initWithParams:(MLOTestingTileParametersViewController *) params label:(NSString *)label widthIsNotHeightExtractor:(MLOTestingTileParameterExtractor) widthIsNotHeightExtractor widthIsHeightExtractor:(MLOTestingTileParameterExtractor) widthIsHeightExtractor defaultValue:(NSInteger) defaultValue;
-(void)extractMode:(MLOTestingTileParametersMode) mode;
-(void)setParamFrame:(CGRect)  paramFrame;
-(void)addToSuperview;
-(void)enterMode:(MLOTestingTileParametersMode)mode;
@end
