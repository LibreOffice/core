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


#include "bundles.hxx"

#include <tools/stream.hxx>

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

sal_uInt32 Bundle::GetColor() const
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
    eCharSetType    ( CST_CCOMPLETE ),
    pCharSetValue   ( NULL ),
    nFontType       ( 0 )
{
}

FontEntry::~FontEntry()
{
    delete pFontName;
    delete pCharSetValue;
}

// ---------------------------------------------------------------

CGMFList::CGMFList() :
    nFontNameCount      ( 0 ),
    nCharSetCount       ( 0 ),
    nFontsAvailable     ( 0 )
{
    aFontEntryList.clear();
}

CGMFList::~CGMFList()
{
    ImplDeleteList();
}

// ---------------------------------------------------------------

CGMFList& CGMFList::operator=( CGMFList& rSource )
{
    ImplDeleteList();
    nFontsAvailable = rSource.nFontsAvailable;
    nFontNameCount  = rSource.nFontNameCount;
    nCharSetCount   = rSource.nCharSetCount;
    for ( size_t i = 0, n = rSource.aFontEntryList.size(); i < n; ++i )
    {
        FontEntry* pPtr = rSource.aFontEntryList[ i ];
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
        aFontEntryList.push_back( pCFontEntry );
    }
    return *this;
}

// ---------------------------------------------------------------

FontEntry* CGMFList::GetFontEntry( sal_uInt32 nIndex )
{
    sal_uInt32 nInd = nIndex;
    if ( nInd )
        nInd--;
    return ( nInd < aFontEntryList.size() ) ? aFontEntryList[ nInd ] : NULL;
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

void CGMFList::InsertName( sal_uInt8* pSource, sal_uInt32 nSize )
{
    FontEntry* pFontEntry;
    if ( nFontsAvailable == nFontNameCount )
    {
        nFontsAvailable++;
        pFontEntry = new FontEntry;
        aFontEntryList.push_back( pFontEntry );
    }
    else
    {
        pFontEntry = aFontEntryList[ nFontNameCount ];
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
    delete[] pBuf;
}

//--------------------------------------------------------------------------

void CGMFList::InsertCharSet( CharSetType eCharSetType, sal_uInt8* pSource, sal_uInt32 nSize )
{
    FontEntry* pFontEntry;
    if ( nFontsAvailable == nCharSetCount )
    {
        nFontsAvailable++;
        pFontEntry = new FontEntry;
        aFontEntryList.push_back( pFontEntry );
    }
    else
    {
        pFontEntry = aFontEntryList[ nCharSetCount ];
    }
    nCharSetCount++;
    pFontEntry->eCharSetType = eCharSetType;
    pFontEntry->pCharSetValue = new sal_Int8[ nSize + 1 ];
    pFontEntry->pCharSetValue[ nSize ] = 0;
    memcpy( pFontEntry->pCharSetValue, pSource , nSize );
}

// ---------------------------------------------------------------

void CGMFList::ImplDeleteList()
{
    for ( size_t i = 0, n = aFontEntryList.size(); i < n; ++i )
        delete aFontEntryList[ i ];
    aFontEntryList.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
