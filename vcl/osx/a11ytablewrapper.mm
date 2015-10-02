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


#include "osx/a11yfactory.h"

#include "a11ytablewrapper.h"

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
        @try
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
        @catch ( ... )
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
            @try
            {
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
            @catch ( ... )
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
            @try
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
            @catch ( ... )
            {
                pResult = nil;
            }
            [ cells autorelease ];
        }
    }
    
    return pResult;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
