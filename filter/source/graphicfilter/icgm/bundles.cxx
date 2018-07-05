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
#include <memory>

void Bundle::SetColor( sal_uInt32 nColor )
{
    mnColor = nColor;
}

FontEntry::FontEntry() :
    pFontName       ( nullptr ),
    eCharSetType    ( CST_CCOMPLETE ),
    pCharSetValue   ( nullptr ),
    nFontType       ( 0 )
{
}

FontEntry::~FontEntry()
{
}

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


CGMFList& CGMFList::operator=( const CGMFList& rSource )
{
    ImplDeleteList();
    nFontsAvailable = rSource.nFontsAvailable;
    nFontNameCount  = rSource.nFontNameCount;
    nCharSetCount   = rSource.nCharSetCount;
    for (auto const & pPtr : rSource.aFontEntryList)
    {
        std::unique_ptr<FontEntry> pCFontEntry(new FontEntry);
        if ( pPtr->pFontName )
        {
            sal_uInt32 nSize = strlen( reinterpret_cast<char*>(pPtr->pFontName.get()) ) + 1;
            pCFontEntry->pFontName.reset( new sal_Int8[ nSize ] );
            memcpy( pCFontEntry->pFontName.get(), pPtr->pFontName.get(), nSize );
        }
        if ( pPtr->pCharSetValue )
        {
            sal_uInt32 nSize = strlen( reinterpret_cast<char*>(pPtr->pCharSetValue.get()) ) + 1;
            pCFontEntry->pCharSetValue.reset( new sal_Int8[ nSize ] );
            memcpy( pCFontEntry->pCharSetValue.get(), pPtr->pCharSetValue.get(), nSize );
        }
        pCFontEntry->eCharSetType = pPtr->eCharSetType;
        pCFontEntry->nFontType = pPtr->nFontType;
        aFontEntryList.push_back( std::move(pCFontEntry) );
    }
    return *this;
}


FontEntry* CGMFList::GetFontEntry( sal_uInt32 nIndex )
{
    sal_uInt32 nInd = nIndex;
    if ( nInd )
        nInd--;
    return ( nInd < aFontEntryList.size() ) ? aFontEntryList[ nInd ].get() : nullptr;
}


static sal_Int8* ImplSearchEntry( sal_Int8* pSource, sal_Int8 const * pDest, sal_uInt32 nComp, sal_uInt32 nSize )
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
    return nullptr;
}

void CGMFList::InsertName( sal_uInt8 const * pSource, sal_uInt32 nSize )
{
    FontEntry* pFontEntry;
    if ( nFontsAvailable == nFontNameCount )
    {
        nFontsAvailable++;
        pFontEntry = new FontEntry;
        aFontEntryList.push_back( std::unique_ptr<FontEntry>(pFontEntry) );
    }
    else
    {
        pFontEntry = aFontEntryList[ nFontNameCount ].get();
    }
    nFontNameCount++;
    std::unique_ptr<sal_Int8[]> pBuf(new sal_Int8[ nSize ]);
    memcpy( pBuf.get(), pSource, nSize );
    sal_Int8* pFound = ImplSearchEntry( pBuf.get(), reinterpret_cast<sal_Int8 const *>("ITALIC"), nSize, 6 );
    if ( pFound )
    {
        pFontEntry->nFontType |= 1;
        sal_uInt32 nPrev = ( pFound - pBuf.get() );
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
            memmove( pFound, pFound + nToCopyOfs, nToCopy );
        }
        nSize -= nToCopyOfs;
    }
    pFound = ImplSearchEntry( pBuf.get(), reinterpret_cast<sal_Int8 const *>("BOLD"), nSize, 4 );
    if ( pFound )
    {
        pFontEntry->nFontType |= 2;

        sal_uInt32 nPrev = ( pFound - pBuf.get() );
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
            memmove( pFound, pFound + nToCopyOfs, nToCopy );
        }
        nSize -= nToCopyOfs;
    }
    pFontEntry->pFontName.reset( new sal_Int8[ nSize + 1 ] );
    pFontEntry->pFontName[ nSize ] = 0;
    memcpy( pFontEntry->pFontName.get(), pBuf.get(), nSize );
}


void CGMFList::InsertCharSet( CharSetType eCharSetType, sal_uInt8 const * pSource, sal_uInt32 nSize )
{
    FontEntry* pFontEntry;
    if ( nFontsAvailable == nCharSetCount )
    {
        nFontsAvailable++;
        pFontEntry = new FontEntry;
        aFontEntryList.push_back( std::unique_ptr<FontEntry>(pFontEntry) );
    }
    else
    {
        pFontEntry = aFontEntryList[ nCharSetCount ].get();
    }
    nCharSetCount++;
    pFontEntry->eCharSetType = eCharSetType;
    pFontEntry->pCharSetValue.reset( new sal_Int8[ nSize + 1 ] );
    pFontEntry->pCharSetValue[ nSize ] = 0;
    memcpy( pFontEntry->pCharSetValue.get(), pSource, nSize );
}


void CGMFList::ImplDeleteList()
{
    aFontEntryList.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
