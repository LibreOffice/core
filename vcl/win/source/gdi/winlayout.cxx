/*************************************************************************
 *
 *  $RCSfile: winlayout.cxx,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: hdu $ $Date: 2002-08-26 16:16:47 $
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

#ifndef DISABLE_UNISCRIBE //TODO: use ENABLE_UNISCRIBE in pmk
#define USE_UNISCRIBE
#endif

// =======================================================================

class WinLayout : public SalLayout
{
public:
                    WinLayout( const ImplLayoutArgs& rArgs )
                        : SalLayout( rArgs ) {}

    virtual bool    LayoutText( const ImplLayoutArgs& ) = 0;
    virtual void    Draw() const = 0;
};

// =======================================================================

class SimpleWinLayout : public WinLayout
{
public:
                    SimpleWinLayout( HDC hDC, const ImplLayoutArgs&, bool bEnableGlyphs );
    virtual         ~SimpleWinLayout();

    virtual bool    LayoutText( const ImplLayoutArgs& );
    virtual void    Draw() const;
    virtual int     GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos, int&,
                        long* pGlyphAdvances, int* pCharIndexes ) const;

    virtual Point   GetCharPosition( int nCharIndex, bool bRTL ) const;
    virtual long    FillDXArray( long* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual bool    HasGlyphs() const { return mbEnableGlyphs; }

protected:
    void            Justify( long nNewWidth );
    void            ApplyDXArray( const long* pDXArray );

private:
    HDC             mhDC;
    bool            mbEnableGlyphs;

    int             mnGlyphCount;
    WCHAR*          mpOutGlyphs;
    int*            mpGlyphAdvances;
    int*            mpGlyphOrigAdvs;
    UINT*           mpChars2Glyphs;
    UINT*           mpGlyphs2Chars;
    int             mnNotdefWidth;
    long            mnWidth;
};

// -----------------------------------------------------------------------

SimpleWinLayout::SimpleWinLayout( HDC hDC, const ImplLayoutArgs& rArgs,
    bool bEnableGlyphs )
:   WinLayout( rArgs ),
    mhDC( hDC ),
    mbEnableGlyphs( bEnableGlyphs ),
    mnGlyphCount( 0 ),
    mpOutGlyphs( NULL ),
    mpGlyphAdvances( NULL ),
    mpGlyphOrigAdvs( NULL ),
    mpChars2Glyphs( NULL ),
    mpGlyphs2Chars( NULL ),
    mnNotdefWidth( -1 ),
    mnWidth( 0 )
{
    // Win32 glyph APIs have serious problems with vertical layout
    // => workaround is to use the unicode methods then
    mbEnableGlyphs &= !(rArgs.mnFlags & SAL_LAYOUT_VERTICAL);
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

bool SimpleWinLayout::LayoutText( const ImplLayoutArgs& rArgs )
{
    // layout text
    int nMaxGlyphCount = rArgs.mnEndCharIndex - rArgs.mnFirstCharIndex;
    DWORD nGcpOption = 0;

    mpOutGlyphs     = new WCHAR[ nMaxGlyphCount ];
    mpGlyphAdvances = new int[ nMaxGlyphCount ];
    mnGlyphCount    = 0;

    if( rArgs.mnFlags & (SAL_LAYOUT_KERNING_PAIRS | SAL_LAYOUT_KERNING_ASIAN) )
        mpGlyphOrigAdvs = new int[ nMaxGlyphCount ];

    GCP_RESULTSW aGCP;
    aGCP.lStructSize    = sizeof(aGCP);
    aGCP.lpOutString    = NULL;
    aGCP.lpOrder        = NULL;
    aGCP.lpDx           = mpGlyphAdvances;
    aGCP.lpCaretPos     = NULL;
    aGCP.lpClass        = NULL;
    aGCP.lpGlyphs       = NULL;
    aGCP.nGlyphs        = nMaxGlyphCount;
    aGCP.nMaxFit        = 0;

    if( mbEnableGlyphs )
        aGCP.lpGlyphs = mpOutGlyphs;
    else
        aGCP.lpOutString = mpOutGlyphs;

    // enable kerning if requested
    if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
        nGcpOption |= GCP_USEKERNING;

    // apply reordering if requested
    char pGcpClass[2];
    if( 0 == (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG) )
    {
        if( nMaxGlyphCount > 1 )
            aGCP.lpOrder = mpGlyphs2Chars = new UINT[ nMaxGlyphCount ];

        aGCP.lpClass = pGcpClass;
        nGcpOption  |= GCP_REORDER;
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
    if( aSalShlData.mbWNT )
    {
        if( mpGlyphOrigAdvs )
        {
            aGCP.lpDx = mpGlyphOrigAdvs;
            nRC = ::GetCharacterPlacementW( mhDC, rArgs.mpStr + rArgs.mnFirstCharIndex,
                        nMaxGlyphCount, 0, &aGCP, (nGcpOption & ~GCP_USEKERNING) );
           aGCP.lpDx = mpGlyphAdvances;
        }
        nRC = ::GetCharacterPlacementW( mhDC, rArgs.mpStr + rArgs.mnFirstCharIndex,
                    nMaxGlyphCount, 0, &aGCP, nGcpOption );
    }
    else
    {
        // convert into ANSI code page
        UINT nACP = GetACP();
        int nMBLen = ::WideCharToMultiByte( nACP,
            WC_COMPOSITECHECK | WC_DISCARDNS | WC_DEFAULTCHAR,
            rArgs.mpStr + rArgs.mnFirstCharIndex, nMaxGlyphCount,
            NULL, 0, NULL, NULL );
        if( (nMBLen <= 0) || (nMBLen >= 8 * nMaxGlyphCount) )
            return false;
        char* pMBStr = (char*)alloca( nMBLen+1 );
        ::WideCharToMultiByte( nACP,
            WC_COMPOSITECHECK | WC_DISCARDNS | WC_DEFAULTCHAR,
            rArgs.mpStr + rArgs.mnFirstCharIndex, nMaxGlyphCount,
            pMBStr, nMBLen, NULL, NULL );
        if( !mbEnableGlyphs )
            aGCP.nGlyphs *= 2;  // ascii length = 2 * unicode length
        // note: because aGCP.lpOutString==NULL GCP_RESULTSA is compatible with GCP_RESULTSW
        if( mpGlyphOrigAdvs )
        {
            aGCP.lpDx = mpGlyphOrigAdvs;
            nRC = ::GetCharacterPlacementA( mhDC, pMBStr, nMBLen,
                        0, (GCP_RESULTSA*)&aGCP, (nGcpOption & ~GCP_USEKERNING) );
           aGCP.lpDx = mpGlyphAdvances;
        }
        nRC = ::GetCharacterPlacementA( mhDC, pMBStr, nMBLen,
                    0, (GCP_RESULTSA*)&aGCP, nGcpOption );
    }

    // cache essential layout properties
    mnGlyphCount = mbEnableGlyphs ? aGCP.nGlyphs : aGCP.nMaxFit;
    mnWidth = nRC & 0xFFFF; // TODO: check API docs for clarification

    if( !nRC )
        return false;

    // #101097# fixup display of notdef glyphs
    // TODO: is there a way to convince Win32(incl W95) API to use notdef directly?
    int i;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        if( !mpGlyphAdvances[i] && (!aGCP.lpGlyphs || (mpOutGlyphs[i] == 3)) )
        {
            if( mnNotdefWidth < 0 )
            {
                SIZE aExtent;
                WCHAR cNotDef = 0xFFFF;
                mnNotdefWidth = GetTextExtentPoint32W(mhDC,&cNotDef,1,&aExtent) ? aExtent.cx : 0;
            }
            mpOutGlyphs[i] = 0;
            mpGlyphAdvances[i] = mnNotdefWidth;
            if( mpGlyphOrigAdvs )
                mpGlyphOrigAdvs[i] = mnNotdefWidth;
            mnWidth += mnNotdefWidth;
        }
    }

    // fixup strong RTL layout by reversing glyph order
    // TODO: mirror glyphs
    if( 0 == (~rArgs.mnFlags & (SAL_LAYOUT_BIDI_STRONG|SAL_LAYOUT_BIDI_RTL))
    && (1 < mnGlyphCount) )
    {
        mpGlyphs2Chars = new UINT[ nMaxGlyphCount ];
        i = 0;
        for( int j = mnGlyphCount; --j >= i; ++i )
        {
            WCHAR nTempGlyph = mpOutGlyphs[ i ];
            int nTempAdvance = mpGlyphAdvances[ i ];
            mpOutGlyphs[ i ]        = mpOutGlyphs[ j ];
            mpGlyphAdvances[ i ]    = mpGlyphAdvances[ j ];
            mpGlyphs2Chars[ i ]     = j + (nMaxGlyphCount - mnGlyphCount);
            mpOutGlyphs[ j ]        = nTempGlyph;
            mpGlyphAdvances[ j ]    = nTempAdvance;
            mpGlyphs2Chars[ j ]     = i + (nMaxGlyphCount - mnGlyphCount);
        }
        for( i = mnGlyphCount; i < nMaxGlyphCount; ++i )
            mpGlyphs2Chars[ i ] = -1;
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

    if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );

    if( (rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN)
    &&  !rArgs.mpDXArray && !rArgs.mnLayoutWidth )
    {
        bool bVertical = false;
        const xub_Unicode* pStr = rArgs.mpStr + rArgs.mnFirstCharIndex;
        // #99658# also do asian kerning one beyond substring
        int nLen = mnGlyphCount;
        if( rArgs.mnFirstCharIndex + nLen < rArgs.mnLength )
            ++nLen;
        for( i = 1; i < nLen; ++i )
        {
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
        *(pGlyphs++) = mpOutGlyphs[ nStart ];
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = mpGlyphAdvances[ nStart ];
        if( pCharIndexes )
        {
            int nCharIndex = mpGlyphs2Chars ? mpGlyphs2Chars[nStart] : nStart;
            if( nCharIndex >= 0 )
                nCharIndex += mnFirstCharIndex;
            *(pCharIndexes++) =  nCharIndex;
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

void SimpleWinLayout::Draw() const
{
    if( mnGlyphCount <= 0 )
        return;

    UINT mnDrawOptions = mbEnableGlyphs ? ETO_GLYPH_INDEX : 0;

#ifndef DEBUG_GETNEXTGLYPHS
    Point aPos = GetDrawPosition();

    if( mbEnableGlyphs || aSalShlData.mbWNT )
    {
        ::ExtTextOutW( mhDC, aPos.X(), aPos.Y(), mnDrawOptions, NULL,
            mpOutGlyphs, mnGlyphCount, mpGlyphAdvances );
    }
    else
    {
        ::ExtTextOutA( mhDC, aPos.X(), aPos.Y(), mnDrawOptions, NULL,
            (char*)mpOutGlyphs, mnGlyphCount, mpGlyphAdvances );
    }
#else // DEBUG_GETNEXTGLYPHS
    #define MAXGLYPHCOUNT 12
    long pLGlyphs[ MAXGLYPHCOUNT ];
    long pWidths[ MAXGLYPHCOUNT ];
    int  pCharIndex[ MAXGLYPHCOUNT ];
    Point aPos;
    for( int nStart = 0;;)
    {
        int nGlyphs = GetNextGlyphs( MAXGLYPHCOUNT, pLGlyphs, aPos, nStart, pWidths, pCharIndex );
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
            return (mnFirstCharIndex + i);
        nWidth += nCharExtra;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

Point SimpleWinLayout::GetCharPosition( int nCharIndex, bool bRTL ) const
{
    //TODO: implement reordering using mpChars2Glyphs[i]
    long nXPos = 0;

    if( mnGlyphCount <= 0 )
        return Point( nXPos, 0 );

    if( !bRTL ) // relative to left edge
    {
        int nCharLimit = mnEndCharIndex;
        if( nCharLimit > nCharIndex )
            nCharLimit = nCharIndex;
        nCharLimit -= mnFirstCharIndex;
        for( int i = 0; i < nCharLimit; ++i )
        {
            int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
            if( j >= 0 )
                nXPos += mpGlyphAdvances[j];
        }
    }
    else        // relative to right edge?
    {
        int nCharStart = nCharIndex - mnFirstCharIndex;
        if( nCharStart < 0 )
            nCharStart = 0;
        int nCharLimit = mnEndCharIndex - mnFirstCharIndex;
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
    long nOldWidth = 0;

    int i;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        nOldWidth += mpGlyphAdvances[i];
        int nDiff = nOldWidth - pDXArray[i];
        if( nDiff>+1 || nDiff<-1)
            break;
    }
    if( i >= mnGlyphCount )
        return;

    mnWidth = 0;
    for( i = 0; i < mnGlyphCount; ++i )
    {
        int j = !mpChars2Glyphs ? i : mpChars2Glyphs[i];
        if( j >= 0 )
            mpGlyphAdvances[j] = pDXArray[i] - mnWidth;
        mnWidth = pDXArray[i];
    }
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
};

class UniscribeLayout : public WinLayout
{
public:
                    UniscribeLayout( HDC hDC, const ImplLayoutArgs& );
    virtual         ~UniscribeLayout();

    virtual bool    LayoutText( const ImplLayoutArgs& );
    virtual void    Draw() const;
    virtual int     GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos, int&,
                        long* pGlyphAdvances, int* pCharIndexes ) const;

    virtual Point   GetCharPosition( int nCharIndex, bool bRTL ) const;
    virtual long    FillDXArray( long* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;

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

bool UniscribeLayout::LayoutText( const ImplLayoutArgs& rArgs )
{
    // determine script items from string
    // TODO: try to avoid itemization since it costs a lot of performance
    SCRIPT_STATE aScriptState = {0,false,false,false,false,false,true,false,false,0,0};
    aScriptState.uBidiLevel = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL));
    aScriptState.fOverrideDirection = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG));
    for( int nItemCapacity = 8; /*FOREVER*/; nItemCapacity *= 2 )
    {
        mpScriptItems = new SCRIPT_ITEM[ nItemCapacity ];
        HRESULT nRC = (*pScriptItemize)( rArgs.mpStr, rArgs.mnLength,
            nItemCapacity, NULL, &aScriptState, mpScriptItems, &mnItemCount );
        if( !nRC )
            break;
        if( (nRC != E_OUTOFMEMORY) || (nItemCapacity > rArgs.mnLength) )
            return false;
        delete[] mpScriptItems;
        mpScriptItems = NULL;
    }

    // allocate arrays
    // TODO: when reusing class try to reuse old allocations
    mnCharCapacity  = rArgs.mnLength;
    mpLogClusters   = new WORD[ mnCharCapacity ];
    mpCharWidths    = new int[ mnCharCapacity ];

    mnGlyphCount    = 0;
    mnGlyphCapacity = 16 + 2 * rArgs.mnLength;  // worst case assumption
    mpGlyphAdvances = new int[ mnGlyphCapacity ];
    mpOutGlyphs     = new WORD[ mnGlyphCapacity ];
    mpGlyphOffsets  = new GOFFSET[ mnGlyphCapacity ];
    mpVisualAttrs   = new SCRIPT_VISATTR[ mnGlyphCapacity ];

    mpVisualItems   = new VisualItem[ mnItemCount ];

    int i;
    for( i = 0; i < mnItemCount; ++i )
    {
        VisualItem& r = mpVisualItems[ i ];

        // TODO: complex item ordering
        int j = i;
        if( (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG) && (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL) )
            j = mnItemCount - i - 1;

        r.mpScriptItem  = &mpScriptItems[ j ];
        r.mnMinGlyphPos = 0;
        r.mnEndGlyphPos = 0;
        r.mnMinCharPos  = r.mpScriptItem->iCharPos;
        r.mnEndCharPos  = (r.mpScriptItem+1)->iCharPos;
        r.mnPixelWidth  = 0;
    }

    // layout script items
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];
        rVisualItem.mnMinGlyphPos = mnGlyphCount;

        // shortcut for skipped items
        if( (rArgs.mnEndCharIndex <= rVisualItem.mnMinCharPos)
         || (rArgs.mnFirstCharIndex >= rVisualItem.mnEndCharPos) )
        {
            for( i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; ++i )
                mpLogClusters[ i ] = 0;
            continue;
        }

        if( 0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG) )
            rVisualItem.mpScriptItem->a.fRTL = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL) );

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

        // TODO: better fall back to matching font
        if( nRC == USP_E_SCRIPT_NOT_IN_FONT )
        {
            // fall back to default layout
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

        ABC aAbcInfo;
        nRC = (*pScriptPlace)( mhDC, &maScriptCache,
            mpOutGlyphs + rVisualItem.mnMinGlyphPos,
            nGlyphCount,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            &rVisualItem.mpScriptItem->a,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            mpGlyphOffsets + rVisualItem.mnMinGlyphPos,
            &aAbcInfo );

        if( nRC != 0 )
            continue;

        rVisualItem.mnPixelWidth = aAbcInfo.abcA + aAbcInfo.abcB + aAbcInfo.abcC;

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

    // justify if requested
    if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs.mpDXArray );

    return true;
}

// -----------------------------------------------------------------------

bool UniscribeLayout::GetItemSubrange( const VisualItem& rVisualItem,
    int& rMinGlyphPos, int& rEndGlyphPos ) const
{
    if( (rVisualItem.mnMinCharPos >= mnEndCharIndex) || (rVisualItem.mnEndCharPos <= mnFirstCharIndex) )
        return false;

    // default: subrange is complete range
    rMinGlyphPos = rVisualItem.mnMinGlyphPos;
    rEndGlyphPos = rVisualItem.mnEndGlyphPos;

    // find glyph subrange if the script item is not used in tuto
    if( (rVisualItem.mnMinCharPos < mnFirstCharIndex) || (mnEndCharIndex < rVisualItem.mnEndCharPos) )
    {
        // get glyph range from char range by looking at cluster boundries
        rMinGlyphPos = rVisualItem.mnEndGlyphPos;
        int nMaxGlyphPos = 0;

        // TODO: optimize for case that LTR/RTL correspond to monotonous glyph indexes
        int i = mnFirstCharIndex;
        if( i < rVisualItem.mnMinCharPos )
            i = rVisualItem.mnMinCharPos;
        int nCharIndexLimit = rVisualItem.mnEndCharPos;
        if( nCharIndexLimit > mnEndCharIndex )
            nCharIndexLimit = mnEndCharIndex;
        for(; i < nCharIndexLimit; ++i )
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
                if( (n < rEndGlyphPos) && (n > nMaxGlyphPos) )
                    rEndGlyphPos = n;
            }
        }
    }

    return true;
}

// -----------------------------------------------------------------------

int UniscribeLayout::GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos,
    int& nStart, long* pGlyphAdvances, int* pCharIndexes ) const
{
    // return zero if no more glyph found
    if( nStart >= mnGlyphCount )
        return 0;

    // calculate glyph position relative to layout base
    // TODO: avoid for nStart!=0 case by reusing rPos
    long nXOffset = 0;
    const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;

    int nMinGlyphPos = 0;
    int nEndGlyphPos = mnGlyphCount;
    int nNextStart = mnGlyphCount;

    // update position in items with visible glyphs
    for( int nItem = 0, i; nItem < mnItemCount ; ++nItem )
    {
        // skip invisible items
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        nNextStart = rVisualItem.mnEndGlyphPos;
        if( !GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        // adjust current glyph pos to first visible glyph
        if( nStart <= rVisualItem.mnMinGlyphPos )
            nStart = nMinGlyphPos;

        // advance to next visual position by using adjusted glyph widths
        int nGLimit = (nStart >= rVisualItem.mnMinGlyphPos) ? nStart : nEndGlyphPos;
        // TODO: shortcut addition with cached ABC width if possible
        for( i = nMinGlyphPos; i < nGLimit; ++i )
            nXOffset += pGlyphWidths[ i ];

        // break in item with current start glyph
        if( nStart >= rVisualItem.mnMinGlyphPos )
        {
            // adjust glyph position relative to cluster start
            i = mnFirstCharIndex;
            if( !rVisualItem.mpScriptItem->a.fRTL )
            {
                int nTmpIndex = mpLogClusters[ i ];
                while( (--i >= rVisualItem.mnMinCharPos) && (nTmpIndex == mpLogClusters[i]) )
                    nXOffset -= mpCharWidths[i];
            }
            else
            {
                int nTmpIndex = mpLogClusters[ rVisualItem.mnEndCharPos - 1 ];
                while( (--i >= rVisualItem.mnMinCharPos) && (nTmpIndex == mpLogClusters[i]) )
                    nXOffset += mpCharWidths[i];
            }

            break;
        }
    }

    // calculate absolute position in pixel units
    const GOFFSET aGOffset = mpGlyphOffsets[ nStart ];
    Point aRelativePos( nXOffset + aGOffset.du, aGOffset.dv );
    rPos = GetDrawPosition( aRelativePos );

    if( pCharIndexes && !mpGlyphs2Chars )
        {} //TODO: implement, also see usage below

    int nCount = 0;
    while( nCount < nLen )
    {
        // update returned values
        *(pGlyphs++) = mpOutGlyphs[ nStart ];
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = pGlyphWidths[ nStart ];
        if( pCharIndexes )
            *(pCharIndexes++) = -1; // TODO: use mpGlyphs2Chars[nGI];

        // stop at item boundary
        ++nCount;
        if( ++nStart >= nEndGlyphPos )
        {
            nStart = nNextStart;
            break;
        }

        // stop when next x-position is unexpected
        if( !pGlyphAdvances  )
            if( (mpGlyphOffsets && (mpGlyphOffsets[nStart].du != aGOffset.du) )
             || (mpJustifications && (mpJustifications[nStart] != mpGlyphAdvances[nStart]) ) )
                break;

        // stop when next y-position is unexpected
        if( mpGlyphOffsets && (mpGlyphOffsets[nStart].dv != aGOffset.dv) )
            break;
    }

    return nCount;
}

// -----------------------------------------------------------------------

void UniscribeLayout::Draw() const
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
        int i = mnFirstCharIndex;
        if( !rVisualItem.mpScriptItem->a.fRTL )
        {
            int nTmpIndex = nMinGlyphPos;
            while( (--i >= rVisualItem.mnMinCharPos) && (nTmpIndex == mpLogClusters[i]) )
                aRelPos.X() -= mpCharWidths[i];
        }
        else
        {
            int nTmpIndex = mpLogClusters[ rVisualItem.mnEndCharPos - 1 ];
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
    int  pCharIndex[ MAXGLYPHCOUNT ];
    Point aPos;
    for( int nStart = 0;;)
    {
        int nGlyphs = GetNextGlyphs( MAXGLYPHCOUNT, pLGlyphs, aPos, nStart, pWidths, pCharIndex );
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

    for( int i = mnFirstCharIndex; i < mnEndCharIndex; ++i )
    {
        nWidth += mpCharWidths[ i ];
        if( pDXArray )
            pDXArray[ i - mnFirstCharIndex ] = nWidth;
    }

    return nWidth;
}

// -----------------------------------------------------------------------

int UniscribeLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
{
    long nWidth = 0;
    for( int i = mnFirstCharIndex; i < mnEndCharIndex; ++i )
    {
        nWidth += mpCharWidths[ i ] * nFactor;
        if( nWidth >= nMaxWidth )
        {
            // go back to cluster start
            while( !mpVisualAttrs[ mpLogClusters[i] ].fClusterStart
                && (--i > mnFirstCharIndex) );
            return i;
        }
        nWidth += nCharExtra;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

Point UniscribeLayout::GetCharPosition( int nCharIndex, bool bRTL ) const
{
    int nStartIndex = mnGlyphCapacity;  // mark as untouched
    int nGlyphIndex = 0;
    int nEndIndex = -1;                 // mark as untouched

    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.mnEndGlyphPos <= 0 )
            continue;

        if( (rVisualItem.mnMinCharPos <= nCharIndex) && (nCharIndex < rVisualItem.mnEndCharPos) )
            nGlyphIndex = mpLogClusters[ nCharIndex ] + rVisualItem.mnMinGlyphPos;

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
    for( int i = mnFirstCharIndex, j = 0; i < mnEndCharIndex; ++i, ++j )
    {
        int nNewCharWidth = pDXArray[j] - nOldWidth;
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
    for( i = 0; i < mnGlyphCapacity; ++i )
        mpJustifications[ i ] = mpGlyphAdvances[ i ];

    // apply new widths to script items
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.mnEndGlyphPos <= 0 )
            continue;

        if( (rVisualItem.mnMinCharPos < mnEndCharIndex)
         && (rVisualItem.mnEndCharPos > mnFirstCharIndex) )
        {
            HRESULT nRC = (*pScriptApplyLogicalWidth)(
                mpCharWidths + rVisualItem.mnMinCharPos,
                rVisualItem.mnEndCharPos - rVisualItem.mnMinCharPos,
                rVisualItem.mnEndGlyphPos - rVisualItem.mnMinGlyphPos,
                mpLogClusters + rVisualItem.mnMinCharPos,
                mpVisualAttrs + rVisualItem.mnMinGlyphPos,
                mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
                &rVisualItem.mpScriptItem->a,
                NULL,
                mpJustifications + rVisualItem.mnMinGlyphPos );
        }
    }
}

// -----------------------------------------------------------------------

void UniscribeLayout::Justify( long nNewWidth )
{
    long nOldWidth = 0;
    int i;
    for( i = mnFirstCharIndex; i < mnEndCharIndex; ++i )
        nOldWidth += mpCharWidths[ i ];

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

        if( (rVisualItem.mnMinCharPos < mnEndCharIndex)
         && (rVisualItem.mnEndCharPos > mnFirstCharIndex) )
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

#endif // USE_UNISCRIBE

// =======================================================================

SalLayout* SalGraphics::LayoutText( const ImplLayoutArgs& rArgs )
{
    WinLayout* pWinLayout = NULL;

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
        // #99019# don't use glyph indices for non-TT fonts
        // also for printer, because the drivers often transparently replace TTs with PS fonts
        // TODO: use a cached value from upper layers
        DWORD nFLI = GetFontLanguageInfo( maGraphicsData.mhDC );
        bool bEnableGlyphs = ((nFLI & GCP_GLYPHSHAPE) != 0);
        if( !bEnableGlyphs )
        {
            TEXTMETRICA aTextMetricA;
            if( ::GetTextMetricsA( maGraphicsData.mhDC, &aTextMetricA )
            &&  (aTextMetricA.tmPitchAndFamily & TMPF_TRUETYPE )
            && !(aTextMetricA.tmPitchAndFamily & TMPF_DEVICE ) )
                bEnableGlyphs = true;
        }
        pWinLayout = new SimpleWinLayout( maGraphicsData.mhDC, rArgs, bEnableGlyphs );
    }

    pWinLayout->LayoutText( rArgs );

    return pWinLayout;
}

// -----------------------------------------------------------------------

void SalGraphics::DrawSalLayout( const SalLayout& rSalLayout )
{
    // we know the SalLayout created by this SalGraphics is a WinLayout
    const WinLayout& rWinLayout = reinterpret_cast<const WinLayout&>( rSalLayout );
    rWinLayout.Draw();
}

// =======================================================================
