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

#include <memory>
#include <string.h>

void Bundle::SetColor( sal_uInt32 nColor )
{
    mnColor = nColor;
}

CGMFList::CGMFList()
    : nFontNameCount(0)
    , nCharSetCount(0)
{
}

FontEntry* CGMFList::GetFontEntry( sal_uInt32 nIndex )
{
    sal_uInt32 nInd = nIndex;
    if ( nInd )
        nInd--;
    return ( nInd < aFontEntryList.size() ) ? &aFontEntryList[nInd] : nullptr;
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
    if (nFontNameCount == aFontEntryList.size())
    {
        aFontEntryList.push_back(FontEntry());
        pFontEntry = &aFontEntryList.back();
    }
    else
    {
        pFontEntry = &aFontEntryList[nFontNameCount];
    }
    nFontNameCount++;

    if (nSize == 0)
        return;

    std::vector<sal_Int8> aBuf(pSource, pSource + nSize);
    sal_Int8* pFound = ImplSearchEntry(aBuf.data(), reinterpret_cast<sal_Int8 const *>("ITALIC"), nSize, 6);
    if (pFound)
    {
        pFontEntry->nFontType |= 1;
        sal_uInt32 nPrev = pFound - aBuf.data();
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
    pFound = ImplSearchEntry(aBuf.data(), reinterpret_cast<sal_Int8 const *>("BOLD"), nSize, 4);
    if ( pFound )
    {
        pFontEntry->nFontType |= 2;

        sal_uInt32 nPrev = pFound - aBuf.data();
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
    pFontEntry->aFontName.assign(aBuf.data(), aBuf.data() + nSize);
}

void CGMFList::InsertCharSet( sal_uInt8 const * pSource, sal_uInt32 nSize )
{
    FontEntry* pFontEntry;
    if (nCharSetCount == aFontEntryList.size())
    {
        aFontEntryList.push_back(FontEntry());
        pFontEntry = &aFontEntryList.back();
    }
    else
    {
        pFontEntry = &aFontEntryList[nCharSetCount];
    }
    nCharSetCount++;

    if (nSize == 0)
        return;

    pFontEntry->aCharSetValue.assign(pSource, pSource + nSize);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
