/*************************************************************************
 *
 *  $RCSfile: winlayout.cxx,v $
 *
 *  $Revision: 1.54 $
 *
 *  last change: $Author: pl $ $Date: 2002-11-18 14:30:50 $
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

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif // _SV_SALGDI_HXX
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif // _SV_SALDATA_HXX

#ifndef _SV_SALLAYOUT_HXX
#include <sallayout.hxx>
#endif // _SV_SALLAYOUT_HXX

#include <cstdio>
#include <malloc.h>
#define alloca _alloca

#ifdef GCP_KERN_HACK
#include <algorithm>
#endif // GCP_KERN_HACK

// =======================================================================

class WinLayout : public SalLayout
{
public:
                    WinLayout( const ImplLayoutArgs& rArgs )
                        : SalLayout( rArgs ) {}
};

// =======================================================================

class SimpleWinLayout : public WinLayout
{
public:
                    SimpleWinLayout( HDC hDC, const ImplLayoutArgs&
#ifdef GCP_KERN_HACK
                        , const KERNINGPAIR* pPairs, int nPairs
#endif // GCP_KERN_HACK
                    );

    virtual         ~SimpleWinLayout();

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;

    virtual int     GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos, int&,
                        long* pGlyphAdvances, int* pCharIndexes ) const;

    virtual Point   GetCharPosition( int nCharPos, bool bRTL ) const;
    virtual long    FillDXArray( long* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual void    GetCaretPositions( long* ) const;

    // for glyph+font+script fallback
    virtual bool    ApplyFallback( SalLayout& rFallback );
    virtual void    UpdateGlyphPos( int nStart, int nXPos );
    virtual void    RemoveNotdefs();

protected:
    void            Justify( long nNewWidth );
    void            ApplyDXArray( const long* pDXArray );

private:
    HDC             mhDC;

    int             mnGlyphCount;
    int             mnCharCount;
    WCHAR*          mpOutGlyphs;
    int*            mpGlyphAdvances;
    int*            mpGlyphOrigAdvs;
    UINT*           mpChars2Glyphs;
    UINT*           mpGlyphs2Chars;
    int             mnNotdefWidth;
    long            mnWidth;

#ifdef GCP_KERN_HACK
    const KERNINGPAIR* mpKerningPairs;
    int             mnKerningPairs;
#endif // GCP_KERN_HACK
};

// -----------------------------------------------------------------------

SimpleWinLayout::SimpleWinLayout( HDC hDC, const ImplLayoutArgs& rArgs,
#ifdef GCP_KERN_HACK
        const KERNINGPAIR* pKerningPairs, int nKerningPairs
#endif // GCP_KERN_HACK
    )
:   WinLayout( rArgs ),
    mhDC( hDC ),
#ifdef GCP_KERN_HACK
    mpKerningPairs( pKerningPairs ),
    mnKerningPairs( nKerningPairs ),
#endif // GCP_KERN_HACK
    mnGlyphCount( 0 ),
    mnCharCount( 0 ),
    mpOutGlyphs( NULL ),
    mpGlyphAdvances( NULL ),
    mpGlyphOrigAdvs( NULL ),
    mpChars2Glyphs( NULL ),
    mpGlyphs2Chars( NULL ),
    mnNotdefWidth( -1 ),
    mnWidth( 0 )
{
    // TODO: use a cached value for bDisableGlyphs from upper layers
    if( (mnLayoutFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING) == 0 )
    {
        // Win32 glyph APIs have serious problems with vertical layout
        // => workaround is to use the unicode methods then
        bool bDisableGlyphs = false;

        if( rArgs.mnFlags & SAL_LAYOUT_VERTICAL )
            bDisableGlyphs = true;
        else
        {
            // #99019# don't use glyph indices for non-TT fonts
            // also for printer, because the drivers often transparently replace TTs with PS fonts
            TEXTMETRICA aTextMetricA;
            if( ::GetTextMetricsA( mhDC, &aTextMetricA ) )
                if( (aTextMetricA.tmPitchAndFamily & TMPF_DEVICE)
                ||  !(aTextMetricA.tmPitchAndFamily & TMPF_TRUETYPE) )
                    bDisableGlyphs = true;
        }

        if( bDisableGlyphs )
            mnLayoutFlags |= SAL_LAYOUT_DISABLE_GLYPH_PROCESSING;
    }

    // TODO: use a cached value for bDisableAsianKern from upper layers
    if( mnLayoutFlags & SAL_LAYOUT_KERNING_ASIAN )
    {
        TEXTMETRICA aTextMetricA;
        if( ::GetTextMetricsA( mhDC, &aTextMetricA )
        && !(aTextMetricA.tmPitchAndFamily & TMPF_FIXED_PITCH) )
            mnLayoutFlags &= ~SAL_LAYOUT_KERNING_ASIAN;
    }
}

// -----------------------------------------------------------------------

SimpleWinLayout::~SimpleWinLayout()
{
    delete[] mpGlyphs2Chars;
    delete[] mpChars2Glyphs;
    delete[] mpGlyphOrigAdvs;
    delete[] mpGlyphAdvances;
    delete[] mpOutGlyphs;
}

// -----------------------------------------------------------------------

bool SimpleWinLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    // layout text
    int nMaxGlyphCount = rArgs.mnEndCharPos - rArgs.mnMinCharPos;
    DWORD nGcpOption = 0;

    mpOutGlyphs     = new WCHAR[ nMaxGlyphCount ];
    mpGlyphAdvances = new int[ nMaxGlyphCount ];
    mnGlyphCount    = 0;

    if( rArgs.mnFlags & (SAL_LAYOUT_KERNING_PAIRS | SAL_LAYOUT_KERNING_ASIAN) )
        mpGlyphOrigAdvs = new int[ nMaxGlyphCount ];

#ifndef GCP_KERN_HACK
    // enable kerning if requested
    if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
        nGcpOption |= GCP_USEKERNING;
#endif // GCP_KERN_HACK

    // apply reordering if requested
    char* pGcpClass = NULL;
    if( (nMaxGlyphCount > 1) && !(rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG) )
    {
        mpGlyphs2Chars = new UINT[ nMaxGlyphCount ];

        pGcpClass = (char*)alloca( nMaxGlyphCount );
        nGcpOption |= GCP_REORDER;
        if( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL )
        {
            pGcpClass[0] = GCPCLASS_PREBOUNDRTL;
            pGcpClass[1] = GCPCLASS_POSTBOUNDRTL;
        }
        else
        {
            pGcpClass[0] = (char)GCPCLASS_PREBOUNDLTR;
            pGcpClass[1] = GCPCLASS_POSTBOUNDLTR;
        }
    }

    DWORD nRC;
    if( aSalShlData.mbWNT )  // TODO: remove when unicode layer successful
    {
        GCP_RESULTSW aGCPW;
        aGCPW.lStructSize   = sizeof(aGCPW);
        aGCPW.lpOutString   = NULL;
        aGCPW.lpGlyphs      = NULL;
        aGCPW.lpCaretPos    = NULL;
        aGCPW.lpClass       = pGcpClass;
        aGCPW.nGlyphs       = nMaxGlyphCount;
        aGCPW.nMaxFit       = 0;

        // get glyphs/outstring and kerned placement
        if( mnLayoutFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING )
            aGCPW.lpOutString = mpOutGlyphs;
        else
            aGCPW.lpGlyphs = mpOutGlyphs;
        aGCPW.lpOrder = mpGlyphs2Chars;
        aGCPW.lpDx = mpGlyphAdvances;
        nRC = ::GetCharacterPlacementW( mhDC,
            rArgs.mpStr + rArgs.mnMinCharPos, nMaxGlyphCount,
            0, &aGCPW, nGcpOption );
        mnGlyphCount = aGCPW.lpOutString ? aGCPW.nMaxFit : aGCPW.nGlyphs;

#ifndef GCP_KERN_HACK
        // get undisturbed placement
        if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
        {
            aGCPW.lpOrder = NULL;
            aGCPW.lpDx = mpGlyphOrigAdvs;
            nRC = ::GetCharacterPlacementW( mhDC,
                rArgs.mpStr + rArgs.mnMinCharPos, nMaxGlyphCount,
                0, &aGCPW, (nGcpOption & ~GCP_USEKERNING) );
        }
#endif // GCP_KERN_HACK
    }
    else
    {
        // TODO: emulate full GetCharacterPlacementW on non-unicode OS
        // TODO: move into uwinapi.dll
        // convert into ANSI code page
        int nMBLen = ::WideCharToMultiByte( CP_ACP,
            WC_COMPOSITECHECK | WC_DISCARDNS | WC_DEFAULTCHAR,
            rArgs.mpStr + rArgs.mnMinCharPos, nMaxGlyphCount,
            NULL, 0, NULL, NULL );
        if( (nMBLen <= 0) || (nMBLen >= 8 * nMaxGlyphCount) )
            return false;
        char* const pMBStr = (char*)alloca( nMBLen+1 );
        ::WideCharToMultiByte( CP_ACP,
            WC_COMPOSITECHECK | WC_DISCARDNS | WC_DEFAULTCHAR,
            rArgs.mpStr + rArgs.mnMinCharPos, nMaxGlyphCount,
            pMBStr, nMBLen, NULL, NULL );

        // resize arrays if needed
        int nArraySize = nMaxGlyphCount;
        if( nArraySize < nMBLen )
            nArraySize = nMBLen;

        if( nArraySize > nMaxGlyphCount )
        {
            delete[] mpGlyphAdvances;
            delete[] mpOutGlyphs;
            mpGlyphAdvances = new int[ nArraySize ];
            mpOutGlyphs = new WCHAR[ nArraySize ];
            if( mpGlyphOrigAdvs )
            {
                delete[] mpGlyphOrigAdvs;
                mpGlyphOrigAdvs = new int[ nArraySize ];
            }
        }

        // get glyphs/outstring and placement
        GCP_RESULTSA aGCPA;
        aGCPA.lStructSize   = sizeof(aGCPA);
        aGCPA.lpOutString   = NULL;
        aGCPA.lpGlyphs      = NULL;
        aGCPA.lpCaretPos    = NULL;
        aGCPA.lpClass       = pGcpClass;
        aGCPA.nMaxFit       = 0;
        aGCPA.nGlyphs       = nArraySize;

        // resize arrays if needed
        if( mnLayoutFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING )
            aGCPA.lpOutString = reinterpret_cast<char*>(mpOutGlyphs);
        else
            aGCPA.lpGlyphs = mpOutGlyphs;

        aGCPA.lpOrder = mpGlyphs2Chars;
        aGCPA.lpDx = mpGlyphAdvances;
        nRC = ::GetCharacterPlacementA( mhDC, pMBStr, nMBLen,
            0, &aGCPA, nGcpOption );
        if( aGCPA.lpGlyphs )
            mnGlyphCount = aGCPA.nGlyphs;
        else
        {
            mnCharCount = aGCPA.nMaxFit;
            mnGlyphCount = nMaxGlyphCount;
        }

        // TODO: map lpOrderA to lpOrderW
        // CHECK: lpDxA->lpDxW mapping

#ifndef GCP_KERN_HACK
        if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
        {
            aGCPA.lpOrder = NULL;
            aGCPA.lpDx = mpGlyphOrigAdvs;
            nRC = ::GetCharacterPlacementA( mhDC, pMBStr, nMBLen,
                0, &aGCPA, (nGcpOption & ~GCP_USEKERNING) );
        }
#endif // GCP_KERN_HACK
    }

    // cache essential layout properties
    mnWidth = nRC & 0xFFFF; // TODO: check API docs for clarification

    if( !nRC )
        return false;

    int i, j;
    // fixup strong RTL layout by reversing glyph order
    // TODO: mirror glyphs
    if( 0 == (~rArgs.mnFlags & (SAL_LAYOUT_BIDI_STRONG|SAL_LAYOUT_BIDI_RTL))
    && (1 < mnGlyphCount) )
    {
        mpGlyphs2Chars = new UINT[ nMaxGlyphCount ];
        i = 0;
        for( j = mnGlyphCount; --j >= i; ++i )
        {
            WCHAR nTempGlyph    = mpOutGlyphs[ i ];
            int nTempAdvance    = mpGlyphAdvances[ i ];
            mpOutGlyphs[i]      = mpOutGlyphs[ j ];
            mpGlyphAdvances[i]  = mpGlyphAdvances[ j ];
            mpGlyphs2Chars[i]   = j + (nMaxGlyphCount - mnGlyphCount);
            mpOutGlyphs[j]      = nTempGlyph;
            mpGlyphAdvances[j]  = nTempAdvance;
            mpGlyphs2Chars[j]   = i + (nMaxGlyphCount - mnGlyphCount);
        }
        for( i = mnGlyphCount; i < nMaxGlyphCount; ++i )
            mpGlyphs2Chars[ i ] = -1;
    }

    // #101097# fixup display of notdef glyphs
    // TODO: is there a way to convince Win32(incl W95) API to use notdef directly?
    for( i = 0; i < mnGlyphCount; ++i )
    {
        // we are only interested in notdef candidates
        if( mpGlyphAdvances[i] != 0 )
            continue;
        if( (mpOutGlyphs[i] != 0)
        && ((mpOutGlyphs[i] != 3) && !(mnLayoutFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING)) )
            continue;

        // request fallback
        int nCharPos = mpGlyphs2Chars ? mpGlyphs2Chars[ i ] : i;
        nCharPos += rArgs.mnMinCharPos;
        bool bRTL = false;  // TODO: get from run
        rArgs.NeedFallback( nCharPos, bRTL );

        // insert a dummy in the meantime
        if( mnNotdefWidth < 0 )
        {
            SIZE aExtent;
            static const WCHAR cNotDef = 0xFFFF;
            mnNotdefWidth = GetTextExtentPoint32W(mhDC,&cNotDef,1,&aExtent) ? aExtent.cx : 0;
        }

        if( !(mnLayoutFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING) )
            mpOutGlyphs[i] = 0;
        mpGlyphAdvances[i] = mnNotdefWidth;
        if( mpGlyphOrigAdvs )
            mpGlyphOrigAdvs[i] = mnNotdefWidth;
        mnWidth += mnNotdefWidth;
    }

    // calculate mpChars2Glyphs if glyph->char mapping also exists
    // note: glyph to char mapping is relative to first character
    if( mpGlyphs2Chars != NULL )
    {
        mpChars2Glyphs = new UINT[ nMaxGlyphCount ];
        for( i = 0; i < mnGlyphCount; ++i )
            mpChars2Glyphs[ mpGlyphs2Chars[ i ] ] = i;
        for(; i < nMaxGlyphCount; ++i )
            mpChars2Glyphs[ i ] = -1;
    }

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs.mpDXArray );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
#ifdef GCP_KERN_HACK
    else if( rArgs.mnFlags & (SAL_LAYOUT_KERNING_ASIAN|SAL_LAYOUT_KERNING_PAIRS) )
    {
#else // GCP_KERN_HACK
    else if( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN )
    {
        if( !(rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS) )
#endif // GCP_KERN_HACK
            for( i = 0; i < mnGlyphCount; ++i )
                mpGlyphOrigAdvs[i] = mpGlyphAdvances[i];

        bool bVertical = false;
        const xub_Unicode* pStr = rArgs.mpStr + rArgs.mnMinCharPos;
        // #99658# also do asian kerning one beyond substring
        int nLen = mnGlyphCount;
        if( rArgs.mnMinCharPos + nLen < rArgs.mnLength )
            ++nLen;
        for( i = 1; i < nLen; ++i )
        {
#ifdef GCP_KERN_HACK
            if( (rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS) && mnKerningPairs )
            {
                const KERNINGPAIR aRefPair = {pStr[i-1],pStr[i],0};
                const KERNINGPAIR* pPair = std::lower_bound( mpKerningPairs,
                    mpKerningPairs + mnKerningPairs, aRefPair, ImplCmpKernData );
                if( pPair->wFirst==aRefPair.wFirst && pPair->wSecond==aRefPair.wSecond )
                {
                    mpGlyphAdvances[ i-1 ] += pPair->iKernAmount;
                    mnWidth += pPair->iKernAmount;
                }
            }
            else if( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN )
#endif // GCP_KERN_HACK

            if( (0x3000 == (0xFF00 & pStr[i-1]))
            &&  (0x3000 == (0xFF00 & pStr[i])) )
            {
                long nKernFirst = +CalcAsianKerning( pStr[i-1], true, bVertical );
                long nKernNext  = -CalcAsianKerning( pStr[i], false, bVertical );

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

    return true;
}

// -----------------------------------------------------------------------

int SimpleWinLayout::GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos, int& nStart,
    long* pGlyphAdvances, int* pCharIndexes ) const
{
    // return zero if no more glyph found
    if( nStart >= mnGlyphCount )
        return 0;

    // calculate glyph position relative to layout base
    // TODO: avoid for nStart!=0 case by reusing rPos
    long nXOffset = 0;
    for( int i = 0; i < nStart; ++i )
        nXOffset += mpGlyphAdvances[ i ];

    // calculate absolute position in pixel units
    Point aRelativePos( nXOffset, 0 );
    rPos = GetDrawPosition( aRelativePos );

    int nCount = 0;
    while( nCount < nLen )
    {
        // update returned values
        long nGlyphIndex = mpOutGlyphs[ nStart ];
        if( mnLayoutFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING )
        {
            if( mnLayoutFlags & SAL_LAYOUT_VERTICAL )
                nGlyphIndex |= GetVerticalFlags( (sal_Unicode)(nGlyphIndex & GF_IDXMASK) );
            nGlyphIndex |= GF_ISCHAR;
        }
        *(pGlyphs++) = nGlyphIndex;
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = mpGlyphAdvances[ nStart ];
        if( pCharIndexes )
        {
            int nCharPos = mpGlyphs2Chars ? mpGlyphs2Chars[nStart] : nStart;
            if( nCharPos >= 0 )
                nCharPos += mnMinCharPos;
            *(pCharIndexes++) = nCharPos;
        }

        // stop at last glyph
        ++nCount;
        if( ++nStart >= mnGlyphCount )
            break;

        // stop when next x-position is unexpected
        if( !pGlyphAdvances && mpGlyphOrigAdvs )
            if( mpGlyphAdvances[nStart] != mpGlyphOrigAdvs[nStart] )
                break;
    }

    return nCount;
}

// -----------------------------------------------------------------------

void SimpleWinLayout::DrawText( SalGraphics& ) const
{
    if( mnGlyphCount <= 0 )
        return;

    UINT mnDrawOptions = ETO_GLYPH_INDEX;
    if( mnLayoutFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING )
        mnDrawOptions = 0;

#ifndef DEBUG_GETNEXTGLYPHS
    Point aPos = GetDrawPosition();

    if( mnDrawOptions || aSalShlData.mbWNT )
    {
        ::ExtTextOutW( mhDC, aPos.X(), aPos.Y(), mnDrawOptions, NULL,
            mpOutGlyphs, mnGlyphCount, mpGlyphAdvances );
    }
    else
    {
        const char* pAnsiStr = (char*)mpOutGlyphs;
        int* pAnsiAdv = mpGlyphAdvances;

        // adjust advance widths when DBCS present
        if( mnGlyphCount != mnCharCount )
        {
            pAnsiAdv = (int*)alloca( mnCharCount * sizeof(int) );
            for( int i = 0, j = 0; i < mnCharCount; ++j, ++i )
            {
                pAnsiAdv[i] = mpGlyphAdvances[j];
                if( (pAnsiStr[i] & 0x80) && ::IsDBCSLeadByte( pAnsiStr[i] ) )
                    pAnsiAdv[ ++i ] = 0;    // trailing byte advance width
            }
            DBG_ASSERT( (j==mnGlyphCount), "SimpleWinLayout::DrawTextA DBCS mismatch" );
        }

        ::ExtTextOutA( mhDC, aPos.X(), aPos.Y(), mnDrawOptions, NULL,
            pAnsiStr, mnCharCount, pAnsiAdv );
    }
#else // DEBUG_GETNEXTGLYPHS
    #define MAXGLYPHCOUNT 8
    long pLGlyphs[ MAXGLYPHCOUNT ];
    long pWidths[ MAXGLYPHCOUNT ];
    int  pCharPosAry[ MAXGLYPHCOUNT ];
    Point aPos;
    for( int nStart = 0;;)
    {
        int nGlyphs = GetNextGlyphs( MAXGLYPHCOUNT, pLGlyphs, aPos, nStart, pWidths, pCharPosAry );
        if( !nGlyphs )
            break;

        WCHAR pWGlyphs[ MAXGLYPHCOUNT ];
        int pGlyphWidths[ MAXGLYPHCOUNT ];
        for( int i = 0; i < nGlyphs; ++i )
        {
            pWGlyphs[i] = pLGlyphs[i];
            pGlyphWidths[i] = pWidths[i];
        }

        ::ExtTextOutW( mhDC, aPos.X(), aPos.Y(), mnDrawOptions, NULL,
            pWGlyphs, nGlyphs, pGlyphWidths );
    }
#endif // DEBUG_GETNEXTGLYPHS
}

// -----------------------------------------------------------------------

long SimpleWinLayout::FillDXArray( long* pDXArray ) const
{
    if( mnWidth && !pDXArray )
        return mnWidth;

    long nWidth = 0;

    for( int i = 0; i < mnGlyphCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        nWidth += mpGlyphAdvances[j];
        if( pDXArray )
            pDXArray[i] = nWidth;
    }

    return nWidth;
}

// -----------------------------------------------------------------------

int SimpleWinLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    long nWidth = 0;

    for( int i = 0; i < mnGlyphCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        nWidth += mpGlyphAdvances[j] * nFactor;
        if( nWidth >= nMaxWidth )
            return (mnMinCharPos + i);
        nWidth += nCharExtra;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

void SimpleWinLayout::GetCaretPositions( long* pCaretXArray ) const
{
    long nXPos = 0;
    int nMaxIdx = 2 * (mnEndCharPos - mnMinCharPos), i;

    if( !mpGlyphs2Chars )
    {
        for( i = 0; i < nMaxIdx; i += 2 )
        {
            pCaretXArray[ i ] = nXPos;
            nXPos += mpGlyphAdvances[ i>>1 ];
            pCaretXArray[ i+1 ] = nXPos;
        }
    }
    else
    {
        for( i = 0; i < nMaxIdx; ++i )
            pCaretXArray[ i ] = -1;

        // assign glyph positions to character positions
        int nLeftIdx = 0;
        for( i = 0; i < mnGlyphCount; ++i )
        {
            long nXRight = nXPos + mpGlyphAdvances[ i ];
            int nCurrIdx = 2 * mpGlyphs2Chars[ i ];
            if( nLeftIdx <= nCurrIdx )
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
            nLeftIdx = nCurrIdx;
            nXPos = nXRight;
        }

        // fixup unknown character positions to neighbor
        for( i = 0; i < nMaxIdx; ++i )
        {
            if( pCaretXArray[ i ] >= 0 )
                nXPos = pCaretXArray[ i ];
            else
                pCaretXArray[ i ] = nXPos;
        }
    }
}

// -----------------------------------------------------------------------

Point SimpleWinLayout::GetCharPosition( int nCharPos, bool bRTL ) const
{
    //TODO: implement reordering using mpChars2Glyphs[i]
    long nXPos = 0;

    if( mnGlyphCount <= 0 )
        return Point( nXPos, 0 );

    if( !bRTL ) // relative to left edge
    {
        int nCharLimit = mnEndCharPos;
        if( nCharLimit > nCharPos )
            nCharLimit = nCharPos;
        nCharLimit -= mnMinCharPos;
        for( int i = 0; i < nCharLimit; ++i )
        {
            int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
            if( j >= 0 )
                nXPos += mpGlyphAdvances[j];
        }
    }
    else        // relative to right edge?
    {
        int nCharStart = nCharPos - mnMinCharPos;
        if( nCharStart < 0 )
            nCharStart = 0;
        int nCharLimit = mnEndCharPos - mnMinCharPos;
        for( int i = nCharStart; i < nCharLimit; ++i )
        {
            int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
            if( j >= 0 )
                nXPos -= mpGlyphAdvances[j];
        }
    }

    return Point( nXPos, 0 );
}

// -----------------------------------------------------------------------

void SimpleWinLayout::Justify( long nNewWidth )
{
    long nOldWidth = mnWidth;
    mnWidth = nNewWidth;

    if( mnGlyphCount <= 0 )
        return;

    if( nNewWidth == nOldWidth )
        return;

    int i = mnGlyphCount - 1;
    // the rightmost glyph cannot be stretched
    nOldWidth -= mpGlyphAdvances[i];
    nNewWidth -= mpGlyphAdvances[i];

    // stretch remaining glyphs to new width
    while( --i >= 0 )
    {
        if( nNewWidth < 0 )
            nNewWidth = 0;
        double fStretch = (nOldWidth<0) ? 0.0 : (double)nNewWidth / nOldWidth;
        nOldWidth -= mpGlyphAdvances[i];
        mpGlyphAdvances[i] = (long)(mpGlyphAdvances[i] * fStretch + 0.5);
        nNewWidth -= mpGlyphAdvances[i];
    }
}

// -----------------------------------------------------------------------

void SimpleWinLayout::ApplyDXArray( const long* pDXArray )
{
    // try to avoid disturbance of text flow for LSB rounding case;
    int i = 0;
    long nOldWidth = 0;
    for(; i < mnGlyphCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        if( j >= 0 )
        {
            nOldWidth += mpGlyphAdvances[ j ];
            int nDiff = nOldWidth - pDXArray[ i ];
#if 0       // disabled because of #104768#
            // works great for static text, but problems when it gets changed
            if( nDiff>+1 || nDiff<-1 )
#else
            // only bother with changing anything when something moved
            if( nDiff != 0 )
#endif
                break;
        }
    }
    if( i >= mnGlyphCount )
        return;

    if( !mpGlyphOrigAdvs )
    {
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphOrigAdvs[ i ] = mpGlyphAdvances[ i ];
    }

    mnWidth = 0;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        if( j >= 0 )
            mpGlyphAdvances[j] = pDXArray[i] - mnWidth;
        mnWidth = pDXArray[i];
    }
}

// -----------------------------------------------------------------------

bool SimpleWinLayout::ApplyFallback( SalLayout& rFallback )
{
    // TODO
    return false;
}

// -----------------------------------------------------------------------

void SimpleWinLayout::UpdateGlyphPos( int nStart, int nXPos )
{
    // TODO
}

// -----------------------------------------------------------------------

void SimpleWinLayout::RemoveNotdefs()
{
    // TODO
}

// =======================================================================

#ifdef USE_UNISCRIBE
#include <Usp10.h>

struct VisualItem
{
    SCRIPT_ITEM*    mpScriptItem;
    int             mnMinGlyphPos;
    int             mnEndGlyphPos;
    int             mnMinCharPos;
    int             mnEndCharPos;
    long            mnPixelWidth;
    ABC             maABCWidths;
};

class UniscribeLayout : public WinLayout
{
public:
                    UniscribeLayout( HDC hDC, const ImplLayoutArgs& );
    virtual         ~UniscribeLayout();

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;
    virtual int     GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos, int&,
                        long* pGlyphAdvances, int* pCharPosAry ) const;

    virtual long    FillDXArray( long* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual void    GetCaretPositions( long* ) const;
    virtual Point   GetCharPosition( int nCharPos, bool bRTL ) const;

    // for glyph+font+script fallback
    virtual bool    ApplyFallback( SalLayout& rFallback );
    virtual void    UpdateGlyphPos( int nStart, int nXPos );
    virtual void    RemoveNotdefs();

protected:
    void            Justify( long nNewWidth );
    void            ApplyDXArray( const long* pDXArray );

    bool            GetItemSubrange( const VisualItem& rVisualItem,
                        int& rMinIndex, int& rEndIndex ) const;

private:
    // item specific info
    SCRIPT_ITEM*    mpScriptItems;  // in logical order
    VisualItem*     mpVisualItems;  // in visual order
    int             mnItemCount;

    // string specific info
    // everything is in logical order
    int             mnCharCapacity;
    WORD*           mpLogClusters;
    int*            mpCharWidths;
    int             mnSubStringMin;

    // glyph specific info
    // everything is in visual order
    int             mnGlyphCount;
    int             mnGlyphCapacity;
    int*            mpGlyphAdvances;
    int*            mpJustifications;
    WORD*           mpOutGlyphs;
    GOFFSET*        mpGlyphOffsets;
    SCRIPT_VISATTR* mpVisualAttrs;
    int*            mpGlyphs2Chars;

    // platform specific info
    HDC                     mhDC;
    mutable SCRIPT_CACHE    maScriptCache;
};

// -----------------------------------------------------------------------
// dynamic loading of usp library

static HMODULE aUspModule;
static bool bUspEnabled = true;

static HRESULT ((WINAPI *pScriptIsComplex)( const WCHAR*, int, DWORD ));
static HRESULT ((WINAPI *pScriptItemize)( const WCHAR*, int, int,
    const SCRIPT_CONTROL*, const SCRIPT_STATE*, SCRIPT_ITEM*, int* ));
static HRESULT ((WINAPI *pScriptShape)( HDC, SCRIPT_CACHE*, const WCHAR*,
    int, int, SCRIPT_ANALYSIS*, WORD*, WORD*, SCRIPT_VISATTR*, int* ));
static HRESULT ((WINAPI *pScriptPlace)( HDC, SCRIPT_CACHE*, const WORD*, int,
    const SCRIPT_VISATTR*, SCRIPT_ANALYSIS*, int*, GOFFSET*, ABC* ));
static HRESULT ((WINAPI *pScriptGetLogicalWidths)( const SCRIPT_ANALYSIS*,
    int, int, const int*, const WORD*, const SCRIPT_VISATTR*, int* ));
static HRESULT ((WINAPI *pScriptApplyLogicalWidth)( const int*, int, int, const WORD*,
    const SCRIPT_VISATTR*, const int*, const SCRIPT_ANALYSIS*, ABC*, int* ));
static HRESULT ((WINAPI *pScriptJustify)( const SCRIPT_VISATTR*,
    const int*, int, int, int, int* ));
static HRESULT ((WINAPI *pScriptTextOut)( const HDC, SCRIPT_CACHE*,
    int, int, UINT, const RECT*, const SCRIPT_ANALYSIS*, const WCHAR*,
    int, const WORD*, int, const int*, const int*, const GOFFSET* ));
static HRESULT ((WINAPI *pScriptGetFontProperties)( HDC, SCRIPT_CACHE*, SCRIPT_FONTPROPERTIES* ));
static HRESULT ((WINAPI *pScriptFreeCache)( SCRIPT_CACHE* ));

// -----------------------------------------------------------------------

static bool InitUSP()
{
    aUspModule = LoadLibraryA( "usp10" );
    if( !aUspModule )
        return (bUspEnabled = false);

    pScriptIsComplex = (HRESULT (WINAPI*)(const WCHAR*,int,DWORD))
        ::GetProcAddress( aUspModule, "ScriptIsComplex" );
    bUspEnabled &= (NULL != pScriptIsComplex);

    pScriptItemize = (HRESULT (WINAPI*)(const WCHAR*,int,int,
        const SCRIPT_CONTROL*,const SCRIPT_STATE*,SCRIPT_ITEM*,int*))
        ::GetProcAddress( aUspModule, "ScriptItemize" );
    bUspEnabled &= (NULL != pScriptItemize);

    pScriptShape = (HRESULT (WINAPI*)(HDC,SCRIPT_CACHE*,const WCHAR*,
        int,int,SCRIPT_ANALYSIS*,WORD*,WORD*,SCRIPT_VISATTR*,int*))
        ::GetProcAddress( aUspModule, "ScriptShape" );
    bUspEnabled &= (NULL != pScriptShape);

    pScriptPlace = (HRESULT (WINAPI*)(HDC, SCRIPT_CACHE*, const WORD*, int,
        const SCRIPT_VISATTR*,SCRIPT_ANALYSIS*,int*,GOFFSET*,ABC*))
        ::GetProcAddress( aUspModule, "ScriptPlace" );
    bUspEnabled &= (NULL != pScriptPlace);

    pScriptGetLogicalWidths = (HRESULT (WINAPI*)(const SCRIPT_ANALYSIS*,
        int,int,const int*,const WORD*,const SCRIPT_VISATTR*,int*))
        ::GetProcAddress( aUspModule, "ScriptGetLogicalWidths" );
    bUspEnabled &= (NULL != pScriptGetLogicalWidths);

    pScriptApplyLogicalWidth = (HRESULT (WINAPI*)(const int*,int,int,const WORD*,
        const SCRIPT_VISATTR*,const int*,const SCRIPT_ANALYSIS*,ABC*,int*))
        ::GetProcAddress( aUspModule, "ScriptApplyLogicalWidth" );
    bUspEnabled &= (NULL != pScriptApplyLogicalWidth);

    pScriptJustify = (HRESULT (WINAPI*)(const SCRIPT_VISATTR*,const int*,
        int,int,int,int*))
        ::GetProcAddress( aUspModule, "ScriptJustify" );
    bUspEnabled &= (NULL != pScriptJustify);

    pScriptGetFontProperties = (HRESULT (WINAPI*)( HDC,SCRIPT_CACHE*,SCRIPT_FONTPROPERTIES*))
        ::GetProcAddress( aUspModule, "ScriptGetFontProperties" );
    bUspEnabled &= (NULL != pScriptGetFontProperties);

    pScriptTextOut = (HRESULT (WINAPI*)(const HDC,SCRIPT_CACHE*,
        int,int,UINT,const RECT*,const SCRIPT_ANALYSIS*,const WCHAR*,
        int,const WORD*,int,const int*,const int*,const GOFFSET*))
        ::GetProcAddress( aUspModule, "ScriptTextOut" );
    bUspEnabled &= (NULL != pScriptTextOut);

    pScriptFreeCache = (HRESULT (WINAPI*)(SCRIPT_CACHE*))
        ::GetProcAddress( aUspModule, "ScriptFreeCache" );
    bUspEnabled &= (NULL != pScriptFreeCache);

    if( !bUspEnabled )
    {
        FreeLibrary( aUspModule );
        aUspModule = NULL;
    }

    return bUspEnabled;
}

// -----------------------------------------------------------------------

UniscribeLayout::UniscribeLayout( HDC hDC, const ImplLayoutArgs& rArgs )
:   WinLayout( rArgs ),
    mhDC( hDC ),
    maScriptCache( NULL ),
    mnItemCount(0),
    mpScriptItems( NULL ),
    mpVisualItems( NULL ),
    mpLogClusters( NULL ),
    mpCharWidths( NULL ),
    mnCharCapacity( 0 ),
    mnSubStringMin( 0 ),
    mnGlyphCapacity(0),
    mnGlyphCount( 0 ),
    mpOutGlyphs( NULL ),
    mpGlyphAdvances( NULL ),
    mpJustifications( NULL ),
    mpGlyphOffsets( NULL ),
    mpVisualAttrs( NULL ),
    mpGlyphs2Chars( NULL )
{}

// -----------------------------------------------------------------------

UniscribeLayout::~UniscribeLayout()
{
    (*pScriptFreeCache)( &maScriptCache );

    delete[] mpScriptItems;
    delete[] mpVisualItems;
    delete[] mpLogClusters;
    delete[] mpCharWidths;
    delete[] mpOutGlyphs;
    delete[] mpGlyphAdvances;
    delete[] mpJustifications;
    delete[] mpGlyphOffsets;
    delete[] mpVisualAttrs;
    delete[] mpGlyphs2Chars;
}

// -----------------------------------------------------------------------

bool UniscribeLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    // determine script items from string
    // TODO: try to avoid itemization since it costs a lot of performance
    SCRIPT_STATE aScriptState = {0,false,false,false,false,false,false,false,false,0,0};
    aScriptState.uBidiLevel         = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL));
    aScriptState.fOverrideDirection = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG));
    aScriptState.fDigitSubstitute   = (0 != (rArgs.mnFlags & SAL_LAYOUT_SUBSTITUTE_DIGITS));
    aScriptState.fArabicNumContext  = aScriptState.fDigitSubstitute & aScriptState.uBidiLevel;
    DWORD nLangId = 0;  // TODO: get language from font
    SCRIPT_CONTROL aScriptControl = {nLangId,false,false,false,false,false,false,false,false,0};
    aScriptControl.fContextDigits   = (0 != (rArgs.mnFlags & SAL_LAYOUT_SUBSTITUTE_DIGITS));
    // determine relevant substring and work only on it
    // when Bidi status is unknown we need to look at the whole string though
    mnSubStringMin = 0;
    int nSubStringEnd = rArgs.mnLength;
    if( aScriptState.fOverrideDirection )
    {
        mnSubStringMin = std::max( rArgs.mnMinCharPos - 8, 0 );
        nSubStringEnd = std::min( rArgs.mnEndCharPos + 8, rArgs.mnLength );
    }
    for( int nItemCapacity = 8; /*FOREVER*/; nItemCapacity *= 2 )
    {
        mpScriptItems = new SCRIPT_ITEM[ nItemCapacity ];
        HRESULT nRC = (*pScriptItemize)(
            rArgs.mpStr + mnSubStringMin, nSubStringEnd - mnSubStringMin,
            nItemCapacity, &aScriptControl, &aScriptState, mpScriptItems, &mnItemCount );
        if( !nRC )  // break loop when everything is correctly itemized
            break;
        delete[] mpScriptItems;
        mpScriptItems = NULL;
        if( (nRC != E_OUTOFMEMORY) || (nItemCapacity > rArgs.mnLength) )
            return false;
    }

    // calculate visual items order
    mpVisualItems = new VisualItem[ mnItemCount ];

    // default item ordering and adding substring offset
    int nItem;
    for( nItem = 0; nItem < mnItemCount; ++nItem )
    {
        mpScriptItems[ nItem ].iCharPos += mnSubStringMin;
        mpVisualItems[ nItem ].mpScriptItem = &mpScriptItems[ nItem ];
    }
    mpScriptItems[ mnItemCount ].iCharPos += mnSubStringMin;

    // calculate visual item order
    if( rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG )
    {
        // override item ordering if requested
        if( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL )
        {
            VisualItem*  pVI = &mpVisualItems[0];
            SCRIPT_ITEM* pSI = &mpScriptItems[mnItemCount];
            for( nItem = mnItemCount; --nItem >= 0;)
                (pVI++)->mpScriptItem = --pSI;
        }
    }
    else if( mnItemCount > 1 )
    {
        // apply bidi algorithm's rule L2 on item level
        // TODO: use faster L2 algorithm
        int nMaxBidiLevel = 0;
        VisualItem* pVI = &mpVisualItems[0];
        VisualItem* const pVIend = pVI + mnItemCount;
        for(; pVI < pVIend; ++pVI )
            if( nMaxBidiLevel < pVI->mpScriptItem->a.s.uBidiLevel )
                nMaxBidiLevel = pVI->mpScriptItem->a.s.uBidiLevel;

        while( --nMaxBidiLevel >= 0 )
        {
            for( pVI = &mpVisualItems[0]; pVI < pVIend; )
            {
                // find item range that needs reordering
                for(; pVI < pVIend; ++pVI )
                    if( nMaxBidiLevel < pVI->mpScriptItem->a.s.uBidiLevel )
                        break;
                VisualItem* pVImin = pVI++;
                for(; pVI < pVIend; ++pVI )
                    if( nMaxBidiLevel >= pVI->mpScriptItem->a.s.uBidiLevel )
                        break;
                VisualItem* pVImax = pVI++;

                // reverse order of items in this range
                for(; pVImin < --pVImax; ++pVImin )
                {
                    SCRIPT_ITEM* pSI = pVImin->mpScriptItem;
                    pVImin->mpScriptItem = pVImax->mpScriptItem;
                    pVImax->mpScriptItem = pSI;
                }
            }
        }
    }

    // allocate arrays
    // TODO: when reusing object reuse old allocations or delete them
    // TODO: use only [nSubStringMin..nSubStringEnd) instead of [0..nSubStringEnd)
    mnCharCapacity  = nSubStringEnd;
    mpLogClusters   = new WORD[ mnCharCapacity ];
    mpCharWidths    = new int[ mnCharCapacity ];

    mnGlyphCount    = 0;
    mnGlyphCapacity = 16 + 2 * rArgs.mnLength;  // worst case assumption
    mpGlyphAdvances = new int[ mnGlyphCapacity ];
    mpOutGlyphs     = new WORD[ mnGlyphCapacity ];
    mpGlyphOffsets  = new GOFFSET[ mnGlyphCapacity ];
    mpVisualAttrs   = new SCRIPT_VISATTR[ mnGlyphCapacity ];

    // layout script items
    for( nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // intitialize visual item info
        rVisualItem.mnMinGlyphPos = mnGlyphCount;
        rVisualItem.mnEndGlyphPos = 0;
        rVisualItem.mnMinCharPos  = rVisualItem.mpScriptItem[0].iCharPos;
        rVisualItem.mnEndCharPos  = rVisualItem.mpScriptItem[1].iCharPos;
        rVisualItem.mnPixelWidth  = 0;

        // shortcut for skipped items
        if( (rArgs.mnEndCharPos <= rVisualItem.mnMinCharPos)
         || (rArgs.mnMinCharPos >= rVisualItem.mnEndCharPos) )
        {
            for( int i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; ++i )
                mpLogClusters[ i ] = 0;
            continue;
        }

        // override bidi analysis if requested
        if( rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG )
        {
            rVisualItem.mpScriptItem->a.fRTL                 = aScriptState.uBidiLevel;;
            rVisualItem.mpScriptItem->a.s.uBidiLevel         = aScriptState.uBidiLevel;
            rVisualItem.mpScriptItem->a.s.fOverrideDirection = aScriptState.fOverrideDirection;
        }

        int nGlyphCount = 0;
        int nCharCount = rVisualItem.mnEndCharPos - rVisualItem.mnMinCharPos;
        HRESULT nRC = (*pScriptShape)( mhDC, &maScriptCache,
            rArgs.mpStr + rVisualItem.mnMinCharPos,
            nCharCount,
            mnGlyphCapacity - rVisualItem.mnMinGlyphPos,
            &rVisualItem.mpScriptItem->a,
            mpOutGlyphs + rVisualItem.mnMinGlyphPos,
            mpLogClusters + rVisualItem.mnMinCharPos,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            &nGlyphCount );

        if( nRC == USP_E_SCRIPT_NOT_IN_FONT )
        {
            // request fallback using different font
            rArgs.NeedFallback( rVisualItem.mnMinCharPos, rVisualItem.mnEndCharPos,
                rVisualItem.mpScriptItem->a.fRTL);

            // for now fall back to default layout
            rVisualItem.mpScriptItem->a.eScript = SCRIPT_UNDEFINED;
            nRC = (*pScriptShape)( mhDC, &maScriptCache,
                rArgs.mpStr + rVisualItem.mnMinCharPos,
                nCharCount,
                mnGlyphCapacity - rVisualItem.mnMinGlyphPos,
                &rVisualItem.mpScriptItem->a,
                mpOutGlyphs + rVisualItem.mnMinGlyphPos,
                mpLogClusters + rVisualItem.mnMinCharPos,
                mpVisualAttrs + rVisualItem.mnMinGlyphPos,
                &nGlyphCount );
        }

        if( nRC != 0 )
            continue;

        // request fallback for notdef glyphs
        for( int i = 0; i < nGlyphCount; ++i )
        {
            if( mpOutGlyphs[i] != 0 )
                continue;

            // TODO: optimize calculation of nCharPos
            int nCharPos = rVisualItem.mnMinCharPos;
            for( int j = nCharCount; --j >= 0; ++nCharPos )
                if( mpLogClusters[ nCharPos ] == i )
                    break;

            rArgs.NeedFallback( nCharPos, rVisualItem.mpScriptItem->a.fRTL);
        }

        nRC = (*pScriptPlace)( mhDC, &maScriptCache,
            mpOutGlyphs + rVisualItem.mnMinGlyphPos,
            nGlyphCount,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            &rVisualItem.mpScriptItem->a,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            mpGlyphOffsets + rVisualItem.mnMinGlyphPos,
            &rVisualItem.maABCWidths );

        if( nRC != 0 )
            continue;

        const ABC& rABC = rVisualItem.maABCWidths;
        rVisualItem.mnPixelWidth = rABC.abcA + rABC.abcB + rABC.abcC;

        nRC = (*pScriptGetLogicalWidths)(
            &rVisualItem.mpScriptItem->a,
            nCharCount, nGlyphCount,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            mpLogClusters + rVisualItem.mnMinCharPos,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            mpCharWidths + rVisualItem.mnMinCharPos );

        // update glyphcount only when we didn't run into a problem
        mnGlyphCount += nGlyphCount;
        rVisualItem.mnEndGlyphPos = mnGlyphCount;
    }

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs.mpDXArray );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );

    return true;
}

// -----------------------------------------------------------------------

bool UniscribeLayout::GetItemSubrange( const VisualItem& rVisualItem,
    int& rMinGlyphPos, int& rEndGlyphPos ) const
{
    // return early when nothing of interst in this item
    if( (rVisualItem.mnEndCharPos <= mnMinCharPos) || (mnEndCharPos <= rVisualItem.mnMinCharPos) )
        return false;

    // default: subrange is complete range
    rMinGlyphPos = rVisualItem.mnMinGlyphPos;
    rEndGlyphPos = rVisualItem.mnEndGlyphPos;

    // return early when the whole item is of interest
    if( (mnMinCharPos <= rVisualItem.mnMinCharPos) && (rVisualItem.mnEndCharPos <= mnEndCharPos ) )
        return true;

    // get glyph range from char range by looking at cluster boundries
    // TODO: optimize for case that LTR/RTL correspond to monotonous glyph indexes
    rMinGlyphPos = rVisualItem.mnEndGlyphPos;
    int nMaxGlyphPos = 0;

    int i = mnMinCharPos;
    if( i < rVisualItem.mnMinCharPos )
        i = rVisualItem.mnMinCharPos;
    int nCharPosLimit = rVisualItem.mnEndCharPos;
    if( nCharPosLimit > mnEndCharPos )
        nCharPosLimit = mnEndCharPos;
    for(; i < nCharPosLimit; ++i )
    {
        int n = mpLogClusters[ i ] + rVisualItem.mnMinGlyphPos;
        if( rMinGlyphPos > n )
            rMinGlyphPos = n;
        if( nMaxGlyphPos < n )
            nMaxGlyphPos = n;
    }

    // account for multiple glyphs at rightmost character
    // test only needed when rightmost glyph isn't referenced
    if( rEndGlyphPos > nMaxGlyphPos + 1 )
    {
        // find cluster end
        // TODO: optimize for case that LTR/RTL correspond to monotonous glyph indexes
        for( int i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; ++i )
        {
            int n = mpLogClusters[ i ] + rVisualItem.mnMinGlyphPos;
            if( (rEndGlyphPos > n) && (n > nMaxGlyphPos) )
            {
                rEndGlyphPos = n;
                if( n-1 <= nMaxGlyphPos )
                    break;
            }
        }
    }

    return true;
}

// -----------------------------------------------------------------------

int UniscribeLayout::GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos,
    int& nVisualStart, long* pGlyphAdvances, int* pCharPosAry ) const
{
    // return zero if no more glyph found
    if( nVisualStart >= mnGlyphCount )
        return 0;

    // calculate glyph position relative to layout base
    // TODO: avoid for nStart!=0 case by reusing rPos
    long nXOffset = 0;
    const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;

    int nVisualMinGlyphPos = 0;
    int nNextVisualStart = 0;
    int nMinGlyphPos = 0;
    int nCurrGlyphPos = 0;
    int nEndGlyphPos = mnGlyphCount;

    // search item with the visual starting glyph,
    // update position in items with visible glyphs
    int i;
    const VisualItem* pVisualItem = NULL;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        // skip invisible items
        pVisualItem = &mpVisualItems[ nItem ];
        if( !GetItemSubrange( *pVisualItem, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        nVisualMinGlyphPos = nNextVisualStart;
        nNextVisualStart += pVisualItem->mnEndGlyphPos - pVisualItem->mnMinGlyphPos;
        if( nNextVisualStart > nVisualStart )
            break;

        // advance to next visual position by using adjusted glyph widths
        // TODO: shortcut addition with cached ABC width if possible
        for( i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            nXOffset += pGlyphWidths[ i ];
    }

    // adjust visual glyph pos to first visible glyph in item
    if( nVisualStart == nVisualMinGlyphPos )
        nVisualStart += nMinGlyphPos - pVisualItem->mnMinGlyphPos;

    nCurrGlyphPos = pVisualItem->mnMinGlyphPos + (nVisualStart - nVisualMinGlyphPos);

    // advance to next visual position by using adjusted glyph widths
    for( i = nMinGlyphPos; i < nCurrGlyphPos; ++i )
        nXOffset += pGlyphWidths[ i ];

    // adjust nXOffset relative to cluster start
    i = mnMinCharPos;
    if( !pVisualItem->mpScriptItem->a.fRTL )
    {
        int nTmpIndex = mpLogClusters[ i ];
        while( (--i >= pVisualItem->mnMinCharPos) && (nTmpIndex == mpLogClusters[i]) )
            nXOffset -= mpCharWidths[i];
    }
    else
    {
        int nTmpIndex = mpLogClusters[ pVisualItem->mnEndCharPos - 1 ];
        while( (--i >= pVisualItem->mnMinCharPos) && (nTmpIndex == mpLogClusters[i]) )
            nXOffset += mpCharWidths[i];
    }

    // calculate absolute position in pixel units
    const GOFFSET aGOffset = mpGlyphOffsets[ nCurrGlyphPos ];
    Point aRelativePos( nXOffset + aGOffset.du, aGOffset.dv );
    rPos = GetDrawPosition( aRelativePos );

    if( pCharPosAry && !mpGlyphs2Chars )
        {} //TODO: implement, also see usage below

    int nCount = 0;
    while( nCount < nLen )
    {
        // update returned values
        *(pGlyphs++) = mpOutGlyphs[ nCurrGlyphPos ];
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = pGlyphWidths[ nCurrGlyphPos ];
        if( pCharPosAry )
            *(pCharPosAry++) = -1; // TODO: use mpGlyphs2Chars[nGI];

        ++nCount;
        ++nVisualStart;
        // stop aftert last visible glyph in item
        if( ++nCurrGlyphPos >= nEndGlyphPos )
        {
            nVisualStart = nNextVisualStart;
            break;
        }

        // stop when next x-position is unexpected
        if( !pGlyphAdvances  )
            if( (mpGlyphOffsets && (mpGlyphOffsets[nCurrGlyphPos].du != aGOffset.du) )
             || (mpJustifications && (mpJustifications[nCurrGlyphPos] != mpGlyphAdvances[nCurrGlyphPos]) ) )
                break;

        // stop when next y-position is unexpected
        if( mpGlyphOffsets && (mpGlyphOffsets[nCurrGlyphPos].dv != aGOffset.dv) )
            break;
    }

    return nCount;
}

// -----------------------------------------------------------------------

void UniscribeLayout::DrawText( SalGraphics& ) const
{
#ifndef DEBUG_GETNEXTGLYPHS
    Point aRelPos = Point(0,0);

    const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // skip if there is nothing to display
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        // adjust draw position relative to cluster start
        int i = mnMinCharPos;
        if( !rVisualItem.mpScriptItem->a.fRTL )
        {
            int nTmpIndex = nMinGlyphPos;
            while( (--i >= rVisualItem.mnMinCharPos) && (nTmpIndex == mpLogClusters[i]) )
                aRelPos.X() -= mpCharWidths[i];
        }
        else
        {
            int nTmpIndex = nEndGlyphPos - 1;
            while( (--i >= rVisualItem.mnMinCharPos) && (nTmpIndex == mpLogClusters[i]) )
                aRelPos.X() += mpCharWidths[i];
        }

        // now draw the matching glyphs in this item
        Point aPos = GetDrawPosition( aRelPos );
        HRESULT nRC = (*pScriptTextOut)( mhDC, &maScriptCache,
            aPos.X(), aPos.Y(), 0, NULL,
            &rVisualItem.mpScriptItem->a, NULL, 0,
            mpOutGlyphs + nMinGlyphPos,
            nEndGlyphPos - nMinGlyphPos,
            mpGlyphAdvances + nMinGlyphPos,
            mpJustifications ? mpJustifications + nMinGlyphPos : NULL,
            mpGlyphOffsets + nMinGlyphPos );

        for( i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            aRelPos.X() += pGlyphWidths[ i ];
    }
#else
    #define MAXGLYPHCOUNT 4
    long pLGlyphs[ MAXGLYPHCOUNT ];
    long pWidths[ MAXGLYPHCOUNT ];
    int  pCharPosAry[ MAXGLYPHCOUNT ];
    Point aPos;
    for( int nStart = 0;;)
    {
        int nGlyphs = GetNextGlyphs( MAXGLYPHCOUNT, pLGlyphs, aPos, nStart, pWidths, pCharPosAry );
        if( !nGlyphs )
            break;

        WORD pWGlyphs[ MAXGLYPHCOUNT ];
        GOFFSET pGlyphOffsets[ MAXGLYPHCOUNT ];
        int pGlyphAdvances[ MAXGLYPHCOUNT ];
        for( int i = 0; i < nGlyphs; ++i )
        {
            pWGlyphs[ i ] = pLGlyphs[ i ];
            pGlyphOffsets[i].du = 0;
            pGlyphOffsets[i].dv = 0;
            pGlyphAdvances[i] = pWidths[i];
        }

        SCRIPT_ANALYSIS aSADummy = {SCRIPT_UNDEFINED,0,0,0,0,0,0};
        HRESULT nRC = (*pScriptTextOut)( mhDC, &maScriptCache,
            aPos.X(), aPos.Y(), 0, NULL,
            &aSADummy, NULL, 0,
            pWGlyphs, nGlyphs,
            pGlyphAdvances, NULL,
            pGlyphOffsets );
    }
#endif
}

// -----------------------------------------------------------------------

long UniscribeLayout::FillDXArray( long* pDXArray ) const
{
    long nWidth = 0;

    for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
    {
        nWidth += mpCharWidths[ i ];
        if( pDXArray )
            pDXArray[ i - mnMinCharPos ] = nWidth;
    }

    return nWidth;
}

// -----------------------------------------------------------------------

int UniscribeLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    long nWidth = 0;
    for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
    {
        nWidth += mpCharWidths[ i ] * nFactor;
        if( nWidth >= nMaxWidth )
        {
            // go back to cluster start
            while( !mpVisualAttrs[ mpLogClusters[i] ].fClusterStart
                && (--i > mnMinCharPos) );
            return i;
        }
        nWidth += nCharExtra;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

void UniscribeLayout::GetCaretPositions( long* pCaretXArray ) const
{
    const int nMaxIdx = 2 * (mnEndCharPos - mnMinCharPos);
    int i;
    for( i = 0; i < nMaxIdx; ++i )
        pCaretXArray[ i ] = -1;
    long* const pGlyphPos = (long*)alloca( (mnGlyphCount+1) * sizeof(long) );
    for( i = 0; i <= mnGlyphCount; ++i )
        pGlyphPos[ i ] = -1;

    long nXPos = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.mnEndGlyphPos <= 0 )
            continue;

        // get glyph positions
        // TODO: rVisualItem's glyph range only partially used
        for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
        {
            pGlyphPos[ i ] = nXPos;
            nXPos += mpGlyphAdvances[ i ];
        }
        // rightmost position of this visualitem
        pGlyphPos[ i ] = nXPos;

        // convert glyph positions to character positions
        i = rVisualItem.mnMinCharPos;
        if( i < mnMinCharPos )
            i = mnMinCharPos;
        for(; (i < rVisualItem.mnEndCharPos) && (i < mnEndCharPos); ++i )
        {
            int j = mpLogClusters[ i ] + rVisualItem.mnMinGlyphPos;
            int nCurrIdx = i * 2;
            if( !rVisualItem.mpScriptItem->a.fRTL )
            {
                // normal positions for LTR case
                pCaretXArray[ nCurrIdx ]   = pGlyphPos[ j ];
                pCaretXArray[ nCurrIdx+1 ] = pGlyphPos[ j+1 ];
            }
            else
            {
                // reverse positions for RTL case
                pCaretXArray[ nCurrIdx ]   = pGlyphPos[ j+1 ];
                pCaretXArray[ nCurrIdx+1 ] = pGlyphPos[ j ];
            }
        }
    }

    // fixup unknown character positions to neighbor
    for( i = 0; i < nMaxIdx; ++i )
    {
        if( pCaretXArray[ i ] >= 0 )
            nXPos = pCaretXArray[ i ];
        else
            pCaretXArray[ i ] = nXPos;
    }
}

// -----------------------------------------------------------------------

Point UniscribeLayout::GetCharPosition( int nCharPos, bool bRTL ) const
{
    int nStartIndex = mnGlyphCapacity;  // mark as untouched
    int nGlyphIndex = 0;
    int nEndIndex = -1;                 // mark as untouched

    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.mnEndGlyphPos <= 0 )
            continue;

        if( (rVisualItem.mnMinCharPos <= nCharPos) && (nCharPos < rVisualItem.mnEndCharPos) )
            nGlyphIndex = mpLogClusters[ nCharPos ] + rVisualItem.mnMinGlyphPos;

        // find the corresponding glyphs
        int nMinIndex, nMaxIndex;
        if( GetItemSubrange( rVisualItem, nMinIndex, nMaxIndex ) )
        {
            if( nStartIndex > nMinIndex )
                nStartIndex = nMinIndex;

            if( nEndIndex < nMaxIndex )
                nEndIndex = nMaxIndex;
        }
    }

    // set StartIndex and EndIndex to defaults when not touched
    if( nStartIndex >= mnGlyphCapacity )
        nStartIndex = 0;
    if( nEndIndex < 0 )
        nEndIndex = mnGlyphCount;

    long nXPos = 0;

    if( !bRTL ) // relative to left edge
    {
        for( int i = nStartIndex; i < nGlyphIndex; ++i )
            nXPos += mpGlyphAdvances[ i ];
    }
    else        // relative to right edge?
    {
        if( nGlyphIndex < nStartIndex )
            nGlyphIndex = nStartIndex;
        for( int i = nGlyphIndex; i < nEndIndex; ++i )
            nXPos -= mpGlyphAdvances[ i ];
    }

    return Point( nXPos, 0 );
}

// -----------------------------------------------------------------------

void UniscribeLayout::ApplyDXArray( const long* pDXArray )
{
    // increase char widths in string range to desired values
    bool bModified = false;
    int nOldWidth = 0;
    for( int i = mnMinCharPos, j = 0; i < mnEndCharPos; ++i, ++j )
    {
        int nNewCharWidth = (pDXArray[j] - nOldWidth);
        // TODO: nNewCharWidth *= mnUnitsPerPixel;
        DBG_ASSERT( mnUnitsPerPixel==1, "UniscribeLayout.mnUnitsPerPixel != 1" );
        if( mpCharWidths[i] != nNewCharWidth )
        {
            mpCharWidths[i] = nNewCharWidth;
            bModified = true;
        }
        nOldWidth = pDXArray[j];
    }

    if( !bModified )
        return;

    // initialize justifications array
    mpJustifications = new int[ mnGlyphCapacity ];
    for( i = 0; i < mnGlyphCount; ++i )
        mpJustifications[ i ] = mpGlyphAdvances[ i ];

    // apply new widths to script items
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.mnEndGlyphPos <= 0 )
            continue;

        if( (rVisualItem.mnMinCharPos < mnEndCharPos)
         && (rVisualItem.mnEndCharPos > mnMinCharPos) )
        {
            if( rVisualItem.mpScriptItem->a.fRTL )
            {
                // HACK: make sure kashida justification is used when possible
                // TODO: make sure this works on all usp versions
                for( i = rVisualItem.mnMinGlyphPos; i < rVisualItem.mnEndGlyphPos; ++i )
                    if( (1U << mpVisualAttrs[i].uJustification) & 0x7F89 )
                        mpVisualAttrs[i].uJustification = SCRIPT_JUSTIFY_ARABIC_KASHIDA;
            }

            HRESULT nRC = (*pScriptApplyLogicalWidth)(
                mpCharWidths + rVisualItem.mnMinCharPos,
                rVisualItem.mnEndCharPos - rVisualItem.mnMinCharPos,
                rVisualItem.mnEndGlyphPos - rVisualItem.mnMinGlyphPos,
                mpLogClusters + rVisualItem.mnMinCharPos,
                mpVisualAttrs + rVisualItem.mnMinGlyphPos,
                mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
                &rVisualItem.mpScriptItem->a,
                &rVisualItem.maABCWidths,
                mpJustifications + rVisualItem.mnMinGlyphPos );

            if( nRC != 0 )
            {
                delete[] mpJustifications;
                mpJustifications = NULL;
                break;
            }

            const ABC& rABC = rVisualItem.maABCWidths;
            rVisualItem.mnPixelWidth = rABC.abcA + rABC.abcB + rABC.abcC;

            if( rVisualItem.mpScriptItem->a.fRTL )
            {
                // right align adjusted glyph positions for RTL item
                // exception: kashida aligned glyphs
                // TODO: make sure this works on all usp versions
                for( i = rVisualItem.mnMinGlyphPos+1; i < rVisualItem.mnEndGlyphPos; ++i )
                    if( mpVisualAttrs[i].uJustification != SCRIPT_JUSTIFY_ARABIC_KASHIDA )
                    {
                        mpJustifications[i-1] += mpJustifications[ i ] - mpGlyphAdvances[ i ];
                        mpJustifications[ i ] = mpGlyphAdvances[ i ];
                    }
            }
        }
    }
}

// -----------------------------------------------------------------------

void UniscribeLayout::Justify( long nNewWidth )
{
    long nOldWidth = 0;
    int i;
    for( i = mnMinCharPos; i < mnEndCharPos; ++i )
        nOldWidth += mpCharWidths[ i ];

    nNewWidth *= mnUnitsPerPixel;
    if( nNewWidth == nOldWidth )
        return;
    double fStretch = (double)nNewWidth / nOldWidth;

    // initialize justifications array
    mpJustifications = new int[ mnGlyphCapacity ];
    for( i = 0; i < mnGlyphCapacity; ++i )
        mpJustifications[ i ] = mpGlyphAdvances[ i ];

    // justify stretched script items
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.mnEndGlyphPos <= 0 )
            continue;

        if( (rVisualItem.mnMinCharPos < mnEndCharPos)
         && (rVisualItem.mnEndCharPos > mnMinCharPos) )
        {
            long nItemWidth = 0;
            for( i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; ++i )
                nItemWidth += mpCharWidths[ i ];

            SCRIPT_FONTPROPERTIES aFontProperties;
            int nMinKashida = 1;
            HRESULT nRC = (*pScriptGetFontProperties)( mhDC, &maScriptCache, &aFontProperties );
            if( !nRC )
                nMinKashida = aFontProperties.iKashidaWidth;

            nRC = (*pScriptJustify) (
                mpVisualAttrs + rVisualItem.mnMinGlyphPos,
                mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
                rVisualItem.mnEndGlyphPos - rVisualItem.mnMinGlyphPos,
                (int)((fStretch-1.0) * nItemWidth + 0.5),
                nMinKashida,
                mpJustifications + rVisualItem.mnMinGlyphPos );
        }
    }
}

// -----------------------------------------------------------------------

bool UniscribeLayout::ApplyFallback( SalLayout& rFallback )
{
    // TODO
    return false;
}

// -----------------------------------------------------------------------

void UniscribeLayout::UpdateGlyphPos( int nStart, int nXPos )
{
    // TODO
}

// -----------------------------------------------------------------------

void UniscribeLayout::RemoveNotdefs()
{
    // TODO
}

#endif // USE_UNISCRIBE

// =======================================================================

SalLayout* SalGraphics::LayoutText( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    WinLayout* pWinLayout = NULL;

    // TODO: use nFallbackLevel
    if( nFallbackLevel > 0 )
        return NULL;

#ifdef USE_UNISCRIBE
    if( !(rArgs.mnFlags & SAL_LAYOUT_COMPLEX_DISABLED)
    &&  bUspEnabled && (aUspModule || InitUSP()) )
    {
        // script complexity is determined in upper layers
        pWinLayout = new UniscribeLayout( maGraphicsData.mhDC, rArgs );
    }
    else
#endif // USE_UNISCRIBE
    {
#ifdef GCP_KERN_HACK
        if( (rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS) && maGraphicsData.mbFontKernInit )
            GetKernPairs( 0, NULL );
#endif // GCP_KERN_HACK

        pWinLayout = new SimpleWinLayout( maGraphicsData.mhDC, rArgs
#ifdef GCP_KERN_HACK
            , maGraphicsData.mpFontKernPairs, maGraphicsData.mnFontKernPairCount
#endif // GCP_KERN_HACK
            );
    }

    if( !pWinLayout->LayoutText( rArgs ) )
    {
        pWinLayout->Release();
        pWinLayout = NULL;
    }

    return pWinLayout;
}

// =======================================================================
