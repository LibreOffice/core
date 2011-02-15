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

#include "salinst.h"
#include "aqua11ywrappercombobox.h"
#include "aqua11yrolehelper.h"
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

// Wrapper for AXCombobox role

@implementation AquaA11yWrapperComboBox : AquaA11yWrapper

#pragma mark -
#pragma mark Specialized Init Method

-(id)initWithAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
    self = [ super initWithAccessibleContext: rxAccessibleContext ];
    if ( self != nil )
    {
        textArea = nil;
    }
    return self;
}

#pragma mark -
#pragma mark Private Helper Method

-(AquaA11yWrapper *)textArea {
    // FIXME: May cause problems when stored. Then get dynamically each time (bad performance!)
    if ( textArea == nil ) {
        NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
        NSArray * elementChildren = [ super childrenAttribute ];
        if ( [ elementChildren count ] > 0 ) {
            NSEnumerator * enumerator = [ elementChildren objectEnumerator ];
            id child;
            while ( ( child = [ enumerator nextObject ] ) ) {
                AquaA11yWrapper * element = ( AquaA11yWrapper * ) child;
                if ( [ [ AquaA11yRoleHelper getNativeRoleFrom: [ element accessibleContext ] ] isEqualToString: NSAccessibilityTextAreaRole ] ) {
                    textArea = element;
                    break;
                }
            }
        }
        [ pool release ];
    }
    return textArea;
}

#pragma mark -
#pragma mark Wrapped Attributes From Contained Text Area

-(id)valueAttribute {
    if ( [ self textArea ] != nil ) {
        return [ [ self textArea ] valueAttribute ];
    }
    return @"";
}

-(id)numberOfCharactersAttribute {
    if ( [ self textArea ] != nil ) {
        return [ [ self textArea ] numberOfCharactersAttribute ];
    }
    return [ NSNumber numberWithInt: 0 ];
}

-(id)selectedTextAttribute {
    if ( [ self textArea ] != nil ) {
        return [ [ self textArea ] selectedTextAttribute ];
    }
    return @"";
}

-(id)selectedTextRangeAttribute {
    if ( [ self textArea ] != nil ) {
        return [ [ self textArea ] selectedTextRangeAttribute ];
    }
    return [ NSValue valueWithRange: NSMakeRange ( 0, 0 ) ];
}

-(id)visibleCharacterRangeAttribute {
    if ( [ self textArea ] != nil ) {
        return [ [ self textArea ] visibleCharacterRangeAttribute ];
    }
    return [ NSValue valueWithRange: NSMakeRange ( 0, 0 ) ];
}

#pragma mark -
#pragma mark Accessibility Protocol

-(BOOL)accessibilityIsAttributeSettable:(NSString *)attribute {
    if ( [ self textArea ] != nil && (
         [ attribute isEqualToString: NSAccessibilitySelectedTextAttribute ]
      || [ attribute isEqualToString: NSAccessibilitySelectedTextRangeAttribute ]
      || [ attribute isEqualToString: NSAccessibilityVisibleCharacterRangeAttribute ] ) ) {
        return [ [ self textArea ] accessibilityIsAttributeSettable: attribute ];
    }
    return [ super accessibilityIsAttributeSettable: attribute ];
}

-(void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute {
    if ( [ self textArea ] != nil && (
         [ attribute isEqualToString: NSAccessibilitySelectedTextAttribute ]
      || [ attribute isEqualToString: NSAccessibilitySelectedTextRangeAttribute ]
      || [ attribute isEqualToString: NSAccessibilityVisibleCharacterRangeAttribute ] ) ) {
        return [ [ self textArea ] accessibilitySetValue: value forAttribute: attribute ];
    }
    return [ super accessibilitySetValue: value forAttribute: attribute ];
}

-(NSArray *)accessibilityAttributeNames {
    // Default Attributes
    NSMutableArray * attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
    // Special Attributes and removing unwanted attributes depending on role
    [ attributeNames removeObjectsInArray: [ NSArray arrayWithObjects:
            NSAccessibilityTitleAttribute, 
            NSAccessibilityChildrenAttribute, 
            nil ]
    ];
    [ attributeNames addObjectsFromArray: [ NSArray arrayWithObjects:
            NSAccessibilityExpandedAttribute, 
            NSAccessibilityValueAttribute, 
            NSAccessibilityNumberOfCharactersAttribute, 
            NSAccessibilitySelectedTextAttribute, 
            NSAccessibilitySelectedTextRangeAttribute, 
            NSAccessibilityVisibleCharacterRangeAttribute, 
            nil ]
    ];
    return attributeNames;
}

@end
