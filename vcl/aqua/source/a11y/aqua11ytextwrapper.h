/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SV_AQUA11TEXTWRAPPER_H
#define _SV_AQUA11TEXTWRAPPER_H

#include "aquavcltypes.h"
#include "aqua11ywrapper.h"

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

#endif // _SV_AQUA11TEXTWRAPPER_H
