/*************************************************************************
 *
 *  $RCSfile: winlayout.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hdu $ $Date: 2002-05-28 18:17:41 $
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

#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif // _SV_POLY_HXX

#include <malloc.h>
#define alloca _alloca

#ifndef DISABLE_UNISCRIBE //TODO
#define USE_UNISCRIBE /*###*/
#endif

// =======================================================================

class WinLayout : public SalLayout
{
public:
                    WinLayout( const ImplLayoutArgs& rArgs )
                        : SalLayout( rArgs ) {}

    virtual bool    LayoutText( const ImplLayoutArgs& ) = 0;
    virtual void    Draw() const = 0;
    virtual bool    GetOutline( SalGraphics&, PolyPolygon& ) const = 0;
};

// =======================================================================

class SimpleWinLayout : public WinLayout
{
public:
                    SimpleWinLayout( HDC hDC, const ImplLayoutArgs& );
    virtual         ~SimpleWinLayout();

    virtual bool    LayoutText( const ImplLayoutArgs& );
    virtual void    Draw() const;
    virtual bool    GetOutline( SalGraphics&, PolyPolygon& ) const;

    virtual Point   GetCharPosition( int nCharIndex, bool bRTL ) const;
    virtual long    FillDXArray( long* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth ) const;

protected:
    void            Justify( long nNewWidth );
    void            ApplyDXArray( const long* pDXArray );

private:
    HDC             mhDC;

    int             mnGlyphCount;
    WCHAR*          mpOutGlyphs;
    int*            mpGlyphAdvances;
    long            mnWidth;
};

// -----------------------------------------------------------------------

SimpleWinLayout::SimpleWinLayout( HDC hDC, const ImplLayoutArgs& rArgs )
:   WinLayout( rArgs ),
    mhDC( hDC ),
    mnGlyphCount( 0 ),
    mpOutGlyphs( NULL ),
    mpGlyphAdvances( NULL ),
    mnWidth( 0 )
{}

// -----------------------------------------------------------------------

SimpleWinLayout::~SimpleWinLayout()
{
    delete[] mpOutGlyphs;
    delete[] mpGlyphAdvances;
}

// -----------------------------------------------------------------------

bool SimpleWinLayout::LayoutText( const ImplLayoutArgs& rArgs )
{
    // layout text
    mnGlyphCount = rArgs.mnEndCharIndex - rArgs.mnFirstCharIndex;
    mpOutGlyphs     = new WCHAR[ mnGlyphCount ];
    mpGlyphAdvances = new int[ mnGlyphCount ];

    GCP_RESULTSW aGCP;
    aGCP.lStructSize    = sizeof(aGCP);
    aGCP.lpOutString    = NULL;
    aGCP.lpOrder        = NULL;
    aGCP.lpDx           = mpGlyphAdvances;
    aGCP.lpCaretPos     = NULL;
    aGCP.lpClass        = NULL;
    aGCP.lpGlyphs       = mpOutGlyphs;
    aGCP.nGlyphs        = mnGlyphCount;
    aGCP.nMaxFit        = 0;

    DWORD nGcpOption = GCP_DISPLAYZWG;
    // enable kerning if requested
    if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
        nGcpOption |= GCP_USEKERNING;
    // justify if requested
    if( rArgs.mnLayoutWidth )
        nGcpOption |= GCP_JUSTIFY | GCP_MAXEXTENT;
    // apply reordering if requested
    if( 0 == (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG) )
        nGcpOption |= GCP_REORDER;
    else if( 0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL) )
        ; //TODO

    DWORD nRC;
    if( aSalShlData.mbWNT )
    {
        nRC = ::GetCharacterPlacementW( mhDC, rArgs.mpStr + rArgs.mnFirstCharIndex,
                    mnGlyphCount, rArgs.mnLayoutWidth, &aGCP, nGcpOption );
        // try again if it didn't fit
        if( aGCP.nMaxFit < mnGlyphCount )
        {
            nGcpOption &= ~(GCP_JUSTIFY |GCP_MAXEXTENT);
            nRC = ::GetCharacterPlacementW( mhDC, rArgs.mpStr + rArgs.mnFirstCharIndex,
                    mnGlyphCount, 0, &aGCP, nGcpOption );
        }
    }
    else
    {
        // convert into ANSI code page
        int nMBLen = WideCharToMultiByte( CP_ACP, 0, rArgs.mpStr + rArgs.mnFirstCharIndex,
            mnGlyphCount, NULL, 0, NULL, NULL );
        if( (nMBLen <= 0) || (nMBLen >= 8 * mnGlyphCount) )
            return false;
        char* pMBStr = (char*)alloca( nMBLen );
        WideCharToMultiByte( CP_ACP, 0, rArgs.mpStr + rArgs.mnFirstCharIndex,
            mnGlyphCount, pMBStr, nMBLen, NULL, NULL );
        // note: because aGCP.lpOutString==NULL GCP_RESULTSA is compatible with GCP_RESULTSW
        nRC = ::GetCharacterPlacementA( mhDC, pMBStr, nMBLen,
                    rArgs.mnLayoutWidth, (GCP_RESULTSA*)&aGCP, nGcpOption );
        // try again if it didn't fit
        if( aGCP.nMaxFit < mnGlyphCount )
        {
            nGcpOption &= ~(GCP_JUSTIFY |GCP_MAXEXTENT);
            nRC = ::GetCharacterPlacementA( mhDC, pMBStr, nMBLen,
                    0, (GCP_RESULTSA*)&aGCP, nGcpOption );
        }
    }

    // cache essential layout properties
    mnWidth = nRC & 0xFFFF; // TODO: check API docs for clarification

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs.mpDXArray );

    if( rArgs.mnLayoutWidth && (rArgs.mnLayoutWidth < mnWidth) )
        Justify( rArgs.mnLayoutWidth );

    if( (rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN)
    &&  !rArgs.mpDXArray && !rArgs.mnLayoutWidth )
    {
        bool bVertical = false;
        const xub_Unicode* pStr = rArgs.mpStr + rArgs.mnFirstCharIndex;
        for( int i = 1; i < mnGlyphCount; ++i )
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

    return (nRC != 0);
}

// -----------------------------------------------------------------------

void SimpleWinLayout::Draw() const
{
    if( mnGlyphCount <= 0 )
        return;

    // #99019# don't use glyph indices for non-TT fonts
    // TODO: use a cached value
    UINT nOptions = 0;
    TEXTMETRICW aTextMetricW;
    GetTextMetricsW( mhDC, &aTextMetricW );
    if( aTextMetricW.tmPitchAndFamily & TMPF_TRUETYPE )
        nOptions = ETO_GLYPH_INDEX;

    const Point aPos = GetDrawPosition();
    ::ExtTextOutW( mhDC, aPos.X(), aPos.Y(), nOptions, NULL,
        mpOutGlyphs, mnGlyphCount, mpGlyphAdvances );
}

// -----------------------------------------------------------------------

bool SimpleWinLayout::GetOutline( SalGraphics& rSalGraphics, PolyPolygon& rPolyPoly ) const
{
    long nWidth = 0;
    bool bRet = false;
    for( int i = 0; i < mnGlyphCount; ++i )
    {
        // get outline of individual glyph
        PolyPolygon aGlyphOutline;
        if( rSalGraphics.GetGlyphOutline( mpOutGlyphs[i], aGlyphOutline ) )
            bRet = true;

        // insert outline at correct position
        aGlyphOutline.Move( nWidth, 0 );
        nWidth += mpGlyphAdvances[i];
        for( int j = 0; j < aGlyphOutline.Count(); ++j )
            rPolyPoly.Insert( aGlyphOutline[j] );
    }

    return bRet;
}

// -----------------------------------------------------------------------

long SimpleWinLayout::FillDXArray( long* pDXArray ) const
{
    if( mnWidth && !pDXArray )
        return mnWidth;

    long nWidth = 0;

    for( int i = 0; i < mnGlyphCount; ++i )
    {
        nWidth += mpGlyphAdvances[i];
        if( pDXArray )
            pDXArray[i] = nWidth;
    }

    return nWidth;
}

// -----------------------------------------------------------------------

int SimpleWinLayout::GetTextBreak( long nMaxWidth ) const
{
    long nWidth = 0;

    for( int i = 0; i < mnGlyphCount; ++i )
    {
        nWidth += mpGlyphAdvances[i];
        if( nWidth >= nMaxWidth )
            return (mnFirstCharIndex + i);
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

Point SimpleWinLayout::GetCharPosition( int nCharIndex, bool bRTL ) const
{
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
            nXPos += mpGlyphAdvances[ i ];
    }
    else        // relative to right edge?
    {
        // assuming SimpleWinLayout only layed out LTR strings
        int nCharStart = nCharIndex - mnFirstCharIndex;
        if( nCharStart < 0 )
            nCharStart = 0;
        int nCharLimit = mnEndCharIndex - mnFirstCharIndex;
        for( int i = nCharStart; i < nCharLimit; ++i )
            nXPos -= mpGlyphAdvances[ i ];
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

    int i = mnGlyphCount-1;
    // the last glyph cannot be stretched
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
        mpGlyphAdvances[i] = pDXArray[i] - mnWidth;
        mnWidth = pDXArray[i];
    }
}

// =======================================================================

#ifdef USE_UNISCRIBE
#include <Usp10.h>

class UniscribeLayout : public WinLayout
{
public:
                    UniscribeLayout( HDC hDC, const ImplLayoutArgs& );
    virtual         ~UniscribeLayout();

    virtual bool    LayoutText( const ImplLayoutArgs& );
    virtual void    Draw() const;
    virtual bool    GetOutline( SalGraphics&, PolyPolygon& ) const;

    virtual Point   GetCharPosition( int nCharIndex, bool bRTL ) const;
    virtual long    FillDXArray( long* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth ) const;

protected:
    void            Justify( long nNewWidth );
    void            ApplyDXArray( const long* pDXArray );

    bool            GetItemSubrange( int nItem, int& nMinIndex, int& nEndIndex ) const;

private:
    HDC             mhDC;
    mutable SCRIPT_CACHE maScriptCache;

    // string specific info
    int             mnCharCapacity;
    WORD*           mpLogClusters;
    int*            mpCharWidths;

    // item specific info
    int             mnItemCount;
    SCRIPT_ITEM*    mpScriptItems;
    int*            mpGlyphCounts;

    // glyph specific info
    int             mnGlyphCapacity;
    int*            mpGlyphAdvances;
    int*            mpJustifications;
    WORD*           mpOutGlyphs;
    GOFFSET*        mpGlyphOffsets;
    SCRIPT_VISATTR* mpVisualAttrs;
};


// -----------------------------------------------------------------------
// dynamic loading of usp library

static bool bUspDisabled = false;
static HMODULE aUspModule;

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
        return !(bUspDisabled = true);

    pScriptIsComplex = (HRESULT (WINAPI*)(const WCHAR*,int,DWORD))
        ::GetProcAddress( aUspModule, "ScriptIsComplex" );
    bUspDisabled |= (pScriptIsComplex == NULL);

    pScriptItemize = (HRESULT (WINAPI*)(const WCHAR*,int,int,
        const SCRIPT_CONTROL*,const SCRIPT_STATE*,SCRIPT_ITEM*,int*))
        ::GetProcAddress( aUspModule, "ScriptItemize" );
    bUspDisabled |= (pScriptItemize == NULL);

    pScriptShape = (HRESULT (WINAPI*)(HDC,SCRIPT_CACHE*,const WCHAR*,
        int,int,SCRIPT_ANALYSIS*,WORD*,WORD*,SCRIPT_VISATTR*,int*))
        ::GetProcAddress( aUspModule, "ScriptShape" );
    bUspDisabled |= (pScriptShape == NULL);

    pScriptPlace = (HRESULT (WINAPI*)(HDC, SCRIPT_CACHE*, const WORD*, int,
        const SCRIPT_VISATTR*,SCRIPT_ANALYSIS*,int*,GOFFSET*,ABC*))
        ::GetProcAddress( aUspModule, "ScriptPlace" );
    bUspDisabled |= (pScriptPlace == NULL);

    pScriptGetLogicalWidths = (HRESULT (WINAPI*)(const SCRIPT_ANALYSIS*,
        int,int,const int*,const WORD*,const SCRIPT_VISATTR*,int*))
        ::GetProcAddress( aUspModule, "ScriptGetLogicalWidths" );
    bUspDisabled |= (pScriptGetLogicalWidths == NULL);

    pScriptApplyLogicalWidth = (HRESULT (WINAPI*)(const int*,int,int,const WORD*,
        const SCRIPT_VISATTR*,const int*,const SCRIPT_ANALYSIS*,ABC*,int*))
        ::GetProcAddress( aUspModule, "ScriptApplyLogicalWidth" );
    bUspDisabled |= (pScriptApplyLogicalWidth == NULL);

    pScriptJustify = (HRESULT (WINAPI*)(const SCRIPT_VISATTR*,const int*,
        int,int,int,int*))
        ::GetProcAddress( aUspModule, "ScriptJustify" );
    bUspDisabled |= (pScriptJustify == NULL);

    pScriptGetFontProperties = (HRESULT (WINAPI*)( HDC,SCRIPT_CACHE*,SCRIPT_FONTPROPERTIES*))
        ::GetProcAddress( aUspModule, "ScriptGetFontProperties" );
    bUspDisabled |= (pScriptGetFontProperties == NULL);

    pScriptTextOut = (HRESULT (WINAPI*)(const HDC,SCRIPT_CACHE*,
        int,int,UINT,const RECT*,const SCRIPT_ANALYSIS*,const WCHAR*,
        int,const WORD*,int,const int*,const int*,const GOFFSET*))
        ::GetProcAddress( aUspModule, "ScriptTextOut" );
    bUspDisabled |= (pScriptTextOut == NULL);

    pScriptFreeCache = (HRESULT (WINAPI*)(SCRIPT_CACHE*))
        ::GetProcAddress( aUspModule, "ScriptFreeCache" );
    bUspDisabled |= (pScriptFreeCache == NULL);

    return (bUspDisabled == false);
}

// -----------------------------------------------------------------------

UniscribeLayout::UniscribeLayout( HDC hDC, const ImplLayoutArgs& rArgs )
:   WinLayout( rArgs ),
    mhDC( hDC ),
    maScriptCache( NULL ),
    mnItemCount(0),
    mpScriptItems( NULL ),
    mpGlyphCounts( NULL ),
    mpLogClusters( NULL ),
    mpCharWidths( NULL ),
    mnCharCapacity( 0 ),
    mnGlyphCapacity(0),
    mpOutGlyphs( NULL ),
    mpGlyphAdvances( NULL ),
    mpJustifications( NULL ),
    mpGlyphOffsets( NULL ),
    mpVisualAttrs( NULL )
{}

// -----------------------------------------------------------------------

UniscribeLayout::~UniscribeLayout()
{
    (*pScriptFreeCache)( &maScriptCache );

    delete[] mpScriptItems;
    delete[] mpGlyphCounts;
    delete[] mpLogClusters;
    delete[] mpCharWidths;
    delete[] mpOutGlyphs;
    delete[] mpGlyphAdvances;
    delete[] mpJustifications;
    delete[] mpGlyphOffsets;
    delete[] mpVisualAttrs;
}

// -----------------------------------------------------------------------

bool UniscribeLayout::LayoutText( const ImplLayoutArgs& rArgs )
{
    // determine script items from string
    static SCRIPT_STATE aScriptState = {0,false,false,false,false,true,false,false,0,0};
    aScriptState.uBidiLevel = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL));
    aScriptState.fOverrideDirection = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG));
    for( int nItemCapacity = 8; /*FOREVER*/; nItemCapacity *= 2 )
    {
        mpScriptItems = new SCRIPT_ITEM[ nItemCapacity ];
        HRESULT nRC = (*pScriptItemize)( rArgs.mpStr, rArgs.mnLength,
            nItemCapacity, NULL, &aScriptState, mpScriptItems, &mnItemCount );
        if( !nRC )
            break;
        if( (nRC != E_OUTOFMEMORY) || (nItemCapacity >= rArgs.mnLength) )
            return false;
        delete[] mpScriptItems;
        mpScriptItems = NULL;
    }

    // allocate arrays
    // TODO: when reusing class try to reuse old allocations
    mpGlyphCounts = new int[ mnItemCount ];

    mnCharCapacity = rArgs.mnLength;
    mpLogClusters   = new WORD[ mnCharCapacity ];
    mpCharWidths    = new int[ mnCharCapacity ];

    mnGlyphCapacity = 16 + 2 * rArgs.mnLength;
    mpGlyphAdvances = new int[ mnGlyphCapacity ];
    mpOutGlyphs     = new WORD[ mnGlyphCapacity ];
    mpGlyphOffsets  = new GOFFSET[ mnGlyphCapacity ];
    mpVisualAttrs   = new SCRIPT_VISATTR[ mnGlyphCapacity ];

    // layout script items
    int nGlyphsProcessed = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        mpGlyphCounts[ nItem ] = 0;

        SCRIPT_ITEM& rScriptItem = mpScriptItems[ nItem ];
        int nCharIndexLimit = mpScriptItems[ nItem+1 ].iCharPos;

        // shortcut for skipped items
        if( (rArgs.mnEndCharIndex <= rScriptItem.iCharPos)
         || (rArgs.mnFirstCharIndex >= nCharIndexLimit) )
        {
            for( int i = rScriptItem.iCharPos; i < nCharIndexLimit; ++i )
                mpLogClusters[ i ] = 0;
            continue;
        }

        if( 0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG) )
            rScriptItem.a.fRTL = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL) );

        int nGlyphCount = 0;
        int nCharCount = nCharIndexLimit - rScriptItem.iCharPos;
        HRESULT nRC = (*pScriptShape)( mhDC, &maScriptCache,
            rArgs.mpStr + rScriptItem.iCharPos,
            nCharCount,
            mnGlyphCapacity - nGlyphsProcessed,
            &rScriptItem.a,
            mpOutGlyphs + nGlyphsProcessed,
            mpLogClusters + rScriptItem.iCharPos,
            mpVisualAttrs + nGlyphsProcessed,
            &nGlyphCount );

        // TODO: better fall back to matching font
        if( nRC == USP_E_SCRIPT_NOT_IN_FONT )
        {
            // fall back to default layout
            rScriptItem.a.eScript = SCRIPT_UNDEFINED;
            nRC = (*pScriptShape)( mhDC, &maScriptCache,
                rArgs.mpStr + rScriptItem.iCharPos,
                nCharCount,
                mnGlyphCapacity - nGlyphsProcessed,
                &rScriptItem.a,
                mpOutGlyphs + nGlyphsProcessed,
                mpLogClusters + rScriptItem.iCharPos,
                mpVisualAttrs + nGlyphsProcessed,
                &nGlyphCount );
        }

        if( nRC != 0 )
            continue;

        ABC aAbcInfo;
        nRC = (*pScriptPlace)( mhDC, &maScriptCache,
            mpOutGlyphs + nGlyphsProcessed, nGlyphCount,
            mpVisualAttrs + nGlyphsProcessed,
            &rScriptItem.a,
            mpGlyphAdvances + nGlyphsProcessed,
            mpGlyphOffsets + nGlyphsProcessed,
            &aAbcInfo );

        if( nRC != 0 )
            continue;

        nRC = (*pScriptGetLogicalWidths)(
            &rScriptItem.a, nCharCount, nGlyphCount,
            mpGlyphAdvances + nGlyphsProcessed,
            mpLogClusters + rScriptItem.iCharPos,
            mpVisualAttrs + nGlyphsProcessed,
            mpCharWidths + rScriptItem.iCharPos );

        mpGlyphCounts[ nItem ] = nGlyphCount;
        nGlyphsProcessed += nGlyphCount;
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

bool UniscribeLayout::GetItemSubrange( int nItem, int& nMinIndex, int& nEndIndex ) const
{
    SCRIPT_ITEM& rScriptItem = mpScriptItems[ nItem ];
    int nCharIndexLimit = mpScriptItems[ nItem+1 ].iCharPos;

    if( (mnFirstCharIndex >= nCharIndexLimit) || (mnEndCharIndex <= rScriptItem.iCharPos) )
        return false;

    // default: subrange is complete range
    nMinIndex = 0;
    nEndIndex = mpGlyphCounts[ nItem ];

    // find glyph subrange if the script item is not used in tuto
    if( (rScriptItem.iCharPos < mnFirstCharIndex) || (mnEndCharIndex < nCharIndexLimit) )
    {
        // get glyph range from char range by looking at cluster boundries
        nMinIndex = mnGlyphCapacity;
        int nMaxIndex = 0;

        // TODO: optimize for case that LTR/RTL correspond to monotonous glyph indexes
        int i = mnFirstCharIndex;
        if( i < rScriptItem.iCharPos )
            i = rScriptItem.iCharPos;
        if( nCharIndexLimit > mnEndCharIndex )
            nCharIndexLimit = mnEndCharIndex;
        for(; i < nCharIndexLimit; ++i )
        {
            int n = mpLogClusters[ i ];
            if( nMinIndex > n )
                nMinIndex = n;
            if( nMaxIndex < n )
                nMaxIndex = n;
        }

        // account for multiple glyphs at rightmost character
        // test only needed when rightmost glyph isn't referenced
        if( nEndIndex > nMaxIndex + 1 )
        {
            // glyph index above currently selected range is new end index
            // TODO: optimize for case that LTR/RTL correspond to monotonous glyph indexes
            int i2 = mpScriptItems[ nItem+1 ].iCharPos;
            for( int i = rScriptItem.iCharPos; i < i2; ++i )
            {
                int n = mpLogClusters[ i ];
                if( (n < nEndIndex) && (n > nMaxIndex) )
                    nEndIndex = n;
            }
        }
    }

    return true;
}

// -----------------------------------------------------------------------

void UniscribeLayout::Draw() const
{
    Point aRelPos = Point(0,0);

    int nGlyphsProcessed = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        // display if there is something to display
        int nStartIndex, nEndIndex;
        if( GetItemSubrange( nItem, nStartIndex, nEndIndex ) )
        {
            // adjust draw position relative to cluster start
            SCRIPT_ITEM& rScriptItem = mpScriptItems[ nItem ];
            int i = mnFirstCharIndex;
            if( !rScriptItem.a.fRTL )
            {
                while( (--i >= rScriptItem.iCharPos) && (nStartIndex == mpLogClusters[i]) )
                    aRelPos -= Point( mpCharWidths[i], 0 );
            }
            else
            {
                int nMaxIndex = mpLogClusters[ i ];
                while( (--i >= rScriptItem.iCharPos) && (nMaxIndex == mpLogClusters[i]) )
                    aRelPos += Point( mpCharWidths[i], 0 );
            }

            // now draw the matching glyphs in this item
            nStartIndex += nGlyphsProcessed;
            nEndIndex += nGlyphsProcessed;

            int* pJustifications = mpJustifications;
            if( pJustifications )
                pJustifications += nStartIndex;

            Point aPos = GetDrawPosition( aRelPos );
            HRESULT nRC = (*pScriptTextOut)( mhDC, &maScriptCache,
                aPos.X(), aPos.Y(), 0, NULL, &rScriptItem.a, NULL, 0,
                mpOutGlyphs + nStartIndex,
                nEndIndex - nStartIndex,
                mpGlyphAdvances + nStartIndex,
                pJustifications,
                mpGlyphOffsets + nStartIndex );

            if( !pJustifications )
                for( i = nStartIndex; i < nEndIndex; ++i )
                    aRelPos += Point( mpGlyphAdvances[ i ], 0 );
            else
                for( i = nStartIndex; i < nEndIndex; ++i )
                    aRelPos += Point( mpJustifications[ i ], 0 );
        }

        nGlyphsProcessed += mpGlyphCounts[ nItem ];
    }
}

// -----------------------------------------------------------------------

bool UniscribeLayout::GetOutline( SalGraphics& rSalGraphics, PolyPolygon& rPolyPoly ) const
{
    Point aRelPos = Point(0,0);

    int nGlyphsProcessed = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        // display if there is something to display
        int nStartIndex, nEndIndex;
        if( GetItemSubrange( nItem, nStartIndex, nEndIndex ) )
        {
            SCRIPT_ITEM& rScriptItem = mpScriptItems[ nItem ];

            // adjust draw position relative to cluster start
            int i = mnFirstCharIndex;
            if( !rScriptItem.a.fRTL )
            {
                while( (--i >= rScriptItem.iCharPos) && (nStartIndex == mpLogClusters[i]) )
                    aRelPos -= Point( mpCharWidths[i], 0 );
            }
            else
            {
                int nMaxIndex = mpLogClusters[ nEndIndex - 1 ];
                while( (--i >= rScriptItem.iCharPos) && (nMaxIndex == mpLogClusters[i]) )
                    aRelPos += Point( mpCharWidths[i], 0 );
            }

            // now we know the matching glyphs in this item
            nStartIndex += nGlyphsProcessed;
            nEndIndex += nGlyphsProcessed;

            for( i = nStartIndex; i < nEndIndex; ++i )
            {
                // get outline of individual glyph
                PolyPolygon aGlyphOutline;
                if( !rSalGraphics.GetGlyphOutline( mpOutGlyphs[i], aGlyphOutline ) )
                    return false;

                // insert outline at correct position
                aGlyphOutline.Move( aRelPos.X(), aRelPos.Y() );
                aRelPos += Point( mpGlyphAdvances[ i ], 0 );
                for( int j = 0; j < aGlyphOutline.Count(); ++j )
                    rPolyPoly.Insert( aGlyphOutline[j] );
            }
        }
    }

    return true;
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

int UniscribeLayout::GetTextBreak( long nMaxWidth ) const
{
    long nWidth = 0;
    int nClusterStart = mnFirstCharIndex;
    for( int i = mnFirstCharIndex; i < mnEndCharIndex; ++i )
    {
        nWidth += mpCharWidths[ i ];
        if( mpVisualAttrs[i].fClusterStart )
            nClusterStart = i;
        if( nWidth >= nMaxWidth )
            return nClusterStart;
    }

    return STRING_LEN;
}

// -----------------------------------------------------------------------

Point UniscribeLayout::GetCharPosition( int nCharIndex, bool bRTL ) const
{
    int nStartIndex = mnGlyphCapacity;  // mark as untouched
    int nGlyphIndex = 0;
    int nEndIndex = -1;                 // mark as untouched

    int nGlyphsProcessed = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        int nGlyphCount = mpGlyphCounts[ nItem ];
        if( !nGlyphCount )
            continue;

        SCRIPT_ITEM& rScriptItem = mpScriptItems[ nItem ];
        int nCharIndexLimit = mpScriptItems[ nItem+1 ].iCharPos;

        if( (rScriptItem.iCharPos <= nCharIndex) && (nCharIndex < nCharIndexLimit) )
            nGlyphIndex = mpLogClusters[ nCharIndex ] + nGlyphsProcessed;

        // display if there is something to display
        int nMinIndex, nMaxIndex;
        if( GetItemSubrange( nItem, nMinIndex, nMaxIndex ) )
        {
            nMinIndex += nGlyphsProcessed;
            if( nStartIndex > nMinIndex )
                nStartIndex = nMinIndex;

            nMaxIndex += nGlyphsProcessed;
            if( nEndIndex < nMaxIndex )
                nEndIndex = nMaxIndex;
        }

        nGlyphsProcessed += nGlyphCount;
    }

    // set StartIndex and EndIndex to defaults when not touched
    if( nStartIndex >= mnGlyphCapacity )
        nStartIndex = 0;
    if( nEndIndex < 0 )
        nEndIndex = nGlyphsProcessed;

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
    int nGlyphsProcessed = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        int nGlyphCount = mpGlyphCounts[ nItem ];
        if( !nGlyphCount )
            continue;

        SCRIPT_ITEM& rScriptItem = mpScriptItems[ nItem ];
        int nCharIndexLimit = mpScriptItems[ nItem+1 ].iCharPos;

        if( (mnEndCharIndex > rScriptItem.iCharPos) && (mnFirstCharIndex < nCharIndexLimit) )
        {
            HRESULT nRC = (*pScriptApplyLogicalWidth)(
                mpCharWidths + rScriptItem.iCharPos,
                nCharIndexLimit - rScriptItem.iCharPos,
                nGlyphCount,
                mpLogClusters + rScriptItem.iCharPos,
                mpVisualAttrs + nGlyphsProcessed,
                mpGlyphAdvances + nGlyphsProcessed,
                &rScriptItem.a,
                NULL,
                mpJustifications + nGlyphsProcessed );
        }

        nGlyphsProcessed += nGlyphCount;
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
    int nGlyphsProcessed = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        int nGlyphCount = mpGlyphCounts[ nItem ];
        if( !nGlyphCount )
            continue;

        SCRIPT_ITEM& rScriptItem = mpScriptItems[ nItem ];
        int nCharIndexLimit = mpScriptItems[ nItem+1 ].iCharPos;

        if( (mnEndCharIndex > rScriptItem.iCharPos) &&  (mnFirstCharIndex < nCharIndexLimit) )
        {
            long nItemWidth = 0;
            for( i = rScriptItem.iCharPos; i < nCharIndexLimit; ++i )
                nItemWidth += mpCharWidths[ i ];

            SCRIPT_FONTPROPERTIES aFontProperties;
            int nMinKashida = 1;
            HRESULT nRC = (*pScriptGetFontProperties)( mhDC, &maScriptCache, &aFontProperties );
            if( !nRC )
                nMinKashida = aFontProperties.iKashidaWidth;

            nRC = (*pScriptJustify) (
                mpVisualAttrs + nGlyphsProcessed,
                mpGlyphAdvances + nGlyphsProcessed,
                nGlyphCount,
                (int)((fStretch-1.0) * nItemWidth + 0.5),
                nMinKashida,
                mpJustifications + nGlyphsProcessed );
        }

        nGlyphsProcessed += nGlyphCount;
    }
}

#endif // USE_UNISCRIBE

// =======================================================================

SalLayout* SalGraphics::LayoutText( const ImplLayoutArgs& rArgs )
{
    WinLayout* pWinLayout = NULL;

#ifdef USE_UNISCRIBE
    if( !(rArgs.mnFlags & SAL_LAYOUT_COMPLEX_DISABLED)
    &&  !bUspDisabled
    && (aUspModule || InitUSP())
    && ((rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL)
        || (S_OK == (*pScriptIsComplex)
                    ( rArgs.mpStr + rArgs.mnFirstCharIndex,
                      rArgs.mnEndCharIndex - rArgs.mnFirstCharIndex, SIC_COMPLEX ) ) ) )
        pWinLayout = new UniscribeLayout( maGraphicsData.mhDC, rArgs );
    else
#endif // USE_UNISCRIBE
        pWinLayout = new SimpleWinLayout( maGraphicsData.mhDC, rArgs );

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

// -----------------------------------------------------------------------

BOOL SalGraphics::GetLayoutOutline( const SalLayout& rSalLayout, PolyPolygon& rPolyPoly )
{
    rPolyPoly.Clear();

    // we know the SalLayout created by this SalGraphics is a WinLayout
    const WinLayout& rWinLayout = reinterpret_cast<const WinLayout&>( rSalLayout );
    return rWinLayout.GetOutline( *this, rPolyPoly );
}

// =======================================================================
