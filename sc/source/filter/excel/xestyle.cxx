/*************************************************************************
 *
 *  $RCSfile: xestyle.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dr $ $Date: 2002-12-06 16:37:46 $
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
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCPATATR_HXX
#include "patattr.hxx"
#endif

#ifndef SC_XEHELPER_HXX
#include "xehelper.hxx"
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
    @descr  Used to remap the CID data vector from list indexes to palette indexes. */
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

XclExpPalette::XclListColor::XclListColor( const Color& rColor, sal_uInt32 nCID ) :
    maColor( rColor ),
    mnCID( nCID ),
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
    mnLastIx( 0 )
{
    SetRecSize( GetColorCount() * 4 + 2 );
    InsertColor( Color( COL_BLACK ), xlColorCellText );
    SetBiff( rRoot.GetBiff() );
}

void XclExpPalette::SetBiff( XclBiff eBiff )
{
    XclDefaultPalette::SetBiff( eBiff );

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
    return pEntry->GetCID();
}

sal_uInt32 XclExpPalette::InsertIndex( sal_uInt16 nIndex )
{
    return EXC_PAL_INDEXBASE | nIndex;
}

void XclExpPalette::ReduceColors()
{
// --- build initial CID data vector (maCIDDataVec) ---

    sal_uInt32 nCount = maColorList.Count();
    maCIDDataVec.resize( nCount );
    for( sal_uInt32 nIx = 0; nIx < nCount; ++nIx )
    {
        XclListColor* pListColor = maColorList.GetObject( nIx );
        maCIDDataVec[ pListColor->GetCID() ].Set( pListColor->GetColor(), nIx );
    }

// --- loop as long as current color count does not fit into palette of current BIFF ---

    // CID data vector (maCIDDataVec) will map CIDs to color list entries (maColorList)
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

    // remap CID data map (maCIDDataVec) from list indexes to palette indexes
    for( XclCIDDataVec::iterator aIter = maCIDDataVec.begin(), aEnd = maCIDDataVec.end();
            aIter != aEnd; ++aIter )
        aIter->mnIndex = aRemapVec[ aIter->mnIndex ].mnPalIndex;
}

sal_uInt16 XclExpPalette::GetColorIndex( sal_uInt32 nCID ) const
{
    sal_uInt16 nRet = 0;
    if( nCID >= EXC_PAL_INDEXBASE )
        nRet = static_cast< sal_uInt16 >( nCID & ~EXC_PAL_INDEXBASE );
    else if( nCID < maCIDDataVec.size() )
        nRet = GetXclIndex( maCIDDataVec[ nCID ].mnIndex );
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
        sal_uInt16& rnXclForeIx, sal_uInt16& rnXclBackIx, sal_uInt16& rnXclPattern,
        sal_uInt32 nCIDForegr, sal_uInt32 nCIDBackgr ) const
{
    if( (nCIDForegr >= EXC_PAL_INDEXBASE) || (nCIDBackgr >= EXC_PAL_INDEXBASE) || (rnXclPattern != EXC_PATT_SOLID) )
    {
        rnXclForeIx = GetColorIndex( nCIDForegr );
        rnXclBackIx = GetColorIndex( nCIDBackgr );
        return;
    }

    rnXclForeIx = rnXclBackIx = 0;
    if( nCIDForegr >= maCIDDataVec.size() )
        return;

    sal_uInt32 nIndex1, nIndex2;
    Color aForeColor( GetOriginalColor( nCIDForegr ) );
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

const Color& XclExpPalette::GetOriginalColor( sal_uInt32 nCID ) const
{
    if( nCID < maCIDDataVec.size() )
        return maCIDDataVec[ nCID ].maColor;
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

        // recalculate CID data map (maps CIDs to color list indexes)
        XclCIDDataVec::iterator aEnd = maCIDDataVec.end();
        for( XclCIDDataVec::iterator aIter = maCIDDataVec.begin(); aIter != aEnd; ++aIter )
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
    mnCID( rRoot.GetPalette().InsertIndex( EXC_FONT_AUTOCOLOR ) ),
    mnHash( 0 ),
    mbHasColor( false )
{
}

void XclExpFont::SetName( const String& rName )
{
    maData.maName = rName;
    sal_uInt32 nRecSize = maData.maName.Len();
    if( GetBiff() >= xlBiff8 )
        (nRecSize *= 2) += 1;
    nRecSize += 15;
    SetRecSize( nRecSize );
}

void XclExpFont::SetColor( const Color& rColor )
{
    SetCID( GetPalette().InsertColor( rColor, xlColorCellText, EXC_FONT_AUTOCOLOR ) );
}

void XclExpFont::SetCID( sal_uInt32 nCID )
{
    mnCID = nCID;
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
    SetOutline( rFont.IsOutline() == TRUE );
    SetShadow( rFont.IsShadow() == TRUE );
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
        (!mbHasColor || !rCmp.mbHasColor || (mnCID == rCmp.mnCID)) &&
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
    switch( eCharSet )
    {
        case RTL_TEXTENCODING_DONTKNOW:     return EXC_FONTCSET_DONTKNOW;
        case RTL_TEXTENCODING_MS_1252:      return EXC_FONTCSET_MS_1252;
        case RTL_TEXTENCODING_APPLE_ROMAN:  return EXC_FONTCSET_APPLE_ROMAN;
        case RTL_TEXTENCODING_IBM_437:      return EXC_FONTCSET_IBM_437;
        case RTL_TEXTENCODING_IBM_850:      return EXC_FONTCSET_IBM_850;
        case RTL_TEXTENCODING_IBM_860:      return EXC_FONTCSET_IBM_860;
        case RTL_TEXTENCODING_IBM_861:      return EXC_FONTCSET_IBM_861;
        case RTL_TEXTENCODING_IBM_863:      return EXC_FONTCSET_IBM_863;
        case RTL_TEXTENCODING_IBM_865:      return EXC_FONTCSET_IBM_865;
        case (rtl_TextEncoding) 9:          return EXC_FONTCSET_SYSTEM;
        case RTL_TEXTENCODING_SYMBOL:       return EXC_FONTCSET_SYMBOL;
    }
    return EXC_FONTCSET_DONTKNOW;
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
            << GetPalette().GetColorIndex( mnCID )
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
    mnMaxCount( 0 )
{
    SetBiff( GetBiff() );
}

void XclExpFontBuffer::SetBiff( XclBiff eBiff )
{
    DBG_ASSERT( maFontList.Empty(), "XclExpFontBuffer::SetBiff - call is too late" );
    switch( eBiff )
    {
        case xlBiff4:   mnMaxCount = EXC_FONT_MAXCOUNT4;    break;
        case xlBiff5:
        case xlBiff7:   mnMaxCount = EXC_FONT_MAXCOUNT5;    break;
        case xlBiff8:   mnMaxCount = EXC_FONT_MAXCOUNT8;    break;
        default:        DBG_ERROR_BIFF();
    }
}

sal_uInt16 XclExpFontBuffer::GetIndex( const XclExpFont& rFont, sal_uInt16 nDefault )
{
    sal_uInt32 nIndex = Find( rFont );
    return (nIndex < mnMaxCount) ? GetXclIndex( nIndex ) : nDefault;
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
                pOldFont->SetCID( rpFont->GetCID() );
            delete rpFont;
            rpFont = pOldFont;
        }
        else if( nCount < mnMaxCount )
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
    XclExpFont* pNewFont = new XclExpFont( *this );
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
    XclExpFont aFont( *this );
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

sal_uInt16 XclExpFontBuffer::GetXclIndex( sal_uInt32 nIndex )
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
    mpFormatter( new SvNumberFormatter( rRoot.GetDoc().GetServiceManager(), LANGUAGE_ENGLISH_US ) )
{
    SetBiff( GetBiff() );

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

void XclExpNumFmtBuffer::SetBiff( XclBiff eBiff )
{
    DBG_ASSERT( maFormatMap.empty(), "XclExpNumFmtBuffer::SetBiff - call is too late" );
    switch( eBiff )
    {
        case xlBiff5:
        case xlBiff7:   mnXclOffset = EXC_FORMAT_OFFSET5;   break;
        case xlBiff8:   mnXclOffset = EXC_FORMAT_OFFSET8;   break;
        default:        DBG_ERROR_BIFF();
    }
}

sal_uInt16 XclExpNumFmtBuffer::Insert( sal_uInt32 nFormatIx )
{
    XclExpNumFmtVec::const_iterator aIter =
        ::std::find_if( maFormatMap.begin(), maFormatMap.end(), XclExpNumFmtPred( nFormatIx ) );
    if( aIter != maFormatMap.end() )
        return aIter->mnXclIx;

    sal_uInt32 nSize = maFormatMap.size();
    if( nSize < 0xFFFFUL - mnXclOffset )
    {
        sal_uInt16 nXclIx = static_cast< sal_uInt16 >( nSize + mnXclOffset );
        maFormatMap.push_back( XclExpNumFmt( nFormatIx, nXclIx ) );
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
    const XclExpDefaultFormat* pEnd = pDefaultFormats + STATIC_TABLE_SIZE( pDefaultFormats );
    for( const XclExpDefaultFormat* pCurr = pDefaultFormats; pCurr != pEnd; ++pCurr )
        WriteFormatRecord( rStrm, pCurr->mnIndex, String( pCurr->mpFormat, RTL_TEXTENCODING_UTF8 ) );
}


// XF, STYLE record - Cell formatting =========================================

// ============================================================================

