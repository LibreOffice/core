/*************************************************************************
 *
 *  $RCSfile: xestyle.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:35 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif
#pragma hdrstop

// ============================================================================

#ifndef SC_XESTYLE_HXX
#include "xestyle.hxx"
#endif

#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif
#ifndef _SV_FONTCVT_HXX
#include <vcl/fontcvt.hxx>
#endif
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _SVX_ALGITEM_HXX
#include <svx/algitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_ROTMODIT_HXX
#include <svx/rotmodit.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_STLPOOL_HXX
#include "stlpool.hxx"
#endif
#ifndef SC_SCPATATR_HXX
#include "patattr.hxx"
#endif
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
#endif

#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

// PALETTE record - color information =========================================

sal_Int32 lcl_GetColorDistance( const Color& rColor1, const Color& rColor2 )
{
    sal_Int32 nDist = rColor1.GetRed() - rColor2.GetRed();
    nDist *= nDist * 77;
    sal_Int32 nDummy = rColor1.GetGreen() - rColor2.GetGreen();
    nDist += nDummy * nDummy * 151;
    nDummy = rColor1.GetBlue() - rColor2.GetBlue();
    nDist += nDummy * nDummy * 28;
    return nDist;
}

void lcl_SetMixedColor( Color& rDest, const Color& rSrc1, const Color& rSrc2 )
{
    rDest.SetRed( static_cast< sal_uInt8 >( (static_cast< sal_uInt16 >( rSrc1.GetRed() ) + rSrc2.GetRed()) / 2 ) );
    rDest.SetGreen( static_cast< sal_uInt8 >( (static_cast< sal_uInt16 >( rSrc1.GetGreen() ) + rSrc2.GetGreen()) / 2 ) );
    rDest.SetBlue( static_cast< sal_uInt8 >( (static_cast< sal_uInt16 >( rSrc1.GetBlue() ) + rSrc2.GetBlue()) / 2 ) );
}

inline XclExpStream& operator<<( XclExpStream& rStrm, const Color& rColor )
{
    return rStrm << rColor.GetRed() << rColor.GetGreen() << rColor.GetBlue() << sal_uInt8( 0 );
}


// additional classes for color reduction -------------------------------------

/** Maps a color list index to a palette index.
    @descr  Used to remap the color ID data vector from list indexes to palette indexes. */
struct XclRemap
{
    sal_uInt32                  mnPalIndex;     /// Index to palette.
    bool                        mbProcessed;    /// true = List color already processed.

    inline explicit             XclRemap() : mnPalIndex( 0 ), mbProcessed( false ) {}
    inline void                 SetIndex( sal_uInt32 nPalIndex )
                                    { mnPalIndex = nPalIndex; mbProcessed = true; }
};

/** Stores the nearest palette color index of a list color. */
struct XclNearest
{
    sal_uInt32                  mnPalIndex;     /// Index to nearest palette color.
    sal_Int32                   mnDist;         /// Distance to palette color.

    inline explicit             XclNearest() : mnPalIndex( 0 ), mnDist( 0 ) {}
};

typedef ::std::vector< XclRemap >   XclRemapVec;
typedef ::std::vector< XclNearest > XclNearestVec;


// ----------------------------------------------------------------------------

IMPL_FIXEDMEMPOOL_NEWDEL( XclExpPalette::XclListColor, 100, 100 )

XclExpPalette::XclListColor::XclListColor( const Color& rColor, sal_uInt32 nColorId ) :
    maColor( rColor ),
    mnColorId( nColorId ),
    mnWeight( 0 )
{
}

void XclExpPalette::XclListColor::UpdateWeighting( XclColorType eType )
{
    switch( eType )
    {
        case xlColorChartLine:  ++mnWeight;     break;
        case xlColorCellBorder:
        case xlColorChartArea:  mnWeight += 2;  break;
        case xlColorCellText:
        case xlColorChartText:  mnWeight += 10; break;
        case xlColorCellArea:   mnWeight += 20; break;
        case xlColorGrid:       mnWeight += 50; break;
    }
}

void XclExpPalette::XclListColor::Merge( const XclListColor& rColor )
{
    sal_uInt32 nWeight2 = rColor.GetWeighting();
    sal_uInt32 nWeightSum = mnWeight + nWeight2;
    sal_uInt32 nWeightSum2 = nWeightSum / 2;
    sal_uInt32 nDummy;

    nDummy = mnWeight * maColor.GetRed() + nWeight2 * rColor.maColor.GetRed() + nWeightSum2;
    maColor.SetRed( static_cast< sal_uInt8 >( nDummy / nWeightSum ) );
    nDummy = mnWeight * maColor.GetGreen() + nWeight2 * rColor.maColor.GetGreen() + nWeightSum2;
    maColor.SetGreen( static_cast< sal_uInt8 >( nDummy / nWeightSum ) );
    nDummy = mnWeight * maColor.GetBlue() + nWeight2 * rColor.maColor.GetBlue() + nWeightSum2;
    maColor.SetBlue( static_cast< sal_uInt8 >( nDummy / nWeightSum ) );

    mnWeight = nWeightSum;
}


// ----------------------------------------------------------------------------

const sal_uInt32 EXC_PAL_INDEXBASE          = 0xFFFF0000;
const sal_Int32 EXC_PAL_MAXDIST             = 0x7FFFFFFF;

XclExpPalette::XclExpPalette( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_PALETTE ),
    XclDefaultPalette( rRoot.GetBiff() ),
    XclExpRoot( rRoot ),
    mnLastIx( 0 )
{
    InsertColor( Color( COL_BLACK ), xlColorCellText );
    OnChangeBiff();
}

void XclExpPalette::OnChangeBiff()
{
    SetDefaultColors( GetBiff() );
    SetRecSize( GetColorCount() * 4 + 2 );

    // initialize maPalette with default colors
    sal_uInt32 nCount = GetColorCount();
    maPalette.resize( nCount );
    XclPaletteColorVec::iterator aIter = maPalette.begin();
    for( sal_uInt32 nIx = 0; nIx < nCount; ++nIx, ++aIter )
        aIter->SetColor( GetDefColor( GetXclIndex( nIx ) ), true );
}

sal_uInt32 XclExpPalette::InsertColor( const Color& rColor, XclColorType eType, sal_uInt16 nAutoDefault )
{
    if( rColor.GetColor() == COL_AUTO )
        return InsertIndex( nAutoDefault );

    sal_uInt32 nIndex;
    bool bIsEqual;
    SearchListEntry( nIndex, bIsEqual, rColor );

    XclListColor* pEntry = maColorList.GetObject( nIndex );
    if( !pEntry || !bIsEqual )
        pEntry = CreateListEntry( rColor, nIndex );

    pEntry->UpdateWeighting( eType );
    mnLastIx = nIndex;
    return pEntry->GetColorId();
}

sal_uInt32 XclExpPalette::InsertIndex( sal_uInt16 nIndex )
{
    return EXC_PAL_INDEXBASE | nIndex;
}

void XclExpPalette::ReduceColors()
{
// --- build initial color ID data vector (maColorIdDataVec) ---

    sal_uInt32 nCount = maColorList.Count();
    maColorIdDataVec.resize( nCount );
    for( sal_uInt32 nIx = 0; nIx < nCount; ++nIx )
    {
        XclListColor* pListColor = maColorList.GetObject( nIx );
        maColorIdDataVec[ pListColor->GetColorId() ].Set( pListColor->GetColor(), nIx );
    }

// --- loop as long as current color count does not fit into palette of current BIFF ---

    // Color ID data vector (maColorIdDataVec) will map color IDs to color list entries (maColorList)
    while( maColorList.Count() > GetColorCount() )
    {
        // find a list color to remove
        sal_uInt32 nRemove = GetLeastUsedListColor();
        // find its nearest neighbor
        sal_uInt32 nNearest = GetNearestListColor( nRemove );
        // merge both colors to one color, remove one color from list
        MergeListColors( nNearest, nRemove );
    }

// --- #104865# use default palette and replace colors with nearest used colors ---

    nCount = maColorList.Count();
    XclRemapVec aRemapVec( nCount );
    XclNearestVec aNearestVec( nCount );

    // in each run: search the best fitting color and replace a default color with it
    for( sal_uInt32 nRun = 0; nRun < nCount; ++nRun )
    {
        sal_uInt32 nIndex;
        // find nearest unused default color for each unprocessed list color
        for( nIndex = 0; nIndex < nCount; ++nIndex )
            aNearestVec[ nIndex ].mnDist = aRemapVec[ nIndex ].mbProcessed ? EXC_PAL_MAXDIST :
                GetNearestPaletteColor( aNearestVec[ nIndex ].mnPalIndex, maColorList.GetObject( nIndex )->GetColor(), true );
        // find the list color which is nearest to a default color
        sal_uInt32 nFound = 0;
        for( nIndex = 1; nIndex < nCount; ++nIndex )
            if( aNearestVec[ nIndex ].mnDist < aNearestVec[ nFound ].mnDist )
                nFound = nIndex;
        // replace default color with list color
        sal_uInt32 nNearest = aNearestVec[ nFound ].mnPalIndex;
        DBG_ASSERT( maColorList.GetObject( nFound ), "XclExpPalette::ReduceColors - missing a color" );
        DBG_ASSERT( nNearest < maPalette.size(), "XclExpPalette::ReduceColors - algorithm error" );
        maPalette[ nNearest ].SetColor( maColorList.GetObject( nFound )->GetColor() );
        aRemapVec[ nFound ].SetIndex( nNearest );
    }

    // remap color ID data map (maColorIdDataVec) from list indexes to palette indexes
    for( XclColorIdDataVec::iterator aIter = maColorIdDataVec.begin(), aEnd = maColorIdDataVec.end();
            aIter != aEnd; ++aIter )
        aIter->mnIndex = aRemapVec[ aIter->mnIndex ].mnPalIndex;
}

sal_uInt16 XclExpPalette::GetColorIndex( sal_uInt32 nColorId ) const
{
    sal_uInt16 nRet = 0;
    if( nColorId >= EXC_PAL_INDEXBASE )
        nRet = static_cast< sal_uInt16 >( nColorId & ~EXC_PAL_INDEXBASE );
    else if( nColorId < maColorIdDataVec.size() )
        nRet = GetXclIndex( maColorIdDataVec[ nColorId ].mnIndex );
    return nRet;
}

sal_uInt16 XclExpPalette::GetColorIndex( const Color& rColor, sal_uInt16 nAutoDefault ) const
{
    if( rColor.GetColor() == COL_AUTO )
        return nAutoDefault;

    sal_uInt32 nIndex;
    GetNearestPaletteColor( nIndex, rColor, false );
    return GetXclIndex( nIndex );
}

void XclExpPalette::GetMixedColors(
        sal_uInt16& rnXclForeIx, sal_uInt16& rnXclBackIx, sal_uInt8& rnXclPattern,
        sal_uInt32 nForeColorId, sal_uInt32 nBackColorId ) const
{
    if( (nForeColorId >= EXC_PAL_INDEXBASE) || (nBackColorId >= EXC_PAL_INDEXBASE) || (rnXclPattern != EXC_PATT_SOLID) )
    {
        rnXclForeIx = GetColorIndex( nForeColorId );
        rnXclBackIx = GetColorIndex( nBackColorId );
        return;
    }

    rnXclForeIx = rnXclBackIx = 0;
    if( nForeColorId >= maColorIdDataVec.size() )
        return;

    sal_uInt32 nIndex1, nIndex2;
    Color aForeColor( GetOriginalColor( nForeColorId ) );
    sal_Int32 nFirstDist = GetNearPaletteColors( nIndex1, nIndex2, aForeColor );
    if( (nIndex1 >= maPalette.size()) || (nIndex2 >= maPalette.size()) )
        return;

    Color aColorArr[ 5 ];
    aColorArr[ 0 ] = maPalette[ nIndex1 ].maColor;
    aColorArr[ 4 ] = maPalette[ nIndex2 ].maColor;
    lcl_SetMixedColor( aColorArr[ 2 ], aColorArr[ 0 ], aColorArr[ 4 ] );
    lcl_SetMixedColor( aColorArr[ 1 ], aColorArr[ 0 ], aColorArr[ 2 ] );
    lcl_SetMixedColor( aColorArr[ 3 ], aColorArr[ 2 ], aColorArr[ 4 ] );

    sal_Int32 nMinDist = nFirstDist;
    sal_uInt32 nMinIndex = 0;
    for( sal_uInt32 nCnt = 1; nCnt < 4; ++nCnt )
    {
        sal_Int32 nDist = lcl_GetColorDistance( aForeColor, aColorArr[ nCnt ] );
        if( nDist < nMinDist )
        {
            nMinDist = nDist;
            nMinIndex = nCnt;
        }
    }
    rnXclForeIx = GetXclIndex( nIndex1 );
    rnXclBackIx = GetXclIndex( nIndex2 );
    if( nMinDist < nFirstDist )
    {
        switch( nMinIndex )
        {
            case 1: rnXclPattern = EXC_PATT_75_PERC;    break;
            case 2: rnXclPattern = EXC_PATT_50_PERC;    break;
            case 3: rnXclPattern = EXC_PATT_25_PERC;    break;
        }
    }
}

ColorData XclExpPalette::GetColorData( sal_uInt16 nXclIndex, ColorData nDefault ) const
{
    if( (nXclIndex >= GetIndexOffset()) && (nXclIndex - GetIndexOffset() < maPalette.size()) )
        return maPalette[ nXclIndex - GetIndexOffset() ].maColor.GetColor();
    return GetDefColorData( nXclIndex, nDefault );
}

const Color& XclExpPalette::GetOriginalColor( sal_uInt32 nColorId ) const
{
    if( nColorId < maColorIdDataVec.size() )
        return maColorIdDataVec[ nColorId ].maColor;
    return maPalette[ 0 ].maColor;
}

void XclExpPalette::SearchListEntry( sal_uInt32& rnIndex, bool& rbIsEqual, const Color& rColor ) const
{
    rbIsEqual = false;
    rnIndex = 0;
    if( maColorList.Empty() )
        return;

    // search optimization for equal-colored objects occuring repeatedly
    XclListColor* pEntry = maColorList.GetObject( mnLastIx );
    if( pEntry && pEntry->IsEqual( rColor ) )
    {
        rnIndex = mnLastIx;
        rbIsEqual = true;
        return;
    }

    sal_uInt32 nFirstIx = 0;
    sal_uInt32 nLastIx = maColorList.Count() - 1;
    sal_uInt32 nNewIx;

    while( !rbIsEqual && (nFirstIx <= nLastIx) )
    {
        nNewIx = (nFirstIx + nLastIx) / 2;
        pEntry = maColorList.GetObject( nNewIx );
        rbIsEqual = pEntry->IsEqual( rColor );
        if( !rbIsEqual )
        {
            if( pEntry->IsGreater( rColor ) )
                nLastIx = nNewIx - 1;
            else
            {
                nFirstIx = nNewIx + 1;
                if( nFirstIx > nLastIx )
                    ++nNewIx;
            }
        }
    }
    rnIndex = nNewIx;
}

XclExpPalette::XclListColor* XclExpPalette::CreateListEntry( const Color& rColor, sal_uInt32 nIndex )
{
    XclListColor* pColor = new XclListColor( rColor, maColorList.Count() );
    maColorList.Insert( pColor, nIndex );
    return pColor;
}

void XclExpPalette::MergeListColors( sal_uInt32 nKeep, sal_uInt32 nRemove )
{
    if( nRemove == 0 )      // don't remove color 0
        ::std::swap( nKeep, nRemove );

    XclListColor* pKeepEntry = maColorList.GetObject( nKeep );
    XclListColor* pRemoveEntry = maColorList.GetObject( nRemove );
    if( pKeepEntry && pRemoveEntry )
    {
        if( nKeep == 0 )
            pKeepEntry->AddWeighting( *pRemoveEntry );
        else
            pKeepEntry->Merge( *pRemoveEntry );
        maColorList.Delete( nRemove );

        // recalculate color ID data map (maps color IDs to color list indexes)
        XclColorIdDataVec::iterator aEnd = maColorIdDataVec.end();
        for( XclColorIdDataVec::iterator aIter = maColorIdDataVec.begin(); aIter != aEnd; ++aIter )
        {
            if( aIter->mnIndex == nRemove ) aIter->mnIndex = nKeep;
            // not "else if", aIter->mnIndex may become greater than nRemove in previous op.
            if( aIter->mnIndex > nRemove ) --aIter->mnIndex;
        }
    }
}

sal_uInt32 XclExpPalette::GetLeastUsedListColor() const
{
    sal_uInt32 nFound = 0;
    sal_uInt32 nCount = maColorList.Count();
    sal_uInt32 nMinW = 0xFFFFFFFF;

    for( sal_uInt32 nIx = 0; nIx < nCount; ++nIx )
    {
        XclListColor* pEntry = maColorList.GetObject( nIx );
        if( pEntry->GetWeighting() < nMinW )
        {
            nFound = nIx;
            nMinW = pEntry->GetWeighting();
        }
    }
    return nFound;
}

sal_uInt32 XclExpPalette::GetNearestListColor( const Color& rColor, sal_uInt32 nIgnore ) const
{
    sal_uInt32 nFound = 0;
    sal_uInt32 nCount = maColorList.Count();
    sal_Int32 nMinD = EXC_PAL_MAXDIST;

    for( sal_uInt32 nIx = 0; nIx < maColorList.Count(); ++nIx )
    {
        if( nIx != nIgnore )
        {
            XclListColor* pCheck = maColorList.GetObject( nIx );
            if( pCheck )
            {
                sal_Int32 nDist = lcl_GetColorDistance( rColor, pCheck->GetColor() );
                if( nDist < nMinD )
                {
                    nFound = nIx;
                    nMinD = nDist;
                }
            }
        }
    }
    return nFound;
}

sal_uInt32 XclExpPalette::GetNearestListColor( sal_uInt32 nIndex ) const
{
    XclListColor* pEntry = maColorList.GetObject( nIndex );
    return pEntry ? GetNearestListColor( pEntry->GetColor(), nIndex ) : 0;
}

sal_Int32 XclExpPalette::GetNearestPaletteColor(
        sal_uInt32& rnIndex, const Color& rColor, bool bDefaultOnly ) const
{
    rnIndex = 0;
    sal_Int32 nDist = EXC_PAL_MAXDIST;

    for( XclPaletteColorVec::const_iterator aIter = maPalette.begin(), aEnd = maPalette.end();
            aIter != aEnd; ++aIter )
    {
        if( !bDefaultOnly || aIter->mbDefault )
        {
            sal_Int32 nCurrDist = lcl_GetColorDistance( rColor, aIter->maColor );
            if( nCurrDist < nDist )
            {
                rnIndex = aIter - maPalette.begin();
                nDist = nCurrDist;
            }
        }
    }
    return nDist;
}

sal_Int32 XclExpPalette::GetNearPaletteColors(
        sal_uInt32& rnFirst, sal_uInt32& rnSecond, const Color& rColor ) const
{
    rnFirst = rnSecond = 0;
    sal_Int32 nDist1 = EXC_PAL_MAXDIST;
    sal_Int32 nDist2 = EXC_PAL_MAXDIST;

    for( XclPaletteColorVec::const_iterator aIter = maPalette.begin(), aEnd = maPalette.end();
            aIter != aEnd; ++aIter )
    {
        sal_Int32 nCurrDist = lcl_GetColorDistance( rColor, aIter->maColor );
        if( nCurrDist < nDist1 )
        {
            rnSecond = rnFirst;
            nDist2 = nDist1;
            rnFirst = aIter - maPalette.begin();
            nDist1 = nCurrDist;
        }
        else if( nCurrDist < nDist2 )
        {
            rnSecond = aIter - maPalette.begin();
            nDist2 = nCurrDist;
        }
    }
    return nDist1;
}

void XclExpPalette::WriteBody( XclExpStream& rStrm )
{
    rStrm << static_cast< sal_uInt16 >( maPalette.size() );
    for( XclPaletteColorVec::const_iterator aIter = maPalette.begin(), aEnd = maPalette.end();
            aIter != aEnd; ++aIter )
        rStrm << aIter->maColor;
}


// FONT record - font information =============================================

inline XclExpStream& operator<<( XclExpStream& rStrm, XclUnderline eUnderl )
{
    return rStrm << static_cast< sal_uInt8 >( eUnderl );
}

inline XclExpStream& operator<<( XclExpStream& rStrm, XclEscapement eEscapem )
{
    return rStrm << static_cast< sal_uInt16 >( eEscapem );
};


// ----------------------------------------------------------------------------

XclExpFont::XclExpFont( const XclExpRoot& rRoot ) :
    XclExpRecord( EXC_ID_FONT, 14 ),
    XclExpRoot( rRoot ),
    mnColorId( rRoot.GetPalette().InsertIndex( EXC_FONT_AUTOCOLOR ) ),
    mnHash( 0 ),
    mbHasColor( false )
{
}

void XclExpFont::SetName( const String& rName )
{
    // #106246# substitute with MS fonts
    String aSubstName( GetSubsFontName( rName, SUBSFONT_ONLYONE | SUBSFONT_MS ) );
    maData.maName = aSubstName.Len() ? aSubstName : rName;
    sal_uInt32 nRecSize = maData.maName.Len();
    if( GetBiff() >= xlBiff8 )
        (nRecSize *= 2) += 1;
    nRecSize += 15;
    SetRecSize( nRecSize );
}

void XclExpFont::SetColor( const Color& rColor )
{
    SetColorId( GetPalette().InsertColor( rColor, xlColorCellText, EXC_FONT_AUTOCOLOR ) );
}

void XclExpFont::SetColorId( sal_uInt32 nColorId )
{
    mnColorId = nColorId;
    mbHasColor = true;
}

void XclExpFont::SetHeight( sal_Int32 nTwips )
{
    maData.mnHeight = static_cast< sal_uInt16 >( ::std::min( nTwips, 0x7FFFL ) );
}

void XclExpFont::SetFont( const Font& rFont )
{
    SetName( rFont.GetName() );
    SetFamily( rFont.GetFamily() );
    SetCharSet( rFont.GetCharSet() );
    SetColor( rFont.GetColor() );
    SetHeight( rFont.GetSize().Height() );
    SetWeight( rFont.GetWeight() );
    SetUnderline( rFont.GetUnderline() );
    SetItalic( rFont.GetItalic() != ITALIC_NONE );
    SetStrikeout( rFont.GetStrikeout() != STRIKEOUT_NONE );
    SetOutline( !!rFont.IsOutline() );  // BOOL->bool
    SetShadow( !!rFont.IsShadow() );    // BOOL->bool
    CalcHash();
}

void XclExpFont::CalcHash()
{
    mnHash = maData.maName.Len();
    mnHash += maData.mnWeight * 3;
    mnHash += maData.mnCharSet * 5;
    mnHash += maData.mnFamily * 7;
    mnHash += maData.mnHeight * 11;
    mnHash += static_cast< sal_uInt32 >( maData.meUnderline ) * 13;
    mnHash += static_cast< sal_uInt32 >( maData.meEscapem ) * 17;
    if( maData.mbItalic ) mnHash += 19;
    if( maData.mbStrikeout ) mnHash += 23;
    if( maData.mbOutline ) mnHash += 29;
    if( maData.mbShadow ) mnHash += 31;
}

bool XclExpFont::operator==( const XclExpFont& rCmp ) const
{
    return
        (mnHash == rCmp.mnHash) &&
        (!mbHasColor || !rCmp.mbHasColor || (mnColorId == rCmp.mnColorId)) &&
        (maData.mnHeight == rCmp.maData.mnHeight) &&
        (maData.mnWeight == rCmp.maData.mnWeight) &&
        (maData.meUnderline == rCmp.maData.meUnderline) &&
        (maData.meEscapem == rCmp.maData.meEscapem) &&
        (maData.mnFamily == rCmp.maData.mnFamily) &&
        (maData.mnCharSet == rCmp.maData.mnCharSet) &&
        (maData.mbItalic == rCmp.maData.mbItalic) &&
        (maData.mbStrikeout == rCmp.maData.mbStrikeout) &&
        (maData.mbOutline == rCmp.maData.mbOutline) &&
        (maData.mbShadow == rCmp.maData.mbShadow) &&
        (maData.maName == rCmp.maData.maName);
}

sal_uInt8 XclExpFont::GetXclFamily( FontFamily eFamily )
{
    switch( eFamily )
    {
        case FAMILY_DONTKNOW:   return EXC_FONTFAM_DONTKNOW;
        case FAMILY_DECORATIVE: return EXC_FONTFAM_DECORATIVE;
        case FAMILY_MODERN:     return EXC_FONTFAM_MODERN;
        case FAMILY_ROMAN:      return EXC_FONTFAM_ROMAN;
        case FAMILY_SCRIPT:     return EXC_FONTFAM_SCRIPT;
        case FAMILY_SWISS:      return EXC_FONTFAM_SWISS;
        case FAMILY_SYSTEM:     return EXC_FONTFAM_SYSTEM;
    }
    DBG_ERRORFILE( "XclExpFont::GetXclFamily - unknown font family" );
    return EXC_FONTFAM_DONTKNOW;
}

sal_uInt8 XclExpFont::GetXclCharSet( rtl_TextEncoding eCharSet )
{
    return rtl_getBestWindowsCharsetFromTextEncoding(eCharSet);
}

sal_uInt16 XclExpFont::GetXclWeight( FontWeight eWeight )
{
    switch( eWeight )
    {
        case WEIGHT_DONTKNOW:   return EXC_FONTWGHT_DONTKNOW;
        case WEIGHT_THIN:       return EXC_FONTWGHT_THIN;
        case WEIGHT_ULTRALIGHT: return EXC_FONTWGHT_ULTRALIGHT;
        case WEIGHT_LIGHT:      return EXC_FONTWGHT_LIGHT;
        case WEIGHT_SEMILIGHT:  return EXC_FONTWGHT_SEMILIGHT;
        case WEIGHT_NORMAL:     return EXC_FONTWGHT_NORMAL;
        case WEIGHT_MEDIUM:     return EXC_FONTWGHT_MEDIUM;
        case WEIGHT_SEMIBOLD:   return EXC_FONTWGHT_SEMIBOLD;
        case WEIGHT_BOLD:       return EXC_FONTWGHT_BOLD;
        case WEIGHT_ULTRABOLD:  return EXC_FONTWGHT_ULTRABOLD;
        case WEIGHT_BLACK:      return EXC_FONTWGHT_BLACK;
    }
    return EXC_FONTWGHT_NORMAL;
}

XclUnderline XclExpFont::GetXclUnderline( FontUnderline eUnderl )
{
    switch( eUnderl )
    {
        case UNDERLINE_SINGLE:  return xlUnderlSingle;
        case UNDERLINE_DOUBLE:  return xlUnderlDouble;
        case UNDERLINE_DOTTED:  return xlUnderlSingleAcc;
    }
    return xlUnderlNone;
}

XclEscapement XclExpFont::GetXclEscapement( SvxEscapement eEscapem )
{
    switch( eEscapem )
    {
        case SVX_ESCAPEMENT_SUPERSCRIPT:    return xlEscSuper;
        case SVX_ESCAPEMENT_SUBSCRIPT:      return xlEscSub;
    }
    return xlEscNone;
}

void XclExpFont::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nAttr = EXC_FONTATTR_NONE;
    ::set_flag( nAttr, EXC_FONTATTR_ITALIC, maData.mbItalic );
    ::set_flag( nAttr, EXC_FONTATTR_STRIKEOUT, maData.mbStrikeout );
    ::set_flag( nAttr, EXC_FONTATTR_OUTLINE, maData.mbOutline );
    ::set_flag( nAttr, EXC_FONTATTR_SHADOW, maData.mbShadow );

    DBG_ASSERT( maData.maName < 256, "XclExpFont::WriteBody - font name too long" );
    XclExpString aFontName;
    if( GetBiff() < xlBiff8 )
        aFontName.AssignByte( maData.maName, GetCharSet(), EXC_STR_8BITLENGTH );
    else
        aFontName.Assign( maData.maName, EXC_STR_FORCEUNICODE | EXC_STR_8BITLENGTH );

    rStrm   << maData.mnHeight
            << nAttr
            << GetPalette().GetColorIndex( mnColorId )
            << maData.mnWeight
            << maData.meEscapem
            << maData.meUnderline
            << maData.mnFamily
            << maData.mnCharSet
            << sal_uInt8( 0 )
            << aFontName;
}


// ----------------------------------------------------------------------------

XclExpFontBuffer::XclExpFontBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnXclMaxCount( 0 )
{
    OnChangeBiff();

    // currently always using Arial/10
    SetCharWidth( 110 );
}

void XclExpFontBuffer::OnChangeBiff()
{
    DBG_ASSERT( maFontList.Empty(), "XclExpFontBuffer::OnChangeBiff - call is too late" );
    switch( GetBiff() )
    {
        case xlBiff4:   mnXclMaxCount = EXC_FONT_MAXCOUNT4; break;
        case xlBiff5:
        case xlBiff7:   mnXclMaxCount = EXC_FONT_MAXCOUNT5; break;
        case xlBiff8:   mnXclMaxCount = EXC_FONT_MAXCOUNT8; break;
        default:        DBG_ERROR_BIFF();
    }
}

sal_uInt16 XclExpFontBuffer::GetIndex( const XclExpFont& rFont, sal_uInt16 nDefault )
{
    sal_uInt32 nIndex = Find( rFont );
    return (nIndex < mnXclMaxCount) ? GetXclIndex( nIndex ) : nDefault;
}

sal_uInt16 XclExpFontBuffer::Insert( XclExpFont*& rpFont )
{
    DBG_ASSERT( rpFont, "XclExpFontBuffer::Insert - no font" );
    sal_uInt32 nIndex = 0;
    if( rpFont )
    {
        nIndex = Find( *rpFont );
        sal_uInt32 nCount = maFontList.Count();
        if( nIndex < nCount )
        {
            // --- font already in buffer ---
            XclExpFont* pOldFont = maFontList.GetObject( nIndex );
            // old font does not have a color -> take color from new font
            if( !pOldFont->HasColor() )
                pOldFont->SetColorId( rpFont->GetColorId() );
            delete rpFont;
            rpFont = pOldFont;
        }
        else if( nCount < mnXclMaxCount )
        {
            // --- font not found, possible to insert ---
            maFontList.Append( rpFont );
            nIndex = nCount;    // old count is last index now
        }
        else
        {
            // --- font not found, buffer is full ---
            delete rpFont;
            rpFont = maFontList.GetObject( 0 );
            nIndex = 0;
        }
    }
    return GetXclIndex( nIndex );
}

sal_uInt16 XclExpFontBuffer::Insert( const Font& rFont )
{
    XclExpFont* pNewFont = new XclExpFont( GetRoot() );
    pNewFont->SetFont( rFont );
    return Insert( pNewFont );
}

sal_uInt16 XclExpFontBuffer::Insert( const ScPatternAttr& rAttr )
{
    Font aFont;
    rAttr.GetFont( aFont, SC_AUTOCOL_RAW );
    return Insert( aFont );
}

void XclExpFontBuffer::Save( XclExpStream& rStrm )
{
    if( maFontList.Empty() )
        InitDefaultFonts();
    maFontList.Save( rStrm );
}

void XclExpFontBuffer::InitDefaultFonts()
{
    XclExpFont aFont( GetRoot() );
    aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Arial" ) ) );
    aFont.SetFamily( FAMILY_DONTKNOW );
    aFont.SetCharSet( RTL_TEXTENCODING_DONTKNOW );
    aFont.SetColor( Color( COL_AUTO ) );
    aFont.SetHeight( 200 );
    aFont.SetWeight( WEIGHT_NORMAL );

    switch( GetBiff() )
    {
        case xlBiff5:
        case xlBiff7:
            maFontList.Append( new XclExpFont( aFont ) );
            aFont.SetWeight( WEIGHT_BOLD );
            maFontList.Append( new XclExpFont( aFont ) );
            aFont.SetWeight( WEIGHT_NORMAL );
            aFont.SetItalic( true );
            maFontList.Append( new XclExpFont( aFont ) );
            aFont.SetWeight( WEIGHT_BOLD );
            maFontList.Append( new XclExpFont( aFont ) );
            aFont.SetWeight( WEIGHT_NORMAL );
            aFont.SetItalic( false );
            maFontList.Append( new XclExpFont( aFont ) );
        break;
        case xlBiff8:
            maFontList.Append( new XclExpFont( aFont ) );
            maFontList.Append( new XclExpFont( aFont ) );
            maFontList.Append( new XclExpFont( aFont ) );
            maFontList.Append( new XclExpFont( aFont ) );
        break;
        default:
            DBG_ERROR_BIFF();
    }
}

sal_uInt16 XclExpFontBuffer::GetXclIndex( sal_uInt32 nIndex ) const
{
    if( nIndex > 3 )
        ++nIndex;
    return static_cast< sal_uInt16 >( nIndex );
}

sal_uInt32 XclExpFontBuffer::Find( const XclExpFont& rFont )
{
    if( maFontList.Empty() )
        InitDefaultFonts();

    sal_uInt32 nCount = maFontList.Count();
    for( sal_uInt32 nFontIx = 0; nFontIx < nCount; ++nFontIx )
        if( *maFontList.GetObject( nFontIx ) == rFont )
            return nFontIx;
    return LIST_ENTRY_NOTFOUND;
}


// FORMAT record - number formats =============================================

/** Data for a default number format. */
struct XclExpDefaultFormat
{
    sal_uInt16                  mnIndex;        /// Excel index of the format.
    const sal_Char*             mpFormat;       /// Format string.
};

static const XclExpDefaultFormat pDefaultFormats[] =
{
    {   0x05,   "#,##0\\ \"€\";\\-#,##0\\ \"€\""                                                },
    {   0x06,   "#,##0\\ \"€\";[Red]\\-#,##0\\ \"€\""                                           },
    {   0x07,   "#,##0.00\\ \"€\";\\-#,##0.00\\ \"€\""                                          },
    {   0x08,   "#,##0.00\\ \"€\";[Red]\\-#,##0.00\\ \"€\""                                     },
    {   0x2A,   "_-* #,##0\\ \"€\"_-;\\-* #,##0\\ \"€\"_-;_-* \"-\"\\ \"€\"_-;_-@_-"          },
    {   0x29,   "_-* #,##0\\ _€_-;\\-* #,##0\\ _€_-;_-* \"-\"\\ _€_-;_-@_-"                   },
    {   0x2C,   "_-* #,##0.00\\ \"€\"_-;\\-* #,##0.00\\ \"€\"_-;_-* \"-\"??\\ \"€\"_-;_-@_-"  },
    {   0x2B,   "_-* #,##0.00\\ _€_-;\\-* #,##0.00\\ _€_-;_-* \"-\"??\\ _€_-;_-@_-"           }
};

/** Predicate for search algorithm. */
struct XclExpNumFmtPred
{
    sal_uInt32                  mnFormatIx;
    inline explicit             XclExpNumFmtPred( sal_uInt32 nFormatIx ) : mnFormatIx( nFormatIx ) {}
    inline bool                 operator()( const XclExpNumFmt& rFormat )
                                    { return rFormat.mnFormatIx == mnFormatIx; }
};


// ----------------------------------------------------------------------------

XclExpNumFmtBuffer::XclExpNumFmtBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    /*  Compiler needs a hint, this doesn't work: new NfKeywordTable;
        cannot convert from 'class String *' to 'class String (*)[54]'
        The effective result here is class String (*)[54*1] */
    mpKeywordTable( new NfKeywordTable[ 1 ] ),
    mpFormatter( new SvNumberFormatter( rRoot.GetDoc().GetServiceManager(), LANGUAGE_ENGLISH_US ) ),
    mnStdFmt( GetFormatter().GetStandardFormat( GetDefLanguage() ) )
{
    OnChangeBiff();

    mpFormatter->FillKeywordTable( *mpKeywordTable, LANGUAGE_ENGLISH_US );
    // remap codes unknown to Excel
    (*mpKeywordTable)[ NF_KEY_NN ] = String( RTL_CONSTASCII_USTRINGPARAM( "DDD" ) );
    (*mpKeywordTable)[ NF_KEY_NNN ] = String( RTL_CONSTASCII_USTRINGPARAM( "DDDD" ) );
    // NNNN gets a separator appended in SvNumberformat::GetMappedFormatString()
    (*mpKeywordTable)[ NF_KEY_NNNN ] = String( RTL_CONSTASCII_USTRINGPARAM( "DDDD" ) );
}

XclExpNumFmtBuffer::~XclExpNumFmtBuffer()
{
    delete[] mpKeywordTable;
}

void XclExpNumFmtBuffer::OnChangeBiff()
{
    DBG_ASSERT( maFormatMap.empty(), "XclExpNumFmtBuffer::OnChangeBiff - call is too late" );
    switch( GetBiff() )
    {
        case xlBiff5:
        case xlBiff7:   mnXclOffset = EXC_FORMAT_OFFSET5;   break;
        case xlBiff8:   mnXclOffset = EXC_FORMAT_OFFSET8;   break;
        default:        DBG_ERROR_BIFF();
    }
}

sal_uInt16 XclExpNumFmtBuffer::Insert( sal_uInt32 nScFormatIx )
{
    XclExpNumFmtVec::const_iterator aIter =
        ::std::find_if( maFormatMap.begin(), maFormatMap.end(), XclExpNumFmtPred( nScFormatIx ) );
    if( aIter != maFormatMap.end() )
        return aIter->mnXclIx;

    sal_uInt32 nSize = maFormatMap.size();
    if( nSize < 0xFFFFUL - mnXclOffset )
    {
        sal_uInt16 nXclIx = static_cast< sal_uInt16 >( nSize + mnXclOffset );
        maFormatMap.push_back( XclExpNumFmt( nScFormatIx, nXclIx ) );
        return nXclIx;
    }

    return 0;
}

void XclExpNumFmtBuffer::Save( XclExpStream& rStrm )
{
    WriteDefaultFormats( rStrm );
    for( XclExpNumFmtVec::const_iterator aIter = maFormatMap.begin(), aEnd = maFormatMap.end(); aIter != aEnd; ++aIter )
        WriteFormatRecord( rStrm, *aIter );
}

void XclExpNumFmtBuffer::WriteFormatRecord( XclExpStream& rStrm, sal_uInt16 nXclIx, const String& rFormatStr )
{
    XclExpString aExpStr;
    if( GetBiff() < xlBiff8 )
        aExpStr.AssignByte( rFormatStr, GetCharSet(), EXC_STR_8BITLENGTH );
    else
        aExpStr.Assign( rFormatStr );

    rStrm.StartRecord( EXC_ID_FORMAT, 2 + aExpStr.GetSize() );
    rStrm << nXclIx << aExpStr;
    rStrm.EndRecord();
}

void XclExpNumFmtBuffer::WriteFormatRecord( XclExpStream& rStrm, const XclExpNumFmt& rFormat )
{
    const SvNumberformat* pEntry = GetFormatter().GetEntry( rFormat.mnFormatIx );
    String aFormatStr;

    if( pEntry )
    {
        if( pEntry->GetType() == NUMBERFORMAT_LOGICAL )
        {
            // build Boolean number format
            Color* pColor = NULL;
            String aTemp;
            const_cast< SvNumberformat* >( pEntry )->GetOutputString( 1.0, aTemp, &pColor );
            aFormatStr.Append( '"' ).Append( aTemp ).AppendAscii( "\";\"" ).Append( aTemp ).AppendAscii( "\";\"" );
            const_cast< SvNumberformat* >( pEntry )->GetOutputString( 0.0, aTemp, &pColor );
            aFormatStr.Append( aTemp ).Append( '"' );
        }
        else
        {
            LanguageType eLang = pEntry->GetLanguage();
            if( eLang != LANGUAGE_ENGLISH_US )
            {
                xub_StrLen nCheckPos;
                sal_Int16 nType = NUMBERFORMAT_DEFINED;
                sal_uInt32 nKey;
                String aTemp( pEntry->GetFormatstring() );
                mpFormatter->PutandConvertEntry( aTemp, nCheckPos, nType, nKey, eLang, LANGUAGE_ENGLISH_US );
                DBG_ASSERT( nCheckPos == 0, "XclExpNumFmtBuffer::WriteFormatRecord - format code not convertible" );
                pEntry = mpFormatter->GetEntry( nKey );
            }

            aFormatStr = pEntry->GetMappedFormatstring( *mpKeywordTable, *mpFormatter->GetLocaleData() );
            if( aFormatStr.EqualsAscii( "Standard" ) )
                aFormatStr.AssignAscii( "General" );
        }
    }
    else
    {
        DBG_ERRORFILE( "XclExpNumFmtBuffer::WriteFormatRecord - format not found" );
        aFormatStr.AssignAscii( "General" );
    }

    WriteFormatRecord( rStrm, rFormat.mnXclIx, aFormatStr );
}

void XclExpNumFmtBuffer::WriteDefaultFormats( XclExpStream& rStrm )
{
    const XclExpDefaultFormat* pEnd = STATIC_TABLE_END( pDefaultFormats );
    for( const XclExpDefaultFormat* pCurr = pDefaultFormats; pCurr != pEnd; ++pCurr )
        WriteFormatRecord( rStrm, pCurr->mnIndex, String( pCurr->mpFormat, RTL_TEXTENCODING_UTF8 ) );
}


// XF, STYLE record - Cell formatting =========================================

XclExpXFBorder::XclExpXFBorder() :
    mnLeftColorId( 0 ),
    mnRightColorId( 0 ),
    mnTopColorId( 0 ),
    mnBottomColorId( 0 ),
    mnLeftLine( EXC_LINE_NONE ),
    mnRightLine( EXC_LINE_NONE ),
    mnTopLine( EXC_LINE_NONE ),
    mnBottomLine( EXC_LINE_NONE )
{
}

#if 0
void lcl_GetBorder2( sal_uInt8& rnFlags, const XclExpXFBorder& rBorder )
{
    ::set_flag( rnFlags, EXC_XF2_LEFTLINE,   rBorder.mnLeftLine   != EXC_LINE_NONE );
    ::set_flag( rnFlags, EXC_XF2_RIGHTLINE,  rBorder.mnRightLine  != EXC_LINE_NONE );
    ::set_flag( rnFlags, EXC_XF2_TOPLINE,    rBorder.mnTopLine    != EXC_LINE_NONE );
    ::set_flag( rnFlags, EXC_XF2_BOTTOMLINE, rBorder.mnBottomLine != EXC_LINE_NONE );
}

void lcl_GetBorder3( sal_uInt32& rnFlags, const XclExpXFBorder& rBorder, const XclExpPalette& rPalette )
{
    ::insert_value( rnFlags, rBorder.mnTopLine,     0, 3 );
    ::insert_value( rnFlags, rBorder.mnLeftLine,    8, 3 );
    ::insert_value( rnFlags, rBorder.mnBottomLine, 16, 3 );
    ::insert_value( rnFlags, rBorder.mnRightLine,  24, 3 );
    ::insert_value( rnFlags, rPalette.GetColorIndex( rBorder.mnTopColorId ),     3, 5 );
    ::insert_value( rnFlags, rPalette.GetColorIndex( rBorder.mnLeftColorId ),   11, 5 );
    ::insert_value( rnFlags, rPalette.GetColorIndex( rBorder.mnBottomColorId ), 19, 5 );
    ::insert_value( rnFlags, rPalette.GetColorIndex( rBorder.mnRightColorId ),  27, 5 );
}
#endif

void lcl_GetBorder5(
        sal_uInt32& rnBorder, sal_uInt32& rnArea,
        const XclExpXFBorder& rBorder, const XclExpPalette& rPalette )
{
    ::insert_value( rnBorder, rBorder.mnTopLine,     0, 3 );
    ::insert_value( rnBorder, rBorder.mnLeftLine,    3, 3 );
    ::insert_value( rnArea,   rBorder.mnBottomLine, 22, 3 );
    ::insert_value( rnBorder, rBorder.mnRightLine,   6, 3 );
    ::insert_value( rnBorder, rPalette.GetColorIndex( rBorder.mnTopColorId ),     9, 7 );
    ::insert_value( rnBorder, rPalette.GetColorIndex( rBorder.mnLeftColorId ),   16, 7 );
    ::insert_value( rnArea,   rPalette.GetColorIndex( rBorder.mnBottomColorId ), 25, 7 );
    ::insert_value( rnBorder, rPalette.GetColorIndex( rBorder.mnRightColorId ),  23, 7 );
}

void lcl_GetBorder8(
        sal_uInt32& rnBorder1, sal_uInt32& rnBorder2,
        const XclExpXFBorder& rBorder, const XclExpPalette& rPalette )
{
    ::insert_value( rnBorder1, rBorder.mnLeftLine,    0, 4 );
    ::insert_value( rnBorder1, rBorder.mnRightLine,   4, 4 );
    ::insert_value( rnBorder1, rBorder.mnTopLine,     8, 4 );
    ::insert_value( rnBorder1, rBorder.mnBottomLine, 12, 4 );
    ::insert_value( rnBorder1, rPalette.GetColorIndex( rBorder.mnLeftColorId ),   16, 7 );
    ::insert_value( rnBorder1, rPalette.GetColorIndex( rBorder.mnRightColorId ),  23, 7 );
    ::insert_value( rnBorder2, rPalette.GetColorIndex( rBorder.mnTopColorId ),     0, 7 );
    ::insert_value( rnBorder2, rPalette.GetColorIndex( rBorder.mnBottomColorId ),  7, 7 );
}


// ----------------------------------------------------------------------------

XclExpXFArea::XclExpXFArea() :
    mnForeColorId( 0 ),
    mnBackColorId( 0 ),
    mnPattern( EXC_PATT_NONE )
{
}

#if 0
void lcl_GetArea2( sal_uInt8& rnFlags, const XclExpXFArea& rArea )
{
    ::set_flag( rnFlags, EXC_XF2_BACKGROUND, rArea.mnPattern != EXC_PATT_NONE );
}

void lcl_GetArea3( sal_uInt16& rnArea, const XclExpXFArea& rArea, const XclExpPalette& rPalette )
{
    ::insert_value( rnArea, rArea.mnPattern, 0, 6 );
    ::insert_value( rnArea, rPalette.GetColorIndex( rArea.mnForeColorId ),  6, 5 );
    ::insert_value( rnArea, rPalette.GetColorIndex( rArea.mnBackColorId ), 11, 5 );
}
#endif

void lcl_GetArea5( sal_uInt32& rnArea, const XclExpXFArea& rArea, const XclExpPalette& rPalette )
{
    sal_uInt16 nXclForeIx, nXclBackIx;
    sal_uInt8 nXclPattern = rArea.mnPattern;
    rPalette.GetMixedColors( nXclForeIx, nXclBackIx, nXclPattern, rArea.mnForeColorId, rArea.mnBackColorId );
    ::insert_value( rnArea, nXclPattern, 16, 6 );
    ::insert_value( rnArea, nXclForeIx,   0, 7 );
    ::insert_value( rnArea, nXclBackIx,   7, 7 );
}

void lcl_GetArea8( sal_uInt32& rnBorder2, sal_uInt16& rnArea, const XclExpXFArea& rArea, const XclExpPalette& rPalette )
{
    sal_uInt16 nXclForeIx, nXclBackIx;
    sal_uInt8 nXclPattern = rArea.mnPattern;
    rPalette.GetMixedColors( nXclForeIx, nXclBackIx, nXclPattern, rArea.mnForeColorId, rArea.mnBackColorId );
    ::insert_value( rnBorder2, nXclPattern, 26, 6 );
    ::insert_value( rnArea,    nXclForeIx,   0, 7 );
    ::insert_value( rnArea,    nXclBackIx,   7, 7 );
}


// ----------------------------------------------------------------------------

XclExpXF::XclExpXF(
        const XclExpRoot& rRoot, const ScPatternAttr* pPattern,
        sal_uInt32 nForcedNumFmt, bool bForceWrapped ) :
    mbCellXF( true )
{
    Init( rRoot, pPattern, nForcedNumFmt, bForceWrapped );
}

XclExpXF::XclExpXF( const XclExpRoot& rRoot, SfxStyleSheetBase& rStyleSheet ) :
    mbCellXF( false )
{
    // create an own ScPatternAttr and let Init() do all the work
    mpOwnPattern.reset( new ScPatternAttr( rStyleSheet.GetItemSet().Clone() ) );
    Init( rRoot, mpOwnPattern.get() );
}

bool XclExpXF::Equals( const ScPatternAttr* pPattern, sal_uInt32 nForcedNumFmt, bool bForceWrapped ) const
{
    return  mbCellXF && (mpPattern == pPattern) &&
            ((nForcedNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND) || (mnNumFmt == nForcedNumFmt)) &&
            (!bForceWrapped || mbWrapped);
}

void XclExpXF::GetBorderLine(
        sal_uInt8& rnXclLine, sal_uInt32& rnColorId,
        XclExpPalette& rPalette, const SvxBorderLine* pLine )
{
    if( pLine )
    {
        sal_uInt16 nOuterWidth = pLine->GetOutWidth();
        sal_uInt16 nDistance = pLine->GetDistance();
        if( nDistance > 0 )
            rnXclLine = EXC_LINE_DOUBLE;
        else if( nOuterWidth > DEF_LINE_WIDTH_2 )
            rnXclLine = EXC_LINE_THICK;
        else if( nOuterWidth > DEF_LINE_WIDTH_1 )
            rnXclLine = EXC_LINE_MEDIUM;
        else if( nOuterWidth > DEF_LINE_WIDTH_0 )
            rnXclLine = EXC_LINE_THIN;
        else if( nOuterWidth > 0 )
            rnXclLine = EXC_LINE_HAIR;
        else
            rnXclLine = EXC_LINE_NONE;
        rnColorId = rPalette.InsertColor( pLine->GetColor(), xlColorCellBorder );
    }
    else
    {
        rnXclLine = EXC_LINE_NONE;
        rnColorId = rPalette.InsertIndex( 0 );
    }
}

void XclExpXF::GetBorder( XclExpXFBorder& rBorder, XclExpPalette& rPalette, const ScPatternAttr& rPattern )
{
    const SvxBoxItem& rBoxItem = static_cast< const SvxBoxItem& >( rPattern.GetItem( ATTR_BORDER ) );
    GetBorderLine( rBorder.mnLeftLine,   rBorder.mnLeftColorId,   rPalette, rBoxItem.GetLeft() );
    GetBorderLine( rBorder.mnRightLine,  rBorder.mnRightColorId,  rPalette, rBoxItem.GetRight() );
    GetBorderLine( rBorder.mnTopLine,    rBorder.mnTopColorId,    rPalette, rBoxItem.GetTop() );
    GetBorderLine( rBorder.mnBottomLine, rBorder.mnBottomColorId, rPalette, rBoxItem.GetBottom() );
}

void XclExpXF::GetArea( XclExpXFArea& rArea, XclExpPalette& rPalette, const ScPatternAttr& rPattern )
{
    const SvxBrushItem& rBrushItem = static_cast< const SvxBrushItem& >( rPattern.GetItem( ATTR_BACKGROUND ) );
    rArea.mnPattern = rBrushItem.GetColor().GetTransparency() ? EXC_PATT_NONE : EXC_PATT_SOLID;
    rArea.mnForeColorId = rPalette.InsertColor( rBrushItem.GetColor(), xlColorCellArea );
    rArea.mnBackColorId = rPalette.InsertColor( Color( COL_BLACK ), xlColorCellArea );
}

void XclExpXF::Init(
        const XclExpRoot& rRoot, const ScPatternAttr* pPattern,
        sal_uInt32 nForcedNumFmt, bool bForceWrapped )
{
    XclExpPalette& rPalette = rRoot.GetPalette();
    XclExpNumFmtBuffer& rNumFmtBuffer = rRoot.GetNumFmtBuffer();

// *** reset members ***

    // mbCellXF set in constructors
    SetRecHeader( EXC_ID_XF, (rRoot.GetBiff() == xlBiff8) ? 20 : 16 );
    mpPattern = pPattern;
    meHorAlign = xlHAlignGeneral;
    meVerAlign = xlVAlignBottom;
    meOrient = xlTextOrientNoRot;
    meTextDir = xlTextDirContext;
    mnParent = mbCellXF ? EXC_XF_DEFAULTSTYLE : EXC_XF_STYLEPARENT;
    mnFont = mnNumFmt = 0;
    if( nForcedNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND )
        mnNumFmt = rNumFmtBuffer.Insert( nForcedNumFmt );
    mnIndent = mnRotation = 0;
    mbLocked = true;        // default in Calc and Excel
    mbHidden = false;
    mbWrapped = bForceWrapped;
    mbProtUsed = mbFontUsed = mbFmtUsed = mbAlignUsed = mbBorderUsed = mbAreaUsed = false;

// *** formatting attributes ***

    if( !mpPattern )
    {
        maArea.mnForeColorId = rPalette.InsertIndex( 64 );
        maArea.mnBackColorId = rPalette.InsertIndex( 65 );
        if( nForcedNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND )
            mnNumFmt = rNumFmtBuffer.Insert( rNumFmtBuffer.GetStandardFormat() );
        return;
    }

// Creates a variable 'name' of type "const 'type' &" and assigns item with 'which' ID from mpPattern.
#define GETITEM( type, name, which ) \
    const type & name = static_cast< const type & >( mpPattern->GetItem( which ) )

    // cell protection
    GETITEM( ScProtectionAttr, rProtItem, ATTR_PROTECTION );
    mbLocked = !!rProtItem.GetProtection();     // BOOL->bool
    mbHidden = rProtItem.GetHideFormula() || rProtItem.GetHideCell();

    // font
    mnFont = rRoot.GetFontBuffer().Insert( *mpPattern );

    // number format
    if( nForcedNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND ) // don't overwrite forced format
    {
        GETITEM( SfxUInt32Item, rNumFmtItem, ATTR_VALUE_FORMAT );
        mnNumFmt = rNumFmtBuffer.Insert( rNumFmtItem.GetValue() );
    }

    // horizontal alignment
    GETITEM( SvxHorJustifyItem, rHorJustItem, ATTR_HOR_JUSTIFY );
    switch( static_cast< SvxCellHorJustify >( rHorJustItem.GetValue() ) )
    {
        case SVX_HOR_JUSTIFY_STANDARD:  meHorAlign = xlHAlignGeneral;   break;
        case SVX_HOR_JUSTIFY_LEFT:      meHorAlign = xlHAlignLeft;      break;
        case SVX_HOR_JUSTIFY_CENTER:    meHorAlign = xlHAlignCenter;    break;
        case SVX_HOR_JUSTIFY_RIGHT:     meHorAlign = xlHAlignRight;     break;
        case SVX_HOR_JUSTIFY_BLOCK:     meHorAlign = xlHAlignJustify;   break;
        case SVX_HOR_JUSTIFY_REPEAT:    meHorAlign = xlHAlignFill;      break;
        default:    DBG_ERROR( "XclExpXF::Init - unknown horizontal adjustment" );
    }

    // text wrap
    if( !mbWrapped )        // only, if not yet forced by caller
    {
        GETITEM( SfxBoolItem, rWrapItem, ATTR_LINEBREAK );
        mbWrapped = !!rWrapItem.GetValue();     // BOOL->bool
    }

    // vertical alignment
    GETITEM( SvxVerJustifyItem, rVerJustItem, ATTR_VER_JUSTIFY );
    switch( static_cast< SvxCellVerJustify >( rVerJustItem.GetValue() ) )
    {
        case SVX_VER_JUSTIFY_STANDARD:  meVerAlign = xlVAlignBottom;    break;
        case SVX_VER_JUSTIFY_TOP:       meVerAlign = xlVAlignTop;       break;
        case SVX_VER_JUSTIFY_CENTER:    meVerAlign = xlVAlignCenter;    break;
        case SVX_VER_JUSTIFY_BOTTOM:    meVerAlign = xlVAlignBottom;    break;
        default:    DBG_ERROR( "XclExpXF::Init - unknown vertical adjustment" );
    }

    // text indent
    GETITEM( SfxUInt16Item, rIndentItem, ATTR_INDENT );
    sal_Int32 nTmpIndent = rIndentItem.GetValue();      // SfxUInt16Item returns INT16, weird...
    (nTmpIndent += 100) /= 200;
    mnIndent = static_cast< sal_uInt8 >( ::std::max( ::std::min( nTmpIndent, 0x0FL ), 0L ) );

    // orientation and rotation
    GETITEM( SvxOrientationItem, rOrientItem, ATTR_ORIENTATION );
    switch( static_cast< SvxCellOrientation >( rOrientItem.GetValue() ) )
    {
        case SVX_ORIENTATION_STANDARD:  meOrient = xlTextOrientNoRot;       break;
        case SVX_ORIENTATION_TOPBOTTOM: meOrient = xlTextOrient90cw;        break;
        case SVX_ORIENTATION_BOTTOMTOP: meOrient = xlTextOrient90ccw;       break;
        case SVX_ORIENTATION_STACKED:   meOrient = xlTextOrientTopBottom;   break;
        default:    DBG_ERROR( "XclExpXF::Init - unknown text orientation" );
    }
    switch( meOrient )
    {
        case xlTextOrientTopBottom: mnRotation = EXC_XF8_STACKED;   break;
        // #i4378# old calc doc's without real rotation
        case xlTextOrient90ccw:     mnRotation = 90;                break;
        case xlTextOrient90cw:      mnRotation = 180;               break;
        default:
        {
            GETITEM( SfxInt32Item, rRotateItem, ATTR_ROTATE_VALUE );
            mnRotation = XclTools::GetXclRotation( rRotateItem.GetValue() );
        }
    }

    // cell border
    GetBorder( maBorder, rPalette, *mpPattern );

    // cell background area
    GetArea( maArea, rPalette, *mpPattern );

    // CTL text direction
    GETITEM( SvxFrameDirectionItem, rFrameDirItem, ATTR_WRITINGDIR );
    switch( static_cast< SvxFrameDirection >( rFrameDirItem.GetValue() ) )
    {
        case FRMDIR_ENVIRONMENT:    meTextDir = xlTextDirContext;   break;
        case FRMDIR_HORI_LEFT_TOP:  meTextDir = xlTextDirLTR;       break;
        case FRMDIR_HORI_RIGHT_TOP: meTextDir = xlTextDirRTL;       break;
        default:    DBG_ERRORFILE( "XclExpXF::Init - unknown CTL text direction" );
    }

#undef GETITEM

// *** used/unused flags ***

    // all font items
    static const sal_uInt16 pnFontWhichIds[] =
    {
        ATTR_FONT_HEIGHT,
        ATTR_FONT_WEIGHT,
        ATTR_FONT_POSTURE,
        ATTR_FONT_UNDERLINE,
        ATTR_FONT_CROSSEDOUT,
        ATTR_FONT_CONTOUR,
        ATTR_FONT_SHADOWED,
        ATTR_FONT_COLOR,
        ATTR_FONT_LANGUAGE,
        0
    };

    const SfxItemSet& rItemSet = mpPattern->GetItemSet();

// Expands to true if rItemSet really contains the item with 'which' ID.
#define CHECKITEM( which ) (rItemSet.GetItemState( which, FALSE ) == SFX_ITEM_SET)

    switch( rRoot.GetBiff() )
    {
        // all 'case's -> run through
        case xlBiff8:   // new in BIFF8/BIFF8X
            mbAlignUsed |= CHECKITEM( ATTR_ROTATE_VALUE ) ||
                CHECKITEM( ATTR_INDENT ) || CHECKITEM( ATTR_WRITINGDIR );

        case xlBiff7:   // new in BIFF7 - nothing
        case xlBiff5:   // new in BIFF5 - nothing
        case xlBiff4:   // new in BIFF4
            mbAlignUsed |= CHECKITEM( ATTR_VER_JUSTIFY ) || CHECKITEM( ATTR_ORIENTATION );

        case xlBiff3:   // attributes in all BIFF versions (BIFF2 does not use the flags at all)
            mbProtUsed   |= CHECKITEM( ATTR_PROTECTION );
            mbFontUsed   |= !!mpPattern->HasItemsSet( pnFontWhichIds );    // BOOL->bool
            mbFmtUsed    |= CHECKITEM( ATTR_VALUE_FORMAT );
            mbBorderUsed |= CHECKITEM( ATTR_BORDER );
            mbAreaUsed   |= CHECKITEM( ATTR_BACKGROUND );
            mbAlignUsed  |= CHECKITEM( ATTR_HOR_JUSTIFY ) || CHECKITEM( ATTR_LINEBREAK );
        break;
    }

#undef CHECKITEM
}

sal_uInt8 XclExpXF::GetUsedAttribMask() const
{
    sal_uInt8 nUsedFlags = 0;
    // "mbCellXF == mb***Used" evaluates to correct value in cell and style XFs
    ::set_flag( nUsedFlags, EXC_XF_DIFF_PROT,   mbCellXF == mbProtUsed );
    ::set_flag( nUsedFlags, EXC_XF_DIFF_FONT,   mbCellXF == mbFontUsed );
    ::set_flag( nUsedFlags, EXC_XF_DIFF_VALFMT, mbCellXF == mbFmtUsed );
    ::set_flag( nUsedFlags, EXC_XF_DIFF_ALIGN,  mbCellXF == mbAlignUsed );
    ::set_flag( nUsedFlags, EXC_XF_DIFF_BORDER, mbCellXF == mbBorderUsed );
    ::set_flag( nUsedFlags, EXC_XF_DIFF_AREA,   mbCellXF == mbAreaUsed );
    return nUsedFlags;
}

void XclExpXF::WriteBody5( XclExpStream& rStrm )
{
    sal_uInt16 nTypeProt = 0;
    ::set_flag( nTypeProt, EXC_XF_LOCKED, mbLocked );
    ::set_flag( nTypeProt, EXC_XF_HIDDEN, mbHidden );
    ::set_flag( nTypeProt, EXC_XF_STYLE, !mbCellXF );
    ::insert_value( nTypeProt, mnParent, 4, 12 );

    sal_uInt16 nAlign = 0;
    ::insert_value( nAlign, meHorAlign, 0, 3 );
    ::set_flag( nAlign, EXC_XF_WRAPPED, mbWrapped );
    ::insert_value( nAlign, meVerAlign, 4, 3 );
    ::insert_value( nAlign, meOrient, 8, 2 );
    ::insert_value( nAlign, GetUsedAttribMask(), 10, 6 );

    sal_uInt32 nArea = 0, nBorder = 0;
    lcl_GetBorder5( nBorder, nArea, maBorder, rStrm.GetRoot().GetPalette() );
    lcl_GetArea5( nArea, maArea, rStrm.GetRoot().GetPalette() );

    rStrm << mnFont << mnNumFmt << nTypeProt << nAlign << nArea << nBorder;
}

void XclExpXF::WriteBody8( XclExpStream& rStrm )
{
    sal_uInt16 nTypeProt = 0;
    ::set_flag( nTypeProt, EXC_XF_LOCKED, mbLocked );
    ::set_flag( nTypeProt, EXC_XF_HIDDEN, mbHidden );
    ::set_flag( nTypeProt, EXC_XF_STYLE, !mbCellXF );
    ::insert_value( nTypeProt, mnParent, 4, 12 );

    sal_uInt16 nAlign = 0;
    ::insert_value( nAlign, meHorAlign, 0, 3 );
    ::set_flag( nAlign, EXC_XF_WRAPPED, mbWrapped );
    ::insert_value( nAlign, meVerAlign, 4, 3 );
    ::insert_value( nAlign, mnRotation, 8, 8 );

    sal_uInt16 nMiscAttrib = 0;
    ::insert_value( nMiscAttrib, mnIndent, 0, 4 );
    ::insert_value( nMiscAttrib, meTextDir, 6, 2 );
    ::insert_value( nMiscAttrib, GetUsedAttribMask(), 10, 6 );

    sal_uInt32 nBorder1 = 0, nBorder2 = 0;
    sal_uInt16 nArea = 0;
    lcl_GetBorder8( nBorder1, nBorder2, maBorder, rStrm.GetRoot().GetPalette() );
    lcl_GetArea8( nBorder2, nArea, maArea, rStrm.GetRoot().GetPalette() );

    rStrm << mnFont << mnNumFmt << nTypeProt << nAlign << nMiscAttrib << nBorder1 << nBorder2 << nArea;
}

void XclExpXF::WriteBody( XclExpStream& rStrm )
{
    switch( rStrm.GetRoot().GetBiff() )
    {
        case xlBiff5:
        case xlBiff7:   WriteBody5( rStrm );    break;
        case xlBiff8:   WriteBody8( rStrm );    break;
        default:        DBG_ERROR_BIFF();
    }
}


// ----------------------------------------------------------------------------

XclExpStyle::XclExpStyle( const String& rName, sal_uInt16 nXFIndex ) :
    XclExpRecord( EXC_ID_STYLE ),
    maName( rName ),
    mnXFIndex( nXFIndex )
{
    DBG_ASSERT( maName.Len(), "XclExpStyle::XclExpStyle - empty style name" );
}

void XclExpStyle::WriteBody( XclExpStream& rStrm )
{
    XclExpString aNameEx;
    if( rStrm.GetRoot().GetBiff() >= xlBiff8 )
        aNameEx.Assign( maName );
    else
        aNameEx.AssignByte( maName, rStrm.GetRoot().GetCharSet(), EXC_STR_8BITLENGTH );
    rStrm << mnXFIndex << aNameEx;
}


// ----------------------------------------------------------------------------

XclExpXFBuffer::XclExpXFBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnXclMaxCount( EXC_XF_MAXCOUNT ),
    mnXclOffset( EXC_XF_OFFSET )
{
}

void XclExpXFBuffer::InsertUserStyles()
{
    SfxStyleSheetIterator aStyleIter( GetDoc().GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
    for( SfxStyleSheetBase* pStyle = aStyleIter.First(); pStyle; pStyle = aStyleIter.Next() )
        if( pStyle->IsUserDefined() )
            InsertStyle( *pStyle );
}

sal_uInt16 XclExpXFBuffer::Insert( const ScPatternAttr* pPattern, bool bForceWrapped )
{
    return InsertCellXF( pPattern, NUMBERFORMAT_ENTRY_NOT_FOUND, bForceWrapped );
}

sal_uInt16 XclExpXFBuffer::Insert( const ScPatternAttr* pPattern, sal_uInt32 nForcedNumFmt )
{
    return InsertCellXF( pPattern, nForcedNumFmt, false );
}

sal_uInt16 XclExpXFBuffer::InsertStyle( SfxStyleSheetBase& rStyleSheet )
{
    // create the XF record
    sal_uInt16 nXFIndex = EXC_XF_DEFAULTSTYLE;
    if( maXFList.Count() < EXC_XF_MAXCOUNT )
    {
        nXFIndex = GetXclIndex( maXFList.Count() );
        maXFList.Append( new XclExpXF( GetRoot(), rStyleSheet ) );
    }

    // create the STYLE record
    if( rStyleSheet.GetName().Len() )
        maStyleList.Append( new XclExpStyle( rStyleSheet.GetName(), nXFIndex ) );

    return nXFIndex;
}

void XclExpXFBuffer::Save( XclExpStream& rStrm )
{
    // XF records
    switch( GetBiff() )
    {
        case xlBiff5:
        case xlBiff7:   WriteDefaultXFs5( rStrm );      break;
        case xlBiff8:   WriteDefaultXFs8( rStrm );      break;
        default:        DBG_ERROR_BIFF();
    }
    maXFList.Save( rStrm );

    // STYLE records
    switch( GetBiff() )
    {
        case xlBiff5:
        case xlBiff7:
        case xlBiff8:   WriteDefaultStyles5( rStrm );   break;
        default:        DBG_ERROR_BIFF();
    }
    maStyleList.Save( rStrm );
}

sal_uInt16 XclExpXFBuffer::GetXclIndex( sal_uInt32 nIndex ) const
{
    DBG_ASSERT( nIndex < mnXclMaxCount, "XclExpXFBuffer::GetXclIndex - invalid index" );
    return static_cast< sal_uInt16 >( nIndex + mnXclOffset );
}

sal_uInt16 XclExpXFBuffer::InsertCellXF(
        const ScPatternAttr* pPattern, sal_uInt32 nForcedNumFmt, bool bForceWrapped )
{
    sal_uInt32 nIndex = LIST_ENTRY_NOTFOUND;

    // try to find existing cell XF
    for( const XclExpXF* pXF = maXFList.First(); pXF; pXF = maXFList.Next() )
    {
        if( pXF->Equals( pPattern, nForcedNumFmt, bForceWrapped ) )
        {
            nIndex = maXFList.GetCurPos();
            break;
        }
    }

    // not found - insert new XF
    if( nIndex == LIST_ENTRY_NOTFOUND )
    {
        nIndex = maXFList.Count();
        if( nIndex < EXC_XF_MAXCOUNT )
            maXFList.Append( new XclExpXF( GetRoot(), pPattern, nForcedNumFmt, bForceWrapped ) );
        else
            // list full - fall back to default cell XF
            return EXC_XF_DEFAULTCELL;
    }

    return GetXclIndex( nIndex );
}

void XclExpXFBuffer::WriteDefaultXFs5( XclExpStream& rStrm )
{
    static const sal_uInt8 pnDefStyle[]      = { 0x00,0x00,0x00,0x00,0xF5,0xFF,0x20,0x00,0xC0,0x20,0x00,0x00,0x00,0x00,0x00,0x00 };
    static const sal_uInt8 pnOutlineStyle1[] = { 0x01,0x00,0x00,0x00,0xF5,0xFF,0x20,0xF4,0xC0,0x20,0x00,0x00,0x00,0x00,0x00,0x00 };
    static const sal_uInt8 pnOutlineStyle2[] = { 0x02,0x00,0x00,0x00,0xF5,0xFF,0x20,0xF4,0xC0,0x20,0x00,0x00,0x00,0x00,0x00,0x00 };
    static const sal_uInt8 pnOutlineStyle3[] = { 0x00,0x00,0x00,0x00,0xF5,0xFF,0x20,0xF4,0xC0,0x20,0x00,0x00,0x00,0x00,0x00,0x00 };
    static const sal_uInt8 pnDefCell[]       = { 0x00,0x00,0x00,0x00,0x01,0x00,0x20,0x00,0xC0,0x20,0x00,0x00,0x00,0x00,0x00,0x00 };
    static const sal_uInt8 pnBuiltInStyle1[] = { 0x05,0x00,0x2B,0x00,0xF5,0xFF,0x20,0xF8,0xC0,0x20,0x00,0x00,0x00,0x00,0x00,0x00 };
    static const sal_uInt8 pnBuiltInStyle2[] = { 0x05,0x00,0x29,0x00,0xF5,0xFF,0x20,0xF8,0xC0,0x20,0x00,0x00,0x00,0x00,0x00,0x00 };
    static const sal_uInt8 pnBuiltInStyle3[] = { 0x05,0x00,0x2C,0x00,0xF5,0xFF,0x20,0xF8,0xC0,0x20,0x00,0x00,0x00,0x00,0x00,0x00 };
    static const sal_uInt8 pnBuiltInStyle4[] = { 0x05,0x00,0x2A,0x00,0xF5,0xFF,0x20,0xF8,0xC0,0x20,0x00,0x00,0x00,0x00,0x00,0x00 };
    static const sal_uInt8 pnBuiltInStyle5[] = { 0x05,0x00,0x09,0x00,0xF5,0xFF,0x20,0xF8,0xC0,0x20,0x00,0x00,0x00,0x00,0x00,0x00 };

    // default style (1)
    XclExpDummyRecord aXF( EXC_ID_XF, pnDefStyle, STATIC_TABLE_SIZE( pnDefStyle ) ); aXF.Save( rStrm );
    // outline styles (14)
    aXF.SetData( pnOutlineStyle1, STATIC_TABLE_SIZE( pnOutlineStyle1 ) ); aXF.SaveRepeated( rStrm, 2 );
    aXF.SetData( pnOutlineStyle2, STATIC_TABLE_SIZE( pnOutlineStyle2 ) ); aXF.SaveRepeated( rStrm, 2 );
    aXF.SetData( pnOutlineStyle3, STATIC_TABLE_SIZE( pnOutlineStyle3 ) ); aXF.SaveRepeated( rStrm, 10 );
    // default cell XF (1)
    aXF.SetData( pnDefCell, STATIC_TABLE_SIZE( pnDefCell ) ); aXF.Save( rStrm );
    // built-in styles (5)
    aXF.SetData( pnBuiltInStyle1, STATIC_TABLE_SIZE( pnBuiltInStyle1 ) ); aXF.Save( rStrm );
    aXF.SetData( pnBuiltInStyle2, STATIC_TABLE_SIZE( pnBuiltInStyle2 ) ); aXF.Save( rStrm );
    aXF.SetData( pnBuiltInStyle3, STATIC_TABLE_SIZE( pnBuiltInStyle3 ) ); aXF.Save( rStrm );
    aXF.SetData( pnBuiltInStyle4, STATIC_TABLE_SIZE( pnBuiltInStyle4 ) ); aXF.Save( rStrm );
    aXF.SetData( pnBuiltInStyle5, STATIC_TABLE_SIZE( pnBuiltInStyle5 ) ); aXF.Save( rStrm );
}

void XclExpXFBuffer::WriteDefaultXFs8( XclExpStream& rStrm )
{
    static const sal_uInt8 pnDefStyle[]      = { 0x00,0x00,0x00,0x00,0xF5,0xFF,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20 };
    static const sal_uInt8 pnOutlineStyle1[] = { 0x01,0x00,0x00,0x00,0xF5,0xFF,0x20,0x00,0x00,0xF4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20 };
    static const sal_uInt8 pnOutlineStyle2[] = { 0x02,0x00,0x00,0x00,0xF5,0xFF,0x20,0x00,0x00,0xF4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20 };
    static const sal_uInt8 pnOutlineStyle3[] = { 0x00,0x00,0x00,0x00,0xF5,0xFF,0x20,0x00,0x00,0xF4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20 };
    static const sal_uInt8 pnDefCell[]       = { 0x00,0x00,0x00,0x00,0x01,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20 };
    static const sal_uInt8 pnBuiltInStyle1[] = { 0x01,0x00,0x2B,0x00,0xF5,0xFF,0x20,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20 };
    static const sal_uInt8 pnBuiltInStyle2[] = { 0x01,0x00,0x29,0x00,0xF5,0xFF,0x20,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20 };
    static const sal_uInt8 pnBuiltInStyle3[] = { 0x01,0x00,0x2C,0x00,0xF5,0xFF,0x20,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20 };
    static const sal_uInt8 pnBuiltInStyle4[] = { 0x01,0x00,0x2A,0x00,0xF5,0xFF,0x20,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20 };
    static const sal_uInt8 pnBuiltInStyle5[] = { 0x01,0x00,0x09,0x00,0xF5,0xFF,0x20,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x20 };

    // default style (1)
    XclExpDummyRecord aXF( EXC_ID_XF, pnDefStyle, STATIC_TABLE_SIZE( pnDefStyle ) ); aXF.Save( rStrm );
    // outline styles (14)
    aXF.SetData( pnOutlineStyle1, STATIC_TABLE_SIZE( pnOutlineStyle1 ) ); aXF.SaveRepeated( rStrm, 2 );
    aXF.SetData( pnOutlineStyle2, STATIC_TABLE_SIZE( pnOutlineStyle2 ) ); aXF.SaveRepeated( rStrm, 2 );
    aXF.SetData( pnOutlineStyle3, STATIC_TABLE_SIZE( pnOutlineStyle3 ) ); aXF.SaveRepeated( rStrm, 10 );
    // default cell XF (1)
    aXF.SetData( pnDefCell, STATIC_TABLE_SIZE( pnDefCell ) ); aXF.Save( rStrm );
    // built-in styles (5)
    aXF.SetData( pnBuiltInStyle1, STATIC_TABLE_SIZE( pnBuiltInStyle1 ) ); aXF.Save( rStrm );
    aXF.SetData( pnBuiltInStyle2, STATIC_TABLE_SIZE( pnBuiltInStyle2 ) ); aXF.Save( rStrm );
    aXF.SetData( pnBuiltInStyle3, STATIC_TABLE_SIZE( pnBuiltInStyle3 ) ); aXF.Save( rStrm );
    aXF.SetData( pnBuiltInStyle4, STATIC_TABLE_SIZE( pnBuiltInStyle4 ) ); aXF.Save( rStrm );
    aXF.SetData( pnBuiltInStyle5, STATIC_TABLE_SIZE( pnBuiltInStyle5 ) ); aXF.Save( rStrm );
}

void XclExpXFBuffer::WriteDefaultStyles5( XclExpStream& rStrm )
{
    XclExpUInt32Record( EXC_ID_STYLE, 0xFF038010 ).Save( rStrm );
    XclExpUInt32Record( EXC_ID_STYLE, 0xFF068011 ).Save( rStrm );
    XclExpUInt32Record( EXC_ID_STYLE, 0xFF048012 ).Save( rStrm );
    XclExpUInt32Record( EXC_ID_STYLE, 0xFF078013 ).Save( rStrm );
    XclExpUInt32Record( EXC_ID_STYLE, 0xFF008000 ).Save( rStrm );
    XclExpUInt32Record( EXC_ID_STYLE, 0xFF058014 ).Save( rStrm );
}


// ============================================================================

