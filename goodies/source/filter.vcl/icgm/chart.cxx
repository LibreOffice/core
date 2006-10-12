/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: chart.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 15:35:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

