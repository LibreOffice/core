/*************************************************************************
 *
 *  $RCSfile: winlayout.cxx,v $
 *
 *  $Revision: 1.81 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:42:04 $
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

#ifndef _SV_SALGDI_H
#include <salgdi.h>
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

#include <psprint/sft.h>

#ifdef USE_UNISCRIBE
#include <Usp10.h>
#endif // USE_UNISCRIBE

#define DROPPED_OUTGLYPH 0xFFFF

// =======================================================================

// win32 specific physical font instance
class ImplWinFontEntry : public ImplFontEntry
{
public:
                            ImplWinFontEntry( ImplFontSelectData& );
                            ~ImplWinFontEntry();

private:
    // TODO: also add HFONT??? Watch out for issues with too many active fonts...

#ifdef GCP_KERN_HACK
public:
    bool                    HasKernData() const;
    void                    SetKernData( int, const KERNINGPAIR* );
    int                     GetKerning( sal_Unicode, sal_Unicode ) const;
private:
    KERNINGPAIR*            mpKerningPairs;
    int                     mnKerningPairs;
#endif // GCP_KERN_HACK

#ifdef USE_UNISCRIBE
public:
    SCRIPT_CACHE&           GetScriptCache() const
                                { return maScriptCache; }
private:
    mutable SCRIPT_CACHE    maScriptCache;
#endif // USE_UNISCRIBE
};

// =======================================================================

class WinLayout : public SalLayout
{
public:
                        WinLayout( HDC, ImplWinFontData&, ImplWinFontEntry& );
    virtual void        InitFont() const;

#ifdef USE_UNISCRIBE
    SCRIPT_CACHE&       GetScriptCache() const
                            { return mrWinFontEntry.GetScriptCache(); }
#endif // USE_UNISCRIBE

protected:
    HDC                 mhDC;
    HFONT               mhFont;
    int                 mnBaseAdv;

    ImplWinFontData&    mrWinFontData;
    ImplWinFontEntry&   mrWinFontEntry;
};

// =======================================================================

class SimpleWinLayout : public WinLayout
{
public:
                    SimpleWinLayout( HDC, BYTE nCharSet, ImplWinFontData&, ImplWinFontEntry& );
    virtual         ~SimpleWinLayout();

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;

    virtual int     GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos, int&,
                        long* pGlyphAdvances, int* pCharIndexes ) const;

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

private:
    int             mnGlyphCount;
    int             mnCharCount;
    WCHAR*          mpOutGlyphs;
    int*            mpGlyphAdvances;
    int*            mpGlyphOrigAdvs;
    int*            mpCharWidths;   // map rel char pos to char width
    int*            mpChars2Glyphs; // map rel char pos to abs glyph pos
    int*            mpGlyphs2Chars; // map abs glyph pos to abs char pos
    bool*           mpGlyphRTLFlags;// BiDi status for glyphs: true=>RTL
    mutable long    mnWidth;
    bool            mbDisableGlyphs;

    int             mnNotdefWidth;
    BYTE            mnCharSet;
};

// =======================================================================

WinLayout::WinLayout( HDC hDC, ImplWinFontData& rWFD, ImplWinFontEntry& rWFE )
:   mhDC( hDC ),
    mhFont( (HFONT)::GetCurrentObject(hDC,OBJ_FONT) ),
    mnBaseAdv( 0 ),
    mrWinFontData( rWFD ),
    mrWinFontEntry( rWFE )
{}

// -----------------------------------------------------------------------

void WinLayout::InitFont() const
{
    ::SelectObject( mhDC, mhFont );
}

// =======================================================================

SimpleWinLayout::SimpleWinLayout( HDC hDC, BYTE nCharSet,
    ImplWinFontData& rWinFontData, ImplWinFontEntry& rWinFontEntry )
:   WinLayout( hDC, rWinFontData, rWinFontEntry ),
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
    if( !aSalShlData.mbWNT )
        mbDisableGlyphs = true;
}

// -----------------------------------------------------------------------

SimpleWinLayout::~SimpleWinLayout()
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

bool SimpleWinLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    // prepare layout
    // TODO: fix case when reusing object
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
            mbDisableGlyphs = mrWinFontData.IsGlyphApiDisabled();
    }

    // TODO: use a cached value for bDisableAsianKern from upper layers
    if( rArgs.mnFlags & SAL_LAYOUT_KERNING_ASIAN )
    {
        TEXTMETRICA aTextMetricA;
        if( ::GetTextMetricsA( mhDC, &aTextMetricA )
        && !(aTextMetricA.tmPitchAndFamily & TMPF_FIXED_PITCH) )
            rArgs.mnFlags &= ~SAL_LAYOUT_KERNING_ASIAN;
    }

    // layout text
    int i, j;

    mnGlyphCount = 0;
    bool bVertical = (rArgs.mnFlags & SAL_LAYOUT_VERTICAL) != 0;
    if( !bVertical )
    {
        // count chars to process as LTR
        rArgs.ResetPos();
        bool bRTL;
        for( bRTL; rArgs.GetNextRun( &i, &j, &bRTL ) && !bRTL; )
            mnGlyphCount += j - i;
        // if there are RTL runs we need room to remember the RTL status
        if( bRTL )
        {
            mpGlyphRTLFlags = new bool[ mnCharCount ];
            for( i = 0; i < mnCharCount; ++i )
                mpGlyphRTLFlags[i] = false;
        }
    }

    const sal_Unicode* pBidiStr;
    if( mnGlyphCount == mnCharCount )
        pBidiStr = rArgs.mpStr + rArgs.mnMinCharPos;
    else
    {
        // rewrite pBidiStr when right to left/partial fallback runs/vertical layout
        sal_Unicode* pStr = (sal_Unicode*)alloca( mnCharCount * sizeof(sal_Unicode) );
        // note: glyph to char mapping is relative to first character
        mpChars2Glyphs = new int[ mnCharCount ];
        mpGlyphs2Chars = new int[ mnCharCount ];

        for( i = 0; i < mnCharCount; ++i )
            mpChars2Glyphs[i] = mpGlyphs2Chars[i] = -1;

        mnGlyphCount = 0;
        rArgs.ResetPos();
        for( bool bRTL; rArgs.GetNextRun( &i, &j, &bRTL ); )
        {
            if( bRTL )
            {
                // right to left
                do {
                    sal_Unicode cChar = rArgs.mpStr[ --j ];
                    pStr[ mnGlyphCount ] = ::GetMirroredChar( cChar );
                    mpChars2Glyphs[ j - rArgs.mnMinCharPos ] = mnGlyphCount;
                    mpGlyphRTLFlags[ mnGlyphCount ] = true;
                    mpGlyphs2Chars[ mnGlyphCount++ ] = j;
                } while( i < j );
            }
            else if( bVertical )
            {
                // vertical mode
                do {
                    sal_Unicode cChar = rArgs.mpStr[ i ];
                    sal_Unicode cVert = ::GetVerticalChar( cChar );
                    pStr[ mnGlyphCount ] = cVert ? cVert : cChar;
                    mpChars2Glyphs[ i - rArgs.mnMinCharPos ] = mnGlyphCount;
                    mpGlyphs2Chars[ mnGlyphCount++ ] = i;
                } while( ++i < j );
            }
            else
            {
                // left to right
                do {
                    sal_Unicode cChar = rArgs.mpStr[ i ];
                    pStr[ mnGlyphCount ] = cChar;
                    mpChars2Glyphs[ i - rArgs.mnMinCharPos ] = mnGlyphCount;
                    mpGlyphs2Chars[ mnGlyphCount++ ] = i;
                } while( ++i < j );
            }
        }
        pBidiStr = pStr;
    }

    mpOutGlyphs     = new WCHAR[ mnGlyphCount ];
    mpGlyphAdvances = new int[ mnGlyphCount ];

    if( rArgs.mnFlags & (SAL_LAYOUT_KERNING_PAIRS | SAL_LAYOUT_KERNING_ASIAN) )
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];

    DWORD nGcpOption = 0;
#ifndef GCP_KERN_HACK
    // enable kerning if requested
    if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
        nGcpOption |= GCP_USEKERNING;
#endif // GCP_KERN_HACK

    DWORD nRC;
    if( aSalShlData.mbWNT )  // TODO: remove when unicode layer successful
    {
        GCP_RESULTSW aGCPW;
        aGCPW.lStructSize   = sizeof(aGCPW);
        aGCPW.lpDx          = mpGlyphAdvances;
        aGCPW.lpCaretPos    = NULL;
        aGCPW.lpClass       = NULL;
        aGCPW.nGlyphs       = mnGlyphCount;
        aGCPW.nMaxFit       = 0;
        aGCPW.lpOrder       = NULL;

        // get glyphs/outstring and kerned placement
        if( mbDisableGlyphs )
        {
            aGCPW.lpOutString = mpOutGlyphs;
            aGCPW.lpGlyphs = NULL;
        }
        else
        {
            aGCPW.lpOutString = NULL;
            aGCPW.lpGlyphs = mpOutGlyphs;
            nGcpOption |= GCP_GLYPHSHAPE;
        }
        nRC = ::GetCharacterPlacementW( mhDC, pBidiStr, mnGlyphCount,
            0, &aGCPW, nGcpOption );
        mnGlyphCount = aGCPW.lpGlyphs ? aGCPW.nGlyphs : aGCPW.nMaxFit;

#ifndef GCP_KERN_HACK
        // get undisturbed placement
        if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
        {
            aGCPW.lpDx = mpGlyphOrigAdvs;
            nRC = ::GetCharacterPlacementW( mhDC, pBidiStr, mnGlyphCount,
                0, &aGCPW, (nGcpOption & ~GCP_USEKERNING) );
        }
#endif // GCP_KERN_HACK
    }
    else
    {
        if( mbDisableGlyphs )
        {
            // GetCharacterPlacementW replacement on non-W platforms:
            // in no-glyphs case unicode chars are interpreted as glyphs
            nRC = 0;
            for( i = 0; i < mnGlyphCount; ++i )
            {
                WCHAR cChar = pBidiStr[ i ];
                mpOutGlyphs[ i ] = cChar;
                ::GetCharWidthW( mhDC, cChar, cChar, &mpGlyphAdvances[i] );
                nRC += mpGlyphAdvances[i];
            }
        }
        else
        {
            // TODO: emulate full GetCharacterPlacementW on non-unicode OS
            // TODO: move into uwinapi.dll
            // convert into ANSI code page
            int nMBLen = ::WideCharToMultiByte( mnCharSet,
                WC_COMPOSITECHECK | WC_DISCARDNS | WC_DEFAULTCHAR,
                pBidiStr, mnGlyphCount, NULL, 0, NULL, NULL );
            if( (nMBLen <= 0) || (nMBLen >= 8 * mnGlyphCount) )
                return false;
            char* const pMBStr = (char*)alloca( nMBLen+1 );
            ::WideCharToMultiByte( mnCharSet,
                WC_COMPOSITECHECK | WC_DISCARDNS | WC_DEFAULTCHAR,
                pBidiStr, mnGlyphCount, pMBStr, nMBLen, NULL, NULL );

            // get glyphs/outstring and placement
            GCP_RESULTSA aGCPA;
            aGCPA.lStructSize   = sizeof(aGCPA);
            aGCPA.lpDx          = mpGlyphAdvances;
            aGCPA.lpCaretPos    = NULL;
            aGCPA.lpClass       = NULL;
            aGCPA.nMaxFit       = 0;
            aGCPA.nGlyphs       = mnGlyphCount;
            aGCPA.lpOrder       = NULL;

            aGCPA.lpGlyphs      = mpOutGlyphs;
            aGCPA.lpOutString   = NULL;
            nGcpOption |= GCP_GLYPHSHAPE;

            nRC = ::GetCharacterPlacementA( mhDC, pMBStr, nMBLen, 0, &aGCPA, nGcpOption );

            mnGlyphCount = aGCPA.nGlyphs;
        }

#ifndef GCP_KERN_HACK
        if( rArgs.mnFlags & SAL_LAYOUT_KERNING_PAIRS )
        {
            aGCPA.lpDx = mpGlyphOrigAdvs;
            nRC = ::GetCharacterPlacementA( mhDC, pMBStr, nMBLen,
                0, &aGCPA, (nGcpOption & ~GCP_USEKERNING) );
        }
#endif // GCP_KERN_HACK
    }

    mnWidth = 0;    // don't use nRC to calc width because of 16bit overflow

    if( !nRC )
        return false;

    // #101097# fixup display of NotDef glyphs
    // TODO: is there a way to convince Win32(incl W95) API to use notdef directly?
    for( i = 0; i < mnGlyphCount; ++i )
    {
        mnWidth += mpGlyphAdvances[i];

        // we are only interested in notdef candidates
        if( mbDisableGlyphs )
        {
            // TODO: improve heurisitics for non-GlyphIndexing NotDef detection
            if( mpGlyphAdvances[i] != 0 )
                continue;
        }
        else
        {
            // TODO: improve heuristic below
            // problem is that some fonts seem to have NotDef at glyphids 0,3,4
            if( (mpGlyphAdvances[i] != 0) || (mpOutGlyphs[i] > 4)  )
                if( mpOutGlyphs[i] != 0 )
                    continue;
        }

        bool bRTL = mpGlyphRTLFlags ? mpGlyphRTLFlags[i] : false;
        int nCharPos = mpGlyphs2Chars ? mpGlyphs2Chars[i]: i + rArgs.mnMinCharPos;

       // double check with the font face, if it has support for the unicode
       sal_Unicode cChar = rArgs.mpStr[ nCharPos ];
       if( mrWinFontData.HasChar( cChar ) )
           continue;

        // request glyph fallback at this position
        rArgs.NeedFallback( nCharPos, bRTL );

        if( rArgs.mnFlags & SAL_LAYOUT_FOR_FALLBACK )
        {
            // when we already are layouting for glyph fallback
            // then a new unresolved glyph is not interesting
            mpOutGlyphs[i] = DROPPED_OUTGLYPH;
            mnNotdefWidth = 0;
        }
        else
        {
            if( mnNotdefWidth < 0 )
            {
                // get the width of the NotDef glyph
                SIZE aExtent;
                WCHAR cNotDef = rArgs.mpStr[ nCharPos ];
                mnNotdefWidth = ::GetTextExtentPoint32W(mhDC,&cNotDef,1,&aExtent) ? aExtent.cx : 0;
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
                int nKernAmount = mrWinFontEntry.GetKerning( pBidiStr[i-1], pBidiStr[i] );
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
    long nXOffset = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nXOffset += mpGlyphAdvances[ i ];

    // calculate absolute position in pixel units
    Point aRelativePos( nXOffset, 0 );
    rPos = GetDrawPosition( aRelativePos );

    int nCount = 0;
    while( nCount < nLen )
    {
        // update return values (nGlyphIndex,nCharPos,nGlyphAdvance)
        long nGlyphIndex = mpOutGlyphs[ nStart ];
        if( mbDisableGlyphs )
        {
            if( mnLayoutFlags & SAL_LAYOUT_VERTICAL )
            {
#ifdef GNG_VERT_HACK
                sal_Unicode cChar = (sal_Unicode)(nGlyphIndex & GF_IDXMASK);
                if( mrWinFontData.HasGSUBstitutions( mhDC )
                &&  mrWinFontData.IsGSUBstituted( cChar ) )
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

void SimpleWinLayout::DrawText( SalGraphics& rGraphics ) const
{
    if( mnGlyphCount <= 0 )
        return;

    HDC aHDC = static_cast<WinSalGraphics&>(rGraphics).mhDC;

    UINT mnDrawOptions = ETO_GLYPH_INDEX;
    if( mbDisableGlyphs )
        mnDrawOptions = 0;

    Point aPos = GetDrawPosition( Point( mnBaseAdv, 0 ) );

     // #108267#, limit the number of glyphs to avoid paint errors
    UINT limitedGlyphCount = Min( 8192, mnGlyphCount );
   // #108267#, break up into glyph portions of a limited size required by Win32 API
    const unsigned int maxGlyphCount = 8192;
    UINT numGlyphPortions = mnGlyphCount / maxGlyphCount;
    UINT remainingGlyphs = mnGlyphCount % maxGlyphCount;

    if( mnDrawOptions || aSalShlData.mbWNT )
    {
        if( numGlyphPortions )
        {
            // #108267#,#109387# break up string into smaller chunks
            // the output positions will be updated by windows (SetTextAlign)
            long i,n;
            POINT oldPos;
            UINT oldTa = ::GetTextAlign( aHDC );
            ::SetTextAlign( aHDC, (oldTa & ~TA_NOUPDATECP) | TA_UPDATECP );
            ::MoveToEx( aHDC, aPos.X(), aPos.Y(), &oldPos );
            for( i=n=0; n<numGlyphPortions; n++, i+=maxGlyphCount )
                ::ExtTextOutW( aHDC, 0, 0, mnDrawOptions, NULL,
                    mpOutGlyphs+i, maxGlyphCount, mpGlyphAdvances+i );
            ::ExtTextOutW( aHDC, 0, 0, mnDrawOptions, NULL,
                mpOutGlyphs+i, remainingGlyphs, mpGlyphAdvances+i );
            ::MoveToEx( aHDC, oldPos.x, oldPos.y, (LPPOINT) NULL);
            ::SetTextAlign( aHDC, oldTa );
        }
        else
            ::ExtTextOutW( aHDC, aPos.X(), aPos.Y(), mnDrawOptions, NULL,
                mpOutGlyphs, mnGlyphCount, mpGlyphAdvances );
    }
    else
    {
        // #108267#, On Win9x, we get paint errors when drawing huge strings, even when
        // split into pieces (see above), seems to be a problem in the internal text clipping
        // so we just cut off the string
        if( !mpGlyphOrigAdvs )
            ::ExtTextOutW( aHDC, aPos.X(), aPos.Y(), 0, NULL,
                mpOutGlyphs, limitedGlyphCount, NULL );
        else
        {
            // workaround for problem in #106259#
            long nXPos = mnBaseAdv;
            for( int i = 0; i < limitedGlyphCount; ++i )
            {
                ::ExtTextOutW( aHDC, aPos.X(), aPos.Y(), 0, NULL,
                    mpOutGlyphs+i, 1, NULL );
                nXPos += mpGlyphAdvances[ i ];
                aPos = GetDrawPosition( Point( nXPos, 0 ) );
            }
        }
    }
}

// -----------------------------------------------------------------------

long SimpleWinLayout::FillDXArray( long* pDXArray ) const
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

int SimpleWinLayout::GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const
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

void SimpleWinLayout::GetCaretPositions( int nMaxIdx, long* pCaretXArray ) const
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

void SimpleWinLayout::Justify( long nNewWidth )
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

void SimpleWinLayout::AdjustLayout( ImplLayoutArgs& rArgs )
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

void SimpleWinLayout::ApplyDXArray( const ImplLayoutArgs& rArgs )
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
    if( i >= mnGlyphCount )
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

void SimpleWinLayout::MoveGlyph( int nStart, long nNewXPos )
{
    if( nStart >= mnGlyphCount )
        return;
    // TODO: cache absolute positions
    int nDelta = mnBaseAdv;
    for( int i = 0; i < nStart; ++i )
        nDelta += mpGlyphAdvances[ i ];
    nDelta = nNewXPos - nDelta;
    if( mnWidth )
    {
        // when width is cached it needs to be adjusted
        mnWidth += nDelta;
    }
    if( nStart > 0 )
    {
        // adjust virtual width when inbetween
        mpGlyphAdvances[ nStart-1 ] += nDelta;
    }
    else
    {
        // change offset when first glyph has fallback
        mnBaseAdv = nNewXPos;
    }
}

// -----------------------------------------------------------------------

void SimpleWinLayout::DropGlyph( int nStart )
{
    mpOutGlyphs[ nStart ] = DROPPED_OUTGLYPH;
}

// -----------------------------------------------------------------------

void SimpleWinLayout::Simplify( bool bIsBase )
{
    // return early if no glyph is dropped
    int i = mnGlyphCount;
    while( (--i >= 0) && (mpOutGlyphs[ i ] != DROPPED_OUTGLYPH) );
    if( i < 0 )
        return;

    // convert to sparse layout when glyphs are dropped
    if( !mpGlyphs2Chars )
    {
        mpGlyphs2Chars = new int[ mnGlyphCount ];
        mpCharWidths = new int[ mnCharCount ];
        // assertion: mnGlyphCount == mnCharCount
        for( int k = 0; k < mnGlyphCount; ++k )
        {
            mpGlyphs2Chars[ k ] = k + mnMinCharPos;
            mpCharWidths[ k ] = mpGlyphAdvances[ k ];
        }
    }

    // when glyphs at end have been removed => reduce width
    for( i = mnGlyphCount; --i >= 0; )
    {
        if( mpOutGlyphs[ i ] != DROPPED_OUTGLYPH )
            break;
        mnWidth -= mpGlyphAdvances[ i ];
        int c = mpGlyphs2Chars[ i ] - mnMinCharPos;
        if( c >= 0 )
            mpCharWidths[ c ] = 0;
    }
    mnGlyphCount = i + 1;

    // keep original widths around
    if( !mpGlyphOrigAdvs )
    {
        mpGlyphOrigAdvs = new int[ mnGlyphCount ];
        for( int k = 0; k < mnGlyphCount; ++k )
            mpGlyphOrigAdvs[ k ] = mpGlyphAdvances[ k ];
    }

    // skip to first dropped glyph
    for( i = 0; i < mnGlyphCount; ++i )
        if( mpOutGlyphs[ i ] == DROPPED_OUTGLYPH )
            break;

    // remove dropped glyphs inbetween
    int nNewGC = i;
    for(; i < mnGlyphCount; ++i )
    {
        mpOutGlyphs[ nNewGC ]     = mpOutGlyphs[ i ];
        mpGlyphAdvances[ nNewGC ] = mpGlyphAdvances[ i ];
        mpGlyphOrigAdvs[ nNewGC ] = mpGlyphOrigAdvs[ i ];
        mpGlyphs2Chars[ nNewGC ]  = mpGlyphs2Chars[ i ];

        if( mpOutGlyphs[ i ] != DROPPED_OUTGLYPH )
        {
            ++nNewGC;
            continue;
        }

        // zero virtual char width for fallback char
        int c = mpGlyphs2Chars[ i ] - mnMinCharPos;
        if( c >= 0 )
            mpCharWidths[ c ] = 0;

        // adjust positions for dropped glyph
        if( nNewGC > 0 )
            mpGlyphAdvances[ nNewGC-1 ] += mpGlyphAdvances[ i ];
        else
            mnBaseAdv += mpGlyphAdvances[ i ];
    }

    mnGlyphCount = nNewGC;
    if( mnGlyphCount <= 0 )
        mnWidth = mnBaseAdv = 0;
}

// =======================================================================

#ifdef USE_UNISCRIBE

struct VisualItem
{
public:
    SCRIPT_ITEM*    mpScriptItem;
    int             mnMinGlyphPos;
    int             mnEndGlyphPos;
    int             mnMinCharPos;
    int             mnEndCharPos;
    //long          mnPixelWidth;
    int             mnXOffset;
    ABC             maABCWidths;

public:
    bool            IsEmpty() const { return (mnEndGlyphPos <= 0); }
};

class UniscribeLayout : public WinLayout
{
public:
                    UniscribeLayout( HDC, ImplWinFontData&, ImplWinFontEntry& );

    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    AdjustLayout( ImplLayoutArgs& );
    virtual void    DrawText( SalGraphics& ) const;
    virtual int     GetNextGlyphs( int nLen, long* pGlyphs, Point& rPos, int&,
                        long* pGlyphAdvances, int* pCharPosAry ) const;

    virtual long    FillDXArray( long* pDXArray ) const;
    virtual int     GetTextBreak( long nMaxWidth, long nCharExtra, int nFactor ) const;
    virtual void    GetCaretPositions( int nArraySize, long* pCaretXArray ) const;

    // for glyph+font+script fallback
    virtual void    MoveGlyph( int nStart, long nNewXPos );
    virtual void    DropGlyph( int nStart );
    virtual void    Simplify( bool bIsBase );

protected:
    virtual         ~UniscribeLayout();

    void            Justify( long nNewWidth );
    void            ApplyDXArray( const ImplLayoutArgs& );

    bool            GetItemSubrange( const VisualItem&,
                        int& rMinIndex, int& rEndIndex ) const;

private:
    // item specific info
    SCRIPT_ITEM*    mpScriptItems;  // in logical order
    VisualItem*     mpVisualItems;  // in visual order
    int             mnItemCount;

    // string specific info
    // everything is in logical order
    int             mnCharCapacity;
    WORD*           mpLogClusters;  // mapping relative to script items
    int*            mpCharWidths;   // map abs char pos to char width
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
    mutable int*    mpGlyphs2Chars; // map abs glyph pos to abs char pos
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

UniscribeLayout::UniscribeLayout( HDC hDC,
    ImplWinFontData& rWinFontData, ImplWinFontEntry& rWinFontEntry )
:   WinLayout( hDC, rWinFontData, rWinFontEntry ),
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
    // for fallback layout prepare to drop unneeded glyphs
    typedef std::vector<int> TIntVector;
    TIntVector aDropChars;
    if( rArgs.mnFlags & SAL_LAYOUT_FOR_FALLBACK )
    {
        // calculate superfluous context char positions
        aDropChars.push_back( 0 );
        aDropChars.push_back( rArgs.mnLength );
        int nMin, nEnd;
        bool bRTL;
        for( rArgs.ResetPos(); rArgs.GetNextRun( &nMin, &nEnd, &bRTL ); )
        {
            aDropChars.push_back( nMin );
            aDropChars.push_back( nEnd );
        }
        // prepare aDropChars for binary search
        std::sort( aDropChars.begin(), aDropChars.end() );
    }

    // prepare layout
    // TODO: fix case when reusing object
    mnMinCharPos = rArgs.mnMinCharPos;
    mnEndCharPos = rArgs.mnEndCharPos;

    // determine script items from string
    // TODO: try to avoid itemization since it costs a lot of performance
    SCRIPT_STATE aScriptState = {0,false,false,false,false,false,false,false,false,0,0};
    aScriptState.uBidiLevel         = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL));
    aScriptState.fOverrideDirection = (0 != (rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG));
    aScriptState.fDigitSubstitute   = (0 != (rArgs.mnFlags & SAL_LAYOUT_SUBSTITUTE_DIGITS));
    aScriptState.fArabicNumContext  = aScriptState.fDigitSubstitute & aScriptState.uBidiLevel;
    DWORD nLangId = 0;  // TODO: get language from font
    SCRIPT_CONTROL aScriptControl = {nLangId,false,false,false,false,false,false,false,false,0};
    aScriptControl.fNeutralOverride = aScriptState.fOverrideDirection;
    aScriptControl.fContextDigits   = (0 != (rArgs.mnFlags & SAL_LAYOUT_SUBSTITUTE_DIGITS));
    // determine relevant substring and work only on it
    // when Bidi status is unknown we need to look at the whole string though
    mnSubStringMin = 0;
    int nSubStringEnd = rArgs.mnLength;
    if( aScriptState.fOverrideDirection )
    {
        // TODO: limit substring to portion limits
        mnSubStringMin = rArgs.mnMinCharPos - 8;
        if( mnSubStringMin < 0 )
            mnSubStringMin = 0;
        nSubStringEnd = rArgs.mnEndCharPos + 8;
        if( nSubStringEnd > rArgs.mnLength )
            nSubStringEnd = rArgs.mnLength;

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
    int nItem, i;

    // adjust char positions by substring offset
    for( nItem = 0; nItem <= mnItemCount; ++nItem )
        mpScriptItems[ nItem ].iCharPos += mnSubStringMin;
    // default visual item ordering
    mpVisualItems = new VisualItem[ mnItemCount ];
    for( nItem = 0; nItem < mnItemCount; ++nItem )
    {
        // initialize char specific item info
        VisualItem& rVisualItem = mpVisualItems[ nItem ];
        SCRIPT_ITEM* pScriptItem = &mpScriptItems[ nItem ];
        rVisualItem.mpScriptItem = pScriptItem;
        rVisualItem.mnMinCharPos = pScriptItem[0].iCharPos;
        rVisualItem.mnEndCharPos = pScriptItem[1].iCharPos;
    }

    // reorder visual item order if needed
    if( rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG )
    {
        // force RTL item ordering if requested
        if( rArgs.mnFlags & SAL_LAYOUT_BIDI_RTL )
        {
            VisualItem* pVI0 = &mpVisualItems[ 0 ];
            VisualItem* pVI1 = &mpVisualItems[ mnItemCount ];
            while( pVI0 < --pVI1 )
            {
                VisualItem aVtmp = *pVI0;
                *(pVI0++) = *pVI1;
                *pVI1 = aVtmp;
            }
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
                while( pVImin < --pVImax )
                {
                    VisualItem aVtmp = *pVImin;
                    *(pVImin++) = *pVImax;
                    *pVImax = aVtmp;
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

    long nXOffset = 0;

    // layout script items
    SCRIPT_CACHE& rScriptCache = GetScriptCache();
    for( nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // initialize glyph specific item info
        rVisualItem.mnMinGlyphPos = mnGlyphCount;
        rVisualItem.mnEndGlyphPos = 0;
        rVisualItem.mnXOffset     = nXOffset;
        //rVisualItem.mnPixelWidth  = 0;

        // shortcut ignorable items
        if( (rArgs.mnEndCharPos <= rVisualItem.mnMinCharPos)
         || (rArgs.mnMinCharPos >= rVisualItem.mnEndCharPos) )
        {
            for( int i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; ++i )
                mpLogClusters[ i ] = -1;
            continue;
        }

        // override bidi analysis if requested
        if( rArgs.mnFlags & SAL_LAYOUT_BIDI_STRONG )
        {
            rVisualItem.mpScriptItem->a.fRTL                 = aScriptState.uBidiLevel;
            rVisualItem.mpScriptItem->a.s.uBidiLevel         = aScriptState.uBidiLevel;
            rVisualItem.mpScriptItem->a.s.fOverrideDirection = aScriptState.fOverrideDirection;
        }

        int nGlyphCount = 0;
        int nCharCount = rVisualItem.mnEndCharPos - rVisualItem.mnMinCharPos;
        HRESULT nRC = (*pScriptShape)( mhDC, &rScriptCache,
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
            // request fallback for whole item
            rArgs.NeedFallback( rVisualItem.mnMinCharPos, rVisualItem.mnEndCharPos,
                rVisualItem.mpScriptItem->a.fRTL );

            // for now fall back to default layout
            rVisualItem.mpScriptItem->a.eScript = SCRIPT_UNDEFINED;
            nRC = (*pScriptShape)( mhDC, &rScriptCache,
                rArgs.mpStr + rVisualItem.mnMinCharPos,
                nCharCount,
                mnGlyphCapacity - rVisualItem.mnMinGlyphPos,
                &rVisualItem.mpScriptItem->a,
                mpOutGlyphs + rVisualItem.mnMinGlyphPos,
                mpLogClusters + rVisualItem.mnMinCharPos,
                mpVisualAttrs + rVisualItem.mnMinGlyphPos,
                &nGlyphCount );

            if( nRC != 0 )
                continue;

            // mark as NotDef glyphs
            for( i = 0; i < nGlyphCount; ++i )
                mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] = 0;
        }
        else if( nRC != 0 )
            // something undefined happened => give up
            continue;
        else
        {
            // check if there are any NotDef glyphs
            for( i = 0; i < nGlyphCount; ++i )
                if( 0 == mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] )
                    break;
            if( i < nGlyphCount )
            {
                // calculate proper charpos limits
                int nMinCharPos = rVisualItem.mnMinCharPos;
                if( nMinCharPos < rArgs.mnMinCharPos )
                    nMinCharPos = rArgs.mnMinCharPos;
                int nEndCharPos = rVisualItem.mnEndCharPos;
                if( nEndCharPos > rArgs.mnEndCharPos )
                    nEndCharPos = rArgs.mnEndCharPos;
                // request fallback for individual NotDef glyphs
                do
                {
                    if( 0 != mpOutGlyphs[ i + rVisualItem.mnMinGlyphPos ] )
                        continue;
                    // find matching nCharPos
                    // TODO: optimize algorithm
                    for( int j = nMinCharPos; j < nEndCharPos; ++j )
                    {
                        if( mpLogClusters[ j ] != i )
                            continue;
                        rArgs.NeedFallback( j, rVisualItem.mpScriptItem->a.fRTL );
                        break;
                    }
                } while( ++i < nGlyphCount );
            }
        }

        nRC = (*pScriptPlace)( mhDC, &rScriptCache,
            mpOutGlyphs + rVisualItem.mnMinGlyphPos,
            nGlyphCount,
            mpVisualAttrs + rVisualItem.mnMinGlyphPos,
            &rVisualItem.mpScriptItem->a,
            mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
            mpGlyphOffsets + rVisualItem.mnMinGlyphPos,
            &rVisualItem.maABCWidths );

        if( nRC != 0 )
            continue;

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

        // update nXOffset
        int nEndGlyphPos;
        if( GetItemSubrange( rVisualItem, i, nEndGlyphPos ) )
            for(; i < nEndGlyphPos; ++i )
                nXOffset += mpGlyphAdvances[ i ];

        // for fallback layout unneeded context glyphs are dropped
        if( rArgs.mnFlags & SAL_LAYOUT_FOR_FALLBACK )
        {
            TIntVector::const_iterator it = aDropChars.begin();
            while( it != aDropChars.end() )
            {
                // find matching "drop range"
                int nMinDropPos = *(it++); // begin of drop range
                if( nMinDropPos >= rVisualItem.mnEndCharPos )
                    break;
                int nEndDropPos = *(it++); // end of drop range
                if( nEndDropPos <= rVisualItem.mnMinCharPos )
                    continue;
                // match "drop range" to visual item
                if( nMinDropPos <= rVisualItem.mnMinCharPos )
                {
                    nMinDropPos = rVisualItem.mnMinCharPos;
                    // drop whole visual item if possible
                    if( nEndDropPos >= rVisualItem.mnEndCharPos )
                    {
                        rVisualItem.mnEndGlyphPos = 0;
                        // recycle those glyphs immediately
                        mnGlyphCount -= nGlyphCount;
                        break;
                    }
                }
                if( nEndDropPos > rVisualItem.mnEndCharPos )
                    nEndDropPos = rVisualItem.mnEndCharPos;
                // drop those glyphs
                for( int j = nMinDropPos; j < nEndDropPos; ++j )
                {
                    int nGlyphPos = rVisualItem.mnMinGlyphPos;
                    nGlyphPos += mpLogClusters[ j ];
                    mpOutGlyphs[ nGlyphPos ] = DROPPED_OUTGLYPH;
                }
            }
        }

        // TODO: shrink glyphpos limits to match charpos/fallback limits
        //pVI->mnMinGlyphPos = nMinGlyphPos;
        //pVI->mnEndGlyphPos = nEndGlyphPos;
    }

    return true;
}

// -----------------------------------------------------------------------

bool UniscribeLayout::GetItemSubrange( const VisualItem& rVisualItem,
    int& rMinGlyphPos, int& rEndGlyphPos ) const
{
    // return early when nothing of interest in this item
    if( rVisualItem.IsEmpty()
     || (rVisualItem.mnEndCharPos <= mnMinCharPos)
     || (mnEndCharPos <= rVisualItem.mnMinCharPos) )
        return false;

    // default: subrange is complete range
    rMinGlyphPos = rVisualItem.mnMinGlyphPos;
    rEndGlyphPos = rVisualItem.mnEndGlyphPos;

    // return early when the whole item is of interest
    if( (mnMinCharPos <= rVisualItem.mnMinCharPos)
     && (rVisualItem.mnEndCharPos <= mnEndCharPos ) )
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
    int& nStart, long* pGlyphAdvances, int* pCharPosAry ) const
{
    if( nStart > mnGlyphCount )        // nStart>MAX means no more glyphs
        return 0;

    int i;
    const VisualItem* pVI = mpVisualItems;
    if( nStart == 0 )                 // nStart==0 for first visible glyph
    {
        // find first visible item
        for( i = 0; i < mnItemCount; ++i, ++pVI )
            if( !pVI->IsEmpty() )
            {
                nStart = pVI->mnMinGlyphPos;
                break;
            }
    }
    else //if( nStart > 0 )           // nStart>0 means absolute glyph pos +1
    {
        --nStart;

        // find matching item
        for( i = 0; i < mnItemCount; ++i, ++pVI )
            if( (nStart >= pVI->mnMinGlyphPos) && (nStart < pVI->mnEndGlyphPos) )
                break;
    }

    if( i >= mnItemCount )
    {
        // everything has been read
        nStart = mnGlyphCount;
        return 0;
    }

    // calculate start of next visible item
    int nNextItemStart = mnGlyphCount;
    while( ++i < mnItemCount )
    {
        if( mpVisualItems[i].IsEmpty() )
            continue;
        nNextItemStart = mpVisualItems[i].mnMinGlyphPos;
        break;
    }

    int nMinGlyphPos, nEndGlyphPos;
    bool bRC = GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos );
    DBG_ASSERT( bRC, "USPLayout::GNG GISR() returned false" );

    if( nStart < nMinGlyphPos )
        nStart = nMinGlyphPos;

    // calculate glyph position relative to layout base
    // advance to next visual position by using adjusted glyph widths
    // TODO: avoid for nStart!=0 case by reusing rPos
    long nXOffset = pVI->mnXOffset;
    const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;
    for( i = nMinGlyphPos; i < nStart; ++i )
        nXOffset += pGlyphWidths[ i ];

    // adjust nXOffset relative to cluster start
    i = mnMinCharPos;
    if( !pVI->mpScriptItem->a.fRTL )
    {
        int nTmpIndex = mpLogClusters[ i ];
        while( (--i >= pVI->mnMinCharPos)
            && (nTmpIndex == mpLogClusters[i]) )
            nXOffset -= mpCharWidths[i];
    }
    else
    {
        int nTmpIndex = mpLogClusters[ pVI->mnEndCharPos - 1 ];
        while( (--i >= pVI->mnMinCharPos)
            && (nTmpIndex == mpLogClusters[i]) )
            nXOffset += mpCharWidths[i];
    }

    // for usage below create mpGlyphs2Chars if needed
    if( pCharPosAry && !mpGlyphs2Chars )
    {
        mpGlyphs2Chars = new int[ mnGlyphCapacity ];
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphs2Chars[ i ] = -1;
        for( int nItem = 0; nItem < mnItemCount; ++nItem )
        {
            // skip invisible items
            VisualItem& rVI = mpVisualItems[ nItem ];
            if( rVI.IsEmpty() )
                continue;
            for( i = rVI.mnEndCharPos; --i >= rVI.mnMinCharPos; )
            {
                int j = mpLogClusters[ i ] + rVI.mnMinGlyphPos;
                mpGlyphs2Chars[ j ] = i;
            }
        }
    }

    // calculate absolute position in pixel units
    const GOFFSET aGOffset = mpGlyphOffsets[ nStart ];
    Point aRelativePos( nXOffset + aGOffset.du, aGOffset.dv );
    rPos = GetDrawPosition( aRelativePos );

    int nCount = 0;
    while( nCount < nLen )
    {
        // update returned values
        *(pGlyphs++) = mpOutGlyphs[ nStart ];
        if( pGlyphAdvances )
            *(pGlyphAdvances++) = pGlyphWidths[ nStart ];
        if( pCharPosAry )
            *(pCharPosAry++) = mpGlyphs2Chars[ nStart ];

        ++nCount;
        // stop aftert last visible glyph in item
        if( ++nStart >= nEndGlyphPos )
        {
            nStart = nNextItemStart;
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

    ++nStart;
    return nCount;
}

// -----------------------------------------------------------------------

void UniscribeLayout::MoveGlyph( int nStart, long nNewXPos )
{
    if( nStart > mnGlyphCount )
        return;

    VisualItem* pVI = mpVisualItems;
    int nMinGlyphPos, nEndGlyphPos;
    if( nStart == 0 )               // nStart==0 for first visible glyph
    {
        for( int i = mnItemCount; --i >= 0; ++pVI )
            if( GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos ) )
                break;
        nStart = nMinGlyphPos;
        DBG_ASSERT( nStart <= mnGlyphCount, "USPLayout::MoveG overflow" );
    }
    else //if( nStart > 0 )         // nStart>0 means absolute glyph pos +1
    {
        --nStart;
        for( int i = mnItemCount; --i >= 0; ++pVI )
            if( (nStart >= pVI->mnMinGlyphPos) && (nStart < pVI->mnEndGlyphPos) )
                break;
        bool bRC = GetItemSubrange( *pVI, nMinGlyphPos, nEndGlyphPos );
        DBG_ASSERT( bRC, "USPLayout::MoveG GISR() returned false" );
    }

    long nDelta = nNewXPos - pVI->mnXOffset;
    if( nStart > nMinGlyphPos )
    {
        int i;
        for( i = nMinGlyphPos; i < nStart; ++i )
            nDelta -= mpGlyphAdvances[ i ];
        mpGlyphAdvances[ i-1 ] += nDelta;
    }
    else
        pVI->mnXOffset += nDelta;
}

// -----------------------------------------------------------------------

void UniscribeLayout::DropGlyph( int nStart )
{
    DBG_ASSERT( nStart<=mnGlyphCount, "USPLayout::MoveG nStart overflow" );

    if( nStart > 0 )        // nStart>0 means absolute glyph pos + 1
        --nStart;
    else // if( !nStart )   // nStart==0 for first visible glyph
    {
        const VisualItem* pVI = mpVisualItems;
        for( int i = mnItemCount, nDummy; --i >= 0; ++pVI )
            if( GetItemSubrange( *pVI, nStart, nDummy ) )
                break;
        DBG_ASSERT( nStart <= mnGlyphCount, "USPLayout::DropG overflow" );
    }

    mpOutGlyphs[ nStart ] = DROPPED_OUTGLYPH;
}

// -----------------------------------------------------------------------

void UniscribeLayout::Simplify( bool bIsBase )
{
    const WCHAR cDroppedGlyph = bIsBase ? DROPPED_OUTGLYPH : 0;
    int i;
    // if there are no dropped glyphs don't bother
    for( i = 0; i < mnGlyphCount; ++i )
        if( mpOutGlyphs[ i ] == cDroppedGlyph )
            break;
    if( i >= mnGlyphCount )
        return;

    // prepare for sparse layout
    if( !mpGlyphs2Chars )
    {
        mpGlyphs2Chars = new int[ mnGlyphCapacity ];
        for( i = 0; i < mnGlyphCount; ++i )
            mpGlyphs2Chars[ i ] = -1;
        for( int nItem = 0; nItem < mnItemCount; ++nItem )
        {
            // skip invisible items
            VisualItem& rVI = mpVisualItems[ nItem ];
            if( rVI.IsEmpty() )
                continue;
            for( i = rVI.mnEndCharPos; --i >= rVI.mnMinCharPos; )
            {
                int j = mpLogClusters[ i ] + rVI.mnMinGlyphPos;
                mpGlyphs2Chars[ j ] = i;
            }
        }
    }

    // remove the dropped glyphs
    const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVI = mpVisualItems[ nItem ];
        if( rVI.IsEmpty() )
            continue;

        // mark replaced character widths
        for( i = rVI.mnMinCharPos; i < rVI.mnEndCharPos; ++i )
        {
            int j = mpLogClusters[ i ] + rVI.mnMinGlyphPos;
            if( mpOutGlyphs[ j ] == cDroppedGlyph )
                mpCharWidths[ i ] = 0;
        }

        // dropped glyphs at start of visual item
        int nEndGlyphPos;
        GetItemSubrange( rVI, i, nEndGlyphPos );
        while( (mpOutGlyphs[i] == cDroppedGlyph) && (i < nEndGlyphPos) )
        {
            rVI.mnXOffset += pGlyphWidths[ i ];
            rVI.mnMinGlyphPos = ++i;
        }

        // when whole item got dropped mark as empty
        if( i >= nEndGlyphPos )
        {
            rVI.mnEndGlyphPos = 0;
            continue;
        }

        // dropped glyphs in the middle of visual item
        for(; i < nEndGlyphPos; ++i )
            if( mpOutGlyphs[ i ] == cDroppedGlyph )
                break;
        int j = i;
        while( ++i < nEndGlyphPos )
        {
            if( mpOutGlyphs[ i ] == cDroppedGlyph )
                continue;
            mpOutGlyphs[ j ]      = mpOutGlyphs[ i ];
            mpGlyphOffsets[ j ]   = mpGlyphOffsets[ i ];
            mpVisualAttrs[ j ]    = mpVisualAttrs[ i ];
            mpGlyphAdvances[ j ]  = mpGlyphAdvances[ i ];
            if( mpJustifications )
                mpJustifications[ j ] = mpJustifications[ i ];
            int k = mpGlyphs2Chars[ i ];
            mpGlyphs2Chars[ j ]   = k;
            mpLogClusters[ k ]    = j;
            rVI.mnEndGlyphPos = ++j;
        }
    }
}

// -----------------------------------------------------------------------

void UniscribeLayout::DrawText( SalGraphics& ) const
{
    int nBaseClusterOffset = 0;
    int nBaseGlyphPos = -1;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        const VisualItem& rVisualItem = mpVisualItems[ nItem ];

        // skip if there is nothing to display
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVisualItem, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        if( nBaseGlyphPos < 0 )
        {
            // adjust draw position relative to cluster start
            if( rVisualItem.mpScriptItem->a.fRTL )
                nBaseGlyphPos = nEndGlyphPos - 1;
            else
                nBaseGlyphPos = nMinGlyphPos;

            const int* pGlyphWidths;
            if( mpJustifications )
                pGlyphWidths = mpJustifications;
            else
                pGlyphWidths = mpGlyphAdvances;

            int i = mnMinCharPos;
            while( (--i >= rVisualItem.mnMinCharPos)
                && (nBaseGlyphPos == mpLogClusters[i]) )
                 nBaseClusterOffset += mpCharWidths[i];

            if( !rVisualItem.mpScriptItem->a.fRTL )
                nBaseClusterOffset = -nBaseClusterOffset;
        }

        // now draw the matching glyphs in this item
        Point aRelPos( rVisualItem.mnXOffset + nBaseClusterOffset, 0 );
        Point aPos = GetDrawPosition( aRelPos );
        SCRIPT_CACHE& rScriptCache = GetScriptCache();
        HRESULT nRC = (*pScriptTextOut)( mhDC, &rScriptCache,
            aPos.X(), aPos.Y(), 0, NULL,
            &rVisualItem.mpScriptItem->a, NULL, 0,
            mpOutGlyphs + nMinGlyphPos,
            nEndGlyphPos - nMinGlyphPos,
            mpGlyphAdvances + nMinGlyphPos,
            mpJustifications ? mpJustifications + nMinGlyphPos : NULL,
            mpGlyphOffsets + nMinGlyphPos );
    }
}

// -----------------------------------------------------------------------

long UniscribeLayout::FillDXArray( long* pDXArray ) const
{
    long nWidth = mnBaseAdv;

    for( int nItem = mnItemCount; --nItem >= 0; )
    {
        const VisualItem& rVI = mpVisualItems[ nItem ];

        // skip if there is nothing to display
        int nMinGlyphPos, nEndGlyphPos;
        if( !GetItemSubrange( rVI, nMinGlyphPos, nEndGlyphPos ) )
            continue;

        // width = xoffset + width of last item
        nWidth = rVI.mnXOffset;
        const int* pGlyphWidths = mpJustifications ? mpJustifications : mpGlyphAdvances;
        for( int i = nMinGlyphPos; i < nEndGlyphPos; ++i )
            nWidth += pGlyphWidths[i];
        break;
    }

    if( pDXArray )
        for( int i = mnMinCharPos; i < mnEndCharPos; ++i )
            pDXArray[ i - mnMinCharPos ] = mpCharWidths[ i ];

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

void UniscribeLayout::GetCaretPositions( int nMaxIdx, long* pCaretXArray ) const
{
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
        if( rVisualItem.IsEmpty() )
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

void UniscribeLayout::AdjustLayout( ImplLayoutArgs& rArgs )
{
    SalLayout::AdjustLayout( rArgs );

    // adjust positions if requested
    if( rArgs.mpDXArray )
        ApplyDXArray( rArgs );
    else if( rArgs.mnLayoutWidth )
        Justify( rArgs.mnLayoutWidth );
}

// -----------------------------------------------------------------------

void UniscribeLayout::ApplyDXArray( const ImplLayoutArgs& rArgs )
{
    const long* pDXArray = rArgs.mpDXArray;

    // increase char widths in string range to desired values
    bool bModified = false;
    int nOldWidth = 0;
    DBG_ASSERT( mnUnitsPerPixel==1, "UniscribeLayout.mnUnitsPerPixel != 1" );
    int i,j;
    for( i = mnMinCharPos, j = 0; i < mnEndCharPos; ++i, ++j )
    {
        int nNewCharWidth = (pDXArray[j] - nOldWidth);
        // TODO: nNewCharWidth *= mnUnitsPerPixel;
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
    long nXOffset = 0;
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.IsEmpty() )
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

            rVisualItem.mnXOffset = nXOffset;
            // update nXOffset
            int nEndGlyphPos;
            if( GetItemSubrange( rVisualItem, i, nEndGlyphPos ) )
                for(; i < nEndGlyphPos; ++i )
                    nXOffset += mpJustifications[ i ];

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
    long nXOffset = 0;
    SCRIPT_CACHE& rScriptCache = GetScriptCache();
    for( int nItem = 0; nItem < mnItemCount; ++nItem )
    {
        VisualItem& rVisualItem = mpVisualItems[ nItem ];
        if( rVisualItem.IsEmpty() )
            continue;

        if( (rVisualItem.mnMinCharPos < mnEndCharPos)
         && (rVisualItem.mnEndCharPos > mnMinCharPos) )
        {
            long nItemWidth = 0;
            for( i = rVisualItem.mnMinCharPos; i < rVisualItem.mnEndCharPos; ++i )
                nItemWidth += mpCharWidths[ i ];
            nItemWidth = (int)((fStretch - 1.0) * nItemWidth + 0.5);

            SCRIPT_FONTPROPERTIES aFontProperties;
            int nMinKashida = 1;
            HRESULT nRC = (*pScriptGetFontProperties)( mhDC, &rScriptCache, &aFontProperties );
            if( !nRC )
                nMinKashida = aFontProperties.iKashidaWidth;

            nRC = (*pScriptJustify) (
                mpVisualAttrs + rVisualItem.mnMinGlyphPos,
                mpGlyphAdvances + rVisualItem.mnMinGlyphPos,
                rVisualItem.mnEndGlyphPos - rVisualItem.mnMinGlyphPos,
                nItemWidth,
                nMinKashida,
                mpJustifications + rVisualItem.mnMinGlyphPos );

            rVisualItem.mnXOffset = nXOffset;
            nXOffset += nItemWidth;
        }
    }
}

#endif // USE_UNISCRIBE

// =======================================================================

SalLayout* WinSalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    WinLayout* pWinLayout = NULL;

    ImplWinFontData& rFontFace      = *mpWinFontData[ nFallbackLevel ];
    ImplWinFontEntry& rFontInstance = *mpWinFontEntry[ nFallbackLevel ];

#ifdef USE_UNISCRIBE
    if( !(rArgs.mnFlags & SAL_LAYOUT_COMPLEX_DISABLED)  // complex text
    &&   (aUspModule || (bUspEnabled && InitUSP())) )   // CTL layout engine
    {
        // script complexity is determined in upper layers
        pWinLayout = new UniscribeLayout( mhDC, rFontFace, rFontInstance );
        // NOTE: it must be guaranteed that the WinSalGraphics lives longer than
        // the created UniscribeLayout, otherwise the data passed into the
        // constructor might become invalid too early
    }
    else
#endif // USE_UNISCRIBE
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

        BYTE eCharSet = ANSI_CHARSET;
        if( mpLogFont )
            eCharSet = mpLogFont->lfCharSet;
        pWinLayout = new SimpleWinLayout( mhDC, eCharSet, rFontFace, rFontInstance );
    }

    return pWinLayout;
}

// =======================================================================

ImplWinFontEntry::ImplWinFontEntry( ImplFontSelectData& rFSD )
:   ImplFontEntry( rFSD ),
    mpKerningPairs( NULL ),
    mnKerningPairs( -1 )
{
#ifdef USE_UNISCRIBE
    maScriptCache = NULL;
#endif // USE_UNISCRIBE
}

// -----------------------------------------------------------------------

ImplWinFontEntry::~ImplWinFontEntry()
{
#ifdef USE_UNISCRIBE
    if( maScriptCache != NULL )
        (*pScriptFreeCache)( &maScriptCache );
#endif // USE_UNISCRIBE
#ifdef GCP_KERN_HACK
    delete[] mpKerningPairs;
#endif // GCP_KERN_HACK
}

// -----------------------------------------------------------------------

bool ImplWinFontEntry::HasKernData() const
{
    return (mnKerningPairs >= 0);
}

// -----------------------------------------------------------------------

void ImplWinFontEntry::SetKernData( int nPairCount, const KERNINGPAIR* pPairData )
{
    mnKerningPairs = nPairCount;
    mpKerningPairs = new KERNINGPAIR[ mnKerningPairs ];
    ::memcpy( mpKerningPairs, (const void*)pPairData, nPairCount*sizeof(KERNINGPAIR) );
}

// -----------------------------------------------------------------------

int ImplWinFontEntry::GetKerning( sal_Unicode cLeft, sal_Unicode cRight ) const
{
    int nKernAmount = 0;
    if( mpKerningPairs )
    {
        const KERNINGPAIR aRefPair = { cLeft, cRight, 0 };
        const KERNINGPAIR* pPair = std::lower_bound( mpKerningPairs,
            mpKerningPairs + mnKerningPairs, aRefPair, ImplCmpKernData );
        if( pPair->wFirst==aRefPair.wFirst && pPair->wSecond==aRefPair.wSecond )
            nKernAmount = pPair->iKernAmount;
    }

    return nKernAmount;
}

// =======================================================================

ImplFontData* ImplWinFontData::Clone() const
{
    ImplFontData* pClone = new ImplWinFontData( *this );
    return pClone;
}

// -----------------------------------------------------------------------

ImplFontEntry* ImplWinFontData::CreateFontInstance( ImplFontSelectData& rFSD ) const
{
    ImplFontEntry* pEntry = new ImplWinFontEntry( rFSD );
    return pEntry;
}

// =======================================================================
