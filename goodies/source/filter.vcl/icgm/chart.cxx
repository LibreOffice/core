/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chart.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_goodies.hxx"

#include <main.hxx>
#include <chart.hxx>

// ---------------------------------------------------------------

CGMChart::CGMChart( CGM& rCGM ) :
    mpCGM       ( &rCGM )
{
    for ( sal_Int8 i = 0; i < 7; i++ )
    {
        mDataNode[ i ].nBoxX1 = mDataNode[ i ].nBoxY1 = 0 ;
        mDataNode[ i ].nBoxX2 = mDataNode[ i ].nBoxY2 = 0 ;

        mDataNode[ i ].nZoneEnum = i;
    }
};

// ---------------------------------------------------------------

CGMChart::~CGMChart()
{
    // delete the whole textentry structure

    TextEntry* pTextEntry;
    while( ( pTextEntry = (TextEntry*)maTextEntryList.First() ) != NULL )
    {
        DeleteTextEntry( pTextEntry );
    }
};

// ---------------------------------------------------------------

void CGMChart::DeleteTextEntry( TextEntry* pTextEntry )
{
    if ( pTextEntry )
    {
        delete pTextEntry->pText;
        for ( TextAttribute* pTAttr = pTextEntry->pAttribute; pTAttr != NULL ; )
        {
            TextAttribute* pTempTAttr = pTAttr;
            pTAttr = pTAttr->pNextAttribute;
            delete pTempTAttr;
        }
        delete pTextEntry;
        maTextEntryList.Remove( pTextEntry );
    }
};

// ---------------------------------------------------------------

void CGMChart::InsertTextEntry( TextEntry* pTextEntry )
{
    maTextEntryList.Insert( pTextEntry );
};

// ---------------------------------------------------------------

TextEntry* CGMChart::GetTextEntry( sal_uInt32 nLine, sal_uInt32 nColumn )
{
    TextEntry* pTextEntry = (TextEntry*)maTextEntryList.First();
    while( pTextEntry && ( ( pTextEntry->nRowOrLineNum != nLine ) && ( pTextEntry->nColumnNum == nColumn ) ) )
    {
        pTextEntry = (TextEntry*)maTextEntryList.Next();
    }
    return (pTextEntry);
};

// ---------------------------------------------------------------

TextEntry* CGMChart::GetFirstTextEntry()
{
    return (TextEntry*)maTextEntryList.First();
};

// ---------------------------------------------------------------
void CGMChart::ResetAnnotation()
{
    mDataNode[ 0 ].nZoneEnum = 0;
}

// ---------------------------------------------------------------

sal_Bool CGMChart::IsAnnotation()
{
    return ( mDataNode[ 0 ].nZoneEnum == 0 );
};

