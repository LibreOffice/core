/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <memory>
#include <utility>
#include <xestyle.hxx>

#include <algorithm>
#include <iterator>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <comphelper/processfactory.hxx>
#include <rtl/tencinfo.h>
#include <vcl/font.hxx>
#include <svl/languageoptions.hxx>
#include <scitems.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/langitem.hxx>
#include <document.hxx>
#include <stlpool.hxx>
#include <stlsheet.hxx>
#include <patattr.hxx>
#include <attrib.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <xestring.hxx>
#include <xltools.hxx>
#include <conditio.hxx>
#include <dbdata.hxx>
#include <filterentries.hxx>
#include <export/ExportTools.hxx>
#include <dpobject.hxx>
#include <dpsave.hxx>
#include <pivot/PivotTableFormats.hxx>

#include <o3tl/safeint.hxx>
#include <oox/export/utils.hxx>
#include <oox/token/tokens.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/relationship.hxx>
#include <svl/numformat.hxx>

using namespace ::com::sun::star;
using namespace oox;

// PALETTE record - color information =========================================

namespace {

sal_uInt32 lclGetWeighting( XclExpColorType eType )
{
    switch( eType )
    {
        case EXC_COLOR_CHARTLINE:   return 1;
        case EXC_COLOR_CELLBORDER:
        case EXC_COLOR_CHARTAREA:   return 2;
        case EXC_COLOR_CELLTEXT:
        case EXC_COLOR_CHARTTEXT:
        case EXC_COLOR_CTRLTEXT:    return 10;
        case EXC_COLOR_TABBG:
        case EXC_COLOR_CELLAREA:    return 20;
        case EXC_COLOR_GRID:        return 50;
        default:    OSL_FAIL( "lclGetWeighting - unknown color type" );
    }
    return 1;
}

sal_Int32 lclGetColorDistance( const Color& rColor1, const Color& rColor2 )
{
    sal_Int32 nDist = rColor1.GetRed() - rColor2.GetRed();
    nDist *= nDist * 77;
    sal_Int32 nDummy = rColor1.GetGreen() - rColor2.GetGreen();
    nDist += nDummy * nDummy * 151;
    nDummy = rColor1.GetBlue() - rColor2.GetBlue();
    nDist += nDummy * nDummy * 28;
    return nDist;
}

sal_uInt8 lclGetMergedColorComp( sal_uInt8 nComp1, sal_uInt32 nWeight1, sal_uInt8 nComp2, sal_uInt32 nWeight2 )
{
    sal_uInt8 nComp1Dist = ::std::min< sal_uInt8 >( nComp1, 0xFF - nComp1 );
    sal_uInt8 nComp2Dist = ::std::min< sal_uInt8 >( nComp2, 0xFF - nComp2 );
    if( nComp1Dist != nComp2Dist )
    {
        /*  #i36945# One of the passed RGB components is nearer at the limits (0x00 or 0xFF).
            Increase its weighting to prevent fading of the colors during reduction. */
        const sal_uInt8& rnCompNearer = (nComp1Dist < nComp2Dist) ? nComp1 : nComp2;
        sal_uInt32& rnWeight = (nComp1Dist < nComp2Dist) ? nWeight1 : nWeight2;
        rnWeight *= ((rnCompNearer - 0x80L) * (rnCompNearer - 0x7FL) / 0x1000L + 1);
    }
    sal_uInt32 nWSum = nWeight1 + nWeight2;
    return static_cast< sal_uInt8 >( (nComp1 * nWeight1 + nComp2 * nWeight2 + nWSum / 2) / nWSum );
}

void lclSetMixedColor( Color& rDest, const Color& rSrc1, const Color& rSrc2 )
{
    rDest.SetRed( static_cast< sal_uInt8 >( (static_cast< sal_uInt16 >( rSrc1.GetRed() ) + rSrc2.GetRed()) / 2 ) );
    rDest.SetGreen( static_cast< sal_uInt8 >( (static_cast< sal_uInt16 >( rSrc1.GetGreen() ) + rSrc2.GetGreen()) / 2 ) );
    rDest.SetBlue( static_cast< sal_uInt8 >( (static_cast< sal_uInt16 >( rSrc1.GetBlue() ) + rSrc2.GetBlue()) / 2 ) );
}

} // namespace

// additional classes for color reduction -------------------------------------

namespace {

/** Represents an entry in a color list.

    The color stores a weighting value, which increases the more the color is
    used in the document. Heavy-weighted colors will change less than others on
    color reduction.
 */
class XclListColor
{
private:
    Color               maColor;        /// The color value of this palette entry.
    sal_uInt32          mnColorId;      /// Unique color ID for color reduction.
    sal_uInt32          mnWeight;       /// Weighting for color reduction.
    bool                mbBaseColor;    /// true = Handle as base color, (don't remove/merge).

public:
    explicit            XclListColor( const Color& rColor, sal_uInt32 nColorId );

    /** Returns the RGB color value of the color. */
    const Color& GetColor() const { return maColor; }
    /** Returns the unique ID of the color. */
    sal_uInt32   GetColorId() const { return mnColorId; }
    /** Returns the current weighting of the color. */
    sal_uInt32   GetWeighting() const { return mnWeight; }
    /** Returns true, if this color is a base color, i.e. it will not be removed or merged. */
    bool         IsBaseColor() const { return mbBaseColor; }

    /** Adds the passed weighting to this color. */
    void         AddWeighting( sal_uInt32 nWeight ) { mnWeight += nWeight; }
    /** Merges this color with rColor, regarding weighting settings. */
    void                Merge( const XclListColor& rColor );
};

XclListColor::XclListColor( const Color& rColor, sal_uInt32 nColorId ) :
    maColor( rColor ),
    mnColorId( nColorId ),
    mnWeight( 0 )
{
    mbBaseColor =
        ((rColor.GetRed()   == 0x00) || (rColor.GetRed()   == 0xFF)) &&
        ((rColor.GetGreen() == 0x00) || (rColor.GetGreen() == 0xFF)) &&
        ((rColor.GetBlue()  == 0x00) || (rColor.GetBlue()  == 0xFF));
}

void XclListColor::Merge( const XclListColor& rColor )
{
    sal_uInt32 nWeight2 = rColor.GetWeighting();
    // do not change RGB value of base colors
    if( !mbBaseColor )
    {
        maColor.SetRed(   lclGetMergedColorComp( maColor.GetRed(),   mnWeight, rColor.maColor.GetRed(),   nWeight2 ) );
        maColor.SetGreen( lclGetMergedColorComp( maColor.GetGreen(), mnWeight, rColor.maColor.GetGreen(), nWeight2 ) );
        maColor.SetBlue(  lclGetMergedColorComp( maColor.GetBlue(),  mnWeight, rColor.maColor.GetBlue(),  nWeight2 ) );
    }
    AddWeighting( nWeight2 );
}

/** Data for each inserted original color, represented by a color ID. */
struct XclColorIdData
{
    Color               maColor;        /// The original inserted color.
    sal_uInt32          mnIndex;        /// Maps current color ID to color list or export color vector.
    /** Sets the contents of this struct. */
    void         Set( const Color& rColor, sal_uInt32 nIndex ) { maColor = rColor; mnIndex = nIndex; }
};

/** A color that will be written to the Excel file. */
struct XclPaletteColor
{
    Color               maColor;        /// Resulting color to export.
    bool                mbUsed;         /// true = Entry is used in the document.

    explicit     XclPaletteColor( const Color& rColor ) : maColor( rColor ), mbUsed( false ) {}
    void         SetColor( const Color& rColor ) { maColor = rColor; mbUsed = true; }
};

/** Maps a color list index to a palette index.
    @descr  Used to remap the color ID data vector from list indexes to palette indexes. */
struct XclRemap
{
    sal_uInt32          mnPalIndex;     /// Index to palette.
    bool                mbProcessed;    /// true = List color already processed.

    explicit     XclRemap() : mnPalIndex( 0 ), mbProcessed( false ) {}
    void         SetIndex( sal_uInt32 nPalIndex )
                            { mnPalIndex = nPalIndex; mbProcessed = true; }
};

/** Stores the nearest palette color index of a list color. */
struct XclNearest
{
    sal_uInt32          mnPalIndex;     /// Index to nearest palette color.
    sal_Int32           mnDist;         /// Distance to palette color.

    explicit     XclNearest() : mnPalIndex( 0 ), mnDist( 0 ) {}
};

} // namespace

class XclExpPaletteImpl
{
public:
    explicit            XclExpPaletteImpl( const XclDefaultPalette& rDefPal );

    /** Inserts the color into the list and updates weighting.
        @param nAutoDefault  The Excel palette index for automatic color.
        @return  A unique ID for this color. */
    sal_uInt32          InsertColor( const Color& rColor, XclExpColorType eType, sal_uInt16 nAutoDefault = 0 );
    /** Returns the color ID representing a fixed Excel palette index (i.e. for auto colors). */
    static sal_uInt32   GetColorIdFromIndex( sal_uInt16 nIndex );

    /** Reduces the color list to the maximum count of the current BIFF version. */
    void                Finalize();

    /** Returns the Excel palette index of the color with passed color ID. */
    sal_uInt16          GetColorIndex( sal_uInt32 nColorId ) const;

    /** Returns a foreground and background color for the two passed color IDs.
        @descr  If rnXclPattern contains a solid pattern, this function tries to find
        the two best fitting colors and a mix pattern (25%, 50% or 75%) for nForeColorId.
        This will result in a better approximation to the passed foreground color. */
    void                GetMixedColors(
                            sal_uInt16& rnXclForeIx, sal_uInt16& rnXclBackIx, sal_uInt8& rnXclPattern,
                            sal_uInt32 nForeColorId, sal_uInt32 nBackColorId ) const;

    /** Returns the RGB color for a (non-zero-based) Excel palette entry.
        @return  The color from current or default palette or COL_AUTO, if nothing else found. */
    Color               GetColor( sal_uInt16 nXclIndex ) const;

    /** Returns true, if all colors of the palette are equal to default palette colors. */
    bool                IsDefaultPalette() const;
    /** Writes the color list (contents of the palette record) to the passed stream. */
    void                WriteBody( XclExpStream& rStrm );
    void                SaveXml( XclExpXmlStream& rStrm );

private:
    /** Returns the Excel index of a 0-based color index. */
    static sal_uInt16 GetXclIndex( sal_uInt32 nIndex )
                            { return static_cast< sal_uInt16 >( nIndex + EXC_COLOR_USEROFFSET ); }

    /** Returns the original inserted color represented by the color ID nColorId. */
    const Color&        GetOriginalColor( sal_uInt32 nColorId ) const;

    /** Searches for rColor, returns the ordered insertion index for rColor in rnIndex. */
    XclListColor*       SearchListEntry( const Color& rColor, sal_uInt32& rnIndex );
    /** Creates and inserts a new color list entry at the specified list position. */
    XclListColor*       CreateListEntry( const Color& rColor, sal_uInt32 nIndex );

    /** Raw and fast reduction of the palette. */
    void                RawReducePalette( sal_uInt32 nPass );
    /** Reduction of one color using advanced color merging based on color weighting. */
    void                ReduceLeastUsedColor();

    /** Finds the least used color and returns its current list index. */
    sal_uInt32          GetLeastUsedListColor() const;
    /** Returns the list index of the color nearest to rColor.
        @param nIgnore  List index of a color which will be ignored.
        @return  The list index of the found color. */
    sal_uInt32          GetNearestListColor( const Color& rColor, sal_uInt32 nIgnore ) const;
    /** Returns the list index of the color nearest to the color with list index nIndex. */
    sal_uInt32          GetNearestListColor( sal_uInt32 nIndex ) const;

    /** Returns in rnIndex the palette index of the color nearest to rColor.
        Searches for default colors only (colors never replaced).
        @return  The distance from passed color to found color. */
    sal_Int32           GetNearestPaletteColor(
                            sal_uInt32& rnIndex,
                            const Color& rColor ) const;
    /** Returns in rnFirst and rnSecond the palette indexes of the two colors nearest to rColor.
        @return  The minimum distance from passed color to found colors. */
    sal_Int32           GetNearPaletteColors(
                            sal_uInt32& rnFirst, sal_uInt32& rnSecond,
                            const Color& rColor ) const;

private:
    typedef std::vector< std::unique_ptr<XclListColor> >     XclListColorList;
    typedef std::shared_ptr< XclListColorList > XclListColorListRef;

    const XclDefaultPalette& mrDefPal;      /// The default palette for the current BIFF version.
    XclListColorListRef mxColorList;        /// Working color list.
    std::vector< XclColorIdData >
                        maColorIdDataVec;   /// Data of all CIDs.
    std::vector< XclPaletteColor >
                        maPalette;          /// Contains resulting colors to export.
    sal_uInt32          mnLastIdx;          /// Last insertion index for search opt.
};

const sal_uInt32 EXC_PAL_INDEXBASE          = 0xFFFF0000;
const sal_uInt32 EXC_PAL_MAXRAWSIZE         = 1024;

XclExpPaletteImpl::XclExpPaletteImpl( const XclDefaultPalette& rDefPal ) :
    mrDefPal( rDefPal ),
    mxColorList( std::make_shared<XclListColorList>() ),
    mnLastIdx( 0 )
{
    // initialize maPalette with default colors
    sal_uInt16 nCount = static_cast< sal_uInt16 >( mrDefPal.GetColorCount() );
    maPalette.reserve( nCount );
    for( sal_uInt16 nIdx = 0; nIdx < nCount; ++nIdx )
        maPalette.emplace_back( mrDefPal.GetDefColor( GetXclIndex( nIdx ) ) );

    InsertColor( COL_BLACK, EXC_COLOR_CELLTEXT );
}

sal_uInt32 XclExpPaletteImpl::InsertColor( const Color& rColor, XclExpColorType eType, sal_uInt16 nAutoDefault )
{
    if( rColor == COL_AUTO )
        return GetColorIdFromIndex( nAutoDefault );

    sal_uInt32 nFoundIdx = 0;
    XclListColor* pEntry = SearchListEntry( rColor, nFoundIdx );
    if( !pEntry || (pEntry->GetColor() != rColor) )
        pEntry = CreateListEntry( rColor, nFoundIdx );
    pEntry->AddWeighting( lclGetWeighting( eType ) );

    return pEntry->GetColorId();
}

sal_uInt32 XclExpPaletteImpl::GetColorIdFromIndex( sal_uInt16 nIndex )
{
    return EXC_PAL_INDEXBASE | nIndex;
}

void XclExpPaletteImpl::Finalize()
{
// --- build initial color ID data vector (maColorIdDataVec) ---

    sal_uInt32 nCount = mxColorList->size();
    maColorIdDataVec.resize( nCount );
    for( sal_uInt32 nIdx = 0; nIdx < nCount; ++nIdx )
    {
        const XclListColor& listColor = *mxColorList->at( nIdx );
        maColorIdDataVec[ listColor.GetColorId() ].Set( listColor.GetColor(), nIdx );
    }

// --- loop as long as current color count does not fit into palette of current BIFF ---

    // phase 1: raw reduction (performance reasons, #i36945#)
    sal_uInt32 nPass = 0;
    while( mxColorList->size() > EXC_PAL_MAXRAWSIZE )
        RawReducePalette( nPass++ );

    // phase 2: precise reduction using advanced color merging based on color weighting
    while( mxColorList->size() > mrDefPal.GetColorCount() )
        ReduceLeastUsedColor();

// --- use default palette and replace colors with nearest used colors ---

    nCount = mxColorList->size();
    std::vector< XclRemap > aRemapVec( nCount );
    std::vector< XclNearest > aNearestVec( nCount );

    // in each run: search the best fitting color and replace a default color with it
    for( sal_uInt32 nRun = 0; nRun < nCount; ++nRun )
    {
        sal_uInt32 nIndex;
        // find nearest unused default color for each unprocessed list color
        for( nIndex = 0; nIndex < nCount; ++nIndex )
            aNearestVec[ nIndex ].mnDist = aRemapVec[ nIndex ].mbProcessed ? SAL_MAX_INT32 :
                GetNearestPaletteColor( aNearestVec[ nIndex ].mnPalIndex, mxColorList->at( nIndex )->GetColor() );
        // find the list color which is nearest to a default color
        sal_uInt32 nFound = 0;
        for( nIndex = 1; nIndex < nCount; ++nIndex )
            if( aNearestVec[ nIndex ].mnDist < aNearestVec[ nFound ].mnDist )
                nFound = nIndex;
        // replace default color with list color
        sal_uInt32 nNearest = aNearestVec[ nFound ].mnPalIndex;
        OSL_ENSURE( nNearest < maPalette.size(), "XclExpPaletteImpl::Finalize - algorithm error" );
        maPalette[ nNearest ].SetColor( mxColorList->at( nFound )->GetColor() );
        aRemapVec[ nFound ].SetIndex( nNearest );
    }

    // remap color ID data map (maColorIdDataVec) from list indexes to palette indexes
    for( auto& rColorIdData : maColorIdDataVec )
        rColorIdData.mnIndex = aRemapVec[ rColorIdData.mnIndex ].mnPalIndex;
}

sal_uInt16 XclExpPaletteImpl::GetColorIndex( sal_uInt32 nColorId ) const
{
    sal_uInt16 nRet = 0;
    if( nColorId >= EXC_PAL_INDEXBASE )
        nRet = static_cast< sal_uInt16 >( nColorId & ~EXC_PAL_INDEXBASE );
    else if( nColorId < maColorIdDataVec.size() )
        nRet = GetXclIndex( maColorIdDataVec[ nColorId ].mnIndex );
    return nRet;
}

void XclExpPaletteImpl::GetMixedColors(
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
    lclSetMixedColor( aColorArr[ 2 ], aColorArr[ 0 ], aColorArr[ 4 ] );
    lclSetMixedColor( aColorArr[ 1 ], aColorArr[ 0 ], aColorArr[ 2 ] );
    lclSetMixedColor( aColorArr[ 3 ], aColorArr[ 2 ], aColorArr[ 4 ] );

    sal_Int32 nMinDist = nFirstDist;
    sal_uInt32 nMinIndex = 0;
    for( sal_uInt32 nCnt = 1; nCnt < 4; ++nCnt )
    {
        sal_Int32 nDist = lclGetColorDistance( aForeColor, aColorArr[ nCnt ] );
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

Color XclExpPaletteImpl::GetColor( sal_uInt16 nXclIndex ) const
{
    if( nXclIndex >= EXC_COLOR_USEROFFSET )
    {
        sal_uInt32 nIdx = nXclIndex - EXC_COLOR_USEROFFSET;
        if( nIdx < maPalette.size() )
            return maPalette[ nIdx ].maColor;
    }
    return mrDefPal.GetDefColor( nXclIndex );
}

bool XclExpPaletteImpl::IsDefaultPalette() const
{
    bool bDefault = true;
    for( sal_uInt32 nIdx = 0, nSize = static_cast< sal_uInt32 >( maPalette.size() ); bDefault && (nIdx < nSize); ++nIdx )
        bDefault = maPalette[ nIdx ].maColor == mrDefPal.GetDefColor( GetXclIndex( nIdx ) );
    return bDefault;
}

void XclExpPaletteImpl::WriteBody( XclExpStream& rStrm )
{
    rStrm << static_cast< sal_uInt16 >( maPalette.size() );
    for( const auto& rColor : maPalette )
        rStrm << rColor.maColor;
}

void XclExpPaletteImpl::SaveXml( XclExpXmlStream& rStrm )
{
    if( maPalette.empty() )
        return;

    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->startElement(XML_colors);
    rStyleSheet->startElement(XML_indexedColors);
    for( const auto& rColor : maPalette )
        rStyleSheet->singleElement(XML_rgbColor, XML_rgb, XclXmlUtils::ToOString(rColor.maColor));
    rStyleSheet->endElement( XML_indexedColors );
    rStyleSheet->endElement( XML_colors );
}

const Color& XclExpPaletteImpl::GetOriginalColor( sal_uInt32 nColorId ) const
{
    if( nColorId < maColorIdDataVec.size() )
        return maColorIdDataVec[ nColorId ].maColor;
    return maPalette[ 0 ].maColor;
}

XclListColor* XclExpPaletteImpl::SearchListEntry( const Color& rColor, sal_uInt32& rnIndex )
{
    rnIndex = 0;

    if (mxColorList->empty())
        return nullptr;

    XclListColor* pEntry = nullptr;

    // search optimization for equal-colored objects occurring repeatedly
    if (mnLastIdx < mxColorList->size())
    {
        pEntry = (*mxColorList)[mnLastIdx].get();
        if( pEntry->GetColor() == rColor )
        {
            rnIndex = mnLastIdx;
            return pEntry;
        }
    }

    // binary search for color
    sal_uInt32 nBegIdx = 0;
    sal_uInt32 nEndIdx = mxColorList->size();
    bool bFound = false;
    while( !bFound && (nBegIdx < nEndIdx) )
    {
        rnIndex = (nBegIdx + nEndIdx) / 2;
        pEntry = (*mxColorList)[rnIndex].get();
        bFound = pEntry->GetColor() == rColor;
        if( !bFound )
        {
            if( pEntry->GetColor() < rColor )
                nBegIdx = rnIndex + 1;
            else
                nEndIdx = rnIndex;
        }
    }

    // not found - use end of range as new insertion position
    if( !bFound )
        rnIndex = nEndIdx;

    mnLastIdx = rnIndex;
    return pEntry;
}

XclListColor* XclExpPaletteImpl::CreateListEntry( const Color& rColor, sal_uInt32 nIndex )
{
    XclListColor* pEntry = new XclListColor( rColor, mxColorList->size() );
    mxColorList->insert(mxColorList->begin() + nIndex, std::unique_ptr<XclListColor>(pEntry));
    return pEntry;
}

void XclExpPaletteImpl::RawReducePalette( sal_uInt32 nPass )
{
    /*  Fast palette reduction - in each call of this function one RGB component
        of each color is reduced to a lower number of distinct values.
        Pass 0: Blue is reduced to 128 distinct values.
        Pass 1: Red is reduced to 128 distinct values.
        Pass 2: Green is reduced to 128 distinct values.
        Pass 3: Blue is reduced to 64 distinct values.
        Pass 4: Red is reduced to 64 distinct values.
        Pass 5: Green is reduced to 64 distinct values.
        And so on...
     */

    XclListColorListRef xOldList = mxColorList;
    mxColorList = std::make_shared<XclListColorList>();

    // maps old list indexes to new list indexes, used to update maColorIdDataVec
    ScfUInt32Vec aListIndexMap;
    aListIndexMap.reserve( xOldList->size() );

    // preparations
    sal_uInt8 nR, nG, nB;
    sal_uInt8& rnComp = ((nPass % 3 == 0) ? nB : ((nPass % 3 == 1) ? nR : nG));
    nPass /= 3;
    OSL_ENSURE( nPass < 7, "XclExpPaletteImpl::RawReducePalette - reduction not terminated" );

    static const sal_uInt8 spnFactor2[] = { 0x81, 0x82, 0x84, 0x88, 0x92, 0xAA, 0xFF };
    sal_uInt8 nFactor1 = static_cast< sal_uInt8 >( 0x02 << nPass );
    sal_uInt8 nFactor2 = spnFactor2[ nPass ];
    sal_uInt8 nFactor3 = static_cast< sal_uInt8 >( 0x40 >> nPass );

    // process each color in the old color list
    for(const std::unique_ptr<XclListColor> & pOldColor : *xOldList)
    {
        // get the old list entry
        const XclListColor* pOldEntry = pOldColor.get();
        nR = pOldEntry->GetColor().GetRed();
        nG = pOldEntry->GetColor().GetGreen();
        nB = pOldEntry->GetColor().GetBlue();

        /*  Calculate the new RGB component (rnComp points to one of nR, nG, nB).
            Using integer arithmetic with its rounding errors, the results of
            this calculation are always exactly in the range 0x00 to 0xFF
            (simply cutting the lower bits would darken the colors slightly). */
        sal_uInt32 nNewComp = rnComp;
        nNewComp /= nFactor1;
        nNewComp *= nFactor2;
        nNewComp /= nFactor3;
        rnComp = static_cast< sal_uInt8 >( nNewComp );
        Color aNewColor( nR, nG, nB );

        // find or insert the new color
        sal_uInt32 nFoundIdx = 0;
        XclListColor* pNewEntry = SearchListEntry( aNewColor, nFoundIdx );
        if( !pNewEntry || (pNewEntry->GetColor() != aNewColor) )
            pNewEntry = CreateListEntry( aNewColor, nFoundIdx );
        pNewEntry->AddWeighting( pOldEntry->GetWeighting() );
        aListIndexMap.push_back( nFoundIdx );
    }

    // update color ID data map (maps color IDs to color list indexes), replace old by new list indexes
    for( auto& rColorIdData : maColorIdDataVec )
        rColorIdData.mnIndex = aListIndexMap[ rColorIdData.mnIndex ];
}

void XclExpPaletteImpl::ReduceLeastUsedColor()
{
    // find a list color to remove
    sal_uInt32 nRemove = GetLeastUsedListColor();
    // find its nearest neighbor
    sal_uInt32 nKeep = GetNearestListColor( nRemove );

    // merge both colors to one color, remove one color from list
    XclListColor* pKeepEntry = mxColorList->at(nKeep).get();
    XclListColor* pRemoveEntry = mxColorList->at(nRemove).get();
    if( !(pKeepEntry && pRemoveEntry) )
        return;

    // merge both colors (if pKeepEntry is a base color, it will not change)
    pKeepEntry->Merge( *pRemoveEntry );
    // remove the less used color, adjust nKeep index if kept color follows removed color
    XclListColorList::iterator itr = mxColorList->begin();
    ::std::advance(itr, nRemove);
    mxColorList->erase(itr);
    if( nKeep > nRemove ) --nKeep;

    // recalculate color ID data map (maps color IDs to color list indexes)
    for( auto& rColorIdData : maColorIdDataVec )
    {
        if( rColorIdData.mnIndex > nRemove )
            --rColorIdData.mnIndex;
        else if( rColorIdData.mnIndex == nRemove )
            rColorIdData.mnIndex = nKeep;
    }
}

sal_uInt32 XclExpPaletteImpl::GetLeastUsedListColor() const
{
    sal_uInt32 nFound = 0;
    sal_uInt32 nMinW = SAL_MAX_UINT32;

    for( sal_uInt32 nIdx = 0, nCount = mxColorList->size(); nIdx < nCount; ++nIdx )
    {
        XclListColor& rEntry = *mxColorList->at( nIdx );
        // ignore the base colors
        if( !rEntry.IsBaseColor() && (rEntry.GetWeighting() < nMinW) )
        {
            nFound = nIdx;
            nMinW = rEntry.GetWeighting();
        }
    }
    return nFound;
}

sal_uInt32 XclExpPaletteImpl::GetNearestListColor( const Color& rColor, sal_uInt32 nIgnore ) const
{
    sal_uInt32 nFound = 0;
    sal_Int32 nMinD = SAL_MAX_INT32;

    for( sal_uInt32 nIdx = 0, nCount = mxColorList->size(); nIdx < nCount; ++nIdx )
    {
        if( nIdx != nIgnore )
        {
            if( XclListColor* pEntry = mxColorList->at(nIdx).get() )
            {
                sal_Int32 nDist = lclGetColorDistance( rColor, pEntry->GetColor() );
                if( nDist < nMinD )
                {
                    nFound = nIdx;
                    nMinD = nDist;
                }
            }
        }
    }
    return nFound;
}

sal_uInt32 XclExpPaletteImpl::GetNearestListColor( sal_uInt32 nIndex ) const
{
    if (nIndex >= mxColorList->size())
        return 0;
    XclListColor* pEntry = mxColorList->at(nIndex).get();
    return GetNearestListColor( pEntry->GetColor(), nIndex );
}

sal_Int32 XclExpPaletteImpl::GetNearestPaletteColor(
        sal_uInt32& rnIndex, const Color& rColor ) const
{
    rnIndex = 0;
    sal_Int32 nDist = SAL_MAX_INT32;

    sal_uInt32 nPaletteIndex = 0;
    for( const auto& rPaletteColor : maPalette )
    {
        if( !rPaletteColor.mbUsed )
        {
            sal_Int32 nCurrDist = lclGetColorDistance( rColor, rPaletteColor.maColor );
            if( nCurrDist < nDist )
            {
                rnIndex = nPaletteIndex;
                nDist = nCurrDist;
            }
        }
        ++nPaletteIndex;
    }
    return nDist;
}

sal_Int32 XclExpPaletteImpl::GetNearPaletteColors(
        sal_uInt32& rnFirst, sal_uInt32& rnSecond, const Color& rColor ) const
{
    rnFirst = rnSecond = 0;
    sal_Int32 nDist1 = SAL_MAX_INT32;
    sal_Int32 nDist2 = SAL_MAX_INT32;

    sal_uInt32 nPaletteIndex = 0;
    for( const auto& rPaletteColor : maPalette )
    {
        sal_Int32 nCurrDist = lclGetColorDistance( rColor, rPaletteColor.maColor );
        if( nCurrDist < nDist1 )
        {
            rnSecond = rnFirst;
            nDist2 = nDist1;
            rnFirst = nPaletteIndex;
            nDist1 = nCurrDist;
        }
        else if( nCurrDist < nDist2 )
        {
            rnSecond = nPaletteIndex;
            nDist2 = nCurrDist;
        }
        ++nPaletteIndex;
    }
    return nDist1;
}

XclExpPalette::XclExpPalette( const XclExpRoot& rRoot ) :
    XclDefaultPalette( rRoot ),
    XclExpRecord( EXC_ID_PALETTE )
{
    mxImpl = std::make_shared<XclExpPaletteImpl>( *this );
    SetRecSize( GetColorCount() * 4 + 2 );
}

XclExpPalette::~XclExpPalette()
{
}

sal_uInt32 XclExpPalette::InsertColor( const Color& rColor, XclExpColorType eType, sal_uInt16 nAutoDefault )
{
    return mxImpl->InsertColor( rColor, eType, nAutoDefault );
}

sal_uInt32 XclExpPalette::GetColorIdFromIndex( sal_uInt16 nIndex )
{
    return XclExpPaletteImpl::GetColorIdFromIndex( nIndex );
}

void XclExpPalette::Finalize()
{
    mxImpl->Finalize();
}

sal_uInt16 XclExpPalette::GetColorIndex( sal_uInt32 nColorId ) const
{
    return mxImpl->GetColorIndex( nColorId );
}

void XclExpPalette::GetMixedColors(
        sal_uInt16& rnXclForeIx, sal_uInt16& rnXclBackIx, sal_uInt8& rnXclPattern,
        sal_uInt32 nForeColorId, sal_uInt32 nBackColorId ) const
{
    return mxImpl->GetMixedColors( rnXclForeIx, rnXclBackIx, rnXclPattern, nForeColorId, nBackColorId );
}

Color XclExpPalette::GetColor( sal_uInt16 nXclIndex ) const
{
    return mxImpl->GetColor( nXclIndex );
}

void XclExpPalette::Save( XclExpStream& rStrm )
{
    if( !mxImpl->IsDefaultPalette() )
        XclExpRecord::Save( rStrm );
}

void XclExpPalette::SaveXml( XclExpXmlStream& rStrm )
{
    if( !mxImpl->IsDefaultPalette() )
        mxImpl->SaveXml( rStrm );
}

void XclExpPalette::WriteBody( XclExpStream& rStrm )
{
    mxImpl->WriteBody( rStrm );
}

// FONT record - font information =============================================

namespace {

typedef ::std::pair< sal_uInt16, sal_Int16 > WhichAndScript;

sal_Int16 lclCheckFontItems( const SfxItemSet& rItemSet,
        const WhichAndScript& rWAS1, const WhichAndScript& rWAS2, const WhichAndScript& rWAS3 )
{
    if( ScfTools::CheckItem( rItemSet, rWAS1.first, false ) ) return rWAS1.second;
    if( ScfTools::CheckItem( rItemSet, rWAS2.first, false ) ) return rWAS2.second;
    if( ScfTools::CheckItem( rItemSet, rWAS3.first, false ) ) return rWAS3.second;
    return 0;
};

} // namespace

sal_Int16 XclExpFontHelper::GetFirstUsedScript( const XclExpRoot& rRoot, const SfxItemSet& rItemSet )
{
    namespace ApiScriptType = css::i18n::ScriptType;

    /*  #i17050# #i107170# We need to determine which font items are set in the
        item set, and which script type we should prefer according to the
        current languages and locales. */

    static const WhichAndScript WAS_LATIN( ATTR_FONT, css::i18n::ScriptType::LATIN );
    static const WhichAndScript WAS_ASIAN( ATTR_CJK_FONT, css::i18n::ScriptType::ASIAN );
    static const WhichAndScript WAS_CMPLX( ATTR_CTL_FONT, css::i18n::ScriptType::COMPLEX );

    /*  do not let a font from a parent style override an explicit
        cell font. */

    sal_Int16 nDefScript = rRoot.GetDefApiScript();
    sal_Int16 nScript = 0;
    const SfxItemSet* pCurrSet = &rItemSet;

    while( (nScript == 0) && pCurrSet )
    {
        switch( nDefScript )
        {
            case ApiScriptType::LATIN:
                nScript = lclCheckFontItems( *pCurrSet, WAS_LATIN, WAS_CMPLX, WAS_ASIAN );
            break;
            case ApiScriptType::ASIAN:
                nScript = lclCheckFontItems( *pCurrSet, WAS_ASIAN, WAS_CMPLX, WAS_LATIN );
            break;
            case ApiScriptType::COMPLEX:
                nScript = lclCheckFontItems( *pCurrSet, WAS_CMPLX, WAS_ASIAN, WAS_LATIN );
            break;
            default:
                OSL_FAIL( "XclExpFontHelper::GetFirstUsedScript - unknown script type" );
                nScript = ApiScriptType::LATIN;
        };
        pCurrSet = pCurrSet->GetParent();
    }

    if (nScript == 0)
        nScript = nDefScript;

    if (nScript == 0)
    {
        OSL_FAIL( "XclExpFontHelper::GetFirstUsedScript - unknown script type" );
        nScript = ApiScriptType::LATIN;
    }

    return nScript;
}

vcl::Font XclExpFontHelper::GetFontFromItemSet( const XclExpRoot& rRoot, const SfxItemSet& rItemSet, sal_Int16 nScript )
{
    // if WEAK is passed, guess script type from existing items in the item set
    if( nScript == css::i18n::ScriptType::WEAK )
        nScript = GetFirstUsedScript( rRoot, rItemSet );

    // convert to core script type constants
    SvtScriptType nScScript = SvtLanguageOptions::FromI18NToSvtScriptType(nScript);

    // fill the font object
    vcl::Font aFont;
    ScPatternAttr::fillFontOnly(aFont, rItemSet, nullptr, nullptr, nullptr, nScScript);
    return aFont;
}

ScDxfFont XclExpFontHelper::GetDxfFontFromItemSet(const XclExpRoot& rRoot, const SfxItemSet& rItemSet)
{
    sal_Int16 nScript = GetFirstUsedScript(rRoot, rItemSet);

    // convert to core script type constants
    SvtScriptType nScScript = SvtLanguageOptions::FromI18NToSvtScriptType(nScript);
    return ScPatternAttr::GetDxfFont(rItemSet, nScScript);
}

bool XclExpFontHelper::CheckItems( const XclExpRoot& rRoot, const SfxItemSet& rItemSet, sal_Int16 nScript, bool bDeep )
{
    static const sal_uInt16 pnCommonIds[] = {
        ATTR_FONT_UNDERLINE, ATTR_FONT_CROSSEDOUT, ATTR_FONT_CONTOUR,
        ATTR_FONT_SHADOWED, ATTR_FONT_COLOR, ATTR_FONT_LANGUAGE, 0 };
    static const sal_uInt16 pnLatinIds[] = {
        ATTR_FONT, ATTR_FONT_HEIGHT, ATTR_FONT_WEIGHT, ATTR_FONT_POSTURE, 0 };
    static const sal_uInt16 pnAsianIds[] = {
        ATTR_CJK_FONT, ATTR_CJK_FONT_HEIGHT, ATTR_CJK_FONT_WEIGHT, ATTR_CJK_FONT_POSTURE, 0 };
    static const sal_uInt16 pnComplexIds[] = {
        ATTR_CTL_FONT, ATTR_CTL_FONT_HEIGHT, ATTR_CTL_FONT_WEIGHT, ATTR_CTL_FONT_POSTURE, 0 };

    bool bUsed = ScfTools::CheckItems( rItemSet, pnCommonIds, bDeep );
    if( !bUsed )
    {
        namespace ApiScriptType = css::i18n::ScriptType;
        // if WEAK is passed, guess script type from existing items in the item set
        if( nScript == ApiScriptType::WEAK )
            nScript = GetFirstUsedScript( rRoot, rItemSet );
        // check the correct items
        switch( nScript )
        {
            case ApiScriptType::LATIN:      bUsed = ScfTools::CheckItems( rItemSet, pnLatinIds, bDeep );    break;
            case ApiScriptType::ASIAN:      bUsed = ScfTools::CheckItems( rItemSet, pnAsianIds, bDeep );    break;
            case ApiScriptType::COMPLEX:    bUsed = ScfTools::CheckItems( rItemSet, pnComplexIds, bDeep );  break;
            default:    OSL_FAIL( "XclExpFontHelper::CheckItems - unknown script type" );
        }
    }
    return bUsed;
}

namespace {

std::size_t lclCalcHash( const XclFontData& rFontData )
{
    std::size_t seed = 0;
    o3tl::hash_combine(seed, rFontData.maName);
    o3tl::hash_combine(seed, rFontData.maComplexColor);
    o3tl::hash_combine(seed, rFontData.mnWeight);
    o3tl::hash_combine(seed, rFontData.mnCharSet);
    o3tl::hash_combine(seed, rFontData.mnFamily);
    o3tl::hash_combine(seed, rFontData.mnHeight);
    o3tl::hash_combine(seed, rFontData.mnUnderline);
    o3tl::hash_combine(seed, rFontData.mnEscapem);
    o3tl::hash_combine(seed, rFontData.mbItalic);
    o3tl::hash_combine(seed, rFontData.mbStrikeout);
    o3tl::hash_combine(seed, rFontData.mbOutline);
    o3tl::hash_combine(seed, rFontData.mbShadow);
    return seed;
}

} // namespace

XclExpFont::XclExpFont( const XclExpRoot& rRoot,
        const XclFontData& rFontData, XclExpColorType eColorType ) :
    XclExpRecord( EXC_ID2_FONT, 14 ),
    XclExpRoot( rRoot ),
    maData( rFontData )
{
    // insert font color into palette
    mnColorId = rRoot.GetPalette().InsertColor(rFontData.maComplexColor.getFinalColor(), eColorType, EXC_COLOR_FONTAUTO);
    // hash value for faster comparison
    mnHash = lclCalcHash( maData );
    // record size
    sal_Int32 nStrLen = maData.maName.getLength();
    SetRecSize( ((GetBiff() == EXC_BIFF8) ? (nStrLen * 2 + 1) : nStrLen) + 15 );
}

bool XclExpFont::Equals( const XclFontData& rFontData, sal_uInt32 nHash ) const
{
    return (mnHash == nHash) && (maData == rFontData);
}

void XclExpFont::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->startElement(XML_font);
    XclXmlUtils::WriteFontData( rStyleSheet, maData, XML_name );
    // OOXTODO: XML_scheme; //scheme/@val values: "major", "minor", "none"
    rStyleSheet->endElement( XML_font );
}

// private --------------------------------------------------------------------

void XclExpFont::WriteBody( XclExpStream& rStrm )
{
    sal_uInt16 nAttr = EXC_FONTATTR_NONE;
    ::set_flag( nAttr, EXC_FONTATTR_ITALIC, maData.mbItalic );
    if( maData.mnUnderline > 0 )
        ::set_flag( nAttr, EXC_FONTATTR_UNDERLINE, true );
    ::set_flag( nAttr, EXC_FONTATTR_STRIKEOUT, maData.mbStrikeout );
    ::set_flag( nAttr, EXC_FONTATTR_OUTLINE, maData.mbOutline );
    ::set_flag( nAttr, EXC_FONTATTR_SHADOW, maData.mbShadow );

    OSL_ENSURE( maData.maName.getLength() < 256, "XclExpFont::WriteBody - font name too long" );
    XclExpString aFontName;
    if( GetBiff() <= EXC_BIFF5 )
        aFontName.AssignByte( maData.maName, GetTextEncoding(), XclStrFlags::EightBitLength );
    else
        aFontName.Assign( maData.maName, XclStrFlags::ForceUnicode | XclStrFlags::EightBitLength );

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

XclExpDxfFont::XclExpDxfFont(const XclExpRoot& rRoot,
        const SfxItemSet& rItemSet):
    XclExpRoot(rRoot)
{
    maDxfData = XclExpFontHelper::GetDxfFontFromItemSet(rRoot, rItemSet);
}

namespace {

const char* getUnderlineOOXValue(FontLineStyle eUnderline)
{
    switch (eUnderline)
    {
        case LINESTYLE_NONE:
        case LINESTYLE_DONTKNOW:
            return "none";
        case LINESTYLE_DOUBLE:
        case LINESTYLE_DOUBLEWAVE:
            return "double";
        default:
            return "single";
    }
}

const char* getFontFamilyOOXValue(FontFamily eValue)
{
    switch (eValue)
    {
        case FAMILY_DONTKNOW:
            return "0";
        case FAMILY_SWISS:
        case FAMILY_SYSTEM:
            return "2";
        case FAMILY_ROMAN:
            return "1";
        case FAMILY_SCRIPT:
            return "4";
        case FAMILY_MODERN:
            return "3";
        case FAMILY_DECORATIVE:
            return "5";
        default:
            return "0";
    }
}

}

void XclExpDxfFont::SaveXml(XclExpXmlStream& rStrm)
{
    if (maDxfData.isEmpty())
        return;

    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->startElement(XML_font);

    if (maDxfData.pFontAttr)
    {
        OUString aFontName = (*maDxfData.pFontAttr)->GetFamilyName();

        aFontName = XclTools::GetXclFontName(aFontName);
        if (!aFontName.isEmpty())
        {
            rStyleSheet->singleElement(XML_name, XML_val, aFontName);
        }

        rtl_TextEncoding eTextEnc = (*maDxfData.pFontAttr)->GetCharSet();
        sal_uInt8 nExcelCharSet = rtl_getBestWindowsCharsetFromTextEncoding(eTextEnc);
        if (nExcelCharSet)
        {
            rStyleSheet->singleElement(XML_charset, XML_val, OString::number(nExcelCharSet));
        }

        FontFamily eFamily = (*maDxfData.pFontAttr)->GetFamily();
        const char* pVal = getFontFamilyOOXValue(eFamily);
        if (pVal)
        {
            rStyleSheet->singleElement(XML_family, XML_val, pVal);
        }
    }

    if (maDxfData.eWeight)
    {
        rStyleSheet->singleElement(XML_b,
                XML_val, ToPsz10(*maDxfData.eWeight != WEIGHT_NORMAL));
    }

    if (maDxfData.eItalic)
    {
        bool bItalic = (*maDxfData.eItalic == ITALIC_OBLIQUE) || (*maDxfData.eItalic == ITALIC_NORMAL);
        rStyleSheet->singleElement(XML_i, XML_val, ToPsz10(bItalic));
    }

    if (maDxfData.eStrike)
    {
        bool bStrikeout =
            (*maDxfData.eStrike == STRIKEOUT_SINGLE) || (*maDxfData.eStrike == STRIKEOUT_DOUBLE) ||
            (*maDxfData.eStrike == STRIKEOUT_BOLD)   || (*maDxfData.eStrike == STRIKEOUT_SLASH)  ||
            (*maDxfData.eStrike == STRIKEOUT_X);

        rStyleSheet->singleElement(XML_strike, XML_val, ToPsz10(bStrikeout));
    }

    if (maDxfData.bOutline)
    {
        rStyleSheet->singleElement(XML_outline, XML_val, ToPsz10(*maDxfData.bOutline));
    }

    if (maDxfData.bShadow)
    {
        rStyleSheet->singleElement(XML_shadow, XML_val, ToPsz10(*maDxfData.bShadow));
    }

    if (maDxfData.aColor)
    {
        rStyleSheet->singleElement(XML_color,
                XML_rgb, XclXmlUtils::ToOString(*maDxfData.aColor));
    }

    if (maDxfData.nFontHeight)
    {
        rStyleSheet->singleElement(XML_sz,
                XML_val, OString::number(*maDxfData.nFontHeight/20));
    }

    if (maDxfData.eUnder)
    {
        const char* pVal = getUnderlineOOXValue(*maDxfData.eUnder);
        rStyleSheet->singleElement(XML_u, XML_val, pVal);
    }

    rStyleSheet->endElement(XML_font);
}

XclExpBlindFont::XclExpBlindFont( const XclExpRoot& rRoot ) :
    XclExpFont( rRoot, XclFontData(), EXC_COLOR_CELLTEXT )
{
}

bool XclExpBlindFont::Equals( const XclFontData& /*rFontData*/, sal_uInt32 /*nHash*/ ) const
{
    return false;
}

void XclExpBlindFont::Save( XclExpStream& /*rStrm*/ )
{
    // do nothing
}

XclExpFontBuffer::XclExpFontBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mnXclMaxSize( 0 )
{
    switch( GetBiff() )
    {
        case EXC_BIFF4: mnXclMaxSize = EXC_FONT_MAXCOUNT4;  break;
        case EXC_BIFF5: mnXclMaxSize = EXC_FONT_MAXCOUNT5;  break;
        case EXC_BIFF8: mnXclMaxSize = EXC_FONT_MAXCOUNT8;  break;
        default:        DBG_ERROR_BIFF();
    }
    InitDefaultFonts();
}

const XclExpFont* XclExpFontBuffer::GetFont( sal_uInt16 nXclFont ) const
{
    return maFontList.GetRecord( nXclFont );
}

const XclFontData& XclExpFontBuffer::GetAppFontData() const
{
    return maFontList.GetRecord( EXC_FONT_APP )->GetFontData(); // exists always
}

sal_uInt16 XclExpFontBuffer::Insert(
        const XclFontData& rFontData, XclExpColorType eColorType, bool bAppFont )
{
    if( bAppFont )
    {
        XclExpFontRef xFont = new XclExpFont( GetRoot(), rFontData, eColorType );
        maFontList.ReplaceRecord( xFont, EXC_FONT_APP );
        // set width of '0' character for column width export
        SetCharWidth( xFont->GetFontData() );
        return EXC_FONT_APP;
    }

    size_t nPos = Find( rFontData );
    if( nPos == EXC_FONTLIST_NOTFOUND )
    {
        // not found in buffer - create new font
        size_t nSize = maFontList.GetSize();
        if( nSize < mnXclMaxSize )
        {
            // possible to insert
            maFontList.AppendNewRecord( new XclExpFont( GetRoot(), rFontData, eColorType ) );
            nPos = nSize;       // old size is last position now
        }
        else
        {
            // buffer is full - ignore new font, use default font
            nPos = EXC_FONT_APP;
        }
    }
    return static_cast< sal_uInt16 >( nPos );
}

sal_uInt16 XclExpFontBuffer::Insert(const SvxFont& rFont, model::ComplexColor const& rComplexColor, XclExpColorType eColorType )
{
    return Insert(XclFontData(rFont, rComplexColor), eColorType);
}

sal_uInt16 XclExpFontBuffer::Insert(const SfxItemSet& rItemSet, sal_Int16 nScript, XclExpColorType eColorType, bool bAppFont )
{
    // #i17050# script type now provided by caller
    vcl::Font aFont = XclExpFontHelper::GetFontFromItemSet(GetRoot(), rItemSet, nScript);
    model::ComplexColor aComplexColor;
    ScPatternAttr::fillColor(aComplexColor, rItemSet, ScAutoFontColorMode::Raw);
    return Insert(XclFontData(aFont, aComplexColor), eColorType, bAppFont );
}

void XclExpFontBuffer::Save( XclExpStream& rStrm )
{
    maFontList.Save( rStrm );
}

void XclExpFontBuffer::SaveXml( XclExpXmlStream& rStrm )
{
    if( maFontList.IsEmpty() )
        return;

    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->startElement(XML_fonts, XML_count, OString::number(maFontList.GetSize()));

    maFontList.SaveXml( rStrm );

    rStyleSheet->endElement( XML_fonts );
}

// private --------------------------------------------------------------------

void XclExpFontBuffer::InitDefaultFonts()
{
    XclFontData aFontData;
    aFontData.maName = "Arial";
    aFontData.SetScFamily( FAMILY_DONTKNOW );
    aFontData.SetFontEncoding( ScfTools::GetSystemTextEncoding() );
    aFontData.SetScHeight( 200 );   // 200 twips = 10 pt
    aFontData.SetScWeight( WEIGHT_NORMAL );

    switch( GetBiff() )
    {
        case EXC_BIFF5:
        {
            maFontList.AppendNewRecord( new XclExpFont( GetRoot(), aFontData, EXC_COLOR_CELLTEXT ) );
            aFontData.SetScWeight( WEIGHT_BOLD );
            maFontList.AppendNewRecord( new XclExpFont( GetRoot(), aFontData, EXC_COLOR_CELLTEXT ) );
            aFontData.SetScWeight( WEIGHT_NORMAL );
            aFontData.SetScPosture( ITALIC_NORMAL );
            maFontList.AppendNewRecord( new XclExpFont( GetRoot(), aFontData, EXC_COLOR_CELLTEXT ) );
            aFontData.SetScWeight( WEIGHT_BOLD );
            maFontList.AppendNewRecord( new XclExpFont( GetRoot(), aFontData, EXC_COLOR_CELLTEXT ) );
            // the blind font with index 4
            maFontList.AppendNewRecord( new XclExpBlindFont( GetRoot() ) );
            // already add the first user defined font (Excel does it too)
            aFontData.SetScWeight( WEIGHT_NORMAL );
            aFontData.SetScPosture( ITALIC_NONE );
            maFontList.AppendNewRecord( new XclExpFont( GetRoot(), aFontData, EXC_COLOR_CELLTEXT ) );
        }
        break;
        case EXC_BIFF8:
        {
            XclExpFontRef xFont = new XclExpFont( GetRoot(), aFontData, EXC_COLOR_CELLTEXT );
            maFontList.AppendRecord( xFont );
            maFontList.AppendRecord( xFont );
            maFontList.AppendRecord( xFont );
            maFontList.AppendRecord( xFont );
            if( GetOutput() == EXC_OUTPUT_BINARY )
                // the blind font with index 4
                maFontList.AppendNewRecord( new XclExpBlindFont( GetRoot() ) );
        }
        break;
        default:
            DBG_ERROR_BIFF();
    }
}

size_t XclExpFontBuffer::Find( const XclFontData& rFontData )
{
    sal_uInt32 nHash = lclCalcHash( rFontData );
    for( size_t nPos = 0, nSize = maFontList.GetSize(); nPos < nSize; ++nPos )
        if( maFontList.GetRecord( nPos )->Equals( rFontData, nHash ) )
            return nPos;
    return EXC_FONTLIST_NOTFOUND;
}

// FORMAT record - number formats =============================================

namespace {

/** Predicate for search algorithm. */
struct XclExpNumFmtPred
{
    sal_uInt32   mnScNumFmt;
    explicit     XclExpNumFmtPred( sal_uInt32 nScNumFmt ) : mnScNumFmt( nScNumFmt ) {}
    bool         operator()( const XclExpNumFmt& rFormat ) const
                            { return rFormat.mnScNumFmt == mnScNumFmt; }
};

}

void XclExpNumFmt::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->singleElement( XML_numFmt,
            XML_numFmtId,   OString::number(mnXclNumFmt),
            XML_formatCode, maNumFmtString );
}

XclExpNumFmtBuffer::XclExpNumFmtBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot ),
    mxFormatter( new SvNumberFormatter( comphelper::getProcessComponentContext(), LANGUAGE_ENGLISH_US ) ),
    mpKeywordTable( new NfKeywordTable ),
    mnStdFmt( GetFormatter().GetStandardIndex( ScGlobal::eLnge ) )
{
    switch( GetBiff() )
    {
        case EXC_BIFF5: mnXclOffset = EXC_FORMAT_OFFSET5;   break;
        case EXC_BIFF8: mnXclOffset = EXC_FORMAT_OFFSET8;   break;
        default:        mnXclOffset = 0; DBG_ERROR_BIFF();
    }

    mxFormatter->FillKeywordTableForExcel( *mpKeywordTable );
}

XclExpNumFmtBuffer::~XclExpNumFmtBuffer()
{
}

sal_uInt16 XclExpNumFmtBuffer::Insert( sal_uInt32 nScNumFmt )
{
    XclExpNumFmtVec::const_iterator aIt =
        ::std::find_if( maFormatMap.begin(), maFormatMap.end(), XclExpNumFmtPred( nScNumFmt ) );
    if( aIt != maFormatMap.end() )
        return aIt->mnXclNumFmt;

    size_t nSize = maFormatMap.size();
    if( nSize < o3tl::make_unsigned( 0xFFFF - mnXclOffset ) )
    {
        sal_uInt16 nXclNumFmt = static_cast< sal_uInt16 >( nSize + mnXclOffset );
        maFormatMap.emplace_back( nScNumFmt, nXclNumFmt, GetFormatCode( nScNumFmt ) );
        return nXclNumFmt;
    }

    return 0;
}

void XclExpNumFmtBuffer::Save( XclExpStream& rStrm )
{
    for( const auto& rEntry : maFormatMap )
        WriteFormatRecord( rStrm, rEntry );
}

void XclExpNumFmtBuffer::SaveXml( XclExpXmlStream& rStrm )
{
    if( maFormatMap.empty() )
        return;

    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->startElement(XML_numFmts, XML_count, OString::number(maFormatMap.size()));
    for( auto& rEntry : maFormatMap )
    {
        rEntry.SaveXml( rStrm );
    }
    rStyleSheet->endElement( XML_numFmts );
}

void XclExpNumFmtBuffer::WriteFormatRecord( XclExpStream& rStrm, sal_uInt16 nXclNumFmt, const OUString& rFormatStr )
{
    XclExpString aExpStr;
    if( GetBiff() <= EXC_BIFF5 )
        aExpStr.AssignByte( rFormatStr, GetTextEncoding(), XclStrFlags::EightBitLength );
    else
        aExpStr.Assign( rFormatStr );

    rStrm.StartRecord( EXC_ID4_FORMAT, 2 + aExpStr.GetSize() );
    rStrm << nXclNumFmt << aExpStr;
    rStrm.EndRecord();
}

void XclExpNumFmtBuffer::WriteFormatRecord( XclExpStream& rStrm, const XclExpNumFmt& rFormat )
{
    WriteFormatRecord( rStrm, rFormat.mnXclNumFmt, GetFormatCode( rFormat.mnScNumFmt ) );
}

namespace {

OUString GetNumberFormatCode(const XclRoot& rRoot, const sal_uInt32 nScNumFmt, SvNumberFormatter* pFormatter, const NfKeywordTable* pKeywordTable)
{
    return rRoot.GetFormatter().GetFormatStringForExcel( nScNumFmt, *pKeywordTable, *pFormatter);
}

}

OUString XclExpNumFmtBuffer::GetFormatCode( sal_uInt32 nScNumFmt )
{
    return GetNumberFormatCode( *this, nScNumFmt, mxFormatter.get(), mpKeywordTable.get() );
}

// XF, STYLE record - Cell formatting =========================================

bool XclExpCellProt::FillFromItemSet( const SfxItemSet& rItemSet, bool bStyle )
{
    const ScProtectionAttr& rProtItem = rItemSet.Get( ATTR_PROTECTION );
    mbLocked = rProtItem.GetProtection();
    mbHidden = rProtItem.GetHideFormula() || rProtItem.GetHideCell();
    return ScfTools::CheckItem( rItemSet, ATTR_PROTECTION, bStyle );
}

void XclExpCellProt::FillToXF3( sal_uInt16& rnProt ) const
{
    ::set_flag( rnProt, EXC_XF_LOCKED, mbLocked );
    ::set_flag( rnProt, EXC_XF_HIDDEN, mbHidden );
}

void XclExpCellProt::SaveXml( XclExpXmlStream& rStrm ) const
{
    rStrm.GetCurrentStream()->singleElement( XML_protection,
            XML_locked,     ToPsz( mbLocked ),
            XML_hidden,     ToPsz( mbHidden ) );
}

bool XclExpCellAlign::FillFromItemSet(const XclRoot& rRoot, const SfxItemSet& rItemSet,
                                      bool bForceLineBreak, XclBiff eBiff, bool bStyle)
{
    bool bUsed = false;
    SvxCellHorJustify eHorAlign = rItemSet.Get( ATTR_HOR_JUSTIFY ).GetValue();
    SvxCellVerJustify eVerAlign = rItemSet.Get( ATTR_VER_JUSTIFY ).GetValue();

    switch( eBiff )
    {
        case EXC_BIFF8: // attributes new in BIFF8
        {
            // text indent
            tools::Long nTmpIndent = rItemSet.Get( ATTR_INDENT ).GetValue();    // already in twips
            tools::Long nSpaceWidth = rRoot.GetSpaceWidth();
            sal_Int32 nIndent = static_cast<double>(nTmpIndent) / (3.0 * nSpaceWidth) + 0.5;
            mnIndent = limit_cast< sal_uInt8 >( nIndent, 0, 15 );
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_INDENT, bStyle );

            // shrink to fit
            mbShrink = rItemSet.Get( ATTR_SHRINKTOFIT ).GetValue();
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_SHRINKTOFIT, bStyle );

            // CTL text direction
            SetScFrameDir( rItemSet.Get( ATTR_WRITINGDIR ).GetValue() );
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_WRITINGDIR, bStyle );

            [[fallthrough]];
        }

        case EXC_BIFF5: // attributes new in BIFF5
        case EXC_BIFF4: // attributes new in BIFF4
        {
            // vertical alignment
            SetScVerAlign( eVerAlign );
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_VER_JUSTIFY, bStyle );

            // stacked/rotation
            bool bStacked = rItemSet.Get( ATTR_STACKED ).GetValue();
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_STACKED, bStyle );
            if( bStacked )
            {
                mnRotation = EXC_ROT_STACKED;
            }
            else
            {
                // rotation
                Degree100 nScRot = rItemSet.Get( ATTR_ROTATE_VALUE ).GetValue();
                mnRotation = XclTools::GetXclRotation( nScRot );
                bUsed |= ScfTools::CheckItem( rItemSet, ATTR_ROTATE_VALUE, bStyle );
            }
            mnOrient = XclTools::GetXclOrientFromRot( mnRotation );

            [[fallthrough]];
        }

        case EXC_BIFF3: // attributes new in BIFF3
        {
            // text wrap
            mbLineBreak = bForceLineBreak || rItemSet.Get( ATTR_LINEBREAK ).GetValue();
            bUsed |= bForceLineBreak || ScfTools::CheckItem( rItemSet, ATTR_LINEBREAK, bStyle );

            [[fallthrough]];
        }

        case EXC_BIFF2: // attributes new in BIFF2
        {
            // horizontal alignment
            SetScHorAlign( eHorAlign );
            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_HOR_JUSTIFY, bStyle );
        }

        break;
        default:    DBG_ERROR_BIFF();
    }

    if (eBiff == EXC_BIFF8)
    {
        // Adjust for distributed alignments.
        if (eHorAlign == SvxCellHorJustify::Block)
        {
            SvxCellJustifyMethod eHorJustMethod =
                rItemSet.GetItem<SvxJustifyMethodItem>(ATTR_HOR_JUSTIFY_METHOD)->GetValue();
            if (eHorJustMethod == SvxCellJustifyMethod::Distribute)
                mnHorAlign = EXC_XF_HOR_DISTRIB;
        }

        if (eVerAlign == SvxCellVerJustify::Block)
        {
            SvxCellJustifyMethod eVerJustMethod =
                rItemSet.GetItem<SvxJustifyMethodItem>(ATTR_VER_JUSTIFY_METHOD)->GetValue();
            if (eVerJustMethod == SvxCellJustifyMethod::Distribute)
                mnVerAlign = EXC_XF_VER_DISTRIB;
        }
    }

    return bUsed;
}

void XclExpCellAlign::FillToXF5( sal_uInt16& rnAlign ) const
{
    ::insert_value( rnAlign, mnHorAlign, 0, 3 );
    ::set_flag( rnAlign, EXC_XF_LINEBREAK, mbLineBreak );
    ::insert_value( rnAlign, mnVerAlign, 4, 3 );
    ::insert_value( rnAlign, mnOrient, 8, 2 );
}

void XclExpCellAlign::FillToXF8( sal_uInt16& rnAlign, sal_uInt16& rnMiscAttrib ) const
{
    ::insert_value( rnAlign, mnHorAlign, 0, 3 );
    ::set_flag( rnAlign, EXC_XF_LINEBREAK, mbLineBreak );
    ::insert_value( rnAlign, mnVerAlign, 4, 3 );
    ::insert_value( rnAlign, mnRotation, 8, 8 );
    ::insert_value( rnMiscAttrib, mnIndent, 0, 4 );
    ::set_flag( rnMiscAttrib, EXC_XF8_SHRINK, mbShrink );
    ::insert_value( rnMiscAttrib, mnTextDir, 6, 2 );
}

static const char* ToHorizontalAlignment( sal_uInt8 nHorAlign )
{
    switch( nHorAlign )
    {
        case EXC_XF_HOR_GENERAL:    return "general";
        case EXC_XF_HOR_LEFT:       return "left";
        case EXC_XF_HOR_CENTER:     return "center";
        case EXC_XF_HOR_RIGHT:      return "right";
        case EXC_XF_HOR_FILL:       return "fill";
        case EXC_XF_HOR_JUSTIFY:    return "justify";
        case EXC_XF_HOR_CENTER_AS:  return "centerContinuous";
        case EXC_XF_HOR_DISTRIB:    return "distributed";
    }
    return "*unknown*";
}

static const char* ToVerticalAlignment( sal_uInt8 nVerAlign )
{
    switch( nVerAlign )
    {
        case EXC_XF_VER_TOP:        return "top";
        case EXC_XF_VER_CENTER:     return "center";
        case EXC_XF_VER_BOTTOM:     return "bottom";
        case EXC_XF_VER_JUSTIFY:    return "justify";
        case EXC_XF_VER_DISTRIB:    return "distributed";
    }
    return "*unknown*";
}

void XclExpCellAlign::SaveXml( XclExpXmlStream& rStrm ) const
{
    rStrm.GetCurrentStream()->singleElement( XML_alignment,
            XML_horizontal,         ToHorizontalAlignment( mnHorAlign ),
            XML_vertical,           ToVerticalAlignment( mnVerAlign ),
            XML_textRotation,       OString::number(mnRotation),
            XML_wrapText,           ToPsz( mbLineBreak ),
            XML_indent,             OString::number(mnIndent),
            // OOXTODO: XML_relativeIndent,     mnIndent?
            // OOXTODO: XML_justifyLastLine,
            XML_shrinkToFit,        ToPsz( mbShrink ),
            XML_readingOrder, sax_fastparser::UseIf(OString::number(mnTextDir), mnTextDir != EXC_XF_TEXTDIR_CONTEXT) );
}

namespace {

void lclGetBorderLine(
        sal_uInt8& rnXclLine, sal_uInt32& rnColorId, model::ComplexColor& rComplexColor,
        const ::editeng::SvxBorderLine* pLine, XclExpPalette& rPalette, XclBiff eBiff )
{
    // Document: sc/qa/unit/data/README.cellborders

    enum CalcLineIndex{Idx_None, Idx_Solid, Idx_Dotted, Idx_Dashed, Idx_FineDashed, Idx_DashDot, Idx_DashDotDot, Idx_DoubleThin, Idx_Last};
    enum ExcelWidthIndex{Width_Hair, Width_Thin, Width_Medium, Width_Thick, Width_Last};
    static const sal_uInt8 Map_LineLO_toMS[Idx_Last][Width_Last] =
    {
    //    0,05  -  0,74                  0,75  -  1,49                   1,50  -  2,49                 2,50  -  9,00          Width Range [pt]
    //   EXC_BORDER_HAIR                EXC_BORDER_THIN                EXC_BORDER_MEDIUM              EXC_BORDER_THICK        MS Width
        {EXC_LINE_NONE                , EXC_LINE_NONE                , EXC_LINE_NONE                , EXC_LINE_NONE                }, //  0    BorderLineStyle::NONE
        {EXC_LINE_HAIR                , EXC_LINE_THIN                , EXC_LINE_MEDIUM              , EXC_LINE_THICK               }, //  1    BorderLineStyle::SOLID
        {EXC_LINE_DOTTED              , EXC_LINE_DOTTED              , EXC_LINE_MEDIUM_SLANT_DASHDOT, EXC_LINE_MEDIUM_SLANT_DASHDOT}, //  2    BorderLineStyle::DOTTED
        {EXC_LINE_DOTTED              , EXC_LINE_DASHED              , EXC_LINE_MEDIUM_DASHED       , EXC_LINE_MEDIUM_DASHED       }, //  3    BorderLineStyle::DASHED
        {EXC_LINE_DASHED              , EXC_LINE_DASHED              , EXC_LINE_MEDIUM_SLANT_DASHDOT, EXC_LINE_MEDIUM_SLANT_DASHDOT}, //  4    BorderLineStyle::FINE_DASHED
        {EXC_LINE_DASHED              , EXC_LINE_THIN_DASHDOT        , EXC_LINE_MEDIUM_DASHDOT      , EXC_LINE_MEDIUM_DASHDOT      }, //  5    BorderLineStyle::DASH_DOT
        {EXC_LINE_DASHED              , EXC_LINE_THIN_DASHDOTDOT     , EXC_LINE_MEDIUM_DASHDOTDOT   , EXC_LINE_MEDIUM_DASHDOTDOT   }, //  6    BorderLineStyle::DASH_DOT_DOT
        {EXC_LINE_DOUBLE              , EXC_LINE_DOUBLE              , EXC_LINE_DOUBLE              , EXC_LINE_DOUBLE              }  //  7    BorderLineStyle::DOUBLE_THIN
    };                                                                                                                                // Line  Name

    rnXclLine = EXC_LINE_NONE;
    if( pLine )
    {
        sal_uInt16 nOuterWidth = pLine->GetOutWidth();
        ExcelWidthIndex nOuterWidthIndx;
        CalcLineIndex  nStyleIndex;

        switch (pLine->GetBorderLineStyle())
        {
            case SvxBorderLineStyle::NONE:
                nStyleIndex = Idx_None;
                break;
            case SvxBorderLineStyle::SOLID:
                nStyleIndex = Idx_Solid;
                break;
            case SvxBorderLineStyle::DOTTED:
                nStyleIndex = Idx_Dotted;
                break;
            case SvxBorderLineStyle::DASHED:
                nStyleIndex = Idx_Dashed;
                break;
            case SvxBorderLineStyle::FINE_DASHED:
                nStyleIndex = Idx_FineDashed;
                break;
            case SvxBorderLineStyle::DASH_DOT:
                nStyleIndex = Idx_DashDot;
                break;
            case SvxBorderLineStyle::DASH_DOT_DOT:
                nStyleIndex = Idx_DashDotDot;
                break;
            case SvxBorderLineStyle::DOUBLE_THIN:
                // the "nOuterWidth" is not right for this line type
                // but at the moment width it not important for that
                // the right function is nOuterWidth = (sal_uInt16) pLine->GetWidth();
                nStyleIndex = Idx_DoubleThin;
                break;
            default:
                nStyleIndex = Idx_Solid;
        }

        if( nOuterWidth >= EXC_BORDER_THICK )
            nOuterWidthIndx = Width_Thick;
        else if( nOuterWidth >= EXC_BORDER_MEDIUM )
            nOuterWidthIndx = Width_Medium;
        else if( nOuterWidth >= EXC_BORDER_THIN )
            nOuterWidthIndx = Width_Thin;
        else if ( nOuterWidth >= EXC_BORDER_HAIR )
            nOuterWidthIndx = Width_Hair;
        else
            nOuterWidthIndx = Width_Thin;

        rnXclLine = Map_LineLO_toMS[nStyleIndex][nOuterWidthIndx];
    }

    if( (eBiff == EXC_BIFF2) && (rnXclLine != EXC_LINE_NONE) )
        rnXclLine = EXC_LINE_THIN;

    if (pLine && (rnXclLine != EXC_LINE_NONE))
    {
        rnColorId = rPalette.InsertColor(pLine->GetColor(), EXC_COLOR_CELLBORDER);
        rComplexColor = pLine->getComplexColor();
    }
    else
    {
        rnColorId = XclExpPalette::GetColorIdFromIndex(0);
    }
}

} // namespace

XclExpCellBorder::XclExpCellBorder() :
    mnLeftColorId(   XclExpPalette::GetColorIdFromIndex( mnLeftColor ) ),
    mnRightColorId(  XclExpPalette::GetColorIdFromIndex( mnRightColor ) ),
    mnTopColorId(    XclExpPalette::GetColorIdFromIndex( mnTopColor ) ),
    mnBottomColorId( XclExpPalette::GetColorIdFromIndex( mnBottomColor ) ),
    mnDiagColorId(   XclExpPalette::GetColorIdFromIndex( mnDiagColor ) )
{
}

bool XclExpCellBorder::FillFromItemSet(
        const SfxItemSet& rItemSet, XclExpPalette& rPalette, XclBiff eBiff, bool bStyle )
{
    bool bUsed = false;

    switch( eBiff )
    {
        case EXC_BIFF8: // attributes new in BIFF8
        {
            const SvxLineItem& rTLBRItem = rItemSet.Get( ATTR_BORDER_TLBR );
            sal_uInt8 nTLBRLine;
            sal_uInt32 nTLBRColorId;
            model::ComplexColor aTLBRComplexColor;
            lclGetBorderLine( nTLBRLine, nTLBRColorId, aTLBRComplexColor, rTLBRItem.GetLine(), rPalette, eBiff );
            mbDiagTLtoBR = (nTLBRLine != EXC_LINE_NONE);

            const SvxLineItem& rBLTRItem = rItemSet.Get( ATTR_BORDER_BLTR );
            sal_uInt8 nBLTRLine;
            sal_uInt32 nBLTRColorId;
            model::ComplexColor aBLTRComplexColor;
            lclGetBorderLine( nBLTRLine, nBLTRColorId, aBLTRComplexColor, rBLTRItem.GetLine(), rPalette, eBiff );
            mbDiagBLtoTR = (nBLTRLine != EXC_LINE_NONE);

            if( ::ScHasPriority( rTLBRItem.GetLine(), rBLTRItem.GetLine() ) )
            {
                mnDiagLine = nTLBRLine;
                mnDiagColorId = nTLBRColorId;
            }
            else
            {
                mnDiagLine = nBLTRLine;
                mnDiagColorId = nBLTRColorId;
            }

            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_BORDER_TLBR, bStyle ) ||
                     ScfTools::CheckItem( rItemSet, ATTR_BORDER_BLTR, bStyle );

            [[fallthrough]];
        }

        case EXC_BIFF5:
        case EXC_BIFF4:
        case EXC_BIFF3:
        case EXC_BIFF2:
        {
            const SvxBoxItem& rBoxItem = rItemSet.Get( ATTR_BORDER );

            lclGetBorderLine(mnLeftLine, mnLeftColorId, maComplexColorLeft, rBoxItem.GetLeft(), rPalette, eBiff);
            lclGetBorderLine(mnRightLine, mnRightColorId, maComplexColorRight, rBoxItem.GetRight(), rPalette, eBiff);
            lclGetBorderLine(mnTopLine, mnTopColorId, maComplexColorTop, rBoxItem.GetTop(), rPalette, eBiff);
            lclGetBorderLine(mnBottomLine, mnBottomColorId, maComplexColorBottom, rBoxItem.GetBottom(), rPalette, eBiff);

            bUsed |= ScfTools::CheckItem( rItemSet, ATTR_BORDER, bStyle );
        }

        break;
        default:    DBG_ERROR_BIFF();
    }

    return bUsed;
}

void XclExpCellBorder::SetFinalColors( const XclExpPalette& rPalette )
{
    mnLeftColor   = rPalette.GetColorIndex( mnLeftColorId );
    mnRightColor  = rPalette.GetColorIndex( mnRightColorId );
    mnTopColor    = rPalette.GetColorIndex( mnTopColorId );
    mnBottomColor = rPalette.GetColorIndex( mnBottomColorId );
    mnDiagColor   = rPalette.GetColorIndex( mnDiagColorId );
}

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
    ::insert_value( rnBorder2, mnDiagColor,   14, 7 );
    ::insert_value( rnBorder2, mnDiagLine,    21, 4 );
    ::set_flag( rnBorder1, EXC_XF_DIAGONAL_TL_TO_BR, mbDiagTLtoBR );
    ::set_flag( rnBorder1, EXC_XF_DIAGONAL_BL_TO_TR, mbDiagBLtoTR );
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

static const char* ToLineStyle( sal_uInt8 nLineStyle )
{
    switch( nLineStyle )
    {
        case EXC_LINE_NONE:              return "none";
        case EXC_LINE_THIN:              return "thin";
        case EXC_LINE_MEDIUM:            return "medium";
        case EXC_LINE_THICK:             return "thick";
        case EXC_LINE_DOUBLE:            return "double";
        case EXC_LINE_HAIR:              return "hair";
        case EXC_LINE_DOTTED:            return "dotted";
        case EXC_LINE_DASHED:            return "dashed";
        case EXC_LINE_MEDIUM_DASHED:     return "mediumDashed";
        case EXC_LINE_THIN_DASHDOT:      return "dashDot";
        case EXC_LINE_THIN_DASHDOTDOT:   return "dashDotDot";
        case EXC_LINE_MEDIUM_DASHDOT:    return "mediumDashDot";
        case EXC_LINE_MEDIUM_DASHDOTDOT: return "mediumDashDotDot";
        case EXC_LINE_MEDIUM_SLANT_DASHDOT: return "slantDashDot";
    }
    return "*unknown*";
}

namespace
{
void lcl_WriteBorder(XclExpXmlStream& rStrm, sal_Int32 nElement, sal_uInt8 nLineStyle, const Color& rColor, model::ComplexColor const& rComplexColor)
{
    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    if( nLineStyle == EXC_LINE_NONE )
    {
        rStyleSheet->singleElement(nElement);
        return;
    }
    else if (rColor == Color(0, 0, 0) && !rComplexColor.isValidThemeType())
    {
        rStyleSheet->singleElement(nElement, XML_style, ToLineStyle(nLineStyle));
        return;
    }

    rStyleSheet->startElement(nElement, XML_style, ToLineStyle(nLineStyle));
    oox::xls::writeComplexColor(rStyleSheet, XML_color, rComplexColor, rColor);
    rStyleSheet->endElement(nElement);
}
} // end anonymous namespace

void XclExpCellBorder::SaveXml(XclExpXmlStream& rStream) const
{
    sax_fastparser::FSHelperPtr& rStyleSheet = rStream.GetCurrentStream();

    XclExpPalette& rPalette = rStream.GetRoot().GetPalette();

    rStyleSheet->startElement( XML_border,
            XML_diagonalUp,     ToPsz( mbDiagBLtoTR ),
            XML_diagonalDown,   ToPsz( mbDiagTLtoBR )
            // OOXTODO: XML_outline
    );

    lcl_WriteBorder(rStream, XML_left, mnLeftLine, rPalette.GetColor(mnLeftColor), maComplexColorLeft);
    lcl_WriteBorder(rStream, XML_right, mnRightLine, rPalette.GetColor(mnRightColor), maComplexColorRight);
    lcl_WriteBorder(rStream, XML_top, mnTopLine, rPalette.GetColor(mnTopColor), maComplexColorTop);
    lcl_WriteBorder(rStream, XML_bottom, mnBottomLine, rPalette.GetColor(mnBottomColor), maComplexColorBottom);
    lcl_WriteBorder(rStream, XML_diagonal, mnDiagLine, rPalette.GetColor(mnDiagColor), maComplexColorDiagonal);

    // OOXTODO: XML_vertical, XML_horizontal
    rStyleSheet->endElement( XML_border );
}

XclExpCellArea::XclExpCellArea() :
    mnForeColorId(XclExpPalette::GetColorIdFromIndex(mnForeColor)),
    mnBackColorId(XclExpPalette::GetColorIdFromIndex(mnBackColor)),
    maForeColor(COL_TRANSPARENT),
    maBackColor(COL_TRANSPARENT)
{
}

XclExpCellArea::XclExpCellArea(Color aForeColor, Color aBackColor)
    : XclCellArea(EXC_PATT_SOLID)
    , mnForeColorId(0)
    , mnBackColorId(0)
    , maForeColor(aForeColor)
    , maBackColor(aBackColor)
{
}

bool XclExpCellArea::FillFromItemSet( const SfxItemSet& rItemSet, XclExpPalette& rPalette, bool bStyle )
{
    const SvxBrushItem& rBrushItem = rItemSet.Get( ATTR_BACKGROUND );

    if (rBrushItem.getComplexColor().getType() != model::ColorType::Unused)
        maForegroundComplexColor = rBrushItem.getComplexColor();

    if( rBrushItem.GetColor().IsTransparent() )
    {
        mnPattern = EXC_PATT_NONE;
        mnForeColorId = XclExpPalette::GetColorIdFromIndex( EXC_COLOR_WINDOWTEXT );
        mnBackColorId = XclExpPalette::GetColorIdFromIndex( EXC_COLOR_WINDOWBACK );
    }
    else
    {
        mnPattern = EXC_PATT_SOLID;
        mnForeColorId = rPalette.InsertColor( rBrushItem.GetColor(), EXC_COLOR_CELLAREA );
        mnBackColorId = XclExpPalette::GetColorIdFromIndex( EXC_COLOR_WINDOWTEXT );
    }
    return ScfTools::CheckItem( rItemSet, ATTR_BACKGROUND, bStyle );
}

void XclExpCellArea::SetFinalColors( const XclExpPalette& rPalette )
{
    rPalette.GetMixedColors( mnForeColor, mnBackColor, mnPattern, mnForeColorId, mnBackColorId );
}

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

static const char* ToPatternType( sal_uInt8 nPattern )
{
    switch( nPattern )
    {
        case EXC_PATT_NONE:         return "none";
        case EXC_PATT_SOLID:        return "solid";
        case EXC_PATT_50_PERC:      return "mediumGray";
        case EXC_PATT_75_PERC:      return "darkGray";
        case EXC_PATT_25_PERC:      return "lightGray";
        case EXC_PATT_12_5_PERC:    return "gray125";
        case EXC_PATT_6_25_PERC:    return "gray0625";
    }
    return "*unknown*";
}

void XclExpCellArea::SaveXml( XclExpXmlStream& rStrm ) const
{
    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->startElement(XML_fill);

    // OOXTODO: XML_gradientFill

    XclExpPalette& rPalette = rStrm.GetRoot().GetPalette();

    if (mnPattern == EXC_PATT_NONE ||
        (mnForeColor == 0 && mnBackColor == 0 && maForeColor == COL_TRANSPARENT && maBackColor == COL_TRANSPARENT))
    {
        rStyleSheet->singleElement(XML_patternFill, XML_patternType, ToPatternType(mnPattern));
    }
    else
    {
        rStyleSheet->startElement(XML_patternFill, XML_patternType, ToPatternType(mnPattern));

        if (maForeColor != COL_TRANSPARENT || maBackColor != COL_TRANSPARENT)
        {
            oox::xls::writeComplexColor(rStyleSheet, XML_fgColor, maForegroundComplexColor, maForeColor);
            oox::xls::writeComplexColor(rStyleSheet, XML_bgColor, maBackgroundComplexColor, maBackColor);
        }
        else
        {
            {
                Color aColor = rPalette.GetColor(mnForeColor);
                if (maForegroundComplexColor.isValidThemeType() || mnForeColor != 0)
                    oox::xls::writeComplexColor(rStyleSheet, XML_fgColor, maForegroundComplexColor, aColor);
                else if (mnForeColor != 0)
                    oox::xls::writeComplexColor(rStyleSheet, XML_fgColor, maForegroundComplexColor, aColor);
            }

            {
                Color aColor = rPalette.GetColor(mnBackColor);
                if (maBackgroundComplexColor.isValidThemeType() || mnBackColor != 0)
                    oox::xls::writeComplexColor(rStyleSheet, XML_bgColor, maBackgroundComplexColor, aColor);
            }
        }
        rStyleSheet->endElement( XML_patternFill );
    }

    rStyleSheet->endElement( XML_fill );
}

bool XclExpColor::FillFromItemSet( const SfxItemSet& rItemSet )
{
    if( !ScfTools::CheckItem( rItemSet, ATTR_BACKGROUND, true ) )
        return false;

    const SvxBrushItem& rBrushItem = rItemSet.Get( ATTR_BACKGROUND );
    maColor = rBrushItem.GetColor();
    maComplexColor = rBrushItem.getComplexColor();

    return true;
}

void XclExpColor::SaveXml( XclExpXmlStream& rStrm ) const
{
    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->startElement(XML_fill);
    rStyleSheet->startElement(XML_patternFill);

    oox::xls::writeComplexColor(rStyleSheet, XML_bgColor, maComplexColor, maColor);

    rStyleSheet->endElement( XML_patternFill );
    rStyleSheet->endElement( XML_fill );
}

XclExpXFId::XclExpXFId() :
    mnXFId( XclExpXFBuffer::GetDefCellXFId() ),
    mnXFIndex( EXC_XF_DEFAULTCELL )
{
}

void XclExpXFId::ConvertXFIndex( const XclExpRoot& rRoot )
{
    mnXFIndex = rRoot.GetXFBuffer().GetXFIndex( mnXFId );
}

XclExpXF::XclExpXF(
        const XclExpRoot& rRoot, const ScPatternAttr& rPattern, sal_Int16 nScript,
        sal_uInt32 nForceScNumFmt, sal_uInt16 nForceXclFont, bool bForceLineBreak ) :
    XclXFBase( true ),
    XclExpRoot( rRoot )
{
    mnParentXFId = GetXFBuffer().InsertStyle( rPattern.GetStyleSheet() );
    Init( rPattern.GetItemSet(), nScript, nForceScNumFmt, nForceXclFont, bForceLineBreak, false );
}

XclExpXF::XclExpXF( const XclExpRoot& rRoot, const SfxStyleSheetBase& rStyleSheet ) :
    XclXFBase( false ),
    XclExpRoot( rRoot ),
    mnParentXFId( XclExpXFBuffer::GetXFIdFromIndex( EXC_XF_STYLEPARENT ) )
{
    bool bDefStyle = (rStyleSheet.GetName() == ScResId( STR_STYLENAME_STANDARD ));
    sal_Int16 nScript = bDefStyle ? GetDefApiScript() : css::i18n::ScriptType::WEAK;
    Init( const_cast< SfxStyleSheetBase& >( rStyleSheet ).GetItemSet(), nScript,
        NUMBERFORMAT_ENTRY_NOT_FOUND, EXC_FONT_NOTFOUND, false, bDefStyle );
}

XclExpXF::XclExpXF( const XclExpRoot& rRoot, bool bCellXF ) :
    XclXFBase( bCellXF ),
    XclExpRoot( rRoot ),
    mnParentXFId( XclExpXFBuffer::GetXFIdFromIndex( EXC_XF_STYLEPARENT ) )
{
    InitDefault();
}

bool XclExpXF::Equals( const ScPatternAttr& rPattern,
        sal_uInt32 nForceScNumFmt, sal_uInt16 nForceXclFont, bool bForceLineBreak ) const
{
    return IsCellXF() && (mpItemSet == &rPattern.GetItemSet()) &&
        (!bForceLineBreak || maAlignment.mbLineBreak) &&
        ((nForceScNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND) || (mnScNumFmt == nForceScNumFmt)) &&
        ((nForceXclFont == EXC_FONT_NOTFOUND) || (mnXclFont == nForceXclFont));
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
        (mnXclFont    == rCmpXF.mnXclFont)    && (mnXclNumFmt  == rCmpXF.mnXclNumFmt) &&
        (mnParentXFId == rCmpXF.mnParentXFId);
}

void XclExpXF::InitDefault()
{
    SetRecHeader( EXC_ID5_XF, (GetBiff() == EXC_BIFF8) ? 20 : 16 );
    mpItemSet = nullptr;
    mnScNumFmt = NUMBERFORMAT_ENTRY_NOT_FOUND;
    mnXclFont = mnXclNumFmt = 0;
    SetXmlIds(0, 0);
}

void XclExpXF::Init( const SfxItemSet& rItemSet, sal_Int16 nScript,
        sal_uInt32 nForceScNumFmt, sal_uInt16 nForceXclFont, bool bForceLineBreak, bool bDefStyle )
{
    InitDefault();
    mpItemSet = &rItemSet;

    // cell protection
    mbProtUsed = maProtection.FillFromItemSet( rItemSet, IsStyleXF() );

    // font
    if( nForceXclFont == EXC_FONT_NOTFOUND )
    {
        mnXclFont = GetFontBuffer().Insert( rItemSet, nScript, EXC_COLOR_CELLTEXT, bDefStyle );
        mbFontUsed = XclExpFontHelper::CheckItems( GetRoot(), rItemSet, nScript, IsStyleXF() );
    }
    else
    {
        mnXclFont = nForceXclFont;
        mbFontUsed = true;
    }

    // number format
    if (nForceScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND)
        mnScNumFmt = nForceScNumFmt;
    else
    {
        // Built-in formats of dedicated languages may be attributed using the
        // system language (or even other?) format with a language attribute,
        // obtain the "real" format key.
        mnScNumFmt = rItemSet.Get( ATTR_VALUE_FORMAT ).GetValue();
        LanguageType nLang = rItemSet.Get( ATTR_LANGUAGE_FORMAT).GetLanguage();
        if (mnScNumFmt >= SV_COUNTRY_LANGUAGE_OFFSET || nLang != LANGUAGE_SYSTEM)
            mnScNumFmt = GetFormatter().GetFormatForLanguageIfBuiltIn( mnScNumFmt, nLang);
    }
    mnXclNumFmt = GetNumFmtBuffer().Insert( mnScNumFmt );
    mbFmtUsed = ScfTools::CheckItem( rItemSet, ATTR_VALUE_FORMAT, IsStyleXF() );

    // alignment
    mbAlignUsed = maAlignment.FillFromItemSet(*this, rItemSet, bForceLineBreak, GetBiff(), IsStyleXF());

    // cell border
    mbBorderUsed = maBorder.FillFromItemSet( rItemSet, GetPalette(), GetBiff(), IsStyleXF() );

    // background area
    mbAreaUsed = maArea.FillFromItemSet( rItemSet, GetPalette(), IsStyleXF() );

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

    rStrm << mnXclFont << mnXclNumFmt << nTypeProt << nAlign << nArea << nBorder;
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

    rStrm << mnXclFont << mnXclNumFmt << nTypeProt << nAlign << nMiscAttrib << nBorder1 << nBorder2 << nArea;
}

void XclExpXF::WriteBody( XclExpStream& rStrm )
{
    XclExpXFId aParentId( mnParentXFId );
    aParentId.ConvertXFIndex( GetRoot() );
    mnParent = aParentId.mnXFIndex;
    switch( GetBiff() )
    {
        case EXC_BIFF5: WriteBody5( rStrm );    break;
        case EXC_BIFF8: WriteBody8( rStrm );    break;
        default:        DBG_ERROR_BIFF();
    }
}

void XclExpXF::SetXmlIds( sal_uInt32 nBorderId, sal_uInt32 nFillId )
{
    mnBorderId = nBorderId;
    mnFillId   = nFillId;
}

void XclExpXF::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();

    sal_Int32 nXfId = 0;
    const XclExpXF* pStyleXF = nullptr;
    if( IsCellXF() )
    {
        sal_uInt16 nXFIndex = rStrm.GetRoot().GetXFBuffer().GetXFIndex( mnParentXFId );
        nXfId = rStrm.GetRoot().GetXFBuffer().GetXmlStyleIndex( nXFIndex );
        pStyleXF = rStrm.GetRoot().GetXFBuffer().GetXFById( mnParentXFId );
    }

    rStyleSheet->startElement( XML_xf,
            XML_numFmtId,           OString::number(mnXclNumFmt),
            XML_fontId,             OString::number(mnXclFont),
            XML_fillId,             OString::number(mnFillId),
            XML_borderId,           OString::number(mnBorderId),
            XML_xfId,               sax_fastparser::UseIf(OString::number(nXfId), !IsStyleXF()),
            // OOXTODO: XML_quotePrefix,
            // OOXTODO: XML_pivotButton,
            // OOXTODO: XML_applyNumberFormat,  ;
            XML_applyFont,          ToPsz( mbFontUsed ),
            // OOXTODO: XML_applyFill,
            XML_applyBorder,        ToPsz( mbBorderUsed ),
            XML_applyAlignment,     ToPsz( mbAlignUsed ),
            XML_applyProtection,    ToPsz( mbProtUsed ) );
    if( mbAlignUsed )
        maAlignment.SaveXml( rStrm );
    else if ( pStyleXF )
        pStyleXF->GetAlignmentData().SaveXml( rStrm );
    if( mbProtUsed )
        maProtection.SaveXml( rStrm );
    else if ( pStyleXF )
        pStyleXF->GetProtectionData().SaveXml( rStrm );

    // OOXTODO: XML_extLst
    rStyleSheet->endElement( XML_xf );
}

XclExpDefaultXF::XclExpDefaultXF( const XclExpRoot& rRoot, bool bCellXF ) :
    XclExpXF( rRoot, bCellXF )
{
}

void XclExpDefaultXF::SetFont( sal_uInt16 nXclFont )
{
    mnXclFont = nXclFont;
    mbFontUsed = true;
}

void XclExpDefaultXF::SetNumFmt( sal_uInt16 nXclNumFmt )
{
    mnXclNumFmt = nXclNumFmt;
    mbFmtUsed = true;
}

XclExpStyle::XclExpStyle( sal_uInt32 nXFId, OUString aStyleName ) :
    XclExpRecord( EXC_ID_STYLE, 4 ),
    maName(std::move( aStyleName )),
    maXFId( nXFId ),
    mnStyleId( EXC_STYLE_USERDEF ),
    mnLevel( EXC_STYLE_NOLEVEL )
{
    OSL_ENSURE( !maName.isEmpty(), "XclExpStyle::XclExpStyle - empty style name" );
#if OSL_DEBUG_LEVEL > 0
    sal_uInt8 nStyleId, nLevel; // do not use members for debug tests
    OSL_ENSURE( !XclTools::GetBuiltInStyleId( nStyleId, nLevel, maName ),
        "XclExpStyle::XclExpStyle - this is a built-in style" );
#endif
}

XclExpStyle::XclExpStyle( sal_uInt32 nXFId, sal_uInt8 nStyleId, sal_uInt8 nLevel ) :
    XclExpRecord( EXC_ID_STYLE, 4 ),
    maXFId( nXFId ),
    mnStyleId( nStyleId ),
    mnLevel( nLevel )
{
}

void XclExpStyle::WriteBody( XclExpStream& rStrm )
{
    maXFId.ConvertXFIndex( rStrm.GetRoot() );
    ::set_flag( maXFId.mnXFIndex, EXC_STYLE_BUILTIN, IsBuiltIn() );
    rStrm << maXFId.mnXFIndex;

    if( IsBuiltIn() )
    {
        rStrm << mnStyleId << mnLevel;
    }
    else
    {
        XclExpString aNameEx;
        if( rStrm.GetRoot().GetBiff() == EXC_BIFF8 )
            aNameEx.Assign( maName );
        else
            aNameEx.AssignByte( maName, rStrm.GetRoot().GetTextEncoding(), XclStrFlags::EightBitLength );
        rStrm << aNameEx;
    }
}

static const char* lcl_StyleNameFromId( sal_Int32 nStyleId )
{
    switch( nStyleId )
    {
        case 0:     return "Normal";
        case 3:     return "Comma";
        case 4:     return "Currency";
        case 5:     return "Percent";
        case 6:     return "Comma [0]";
        case 7:     return "Currency [0]";
    }
    return "*unknown*";
}

void XclExpStyle::SaveXml( XclExpXmlStream& rStrm )
{
    constexpr sal_Int32 CELL_STYLE_MAX_BUILTIN_ID = 54;
    OString sName;
    OString sBuiltinId;
    const char* pBuiltinId = nullptr;
    if( IsBuiltIn() )
    {
        sName = OString( lcl_StyleNameFromId( mnStyleId ) );
        sBuiltinId = OString::number( std::min( static_cast<sal_Int32>( CELL_STYLE_MAX_BUILTIN_ID - 1 ), static_cast <sal_Int32>( mnStyleId ) ) );
        pBuiltinId = sBuiltinId.getStr();
    }
    else
        sName = maName.toUtf8();

    // get the index in sortedlist associated with the mnXId
    sal_Int32 nXFId = rStrm.GetRoot().GetXFBuffer().GetXFIndex( maXFId.mnXFId );
    // get the style index associated with index into sortedlist
    nXFId = rStrm.GetRoot().GetXFBuffer().GetXmlStyleIndex( nXFId );
    rStrm.GetCurrentStream()->singleElement( XML_cellStyle,
            XML_name,      sName,
            XML_xfId,      OString::number(nXFId),
// builtinId of 54 or above is invalid according to OpenXML SDK validator.
            XML_builtinId, pBuiltinId
            // OOXTODO: XML_iLevel,
            // OOXTODO: XML_hidden,
            // XML_customBuiltin,  ToPsz( ! IsBuiltIn() )
    );
    // OOXTODO: XML_extLst
}

namespace {

const sal_uInt32 EXC_XFLIST_INDEXBASE   = 0xFFFE0000;
/** Maximum count of XF records to store in the XF list (performance). */
const sal_uInt32 EXC_XFLIST_HARDLIMIT   = 256 * 1024;

bool lclIsBuiltInStyle( const OUString& rStyleName )
{
    return
        XclTools::IsBuiltInStyleName( rStyleName ) ||
        XclTools::IsCondFormatStyleName( rStyleName );
}

} // namespace

XclExpXFBuffer::XclExpBuiltInInfo::XclExpBuiltInInfo() :
    mnStyleId( EXC_STYLE_USERDEF ),
    mnLevel( EXC_STYLE_NOLEVEL ),
    mbPredefined( true ),
    mbHasStyleRec( false )
{
}

namespace {

/** Predicate for search algorithm. */
struct XclExpBorderPred
{
    const XclExpCellBorder&
                        mrBorder;
    explicit     XclExpBorderPred( const XclExpCellBorder& rBorder ) : mrBorder( rBorder ) {}
    bool                operator()( const XclExpCellBorder& rBorder ) const;
};

}

bool XclExpBorderPred::operator()( const XclExpCellBorder& rBorder ) const
{
    return
        mrBorder.mnLeftColor     == rBorder.mnLeftColor &&
        mrBorder.mnRightColor    == rBorder.mnRightColor &&
        mrBorder.mnTopColor      == rBorder.mnTopColor &&
        mrBorder.mnBottomColor   == rBorder.mnBottomColor &&
        mrBorder.mnDiagColor     == rBorder.mnDiagColor &&
        mrBorder.mnLeftLine      == rBorder.mnLeftLine &&
        mrBorder.mnRightLine     == rBorder.mnRightLine &&
        mrBorder.mnTopLine       == rBorder.mnTopLine &&
        mrBorder.mnBottomLine    == rBorder.mnBottomLine &&
        mrBorder.mnDiagLine      == rBorder.mnDiagLine &&
        mrBorder.mbDiagTLtoBR    == rBorder.mbDiagTLtoBR &&
        mrBorder.mbDiagBLtoTR    == rBorder.mbDiagBLtoTR &&
        mrBorder.mnLeftColorId   == rBorder.mnLeftColorId &&
        mrBorder.mnRightColorId  == rBorder.mnRightColorId &&
        mrBorder.mnTopColorId    == rBorder.mnTopColorId &&
        mrBorder.mnBottomColorId == rBorder.mnBottomColorId &&
        mrBorder.mnDiagColorId   == rBorder.mnDiagColorId;
}

namespace {

struct XclExpFillPred
{
    const XclExpCellArea&
                        mrFill;
    explicit     XclExpFillPred( const XclExpCellArea& rFill ) : mrFill( rFill ) {}
    bool                operator()( const XclExpCellArea& rFill ) const;
};

}

bool XclExpFillPred::operator()( const XclExpCellArea& rFill ) const
{
    return
        mrFill.mnForeColor      == rFill.mnForeColor &&
        mrFill.mnBackColor      == rFill.mnBackColor &&
        mrFill.mnPattern        == rFill.mnPattern &&
        mrFill.mnForeColorId    == rFill.mnForeColorId &&
        mrFill.mnBackColorId    == rFill.mnBackColorId;
}

XclExpXFBuffer::XclExpXFBuffer( const XclExpRoot& rRoot ) :
    XclExpRoot( rRoot )
{
}

void XclExpXFBuffer::Initialize()
{
    InsertDefaultRecords();
    InsertUserStyles();
}

sal_uInt32 XclExpXFBuffer::Insert( const ScPatternAttr* pPattern, sal_Int16 nScript )
{
    return InsertCellXF( pPattern, nScript, NUMBERFORMAT_ENTRY_NOT_FOUND, EXC_FONT_NOTFOUND, false );
}

sal_uInt32 XclExpXFBuffer::InsertWithFont( const ScPatternAttr* pPattern, sal_Int16 nScript,
        sal_uInt16 nForceXclFont, bool bForceLineBreak )
{
    return InsertCellXF( pPattern, nScript, NUMBERFORMAT_ENTRY_NOT_FOUND, nForceXclFont, bForceLineBreak );
}

sal_uInt32 XclExpXFBuffer::InsertWithNumFmt( const ScPatternAttr* pPattern, sal_Int16 nScript, sal_uInt32 nForceScNumFmt, bool bForceLineBreak )
{
    return InsertCellXF( pPattern, nScript, nForceScNumFmt, EXC_FONT_NOTFOUND, bForceLineBreak );
}

sal_uInt32 XclExpXFBuffer::InsertStyle( const SfxStyleSheetBase* pStyleSheet )
{
    return pStyleSheet ? InsertStyleXF( *pStyleSheet ) : GetXFIdFromIndex( EXC_XF_DEFAULTSTYLE );
}

sal_uInt32 XclExpXFBuffer::GetXFIdFromIndex( sal_uInt16 nXFIndex )
{
    return EXC_XFLIST_INDEXBASE | nXFIndex;
}

sal_uInt32 XclExpXFBuffer::GetDefCellXFId()
{
    return GetXFIdFromIndex( EXC_XF_DEFAULTCELL );
}

const XclExpXF* XclExpXFBuffer::GetXFById( sal_uInt32 nXFId ) const
{
    return maXFList.GetRecord( nXFId );
}

void XclExpXFBuffer::Finalize()
{
    for( size_t nPos = 0, nSize = maXFList.GetSize(); nPos < nSize; ++nPos )
        maXFList.GetRecord( nPos )->SetFinalColors();

    sal_uInt32 nTotalCount = static_cast< sal_uInt32 >( maXFList.GetSize() );
    sal_uInt32 nId;
    maXFIndexVec.resize( nTotalCount, EXC_XF_DEFAULTCELL );
    maStyleIndexes.resize( nTotalCount, EXC_XF_DEFAULTCELL );
    maCellIndexes.resize( nTotalCount, EXC_XF_DEFAULTCELL );

    XclExpBuiltInMap::const_iterator aBuiltInEnd = maBuiltInMap.end();
    /*  nMaxBuiltInXFId used to decide faster whether an XF record is
        user-defined. If the current XF ID is greater than this value,
        maBuiltInMap doesn't need to be searched. */
    sal_uInt32 nMaxBuiltInXFId = maBuiltInMap.empty() ? 0 : maBuiltInMap.rbegin()->first;

    // *** map all built-in XF records (cell and style) *** -------------------

    // do not change XF order -> std::map<> iterates elements in ascending order
    for( const auto& rEntry : maBuiltInMap )
        AppendXFIndex( rEntry.first );

    // *** insert all user-defined style XF records, without reduce *** -------

    sal_uInt32 nStyleXFCount = 0;       // counts up to EXC_XF_MAXSTYLECOUNT limit

    for( nId = 0; nId < nTotalCount; ++nId )
    {
        XclExpXFRef xXF = maXFList.GetRecord( nId );
        if( xXF->IsStyleXF() && ((nId > nMaxBuiltInXFId) || (maBuiltInMap.find( nId ) == aBuiltInEnd)) )
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

    // *** insert all cell XF records *** -------------------------------------

    // start position to search for equal inserted XF records
    size_t nSearchStart = maSortedXFList.GetSize();

    // break the loop if XF limit reached - maXFIndexVec is already initialized with default index
    XclExpXFRef xDefCellXF = maXFList.GetRecord( EXC_XF_DEFAULTCELL );
    for( nId = 0; (nId < nTotalCount) && (maSortedXFList.GetSize() < EXC_XF_MAXCOUNT); ++nId )
    {
        XclExpXFRef xXF = maXFList.GetRecord( nId );
        if( xXF->IsCellXF() && ((nId > nMaxBuiltInXFId) || (maBuiltInMap.find( nId ) == aBuiltInEnd)) )
        {
            // try to find an XF record equal to *xXF, which is already inserted
            sal_uInt16 nFoundIndex = EXC_XF_NOTFOUND;

            // first try if it is equal to the default cell XF
            if( xDefCellXF->Equals( *xXF ) )
            {
                nFoundIndex = EXC_XF_DEFAULTCELL;
            }
            else for( size_t nSearchPos = nSearchStart, nSearchEnd = maSortedXFList.GetSize();
                        (nSearchPos < nSearchEnd) && (nFoundIndex == EXC_XF_NOTFOUND); ++nSearchPos )
            {
                if( maSortedXFList.GetRecord( nSearchPos )->Equals( *xXF ) )
                    nFoundIndex = static_cast< sal_uInt16 >( nSearchPos );
            }

            if( nFoundIndex != EXC_XF_NOTFOUND )
                // equal XF already in the list, use its resulting XF index
                maXFIndexVec[ nId ] = nFoundIndex;
            else
                AppendXFIndex( nId );
        }
    }

    sal_uInt16 nXmlStyleIndex   = 0;
    sal_uInt16 nXmlCellIndex    = 0;

    size_t nXFCount = maSortedXFList.GetSize();
    for( size_t i = 0; i < nXFCount; ++i )
    {
        XclExpXFList::RecordRefType xXF = maSortedXFList.GetRecord( i );
        if( xXF->IsStyleXF() )
            maStyleIndexes[ i ] = nXmlStyleIndex++;
        else
            maCellIndexes[ i ] = nXmlCellIndex++;
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

sal_Int32 XclExpXFBuffer::GetXmlStyleIndex( sal_uInt32 nXFIndex ) const
{
    OSL_ENSURE( nXFIndex < maStyleIndexes.size(), "XclExpXFBuffer::GetXmlStyleIndex - invalid index!" );
    if( nXFIndex >= maStyleIndexes.size() )
        return 0;   // should be caught/debugged via above assert; return "valid" index.
    return maStyleIndexes[ nXFIndex ];
}

sal_Int32 XclExpXFBuffer::GetXmlCellIndex( sal_uInt32 nXFIndex ) const
{
    OSL_ENSURE( nXFIndex < maCellIndexes.size(), "XclExpXFBuffer::GetXmlStyleIndex - invalid index!" );
    if( nXFIndex >= maCellIndexes.size() )
        return 0;   // should be caught/debugged via above assert; return "valid" index.
    return maCellIndexes[ nXFIndex ];
}

void XclExpXFBuffer::Save( XclExpStream& rStrm )
{
    // save all XF records contained in the maSortedXFList vector (sorted by XF index)
    maSortedXFList.Save( rStrm );
    // save all STYLE records
    maStyleList.Save( rStrm );
}

static void lcl_GetCellCounts( const XclExpRecordList< XclExpXF >& rXFList, sal_Int32& rCells, sal_Int32& rStyles )
{
    rCells  = 0;
    rStyles = 0;
    size_t nXFCount = rXFList.GetSize();
    for( size_t i = 0; i < nXFCount; ++i )
    {
        XclExpRecordList< XclExpXF >::RecordRefType xXF = rXFList.GetRecord( i );
        if( xXF->IsCellXF() )
            ++rCells;
        else if( xXF->IsStyleXF() )
            ++rStyles;
    }
}

void XclExpXFBuffer::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();

    rStyleSheet->startElement(XML_fills, XML_count, OString::number(maFills.size()));
    for( const auto& rFill : maFills )
    {
        rFill.SaveXml( rStrm );
    }
    rStyleSheet->endElement( XML_fills );

    rStyleSheet->startElement(XML_borders, XML_count, OString::number(maBorders.size()));
    for( const auto& rBorder : maBorders )
    {
        rBorder.SaveXml( rStrm );
    }
    rStyleSheet->endElement( XML_borders );

    // save all XF records contained in the maSortedXFList vector (sorted by XF index)
    sal_Int32 nCells, nStyles;
    lcl_GetCellCounts( maSortedXFList, nCells, nStyles );

    if( nStyles > 0 )
    {
        rStyleSheet->startElement(XML_cellStyleXfs, XML_count, OString::number(nStyles));
        size_t nXFCount = maSortedXFList.GetSize();
        for( size_t i = 0; i < nXFCount; ++i )
        {
            XclExpXFList::RecordRefType xXF = maSortedXFList.GetRecord( i );
            if( ! xXF->IsStyleXF() )
                continue;
            SaveXFXml( rStrm, *xXF );
        }
        rStyleSheet->endElement( XML_cellStyleXfs );
    }

    if( nCells > 0 )
    {
        rStyleSheet->startElement(XML_cellXfs, XML_count, OString::number(nCells));
        size_t nXFCount = maSortedXFList.GetSize();
        for( size_t i = 0; i < nXFCount; ++i )
        {
            XclExpXFList::RecordRefType xXF = maSortedXFList.GetRecord( i );
            if( ! xXF->IsCellXF() )
                continue;
            SaveXFXml( rStrm, *xXF );
        }
        rStyleSheet->endElement( XML_cellXfs );
    }

    // save all STYLE records
    rStyleSheet->startElement(XML_cellStyles, XML_count, OString::number(maStyleList.GetSize()));
    maStyleList.SaveXml( rStrm );
    rStyleSheet->endElement( XML_cellStyles );
}

void XclExpXFBuffer::SaveXFXml( XclExpXmlStream& rStrm, XclExpXF& rXF )
{
    XclExpBorderList::iterator aBorderPos =
        std::find_if( maBorders.begin(), maBorders.end(), XclExpBorderPred( rXF.GetBorderData() ) );
    OSL_ENSURE( aBorderPos != maBorders.end(), "XclExpXFBuffer::SaveXml - Invalid @borderId!" );
    XclExpFillList::iterator aFillPos =
        std::find_if( maFills.begin(), maFills.end(), XclExpFillPred( rXF.GetAreaData() ) );
    OSL_ENSURE( aFillPos != maFills.end(), "XclExpXFBuffer::SaveXml - Invalid @fillId!" );

    sal_Int32 nBorderId = 0, nFillId = 0;
    if( aBorderPos != maBorders.end() )
        nBorderId = std::distance( maBorders.begin(), aBorderPos );
    if( aFillPos != maFills.end() )
        nFillId = std::distance( maFills.begin(), aFillPos );

    rXF.SetXmlIds( nBorderId, nFillId );
    rXF.SaveXml( rStrm );
}

sal_uInt32 XclExpXFBuffer::FindXF( const ScPatternAttr& rPattern,
        sal_uInt32 nForceScNumFmt, sal_uInt16 nForceXclFont, bool bForceLineBreak ) const
{
    if (nForceScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND && nForceXclFont == EXC_FONT_NOTFOUND)
    {
        FindKey key1 { /*mbCellXF*/true, &rPattern.GetItemSet(), nForceScNumFmt, 0 };
        FindKey key2 { /*mbCellXF*/true, &rPattern.GetItemSet(), nForceScNumFmt, EXC_FONT_NOTFOUND };
        auto it1 = maXFFindMap.lower_bound(key1);
        if (it1 != maXFFindMap.end())
        {
            auto it2 = maXFFindMap.upper_bound(key2);
            for (auto it = it1; it != it2; ++it)
                for (auto const & nPos : it->second)
                    if( maXFList.GetRecord( nPos )->Equals( rPattern, nForceScNumFmt, nForceXclFont, bForceLineBreak ) )
                        return nPos;
        }
    }
    else if (nForceScNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND || nForceXclFont == EXC_FONT_NOTFOUND)
    {
        FindKey key1 { /*mbCellXF*/true, &rPattern.GetItemSet(), 0, 0 };
        FindKey key2 { /*mbCellXF*/true, &rPattern.GetItemSet(), NUMBERFORMAT_ENTRY_NOT_FOUND, EXC_FONT_NOTFOUND };
        auto it1 = maXFFindMap.lower_bound(key1);
        if (it1 != maXFFindMap.end())
        {
            auto it2 = maXFFindMap.upper_bound(key2);
            for (auto it = it1; it != it2; ++it)
                for (auto const & nPos : it->second)
                    if( maXFList.GetRecord( nPos )->Equals( rPattern, nForceScNumFmt, nForceXclFont, bForceLineBreak ) )
                        return nPos;
        }
    }
    else
    {
        FindKey key { /*mbCellXF*/true, &rPattern.GetItemSet(), nForceScNumFmt, nForceXclFont };
        auto it = maXFFindMap.find(key);
        if (it == maXFFindMap.end())
            return EXC_XFID_NOTFOUND;
        for (auto const & nPos : it->second)
            if( maXFList.GetRecord( nPos )->Equals( rPattern, nForceScNumFmt, nForceXclFont, bForceLineBreak ) )
                return nPos;
    }
    return EXC_XFID_NOTFOUND;
}

sal_uInt32 XclExpXFBuffer::FindXF( const SfxStyleSheetBase& rStyleSheet ) const
{
    const SfxItemSet* pItemSet = &const_cast< SfxStyleSheetBase& >( rStyleSheet ).GetItemSet();
    FindKey key1 { /*mbCellXF*/false, pItemSet, 0, 0 };
    FindKey key2 { /*mbCellXF*/false, pItemSet, NUMBERFORMAT_ENTRY_NOT_FOUND, EXC_FONT_NOTFOUND };
    auto it1 = maXFFindMap.lower_bound(key1);
    auto it2 = maXFFindMap.upper_bound(key2);
    for (auto it = it1; it != it2; ++it)
        for (auto const & nPos : it->second)
            if( maXFList.GetRecord( nPos )->Equals( rStyleSheet ) )
                return nPos;
    return EXC_XFID_NOTFOUND;
}

sal_uInt32 XclExpXFBuffer::FindBuiltInXF( sal_uInt8 nStyleId, sal_uInt8 nLevel ) const
{
    auto aIt = std::find_if(maBuiltInMap.begin(), maBuiltInMap.end(),
        [&nStyleId, nLevel](const XclExpBuiltInMap::value_type& rEntry) {
            return (rEntry.second.mnStyleId == nStyleId) && (rEntry.second.mnLevel == nLevel);
        });
    if (aIt != maBuiltInMap.end())
        return aIt->first;
    return EXC_XFID_NOTFOUND;
}

XclExpXFBuffer::FindKey XclExpXFBuffer::ToFindKey(XclExpXF const & rRec)
{
    return { rRec.IsCellXF(), rRec.GetItemSet(), rRec.GetScNumFmt(), rRec.GetXclFont() };
}

sal_uInt32 XclExpXFBuffer::InsertCellXF( const ScPatternAttr* pPattern, sal_Int16 nScript,
        sal_uInt32 nForceScNumFmt, sal_uInt16 nForceXclFont, bool bForceLineBreak )
{
    if( !pPattern )
        pPattern = &GetDoc().getCellAttributeHelper().getDefaultCellAttribute();

    // special handling for default cell formatting
    if ( pPattern->isDefault() && !bForceLineBreak &&
        (nForceScNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND) &&
        (nForceXclFont == EXC_FONT_NOTFOUND) )
    {
        // Is it the first try to insert the default cell format?
        bool& rbPredefined = maBuiltInMap[ EXC_XF_DEFAULTCELL ].mbPredefined;
        if( rbPredefined )
        {
            // remove old entry in find-map
            auto & rPositions = maXFFindMap[ToFindKey(*maXFList.GetRecord(EXC_XF_DEFAULTCELL))];
            auto it = std::find(rPositions.begin(), rPositions.end(), EXC_XF_DEFAULTCELL);
            rPositions.erase(it);
            // replace default cell pattern
            XclExpXFRef xNewXF = new XclExpXF( GetRoot(), *pPattern, nScript );
            maXFList.ReplaceRecord( xNewXF, EXC_XF_DEFAULTCELL );
            // and add new entry in find-map
            maXFFindMap[ToFindKey(*xNewXF)].push_back(EXC_XF_DEFAULTCELL);
            rbPredefined = false;
        }
        return GetDefCellXFId();
    }

    sal_uInt32 nXFId = FindXF( *pPattern, nForceScNumFmt, nForceXclFont, bForceLineBreak );
    if( nXFId == EXC_XFID_NOTFOUND )
    {
        // not found - insert new cell XF
        if( maXFList.GetSize() < EXC_XFLIST_HARDLIMIT )
        {
            auto pNewExp = new XclExpXF(
                GetRoot(), *pPattern, nScript, nForceScNumFmt, nForceXclFont, bForceLineBreak );
            maXFList.AppendNewRecord( pNewExp );
            // do not set nXFId before the AppendNewRecord() call - it may insert 2 XFs (style+cell)
            nXFId = static_cast< sal_uInt32 >( maXFList.GetSize() - 1 );
            maXFFindMap[ToFindKey(*pNewExp)].push_back(nXFId);
        }
        else
        {
            // list full - fall back to default cell XF
            nXFId = GetDefCellXFId();
        }
    }
    return nXFId;
}

sal_uInt32 XclExpXFBuffer::InsertStyleXF( const SfxStyleSheetBase& rStyleSheet )
{
    // *** try, if it is a built-in style - create new XF or replace existing predefined XF ***

    sal_uInt8 nStyleId, nLevel;
    if( XclTools::GetBuiltInStyleId( nStyleId, nLevel, rStyleSheet.GetName() ) )
    {
        // try to find the built-in XF record (if already created in InsertDefaultRecords())
        sal_uInt32 nXFId = FindBuiltInXF( nStyleId, nLevel );
        if( nXFId == EXC_XFID_NOTFOUND )
        {
            // built-in style XF not yet created - do it now
            XclExpXFRef xXF = new XclExpXF( GetRoot(), rStyleSheet );
            nXFId = AppendBuiltInXFWithStyle( xXF, nStyleId, nLevel );
            // this new XF record is not predefined
            maBuiltInMap[ nXFId ].mbPredefined = false;
        }
        else
        {
            OSL_ENSURE( maXFList.HasRecord( nXFId ), "XclExpXFBuffer::InsertStyleXF - built-in XF not found" );
            // XF record still predefined? -> Replace with real XF
            bool& rbPredefined = maBuiltInMap[ nXFId ].mbPredefined;
            if( rbPredefined )
            {
                // remove old entry in find-map
                auto & rPositions = maXFFindMap[ToFindKey(*maXFList.GetRecord(nXFId))];
                auto it = std::find(rPositions.begin(), rPositions.end(), nXFId);
                rPositions.erase(it);
                // replace predefined built-in style (ReplaceRecord() deletes old record)
                XclExpXFRef pNewExp = new XclExpXF( GetRoot(), rStyleSheet );
                maXFList.ReplaceRecord( pNewExp, nXFId );
                // and add new entry in find-map
                maXFFindMap[ToFindKey(*pNewExp)].push_back(nXFId);
                rbPredefined = false;
            }
        }

        // STYLE already inserted? (may be not, i.e. for RowLevel/ColLevel or Hyperlink styles)
        bool& rbHasStyleRec = maBuiltInMap[ nXFId ].mbHasStyleRec;
        if( !rbHasStyleRec )
        {
            maStyleList.AppendNewRecord( new XclExpStyle( nXFId, nStyleId, nLevel ) );
            rbHasStyleRec = true;
        }

        return nXFId;
    }

    // *** try to find the XF record of a user-defined style ***

    sal_uInt32 nXFId = FindXF( rStyleSheet );
    if( nXFId == EXC_XFID_NOTFOUND )
    {
        // not found - insert new style XF and STYLE
        nXFId = static_cast< sal_uInt32 >( maXFList.GetSize() );
        if( nXFId < EXC_XFLIST_HARDLIMIT )
        {
            auto pNewExp = new XclExpXF( GetRoot(), rStyleSheet );
            maXFList.AppendNewRecord( pNewExp );
            // create the STYLE record
            if( !rStyleSheet.GetName().isEmpty() )
                maStyleList.AppendNewRecord( new XclExpStyle( nXFId, rStyleSheet.GetName() ) );
            maXFFindMap[ToFindKey(*pNewExp)].push_back(nXFId);
        }
        else
            // list full - fall back to default style XF
            nXFId = GetXFIdFromIndex( EXC_XF_DEFAULTSTYLE );
    }
    return nXFId;
}

void XclExpXFBuffer::InsertUserStyles()
{
    SfxStyleSheetIterator aStyleIter( GetDoc().GetStyleSheetPool(), SfxStyleFamily::Para );
    for( SfxStyleSheetBase* pStyleSheet = aStyleIter.First(); pStyleSheet; pStyleSheet = aStyleIter.Next() )
        if( pStyleSheet->IsUserDefined() && !lclIsBuiltInStyle( pStyleSheet->GetName() ) )
            InsertStyleXF( *pStyleSheet );
}

sal_uInt32 XclExpXFBuffer::AppendBuiltInXF( XclExpXFRef const & xXF, sal_uInt8 nStyleId, sal_uInt8 nLevel )
{
    sal_uInt32 nXFId = static_cast< sal_uInt32 >( maXFList.GetSize() );
    maXFList.AppendRecord( xXF );
    maXFFindMap[ToFindKey(*xXF)].push_back(nXFId);
    XclExpBuiltInInfo& rInfo = maBuiltInMap[ nXFId ];
    rInfo.mnStyleId = nStyleId;
    rInfo.mnLevel = nLevel;
    rInfo.mbPredefined = true;
    return nXFId;
}

sal_uInt32 XclExpXFBuffer::AppendBuiltInXFWithStyle( XclExpXFRef const & xXF, sal_uInt8 nStyleId, sal_uInt8 nLevel )
{
    sal_uInt32 nXFId = AppendBuiltInXF( xXF, nStyleId, nLevel );
    maStyleList.AppendNewRecord( new XclExpStyle( nXFId, nStyleId, nLevel ) );
    maBuiltInMap[ nXFId ].mbHasStyleRec = true;  // mark existing STYLE record
    return nXFId;
}

static XclExpCellArea lcl_GetPatternFill_None()
{
    XclExpCellArea aFill;
    aFill.mnPattern = EXC_PATT_NONE;
    return aFill;
}

static XclExpCellArea lcl_GetPatternFill_Gray125()
{
    XclExpCellArea aFill;
    aFill.mnPattern     = EXC_PATT_12_5_PERC;
    aFill.mnForeColor   = 0;
    aFill.mnBackColor   = 0;
    return aFill;
}

void XclExpXFBuffer::InsertDefaultRecords()
{
    maFills.push_back( lcl_GetPatternFill_None() );
    maFills.push_back( lcl_GetPatternFill_Gray125() );

    // index 0: default style
    if( SfxStyleSheetBase* pDefStyleSheet = GetStyleSheetPool().Find( ScResId( STR_STYLENAME_STANDARD ), SfxStyleFamily::Para ) )
    {
        XclExpXFRef xDefStyle = new XclExpXF( GetRoot(), *pDefStyleSheet );
        sal_uInt32 nXFId = AppendBuiltInXFWithStyle( xDefStyle, EXC_STYLE_NORMAL );
        // mark this XF as not predefined, prevents overwriting
        maBuiltInMap[ nXFId ].mbPredefined = false;
    }
    else
    {
        OSL_FAIL( "XclExpXFBuffer::InsertDefaultRecords - default style not found" );
        XclExpXFRef xDefStyle = new XclExpDefaultXF( GetRoot(), false );
        xDefStyle->SetAllUsedFlags( true );
        AppendBuiltInXFWithStyle( xDefStyle, EXC_STYLE_NORMAL );
    }

    // index 1-14: RowLevel and ColLevel styles (without STYLE records)
    XclExpDefaultXF aLevelStyle( GetRoot(), false );
    // RowLevel_1, ColLevel_1
    aLevelStyle.SetFont( 1 );
    AppendBuiltInXF( new XclExpDefaultXF( aLevelStyle ), EXC_STYLE_ROWLEVEL, 0 );
    AppendBuiltInXF( new XclExpDefaultXF( aLevelStyle ), EXC_STYLE_COLLEVEL, 0 );
    // RowLevel_2, ColLevel_2
    aLevelStyle.SetFont( 2 );
    AppendBuiltInXF( new XclExpDefaultXF( aLevelStyle ), EXC_STYLE_ROWLEVEL, 1 );
    AppendBuiltInXF( new XclExpDefaultXF( aLevelStyle ), EXC_STYLE_COLLEVEL, 1 );
    // RowLevel_3, ColLevel_3 ... RowLevel_7, ColLevel_7
    aLevelStyle.SetFont( 0 );
    for( sal_uInt8 nLevel = 2; nLevel < EXC_STYLE_LEVELCOUNT; ++nLevel )
    {
        AppendBuiltInXF( new XclExpDefaultXF( aLevelStyle ), EXC_STYLE_ROWLEVEL, nLevel );
        AppendBuiltInXF( new XclExpDefaultXF( aLevelStyle ), EXC_STYLE_COLLEVEL, nLevel );
    }

    // index 15: default hard cell format, placeholder to be able to add more built-in styles
    maXFList.AppendNewRecord( new XclExpDefaultXF( GetRoot(), true ) );
    maXFFindMap[ToFindKey(*maXFList.GetRecord(maXFList.GetSize()-1))].push_back(maXFList.GetSize()-1);
    maBuiltInMap[ EXC_XF_DEFAULTCELL ].mbPredefined = true;

    // index 16-20: other built-in styles
    {
        XclExpDefaultXF aFormatStyle( GetRoot(), false );
        aFormatStyle.SetFont( 1 );
        aFormatStyle.SetNumFmt( 43 );
        AppendBuiltInXFWithStyle( new XclExpDefaultXF( aFormatStyle ), EXC_STYLE_COMMA );
        aFormatStyle.SetNumFmt( 41 );
        AppendBuiltInXFWithStyle( new XclExpDefaultXF( aFormatStyle ), EXC_STYLE_COMMA_0 );
        aFormatStyle.SetNumFmt( 44 );
        AppendBuiltInXFWithStyle( new XclExpDefaultXF( aFormatStyle ), EXC_STYLE_CURRENCY );
        aFormatStyle.SetNumFmt( 42 );
        AppendBuiltInXFWithStyle( new XclExpDefaultXF( aFormatStyle ), EXC_STYLE_CURRENCY_0 );
        aFormatStyle.SetNumFmt( 9 );
        AppendBuiltInXFWithStyle( new XclExpDefaultXF( std::move(aFormatStyle) ), EXC_STYLE_PERCENT );
    }

    // other built-in style XF records (i.e. Hyperlink styles) are created on demand

    /*  Insert the real default hard cell format -> 0 is document default pattern.
        Do it here (and not already above) to really have all built-in styles. */
    Insert( nullptr, GetDefApiScript() );
}

void XclExpXFBuffer::AppendXFIndex( sal_uInt32 nXFId )
{
    OSL_ENSURE( nXFId < maXFIndexVec.size(), "XclExpXFBuffer::AppendXFIndex - XF ID out of range" );
    maXFIndexVec[ nXFId ] = static_cast< sal_uInt16 >( maSortedXFList.GetSize() );
    XclExpXFRef xXF = maXFList.GetRecord( nXFId );
    AddBorderAndFill( *xXF );
    maSortedXFList.AppendRecord( xXF );
    OSL_ENSURE( maXFList.HasRecord( nXFId ), "XclExpXFBuffer::AppendXFIndex - XF not found" );
}

void XclExpXFBuffer::AddBorderAndFill( const XclExpXF& rXF )
{
    if( std::none_of( maBorders.begin(), maBorders.end(), XclExpBorderPred( rXF.GetBorderData() ) ) )
    {
        maBorders.push_back( rXF.GetBorderData() );
    }

    if( std::none_of( maFills.begin(), maFills.end(), XclExpFillPred( rXF.GetAreaData() ) ) )
    {
        maFills.push_back( rXF.GetAreaData() );
    }
}

XclExpDxfs::XclExpDxfs( const XclExpRoot& rRoot )
    : XclExpRoot( rRoot ),
    mpKeywordTable( new NfKeywordTable )
{
    sal_Int32 nDxfId = 0;
    // Special number formatter for conversion.
    SvNumberFormatterPtr xFormatter(new SvNumberFormatter( comphelper::getProcessComponentContext(), LANGUAGE_ENGLISH_US ));
    xFormatter->FillKeywordTableForExcel( *mpKeywordTable );

    SCTAB nTables = rRoot.GetDoc().GetTableCount();
    for(SCTAB nTab = 0; nTab < nTables; ++nTab)
    {
        // Color filters
        std::vector<ScDBData*> pDBData = rRoot.GetDoc().GetDBCollection()->GetAllDBsFromTab(nTab);
        for (auto& pData : pDBData)
        {
            ScRange aRange;
            pData->GetArea(aRange);
            for (auto nCol = aRange.aStart.Col(); nCol <= aRange.aEnd.Col(); nCol++)
            {
                ScFilterEntries aFilterEntries;
                rRoot.GetDoc().GetFilterEntriesArea(nCol, aRange.aStart.Row(),
                                                    aRange.aEnd.Row(), nTab, true, aFilterEntries);

                // Excel has all filter values stored as foreground colors
                // Does not matter it is text color or cell background color
                for (auto& rColor : aFilterEntries.getBackgroundColors())
                {
                    if (!maColorToDxfId.emplace(rColor, nDxfId).second)
                        continue;

                    std::unique_ptr<XclExpCellArea> pExpCellArea(new XclExpCellArea(rColor, 0));
                    maDxf.push_back(std::make_unique<XclExpDxf>(rRoot, std::move(pExpCellArea)));
                    nDxfId++;
                }
                for (auto& rColor : aFilterEntries.getTextColors())
                {
                    if (!maColorToDxfId.emplace(rColor, nDxfId).second)
                        continue;

                    std::unique_ptr<XclExpCellArea> pExpCellArea(new XclExpCellArea(rColor, 0));
                    maDxf.push_back(std::make_unique<XclExpDxf>(rRoot, std::move(pExpCellArea)));
                    nDxfId++;
                }
            }
        }

        // Conditional formatting
        ScConditionalFormatList* pList = rRoot.GetDoc().GetCondFormList(nTab);
        if (pList)
        {
            for (const auto& rxItem : *pList)
            {
                size_t nEntryCount = rxItem->size();
                for (size_t nFormatEntry = 0; nFormatEntry < nEntryCount; ++nFormatEntry)
                {
                    const ScFormatEntry* pFormatEntry = rxItem->GetEntry(nFormatEntry);
                    if (!pFormatEntry
                        || (pFormatEntry->GetType() != ScFormatEntry::Type::Condition
                            && pFormatEntry->GetType() != ScFormatEntry::Type::Date
                            && pFormatEntry->GetType() != ScFormatEntry::Type::ExtCondition))
                        continue;

                    OUString aStyleName;
                    if (pFormatEntry->GetType() == ScFormatEntry::Type::Condition
                        || pFormatEntry->GetType() == ScFormatEntry::Type::ExtCondition)
                    {
                        const ScCondFormatEntry* pEntry = static_cast<const ScCondFormatEntry*>(pFormatEntry);
                        aStyleName= pEntry->GetStyle();
                    }
                    else
                    {
                        const ScCondDateFormatEntry* pEntry = static_cast<const ScCondDateFormatEntry*>(pFormatEntry);
                        aStyleName = pEntry->GetStyleName();
                    }

                    if (maStyleNameToDxfId.emplace(aStyleName, nDxfId).second)
                    {
                        SfxStyleSheetBase* pStyle = rRoot.GetDoc().GetStyleSheetPool()->Find(aStyleName, SfxStyleFamily::Para);
                        if(!pStyle)
                            continue;

                        SfxItemSet& rSet = pStyle->GetItemSet();
                        fillDxfFrom(rSet, xFormatter);
                        nDxfId++;
                    }

                }
            }
        }
    }

    ScDPCollection* pCollection = rRoot.GetDoc().GetDPCollection();
    for (size_t nIndex = 0; nIndex < pCollection->GetCount(); nIndex++)
    {
        const ScDPObject& rObject = (*pCollection)[nIndex];
        ScDPSaveData* pSaveData = rObject.GetSaveData();
        if (pSaveData && pSaveData->hasFormats())
        {
            sc::PivotTableFormats const& rFormats = pSaveData->getFormats();
            for (sc::PivotTableFormat const& rFormat : rFormats.getVector())
            {
                if (!rFormat.pPattern)
                    continue;

                SfxItemSet& rItemSet = rFormat.pPattern->GetItemSet();
                fillDxfFrom(rItemSet, xFormatter);
                maPatternToDxfId.emplace(rFormat.pPattern.get(), nDxfId);
                nDxfId++;
            }
        }
    }
}

void XclExpDxfs::fillDxfFrom(const SfxItemSet& rItemSet, const SvNumberFormatterPtr& xFormatter)
{
    std::unique_ptr<XclExpCellBorder> pBorder(new XclExpCellBorder);
    if (!pBorder->FillFromItemSet(rItemSet, GetPalette(), GetBiff()))
        pBorder.reset();

    std::unique_ptr<XclExpCellAlign> pAlign(new XclExpCellAlign);
    if (!pAlign->FillFromItemSet(GetRoot(), rItemSet, false, GetBiff()))
        pAlign.reset();

    std::unique_ptr<XclExpCellProt> pCellProtection(new XclExpCellProt);
    if (!pCellProtection->FillFromItemSet(rItemSet))
        pCellProtection.reset();

    std::unique_ptr<XclExpColor> pColor(new XclExpColor);
    if (!pColor->FillFromItemSet(rItemSet))
        pColor.reset();

    std::unique_ptr<XclExpDxfFont> pFont(new XclExpDxfFont(GetRoot(), rItemSet));

    std::unique_ptr<XclExpNumFmt> pNumberFormat;
    if (const SfxUInt32Item* pPoolItem = rItemSet.GetItemIfSet(ATTR_VALUE_FORMAT))
    {
        sal_uInt32 nScNumberFormat = pPoolItem->GetValue();
        sal_Int32 nXclNumberFormat = GetRoot().GetNumFmtBuffer().Insert(nScNumberFormat);
        pNumberFormat.reset(new XclExpNumFmt(nScNumberFormat, nXclNumberFormat, GetNumberFormatCode(*this, nScNumberFormat, xFormatter.get(), mpKeywordTable.get())));
    }

    maDxf.push_back(std::make_unique<XclExpDxf>(GetRoot(), std::move(pAlign), std::move(pBorder),
                   std::move(pFont), std::move(pNumberFormat), std::move(pCellProtection), std::move(pColor)));
}

sal_Int32 XclExpDxfs::GetDxfId( const OUString& rStyleName ) const
{
    std::map<OUString, sal_Int32>::const_iterator itr = maStyleNameToDxfId.find(rStyleName);
    if(itr!= maStyleNameToDxfId.end())
        return itr->second;
    return -1;
}

sal_Int32 XclExpDxfs::GetDxfByColor(Color aColor) const
{
    std::map<Color, sal_Int32>::const_iterator itr = maColorToDxfId.find(aColor);
    if (itr != maColorToDxfId.end())
        return itr->second;
    return -1;
}

sal_Int32 XclExpDxfs::GetDxfIdForPattern(ScPatternAttr* pPattern) const
{
    auto iterator = maPatternToDxfId.find(pPattern);
    if (iterator != maPatternToDxfId.end())
        return iterator->second;
    return -1;
}

void XclExpDxfs::addColor(Color aColor)
{
    maColorToDxfId.emplace(aColor, maDxf.size());

    std::unique_ptr<XclExpCellArea> pExpCellArea(new XclExpCellArea(aColor, 0));
    maDxf.push_back(std::make_unique<XclExpDxf>(GetRoot(), std::move(pExpCellArea)));
}

void XclExpDxfs::SaveXml( XclExpXmlStream& rStrm )
{
    if(maDxf.empty())
        return;

    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->startElement(XML_dxfs, XML_count, OString::number(maDxf.size()));

    for ( auto& rxDxf : maDxf )
    {
        rxDxf->SaveXml( rStrm );
    }

    rStyleSheet->endElement( XML_dxfs );
}

void XclExpDxfs::Finalize()
{
    for (auto& rxDxf : maDxf)
    {
        rxDxf->SetFinalColors();
    }
}

XclExpDxf::XclExpDxf( const XclExpRoot& rRoot, std::unique_ptr<XclExpCellAlign> pAlign, std::unique_ptr<XclExpCellBorder> pBorder,
            std::unique_ptr<XclExpDxfFont> pFont, std::unique_ptr<XclExpNumFmt> pNumberFmt, std::unique_ptr<XclExpCellProt> pProt,
            std::unique_ptr<XclExpColor> pColor)
    : XclExpRoot( rRoot ),
    mpAlign(std::move(pAlign)),
    mpBorder(std::move(pBorder)),
    mpFont(std::move(pFont)),
    mpNumberFmt(std::move(pNumberFmt)),
    mpProt(std::move(pProt)),
    mpColor(std::move(pColor))
{
}

XclExpDxf::XclExpDxf(const XclExpRoot& rRoot, std::unique_ptr<XclExpCellArea> pCellArea)
    : XclExpRoot(rRoot)
    , mpCellArea(std::move(pCellArea))
{
}

XclExpDxf::~XclExpDxf()
{
}

void XclExpDxf::SetFinalColors()
{
    if (mpBorder)
    {
        mpBorder->SetFinalColors(GetPalette());
    }
}

void XclExpDxf::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->startElement(XML_dxf);

    if (mpFont)
        mpFont->SaveXml(rStrm);
    if (mpNumberFmt)
        mpNumberFmt->SaveXml(rStrm);
    if (mpColor)
        mpColor->SaveXml(rStrm);
    if (mpAlign)
        mpAlign->SaveXml(rStrm);
    if (mpBorder)
        mpBorder->SaveXml(rStrm);
    if (mpProt)
        mpProt->SaveXml(rStrm);
    if (mpCellArea)
        mpCellArea->SaveXml(rStrm);
    rStyleSheet->endElement( XML_dxf );
}

void XclExpDxf::SaveXmlExt( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr& rStyleSheet = rStrm.GetCurrentStream();
    rStyleSheet->startElementNS( XML_x14, XML_dxf );

    if (mpFont)
        mpFont->SaveXml(rStrm);
    if (mpNumberFmt)
        mpNumberFmt->SaveXml(rStrm);
    if (mpColor)
        mpColor->SaveXml(rStrm);
    if (mpAlign)
        mpAlign->SaveXml(rStrm);
    if (mpBorder)
        mpBorder->SaveXml(rStrm);
    if (mpProt)
        mpProt->SaveXml(rStrm);
    rStyleSheet->endElementNS( XML_x14, XML_dxf );
}


XclExpXmlStyleSheet::XclExpXmlStyleSheet( const XclExpRoot& rRoot )
    : XclExpRoot( rRoot )
{
}

void XclExpXmlStyleSheet::SaveXml( XclExpXmlStream& rStrm )
{
    sax_fastparser::FSHelperPtr aStyleSheet = rStrm.CreateOutputStream(
            u"xl/styles.xml"_ustr,
            u"styles.xml",
            rStrm.GetCurrentStream()->getOutputStream(),
            "application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml",
            oox::getRelationship(Relationship::STYLES));
    rStrm.PushStream( aStyleSheet );

    aStyleSheet->startElement(XML_styleSheet, XML_xmlns, rStrm.getNamespaceURL(OOX_NS(xls)));

    CreateRecord( EXC_ID_FORMATLIST )->SaveXml( rStrm );
    CreateRecord( EXC_ID_FONTLIST )->SaveXml( rStrm );
    CreateRecord( EXC_ID_XFLIST )->SaveXml( rStrm );
    CreateRecord( EXC_ID_DXFS )->SaveXml( rStrm );
    CreateRecord( EXC_ID_PALETTE )->SaveXml( rStrm );

    aStyleSheet->endElement( XML_styleSheet );

    rStrm.PopStream();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
