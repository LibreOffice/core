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

#include <tools/svwin.h>

#include <rtl/ustring.hxx>
#include <osl/module.h>
#include <salgdi.h>
#include <saldata.hxx>
#include <vcl/sallayout.hxx>

#ifndef __H_FT2LIB
#include <wingdi.h>
#include <ft2lib.h>
#endif

#include <cstdio>
#include <malloc.h>

#ifdef GCP_KERN_HACK
    #include <algorithm>
#endif // GCP_KERN_HACK

// for GetMirroredChar
#include <vcl/svapp.hxx>

#include <boost/unordered_map.hpp>
typedef boost::unordered_map<int,int> IntMap;

#define DROPPED_OUTGLYPH 0xFFFF

using namespace rtl;

// =======================================================================

// OS/2 specific physical font instance
class ImplOs2FontEntry : public ImplFontEntry
{
public:
    ImplOs2FontEntry( ImplFontSelectData& );
    ~ImplOs2FontEntry();

private:
    // TODO: also add HFONT??? Watch out for issues with too many active fonts...

#ifdef GCP_KERN_HACK
public:
    bool                    HasKernData() const;
    void                    SetKernData( int, const KERNINGPAIRS* );
    int                     GetKerning( sal_Unicode, sal_Unicode ) const;
private:
    KERNINGPAIRS*           mpKerningPairs;
    int                     mnKerningPairs;
#endif // GCP_KERN_HACK

public:
    int                     GetCachedGlyphWidth( int nCharCode ) const;
    void                    CacheGlyphWidth( int nCharCode, int nCharWidth );
private:
    IntMap                  maWidthMap;
};

// -----------------------------------------------------------------------

inline void ImplOs2FontEntry::CacheGlyphWidth( int nCharCode, int nCharWidth )
{
    maWidthMap[ nCharCode ] = nCharWidth;
}

inline int ImplOs2FontEntry::GetCachedGlyphWidth( int nCharCode ) const
{
    IntMap::const_iterator it = maWidthMap.find( nCharCode );
    if( it == maWidthMap.end() )
        return -1;
    return it->second;
}

// =======================================================================

class Os2Layout : public SalLayout
{
public:
                        Os2Layout( HDC, const ImplOs2FontData&, ImplOs2FontEntry& );
    virtual void        InitFont() const;
    void                SetFontScale( float f ) { mfFontScale = f; }
    float               GetFontScale() const    { return mfFontScale; }

protected:
    HPS                 mhPS;               // OS2 device handle
    FATTRS              mhFont;
    int                 mnBaseAdv;          // x-offset relative to Layout origin
    float               mfFontScale;        // allows metrics emulation of huge font sizes

    const ImplOs2FontData&    mrOs2FontData;
    ImplOs2FontEntry&   mrOs2FontEntry;
};

// =======================================================================

class Os2SalLayout : public Os2Layout
{
public:
                    Os2SalLayout( HPS, BYTE nCharSet, const ImplOs2FontData&, ImplOs2FontEntry& );
    virtual         ~Os2SalLayout();

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;

    virtual int     GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int&,
                        sal_Int32* pGlyphAdvances, int* pCharIndexes ) const;

    virtual long    FillDXArray( long* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const;

    // for glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );

protected:
    void            Justify( long nNewWidth );
    void            ApplyDXArray( const ImplLayoutArgs& );

protected:

private:
    int             mnGlyphCount;
    int             mnCharCount;
    sal_Unicode*    mpOutGlyphs;
    int*            mpGlyphAdvances;    // if possible this is shared with mpGlyphAdvances[]
    int*            mpGlyphOrigAdvs;
    int*            mpCharWidths;       // map rel char pos to char width
    int*            mpChars2Glyphs;     // map rel char pos to abs glyph pos
    int*            mpGlyphs2Chars;     // map abs glyph pos to abs char pos
    bool*           mpGlyphRTLFlags;    // BiDi status for glyphs: true=>RTL
    mutable long    mnWidth;
    bool            mbDisableGlyphs;

    int             mnNotdefWidth;
    BYTE            mnCharSet;

};

// =======================================================================

Os2Layout::Os2Layout( HPS hPS, const ImplOs2FontData& rWFD, ImplOs2FontEntry& rWFE )
:   mhPS( hPS ),
    mnBaseAdv( 0 ),
    mfFontScale( 1.0 ),
    mrOs2FontData( rWFD ),
    mrOs2FontEntry( rWFE )
{
    BOOL fSuccess;
    fSuccess = Ft2QueryLogicalFont( mhPS, LCID_BASE, NULL, &mhFont, sizeof(FATTRS));
}

// -----------------------------------------------------------------------

void Os2Layout::InitFont() const
{
    // select fallback level 0 font
    APIRET rc = Ft2CreateLogFont( mhPS, NULL, LCID_BASE, (PFATTRS)&mhFont);
}

// =======================================================================

Os2SalLayout::Os2SalLayout( HPS hPS, BYTE nCharSet,
    const ImplOs2FontData& rOs2FontData, ImplOs2FontEntry& rOs2FontEntry )
:   Os2Layout( hPS, rOs2FontData, rOs2FontEntry ),
    mnGlyphCount( 0 ),
    mnCharCount( 0 ),
    mpOutGlyphs( NULL ),
    mpGlyphAdvances( NULL ),
    mpGlyphOrigAdvs( NULL ),
    mpCharWidths( NULL ),
    mpChars2Glyphs( NULL ),
    mpGlyphs2Chars( NULL ),
    mpGlyphRTLFlags( NULL ),
    mnWidth( 0 ),
    mnNotdefWidth( -1 ),
    mnCharSet( nCharSet ),
    mbDisableGlyphs( false )
{
    mbDisableGlyphs = true;
}

// -----------------------------------------------------------------------

Os2SalLayout::~Os2SalLayout()
{
    delete[] mpGlyphRTLFlags;
    delete[] mpGlyphs2Chars;
    delete[] mpChars2Glyphs;
    if( mpCharWidths != mpGlyphAdvances )
        delete[] mpCharWidths;
    delete[] mpGlyphOrigAdvs;
    delete[] mpGlyphAdvances;
    delete[] mpOutGlyphs;
}

// -----------------------------------------------------------------------

bool Os2SalLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    // prepare layout
    // TODO: fix case when recyclying old Os2SalLayout object
    mbDisableGlyphs |= ((rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING) != 0);
    mnCharCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;

    if( !mbDisableGlyphs )
    {
        // Win32 glyph APIs have serious problems with vertical layout
        // => workaround is to use the unicode methods then
        if( rArgs.mnFlags & SAL_LAYOUT_VERTICAL )
            mbDisableGlyphs = true;
        else
            // use cached value from font face
            mbDisableGlyphs = mrOs2FontData.IsGlyphApiDisabled();
    }

    // TODO: use a cached value for bDisableAsianKern from upper layers

    // layout text
    int i, j;

    mnGlyphCount = 0;
    bool bVertical = (rArgs.mnFlags & SAL_LAYOUT_VERTICAL) != 0;

    // count the number of chars to process if no RTL run
    rArgs.ResetPos();
    bool bHasRTL = false;
    while( rArgs.GetNextRun( &i, &j, &bHasRTL ) && !bHasRTL )
        mnGlyphCount += j - i;

    // if there are RTL runs we need room to remember individual BiDi flags
    if( bHasRTL )
    {
        mpGlyphRTLFlags = new bool[ mnCharCount ];
        for( i = 0; i < mnCharCount; ++i )
            mpGlyphRTLFlags[i] = false;
    }

    // rewrite the logical string if needed to prepare for the API calls
    const sal_Unicode* pBidiStr = rArgs.mpStr + rArgs.mnMinCharPos;
    if( (mnGlyphCount != mnCharCount) || bVertical )
    {
        // we need to rewrite the pBidiStr when any of
        // - BiDirectional layout
        // - vertical layout
        // - partial runs (e.g. with control chars or for glyph fallback)
        // are involved
        sal_Unicode* pRewrittenStr = (sal_Unicode*)alloca( mnCharCount * sizeof(sal_Unicode) );
        pBidiStr = pRewrittenStr;

        // note: glyph to char mapping is relative to first character
        mpChars2Glyphs = new int[ mnCharCount ];
        mpGlyphs2Chars = new int[ mnCharCount ];
        for( i = 0; i < mnCharCount; ++i )
            mpChars2Glyphs[i] = mpGlyphs2Chars[i] = -1;

        mnGlyphCount = 0;
        rArgs.ResetPos();
        bool bIsRTL = false;
        while( rArgs.GetNextRun( &i, &j, &bIsRTL ) )
        {
            do
            {
                // get the next leftmost character in this run
                int nCharPos = bIsRTL ? --j : i++;
                sal_Unicode cChar = rArgs.mpStr[ nCharPos ];

                // in the RTL case mirror the character and remember its RTL status
                if( bIsRTL )
                {
                    cChar = ::GetMirroredChar( cChar );
                    mpGlyphRTLFlags[ mnGlyphCount ] = true;
                }

                // for vertical writing use vertical alternatives
                if( bVertical )
                {
                    sal_Unicode cVert = ::GetVerticalChar( cChar );
                    if( cVert )
                        cChar = cVert;
                }

                // rewrite the original string
                // update the mappings between original and rewritten string
                pRewrittenStr[ mnGlyphCount ] = cChar;
                mpGlyphs2Chars[ mnGlyphCount ] = nCharPos;
                mpChars2Glyphs[ nCharPos - rArgs.mnMinCharPos ] = mnGlyphCount;
                ++mnGlyphCount;
            } while( i < j );
        }
    }

    mpOutGlyphs     = new sal_Unicode[ mnGlyphCount ];
    mpGlyphAdvances = new int[ mnGlyphCount ];

    if( rArgs.mnFlags & (SAL_LAYOUT_KERNING_PAIRS | SAL_LAYOUT_KERNING_ASIAN) )
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];

#ifndef GCP_KERN_HACK
    DWORD nGcpOption = 0;
    // enable kerning if requested
    if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
        nGcpOption |= GCP_USEKERNING;
#endif // GCP_KERN_HACK

    LONG    lLcid = Ft2QueryCharSet( mhPS);

    for( i = 0; i < mnGlyphCount; ++i )
        mpOutGlyphs[i] = pBidiStr[ i ];
    mnWidth = 0;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        const sal_Unicode* pCodes = &pBidiStr[i];
        // check for surrogate pairs
        if( (pCodes[0] & 0xFC00) == 0xDC00 )
            continue;
        bool bSurrogate = ((pCodes[0] & 0xFC00) == 0xD800);

        // get the width of the corresponding code point
        int nCharCode = pCodes[0];
        if( bSurrogate )
            nCharCode = 0x10000 + ((pCodes[0] & 0x03FF) << 10) + (pCodes[1] & 0x03FF);
        int nGlyphWidth = mrOs2FontEntry.GetCachedGlyphWidth( nCharCode );
        if( nGlyphWidth == -1 )
        {
            if (!Ft2QueryStringWidthW( mhPS, (LPWSTR)&pCodes[0], 1, (LONG*)&nGlyphWidth))
                nGlyphWidth = 0;
            mrOs2FontEntry.CacheGlyphWidth( nCharCode, nGlyphWidth );
        }
        mpGlyphAdvances[ i ] = nGlyphWidth;
        mnWidth += nGlyphWidth;

        // remaining codes of surrogate pair get a zero width
        if( bSurrogate )
            mpGlyphAdvances[ i+1 ] = 0;

        // check with the font face if glyph fallback is needed
        if( mrOs2FontData.HasChar( nCharCode ) )
            continue;
        // Type1 charmaps are not complete (or buggy), use FT2 to check again
        if (Ft2FontSupportsUnicodeChar( mhPS, lLcid, TRUE, nCharCode))
            continue;

#if OSL_DEBUG_LEVEL > 1
        debug_printf("Os2SalLayout::LayoutText font does not support unicode char\n");
#endif
        // request glyph fallback at this position in the string
        bool bRTL = mpGlyphRTLFlags ? mpGlyphRTLFlags[i] : false;
        int nCharPos = mpGlyphs2Chars ? mpGlyphs2Chars[i]: i + rArgs.mnMinCharPos;
        rArgs.NeedFallback( nCharPos, bRTL );
        if( bSurrogate )
            rArgs.NeedFallback( nCharPos+1, bRTL );

        if( rArgs.mnFlags & SAL_LAYOUT_FOR_FALLBACK )
        {
            // when we already are layouting for glyph fallback
            // then a new unresolved glyph is not interesting
            mnNotdefWidth = 0;
            mpOutGlyphs[i] = DROPPED_OUTGLYPH;
            if( mbDisableGlyphs && bSurrogate )
                mpOutGlyphs[i+1] = DROPPED_OUTGLYPH;
        }
        else
        {
            if( mnNotdefWidth < 0 )
            {
                // get the width of the NotDef glyph
                LONG aExtent;
                mnNotdefWidth = 0;
                if (Ft2QueryStringWidthW( mhPS, (LPWSTR)&rArgs.mpStr[ nCharPos ], 1, &aExtent))
                    mnNotdefWidth = aExtent;
            }
            // use a better NotDef glyph
            if( !mbDisableGlyphs )
                mpOutGlyphs[i] = 0;
        }

        // replace the current glyph with the NotDef glyph
        mnWidth += mnNotdefWidth - mpGlyphAdvances[i];
        mpGlyphAdvances[i] = mnNotdefWidth;
        if( mpGlyphOrigAdvs )
            mpGlyphOrigAdvs[i] = mnNotdefWidth;
    }

#ifdef GCP_KERN_HACK
    // apply kerning if the layout engine has not yet done it
    if( rArgs.mnFlags & (SAL_LAYOUT_KERNING_ASIAN|SAL_LAYOUT_KERNING_PAIRS) )
    {
#else // GCP_KERN_HACK
    // apply just asian kerning
    if( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN )
    {
        if( !(rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS) )
#endif // GCP_KERN_HACK
            for( i = 0; i < mnGlyphCount; ++i )
                mpGlyphOrigAdvs[i] = mpGlyphAdvances[i];

        // #99658# also apply asian kerning on the substring border
        int nLen = mnGlyphCount;
        if( rArgs.mnMinCharPos + nLen < rArgs.mnLength )
            ++nLen;
        for( i = 1; i < nLen; ++i )
        {
#ifdef GCP_KERN_HACK
            if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
            {
                int nKernAmount = mrOs2FontEntry.GetKerning( pBidiStr[i-1], pBidiStr[i] );
                mpGlyphAdvances[ i-1 ] += nKernAmount;
                mnWidth += nKernAmount;
            }
            else if( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN )
#endif // GCP_KERN_HACK

            if( (0x3000 == (0xFF00 & pBidiStr[i-1]))
            &&  (0x3000 == (0xFF00 & pBidiStr[i])) )
            {
                long nKernFirst = +CalcAsianKerning( pBidiStr[i-1], true, bVertical );
                long nKernNext  = -CalcAsianKerning( pBidiStr[i], false, bVertical );

                long nDelta = (nKernFirst < nKernNext) ? nKernFirst : nKernNext;
                if( nDelta<0 && nKernFirst!=0 && nKernNext!=0 )
                {
                    nDelta = (nDelta * mpGlyphAdvances[i-1] + 2) / 4;
                    mpGlyphAdvances[i-1] += nDelta;
                    mnWidth += nDelta;
                }
            }
        }
    }

    // calculate virtual char widths
    if( !mpGlyphs2Chars )
        mpCharWidths = mpGlyphAdvances;
    else
    {
        mpCharWidths = new int[ mnCharCount ];
        for( i = 0; i < mnCharCount; ++i )
            mpCharWidths[ i ] = 0;
        for( i = 0; i < mnGlyphCount; ++i )
        {
            int j = mpGlyphs2Chars[ i ] - rArgs.mnMinCharPos;
            if( j >= 0 )
                mpCharWidths[ j ] += mpGlyphAdvances[ i ];
        }
    }

    // scale layout metrics if needed
    if( mfFontScale != 1.0 )
    {
        mnWidth   *= mfFontScale;
        mnBaseAdv *= mfFontScale;
        for( i = 0; i < mnCharCount; ++i )
            mpCharWidths[ i ] *= mfFontScale;
        if( mpGlyphAdvances != mpCharWidths )
            for( i = 0; i < mnGlyphCount; ++i )
                mpGlyphAdvances[ i ] *= mfFontScale;
        if( mpGlyphOrigAdvs && (mpGlyphOrigAdvs != mpGlyphAdvances) )
            for( i = 0; i < mnGlyphCount; ++i )
                mpGlyphOrigAdvs[ i ] *= mfFontScale;
    }

    return true;
}

// -----------------------------------------------------------------------

int Os2SalLayout::GetNextGlyphs( int nLen, sal_GlyphId* pGlyphs, Point& rPos, int& nStart,
    sal_Int32* pGlyphAdvances, int* pCharIndexes ) const
{
    // return zero if no more glyph found
    if( nStart >= mnGlyphCount )
        return 0;

    // calculate glyph position relative to layout base
    // TODO: avoid for nStart!=0 case by reusing rPos
    long nXOffset = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nXOffset += mpGlyphAdvances[ i ];

    // calculate absolute position in pixel units
    Point aRelativePos( nXOffset, 0 );
    rPos = GetDrawPosition( aRelativePos );

    int nCount = 0;
    while( nCount < nLen )
    {
        // update return values {nGlyphIndex,nCharPos,nGlyphAdvance}
        long nGlyphIndex = mpOutGlyphs[ nStart ];
        if( mbDisableGlyphs )
        {
            if( mnLayoutFlags & SAL_LAYOUT_VERTICAL )
            {
                sal_Unicode cChar = (sal_Unicode)(nGlyphIndex & GF_IDXMASK);
#ifdef GNG_VERT_HACK
                if( mrOs2FontData.HasGSUBstitutions( mhPS )
                &&  mrOs2FontData.IsGSUBstituted( cChar ) )
                    nGlyphIndex |= GF_ROTL | GF_GSUB;
                else
#endif // GNG_VERT_HACK
                {
                    nGlyphIndex |= GetVerticalFlags( cChar );
                    if( !(nGlyphIndex & GF_ROTMASK) )
                        nGlyphIndex |= GF_VERT;
                }
            }
            nGlyphIndex |= GF_ISCHAR;
        }
        ++nCount;
        *(pGlyphs++) = nGlyphIndex;
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = mpGlyphAdvances[ nStart ];
        if( pCharIndexes )
        {
            int nCharPos;
            if( !mpGlyphs2Chars )
                nCharPos = nStart + mnMinCharPos;
            else
                nCharPos = mpGlyphs2Chars[nStart];
            *(pCharIndexes++) = nCharPos;
        }

        // stop at last glyph
        if( ++nStart >= mnGlyphCount )
            break;

        // stop when next x-position is unexpected
        if( !pGlyphAdvances && mpGlyphOrigAdvs )
            if( mpGlyphAdvances[nStart-1] != mpGlyphOrigAdvs[nStart-1] )
                break;
    }

    return nCount;
}

// -----------------------------------------------------------------------

void Os2SalLayout::DrawText( SalGraphics& rGraphics ) const
{
    if( mnGlyphCount <= 0 )
        return;

    Point   aPos = GetDrawPosition( Point( mnBaseAdv, 0 ) );
    POINTL  aPt;
    APIRET  rc;

    aPt.x = aPos.X();
    aPt.y = static_cast<Os2SalGraphics&>(rGraphics).mnHeight - aPos.Y();

    // ft2lib doesn't work with printer hps, so we fallback to codepage printing
    // until cp1200 support will work.
    if (static_cast<Os2SalGraphics&>(rGraphics).mbPrinter) {
        // convert to codepage
        ByteString str( mpOutGlyphs, gsl_getSystemTextEncoding() );
        // gliph size is not recalculated, so it could be wrong!
        rc = Ft2CharStringPosAtA( static_cast<Os2SalGraphics&>(rGraphics).mhPS,
                    &aPt, NULL, CHS_VECTOR, mnGlyphCount, (PSZ)str.GetBuffer(),
                    (LONG*)mpGlyphAdvances, 0);
    } else {
        // try unicode rendering to screen
        rc = Ft2CharStringPosAtW( static_cast<Os2SalGraphics&>(rGraphics).mhPS,
                    &aPt, NULL, CHS_VECTOR, mnGlyphCount, (LPWSTR)mpOutGlyphs,
                    (LONG*)mpGlyphAdvances, 0);
        if (rc == GPI_ERROR) {
            // if *W fails, convert to codepage and use *A (fallback to GPI into ft2)
            ByteString str( mpOutGlyphs, gsl_getSystemTextEncoding() );
#if OSL_DEBUG_LEVEL > 1
            debug_printf("Os2SalLayout::DrawText HPS %08x PosAtW failed '%s'!\n",static_cast<Os2SalGraphics&>(rGraphics).mhPS,str.GetBuffer());
#endif
            // gliph size is not recalculated, so it could be wrong!
            rc = Ft2CharStringPosAtA( static_cast<Os2SalGraphics&>(rGraphics).mhPS,
                        &aPt, NULL, CHS_VECTOR, mnGlyphCount, (PSZ)str.GetBuffer(),
                        (LONG*)mpGlyphAdvances, 0);
        }
    }
}

// -----------------------------------------------------------------------

long Os2SalLayout::FillDXArray( long* pDXArray ) const
{
    if( !mnWidth )
    {
        long mnWidth = mnBaseAdv;
        for( int i = 0; i < mnGlyphCount; ++i )
            mnWidth += mpGlyphAdvances[ i ];
    }

    if( pDXArray != NULL )
    {
        for( int i = 0; i < mnCharCount; ++i )
             pDXArray[ i ] = mpCharWidths[ i ];
    }

    return mnWidth;
}

// -----------------------------------------------------------------------

int Os2SalLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
// NOTE: the nFactor is used to prevent rounding errors for small nCharExtra values
{
    if( mnWidth )
        if( (mnWidth * nFactor + mnCharCount * nCharExtra) <= nMaxWidth )
            return STRING_LEN;

    long nExtraWidth = mnBaseAdv * nFactor;
    for( int n = 0; n < mnCharCount; ++n )
    {
        // skip unused characters
        if( mpChars2Glyphs && (mpChars2Glyphs[n] < 0) )
            continue;
        // add char widths until max
        nExtraWidth += mpCharWidths[ n ] * nFactor;
        if( nExtraWidth >= nMaxWidth )
            return (mnMinCharPos + n);
        nExtraWidth += nCharExtra;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

void Os2SalLayout::GetCaretPositions( int nMaxIdx, long* pCaretXArray ) const
{
    long nXPos = mnBaseAdv;

    if( !mpGlyphs2Chars )
    {
        for( int i = 0; i < nMaxIdx; i += 2 )
        {
            pCaretXArray[ i ] = nXPos;
            nXPos += mpGlyphAdvances[ i>>1 ];
            pCaretXArray[ i+1 ] = nXPos;
        }
    }
    else
    {
        int  i;
        for( i = 0; i < nMaxIdx; ++i )
            pCaretXArray[ i ] = -1;

        // assign glyph positions to character positions
        for( i = 0; i < mnGlyphCount; ++i )
        {
            int nCurrIdx = mpGlyphs2Chars[ i ] - mnMinCharPos;
            long nXRight = nXPos + mpCharWidths[ nCurrIdx ];
            nCurrIdx *= 2;
            if( !(mpGlyphRTLFlags && mpGlyphRTLFlags[i]) )
            {
                // normal positions for LTR case
                pCaretXArray[ nCurrIdx ]   = nXPos;
                pCaretXArray[ nCurrIdx+1 ] = nXRight;
            }
            else
            {
                // reverse positions for RTL case
                pCaretXArray[ nCurrIdx ]   = nXRight;
                pCaretXArray[ nCurrIdx+1 ] = nXPos;
            }
            nXPos += mpGlyphAdvances[ i ];
        }
    }
}

// -----------------------------------------------------------------------

void Os2SalLayout::Justify( long nNewWidth )
{
    long nOldWidth = mnWidth;
    mnWidth = nNewWidth;

    if( mnGlyphCount <= 0 )
        return;

    if( nNewWidth == nOldWidth )
        return;

    // the rightmost glyph cannot be stretched
    const int nRight = mnGlyphCount - 1;
    nOldWidth -= mpGlyphAdvances[ nRight ];
    nNewWidth -= mpGlyphAdvances[ nRight ];

    // count stretchable glyphs
    int nStretchable = 0, i;
    for( i = 0; i < nRight; ++i )
        if( mpGlyphAdvances[i] >= 0 )
            ++nStretchable;

    // stretch these glyphs
    int nDiffWidth = nNewWidth - nOldWidth;
    for( i = 0; (i < nRight) && (nStretchable > 0); ++i )
    {
        if( mpGlyphAdvances[i] <= 0 )
            continue;
        int nDeltaWidth = nDiffWidth / nStretchable;
        mpGlyphAdvances[i] += nDeltaWidth;
        --nStretchable;
        nDiffWidth -= nDeltaWidth;
    }
}

// -----------------------------------------------------------------------

void Os2SalLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
    else
        return;

    // recalculate virtual char widths if they were changed
    if( mpCharWidths != mpGlyphAdvances )
    {
        int i;
        if( !mpGlyphs2Chars )
        {
            // standard LTR case
            for( i = 0; i < mnGlyphCount; ++i )
                 mpCharWidths[ i ] = mpGlyphAdvances[ i ];
        }
        else
        {
            // BiDi or complex case
            for( i = 0; i < mnCharCount; ++i )
                mpCharWidths[ i ] = 0;
            for( i = 0; i < mnGlyphCount; ++i )
            {
                int j = mpGlyphs2Chars[ i ] - rArgs.mnMinCharPos;
                if( j >= 0 )
                    mpCharWidths[ j ] += mpGlyphAdvances[ i ];
            }
        }
    }
}

// -----------------------------------------------------------------------

void Os2SalLayout::ApplyDXArray( const ImplLayoutArgs& rArgs )
{
    // try to avoid disturbance of text flow for LSB rounding case;
    const long* pDXArray = rArgs.mpDXArray;

    int i = 0;
    long nOldWidth = mnBaseAdv;
    for(; i < mnCharCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        if( j >= 0 )
        {
            nOldWidth += mpGlyphAdvances[ j ];
            int nDiff = nOldWidth - pDXArray[ i ];

           // disabled because of #104768#
            // works great for static text, but problems when typing
            // if( nDiff>+1 || nDiff<-1 )
            // only bother with changing anything when something moved
            if( nDiff != 0 )
                break;
        }
    }
    if( i >= mnCharCount )
        return;

    if( !mpGlyphOrigAdvs )
    {
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphOrigAdvs[ i ] = mpGlyphAdvances[ i ];
    }

    mnWidth = mnBaseAdv;
    for( i = 0; i < mnCharCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        if( j >= 0 )
            mpGlyphAdvances[j] = pDXArray[i] - mnWidth;
        mnWidth = pDXArray[i];
    }
}

// -----------------------------------------------------------------------

void Os2SalLayout::MoveGlyph( int nStart, long nNewXPos )
{
   if( nStart > mnGlyphCount )
        return;

    // calculate the current x-position of the requested glyph
    // TODO: cache absolute positions
    int nXPos = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nXPos += mpGlyphAdvances[i];

    // calculate the difference to the current glyph position
    int nDelta = nNewXPos - nXPos;

    // adjust the width of the layout if it was already cached
    if( mnWidth )
        mnWidth += nDelta;

    // depending on whether the requested glyph is leftmost in the layout
    // adjust either the layout's or the requested glyph's relative position
    if( nStart > 0 )
        mpGlyphAdvances[ nStart-1 ] += nDelta;
    else
        mnBaseAdv += nDelta;
}

// -----------------------------------------------------------------------

void Os2SalLayout::DropGlyph( int nStart )
{
    mpOutGlyphs[ nStart ] = DROPPED_OUTGLYPH;
}

// -----------------------------------------------------------------------

void Os2SalLayout::Simplify( bool bIsBase )
{
    // return early if no glyph has been dropped
    int i = mnGlyphCount;
    while( (--i >= 0) && (mpOutGlyphs[ i ] != DROPPED_OUTGLYPH) );
    if( i < 0 )
        return;

    // convert the layout to a sparse layout if it is not already
    if( !mpGlyphs2Chars )
    {
        mpGlyphs2Chars = new int[ mnGlyphCount ];
        mpCharWidths = new int[ mnCharCount ];
        // assertion: mnGlyphCount == mnCharCount
        for( int k = 0; k < mnGlyphCount; ++k )
        {
            mpGlyphs2Chars[ k ] = mnMinCharPos + k;
            mpCharWidths[ k ] = mpGlyphAdvances[ k ];
        }
    }

    // remove dropped glyphs that are rightmost in the layout
    for( i = mnGlyphCount; --i >= 0; )
    {
        if( mpOutGlyphs[ i ] != DROPPED_OUTGLYPH )
            break;
        if( mnWidth )
            mnWidth -= mpGlyphAdvances[ i ];
        int nRelCharPos = mpGlyphs2Chars[ i ] - mnMinCharPos;
        if( nRelCharPos >= 0 )
            mpCharWidths[ nRelCharPos ] = 0;
    }
    mnGlyphCount = i + 1;

    // keep original glyph widths around
    if( !mpGlyphOrigAdvs )
    {
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];
        for( int k = 0; k < mnGlyphCount; ++k )
            mpGlyphOrigAdvs[ k ] = mpGlyphAdvances[ k ];
    }

    // remove dropped glyphs inside the layout
    int nNewGC = 0;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
        {
            // adjust relative position to last valid glyph
            int nDroppedWidth = mpGlyphAdvances[ i ];
            mpGlyphAdvances[ i ] = 0;
            if( nNewGC > 0 )
                mpGlyphAdvances[ nNewGC-1 ] += nDroppedWidth;
            else
                mnBaseAdv += nDroppedWidth;

            // zero the virtual char width for the char that has a fallback
            int nRelCharPos = mpGlyphs2Chars[ i ] - mnMinCharPos;
            if( nRelCharPos >= 0 )
                mpCharWidths[ nRelCharPos ] = 0;
        }
        else
        {
            if( nNewGC != i )
            {
                // rearrange the glyph array to get rid of the dropped glyph
                mpOutGlyphs[ nNewGC ]     = mpOutGlyphs[ i ];
                mpGlyphAdvances[ nNewGC ] = mpGlyphAdvances[ i ];
                mpGlyphOrigAdvs[ nNewGC ] = mpGlyphOrigAdvs[ i ];
                mpGlyphs2Chars[ nNewGC ]  = mpGlyphs2Chars[ i ];
            }
            ++nNewGC;
        }
    }

    mnGlyphCount = nNewGC;
    if( mnGlyphCount <= 0 )
        mnWidth = mnBaseAdv = 0;
}

// =======================================================================

SalLayout* Os2SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    Os2SalLayout* pLayout = NULL;
    DBG_ASSERT( mpOs2FontEntry[nFallbackLevel], "WinSalGraphics mpWinFontEntry==NULL");

    const ImplOs2FontData& rFontFace      = *mpOs2FontData[ nFallbackLevel ];
    ImplOs2FontEntry& rFontInstance = *mpOs2FontEntry[ nFallbackLevel ];

    {
#ifdef GCP_KERN_HACK
        if( (rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS) && !rFontInstance.HasKernData() )
        {
            // TODO: directly cache kerning info in the rFontInstance
            // TODO: get rid of kerning methods+data in WinSalGraphics object
            GetKernPairs( 0, NULL );
            rFontInstance.SetKernData( mnFontKernPairCount, mpFontKernPairs );
        }
#endif // GCP_KERN_HACK

        pLayout = new Os2SalLayout( mhPS, 0, rFontFace, rFontInstance );
    }

    if( mfFontScale != 1.0 )
        pLayout->SetFontScale( mfFontScale );

    return pLayout;
}

// =======================================================================

ImplOs2FontEntry::ImplOs2FontEntry( ImplFontSelectData& rFSD )
:   ImplFontEntry( rFSD ),
    maWidthMap( 512 )
#ifdef GCP_KERN_HACK
    ,mpKerningPairs( NULL )
    ,mnKerningPairs( -1 )
#endif // GCP_KERN_HACK
{
}

// -----------------------------------------------------------------------

ImplOs2FontEntry::~ImplOs2FontEntry()
{
#ifdef GCP_KERN_HACK
    delete[] mpKerningPairs;
#endif // GCP_KERN_HACK
}

// -----------------------------------------------------------------------

#ifdef GCP_KERN_HACK
bool ImplOs2FontEntry::HasKernData() const
{
    return (mnKerningPairs >= 0);
}

// -----------------------------------------------------------------------

void ImplOs2FontEntry::SetKernData( int nPairCount, const KERNINGPAIRS* pPairData )
{
    mnKerningPairs = nPairCount;
    mpKerningPairs = new KERNINGPAIRS[ mnKerningPairs ];
    ::memcpy( mpKerningPairs, (const void*)pPairData, nPairCount*sizeof(KERNINGPAIRS) );
}

// -----------------------------------------------------------------------

int ImplOs2FontEntry::GetKerning( sal_Unicode cLeft, sal_Unicode cRight ) const
{
    int nKernAmount = 0;
    if( mpKerningPairs )
    {
        const KERNINGPAIRS aRefPair = { cLeft, cRight, 0 };
        const KERNINGPAIRS* pFirstPair = mpKerningPairs;
        const KERNINGPAIRS* pEndPair = mpKerningPairs + mnKerningPairs;
        const KERNINGPAIRS* pPair = std::lower_bound( pFirstPair,
            pEndPair, aRefPair, ImplCmpKernData );
        if( (pPair != pEndPair)
        &&  (pPair->sFirstChar == aRefPair.sFirstChar)
        &&  (pPair->sSecondChar == aRefPair.sSecondChar) )
            nKernAmount = pPair->lKerningAmount;
    }

    return nKernAmount;
}
#endif // GCP_KERN_HACK

// =======================================================================

ImplFontData* ImplOs2FontData::Clone() const
{
    if( mpUnicodeMap )
        mpUnicodeMap->AddReference();
    ImplFontData* pClone = new ImplOs2FontData( *this );
    return pClone;
}

// -----------------------------------------------------------------------

ImplFontEntry* ImplOs2FontData::CreateFontInstance( ImplFontSelectData& rFSD ) const
{
    ImplFontEntry* pEntry = new ImplOs2FontEntry( rFSD );
    return pEntry;
}

// =======================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
