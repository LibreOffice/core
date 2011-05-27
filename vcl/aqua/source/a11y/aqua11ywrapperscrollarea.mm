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

#include "aqua/salinst.h"

#include "aqua11ywrapperscrollarea.h"
#include "aqua11ywrapperscrollbar.h"
#include "aqua11yrolehelper.h"

// Wrapper for AXScrollArea role

@implementation AquaA11yWrapperScrollArea : AquaA11yWrapper

-(id)scrollBarWithOrientation:(NSString *)orientation {
    AquaA11yWrapper * theScrollBar = nil;
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    NSArray * elementChildren = [ self accessibilityAttributeValue: NSAccessibilityChildrenAttribute ];
    if ( [ elementChildren count ] > 0 ) {
        NSEnumerator * enumerator = [ elementChildren objectEnumerator ];
        id child;
        while ( ( child = [ enumerator nextObject ] ) ) {
            AquaA11yWrapper * element = ( AquaA11yWrapper * ) child;
            if ( [ element isKindOfClass: [ AquaA11yWrapperScrollBar class ] ] ) { 
                AquaA11yWrapperScrollBar * scrollBar = (AquaA11yWrapperScrollBar *) element;
                if ( [ [ scrollBar orientationAttribute ] isEqualToString: orientation ] ) {
                    theScrollBar = scrollBar;
                    break;
                }
            }
        }
    }
    [ pool release ];
    return theScrollBar;
}

-(id)verticalScrollBarAttribute {
    return [ self scrollBarWithOrientation: NSAccessibilityVerticalOrientationValue ];
}

-(id)horizontalScrollBarAttribute {
    return [ self scrollBarWithOrientation: NSAccessibilityHorizontalOrientationValue ];
}

-(NSArray *)accessibilityAttributeNames {
    // Default Attributes
    NSMutableArray * attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
    // Special Attributes and removing unwanted attributes depending on role
    [ attributeNames removeObject: NSAccessibilityEnabledAttribute ];
    [ attributeNames addObjectsFromArray: [ NSArray arrayWithObjects:
            NSAccessibilityContentsAttribute, 
            NSAccessibilityVerticalScrollBarAttribute, 
            NSAccessibilityHorizontalScrollBarAttribute, 
            nil ]
    ];
    return attributeNames;
}

@end
