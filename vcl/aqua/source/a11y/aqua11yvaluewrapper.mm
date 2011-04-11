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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "aqua11yvaluewrapper.h"
#include "aqua11ywrapperstatictext.h"

using namespace ::com::sun::star::uno;

// Wrapper for XAccessibleValue
// Remember: A UNO-Value is a single numeric value. Regarding the Mac A11y-API, a value can be anything!

@implementation AquaA11yValueWrapper : NSObject

+(id)valueAttributeForElement:(AquaA11yWrapper *)wrapper {
    // TODO: Detect Type from Any
    if ( [ wrapper accessibleValue ] != nil ) {
        long value = 0;
        [ wrapper accessibleValue ] -> getCurrentValue() >>= value;
        return [ NSNumber numberWithLong: value ];
    }
    return [ NSNumber numberWithLong: 0 ];
}

+(id)minValueAttributeForElement:(AquaA11yWrapper *)wrapper {
    // TODO: Detect Type from Any
    if ( [ wrapper accessibleValue ] != nil ) {
        long value = 0;
        [ wrapper accessibleValue ] -> getMinimumValue() >>= value;
        return [ NSNumber numberWithLong: value ];
    }
    return [ NSNumber numberWithLong: 0 ];
}

+(id)maxValueAttributeForElement:(AquaA11yWrapper *)wrapper {
    // TODO: Detect Type from Any
    if ( [ wrapper accessibleValue ] != nil ) {
        long value = 0;
        [ wrapper accessibleValue ] -> getMaximumValue() >>= value;
        return [ NSNumber numberWithLong: value ];
    }
    return [ NSNumber numberWithLong: 0 ];
}

+(void)setValueAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value {
    // TODO: Detect Type from NSNumber
    if ( [ value isKindOfClass: [ NSNumber class ] ] 
      && [ wrapper accessibleValue ] != nil ) {
        NSNumber * number = (NSNumber *) value;
        Any numberAny ( [ number longValue ] );
        [ wrapper accessibleValue ] -> setCurrentValue ( numberAny );
    }
}

+(void)addAttributeNamesTo:(NSMutableArray *)attributeNames {
    [ attributeNames addObject: NSAccessibilityValueAttribute ];
}

+(BOOL)isAttributeSettable:(NSString *)attribute forElement:(AquaA11yWrapper *)wrapper {
    BOOL isSettable = NO;
    if ( [ wrapper accessibleValue ] != nil 
      && [ attribute isEqualToString: NSAccessibilityValueAttribute ] 
      && ! [ wrapper isKindOfClass: [ AquaA11yWrapperStaticText class ] ] ) {
        isSettable = YES;
    }
    return isSettable;
}

@end
