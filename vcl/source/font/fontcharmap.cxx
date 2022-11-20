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
#include <utility>
#include <vcl/fontcharmap.hxx>
#include <impfontcharmap.hxx>
#include <sal/log.hxx>

#include <algorithm>
#include <vector>

static ImplFontCharMapRef g_pDefaultImplFontCharMap;
const std::vector<sal_uInt32> aDefaultUnicodeRanges = { 0x0020, 0xD800, 0xE000, 0xFFF0 };
const std::vector<sal_uInt32> aDefaultSymbolRanges = { 0x0020, 0x0100, 0xF020, 0xF100 };

ImplFontCharMap::~ImplFontCharMap()
{
}

ImplFontCharMap::ImplFontCharMap(bool bMicrosoftSymbolMap, std::vector<sal_uInt32> aRangeCodes)
:   maRangeCodes(std::move(aRangeCodes))
,   mnCharCount( 0 )
,   m_bMicrosoftSymbolMap(bMicrosoftSymbolMap)
{
    for (size_t i = 0; i < maRangeCodes.size(); i += 2)
    {
        sal_UCS4 cFirst = maRangeCodes[i];
        sal_UCS4 cLast = maRangeCodes[i + 1];
        mnCharCount += cLast - cFirst;
    }
}

ImplFontCharMapRef const & ImplFontCharMap::getDefaultMap(bool bMicrosoftSymbolMap)
{
    const auto& rRanges = bMicrosoftSymbolMap ? aDefaultSymbolRanges : aDefaultUnicodeRanges;
    g_pDefaultImplFontCharMap = ImplFontCharMapRef(new ImplFontCharMap(bMicrosoftSymbolMap, rRanges));
    return g_pDefaultImplFontCharMap;
}

bool ImplFontCharMap::isDefaultMap() const
{
    const bool bIsDefault = (maRangeCodes == aDefaultUnicodeRanges) || (maRangeCodes == aDefaultSymbolRanges);
    return bIsDefault;
}

static unsigned GetUShort(const unsigned char* p) { return((p[0]<<8) | p[1]);}

bool HasMicrosoftSymbolCmap(const unsigned char* pCmap, int nLength)
{
    // parse the table header and check for validity
    if( !pCmap || (nLength < 24) )
        return false;

    if( GetUShort( pCmap ) != 0x0000 ) // simple check for CMAP corruption
        return false;

    int nSubTables = GetUShort(pCmap + 2);
    if( (nSubTables <= 0) || (nSubTables > (nLength - 24) / 8) )
        return false;

    for (const unsigned char* p = pCmap + 4; --nSubTables >= 0; p += 8)
    {
        int nPlatform = GetUShort(p);
        int nEncoding = GetUShort(p + 2);
        // https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
        // When the platformID is 3 (Windows), an encoding of 0 is Symbol
        if (nPlatform == 3 && nEncoding == 0)
            return true;
    }

    return false;
}

FontCharMap::FontCharMap()
    : mpImplFontCharMap( ImplFontCharMap::getDefaultMap() )
{
}

FontCharMap::FontCharMap( ImplFontCharMapRef pIFCMap )
    : mpImplFontCharMap(std::move( pIFCMap ))
{
}

FontCharMap::FontCharMap(bool bMicrosoftSymbolMap, std::vector<sal_uInt32> aRangeCodes)
    : mpImplFontCharMap(new ImplFontCharMap(bMicrosoftSymbolMap, std::move(aRangeCodes)))
{
}

FontCharMap::~FontCharMap()
{
    mpImplFontCharMap = nullptr;
}

FontCharMapRef FontCharMap::GetDefaultMap(bool bMicrosoftSymbolMap)
{
    FontCharMapRef xFontCharMap( new FontCharMap( ImplFontCharMap::getDefaultMap(bMicrosoftSymbolMap) ) );
    return xFontCharMap;
}

bool FontCharMap::IsDefaultMap() const
{
    return mpImplFontCharMap->isDefaultMap();
}

bool FontCharMap::isMicrosoftSymbolMap() const { return mpImplFontCharMap->m_bMicrosoftSymbolMap; }

int FontCharMap::GetCharCount() const
{
    return mpImplFontCharMap->mnCharCount;
}

int FontCharMap::CountCharsInRange( sal_UCS4 cMin, sal_UCS4 cMax ) const
{
    const auto& rRanges = mpImplFontCharMap->maRangeCodes;
    int nCount = 0;

    // find and adjust range and char count for cMin
    int nRangeMin = findRangeIndex( cMin );
    if( nRangeMin & 1 )
        ++nRangeMin;
    else if (cMin > rRanges[nRangeMin])
        nCount -= cMin - rRanges[nRangeMin];

    // find and adjust range and char count for cMax
    int nRangeMax = findRangeIndex( cMax );
    if( nRangeMax & 1 )
        --nRangeMax;
    else
        nCount -= rRanges[nRangeMax + 1] - cMax - 1;

    // count chars in complete ranges between cMin and cMax
    for( int i = nRangeMin; i <= nRangeMax; i+=2 )
        nCount += rRanges[i + 1] - rRanges[i];

    return nCount;
}

bool FontCharMap::HasChar( sal_UCS4 cChar ) const
{
    const int nRange = findRangeIndex( cChar );
    if (nRange==0 && cChar < mpImplFontCharMap->maRangeCodes[0])
        return false;
    return ((nRange & 1) == 0); // inside a range
}

sal_UCS4 FontCharMap::GetFirstChar() const
{
    return mpImplFontCharMap->maRangeCodes.front();
}

sal_UCS4 FontCharMap::GetLastChar() const
{
    return mpImplFontCharMap->maRangeCodes.back() - 1;
}

sal_UCS4 FontCharMap::GetNextChar( sal_UCS4 cChar ) const
{
    if( cChar < GetFirstChar() )
        return GetFirstChar();
    if( cChar >= GetLastChar() )
        return GetLastChar();

    int nRange = findRangeIndex( cChar + 1 );
    if( nRange & 1 )                       // outside of range?
        return mpImplFontCharMap->maRangeCodes[nRange + 1]; // => first in next range
    return (cChar + 1);
}

sal_UCS4 FontCharMap::GetPrevChar( sal_UCS4 cChar ) const
{
    if( cChar <= GetFirstChar() )
        return GetFirstChar();
    if( cChar > GetLastChar() )
        return GetLastChar();

    int nRange = findRangeIndex( cChar - 1 );
    if( nRange & 1 )                            // outside a range?
        return mpImplFontCharMap->maRangeCodes[nRange] - 1;    // => last in prev range
    return (cChar - 1);
}

int FontCharMap::GetIndexFromChar( sal_UCS4 cChar ) const
{
    // TODO: improve linear walk?
    int nCharIndex = 0;
    const auto& rRanges = mpImplFontCharMap->maRangeCodes;
    for (size_t i = 0; i < rRanges.size(); i += 2)
    {
        sal_UCS4 cFirst = rRanges[i];
        sal_UCS4 cLast = rRanges[i + 1];
        if( cChar >= cLast )
            nCharIndex += cLast - cFirst;
        else if( cChar >= cFirst )
            return nCharIndex + (cChar - cFirst);
        else
            break;
    }

    return -1;
}

sal_UCS4 FontCharMap::GetCharFromIndex( int nIndex ) const
{
    // TODO: improve linear walk?
    const auto& rRanges = mpImplFontCharMap->maRangeCodes;
    for (size_t i = 0; i < rRanges.size(); i += 2)
    {
        sal_UCS4 cFirst = rRanges[i];
        sal_UCS4 cLast = rRanges[i + 1];
        nIndex -= cLast - cFirst;
        if( nIndex < 0 )
            return (cLast + nIndex);
    }

    // we can only get here with an out-of-bounds charindex
    return mpImplFontCharMap->maRangeCodes.front();
}

int FontCharMap::findRangeIndex( sal_UCS4 cChar ) const
{
    const auto& rRanges = mpImplFontCharMap->maRangeCodes;
    int nLower = 0;
    int nMid = rRanges.size() / 2;
    int nUpper = rRanges.size() - 1;
    while( nLower < nUpper )
    {
        if (cChar >= rRanges[nMid])
            nLower = nMid;
        else
            nUpper = nMid - 1;
        nMid = (nLower + nUpper + 1) / 2;
    }

    return nMid;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
