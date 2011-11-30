/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



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
