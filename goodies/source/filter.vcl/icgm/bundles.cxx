/*************************************************************************
 *
 *  $RCSfile: bundles.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "bundles.hxx"

#include <vcl/salbtype.hxx>
#include <tools/stream.hxx>
#include <tools/list.hxx>

Bundle& Bundle::operator=( Bundle& rSource )
{
    mnColor = rSource.mnColor;
    mnBundleIndex = rSource.mnBundleIndex;
    return *this;
};

// ---------------------------------------------------------------

void Bundle::SetColor( sal_uInt32 nColor )
{
    mnColor = nColor;
}

sal_uInt32 Bundle::GetColor()
{
    return mnColor;
}

// ---------------------------------------------------------------

LineBundle& LineBundle::operator=( LineBundle& rSource )
{
    SetIndex( rSource.GetIndex() );
    eLineType = rSource.eLineType;
    nLineWidth = rSource.nLineWidth;
    return *this;
};

MarkerBundle& MarkerBundle::operator=( MarkerBundle& rSource )
{
    SetIndex( rSource.GetIndex() );
    eMarkerType = rSource.eMarkerType;
    nMarkerSize = rSource.nMarkerSize;
    return *this;
};

EdgeBundle& EdgeBundle::operator=( EdgeBundle& rSource )
{
    SetIndex( rSource.GetIndex() );
    eEdgeType = rSource.eEdgeType;
    nEdgeWidth = rSource.nEdgeWidth;
    return *this;
};

TextBundle& TextBundle::operator=( TextBundle& rSource )
{
    SetIndex( rSource.GetIndex() );
    nTextFontIndex = rSource.nTextFontIndex;
    eTextPrecision = rSource.eTextPrecision;
    nCharacterExpansion = rSource.nCharacterExpansion;
    nCharacterSpacing = rSource.nCharacterSpacing;
    return *this;
};

FillBundle& FillBundle::operator=( FillBundle& rSource )
{
    SetIndex( rSource.GetIndex() );
    eFillInteriorStyle = rSource.eFillInteriorStyle;
    nFillPatternIndex = rSource.nFillPatternIndex;
    nFillHatchIndex = rSource.nFillHatchIndex;
    return *this;
};

// ---------------------------------------------------------------

FontEntry::FontEntry() :
    pFontName       ( NULL ),
    pCharSetValue   ( NULL ),
    eCharSetType    ( CST_CCOMPLETE ),
    nFontType       ( 0 )
{
}

FontEntry::~FontEntry()
{
    delete pFontName;
    delete pCharSetValue;
}

// ---------------------------------------------------------------

FontList::FontList() :
    nFontsAvailable     ( 0 ),
    nFontNameCount      ( 0 ),
    nCharSetCount       ( 0 )
{
    aFontEntryList.Clear();
}

FontList::~FontList()
{
    ImplDeleteList();
}

// ---------------------------------------------------------------

FontList& FontList::operator=( FontList& rSource )
{
    ImplDeleteList();
    nFontsAvailable = rSource.nFontsAvailable;
    nFontNameCount = rSource.nFontNameCount;
    nCharSetCount = rSource.nCharSetCount;
    FontEntry* pPtr = (FontEntry*)rSource.aFontEntryList.First();
    while( pPtr )
    {
        FontEntry* pCFontEntry = new FontEntry;
        if ( pPtr->pFontName )
        {
            sal_uInt32 nSize = strlen( (const char*)pPtr->pFontName ) + 1;
            pCFontEntry->pFontName = new sal_Int8[ nSize ];
            memcpy( pCFontEntry->pFontName, pPtr->pFontName, nSize );
        }
        if ( pPtr->pCharSetValue )
        {
            sal_uInt32 nSize = strlen( (const char*)pPtr->pCharSetValue ) + 1;
            pCFontEntry->pCharSetValue = new sal_Int8[ nSize ];
            memcpy( pCFontEntry->pCharSetValue, pPtr->pCharSetValue, nSize );
        }
        pCFontEntry->eCharSetType = pPtr->eCharSetType;
        pCFontEntry->nFontType = pPtr->nFontType;
        aFontEntryList.Insert( pCFontEntry, LIST_APPEND );
        pPtr = (FontEntry*)rSource.aFontEntryList.Next();
    }
    return *this;
}

// ---------------------------------------------------------------

FontEntry* FontList::GetFontEntry( sal_uInt32 nIndex )
{
    sal_uInt32 nInd = nIndex;
    if ( nInd )
        nInd--;
    return (FontEntry*)aFontEntryList.GetObject( nInd );
}

// ---------------------------------------------------------------

static sal_Int8* ImplSearchEntry( sal_Int8* pSource, sal_Int8* pDest, sal_uInt32 nComp, sal_uInt32 nSize )
{
    while ( nComp-- >= nSize )
    {
        sal_uInt32 i;
        for ( i = 0; i < nSize; i++ )
        {
            if ( ( pSource[i]&~0x20 ) != ( pDest[i]&~0x20 ) )
                break;
        }
        if ( i == nSize )
            return pSource;
        pSource++;
    }
    return NULL;
}

void FontList::InsertName( sal_uInt8* pSource, sal_uInt32 nSize )
{
    FontEntry* pFontEntry;
    if ( nFontsAvailable == nFontNameCount )
    {
        nFontsAvailable++;
        pFontEntry = new FontEntry;
        aFontEntryList.Insert( pFontEntry, LIST_APPEND );
    }
    else
    {
        pFontEntry = (FontEntry*)aFontEntryList.GetObject( nFontNameCount );
    }
    nFontNameCount++;
    sal_Int8* pBuf = new sal_Int8[ nSize ];
    memcpy( pBuf, pSource, nSize );
    sal_Int8* pFound = ImplSearchEntry( pBuf, (sal_Int8*)"ITALIC", nSize, 6 );
    if ( pFound )
    {
        pFontEntry->nFontType |= 1;
        sal_uInt32 nPrev = ( pFound - pBuf );
        sal_uInt32 nToCopyOfs = 6;
        if ( nPrev && ( pFound[ -1 ] == '-' || pFound[ -1 ] == ' ' ) )
        {
            nPrev--;
            pFound--;
            nToCopyOfs++;
        }
        sal_uInt32 nToCopy = nSize - nToCopyOfs - nPrev;
        if ( nToCopy )
        {
            memcpy( pFound, pFound + nToCopyOfs, nToCopy );
        }
        nSize -= nToCopyOfs;
    }
    pFound = ImplSearchEntry( pBuf, (sal_Int8*)"BOLD", nSize, 4 );
    if ( pFound )
    {
        pFontEntry->nFontType |= 2;

        sal_uInt32 nPrev = ( pFound - pBuf );
        sal_uInt32 nToCopyOfs = 4;
        if ( nPrev && ( pFound[ -1 ] == '-' || pFound[ -1 ] == ' ' ) )
        {
            nPrev--;
            pFound--;
            nToCopyOfs++;
        }
        sal_uInt32 nToCopy = nSize - nToCopyOfs - nPrev;
        if ( nToCopy )
        {
            memcpy( pFound, pFound + nToCopyOfs, nToCopy );
        }
        nSize -= nToCopyOfs;
    }
    pFontEntry->pFontName = new sal_Int8[ nSize + 1 ];
    pFontEntry->pFontName[ nSize ] = 0;
    memcpy( pFontEntry->pFontName, pBuf, nSize );
    delete pBuf;
}

//--------------------------------------------------------------------------

void FontList::InsertCharSet( CharSetType eCharSetType, sal_uInt8* pSource, sal_uInt32 nSize )
{
    FontEntry* pFontEntry;
    if ( nFontsAvailable == nCharSetCount )
    {
        nFontsAvailable++;
        pFontEntry = new FontEntry;
        aFontEntryList.Insert( pFontEntry, LIST_APPEND );
    }
    else
    {
        pFontEntry = (FontEntry*)aFontEntryList.GetObject( nCharSetCount );
    }
    nCharSetCount++;
    pFontEntry->eCharSetType = eCharSetType;
    pFontEntry->pCharSetValue = new sal_Int8[ nSize + 1 ];
    pFontEntry->pCharSetValue[ nSize ] = 0;
    memcpy( pFontEntry->pCharSetValue, pSource , nSize );
}

// ---------------------------------------------------------------

void FontList::ImplDeleteList()
{
    FontEntry* pFontEntry = (FontEntry*)aFontEntryList.First();
    while( pFontEntry )
    {
        delete pFontEntry;
        pFontEntry = (FontEntry*)aFontEntryList.Next();
    }
    aFontEntryList.Clear();
}

