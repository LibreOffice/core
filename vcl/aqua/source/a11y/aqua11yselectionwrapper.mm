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
#include "aqua/aqua11yfactory.h"

#include "aqua11yselectionwrapper.h"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

@implementation AquaA11ySelectionWrapper : NSObject

+(id)selectedChildrenAttributeForElement:(AquaA11yWrapper *)wrapper
{
    Reference< XAccessibleSelection > xAccessibleSelection = [ wrapper accessibleSelection ];
    if( xAccessibleSelection.is() )
    {
        NSMutableArray * children = [ [ NSMutableArray alloc ] init ];
        try {
            sal_Int32 n = xAccessibleSelection -> getSelectedAccessibleChildCount();
            for ( sal_Int32 i=0 ; i < n ; ++i ) {
                [ children addObject: [ AquaA11yFactory wrapperForAccessible: xAccessibleSelection -> getSelectedAccessibleChild( i ) ] ];
            }
            
            return children;
            
        } catch ( Exception& e)
        {
        }
    }
    
    return nil;
}


+(void)addAttributeNamesTo:(NSMutableArray *)attributeNames
{
    [ attributeNames addObject: NSAccessibilitySelectedChildrenAttribute ];
}

+(BOOL)isAttributeSettable:(NSString *)attribute forElement:(AquaA11yWrapper *)wrapper
{
    (void)wrapper;
    if ( [ attribute isEqualToString: NSAccessibilitySelectedChildrenAttribute ] )
    {
        return YES;
    }
    else
    {
        return NO;
    }
}

+(void)setSelectedChildrenAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value
{
    Reference< XAccessibleSelection > xAccessibleSelection = [ wrapper accessibleSelection ];
    try {
        xAccessibleSelection -> clearAccessibleSelection();
        
        unsigned c = [ value count ];
        for ( unsigned i = 0 ; i < c ; ++i ) {
            xAccessibleSelection -> selectAccessibleChild( [ [ value objectAtIndex: i ] accessibleContext ] -> getAccessibleIndexInParent() );
        }
    } catch ( Exception& e) {
    }            
}

@end
