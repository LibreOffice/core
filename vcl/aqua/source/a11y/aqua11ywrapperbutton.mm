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

#include "aqua11ywrapperbutton.h"
#include "aqua11ytextwrapper.h"

// Wrapper for AXButton role

@implementation AquaA11yWrapperButton : AquaA11yWrapper

-(id)valueAttribute {
    return [ NSString string ]; // we propagate AXTitle, that's enough
}

-(id)descriptionAttribute {
    return [ NSString string ]; // we propagate AXTitle, that's enough
}

-(NSArray *)accessibilityAttributeNames {
    // Default Attributes
    NSMutableArray * attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
    // Special Attributes and removing unwanted attributes depending on role
    if ( [ attributeNames containsObject: NSAccessibilityTitleAttribute ] ) {
        [ attributeNames removeObject: NSAccessibilityDescriptionAttribute ];
    } else {
        [ attributeNames addObject: NSAccessibilityTitleAttribute ];
    }
    // Remove text-specific attributes
    [ attributeNames removeObjectsInArray: [ AquaA11yTextWrapper specialAttributeNames ] ];
    return attributeNames;
}

@end
