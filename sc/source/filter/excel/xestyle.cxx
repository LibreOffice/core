/*************************************************************************
 *
 *  $RCSfile: xestyle.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 16:19:18 $
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
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
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
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX
#include <svx/escpitem.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_STLPOOL_HXX
#include "stlpool.hxx"
#endif
#ifndef SC_STLSHEET_HXX
#include "stlsheet.hxx"
#endif
#ifndef SC_SCPATATR_HXX
#include "patattr.hxx"
#endif
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
#endif
#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif

#ifndef SC_XESTRING_HXX
#include "xestring.hxx"
#endif


// PALETTE record - color information =========================================

sal_Int32 lcl_xestyle_GetColorDistance( const Color& rColor1, const Color& rColor2 )
{
    sal_Int32 nDist = rColor1.GetRed() - rColor2.GetRed();
    nDist *= nDist * 77;
    sal_Int32 nDummy = rColor1.GetGreen() - rColor2.GetGreen();
    nDist += nDummy * nDummy * 151;
    nDummy = rColor1.GetBlue() - rColor2.GetBlue();
    nDist += nDummy * nDummy * 28;
    return nDist;
}

void lcl_xestyle_SetMixedColor( Color& rDest, const Color& rSrc1, const Color& rSrc2 )
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
    XclDefaultPalette( rRoot ),
    XclExpRoot( rRoot ),
    mnLastIx( 0 )
{
    SetRecSize( GetColorCount() * 4 + 2 );

    // initialize maPalette with default colors
    sal_uInt32 nCount = GetColorCount();
    maPalette.resize( nCount );
    XclPaletteColorVec::iterator aIter = maPalette.begin();
    for( sal_uInt32 nIx = 0; nIx < nCount; ++nIx, ++aIter )
        aIter->SetColor( GetDefColor( GetXclIndex( nIx ) ), true );

    InsertColor( Color( COL_BLACK ), xlColorCellText );
}

sal_uInt32 XclExpPalette::InsertColor( const Color& rColor, XclColorType eType, sal_uInt16 nAutoDefault )
{
    if( rColor.GetColor() == COL_AUTO )
        return GetColorIdFromIndex( nAutoDefault );

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

sal_uInt32 XclExpPalette::GetColorIdFromIndex( sal_uInt16 nIndex )
{
    return EXC_PAL_INDEXBASE | nIndex;
}

void XclExpPalette::Reduce()
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
        DBG_ASSERT( maColorList.GetObject( nFound ), "XclExpPalette::Reduce - missing a color" );
        DBG_ASSERT( nNearest < maPalette.size(), "XclExpPalette::Reduce - algorithm error" );
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
    rnXclForeIx = GetColorIndex( nForeColorId );
    rnXclBackIx = GetColorIndex( nBackColorId );
    if( (rnXclPattern != EXC_PATT_SOLID) || (nForeColorId >= maColorIdDataVec.size()) )
        return;

    // now we have solid pattern, and a defined foreground (background doesn't care for solid pattern)

    sal_uInt32 nIndex1, nIndex2;
    Color aForeColor( GetOriginalColor( nForeColorId ) );
    sal_Int32 nFirstDist = GetNearPaletteColors( nIndex1, nIndex2, aForeColor );
    if( (nIndex1 >= maPalette.size()) || (nIndex2 >= maPalette.size()) )
        return;

    Color aColorArr[ 5 ];
    aColorArr[ 0 ] = maPalette[ nIndex1 ].maColor;
    aColorArr[ 4 ] = maPalette[ nIndex2 ].maColor;
    lcl_xestyle_SetMixedColor( aColorArr[ 2 ], aColorArr[ 0 ], aColorArr[ 4 ] );
    lcl_xestyle_SetMixedColor( aColorArr[ 1 ], aColorArr[ 0 ], aColorArr[ 2 ] );
    lcl_xestyle_SetMixedColor( aColorArr[ 3 ], aColorArr[ 2 ], aColorArr[ 4 ] );

    sal_Int32 nMinDist = nFirstDist;
    sal_uInt32 nMinIndex = 0;
    for( sal_uInt32 nCnt = 1; nCnt < 4; ++nCnt )
    {
        sal_Int32 nDist = lcl_xestyle_GetColorDistance( aForeColor, aColorArr[ nCnt ] );
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

ColorData XclExpPalette::GetColorData( sal_uInt16 nXclIndex ) const
{
    if( nXclIndex >= EXC_COLOR_USEROFFSET )
    {
        sal_uInt32 nIx = nXclIndex - EXC_COLOR_USEROFFSET;
        if( nIx < maPalette.size() )
            return maPalette[ nIx ].maColor.GetColor();
    }
    return GetDefColorData( nXclIndex );
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
                sal_Int32 nDist = lcl_xestyle_GetColorDistance( rColor, pCheck->GetColor() );
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
            sal_Int32 nCurrDist = lcl_xestyle_GetColorDistance( rColor, aIter->maColor );
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
        sal_Int32 nCurrDist = lcl_xestyle_GetColorDistance( rColor, aIter->maColor );
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

XclExpFont::XclExpFont( const XclExpRoot& rRoot, const Font& rFont ) :
    XclExpRecord( EXC_ID_FONT ),
    XclExpRoot( rRoot ),
    maData( rFont )
{
    SetColorHashAndRecSize(rFont);
}

XclExpFont::XclExpFont( const XclExpRoot& rRoot, const SvxFont& rFont ) :
    XclExpRecord( EXC_ID_FONT ),
    XclExpRoot( rRoot ),
    maData( rFont )
{
    SetColorHashAndRecSize(rFont);
}

XclExpFont::XclExpFont( const XclExpRoot& rRoot, const XclFontData& rFontData ) :
    XclExpRecord( EXC_ID_FONT, 14 ),
    XclExpRoot( rRoot ),
    maData( rFontData ),
    mnColorId( XclExpPalette::GetColorIdFromIndex( EXC_COLOR_FONTAUTO ) ),
    mbHasColor( false )
{
    CalcHash();
}

void XclExpFont::SetColorId( sal_uInt32 nColorId )
{
    mnColorId = nColorId;
    mbHasColor = true;
}

bool XclExpFont::Equals( const XclExpFont& rCmpFont ) const
{
    return
        (mnHash == rCmpFont.mnHash) &&
        (!mbHasColor || !rCmpFont.mbHasColor || (mnColorId == rCmpFont.mnColorId)) &&
        (maData == rCmpFont.maData);
}

void XclExpFont::CalcHash()
{
    mnHash = maData.maName.Len();
    mnHash += maData.mnWeight * 3;
    mnHash += maData.mnCharSet * 5;
    mnHash += maData.mnFamily * 7;
    mnHash += maData.mnHeight * 11;
    mnHash += maData.mnUnderline * 13;
    mnHash += maData.mnEscapem * 17;
    if( maData.mbItalic ) mnHash += 19;
    if( maData.mbStrikeout ) mnHash += 23;
    if( maData.mbOutline ) mnHash += 29;
    if( maData.mbShadow ) mnHash += 31;
}

void XclExpFont::SetColorHashAndRecSize( const Font& rFont )
{
    SetColorId( GetPalette().InsertColor( rFont.GetColor(), xlColorCellText, EXC_COLOR_FONTAUTO ) );
    CalcHash();

    sal_uInt32 nRecSize = maData.maName.Len();
    if( GetBiff() >= xlBiff8 )
        (nRecSize *= 2) += 1;
    nRecSize += 15;
    SetRecSize( nRecSize );
}

void XclExpFont::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nAttr = EXC_FONTATTR_NONE;
    ::set_flag( nAttr, EXC_FONTATTR_ITALIC, maData.mbItalic );
    ::set_flag( nAttr, EXC_FONTATTR_STRIKEOUT, maData.mbStrikeout );
    ::set_flag( nAttr, EXC_FONTATTR_OUTLINE, maData.mbOutline );
    ::set_flag( nAttr, EXC_FONTATTR_SHADOW, maData.mbShadow );

    DBG_ASSERT( maData.maName.Len() < 256, "XclExpFont::WriteBody - font name too long" );
    XclExpString aFontName;
    if( GetBiff() < xlBiff8 )
        aFontName.AssignByte( maData.maName, GetCharSet(), EXC_STR_8BITLENGTH );
    else
        aFontName.Assign( maData.maName, EXC_STR_FORCEUNICODE | EXC_STR_8BITLENGTH );

    rStrm   << maData.mnHeight
            << nAttr
            << GetPalette().GetColorIndex( mnColorId )
            << maData.mnWeight
            << maData.mnEscapem
            << maData.mnUnderline
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
    switch( GetBiff() )
    {
        case xlBiff4:   mnXclMaxCount = EXC_FONT_MAXCOUNT4; break;
        case xlBiff5:
        case xlBiff7:   mnXclMaxCount = EXC_FONT_MAXCOUNT5; break;
        case xlBiff8:   mnXclMaxCount = EXC_FONT_MAXCOUNT8; break;
        default:        DBG_ERROR_BIFF();
    }
    InitDefaultFonts();
}

sal_uInt16 XclExpFontBuffer::GetIndex( const XclExpFont& rFont, sal_uInt16 nDefault )
{
    sal_uInt32 nIndex = Find( rFont );
    return (nIndex < mnXclMaxCount) ? GetXclIndex( nIndex ) : nDefault;
}

sal_uInt16 XclExpFontBuffer::Insert( XclExpFont*& rpFont, bool bAppFont )
{
    DBG_ASSERT( rpFont, "XclExpFontBuffer::Insert - no font" );
    sal_uInt32 nIndex = EXC_FONT_APP;
    if( rpFont )
    {
        if( bAppFont )
        {
            maFontList.Replace( rpFont, EXC_FONT_APP );
            nIndex = EXC_FONT_APP;
            // #108487# set width of '0' character for column width export
            SetCharWidth( rpFont->GetFontData() );
        }
        else
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
                rpFont = maFontList.GetObject( EXC_FONT_APP );
                nIndex = EXC_FONT_APP;
            }
        }
    }
    return GetXclIndex( nIndex );
}

sal_uInt16 XclExpFontBuffer::Insert( const Font& rFont, bool bAppFont )
{
    SvxFont aSvxFont(rFont);
    return Insert( aSvxFont, bAppFont );
}

sal_uInt16 XclExpFontBuffer::Insert( const SvxFont& rFont, bool bAppFont )
{
    XclExpFont* pNewFont = new XclExpFont( GetRoot(), rFont );
    return Insert( pNewFont, bAppFont );
}

sal_uInt16 XclExpFontBuffer::Insert( const SfxItemSet& rItemSet, bool bAppFont )
{
    /*  We need to determine if a CJK or CTL fontitem is set in the itemset.
        This is discussed in #i17050#. It is possible that both may be present.
        In this case, we will choose CJK. Either option is equally correct. */
    BYTE nScript = SCRIPTTYPE_LATIN;

    // #114008# do not let a font from a parent style override an explicit cell font
    const SfxItemSet* pCurrSet = &rItemSet;
    bool bFound = false;
    while( !bFound && pCurrSet )
    {
        bFound = true;
        if( ScfTools::CheckItem( *pCurrSet, ATTR_CJK_FONT, false ) )
            nScript = SCRIPTTYPE_ASIAN;
        else if( ScfTools::CheckItem( *pCurrSet, ATTR_CTL_FONT, false ) )
            nScript = SCRIPTTYPE_COMPLEX;
        else if( ScfTools::CheckItem( *pCurrSet, ATTR_FONT, false ) )
            nScript = SCRIPTTYPE_LATIN;
        else
            bFound = false;
        pCurrSet = pCurrSet->GetParent();
    }

    Font aFont;
    ScPatternAttr::GetFont( aFont, rItemSet, SC_AUTOCOL_RAW, NULL, NULL, NULL, nScript );
    return Insert( aFont, bAppFont );
}

sal_uInt16 XclExpFontBuffer::Insert( const ScPatternAttr& rPattern, bool bAppFont )
{
    return Insert( rPattern.GetItemSet(), bAppFont );
}

void XclExpFontBuffer::Save( XclExpStream& rStrm )
{
    maFontList.Save( rStrm );
}

void XclExpFontBuffer::InitDefaultFonts()
{
    XclFontData aFontData;
    aFontData.maName.AssignAscii( "Arial" );
    aFontData.SetScFamily( FAMILY_DONTKNOW );
    aFontData.SetScCharSet( RTL_TEXTENCODING_DONTKNOW );
    aFontData.SetScHeight( 200 );   // 200 twips = 10 pt
    aFontData.SetScWeight( WEIGHT_NORMAL );

    switch( GetBiff() )
    {
        case xlBiff5:
        case xlBiff7:
            maFontList.Append( new XclExpFont( GetRoot(), aFontData ) );
            aFontData.SetScWeight( WEIGHT_BOLD );
            maFontList.Append( new XclExpFont( GetRoot(), aFontData ) );
            aFontData.SetScWeight( WEIGHT_NORMAL );
            aFontData.SetScPosture( ITALIC_NORMAL );
            maFontList.Append( new XclExpFont( GetRoot(), aFontData ) );
            aFontData.SetScWeight( WEIGHT_BOLD );
            maFontList.Append( new XclExpFont( GetRoot(), aFontData ) );
            aFontData.SetScWeight( WEIGHT_NORMAL );
            aFontData.SetScPosture( ITALIC_NONE );
            maFontList.Append( new XclExpFont( GetRoot(), aFontData ) );
        break;
        case xlBiff8:
            maFontList.Append( new XclExpFont( GetRoot(), aFontData ) );
            maFontList.Append( new XclExpFont( GetRoot(), aFontData ) );
            maFontList.Append( new XclExpFont( GetRoot(), aFontData ) );
            maFontList.Append( new XclExpFont( GetRoot(), aFontData ) );
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
    sal_uInt32 nCount = maFontList.Count();
    for( sal_uInt32 nFontIx = 1; nFontIx < nCount; ++nFontIx )
        if( maFontList.GetObject( nFontIx )->Equals( rFont ) )
            return nFontIx;
    return LIST_ENTRY_NOTFOUND;
}


// FORMAT record - number formats =============================================


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
    mnStdFmt( GetFormatter().GetStandardFormat( ScGlobal::eLnge ) )
{
    switch( GetBiff() )
    {
        case xlBiff5:
        case xlBiff7:   mnXclOffset = EXC_FORMAT_OFFSET5;   break;
        case xlBiff8:   mnXclOffset = EXC_FORMAT_OFFSET8;   break;
        default:        DBG_ERROR_BIFF();
    }

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



// XF, STYLE record - Cell formatting =========================================

bool XclExpCellProt::FillFromItemSet( const SfxItemSet& rItemSet, XclBiff eBiff, bool bStyle )
{
    const ScProtectionAttr& rProtItem = GETITEM( rItemSet, ScProtectionAttr, ATTR_PROTECTION );
    mbLocked = rProtItem.GetProtection();
    mbHidden = rProtItem.GetHideFormula() || rProtItem.GetHideCell();
    return ScfTools::CheckItem( rItemSet, ATTR_PROTECTION, bStyle );
}

#if 0
void XclExpCellProt::FillToXF2( sal_uInt8& rnNumFmt ) const
{
    ::set_flag( rnNumFmt, EXC_XF2_LOCKED, mbLocked );
    ::set_flag( rnNumFmt, EXC_XF2_HIDDEN, mbHidden );
}
#endif

void XclExpCellProt::FillToXF3( sal_uInt16& rnProt ) const
{
    ::set_flag( rnProt, EXC_XF_LOCKED, mbLocked );
    ::set_flag( rnProt, EXC_XF_HIDDEN, mbHidden );
}


// ----------------------------------------------------------------------------

bool XclExpCellAlign::FillFromItemSet( const SfxItemSet& rItemSet, XclBiff eBiff, bool bForceWrapped, bool bStyle )
{
    bool bUsed = false;

    switch( eBiff )
    {
        // ALL 'case's - run through!

        case xlBiff8:   // attributes new in BIFF8
        {
            // text indent
            sal_Int32 nTmpIndent = GETITEMVALUE( rItemSet, SfxUInt16Item, ATTR_INDENT, sal_Int32 );
            (nTmpIndent += 100) /= 200; // 1 Excel unit == 10 pt == 200 twips
            mnIndent = static_cast< sal_uInt8 >( ::std::max( ::std::min( nTmpIndent, 0x0FL ), 0L ) );
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_INDENT, bStyle );

            // rotation
            sal_Int32 nScRot = GETITEMVALUE( rItemSet, SfxInt32Item, ATTR_ROTATE_VALUE, sal_Int32 );
            mnRotation = XclTools::GetXclRotation( nScRot );
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_ROTATE_VALUE, bStyle );

            // CTL text direction
            switch( GETITEMVALUE( rItemSet, SvxFrameDirectionItem, ATTR_WRITINGDIR, SvxFrameDirection ) )
            {
                case FRMDIR_ENVIRONMENT:    meTextDir = xlTextDirContext;   break;
                case FRMDIR_HORI_LEFT_TOP:  meTextDir = xlTextDirLTR;       break;
                case FRMDIR_HORI_RIGHT_TOP: meTextDir = xlTextDirRTL;       break;
                default:    DBG_ERRORFILE( "XclExpCellAlign::FillFromItemSet - unknown CTL text direction" );
            }
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_WRITINGDIR, bStyle );
        }

        case xlBiff7:   // attributes new in BIFF7
        case xlBiff5:   // attributes new in BIFF5
        case xlBiff4:   // attributes new in BIFF4
        {
            // vertical alignment
            switch( GETITEMVALUE( rItemSet, SvxVerJustifyItem, ATTR_VER_JUSTIFY, SvxCellVerJustify ) )
            {
                case SVX_VER_JUSTIFY_STANDARD:  meVerAlign = xlVAlignBottom;    break;
                case SVX_VER_JUSTIFY_TOP:       meVerAlign = xlVAlignTop;       break;
                case SVX_VER_JUSTIFY_CENTER:    meVerAlign = xlVAlignCenter;    break;
                case SVX_VER_JUSTIFY_BOTTOM:    meVerAlign = xlVAlignBottom;    break;
                default:    DBG_ERROR( "XclExpCellAlign::FillFromItemSet - unknown vertical alignment" );
            }
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_VER_JUSTIFY, bStyle );

            // orientation
            switch( GETITEMVALUE( rItemSet, SvxOrientationItem, ATTR_ORIENTATION, SvxCellOrientation ) )
            {
                case SVX_ORIENTATION_STANDARD:  meOrient = xlTextOrientNoRot;       break;
                case SVX_ORIENTATION_TOPBOTTOM: meOrient = xlTextOrient90cw;        break;
                case SVX_ORIENTATION_BOTTOMTOP: meOrient = xlTextOrient90ccw;       break;
                case SVX_ORIENTATION_STACKED:   meOrient = xlTextOrientTopBottom;   break;
                default:    DBG_ERROR( "XclExpCellAlign::FillFromItemSet - unknown text orientation" );
            }
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_ORIENTATION, bStyle );
        }

        case xlBiff3:   // attributes new in BIFF3
        {
            // text wrap
            mbWrapped = bForceWrapped || GETITEMBOOL( rItemSet, ATTR_LINEBREAK );
            bUsed |= bForceWrapped | ScfTools::CheckItem( rItemSet, ATTR_LINEBREAK, bStyle );
        }

        case xlBiff2:   // attributes new in BIFF2
        {
            // horizontal alignment
            switch( GETITEMVALUE( rItemSet, SvxHorJustifyItem, ATTR_HOR_JUSTIFY, SvxCellHorJustify ) )
            {
                case SVX_HOR_JUSTIFY_STANDARD:  meHorAlign = xlHAlignGeneral;   break;
                case SVX_HOR_JUSTIFY_LEFT:      meHorAlign = xlHAlignLeft;      break;
                case SVX_HOR_JUSTIFY_CENTER:    meHorAlign = xlHAlignCenter;    break;
                case SVX_HOR_JUSTIFY_RIGHT:     meHorAlign = xlHAlignRight;     break;
                case SVX_HOR_JUSTIFY_BLOCK:     meHorAlign = xlHAlignJustify;   break;
                case SVX_HOR_JUSTIFY_REPEAT:    meHorAlign = xlHAlignFill;      break;
                default:    DBG_ERROR( "XclExpCellAlign::FillFromItemSet - unknown horizontal alignment" );
            }
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_HOR_JUSTIFY, bStyle );
        }

        break;
        default:    DBG_ERROR_BIFF();
    }

    // combine orientation/rotation
    switch( meOrient )
    {
        case xlTextOrientTopBottom: mnRotation = EXC_XF8_STACKED;   break;
        // #i4378# old calc doc's without real rotation do not have an ATTR_ROTATE_VALUE set
        case xlTextOrient90ccw:     mnRotation = 90;                break;
        case xlTextOrient90cw:      mnRotation = 180;               break;
    }
    if( meOrient == xlTextOrientNoRot )
    {
        if( (45 < mnRotation) && (mnRotation <= 90) )
            meOrient = xlTextOrient90ccw;
        else if( (135 < mnRotation) && (mnRotation <= 180) )
            meOrient = xlTextOrient90cw;
    }

    return bUsed;
}

#if 0
void XclExpCellAlign::FillToXF2( sal_uInt8& rnFlags ) const
{
    ::insert_value( rnFlags, meHorAlign, 0, 3 );
}

void XclExpCellAlign::FillToXF3( sal_uInt16& rnAlign ) const
{
    ::insert_value( rnAlign, meHorAlign, 0, 3 );
    ::set_flag( rnAlign, EXC_XF_WRAPPED, mbWrapped );
}

void XclExpCellAlign::FillToXF4( sal_uInt16& rnAlign ) const
{
    FillToXF3( rnAlign );
    ::insert_value( rnAlign, meVerAlign, 4, 2 );
    ::insert_value( rnAlign, meOrient, 6, 2 );
}
#endif

void XclExpCellAlign::FillToXF5( sal_uInt16& rnAlign ) const
{
    ::insert_value( rnAlign, meHorAlign, 0, 3 );
    ::set_flag( rnAlign, EXC_XF_WRAPPED, mbWrapped );
    ::insert_value( rnAlign, meVerAlign, 4, 3 );
    ::insert_value( rnAlign, meOrient, 8, 2 );
}

void XclExpCellAlign::FillToXF8( sal_uInt16& rnAlign, sal_uInt16& rnMiscAttrib ) const
{
    ::insert_value( rnAlign, meHorAlign, 0, 3 );
    ::set_flag( rnAlign, EXC_XF_WRAPPED, mbWrapped );
    ::insert_value( rnAlign, meVerAlign, 4, 3 );
    ::insert_value( rnAlign, mnRotation, 8, 8 );
    ::insert_value( rnMiscAttrib, mnIndent, 0, 4 );
    ::insert_value( rnMiscAttrib, meTextDir, 6, 2 );
}


// ----------------------------------------------------------------------------

XclExpCellBorder::XclExpCellBorder() :
    mnLeftColorId(   XclExpPalette::GetColorIdFromIndex( mnLeftColor ) ),
    mnRightColorId(  XclExpPalette::GetColorIdFromIndex( mnRightColor ) ),
    mnTopColorId(    XclExpPalette::GetColorIdFromIndex( mnTopColor ) ),
    mnBottomColorId( XclExpPalette::GetColorIdFromIndex( mnBottomColor ) )
{
}


void lcl_xestyle_GetBorderLine(
        sal_uInt8& rnXclLine, sal_uInt32& rnColorId,
        const SvxBorderLine* pLine, XclExpPalette& rPalette, XclBiff eBiff )
{
    rnXclLine = EXC_LINE_NONE;
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
    }
    if( (eBiff == xlBiff2) && (rnXclLine != EXC_LINE_NONE) )
        rnXclLine = EXC_LINE_THIN;

    rnColorId = (pLine && (rnXclLine != EXC_LINE_NONE)) ?
        rPalette.InsertColor( pLine->GetColor(), xlColorCellBorder ) :
        XclExpPalette::GetColorIdFromIndex( 0 );
}


bool XclExpCellBorder::FillFromItemSet( const SfxItemSet& rItemSet, XclExpPalette& rPalette, XclBiff eBiff, bool bStyle )
{
    const SvxBoxItem& rBoxItem = GETITEM( rItemSet, SvxBoxItem, ATTR_BORDER );
    lcl_xestyle_GetBorderLine( mnLeftLine,   mnLeftColorId,   rBoxItem.GetLeft(),   rPalette, eBiff );
    lcl_xestyle_GetBorderLine( mnRightLine,  mnRightColorId,  rBoxItem.GetRight(),  rPalette, eBiff );
    lcl_xestyle_GetBorderLine( mnTopLine,    mnTopColorId,    rBoxItem.GetTop(),    rPalette, eBiff );
    lcl_xestyle_GetBorderLine( mnBottomLine, mnBottomColorId, rBoxItem.GetBottom(), rPalette, eBiff );
    return ScfTools::CheckItem( rItemSet, ATTR_BORDER, bStyle );
}

void XclExpCellBorder::SetFinalColors( const XclExpPalette& rPalette )
{
    mnLeftColor   = rPalette.GetColorIndex( mnLeftColorId );
    mnRightColor  = rPalette.GetColorIndex( mnRightColorId );
    mnTopColor    = rPalette.GetColorIndex( mnTopColorId );
    mnBottomColor = rPalette.GetColorIndex( mnBottomColorId );
}

#if 0
void XclExpCellBorder::FillToXF2( sal_uInt8& rnFlags ) const
{
    ::set_flag( rnFlags, EXC_XF2_LEFTLINE,   mnLeftLine   != EXC_LINE_NONE );
    ::set_flag( rnFlags, EXC_XF2_RIGHTLINE,  mnRightLine  != EXC_LINE_NONE );
    ::set_flag( rnFlags, EXC_XF2_TOPLINE,    mnTopLine    != EXC_LINE_NONE );
    ::set_flag( rnFlags, EXC_XF2_BOTTOMLINE, mnBottomLine != EXC_LINE_NONE );
}

void XclExpCellBorder::FillToXF3( sal_uInt32& rnBorder ) const
{
    ::insert_value( rnBorder, mnTopLine,      0, 3 );
    ::insert_value( rnBorder, mnLeftLine,     8, 3 );
    ::insert_value( rnBorder, mnBottomLine,  16, 3 );
    ::insert_value( rnBorder, mnRightLine,   24, 3 );
    ::insert_value( rnBorder, mnTopColor,     3, 5 );
    ::insert_value( rnBorder, mnLeftColor,   11, 5 );
    ::insert_value( rnBorder, mnBottomColor, 19, 5 );
    ::insert_value( rnBorder, mnRightColor,  27, 5 );
}
#endif

void XclExpCellBorder::FillToXF5( sal_uInt32& rnBorder, sal_uInt32& rnArea ) const
{
    ::insert_value( rnBorder, mnTopLine,      0, 3 );
    ::insert_value( rnBorder, mnLeftLine,     3, 3 );
    ::insert_value( rnArea,   mnBottomLine,  22, 3 );
    ::insert_value( rnBorder, mnRightLine,    6, 3 );
    ::insert_value( rnBorder, mnTopColor,     9, 7 );
    ::insert_value( rnBorder, mnLeftColor,   16, 7 );
    ::insert_value( rnArea,   mnBottomColor, 25, 7 );
    ::insert_value( rnBorder, mnRightColor,  23, 7 );
}

void XclExpCellBorder::FillToXF8( sal_uInt32& rnBorder1, sal_uInt32& rnBorder2 ) const
{
    ::insert_value( rnBorder1, mnLeftLine,     0, 4 );
    ::insert_value( rnBorder1, mnRightLine,    4, 4 );
    ::insert_value( rnBorder1, mnTopLine,      8, 4 );
    ::insert_value( rnBorder1, mnBottomLine,  12, 4 );
    ::insert_value( rnBorder1, mnLeftColor,   16, 7 );
    ::insert_value( rnBorder1, mnRightColor,  23, 7 );
    ::insert_value( rnBorder2, mnTopColor,     0, 7 );
    ::insert_value( rnBorder2, mnBottomColor,  7, 7 );
}

void XclExpCellBorder::FillToCF8( sal_uInt16& rnLine, sal_uInt32& rnColor ) const
{
    ::insert_value( rnLine,  mnLeftLine,     0, 4 );
    ::insert_value( rnLine,  mnRightLine,    4, 4 );
    ::insert_value( rnLine,  mnTopLine,      8, 4 );
    ::insert_value( rnLine,  mnBottomLine,  12, 4 );
    ::insert_value( rnColor, mnLeftColor,    0, 7 );
    ::insert_value( rnColor, mnRightColor,   7, 7 );
    ::insert_value( rnColor, mnTopColor,    16, 7 );
    ::insert_value( rnColor, mnBottomColor, 23, 7 );
}


// ----------------------------------------------------------------------------

XclExpCellArea::XclExpCellArea() :
    mnForeColorId( XclExpPalette::GetColorIdFromIndex( mnForeColor ) ),
    mnBackColorId( XclExpPalette::GetColorIdFromIndex( mnBackColor ) )
{
}

bool XclExpCellArea::FillFromItemSet( const SfxItemSet& rItemSet, XclExpPalette& rPalette, XclBiff eBiff, bool bStyle )
{
    const SvxBrushItem& rBrushItem = GETITEM( rItemSet, SvxBrushItem, ATTR_BACKGROUND );
    if( rBrushItem.GetColor().GetTransparency() )
    {
        mnPattern = EXC_PATT_NONE;
        mnForeColorId = XclExpPalette::GetColorIdFromIndex( EXC_COLOR_WINDOWTEXT );
        mnBackColorId = XclExpPalette::GetColorIdFromIndex( EXC_COLOR_WINDOWBACK );
    }
    else
    {
        mnPattern = EXC_PATT_SOLID;
        mnForeColorId = rPalette.InsertColor( rBrushItem.GetColor(), xlColorCellArea );
        mnBackColorId = XclExpPalette::GetColorIdFromIndex( EXC_COLOR_WINDOWTEXT );
    }
    return ScfTools::CheckItem( rItemSet, ATTR_BACKGROUND, bStyle );
}

void XclExpCellArea::SetFinalColors( const XclExpPalette& rPalette )
{
    rPalette.GetMixedColors( mnForeColor, mnBackColor, mnPattern, mnForeColorId, mnBackColorId );
}

#if 0
void XclExpCellArea::FillToXF2( sal_uInt8& rnFlags ) const
{
    ::set_flag( rnFlags, EXC_XF2_BACKGROUND, mnPattern != EXC_PATT_NONE );
}

void XclExpCellArea::FillToXF3( sal_uInt16& rnArea ) const
{
    ::insert_value( rnArea, mnPattern,    0, 6 );
    ::insert_value( rnArea, mnForeColor,  6, 5 );
    ::insert_value( rnArea, mnBackColor, 11, 5 );
}
#endif

void XclExpCellArea::FillToXF5( sal_uInt32& rnArea ) const
{
    ::insert_value( rnArea, mnPattern,   16, 6 );
    ::insert_value( rnArea, mnForeColor,  0, 7 );
    ::insert_value( rnArea, mnBackColor,  7, 7 );
}

void XclExpCellArea::FillToXF8( sal_uInt32& rnBorder2, sal_uInt16& rnArea ) const
{
    ::insert_value( rnBorder2, mnPattern,   26, 6 );
    ::insert_value( rnArea,    mnForeColor,  0, 7 );
    ::insert_value( rnArea,    mnBackColor,  7, 7 );
}

void XclExpCellArea::FillToCF8( sal_uInt16& rnPattern, sal_uInt16& rnColor ) const
{
    XclCellArea aTmp( *this );
    if( !aTmp.IsTransparent() && (aTmp.mnBackColor == EXC_COLOR_WINDOWTEXT) )
        aTmp.mnBackColor = 0;
    if( aTmp.mnPattern == EXC_PATT_SOLID )
        ::std::swap( aTmp.mnForeColor, aTmp.mnBackColor );
    ::insert_value( rnColor,   aTmp.mnForeColor,  0, 7 );
    ::insert_value( rnColor,   aTmp.mnBackColor,  7, 7 );
    ::insert_value( rnPattern, aTmp.mnPattern,   10, 6 );
}


// ----------------------------------------------------------------------------

XclExpXF::XclExpXF(
        const XclExpRoot& rRoot, const ScPatternAttr& rPattern,
        sal_uInt32 nForcedNumFmt, bool bForceWrapped, sal_uInt16 nFontIx) :
    XclXFBase( true ),
    XclExpRoot( rRoot )
{
    mnParentXFId = GetXFBuffer().InsertStyle( rPattern.GetStyleSheet() );
    Init( rPattern.GetItemSet(), nForcedNumFmt, bForceWrapped, nFontIx, false  );
}

XclExpXF::XclExpXF( const XclExpRoot& rRoot, const SfxStyleSheetBase& rStyleSheet ) :
    XclXFBase( false ),
    XclExpRoot( rRoot ),
    mnParentXFId( XclExpXFBuffer::GetXFIdFromIndex( EXC_XF_STYLEPARENT ) )
{
    bool bDefStyle = (rStyleSheet.GetName() == ScGlobal::GetRscString( STR_STYLENAME_STANDARD ));
    Init( const_cast< SfxStyleSheetBase& >( rStyleSheet ).GetItemSet(), NUMBERFORMAT_ENTRY_NOT_FOUND, false, EXC_FONT_NOTFOUND, bDefStyle );
}

XclExpXF::XclExpXF( const XclExpRoot& rRoot, bool bCellXF ) :
    XclXFBase( bCellXF ),
    XclExpRoot( rRoot ),
    mnParentXFId( XclExpXFBuffer::GetXFIdFromIndex( EXC_XF_STYLEPARENT ) )
{
    InitDefault();
}

bool XclExpXF::Equals( const ScPatternAttr& rPattern, sal_uInt32 nForcedNumFmt, bool bForceWrapped, sal_uInt16 nFontIx) const
{
    return IsCellXF() && (mpItemSet == &rPattern.GetItemSet()) &&
        ((nForcedNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND) || (mnNumFmt == nForcedNumFmt)) &&
        (!bForceWrapped || maAlignment.mbWrapped) &&
        ((nFontIx == EXC_FONT_NOTFOUND) || (mnFont == nFontIx)) ;
}

bool XclExpXF::Equals( const SfxStyleSheetBase& rStyleSheet ) const
{
    return IsStyleXF() && (mpItemSet == &const_cast< SfxStyleSheetBase& >( rStyleSheet ).GetItemSet());
}

void XclExpXF::SetFinalColors()
{
    maBorder.SetFinalColors( GetPalette() );
    maArea.SetFinalColors( GetPalette() );
}

bool XclExpXF::Equals( const XclExpXF& rCmpXF ) const
{
    return XclXFBase::Equals( rCmpXF ) &&
        (maProtection == rCmpXF.maProtection) && (maAlignment  == rCmpXF.maAlignment) &&
        (maBorder     == rCmpXF.maBorder)     && (maArea       == rCmpXF.maArea)      &&
        (mnFont       == rCmpXF.mnFont)       && (mnNumFmt     == rCmpXF.mnNumFmt)    &&
        (mnParentXFId == rCmpXF.mnParentXFId);
}

void XclExpXF::InitDefault()
{
    SetRecHeader( EXC_ID_XF, (GetBiff() == xlBiff8) ? 20 : 16 );
    mpItemSet = NULL;
    mnFont = mnNumFmt = 0;
}

void XclExpXF::Init( const SfxItemSet& rItemSet, sal_uInt32 nForcedNumFmt, bool bForceWrapped, sal_uInt16 nFontIx, bool bDefStyle )
{
    InitDefault();
    mpItemSet = &rItemSet;

    // cell protection
    mbProtUsed = maProtection.FillFromItemSet( rItemSet, GetBiff(), IsStyleXF() );

    // font
    if(nFontIx == EXC_FONT_NOTFOUND)
    {
        mnFont = GetFontBuffer().Insert( rItemSet, bDefStyle );
        mbFontUsed =
            ScfTools::CheckItem( rItemSet, ATTR_FONT_HEIGHT,     IsStyleXF() ) ||
            ScfTools::CheckItem( rItemSet, ATTR_FONT_WEIGHT,     IsStyleXF() ) ||
            ScfTools::CheckItem( rItemSet, ATTR_FONT_POSTURE,    IsStyleXF() ) ||
            ScfTools::CheckItem( rItemSet, ATTR_FONT_UNDERLINE,  IsStyleXF() ) ||
            ScfTools::CheckItem( rItemSet, ATTR_FONT_CROSSEDOUT, IsStyleXF() ) ||
            ScfTools::CheckItem( rItemSet, ATTR_FONT_CONTOUR,    IsStyleXF() ) ||
            ScfTools::CheckItem( rItemSet, ATTR_FONT_SHADOWED,   IsStyleXF() ) ||
            ScfTools::CheckItem( rItemSet, ATTR_FONT_COLOR,      IsStyleXF() ) ||
            ScfTools::CheckItem( rItemSet, ATTR_FONT_LANGUAGE,   IsStyleXF() );
    }
    else
    {
        mnFont = nFontIx;
        mbFontUsed = true;
    }

    // number format
    sal_uInt32 nScFmt = (nForcedNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND) ?
        GETITEMVALUE( rItemSet, SfxUInt32Item, ATTR_VALUE_FORMAT, sal_uInt32 ) : nForcedNumFmt;
    mnNumFmt = GetNumFmtBuffer().Insert( nScFmt );
    mbFmtUsed = ScfTools::CheckItem( rItemSet, ATTR_VALUE_FORMAT, IsStyleXF() );

    // alignment
    mbAlignUsed = maAlignment.FillFromItemSet( rItemSet, GetBiff(), bForceWrapped, IsStyleXF() );

    // cell border
    mbBorderUsed = maBorder.FillFromItemSet( rItemSet, GetPalette(), GetBiff(), IsStyleXF() );

    // background area
    mbAreaUsed = maArea.FillFromItemSet( rItemSet, GetPalette(), GetBiff(), IsStyleXF() );

    // set all b***Used flags to true in "Default"/"Normal" style
    if( bDefStyle )
        SetAllUsedFlags( true );
}

sal_uInt8 XclExpXF::GetUsedFlags() const
{
    sal_uInt8 nUsedFlags = 0;
    /*  In cell XFs a set bit means a used attribute, in style XFs a cleared bit.
        "mbCellXF == mb***Used" evaluates to correct value in cell and style XFs. */
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
    sal_uInt16 nTypeProt = 0, nAlign = 0;
    sal_uInt32 nArea = 0, nBorder = 0;

    ::set_flag( nTypeProt, EXC_XF_STYLE, IsStyleXF() );
    ::insert_value( nTypeProt, mnParent, 4, 12 );
    ::insert_value( nAlign, GetUsedFlags(), 10, 6 );

    maProtection.FillToXF3( nTypeProt );
    maAlignment.FillToXF5( nAlign );
    maBorder.FillToXF5( nBorder, nArea );
    maArea.FillToXF5( nArea );

    rStrm << mnFont << mnNumFmt << nTypeProt << nAlign << nArea << nBorder;
}

void XclExpXF::WriteBody8( XclExpStream& rStrm )
{
    sal_uInt16 nTypeProt = 0, nAlign = 0, nMiscAttrib = 0, nArea = 0;
    sal_uInt32 nBorder1 = 0, nBorder2 = 0;

    ::set_flag( nTypeProt, EXC_XF_STYLE, IsStyleXF() );
    ::insert_value( nTypeProt, mnParent, 4, 12 );
    ::insert_value( nMiscAttrib, GetUsedFlags(), 10, 6 );

    maProtection.FillToXF3( nTypeProt );
    maAlignment.FillToXF8( nAlign, nMiscAttrib );
    maBorder.FillToXF8( nBorder1, nBorder2 );
    maArea.FillToXF8( nBorder2, nArea );

    rStrm << mnFont << mnNumFmt << nTypeProt << nAlign << nMiscAttrib << nBorder1 << nBorder2 << nArea;
}

void XclExpXF::WriteBody( XclExpStream& rStrm )
{
    mnParent = GetXFBuffer().GetXFIndex( mnParentXFId );
    switch( GetBiff() )
    {
        case xlBiff5:
        case xlBiff7:   WriteBody5( rStrm );    break;
        case xlBiff8:   WriteBody8( rStrm );    break;
        default:        DBG_ERROR_BIFF();
    }
}


// ----------------------------------------------------------------------------

XclExpDefaultXF::XclExpDefaultXF( const XclExpRoot& rRoot, bool bCellXF ) :
    XclExpXF( rRoot, bCellXF )
{
}

void XclExpDefaultXF::SetParent( sal_uInt32 nParentXFId )
{
    DBG_ASSERT( IsCellXF(), "XclExpDefaultXF::SetParent - not allowed in style XFs" );
    if( IsCellXF() )
        mnParentXFId = nParentXFId;
}

void XclExpDefaultXF::SetUsedFlags(
        bool bProtUsed, bool bFontUsed, bool bFmtUsed,
        bool bAlignUsed, bool bBorderUsed, bool bAreaUsed )
{
    mbProtUsed    = bProtUsed;
    mbFontUsed    = bFontUsed;
    mbFmtUsed     = bFmtUsed;
    mbAlignUsed   = bAlignUsed;
    mbBorderUsed  = bBorderUsed;
    mbAreaUsed    = bAreaUsed;
}

void XclExpDefaultXF::SetProtection( const XclExpCellProt& rProtection )
{
    maProtection = rProtection;
    mbProtUsed = true;
}

void XclExpDefaultXF::SetFont( sal_uInt16 nFont )
{
    mnFont = nFont;
    mbFontUsed = true;
}

void XclExpDefaultXF::SetNumFmt( sal_uInt16 nNumFmt )
{
    mnNumFmt = nNumFmt;
    mbFmtUsed = true;
}

void XclExpDefaultXF::SetAlignment( const XclExpCellAlign& rAlignment )
{
    maAlignment = rAlignment;
    mbAlignUsed = true;
}

void XclExpDefaultXF::SetBorder( const XclExpCellBorder& rBorder )
{
    maBorder = rBorder;
    mbBorderUsed = true;
}

void XclExpDefaultXF::SetArea( const XclExpCellArea& rArea )
{
    maArea = rArea;
    mbAreaUsed = true;
}


// ----------------------------------------------------------------------------

XclExpStyle::XclExpStyle( sal_uInt32 nXFId, const String& rStyleName ) :
    XclExpRecord( EXC_ID_STYLE, 4 ),
    maName( rStyleName ),
    mnXFId( nXFId ),
    mnStyleId( EXC_STYLE_USERDEF ),
    mnLevel( EXC_STYLE_NOLEVEL )
{
    DBG_ASSERT( maName.Len(), "XclExpStyle::XclExpStyle - empty style name" );
#ifdef DBG_UTIL
    sal_uInt8 nStyleId, nLevel; // do not use members for debug tests
    DBG_ASSERT( !XclTools::GetBuiltInStyleId( nStyleId, nLevel, maName ),
        "XclExpStyle::XclExpStyle - this is a built-in style" );
#endif
}

XclExpStyle::XclExpStyle( sal_uInt32 nXFId, sal_uInt8 nStyleId, sal_uInt8 nLevel ) :
    XclExpRecord( EXC_ID_STYLE, 4 ),
    mnXFId( nXFId ),
    mnStyleId( nStyleId ),
    mnLevel( nLevel )
{
}

void XclExpStyle::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nXFIndex = rStrm.GetRoot().GetXFBuffer().GetXFIndex( mnXFId );
    ::set_flag( nXFIndex, EXC_STYLE_BUILTIN, IsBuiltIn() );
    rStrm << nXFIndex;

    if( IsBuiltIn() )
    {
        rStrm << mnStyleId << mnLevel;
    }
    else
    {
        XclExpString aNameEx;
        if( rStrm.GetRoot().GetBiff() >= xlBiff8 )
            aNameEx.Assign( maName );
        else
            aNameEx.AssignByte( maName, rStrm.GetRoot().GetCharSet(), EXC_STR_8BITLENGTH );
        rStrm << aNameEx;
    }
}


// ----------------------------------------------------------------------------

const sal_uInt32 EXC_XFLIST_INDEXBASE   = 0xFFFE0000;
const sal_uInt32 EXC_XFLIST_NOTFOUND    = LIST_ENTRY_NOTFOUND;
/** Maximum count of XF records to store in the XF list (performance). */
const sal_uInt32 EXC_XFLIST_HARDLIMIT   = 256 * 1024;


bool lcl_xestyle_IsBuiltInStyle( const String& rStyleName )
{
    return
        XclTools::IsBuiltInStyleName( rStyleName ) ||
        XclTools::IsCondFormatStyleName( rStyleName );
}


XclExpXFBuffer::XclExpXFBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

void XclExpXFBuffer::InitDefaults()
{
    InsertDefaultRecords();
    InsertUserStyles();
}

sal_uInt32 XclExpXFBuffer::Insert( const ScPatternAttr* pPattern, bool bForceWrapped, sal_uInt16 nFontIx )
{
    return InsertCellXF( pPattern, NUMBERFORMAT_ENTRY_NOT_FOUND, bForceWrapped, nFontIx );
}

sal_uInt32 XclExpXFBuffer::Insert( const ScPatternAttr* pPattern, sal_uInt32 nForcedNumFmt )
{
    return InsertCellXF( pPattern, nForcedNumFmt, false, EXC_FONT_NOTFOUND );
}

sal_uInt32 XclExpXFBuffer::InsertStyle( const SfxStyleSheetBase* pStyleSheet )
{
    return pStyleSheet ? InsertStyleXF( *pStyleSheet ) : GetXFIdFromIndex( EXC_XF_DEFAULTSTYLE );
}

sal_uInt32 XclExpXFBuffer::GetXFIdFromIndex( sal_uInt16 nXFIndex )
{
    return EXC_XFLIST_INDEXBASE | nXFIndex;
}

void XclExpXFBuffer::Reduce()
{
    for( XclExpXF* pXF = maXFList.First(); pXF; pXF = maXFList.Next() )
        pXF->SetFinalColors();

    sal_uInt32 nCount = maXFList.Count();
    sal_uInt32 nId;
    maXFIndexVec.resize( nCount, EXC_XF_DEFAULTCELL );
    maXFPtrVec.reserve( ::std::min( nCount, EXC_XF_MAXCOUNT ) );

// --- map all built-in XF records, without changing their order ---

    for( nId = 0; nId < EXC_XF_USEROFFSET; ++nId )
        AppendXFIndex( nId );

// --- insert all style XF records, without reduce ---

    sal_uInt32 nStyleXFCount = 0;       // counts up to EXC_XF_MAXSTYLECOUNT limit

    for( nId = EXC_XF_USEROFFSET; nId < nCount; ++nId )
    {
        const XclExpXF* pXF = maXFList.GetObject( nId );
        if( pXF->IsStyleXF() )
        {
            if( nStyleXFCount < EXC_XF_MAXSTYLECOUNT )
            {
                // maximum count of styles not reached
                AppendXFIndex( nId );
                ++nStyleXFCount;
            }
            else
            {
                /*  Maximum count of styles reached - do not append more
                    pointers to XFs; use default style XF instead; do not break
                    the loop to initialize all maXFIndexVec elements. */
                maXFIndexVec[ nId ] = EXC_XF_DEFAULTSTYLE;
            }
        }
    }

// --- insert all cell XF records ---

    // start iterator to search for equal inserted XF records
    XclExpXFPtrVec::const_iterator aSearchStartIter = maXFPtrVec.end();

    // break the loop if XF limit reached - maXFIndexVec is already initialized with default index
    for( nId = EXC_XF_USEROFFSET; (nId < nCount) && (maXFPtrVec.size() < EXC_XF_MAXCOUNT); ++nId )
    {
        const XclExpXF* pXF = maXFList.GetObject( nId );
        if( pXF->IsCellXF() )
        {
            // try to find an XF record equal to *pXF, which is already inserted
            sal_uInt32 nFoundIndex = LIST_ENTRY_NOTFOUND;
            for( XclExpXFPtrVec::const_iterator aIter = aSearchStartIter, aEnd = maXFPtrVec.end();
                    (aIter != aEnd) && (nFoundIndex == LIST_ENTRY_NOTFOUND); ++aIter )
            {
                if( (**aIter).Equals( *pXF ) )
                    nFoundIndex = aIter - maXFPtrVec.begin();
            }

            if( nFoundIndex != LIST_ENTRY_NOTFOUND )
                // equal XF already in the list, use its resulting XF index
                maXFIndexVec[ nId ] = static_cast< sal_uInt16 >( nFoundIndex );
            else
                AppendXFIndex( nId );
        }
    }
}

sal_uInt16 XclExpXFBuffer::GetXFIndex( sal_uInt32 nXFId ) const
{
    sal_uInt16 nXFIndex = EXC_XF_DEFAULTSTYLE;
    if( nXFId >= EXC_XFLIST_INDEXBASE )
        nXFIndex = static_cast< sal_uInt16 >( nXFId & ~EXC_XFLIST_INDEXBASE );
    else if( nXFId < maXFIndexVec.size() )
        nXFIndex = maXFIndexVec[ nXFId ];
    return nXFIndex;
}

void XclExpXFBuffer::Save( XclExpStream& rStrm )
{
    // save all XF records contained in the maXFPtrVec vector
    for( XclExpXFPtrVec::iterator aIter = maXFPtrVec.begin(), aEnd = maXFPtrVec.end(); aIter != aEnd; ++aIter )
        (**aIter).Save( rStrm );
    // save all STYLE records
    maStyleList.Save( rStrm );
}

sal_uInt32 XclExpXFBuffer::FindXF( const ScPatternAttr& rPattern, sal_uInt32 nForcedNumFmt, bool bForceWrapped, sal_uInt16 nFontIx ) const
{
    for( const XclExpXF* pXF = maXFList.First(); pXF; pXF = maXFList.Next() )
        if( pXF->Equals( rPattern, nForcedNumFmt, bForceWrapped, nFontIx ) )
            return maXFList.GetCurPos();
    return EXC_XFLIST_NOTFOUND;
}

sal_uInt32 XclExpXFBuffer::FindXF( const SfxStyleSheetBase& rStyleSheet ) const
{
    for( const XclExpXF* pXF = maXFList.First(); pXF; pXF = maXFList.Next() )
        if( pXF->Equals( rStyleSheet ) )
            return maXFList.GetCurPos();
    return EXC_XFLIST_NOTFOUND;
}

sal_uInt32 XclExpXFBuffer::FindStyle( sal_uInt32 nXFId ) const
{
    for( const XclExpStyle* pStyle = maStyleList.First(); pStyle; pStyle = maStyleList.Next() )
        if( pStyle->GetXFId() == nXFId )
            return maStyleList.GetCurPos();
    return EXC_XFLIST_NOTFOUND;
}

sal_uInt32 XclExpXFBuffer::InsertCellXF(
        const ScPatternAttr* pPattern, sal_uInt32 nForcedNumFmt, bool bForceWrapped, sal_uInt16 nFontIx )
{
    const ScPatternAttr* pDefPattern = GetDoc().GetDefPattern();

    // special handling for default cell formatting
    if( !pPattern || (pPattern == pDefPattern) )
    {
        if( maPredefined[ EXC_XF_DEFAULTCELL ] )
        {
            maPredefined[ EXC_XF_DEFAULTCELL ] = false;
            // replace default cell pattern
            maXFList.Replace( new XclExpXF( GetRoot(), *pDefPattern, nForcedNumFmt, bForceWrapped ), EXC_XF_DEFAULTCELL );
        }
        return GetXFIdFromIndex( EXC_XF_DEFAULTCELL );
    }

    sal_uInt32 nXFId = FindXF( *pPattern, nForcedNumFmt, bForceWrapped, nFontIx);
    if( nXFId == EXC_XFLIST_NOTFOUND )
    {
        // not found - insert new cell XF
        if( maXFList.Count() < EXC_XFLIST_HARDLIMIT )
        {
            maXFList.Append( new XclExpXF( GetRoot(), *pPattern, nForcedNumFmt, bForceWrapped, nFontIx) );
            // do not set nIndex before the Append() command - it may insert 2 XFs (style+cell)
            nXFId = maXFList.Count() - 1;
        }
        else
            // list full - fall back to default cell XF
            nXFId = GetXFIdFromIndex( EXC_XF_DEFAULTCELL );
    }
    return nXFId;
}

sal_uInt32 XclExpXFBuffer::InsertStyleXF( const SfxStyleSheetBase& rStyleSheet )
{
    // try, if it is a predefined built-in style - replace existing XF record
    sal_uInt8 nStyleId, nLevel;
    if( XclTools::GetBuiltInStyleId( nStyleId, nLevel, rStyleSheet.GetName() ) )
    {
        sal_uInt16 nXFIndex = XclTools::GetBuiltInXFIndex( nStyleId, nLevel );
        DBG_ASSERT( maXFList.GetObject( nXFIndex ), "XclExpXFBuffer::InsertStyleXF - built-in XF not found" );
        if( maPredefined[ nXFIndex ] )
        {
            maPredefined[ nXFIndex ] = false;
            // replace predefined built-in style (Replace() deletes old object)
            maXFList.Replace( new XclExpXF( GetRoot(), rStyleSheet ), nXFIndex );
        }

        sal_uInt32 nXFId = GetXFIdFromIndex( nXFIndex );

        // STYLE already inserted? (No: i.e. for RowLevel/ColLevel styles or Hyperlink/Followed_Hyperlink)
        if( FindStyle( nXFId ) == LIST_ENTRY_NOTFOUND )
            maStyleList.Append( new XclExpStyle( nXFId, nStyleId, nLevel ) );

        return nXFId;
    }

    // try to find the XF record of a user-defined style
    sal_uInt32 nXFId = FindXF( rStyleSheet );
    if( nXFId == EXC_XFLIST_NOTFOUND )
    {
        // not found - insert new style XF and STYLE
        nXFId = maXFList.Count();
        if( nXFId < EXC_XFLIST_HARDLIMIT )
        {
            maXFList.Append( new XclExpXF( GetRoot(), rStyleSheet ) );
            // create the STYLE record
            if( rStyleSheet.GetName().Len() )
                maStyleList.Append( new XclExpStyle( nXFId, rStyleSheet.GetName() ) );
        }
        else
            // list full - fall back to default style XF
            nXFId = GetXFIdFromIndex( EXC_XF_DEFAULTSTYLE );
    }
    return nXFId;
}

void XclExpXFBuffer::InsertUserStyles()
{
    SfxStyleSheetIterator aStyleIter( GetDoc().GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
    for( SfxStyleSheetBase* pStyleSheet = aStyleIter.First(); pStyleSheet; pStyleSheet = aStyleIter.Next() )
        if( pStyleSheet->IsUserDefined() && !lcl_xestyle_IsBuiltInStyle( pStyleSheet->GetName() ) )
            InsertStyleXF( *pStyleSheet );
}

void XclExpXFBuffer::InsertDefaultStyle( XclExpXF* pXF, sal_uInt8 nStyleId, sal_uInt8 nLevel )
{
    // XF ID must be a built-in ID - get it using current list size
    sal_uInt32 nXFId = GetXFIdFromIndex( static_cast< sal_uInt16 >( maXFList.Count() ) );
    maStyleList.Append( new XclExpStyle( nXFId, nStyleId, nLevel ) );
    maXFList.Append( pXF );
}

void XclExpXFBuffer::InsertDefaultRecords()
{
    maPredefined.clear();
    maPredefined.resize( EXC_XF_USEROFFSET, true );

    if( SfxStyleSheetBase* pDefStyleSheet = GetStyleSheetPool().Find( ScGlobal::GetRscString( STR_STYLENAME_STANDARD ), SFX_STYLE_FAMILY_PARA ) )
    {
        maXFList.Append( new XclExpXF( GetRoot(), *pDefStyleSheet ) );
        maPredefined[ EXC_STYLE_NORMAL ] = false;
    }
    else
    {
        DBG_ERRORFILE( "XclExpXFBuffer::InsertDefaultRecords - Default style not found" );
        XclExpDefaultXF* pDefStyle = new XclExpDefaultXF( GetRoot(), false );
        pDefStyle->SetAllUsedFlags( true );
        InsertDefaultStyle( pDefStyle, EXC_STYLE_NORMAL );
    }

    XclExpDefaultXF aLevelStyle( GetRoot(), false );
    aLevelStyle.SetFont( 1 );
    maXFList.Append( new XclExpDefaultXF( aLevelStyle ) );      // RowLevel_1
    maXFList.Append( new XclExpDefaultXF( aLevelStyle ) );      // ColLevel_1
    aLevelStyle.SetFont( 2 );
    maXFList.Append( new XclExpDefaultXF( aLevelStyle ) );      // RowLevel_2
    maXFList.Append( new XclExpDefaultXF( aLevelStyle ) );      // ColLevel_2
    aLevelStyle.SetFont( 0 );
    for( int nIndex = 4; nIndex < 2 * EXC_STYLE_LEVELCOUNT; ++nIndex )
        maXFList.Append( new XclExpDefaultXF( aLevelStyle ) );  // (Row|Col)Level_(3-7)

    // default hard cell format
    maXFList.Append( new XclExpDefaultXF( GetRoot(), true ) );

    // other built-in styles
    XclExpDefaultXF aFormatStyle( GetRoot(), false );
    aFormatStyle.SetNumFmt( 43 );
    InsertDefaultStyle( new XclExpDefaultXF( aFormatStyle ), EXC_STYLE_COMMA );
    aFormatStyle.SetNumFmt( 41 );
    InsertDefaultStyle( new XclExpDefaultXF( aFormatStyle ), EXC_STYLE_COMMA_0 );
    aFormatStyle.SetNumFmt( 44 );
    InsertDefaultStyle( new XclExpDefaultXF( aFormatStyle ), EXC_STYLE_CURRENCY );
    aFormatStyle.SetNumFmt( 42 );
    InsertDefaultStyle( new XclExpDefaultXF( aFormatStyle ), EXC_STYLE_CURRENCY_0 );
    aFormatStyle.SetNumFmt( 9 );
    InsertDefaultStyle( new XclExpDefaultXF( aFormatStyle ), EXC_STYLE_PERCENT );

    // Create two placeholders for possible Hyperlink/Followed_Hyperlink styles
    aFormatStyle.SetNumFmt( 0 );
    aFormatStyle.SetAllUsedFlags( false );
    maXFList.Append( new XclExpDefaultXF( aFormatStyle ) );
    maXFList.Append( new XclExpDefaultXF( aFormatStyle ) );

    // default hard cell format -> NULL is document default pattern
    // do it here to really have all built-in styles
    Insert( NULL );
}

void XclExpXFBuffer::AppendXFIndex( sal_uInt32 nXFId )
{
    DBG_ASSERT( nXFId < maXFIndexVec.size(), "XclExpXFBuffer::AppendXFIndex - XF ID out of range" );
    maXFIndexVec[ nXFId ] = static_cast< sal_uInt16 >( maXFPtrVec.size() );
    maXFPtrVec.push_back( maXFList.GetObject( nXFId ) );
    DBG_ASSERT( maXFPtrVec.back(), "XclExpXFBuffer::AppendXFIndex - XF not found" );
}

// ============================================================================

