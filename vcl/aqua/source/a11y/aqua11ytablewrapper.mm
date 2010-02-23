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

#include "aqua11ytablewrapper.h"
#include "aqua11yfactory.h"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;

@implementation AquaA11yTableWrapper : NSObject

+(id)childrenAttributeForElement:(AquaA11yWrapper *)wrapper
{
    try
    {
        NSMutableArray * cells = [ [ NSMutableArray alloc ] init ];
        XAccessibleComponent * accessibleComponent = [ wrapper accessibleComponent ];
        XAccessibleTable * accessibleTable = [ wrapper accessibleTable ];
        // find out which cells are actually visible by determining the top-left-cell and the bottom-right-cell
        Size tableSize = accessibleComponent -> getSize();
        Point point;
        point.X = 0;
        point.Y = 0;
        Reference < XAccessible > rAccessibleTopLeft = accessibleComponent -> getAccessibleAtPoint ( point );
        point.X = tableSize.Width - 1;
        point.Y = tableSize.Height - 1;
        Reference < XAccessible > rAccessibleBottomRight = accessibleComponent -> getAccessibleAtPoint ( point );
        if ( rAccessibleTopLeft.is() && rAccessibleBottomRight.is() )
        {
            sal_Int32 idxTopLeft = rAccessibleTopLeft -> getAccessibleContext() -> getAccessibleIndexInParent();
            sal_Int32 idxBottomRight = rAccessibleBottomRight -> getAccessibleContext() -> getAccessibleIndexInParent();
            sal_Int32 rowTopLeft = accessibleTable -> getAccessibleRow ( idxTopLeft );
            sal_Int32 columnTopLeft = accessibleTable -> getAccessibleColumn ( idxTopLeft );
            sal_Int32 rowBottomRight = accessibleTable -> getAccessibleRow ( idxBottomRight );
            sal_Int32 columnBottomRight = accessibleTable -> getAccessibleColumn ( idxBottomRight );
            // create an array containing the visible cells
            for ( sal_Int32 rowCount = rowTopLeft; rowCount <= rowBottomRight; rowCount++ )
            {
                for ( sal_Int32 columnCount = columnTopLeft; columnCount <= columnBottomRight; columnCount++ )
                {
                    Reference < XAccessible > rAccessibleCell = accessibleTable -> getAccessibleCellAt ( rowCount, columnCount );
                    if ( rAccessibleCell.is() )
                    {
                        id cell_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rAccessibleCell -> getAccessibleContext() ];
                        [ cells addObject: cell_wrapper ];
                        [ cell_wrapper release ];
                    }
                }
            }
        }
        [ cells autorelease ];
        return NSAccessibilityUnignoredChildren( cells );
    }
    catch (const Exception &e) 
    {
        // TODO: Log
        return nil;
    }
}

@end
