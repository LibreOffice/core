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

#include "aqua/aqua11yfactory.h"

#include "aqua11ytablewrapper.h"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;

@implementation AquaA11yTableWrapper : AquaA11yWrapper

+(id)childrenAttributeForElement:(AquaA11yTableWrapper *)wrapper
{
    XAccessibleTable * accessibleTable = [ wrapper accessibleTable ];
    NSArray* pResult = nil;
    if( accessibleTable )
    {
        NSMutableArray * cells = [ [ NSMutableArray alloc ] init ];
        try
        {
            sal_Int32 nRows = accessibleTable->getAccessibleRowCount();
            sal_Int32 nCols = accessibleTable->getAccessibleColumnCount();
    
            if( nRows * nCols < MAXIMUM_ACCESSIBLE_TABLE_CELLS )
            {
                // make all children visible to the hierarchy
                for ( sal_Int32 rowCount = 0; rowCount < nRows; rowCount++ )
                {
                    for ( sal_Int32 columnCount = 0; columnCount < nCols; columnCount++ )
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
            else
            {
                XAccessibleComponent * accessibleComponent = [ wrapper accessibleComponent ];
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
            }
            pResult = NSAccessibilityUnignoredChildren( cells );
        }
        catch (const Exception &e) 
        {
        }
        [cells autorelease];
    }
    
    return pResult;
}

+(void)addAttributeNamesTo: (NSMutableArray *)attributeNames object: (AquaA11yWrapper*)pObject
{
    XAccessibleTable * accessibleTable = [ pObject accessibleTable ];
    if( accessibleTable )
    {
        sal_Int32 nRows = accessibleTable->getAccessibleRowCount();
        sal_Int32 nCols = accessibleTable->getAccessibleColumnCount();    
        
        
        if( nRows*nCols < MAXIMUM_ACCESSIBLE_TABLE_CELLS )
        {
            [ attributeNames addObject: NSAccessibilityRowsAttribute ];
            [ attributeNames addObject: NSAccessibilityColumnsAttribute ];
        }
    }
}

-(id)rowsAttribute
{
    NSArray* pResult = nil;

    XAccessibleTable * accessibleTable = [ self accessibleTable ];
    if( accessibleTable )
    {
        sal_Int32 nRows = accessibleTable->getAccessibleRowCount();
        sal_Int32 nCols = accessibleTable->getAccessibleColumnCount();    
        if( nRows * nCols < MAXIMUM_ACCESSIBLE_TABLE_CELLS )
        {
            NSMutableArray * cells = [ [ NSMutableArray alloc ] init ];
            try
            {
                // find out number of rows
                sal_Int32 nRows = accessibleTable->getAccessibleRowCount();
                for( sal_Int32 n = 0; n < nRows; n++ )
                {
                    Reference < XAccessible > rAccessibleCell = accessibleTable -> getAccessibleCellAt ( n, 0 );
                    if ( rAccessibleCell.is() )
                    {
                        id cell_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rAccessibleCell -> getAccessibleContext() ];
                        [ cells addObject: cell_wrapper ];
                        [ cell_wrapper release ];
                    }
                }
                pResult = NSAccessibilityUnignoredChildren( cells );
            }
            catch (const Exception &e) 
            {
                pResult = nil;
            }
            [ cells autorelease ];
        }
    }
    
    return pResult;
}

-(id)columnsAttribute
{
    NSArray* pResult = nil;

    XAccessibleTable * accessibleTable = [ self accessibleTable ];
    
    if( accessibleTable )
    {
        sal_Int32 nRows = accessibleTable->getAccessibleRowCount();
        sal_Int32 nCols = accessibleTable->getAccessibleColumnCount();    
        if( nRows * nCols < MAXIMUM_ACCESSIBLE_TABLE_CELLS )
        {
            NSMutableArray * cells = [ [ NSMutableArray alloc ] init ];
            try
            {
                // find out number of columns
                for( sal_Int32 n = 0; n < nCols; n++ )
                {
                    Reference < XAccessible > rAccessibleCell = accessibleTable -> getAccessibleCellAt ( 0, n );
                    if ( rAccessibleCell.is() )
                    {
                        id cell_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rAccessibleCell -> getAccessibleContext() ];
                        [ cells addObject: cell_wrapper ];
                        [ cell_wrapper release ];
                    }
                }
                pResult = NSAccessibilityUnignoredChildren( cells );
            }
            catch (const Exception &e) 
            {
                pResult = nil;
            }
            [ cells autorelease ];
        }
    }
    
    return pResult;
}

@end
