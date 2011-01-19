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

#include "aqua11ycomponentwrapper.h"
#include "aqua11yrolehelper.h"
#include <com/sun/star/accessibility/AccessibleRole.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;

// Wrapper for XAccessibleComponent and XAccessibleExtendedComponent

@implementation AquaA11yComponentWrapper : NSObject

+(id)sizeAttributeForElement:(AquaA11yWrapper *)wrapper {
    Size size = [ wrapper accessibleComponent ] -> getSize();
    NSSize nsSize = NSMakeSize ( (float) size.Width, (float) size.Height );
    return [ NSValue valueWithSize: nsSize ];
}

// TODO: should be merged with AquaSalFrame::VCLToCocoa... to a general helper method
+(id)positionAttributeForElement:(AquaA11yWrapper *)wrapper {
    // VCL coordinates are in upper-left-notation, Cocoa likes it the Cartesian way (lower-left)
    NSRect screenRect = [ [ NSScreen mainScreen ] frame ];
    Size size = [ wrapper accessibleComponent ] -> getSize();
    Point location = [ wrapper accessibleComponent ] -> getLocationOnScreen();
    NSPoint nsPoint = NSMakePoint ( (float) location.X, (float) ( screenRect.size.height - size.Height - location.Y ) );
    return [ NSValue valueWithPoint: nsPoint ];
}

+(id)descriptionAttributeForElement:(AquaA11yWrapper *)wrapper {
    if ( [ wrapper accessibleExtendedComponent ] != nil ) {
        return CreateNSString ( [ wrapper accessibleExtendedComponent ] -> getToolTipText() );
    } else {
        return nil;
    }
}

+(void)addAttributeNamesTo:(NSMutableArray *)attributeNames {
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    [ attributeNames addObjectsFromArray: [ NSArray arrayWithObjects: 
            NSAccessibilitySizeAttribute, 
            NSAccessibilityPositionAttribute, 
            NSAccessibilityFocusedAttribute, 
            NSAccessibilityEnabledAttribute, 
            nil ] ];
    [ pool release ];
}

+(BOOL)isAttributeSettable:(NSString *)attribute forElement:(AquaA11yWrapper *)wrapper {
    BOOL isSettable = NO;
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    if ( [ attribute isEqualToString: NSAccessibilityFocusedAttribute ] 
      && ! [ [ AquaA11yRoleHelper getNativeRoleFrom: [ wrapper accessibleContext ] ] isEqualToString: NSAccessibilityScrollBarRole ] 
      && ! [ [ AquaA11yRoleHelper getNativeRoleFrom: [ wrapper accessibleContext ] ] isEqualToString: NSAccessibilityStaticTextRole ] ) {
        isSettable = YES;
    }
    [ pool release ];
    return isSettable;
}

+(void)setFocusedAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value {
    if ( [ value boolValue ] == YES ) {
        if ( [ wrapper accessibleContext ] -> getAccessibleRole() == AccessibleRole::COMBO_BOX ) {
            // special treatment for comboboxes: find the corresponding PANEL and set focus to it
            Reference < XAccessible > rxParent = [ wrapper accessibleContext ] -> getAccessibleParent();
            if ( rxParent.is() ) {
                Reference < XAccessibleContext > rxContext = rxParent->getAccessibleContext();
                if ( rxContext.is() && rxContext -> getAccessibleRole() == AccessibleRole::PANEL ) {
                    Reference < XAccessibleComponent > rxComponent = Reference < XAccessibleComponent > ( rxParent -> getAccessibleContext(), UNO_QUERY );
                    if ( rxComponent.is() ) {
                        rxComponent -> grabFocus();
                    }
                }
            }
        } else {
            [ wrapper accessibleComponent ] -> grabFocus();
        }
    }
}

@end
