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
const sal_UCS4 aDefaultUnicodeRanges[] = {0x0020,0xD800, 0xE000,0xFFF0};
const sal_UCS4 aDefaultSymbolRanges[] = {0x0020,0x0100, 0xF020,0xF100};

ImplFontCharMap::~ImplFontCharMap()
{
    if( !isDefaultMap() )
    {
        delete[] mpRangeCodes;
    }
}

ImplFontCharMap::ImplFontCharMap(bool bSymbolic, const sal_UCS4* pRangeCodes, int nRangeCount)
:   mpRangeCodes(pRangeCodes)
,   mnRangeCount(nRangeCount)
,   mnCharCount( 0 )
,   m_bSymbolic(bSymbolic)
{
    const sal_UCS4* pRangePtr = mpRangeCodes;
    for( int i = mnRangeCount; --i >= 0; pRangePtr += 2 )
    {
        sal_UCS4 cFirst = pRangePtr[0];
        sal_UCS4 cLast  = pRangePtr[1];
        mnCharCount += cLast - cFirst;
    }
}

ImplFontCharMapRef const & ImplFontCharMap::getDefaultMap( bool bSymbols )
{
    const sal_UCS4* pRangeCodes = aDefaultUnicodeRanges;
    int nCodesCount = std::size(aDefaultUnicodeRanges);
    if( bSymbols )
    {
        pRangeCodes = aDefaultSymbolRanges;
        nCodesCount = std::size(aDefaultSymbolRanges);
    }

    g_pDefaultImplFontCharMap = ImplFontCharMapRef(new ImplFontCharMap(bSymbols, pRangeCodes, nCodesCount/2));

    return g_pDefaultImplFontCharMap;
}

bool ImplFontCharMap::isDefaultMap() const
{
    const bool bIsDefault = (mpRangeCodes == aDefaultUnicodeRanges) || (mpRangeCodes == aDefaultSymbolRanges);
    return bIsDefault;
}

static unsigned GetUShort(const char* p) { return((p[0]<<8) | p[1]);}

bool HasSymbolCmap(const char* pCmap, int nLength)
{
    // parse the table header and check for validity
    if( !pCmap || (nLength < 24) )
        return false;

    if( GetUShort( pCmap ) != 0x0000 ) // simple check for CMAP corruption
        return false;

    int nSubTables = GetUShort(pCmap + 2);
    if( (nSubTables <= 0) || (nSubTables > (nLength - 24) / 8) )
        return false;

    for (const char* p = pCmap + 4; --nSubTables >= 0; p += 8)
    {
        int nPlatform = GetUShort(p);
        int nEncoding = GetUShort(p + 2);
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

FontCharMap::FontCharMap(bool bSymbolic, const sal_UCS4* pRangeCodes, int nRangeCount)
    : mpImplFontCharMap(new ImplFontCharMap(bSymbolic, pRangeCodes, nRangeCount))
{
}

FontCharMap::~FontCharMap()
{
    mpImplFontCharMap = nullptr;
}

FontCharMapRef FontCharMap::GetDefaultMap( bool bSymbol )
{
    FontCharMapRef xFontCharMap( new FontCharMap( ImplFontCharMap::getDefaultMap( bSymbol ) ) );
    return xFontCharMap;
}

bool FontCharMap::IsDefaultMap() const
{
    return mpImplFontCharMap->isDefaultMap();
}

bool FontCharMap::isSymbolic() const { return mpImplFontCharMap->m_bSymbolic; }

int FontCharMap::GetCharCount() const
{
    return mpImplFontCharMap->mnCharCount;
}

int FontCharMap::CountCharsInRange( sal_UCS4 cMin, sal_UCS4 cMax ) const
{
    int nCount = 0;

    // find and adjust range and char count for cMin
    int nRangeMin = findRangeIndex( cMin );
    if( nRangeMin & 1 )
        ++nRangeMin;
    else if( cMin > mpImplFontCharMap->mpRangeCodes[ nRangeMin ] )
        nCount -= cMin - mpImplFontCharMap->mpRangeCodes[ nRangeMin ];

    // find and adjust range and char count for cMax
    int nRangeMax = findRangeIndex( cMax );
    if( nRangeMax & 1 )
        --nRangeMax;
    else
        nCount -= mpImplFontCharMap->mpRangeCodes[ nRangeMax+1 ] - cMax - 1;

    // count chars in complete ranges between cMin and cMax
    for( int i = nRangeMin; i <= nRangeMax; i+=2 )
        nCount += mpImplFontCharMap->mpRangeCodes[i+1] - mpImplFontCharMap->mpRangeCodes[i];

    return nCount;
}

bool FontCharMap::HasChar( sal_UCS4 cChar ) const
{
    const int nRange = findRangeIndex( cChar );
    if( nRange==0 && cChar < mpImplFontCharMap->mpRangeCodes[0] )
        return false;
    return ((nRange & 1) == 0); // inside a range
}

sal_UCS4 FontCharMap::GetFirstChar() const
{
    return mpImplFontCharMap->mpRangeCodes[0];
}

sal_UCS4 FontCharMap::GetLastChar() const
{
    return (mpImplFontCharMap->mpRangeCodes[ 2*mpImplFontCharMap->mnRangeCount-1 ] - 1);
}

sal_UCS4 FontCharMap::GetNextChar( sal_UCS4 cChar ) const
{
    if( cChar < GetFirstChar() )
        return GetFirstChar();
    if( cChar >= GetLastChar() )
        return GetLastChar();

    int nRange = findRangeIndex( cChar + 1 );
    if( nRange & 1 )                       // outside of range?
        return mpImplFontCharMap->mpRangeCodes[ nRange + 1 ]; // => first in next range
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
        return (mpImplFontCharMap->mpRangeCodes[ nRange ] - 1);    // => last in prev range
    return (cChar - 1);
}

int FontCharMap::GetIndexFromChar( sal_UCS4 cChar ) const
{
    // TODO: improve linear walk?
    int nCharIndex = 0;
    const sal_UCS4* pRange = &mpImplFontCharMap->mpRangeCodes[0];
    for( int i = 0; i < mpImplFontCharMap->mnRangeCount; ++i )
    {
        sal_UCS4 cFirst = *(pRange++);
        sal_UCS4 cLast  = *(pRange++);
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
    const sal_UCS4* pRange = &mpImplFontCharMap->mpRangeCodes[0];
    for( int i = 0; i < mpImplFontCharMap->mnRangeCount; ++i )
    {
        sal_UCS4 cFirst = *(pRange++);
        sal_UCS4 cLast  = *(pRange++);
        nIndex -= cLast - cFirst;
        if( nIndex < 0 )
            return (cLast + nIndex);
    }

    // we can only get here with an out-of-bounds charindex
    return mpImplFontCharMap->mpRangeCodes[0];
}

int FontCharMap::findRangeIndex( sal_UCS4 cChar ) const
{
    int nLower = 0;
    int nMid   = mpImplFontCharMap->mnRangeCount;
    int nUpper = 2 * mpImplFontCharMap->mnRangeCount - 1;
    while( nLower < nUpper )
    {
        if( cChar >= mpImplFontCharMap->mpRangeCodes[ nMid ] )
            nLower = nMid;
        else
            nUpper = nMid - 1;
        nMid = (nLower + nUpper + 1) / 2;
    }

    return nMid;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
