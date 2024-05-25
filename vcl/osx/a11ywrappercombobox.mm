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


#include <vcl/svapp.hxx>
#include <osx/salinst.h>

#include "a11ywrappercombobox.h"
#include "a11yrolehelper.h"

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
                AquaA11yWrapper * element = static_cast<AquaA11yWrapper *>(child);
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
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return NO;

    if ( [ self textArea ] != nil && (
         [ attribute isEqualToString: NSAccessibilitySelectedTextAttribute ]
      || [ attribute isEqualToString: NSAccessibilitySelectedTextRangeAttribute ]
      || [ attribute isEqualToString: NSAccessibilityVisibleCharacterRangeAttribute ] ) ) {
        return [ [ self textArea ] accessibilityIsAttributeSettable: attribute ];
    }
    return [ super accessibilityIsAttributeSettable: attribute ];
}

-(void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return;

    if ( [ self textArea ] != nil && (
         [ attribute isEqualToString: NSAccessibilitySelectedTextAttribute ]
      || [ attribute isEqualToString: NSAccessibilitySelectedTextRangeAttribute ]
      || [ attribute isEqualToString: NSAccessibilityVisibleCharacterRangeAttribute ] ) ) {
        return [ [ self textArea ] accessibilitySetValue: value forAttribute: attribute ];
    }
    return [ super accessibilitySetValue: value forAttribute: attribute ];
}

-(NSArray *)accessibilityAttributeNames {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return [ NSArray array ];

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
