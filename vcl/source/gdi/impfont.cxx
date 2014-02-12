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
#include <vcl/metric.hxx>
#include <outfont.hxx>
#include <impfont.hxx>

#include <vector>
#include <set>

// =======================================================================

CmapResult::CmapResult( bool bSymbolic,
    const sal_UCS4* pRangeCodes, int nRangeCount,
    const int* pStartGlyphs, const sal_uInt16* pExtraGlyphIds )
:   mpRangeCodes( pRangeCodes)
,   mpStartGlyphs( pStartGlyphs)
,   mpGlyphIds( pExtraGlyphIds)
,   mnRangeCount( nRangeCount)
,   mbSymbolic( bSymbolic)
,   mbRecoded( false)
{}

// =======================================================================

ImplFontCharMap::ImplFontCharMap( const CmapResult& rCR )
:   mpRangeCodes( rCR.mpRangeCodes )
,   mpStartGlyphs( rCR.mpStartGlyphs )
,   mpGlyphIds( rCR.mpGlyphIds )
,   mnRangeCount( rCR.mnRangeCount )
,   mnCharCount( 0 )
,   mnRefCount( 1 )
{
    const sal_UCS4* pRangePtr = mpRangeCodes;
    for( int i = mnRangeCount; --i >= 0; pRangePtr += 2 )
    {
        sal_UCS4 cFirst = pRangePtr[0];
        sal_UCS4 cLast  = pRangePtr[1];
        mnCharCount += cLast - cFirst;
    }
}

static ImplFontCharMap* pDefaultImplFontCharMap = NULL;
static const sal_UCS4 aDefaultUnicodeRanges[] = {0x0020,0xD800, 0xE000,0xFFF0};
static const sal_UCS4 aDefaultSymbolRanges[] = {0x0020,0x0100, 0xF020,0xF100};

// -----------------------------------------------------------------------

bool ImplFontCharMap::IsDefaultMap() const
{
    const bool bIsDefault = (mpRangeCodes == aDefaultUnicodeRanges) || (mpRangeCodes == aDefaultSymbolRanges);
    return bIsDefault;
}

// -----------------------------------------------------------------------

ImplFontCharMap::~ImplFontCharMap()
{
    if( IsDefaultMap() )
        return;
    delete[] mpRangeCodes;
    delete[] mpStartGlyphs;
    delete[] mpGlyphIds;
}

// -----------------------------------------------------------------------

ImplFontCharMap* ImplFontCharMap::GetDefaultMap( bool bSymbols)
{
    if( pDefaultImplFontCharMap )
        pDefaultImplFontCharMap->AddReference();
    else
    {
        const sal_UCS4* pRangeCodes = aDefaultUnicodeRanges;
        int nCodesCount = sizeof(aDefaultUnicodeRanges) / sizeof(*pRangeCodes);
        if( bSymbols )
        {
            pRangeCodes = aDefaultSymbolRanges;
            nCodesCount = sizeof(aDefaultSymbolRanges) / sizeof(*pRangeCodes);
        }

        CmapResult aDefaultCR( bSymbols, pRangeCodes, nCodesCount/2 );
        pDefaultImplFontCharMap = new ImplFontCharMap( aDefaultCR );
    }

    return pDefaultImplFontCharMap;
}

// -----------------------------------------------------------------------

void ImplFontCharMap::AddReference( void) const
{
    ++mnRefCount;
}

// -----------------------------------------------------------------------

void ImplFontCharMap::DeReference( void) const
{
    if( --mnRefCount <= 0 )
        if( this != pDefaultImplFontCharMap )
            delete this;
}

// -----------------------------------------------------------------------

int ImplFontCharMap::GetCharCount() const
{
    return mnCharCount;
}

// -----------------------------------------------------------------------

int ImplFontCharMap::ImplFindRangeIndex( sal_UCS4 cChar ) const
{
    int nLower = 0;
    int nMid   = mnRangeCount;
    int nUpper = 2 * mnRangeCount - 1;
    while( nLower < nUpper )
    {
        if( cChar >= mpRangeCodes[ nMid ] )
            nLower = nMid;
        else
            nUpper = nMid - 1;
        nMid = (nLower + nUpper + 1) / 2;
    }

    return nMid;
}

// -----------------------------------------------------------------------

bool ImplFontCharMap::HasChar( sal_UCS4 cChar ) const
{
    bool bHasChar = false;

    if( mpStartGlyphs  == NULL ) { // only the char-ranges are known
        const int nRange = ImplFindRangeIndex( cChar );
        if( nRange==0 && cChar<mpRangeCodes[0] )
            return false;
        bHasChar = ((nRange & 1) == 0); // inside a range
    } else { // glyph mapping is available
        const int nGlyphIndex = GetGlyphIndex( cChar );
        bHasChar = (nGlyphIndex != 0); // not the notdef-glyph
    }

    return bHasChar;
}

// -----------------------------------------------------------------------

int ImplFontCharMap::GetGlyphIndex( sal_UCS4 cChar ) const
{
    // return -1 if the object doesn't know the glyph ids
    if( !mpStartGlyphs )
        return -1;

    // return 0 if the unicode doesn't have a matching glyph
    int nRange = ImplFindRangeIndex( cChar );
    // check that we are inside any range
    if( (nRange == 0) && (cChar < mpRangeCodes[0]) ) {
        // symbol aliasing gives symbol fonts a second chance
        const bool bSymbolic = (mpRangeCodes[0]>=0xF000) && (mpRangeCodes[1]<=0xF0FF);
        if( !bSymbolic )
            return 0;
        // check for symbol aliasing (U+F0xx -> U+00xx)
        nRange = ImplFindRangeIndex( cChar | 0xF000 );
    }
    // check that we are inside a range
    if( (nRange & 1) != 0 )
        return 0;

    // get glyph index directly or indirectly
    int nGlyphIndex = cChar - mpRangeCodes[ nRange ];
    const int nStartIndex = mpStartGlyphs[ nRange/2 ];
    if( nStartIndex >= 0 ) {
        // the glyph index can be calculated
        nGlyphIndex += nStartIndex;
    } else {
        // the glyphid array has the glyph index
        nGlyphIndex = mpGlyphIds[ nGlyphIndex - nStartIndex];
    }

    return nGlyphIndex;
}

// -----------------------------------------------------------------------

// returns the number of chars supported by the font, which
// are inside the unicode range from cMin to cMax (inclusive)
int ImplFontCharMap::CountCharsInRange( sal_UCS4 cMin, sal_UCS4 cMax ) const
{
    int nCount = 0;

    // find and adjust range and char count for cMin
    int nRangeMin = ImplFindRangeIndex( cMin );
    if( nRangeMin & 1 )
        ++nRangeMin;
    else if( cMin > mpRangeCodes[ nRangeMin ] )
        nCount -= cMin - mpRangeCodes[ nRangeMin ];

    // find and adjust range and char count for cMax
    int nRangeMax = ImplFindRangeIndex( cMax );
    if( nRangeMax & 1 )
        --nRangeMax;
    else
        nCount -= mpRangeCodes[ nRangeMax+1 ] - cMax - 1;

    // count chars in complete ranges between cMin and cMax
    for( int i = nRangeMin; i <= nRangeMax; i+=2 )
        nCount += mpRangeCodes[i+1] - mpRangeCodes[i];

    return nCount;
}

// -----------------------------------------------------------------------

sal_UCS4 ImplFontCharMap::GetFirstChar() const
{
    return mpRangeCodes[0];
}

// -----------------------------------------------------------------------

sal_UCS4 ImplFontCharMap::GetLastChar() const
{
    return (mpRangeCodes[ 2*mnRangeCount-1 ] - 1);
}

// -----------------------------------------------------------------------

sal_UCS4 ImplFontCharMap::GetNextChar( sal_UCS4 cChar ) const
{
    if( cChar < GetFirstChar() )
        return GetFirstChar();
    if( cChar >= GetLastChar() )
        return GetLastChar();

    int nRange = ImplFindRangeIndex( cChar + 1 );
    if( nRange & 1 )                       // outside of range?
        return mpRangeCodes[ nRange + 1 ]; // => first in next range
    return (cChar + 1);
}

// -----------------------------------------------------------------------

sal_UCS4 ImplFontCharMap::GetPrevChar( sal_UCS4 cChar ) const
{
    if( cChar <= GetFirstChar() )
        return GetFirstChar();
    if( cChar > GetLastChar() )
        return GetLastChar();

    int nRange = ImplFindRangeIndex( cChar - 1 );
    if( nRange & 1 )                            // outside a range?
        return (mpRangeCodes[ nRange ] - 1);    // => last in prev range
    return (cChar - 1);
}

// -----------------------------------------------------------------------

int ImplFontCharMap::GetIndexFromChar( sal_UCS4 cChar ) const
{
    // TODO: improve linear walk?
    int nCharIndex = 0;
    const sal_UCS4* pRange = &mpRangeCodes[0];
    for( int i = 0; i < mnRangeCount; ++i )
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

// -----------------------------------------------------------------------

sal_UCS4 ImplFontCharMap::GetCharFromIndex( int nCharIndex ) const
{
    // TODO: improve linear walk?
    const sal_UCS4* pRange = &mpRangeCodes[0];
    for( int i = 0; i < mnRangeCount; ++i )
    {
        sal_UCS4 cFirst = *(pRange++);
        sal_UCS4 cLast  = *(pRange++);
        nCharIndex -= cLast - cFirst;
        if( nCharIndex < 0 )
            return (cLast + nCharIndex);
    }

    // we can only get here with an out-of-bounds charindex
    return mpRangeCodes[0];
}

// =======================================================================

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static unsigned Getsal_uInt16( const unsigned char* p ){ return((p[0]<<8) | p[1]);}
static int GetSShort( const unsigned char* p ){ return((static_cast<signed char>(p[0])<<8)|p[1]);}

// TODO: move CMAP parsing directly into the ImplFontCharMap class
bool ParseCMAP( const unsigned char* pCmap, int nLength, CmapResult& rResult )
{
    rResult.mpRangeCodes = NULL;
    rResult.mpStartGlyphs= NULL;
    rResult.mpGlyphIds   = NULL;
    rResult.mnRangeCount = 0;
    rResult.mbRecoded    = false;
    rResult.mbSymbolic   = false;

    // parse the table header and check for validity
    if( !pCmap || (nLength < 24) )
        return false;

    if( Getsal_uInt16( pCmap ) != 0x0000 ) // simple check for CMAP corruption
        return false;

    int nSubTables = Getsal_uInt16( pCmap + 2 );
    if( (nSubTables <= 0) || (nLength < (24 + 8*nSubTables)) )
        return false;

    // find the most interesting subtable in the CMAP
    rtl_TextEncoding eRecodeFrom = RTL_TEXTENCODING_UNICODE;
    int nOffset = 0;
    int nFormat = -1;
    int nBestVal = 0;
    for( const unsigned char* p = pCmap + 4; --nSubTables >= 0; p += 8 )
    {
        int nPlatform = Getsal_uInt16( p );
        int nEncoding = Getsal_uInt16( p+2 );
        int nPlatformEncoding = (nPlatform << 8) + nEncoding;

        int nValue;
        rtl_TextEncoding eTmpEncoding = RTL_TEXTENCODING_UNICODE;
        switch( nPlatformEncoding )
        {
            case 0x000: nValue = 20; break;                             // Unicode 1.0
            case 0x001: nValue = 21; break;                             // Unicode 1.1
            case 0x002: nValue = 22; break;                             // iso10646_1993
            case 0x003: nValue = 23; break;                             // UCS-2
            case 0x004: nValue = 24; break;                             // UCS-4
            case 0x100: nValue = 22; break;                             // Mac Unicode<2.0
            case 0x103: nValue = 23; break;                             // Mac Unicode>2.0
            case 0x300: nValue =  5; rResult.mbSymbolic = true; break;  // Win Symbol
            case 0x301: nValue = 28; break;                             // Win UCS-2
            case 0x30A: nValue = 29; break;                             // Win-UCS-4
            case 0x302: nValue = 11; eTmpEncoding = RTL_TEXTENCODING_SHIFT_JIS; break;
            case 0x303: nValue = 12; eTmpEncoding = RTL_TEXTENCODING_GB_18030; break;
            case 0x304: nValue = 11; eTmpEncoding = RTL_TEXTENCODING_BIG5; break;
            case 0x305: nValue = 11; eTmpEncoding = RTL_TEXTENCODING_MS_949; break;
            case 0x306: nValue = 11; eTmpEncoding = RTL_TEXTENCODING_MS_1361; break;
            default:    nValue = 0; break;
        }

        if( nValue <= 0 )   // ignore unknown encodings
            continue;

        int nTmpOffset = GetUInt( p+4 );
        int nTmpFormat = Getsal_uInt16( pCmap + nTmpOffset );
        if( nTmpFormat == 12 )                  // 32bit code -> glyph map format
            nValue += 3;
        else if( nTmpFormat != 4 )              // 16bit code -> glyph map format
            continue;                           // ignore other formats

        if( nBestVal < nValue )
        {
            nBestVal = nValue;
            nOffset = nTmpOffset;
            nFormat = nTmpFormat;
            eRecodeFrom = eTmpEncoding;
        }
    }

    // parse the best CMAP subtable
    int nRangeCount = 0;
    sal_UCS4* pCodePairs = NULL;
    int* pStartGlyphs = NULL;

    typedef std::vector<sal_uInt16> U16Vector;
    U16Vector aGlyphIdArray;
    aGlyphIdArray.reserve( 0x1000 );
    aGlyphIdArray.push_back( 0 );

    // format 4, the most common 16bit char mapping table
    if( (nFormat == 4) && ((nOffset+16) < nLength) )
    {
        int nSegCountX2 = Getsal_uInt16( pCmap + nOffset + 6 );
        nRangeCount = nSegCountX2/2 - 1;
        pCodePairs = new sal_UCS4[ nRangeCount * 2 ];
        pStartGlyphs = new int[ nRangeCount ];
        const unsigned char* pLimitBase = pCmap + nOffset + 14;
        const unsigned char* pBeginBase = pLimitBase + nSegCountX2 + 2;
        const unsigned char* pDeltaBase = pBeginBase + nSegCountX2;
        const unsigned char* pOffsetBase = pDeltaBase + nSegCountX2;
        sal_UCS4* pCP = pCodePairs;
        for( int i = 0; i < nRangeCount; ++i )
        {
            const sal_UCS4 cMinChar = Getsal_uInt16( pBeginBase + 2*i );
            const sal_UCS4 cMaxChar = Getsal_uInt16( pLimitBase + 2*i );
            const int nGlyphDelta  = GetSShort( pDeltaBase + 2*i );
            const int nRangeOffset = Getsal_uInt16( pOffsetBase + 2*i );
            if( cMinChar > cMaxChar ) {  // no sane font should trigger this
                SAL_WARN("vcl.gdi", "Min char should never be more than the max char!");
                break;
            }
            if( cMaxChar == 0xFFFF ) {
                SAL_WARN("vcl.gdi", "Format 4 char should not be 0xFFFF");
                break;
            }
            *(pCP++) = cMinChar;
            *(pCP++) = cMaxChar + 1;
            if( !nRangeOffset ) {
                // glyphid can be calculated directly
                pStartGlyphs[i] = (cMinChar + nGlyphDelta) & 0xFFFF;
            } else {
                // update the glyphid-array with the glyphs in this range
                pStartGlyphs[i] = -(int)aGlyphIdArray.size();
                const unsigned char* pGlyphIdPtr = pOffsetBase + 2*i + nRangeOffset;
                for( sal_UCS4 c = cMinChar; c <= cMaxChar; ++c, pGlyphIdPtr+=2 ) {
                    const int nGlyphIndex = Getsal_uInt16( pGlyphIdPtr ) + nGlyphDelta;
                    aGlyphIdArray.push_back( static_cast<sal_uInt16>(nGlyphIndex) );
                }
            }
        }
        nRangeCount = (pCP - pCodePairs) / 2;
    }
    // format 12, the most common 32bit char mapping table
    else if( (nFormat == 12) && ((nOffset+16) < nLength) )
    {
        nRangeCount = GetUInt( pCmap + nOffset + 12 );
        pCodePairs = new sal_UCS4[ nRangeCount * 2 ];
        pStartGlyphs = new int[ nRangeCount ];
        const unsigned char* pGroup = pCmap + nOffset + 16;
        sal_UCS4* pCP = pCodePairs;
        for( int i = 0; i < nRangeCount; ++i )
        {
            sal_UCS4 cMinChar = GetUInt( pGroup + 0 );
            sal_UCS4 cMaxChar = GetUInt( pGroup + 4 );
            int nGlyphId = GetUInt( pGroup + 8 );
            pGroup += 12;

            if( cMinChar > cMaxChar ) {   // no sane font should trigger this
                SAL_WARN("vcl.gdi", "Min char should never be more than the max char!");
                break;
            }

            *(pCP++) = cMinChar;
            *(pCP++) = cMaxChar + 1;
            pStartGlyphs[i] = nGlyphId;
        }
        nRangeCount = (pCP - pCodePairs) / 2;
    }

    // check if any subtable resulted in something usable
    if( nRangeCount <= 0 )
    {
        delete[] pCodePairs;
        delete[] pStartGlyphs;

        // even when no CMAP is available we know it for symbol fonts
        if( rResult.mbSymbolic )
        {
            pCodePairs = new sal_UCS4[4];
            pCodePairs[0] = 0x0020;    // aliased symbols
            pCodePairs[1] = 0x0100;
            pCodePairs[2] = 0xF020;    // original symbols
            pCodePairs[3] = 0xF100;
            rResult.mpRangeCodes = pCodePairs;
            rResult.mnRangeCount = 2;
            return true;
        }

        return false;
    }

    // recode the code ranges to their unicode encoded ranges if needed
    rtl_TextToUnicodeConverter aConverter = NULL;
    rtl_UnicodeToTextContext aCvtContext = NULL;

    rResult.mbRecoded = ( eRecodeFrom != RTL_TEXTENCODING_UNICODE );
    if( rResult.mbRecoded )
    {
        aConverter = rtl_createTextToUnicodeConverter( eRecodeFrom );
        aCvtContext = rtl_createTextToUnicodeContext( aConverter );
    }

    if( aConverter && aCvtContext )
    {
        // determine the set of supported unicodes from encoded ranges
        typedef std::set<sal_UCS4> Ucs4Set;
        Ucs4Set aSupportedUnicodes;

        static const int NINSIZE = 64;
        static const int NOUTSIZE = 64;
        sal_Char    cCharsInp[ NINSIZE ];
        sal_Unicode cCharsOut[ NOUTSIZE ];
        sal_UCS4* pCP = pCodePairs;
        for( int i = 0; i < nRangeCount; ++i )
        {
            sal_UCS4 cMin = *(pCP++);
            sal_UCS4 cEnd = *(pCP++);
            while( cMin < cEnd )
            {
                int j = 0;
                for(; (cMin < cEnd) && (j < NINSIZE); ++cMin )
                {
                    if( cMin >= 0x0100 )
                        cCharsInp[ j++ ] = static_cast<sal_Char>(cMin >> 8);
                    if( (cMin >= 0x0100) || (cMin < 0x00A0)  )
                        cCharsInp[ j++ ] = static_cast<sal_Char>(cMin);
                }

                sal_uInt32 nCvtInfo;
                sal_Size nSrcCvtBytes;
                int nOutLen = rtl_convertTextToUnicode(
                    aConverter, aCvtContext,
                    cCharsInp, j, cCharsOut, NOUTSIZE,
                    RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE
                    | RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE,
                    &nCvtInfo, &nSrcCvtBytes );

                for( j = 0; j < nOutLen; ++j )
                    aSupportedUnicodes.insert( cCharsOut[j] );
            }
        }

        rtl_destroyTextToUnicodeConverter( aCvtContext );
        rtl_destroyTextToUnicodeConverter( aConverter );

        // convert the set of supported unicodes to ranges
        typedef std::vector<sal_UCS4> Ucs4Vector;
        Ucs4Vector aSupportedRanges;

        Ucs4Set::const_iterator itChar = aSupportedUnicodes.begin();
        for(; itChar != aSupportedUnicodes.end(); ++itChar )
        {
            if( aSupportedRanges.empty()
            || (aSupportedRanges.back() != *itChar) )
            {
                // add new range beginning with current unicode
                aSupportedRanges.push_back( *itChar );
                aSupportedRanges.push_back( 0 );
            }

            // extend existing range to include current unicode
            aSupportedRanges.back() = *itChar + 1;
        }

        // glyph mapping for non-unicode fonts not implemented
        delete[] pStartGlyphs;
        pStartGlyphs = NULL;
        aGlyphIdArray.clear();

        // make a pCodePairs array using the vector from above
        delete[] pCodePairs;
        nRangeCount = aSupportedRanges.size() / 2;
        if( nRangeCount <= 0 )
            return false;
        pCodePairs = new sal_UCS4[ nRangeCount * 2 ];
        Ucs4Vector::const_iterator itInt = aSupportedRanges.begin();
        for( pCP = pCodePairs; itInt != aSupportedRanges.end(); ++itInt )
            *(pCP++) = *itInt;
    }

    // prepare the glyphid-array if needed
    // TODO: merge ranges if they are close enough?
    sal_uInt16* pGlyphIds = NULL;
    if( !aGlyphIdArray.empty())
    {
        pGlyphIds = new sal_uInt16[ aGlyphIdArray.size() ];
        sal_uInt16* pOut = pGlyphIds;
        U16Vector::const_iterator it = aGlyphIdArray.begin();
        while( it != aGlyphIdArray.end() )
            *(pOut++) = *(it++);
    }

    // update the result struct
    rResult.mpRangeCodes = pCodePairs;
    rResult.mpStartGlyphs = pStartGlyphs;
    rResult.mnRangeCount = nRangeCount;
    rResult.mpGlyphIds = pGlyphIds;
    return true;
}

// =======================================================================

FontCharMap::FontCharMap()
:   mpImpl( ImplFontCharMap::GetDefaultMap() )
{}

// -----------------------------------------------------------------------

FontCharMap::~FontCharMap()
{
    mpImpl->DeReference();
    mpImpl = NULL;
}

// -----------------------------------------------------------------------

int FontCharMap::GetCharCount() const
{
    return mpImpl->GetCharCount();
}

// -----------------------------------------------------------------------

int FontCharMap::CountCharsInRange( sal_UCS4 cMin, sal_UCS4 cMax ) const
{
    return mpImpl->CountCharsInRange( cMin, cMax );
}

// -----------------------------------------------------------------------

void FontCharMap::Reset( const ImplFontCharMap* pNewMap )
{
    if( pNewMap == NULL )
    {
        mpImpl->DeReference();
        mpImpl = ImplFontCharMap::GetDefaultMap();
    }
    else if( pNewMap != mpImpl )
    {
        mpImpl->DeReference();
        mpImpl = pNewMap;
        mpImpl->AddReference();
    }
}

// -----------------------------------------------------------------------

bool FontCharMap::IsDefaultMap() const
{
    return mpImpl->IsDefaultMap();
}

// -----------------------------------------------------------------------

bool FontCharMap::HasChar( sal_UCS4 cChar ) const
{
    return mpImpl->HasChar( cChar );
}

// -----------------------------------------------------------------------

sal_UCS4 FontCharMap::GetFirstChar() const
{
    return mpImpl->GetFirstChar();
}

// -----------------------------------------------------------------------

sal_UCS4 FontCharMap::GetNextChar( sal_UCS4 cChar ) const
{
    return mpImpl->GetNextChar( cChar );
}

// -----------------------------------------------------------------------

sal_UCS4 FontCharMap::GetPrevChar( sal_UCS4 cChar ) const
{
    return mpImpl->GetPrevChar( cChar );
}

// -----------------------------------------------------------------------

int FontCharMap::GetIndexFromChar( sal_UCS4 cChar ) const
{
    return mpImpl->GetIndexFromChar( cChar );
}

// -----------------------------------------------------------------------

sal_UCS4 FontCharMap::GetCharFromIndex( int nIndex ) const
{
    return mpImpl->GetCharFromIndex( nIndex );
}

// =======================================================================

// on some systems we have to get the font attributes from the name table
// since neither head's macStyle nor OS/2's panose are easily available
// during font enumeration. macStyle bits would be not sufficient anyway
// and SFNT fonts on Mac usually do not contain an OS/2 table.
void UpdateAttributesFromPSName( const OUString& rPSName, ImplDevFontAttributes& rDFA )
{
    OString aPSName( OUStringToOString( rPSName, RTL_TEXTENCODING_UTF8 ).toAsciiLowerCase() );

    // TODO: use a multi-string ignore-case matcher once it becomes available
    if( (aPSName.indexOf("regular") != -1)
    ||  (aPSName.indexOf("normal") != -1)
    ||  (aPSName.indexOf("roman") != -1)
    ||  (aPSName.indexOf("medium") != -1)
    ||  (aPSName.indexOf("plain") != -1)
    ||  (aPSName.indexOf("standard") != -1)
    ||  (aPSName.indexOf("std") != -1) )
    {
        rDFA.SetWidthType(WIDTH_NORMAL);
        rDFA.SetWeight(WEIGHT_NORMAL);
        rDFA.SetItalic(ITALIC_NONE);
    }

    // heuristics for font weight
    if (aPSName.indexOf("extrablack") != -1)
        rDFA.SetWeight(WEIGHT_BLACK);
    else if (aPSName.indexOf("black") != -1)
        rDFA.SetWeight(WEIGHT_BLACK);
    else if( (aPSName.indexOf("semibold") != -1)
    ||  (aPSName.indexOf("smbd") != -1))
        rDFA.SetWeight(WEIGHT_SEMIBOLD);
    else if (aPSName.indexOf("ultrabold") != -1)
        rDFA.SetWeight(WEIGHT_ULTRABOLD);
    else if (aPSName.indexOf("extrabold") != -1)
        rDFA.SetWeight(WEIGHT_BLACK);
    else if( (aPSName.indexOf("bold") != -1)
    ||  (aPSName.indexOf("-bd") != -1))
        rDFA.SetWeight(WEIGHT_BOLD);
    else if (aPSName.indexOf("extralight") != -1)
        rDFA.SetWeight(WEIGHT_ULTRALIGHT);
    else if (aPSName.indexOf("ultralight") != -1)
        rDFA.SetWeight(WEIGHT_ULTRALIGHT);
    else if (aPSName.indexOf("light") != -1)
        rDFA.SetWeight(WEIGHT_LIGHT);
    else if (aPSName.indexOf("thin") != -1)
        rDFA.SetWeight(WEIGHT_THIN);
    else if (aPSName.indexOf("-w3") != -1)
        rDFA.SetWeight(WEIGHT_LIGHT);
    else if (aPSName.indexOf("-w4") != -1)
        rDFA.SetWeight(WEIGHT_SEMILIGHT);
    else if (aPSName.indexOf("-w5") != -1)
        rDFA.SetWeight(WEIGHT_NORMAL);
    else if (aPSName.indexOf("-w6") != -1)
        rDFA.SetWeight(WEIGHT_SEMIBOLD);
    else if (aPSName.indexOf("-w7") != -1)
        rDFA.SetWeight(WEIGHT_BOLD);
    else if (aPSName.indexOf("-w8") != -1)
        rDFA.SetWeight(WEIGHT_ULTRABOLD);
    else if (aPSName.indexOf("-w9") != -1)
        rDFA.SetWeight(WEIGHT_BLACK);

    // heuristics for font slant
    if( (aPSName.indexOf("italic") != -1)
    ||  (aPSName.indexOf(" ital") != -1)
    ||  (aPSName.indexOf("cursive") != -1)
    ||  (aPSName.indexOf("-it") != -1)
    ||  (aPSName.indexOf("lightit") != -1)
    ||  (aPSName.indexOf("mediumit") != -1)
    ||  (aPSName.indexOf("boldit") != -1)
    ||  (aPSName.indexOf("cnit") != -1)
    ||  (aPSName.indexOf("bdcn") != -1)
    ||  (aPSName.indexOf("bdit") != -1)
    ||  (aPSName.indexOf("condit") != -1)
    ||  (aPSName.indexOf("bookit") != -1)
    ||  (aPSName.indexOf("blackit") != -1) )
        rDFA.SetItalic(ITALIC_NORMAL);
    if( (aPSName.indexOf("oblique") != -1)
    ||  (aPSName.indexOf("inclined") != -1)
    ||  (aPSName.indexOf("slanted") != -1) )
        rDFA.SetItalic(ITALIC_OBLIQUE);

    // heuristics for font width
    if( (aPSName.indexOf("condensed") != -1)
    ||  (aPSName.indexOf("-cond") != -1)
    ||  (aPSName.indexOf("boldcond") != -1)
    ||  (aPSName.indexOf("boldcn") != -1)
    ||  (aPSName.indexOf("cnit") != -1) )
        rDFA.SetWidthType(WIDTH_CONDENSED);
    else if (aPSName.indexOf("narrow") != -1)
        rDFA.SetWidthType(WIDTH_SEMI_CONDENSED);
    else if (aPSName.indexOf("expanded") != -1)
        rDFA.SetWidthType(WIDTH_EXPANDED);
    else if (aPSName.indexOf("wide") != -1)
        rDFA.SetWidthType(WIDTH_EXPANDED);

    // heuristics for font pitch
    if( (aPSName.indexOf("mono") != -1)
    ||  (aPSName.indexOf("courier") != -1)
    ||  (aPSName.indexOf("monaco") != -1)
    ||  (aPSName.indexOf("typewriter") != -1) )
        rDFA.SetPitch(PITCH_FIXED);

    // heuristics for font family type
    if( (aPSName.indexOf("script") != -1)
    ||  (aPSName.indexOf("chancery") != -1)
    ||  (aPSName.indexOf("zapfino") != -1))
        rDFA.SetFamilyType(FAMILY_SCRIPT);
    else if( (aPSName.indexOf("comic") != -1)
    ||  (aPSName.indexOf("outline") != -1)
    ||  (aPSName.indexOf("pinpoint") != -1) )
        rDFA.SetFamilyType(FAMILY_DECORATIVE);
    else if( (aPSName.indexOf("sans") != -1)
    ||  (aPSName.indexOf("arial") != -1) )
        rDFA.SetFamilyType(FAMILY_SWISS);
    else if( (aPSName.indexOf("roman") != -1)
    ||  (aPSName.indexOf("times") != -1) )
        rDFA.SetFamilyType(FAMILY_ROMAN);

    // heuristics for codepoint semantic
    if( (aPSName.indexOf("symbol") != -1)
    ||  (aPSName.indexOf("dings") != -1)
    ||  (aPSName.indexOf("dingbats") != -1)
    ||  (aPSName.indexOf("ornaments") != -1)
    ||  (aPSName.indexOf("embellishments") != -1) )
        rDFA.SetSymbolFlag(true);

   // #i100020# special heuristic for names with single-char styles
   // NOTE: we are checking name that hasn't been lower-cased
   if( rPSName.getLength() > 3 )
   {
        int i = rPSName.getLength();
        sal_Unicode c = rPSName[--i];
        if( c == 'C' ) { // "capitals"
            rDFA.SetFamilyType(FAMILY_DECORATIVE);
            c = rPSName[--i];
        }
        if( c == 'O' ) { // CFF-based OpenType
            c = rPSName[--i];
        }
        if( c == 'I' ) { // "italic"
            rDFA.SetItalic(ITALIC_NORMAL);
            c = rPSName[--i];
        }
        if( c == 'B' )   // "bold"
            rDFA.SetWeight(WEIGHT_BOLD);
        if( c == 'C' )   // "capitals"
            rDFA.SetFamilyType(FAMILY_DECORATIVE);
        // TODO: check that all single-char styles have been resolved?
    }
}

// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
