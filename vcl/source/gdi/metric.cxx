/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_vcl.hxx"

#include <vcl/impfont.hxx>
#include <vcl/metric.hxx>

#include <vector>
#include <set>

#include <cstdio>

// =======================================================================

ImplFontMetric::ImplFontMetric()
:   mnAscent( 0 ),
    mnDescent( 0 ),
    mnIntLeading( 0 ),
    mnExtLeading( 0 ),
    mnLineHeight( 0 ),
    mnSlant( 0 ),
    mnMiscFlags( 0 ),
    mnRefCount( 1 )
{}

// -----------------------------------------------------------------------

inline void ImplFontMetric::AddReference()
{
    // TODO: disable refcounting on the default maps?
    ++mnRefCount;
}

// -----------------------------------------------------------------------

inline void ImplFontMetric::DeReference()
{
    // TODO: disable refcounting on the default maps?
    if( --mnRefCount <= 0 )
        delete this;
}

// -----------------------------------------------------------------------

bool ImplFontMetric::operator==( const ImplFontMetric& r ) const
{
    if( mnMiscFlags  != r.mnMiscFlags )
        return false;
    if( mnAscent     != r.mnAscent )
        return false;
    if( mnDescent    != r.mnDescent )
        return false;
    if( mnIntLeading != r.mnIntLeading )
        return false;
    if( mnExtLeading != r.mnExtLeading )
        return false;
    if( mnSlant      != r.mnSlant )
        return false;

    return true;
}

// =======================================================================

FontInfo::FontInfo()
:   mpImplMetric( new ImplFontMetric )
{}

// -----------------------------------------------------------------------

FontInfo::FontInfo( const FontInfo& rInfo )
:  Font( rInfo )
{
    mpImplMetric = rInfo.mpImplMetric;
    mpImplMetric->AddReference();
}

// -----------------------------------------------------------------------

FontInfo::~FontInfo()
{
    mpImplMetric->DeReference();
}

// -----------------------------------------------------------------------

FontInfo& FontInfo::operator=( const FontInfo& rInfo )
{
    Font::operator=( rInfo );

    if( mpImplMetric != rInfo.mpImplMetric )
    {
        mpImplMetric->DeReference();
        mpImplMetric = rInfo.mpImplMetric;
        mpImplMetric->AddReference();
    }

    return *this;
}

// -----------------------------------------------------------------------

sal_Bool FontInfo::operator==( const FontInfo& rInfo ) const
{
    if( !Font::operator==( rInfo ) )
        return sal_False;
    if( mpImplMetric == rInfo.mpImplMetric )
        return sal_True;
    if( *mpImplMetric == *rInfo.mpImplMetric  )
        return sal_True;
    return sal_False;
}

// -----------------------------------------------------------------------

FontType FontInfo::GetType() const
{
    return (mpImplMetric->IsScalable() ? TYPE_SCALABLE : TYPE_RASTER);
}

// -----------------------------------------------------------------------

sal_Bool FontInfo::IsDeviceFont() const
{
    return mpImplMetric->IsDeviceFont();
}

// -----------------------------------------------------------------------

sal_Bool FontInfo::SupportsLatin() const
{
    return mpImplMetric->SupportsLatin();
}

// -----------------------------------------------------------------------

sal_Bool FontInfo::SupportsCJK() const
{
    return mpImplMetric->SupportsCJK();
}

// -----------------------------------------------------------------------

sal_Bool FontInfo::SupportsCTL() const
{
    return mpImplMetric->SupportsCTL();
}

// =======================================================================

FontMetric::FontMetric( const FontMetric& rMetric )
:    FontInfo( rMetric )
{}

// -----------------------------------------------------------------------

long FontMetric::GetAscent() const
{
    return mpImplMetric->GetAscent();
}

// -----------------------------------------------------------------------

long FontMetric::GetDescent() const
{
    return mpImplMetric->GetDescent();
}

// -----------------------------------------------------------------------

long FontMetric::GetIntLeading() const
{
    return mpImplMetric->GetIntLeading();
}

// -----------------------------------------------------------------------

long FontMetric::GetExtLeading() const
{
    return mpImplMetric->GetExtLeading();
}

// -----------------------------------------------------------------------

long FontMetric::GetLineHeight() const
{
    return mpImplMetric->GetLineHeight();
}

// -----------------------------------------------------------------------

long FontMetric::GetSlant() const
{
    return mpImplMetric->GetSlant();
}

// -----------------------------------------------------------------------

FontMetric& FontMetric::operator =( const FontMetric& rMetric )
{
    FontInfo::operator=( rMetric );
    return *this;
}

// -----------------------------------------------------------------------

sal_Bool FontMetric::operator==( const FontMetric& rMetric ) const
{
    return FontInfo::operator==( rMetric );
}

// =======================================================================

CmapResult::CmapResult( bool bSymbolic,
    const sal_uInt32* pRangeCodes, int nRangeCount,
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
,   mnRefCount( 0 )
{
    const sal_uInt32* pRangePtr = mpRangeCodes;
    for( int i = mnRangeCount; --i >= 0; pRangePtr += 2 )
    {
        sal_uInt32 cFirst = pRangePtr[0];
        sal_uInt32 cLast  = pRangePtr[1];
        mnCharCount += cLast - cFirst;
    }
}

static ImplFontCharMap* pDefaultUnicodeImplFontCharMap = NULL;
static ImplFontCharMap* pDefaultSymbolImplFontCharMap = NULL;
static const sal_uInt32 aDefaultUnicodeRanges[] = {0x0020,0xD800, 0xE000,0xFFF0};
static const sal_uInt32 aDefaultSymbolRanges[] = {0x0020,0x0100, 0xF020,0xF100};

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

namespace
{
    ImplFontCharMap *GetDefaultUnicodeMap()
    {
        if( !pDefaultUnicodeImplFontCharMap )
        {
            const sal_uInt32* pRangeCodes = aDefaultUnicodeRanges;
            int nCodesCount = sizeof(aDefaultUnicodeRanges) / sizeof(*pRangeCodes);
            CmapResult aDefaultCR( false, pRangeCodes, nCodesCount/2 );
            pDefaultUnicodeImplFontCharMap = new ImplFontCharMap( aDefaultCR );
            pDefaultUnicodeImplFontCharMap->AddReference();
        }

        return pDefaultUnicodeImplFontCharMap;
    }

    ImplFontCharMap *GetDefaultSymbolMap()
    {
        if( !pDefaultSymbolImplFontCharMap )
        {
            const sal_uInt32* pRangeCodes = aDefaultSymbolRanges;
            int nCodesCount = sizeof(aDefaultSymbolRanges) / sizeof(*pRangeCodes);
            CmapResult aDefaultCR( true, pRangeCodes, nCodesCount/2 );
            pDefaultSymbolImplFontCharMap = new ImplFontCharMap( aDefaultCR );
            pDefaultSymbolImplFontCharMap->AddReference();
        }

        return pDefaultSymbolImplFontCharMap;
    }
}

ImplFontCharMap* ImplFontCharMap::GetDefaultMap( bool bSymbols)
{
    return bSymbols ? GetDefaultSymbolMap() : GetDefaultUnicodeMap();
}

// -----------------------------------------------------------------------

void ImplFontCharMap::AddReference( void ) const
{
    // TODO: disable refcounting on the default maps?
    ++mnRefCount;
}

// -----------------------------------------------------------------------

void ImplFontCharMap::DeReference( void ) const
{
    if( --mnRefCount <= 0 )
        if( (this != pDefaultUnicodeImplFontCharMap) && (this != pDefaultSymbolImplFontCharMap) )
            delete this;
}

// -----------------------------------------------------------------------

int ImplFontCharMap::GetCharCount() const
{
    return mnCharCount;
}

// -----------------------------------------------------------------------

int ImplFontCharMap::ImplFindRangeIndex( sal_uInt32 cChar ) const
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

bool ImplFontCharMap::HasChar( sal_uInt32 cChar ) const
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

int ImplFontCharMap::GetGlyphIndex( sal_uInt32 cChar ) const
{
    // return -1 if the object doesn't know the glyph ids
    if( !mpStartGlyphs )
        return -1;

    // return 0 if the unicode doesn't have a matching glyph
    int nRange = ImplFindRangeIndex( cChar );
    // check that we are inside any range
    if( (nRange == 0) && (cChar < mpRangeCodes[0]) ) {
        // symbol aliasing gives symbol fonts a second chance
        const bool bSymbolic = (mpRangeCodes[0]>=0xF000) & (mpRangeCodes[1]<=0xF0FF);
        if( !bSymbolic )
            return 0;
        // check for symbol aliasing (U+00xx <-> U+F0xx)
        cChar |= 0xF000;
        nRange = ImplFindRangeIndex( cChar );
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
        nGlyphIndex = mpGlyphIds[ nGlyphIndex - nStartIndex ];
    }

    return nGlyphIndex;
}

// -----------------------------------------------------------------------

// returns the number of chars supported by the font, which
// are inside the unicode range from cMin to cMax (inclusive)
int ImplFontCharMap::CountCharsInRange( sal_uInt32 cMin, sal_uInt32 cMax ) const
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

sal_uInt32 ImplFontCharMap::GetFirstChar() const
{
    return mpRangeCodes[0];
}

// -----------------------------------------------------------------------

sal_uInt32 ImplFontCharMap::GetLastChar() const
{
    return (mpRangeCodes[ 2*mnRangeCount-1 ] - 1);
}

// -----------------------------------------------------------------------

sal_uInt32 ImplFontCharMap::GetNextChar( sal_uInt32 cChar ) const
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

sal_uInt32 ImplFontCharMap::GetPrevChar( sal_uInt32 cChar ) const
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

int ImplFontCharMap::GetIndexFromChar( sal_uInt32 cChar ) const
{
    // TODO: improve linear walk?
    int nCharIndex = 0;
    const sal_uInt32* pRange = &mpRangeCodes[0];
    for( int i = 0; i < mnRangeCount; ++i )
    {
        sal_uInt32 cFirst = *(pRange++);
        sal_uInt32 cLast  = *(pRange++);
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

sal_uInt32 ImplFontCharMap::GetCharFromIndex( int nCharIndex ) const
{
    // TODO: improve linear walk?
    const sal_uInt32* pRange = &mpRangeCodes[0];
    for( int i = 0; i < mnRangeCount; ++i )
    {
        sal_uInt32 cFirst = *(pRange++);
        sal_uInt32 cLast  = *(pRange++);
        nCharIndex -= cLast - cFirst;
        if( nCharIndex < 0 )
            return (cLast + nCharIndex);
    }

    // we can only get here with an out-of-bounds charindex
    return mpRangeCodes[0];
}

// =======================================================================

static unsigned GetUInt( const unsigned char* p ) { return((p[0]<<24)+(p[1]<<16)+(p[2]<<8)+p[3]);}
static unsigned GetUShort( const unsigned char* p ){ return((p[0]<<8) | p[1]);}
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

    if( GetUShort( pCmap ) != 0x0000 ) // simple check for CMAP corruption
        return false;

    int nSubTables = GetUShort( pCmap + 2 );
    if( (nSubTables <= 0) || (nLength < (24 + 8*nSubTables)) )
        return false;

    // find the most interesting subtable in the CMAP
    rtl_TextEncoding eRecodeFrom = RTL_TEXTENCODING_UNICODE;
    int nOffset = 0;
    int nFormat = -1;
    int nBestVal = 0;
    for( const unsigned char* p = pCmap + 4; --nSubTables >= 0; p += 8 )
    {
        int nPlatform = GetUShort( p );
        int nEncoding = GetUShort( p+2 );
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
        int nTmpFormat = GetUShort( pCmap + nTmpOffset );
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
    sal_uInt32* pCodePairs = NULL;
    int* pStartGlyphs = NULL;

    typedef std::vector<sal_uInt16> U16Vector;
    U16Vector aGlyphIdArray;
    aGlyphIdArray.reserve( 0x1000 );
    aGlyphIdArray.push_back( 0 );

    // format 4, the most common 16bit char mapping table
    if( (nFormat == 4) && ((nOffset+16) < nLength) )
    {
        int nSegCountX2 = GetUShort( pCmap + nOffset + 6 );
        nRangeCount = nSegCountX2/2 - 1;
        pCodePairs = new sal_uInt32[ nRangeCount * 2 ];
        pStartGlyphs = new int[ nRangeCount ];
        const unsigned char* pLimitBase = pCmap + nOffset + 14;
        const unsigned char* pBeginBase = pLimitBase + nSegCountX2 + 2;
        const unsigned char* pDeltaBase = pBeginBase + nSegCountX2;
        const unsigned char* pOffsetBase = pDeltaBase + nSegCountX2;
        sal_uInt32* pCP = pCodePairs;
        for( int i = 0; i < nRangeCount; ++i )
        {
            const sal_uInt32 cMinChar = GetUShort( pBeginBase + 2*i );
            const sal_uInt32 cMaxChar = GetUShort( pLimitBase + 2*i );
            const int nGlyphDelta  = GetSShort( pDeltaBase + 2*i );
            const int nRangeOffset = GetUShort( pOffsetBase + 2*i );
            if( cMinChar > cMaxChar )   // no sane font should trigger this
                break;
            if( cMaxChar == 0xFFFF )
                break;
            *(pCP++) = cMinChar;
            *(pCP++) = cMaxChar + 1;
            if( !nRangeOffset ) {
                // glyphid can be calculated directly
                pStartGlyphs[i] = (cMinChar + nGlyphDelta) & 0xFFFF;
            } else {
                // update the glyphid-array with the glyphs in this range
                pStartGlyphs[i] = -(int)aGlyphIdArray.size();
                const unsigned char* pGlyphIdPtr = pOffsetBase + 2*i + nRangeOffset;
                for( sal_uInt32 c = cMinChar; c <= cMaxChar; ++c, pGlyphIdPtr+=2 ) {
                    const int nGlyphIndex = GetUShort( pGlyphIdPtr ) + nGlyphDelta;
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
        pCodePairs = new sal_uInt32[ nRangeCount * 2 ];
        pStartGlyphs = new int[ nRangeCount ];
        const unsigned char* pGroup = pCmap + nOffset + 16;
        sal_uInt32* pCP = pCodePairs;
        for( int i = 0; i < nRangeCount; ++i )
        {
            sal_uInt32 cMinChar = GetUInt( pGroup + 0 );
            sal_uInt32 cMaxChar = GetUInt( pGroup + 4 );
            int nGlyphId = GetUInt( pGroup + 8 );
            pGroup += 12;
            if( cMinChar > cMaxChar )   // no sane font should trigger this
                break;
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
            pCodePairs = new sal_uInt32[4];
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
        typedef std::set<sal_uInt32> IntSet;
        IntSet aSupportedUnicodes;

        static const int NINSIZE = 64;
        static const int NOUTSIZE = 64;
        sal_Char    cCharsInp[ NINSIZE ];
        sal_Unicode cCharsOut[ NOUTSIZE ];
        sal_uInt32* pCP = pCodePairs;
        for( int i = 0; i < nRangeCount; ++i )
        {
            sal_uInt32 cMin = *(pCP++);
            sal_uInt32 cEnd = *(pCP++);
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
        typedef std::vector<sal_uInt32> IntVector;
        IntVector aSupportedRanges;

        IntSet::const_iterator itChar = aSupportedUnicodes.begin();
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
        pCodePairs = new sal_uInt32[ nRangeCount * 2 ];
        IntVector::const_iterator itInt = aSupportedRanges.begin();
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
{
    mpImpl->AddReference();
}

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

int FontCharMap::CountCharsInRange( sal_uInt32 cMin, sal_uInt32 cMax ) const
{
    return mpImpl->CountCharsInRange( cMin, cMax );
}

// -----------------------------------------------------------------------

void FontCharMap::Reset( const ImplFontCharMap* pNewMap )
{
    mpImpl->DeReference();
    if( pNewMap == NULL )
        mpImpl = ImplFontCharMap::GetDefaultMap();
    else if( pNewMap != mpImpl )
        mpImpl = pNewMap;
    mpImpl->AddReference();
}

// -----------------------------------------------------------------------

sal_Bool FontCharMap::IsDefaultMap() const
{
    return mpImpl->IsDefaultMap();
}

// -----------------------------------------------------------------------

sal_Bool FontCharMap::HasChar( sal_uInt32 cChar ) const
{
    return mpImpl->HasChar( cChar );
}

// -----------------------------------------------------------------------

sal_uInt32 FontCharMap::GetFirstChar() const
{
    return mpImpl->GetFirstChar();
}

// -----------------------------------------------------------------------

sal_uInt32 FontCharMap::GetLastChar() const
{
    return mpImpl->GetLastChar();
}

// -----------------------------------------------------------------------

sal_uInt32 FontCharMap::GetNextChar( sal_uInt32 cChar ) const
{
    return mpImpl->GetNextChar( cChar );
}

// -----------------------------------------------------------------------

sal_uInt32 FontCharMap::GetPrevChar( sal_uInt32 cChar ) const
{
    return mpImpl->GetPrevChar( cChar );
}

// -----------------------------------------------------------------------

int FontCharMap::GetIndexFromChar( sal_uInt32 cChar ) const
{
    return mpImpl->GetIndexFromChar( cChar );
}

// -----------------------------------------------------------------------

sal_uInt32 FontCharMap::GetCharFromIndex( int nIndex ) const
{
    return mpImpl->GetCharFromIndex( nIndex );
}

// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
