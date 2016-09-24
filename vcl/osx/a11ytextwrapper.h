/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_VCL_OSX_A11YTEXTWRAPPER_H
#define INCLUDED_VCL_OSX_A11YTEXTWRAPPER_H

#include "osx/osxvcltypes.h"
#include "osx/a11ywrapper.h"

@interface AquaA11yTextWrapper : NSObject
{
}
+(id)valueAttributeForElement:(AquaA11yWrapper *)wrapper;
+(id)numberOfCharactersAttributeForElement:(AquaA11yWrapper *)wrapper;
+(id)selectedTextAttributeForElement:(AquaA11yWrapper *)wrapper;
+(id)selectedTextRangeAttributeForElement:(AquaA11yWrapper *)wrapper;
+(id)visibleCharacterRangeAttributeForElement:(AquaA11yWrapper *)wrapper;
+(id)sharedTextUIElementsAttributeForElement:(AquaA11yWrapper *)wrapper;
+(id)sharedCharacterRangeAttributeForElement:(AquaA11yWrapper *)wrapper;
+(id)stringForRangeAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)range;
+(id)attributedStringForRangeAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)range;
+(id)rangeForIndexAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)index;
+(id)rangeForPositionAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)point;
+(id)boundsForRangeAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)range;
+(id)styleRangeForIndexAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)index;
+(id)rTFForRangeAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)range;
+(id)lineForIndexAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)index;
+(id)rangeForLineAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)line;
+(void)addAttributeNamesTo:(NSMutableArray *)attributeNames;
+(void)addParameterizedAttributeNamesTo:(NSMutableArray *)attributeNames;
+(NSArray *)specialAttributeNames;
+(NSArray *)specialParameterizedAttributeNames;
+(BOOL)isAttributeSettable:(NSString *)attribute forElement:(AquaA11yWrapper *)wrapper;
+(void)setVisibleCharacterRangeAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value;
+(void)setSelectedTextRangeAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value;
+(void)setSelectedTextAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value;
+(void)setValueAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value;
@end

#endif // INCLUDED_VCL_OSX_A11YTEXTWRAPPER_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
