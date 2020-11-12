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


#include "a11yvaluewrapper.h"
#include "a11ywrapperstatictext.h"

using namespace ::com::sun::star::uno;

// Wrapper for XAccessibleValue
// Remember: A UNO-Value is a single numeric value. Regarding the Mac A11y-API, a value can be anything!

@implementation AquaA11yValueWrapper : NSObject

+(id)valueAttributeForElement:(AquaA11yWrapper *)wrapper {
    // TODO: Detect Type from Any
    if ( [ wrapper accessibleValue ] ) {
        sal_Int32 value = 0;
        [ wrapper accessibleValue ] -> getCurrentValue() >>= value;
        return [ NSNumber numberWithLong: value ];
    }
    return [ NSNumber numberWithLong: 0 ];
}

+(id)minValueAttributeForElement:(AquaA11yWrapper *)wrapper {
    // TODO: Detect Type from Any
    if ( [ wrapper accessibleValue ] ) {
        sal_Int32 value = 0;
        [ wrapper accessibleValue ] -> getMinimumValue() >>= value;
        return [ NSNumber numberWithLong: value ];
    }
    return [ NSNumber numberWithLong: 0 ];
}

+(id)maxValueAttributeForElement:(AquaA11yWrapper *)wrapper {
    // TODO: Detect Type from Any
    if ( [ wrapper accessibleValue ] ) {
        sal_Int32 value = 0;
        [ wrapper accessibleValue ] -> getMaximumValue() >>= value;
        return [ NSNumber numberWithLong: value ];
    }
    return [ NSNumber numberWithLong: 0 ];
}

+(void)setValueAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value {
    // TODO: Detect Type from NSNumber
    if ( [ value isKindOfClass: [ NSNumber class ] ] 
      && [ wrapper accessibleValue ] ) {
        NSNumber * number = static_cast<NSNumber *>(value);
        Any numberAny ( [ number longValue ] );
        [ wrapper accessibleValue ] -> setCurrentValue ( numberAny );
    }
}

+(void)addAttributeNamesTo:(NSMutableArray *)attributeNames {
    [ attributeNames addObject: NSAccessibilityValueAttribute ];
}

+(BOOL)isAttributeSettable:(NSString *)attribute forElement:(AquaA11yWrapper *)wrapper {
    bool isSettable = false;
    if ( [ wrapper accessibleValue ]
      && [ attribute isEqualToString: NSAccessibilityValueAttribute ] 
      && ! [ wrapper isKindOfClass: [ AquaA11yWrapperStaticText class ] ] ) {
        isSettable = true;
    }
    return isSettable;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
