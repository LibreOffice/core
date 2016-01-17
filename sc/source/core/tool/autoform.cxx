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

#include "autoform.hxx"

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/itemset.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <editeng/langitem.hxx>
#include <tools/urlobj.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <tools/tenccvt.hxx>

#include "globstr.hrc"
#include "document.hxx"

/*
 * XXX: BIG RED NOTICE! Changes MUST be binary file format compatible and MUST
 * be synchronized with Writer's SwTableAutoFmtTbl sw/source/core/doc/tblafmt.cxx
 */

const sal_Char *linker_dummy = "";

static const sal_Char sAutoTblFmtName[] = "autotbl.fmt";

// till SO5PF
const sal_uInt16 AUTOFORMAT_ID_X        = 9501;
const sal_uInt16 AUTOFORMAT_ID_358      = 9601;
const sal_uInt16 AUTOFORMAT_DATA_ID_X   = 9502;

// from SO5 on
// in following versions the value of the IDs must be higher
const sal_uInt16 AUTOFORMAT_ID_504      = 9801;
const sal_uInt16 AUTOFORMAT_DATA_ID_504 = 9802;

const sal_uInt16 AUTOFORMAT_DATA_ID_552 = 9902;

// --- from 641 on: CJK and CTL font settings
const sal_uInt16 AUTOFORMAT_DATA_ID_641 = 10002;

// --- from 680/dr14 on: diagonal frame lines
const sal_uInt16 AUTOFORMAT_ID_680DR14      = 10011;
const sal_uInt16 AUTOFORMAT_DATA_ID_680DR14 = 10012;

// --- from 680/dr25 on: store strings as UTF-8
const sal_uInt16 AUTOFORMAT_ID_680DR25      = 10021;

// --- from DEV300/overline2 on: overline support
const sal_uInt16 AUTOFORMAT_ID_300OVRLN      = 10031;
const sal_uInt16 AUTOFORMAT_DATA_ID_300OVRLN = 10032;

// --- Bug fix to fdo#31005: Table Autoformats does not save/apply all properties (Writer and Calc)
const sal_uInt16 AUTOFORMAT_ID_31005      = 10041;
const sal_uInt16 AUTOFORMAT_DATA_ID_31005 = 10042;

// current version
const sal_uInt16 AUTOFORMAT_ID          = AUTOFORMAT_ID_31005;
const sal_uInt16 AUTOFORMAT_DATA_ID     = AUTOFORMAT_DATA_ID_31005;

namespace
{
    /// Read an AutoFormatSwBlob from stream.
    SvStream& operator>>(SvStream &stream, AutoFormatSwBlob &blob)
    {
        blob.Reset();

        sal_uInt64 endOfBlob = 0;
        stream.ReadUInt64( endOfBlob );

        const sal_uInt64 currentPosition = stream.Tell();
        const sal_uInt64 blobSize = endOfBlob - currentPosition;
        // A zero-size indicates an empty blob. This happens when Calc creates a new autoformat,
        // since it (naturally) doesn't have any writer-specific data to write.
        if (blobSize)
        {
            blob.pData = new sal_uInt8[blobSize];
            blob.size = static_cast<sal_Size>(blobSize);
            stream.Read(blob.pData, blob.size);
        }

        return stream;
    }

    /// Write an AutoFormatSwBlob to stream.
    SvStream& WriteAutoFormatSwBlob(SvStream &stream, AutoFormatSwBlob &blob)
    {
        const sal_uInt64 endOfBlob = stream.Tell() + sizeof(sal_uInt64) + blob.size;
        stream.WriteUInt64( endOfBlob );
        if (blob.size)
            stream.Write(blob.pData, blob.size);

        return stream;
    }
}

ScAfVersions::ScAfVersions() :
    nFontVersion(0),
    nFontHeightVersion(0),
    nWeightVersion(0),
    nPostureVersion(0),
    nUnderlineVersion(0),
    nOverlineVersion(0),
    nCrossedOutVersion(0),
    nContourVersion(0),
    nShadowedVersion(0),
    nColorVersion(0),
    nBoxVersion(0),
    nLineVersion(0),
    nBrushVersion(0),
    nAdjustVersion(0),
    nHorJustifyVersion(0),
    nVerJustifyVersion(0),
    nOrientationVersion(0),
    nMarginVersion(0),
    nBoolVersion(0),
    nInt32Version(0),
    nRotateModeVersion(0),
    nNumFmtVersion(0)
{
}

void ScAfVersions::Load( SvStream& rStream, sal_uInt16 nVer )
{
    rStream.ReadUInt16( nFontVersion );
    rStream.ReadUInt16( nFontHeightVersion );
    rStream.ReadUInt16( nWeightVersion );
    rStream.ReadUInt16( nPostureVersion );
    rStream.ReadUInt16( nUnderlineVersion );
    if ( nVer >= AUTOFORMAT_ID_300OVRLN )
        rStream.ReadUInt16( nOverlineVersion );
    rStream.ReadUInt16( nCrossedOutVersion );
    rStream.ReadUInt16( nContourVersion );
    rStream.ReadUInt16( nShadowedVersion );
    rStream.ReadUInt16( nColorVersion );
    rStream.ReadUInt16( nBoxVersion );
    if ( nVer >= AUTOFORMAT_ID_680DR14 )
        rStream.ReadUInt16( nLineVersion );
    rStream.ReadUInt16( nBrushVersion );
    rStream.ReadUInt16( nAdjustVersion );
    if (nVer >= AUTOFORMAT_ID_31005)
        rStream >> swVersions;
    rStream.ReadUInt16( nHorJustifyVersion );
    rStream.ReadUInt16( nVerJustifyVersion );
    rStream.ReadUInt16( nOrientationVersion );
    rStream.ReadUInt16( nMarginVersion );
    rStream.ReadUInt16( nBoolVersion );
    if ( nVer >= AUTOFORMAT_ID_504 )
    {
        rStream.ReadUInt16( nInt32Version );
        rStream.ReadUInt16( nRotateModeVersion );
    }
    rStream.ReadUInt16( nNumFmtVersion );
}

void ScAfVersions::Write(SvStream& rStream, sal_uInt16 fileVersion)
{
    rStream.WriteUInt16( SvxFontItem(ATTR_FONT).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxFontHeightItem(240, 100, ATTR_FONT_HEIGHT).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxWeightItem(WEIGHT_NORMAL, ATTR_FONT_WEIGHT).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxPostureItem(ITALIC_NONE, ATTR_FONT_POSTURE).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxUnderlineItem(UNDERLINE_NONE, ATTR_FONT_UNDERLINE).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxOverlineItem(UNDERLINE_NONE, ATTR_FONT_OVERLINE).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxCrossedOutItem(STRIKEOUT_NONE, ATTR_FONT_CROSSEDOUT).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxContourItem(false, ATTR_FONT_CONTOUR).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxShadowedItem(false, ATTR_FONT_SHADOWED).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxColorItem(ATTR_FONT_COLOR).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxBoxItem(ATTR_BORDER).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxLineItem(SID_FRAME_LINESTYLE).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxBrushItem(ATTR_BACKGROUND).GetVersion(fileVersion) );

    rStream.WriteUInt16( SvxAdjustItem(SVX_ADJUST_LEFT, 0).GetVersion(fileVersion) );
    if (fileVersion >= SOFFICE_FILEFORMAT_50)
        WriteAutoFormatSwBlob( rStream, swVersions );

    rStream.WriteUInt16( SvxHorJustifyItem(SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxVerJustifyItem(SVX_VER_JUSTIFY_STANDARD, ATTR_VER_JUSTIFY).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxOrientationItem(SVX_ORIENTATION_STANDARD, 0).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxMarginItem(ATTR_MARGIN).GetVersion(fileVersion) );
    rStream.WriteUInt16( SfxBoolItem(ATTR_LINEBREAK).GetVersion(fileVersion) );
    rStream.WriteUInt16( SfxInt32Item(ATTR_ROTATE_VALUE).GetVersion(fileVersion) );
    rStream.WriteUInt16( SvxRotateModeItem(SVX_ROTATE_MODE_STANDARD,0).GetVersion(fileVersion) );

    rStream.WriteUInt16( 0 );       // Num-Format
}

ScAutoFormatDataField::ScAutoFormatDataField() :
    aFont( ATTR_FONT ),
    aHeight( 240, 100, ATTR_FONT_HEIGHT ),
    aWeight( WEIGHT_NORMAL, ATTR_FONT_WEIGHT ),
    aPosture( ITALIC_NONE, ATTR_FONT_POSTURE ),

    aCJKFont( ATTR_CJK_FONT ),
    aCJKHeight( 240, 100, ATTR_CJK_FONT_HEIGHT ),
    aCJKWeight( WEIGHT_NORMAL, ATTR_CJK_FONT_WEIGHT ),
    aCJKPosture( ITALIC_NONE, ATTR_CJK_FONT_POSTURE ),

    aCTLFont( ATTR_CTL_FONT ),
    aCTLHeight( 240, 100, ATTR_CTL_FONT_HEIGHT ),
    aCTLWeight( WEIGHT_NORMAL, ATTR_CTL_FONT_WEIGHT ),
    aCTLPosture( ITALIC_NONE, ATTR_CTL_FONT_POSTURE ),

    aUnderline( UNDERLINE_NONE,ATTR_FONT_UNDERLINE ),
    aOverline( UNDERLINE_NONE,ATTR_FONT_OVERLINE ),
    aCrossedOut( STRIKEOUT_NONE, ATTR_FONT_CROSSEDOUT ),
    aContour( false, ATTR_FONT_CONTOUR ),
    aShadowed( false, ATTR_FONT_SHADOWED ),
    aColor( ATTR_FONT_COLOR ),
    aBox( ATTR_BORDER ),
    aTLBR( ATTR_BORDER_TLBR ),
    aBLTR( ATTR_BORDER_BLTR ),
    aBackground( ATTR_BACKGROUND ),
    aAdjust( SVX_ADJUST_LEFT, 0 ),
    aHorJustify( SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY ),
    aVerJustify( SVX_VER_JUSTIFY_STANDARD, ATTR_VER_JUSTIFY ),
    aMargin( ATTR_MARGIN ),
    aLinebreak( ATTR_LINEBREAK ),
    aRotateAngle( ATTR_ROTATE_VALUE ),
    aRotateMode( SVX_ROTATE_MODE_STANDARD, ATTR_ROTATE_MODE )
{
}

ScAutoFormatDataField::ScAutoFormatDataField( const ScAutoFormatDataField& rCopy ) :
    aFont( rCopy.aFont ),
    aHeight( rCopy.aHeight ),
    aWeight( rCopy.aWeight ),
    aPosture( rCopy.aPosture ),
    aCJKFont( rCopy.aCJKFont ),
    aCJKHeight( rCopy.aCJKHeight ),
    aCJKWeight( rCopy.aCJKWeight ),
    aCJKPosture( rCopy.aCJKPosture ),
    aCTLFont( rCopy.aCTLFont ),
    aCTLHeight( rCopy.aCTLHeight ),
    aCTLWeight( rCopy.aCTLWeight ),
    aCTLPosture( rCopy.aCTLPosture ),
    aUnderline( rCopy.aUnderline ),
    aOverline( rCopy.aOverline ),
    aCrossedOut( rCopy.aCrossedOut ),
    aContour( rCopy.aContour ),
    aShadowed( rCopy.aShadowed ),
    aColor( rCopy.aColor ),
    aBox( rCopy.aBox ),
    aTLBR( rCopy.aTLBR ),
    aBLTR( rCopy.aBLTR ),
    aBackground( rCopy.aBackground ),
    aAdjust( rCopy.aAdjust ),
    aHorJustify( rCopy.aHorJustify ),
    aVerJustify( rCopy.aVerJustify ),
    aStacked( rCopy.aStacked ),
    aMargin( rCopy.aMargin ),
    aLinebreak( rCopy.aLinebreak ),
    aRotateAngle( rCopy.aRotateAngle ),
    aRotateMode( rCopy.aRotateMode ),
    aNumFormat( rCopy.aNumFormat )
{
}

ScAutoFormatDataField::~ScAutoFormatDataField()
{
}

void ScAutoFormatDataField::SetAdjust( const SvxAdjustItem& rAdjust )
{
    aAdjust.SetAdjust( rAdjust.GetAdjust() );
    aAdjust.SetOneWord( rAdjust.GetOneWord() );
    aAdjust.SetLastBlock( rAdjust.GetLastBlock() );
}

#define READ( aItem, ItemType, nVers )      \
    pNew = aItem.Create( rStream, nVers );  \
    aItem = *static_cast<ItemType*>(pNew);  \
    delete pNew;

bool ScAutoFormatDataField::Load( SvStream& rStream, const ScAfVersions& rVersions, sal_uInt16 nVer )
{
    SfxPoolItem* pNew;
    SvxOrientationItem aOrientation( SVX_ORIENTATION_STANDARD, 0 );

    READ( aFont,        SvxFontItem,        rVersions.nFontVersion)
    READ( aHeight,      SvxFontHeightItem,  rVersions.nFontHeightVersion)
    READ( aWeight,      SvxWeightItem,      rVersions.nWeightVersion)
    READ( aPosture,     SvxPostureItem,     rVersions.nPostureVersion)
    // --- from 641 on: CJK and CTL font settings
    if( AUTOFORMAT_DATA_ID_641 <= nVer )
    {
        READ( aCJKFont,     SvxFontItem,        rVersions.nFontVersion)
        READ( aCJKHeight,   SvxFontHeightItem,  rVersions.nFontHeightVersion)
        READ( aCJKWeight,   SvxWeightItem,      rVersions.nWeightVersion)
        READ( aCJKPosture,  SvxPostureItem,     rVersions.nPostureVersion)
        READ( aCTLFont,     SvxFontItem,        rVersions.nFontVersion)
        READ( aCTLHeight,   SvxFontHeightItem,  rVersions.nFontHeightVersion)
        READ( aCTLWeight,   SvxWeightItem,      rVersions.nWeightVersion)
        READ( aCTLPosture,  SvxPostureItem,     rVersions.nPostureVersion)
    }
    READ( aUnderline,   SvxUnderlineItem,   rVersions.nUnderlineVersion)
    if ( nVer >= AUTOFORMAT_DATA_ID_300OVRLN )
    {
        READ( aOverline,    SvxOverlineItem,    rVersions.nOverlineVersion)
    }
    READ( aCrossedOut,  SvxCrossedOutItem,  rVersions.nCrossedOutVersion)
    READ( aContour,     SvxContourItem,     rVersions.nContourVersion)
    READ( aShadowed,    SvxShadowedItem,    rVersions.nShadowedVersion)
    READ( aColor,       SvxColorItem,       rVersions.nColorVersion)
    READ( aBox,         SvxBoxItem,         rVersions.nBoxVersion)

    // --- from 680/dr14 on: diagonal frame lines
    if( AUTOFORMAT_DATA_ID_680DR14 <= nVer )
    {
        READ( aTLBR, SvxLineItem, rVersions.nLineVersion)
        READ( aBLTR, SvxLineItem, rVersions.nLineVersion)
    }

    READ( aBackground,  SvxBrushItem,       rVersions.nBrushVersion)

    pNew = aAdjust.Create( rStream, rVersions.nAdjustVersion );
    SetAdjust( *static_cast<SvxAdjustItem*>(pNew) );
    delete pNew;

    if (nVer >= AUTOFORMAT_DATA_ID_31005)
        rStream >> m_swFields;

    READ( aHorJustify,   SvxHorJustifyItem,  rVersions.nHorJustifyVersion)
    READ( aVerJustify,   SvxVerJustifyItem,  rVersions.nVerJustifyVersion)
    READ( aOrientation,  SvxOrientationItem, rVersions.nOrientationVersion)
    READ( aMargin,       SvxMarginItem,      rVersions.nMarginVersion)

    pNew = aLinebreak.Create( rStream, rVersions.nBoolVersion );
    SetLinebreak( *static_cast<SfxBoolItem*>(pNew) );
    delete pNew;

    if ( nVer >= AUTOFORMAT_DATA_ID_504 )
    {
        pNew = aRotateAngle.Create( rStream, rVersions.nInt32Version );
        SetRotateAngle( *static_cast<SfxInt32Item*>(pNew) );
        delete pNew;
        pNew = aRotateMode.Create( rStream, rVersions.nRotateModeVersion );
        SetRotateMode( *static_cast<SvxRotateModeItem*>(pNew) );
        delete pNew;
    }

    if( 0 == rVersions.nNumFmtVersion )
    {
        // --- from 680/dr25 on: store strings as UTF-8
        rtl_TextEncoding eCharSet = (nVer >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        aNumFormat.Load( rStream, eCharSet );
    }

    //  adjust charset in font
    rtl_TextEncoding eSysSet = osl_getThreadTextEncoding();
    rtl_TextEncoding eSrcSet = rStream.GetStreamCharSet();
    if( eSrcSet != eSysSet && aFont.GetCharSet() == eSrcSet )
        aFont.SetCharSet(eSysSet);

    aStacked.SetValue( aOrientation.IsStacked() );
    aRotateAngle.SetValue( aOrientation.GetRotation( aRotateAngle.GetValue() ) );

    return (rStream.GetError() == 0);
}

bool ScAutoFormatDataField::Save( SvStream& rStream, sal_uInt16 fileVersion )
{
    SvxOrientationItem aOrientation( aRotateAngle.GetValue(), aStacked.GetValue(), 0 );

    aFont.Store         ( rStream, aFont.GetVersion( fileVersion ) );
    aHeight.Store       ( rStream, aHeight.GetVersion( fileVersion ) );
    aWeight.Store       ( rStream, aWeight.GetVersion( fileVersion ) );
    aPosture.Store      ( rStream, aPosture.GetVersion( fileVersion ) );
    // --- from 641 on: CJK and CTL font settings
    aCJKFont.Store      ( rStream, aCJKFont.GetVersion( fileVersion ) );
    aCJKHeight.Store    ( rStream, aCJKHeight.GetVersion( fileVersion ) );
    aCJKWeight.Store    ( rStream, aCJKWeight.GetVersion( fileVersion ) );
    aCJKPosture.Store   ( rStream, aCJKPosture.GetVersion( fileVersion ) );
    aCTLFont.Store      ( rStream, aCTLFont.GetVersion( fileVersion ) );
    aCTLHeight.Store    ( rStream, aCTLHeight.GetVersion( fileVersion ) );
    aCTLWeight.Store    ( rStream, aCTLWeight.GetVersion( fileVersion ) );
    aCTLPosture.Store   ( rStream, aCTLPosture.GetVersion( fileVersion ) );

    aUnderline.Store    ( rStream, aUnderline.GetVersion( fileVersion ) );
    // --- from DEV300/overline2 on: overline support
    aOverline.Store     ( rStream, aOverline.GetVersion( fileVersion ) );
    aCrossedOut.Store   ( rStream, aCrossedOut.GetVersion( fileVersion ) );
    aContour.Store      ( rStream, aContour.GetVersion( fileVersion ) );
    aShadowed.Store     ( rStream, aShadowed.GetVersion( fileVersion ) );
    aColor.Store        ( rStream, aColor.GetVersion( fileVersion ) );
    aBox.Store          ( rStream, aBox.GetVersion( fileVersion ) );

    // --- from 680/dr14 on: diagonal frame lines
    aTLBR.Store         ( rStream, aTLBR.GetVersion( fileVersion ) );
    aBLTR.Store         ( rStream, aBLTR.GetVersion( fileVersion ) );

    aBackground.Store   ( rStream, aBackground.GetVersion( fileVersion ) );

    aAdjust.Store       ( rStream, aAdjust.GetVersion( fileVersion ) );
    if (fileVersion >= SOFFICE_FILEFORMAT_50)
        WriteAutoFormatSwBlob( rStream, m_swFields );

    aHorJustify.Store   ( rStream, aHorJustify.GetVersion( fileVersion ) );
    aVerJustify.Store   ( rStream, aVerJustify.GetVersion( fileVersion ) );
    aOrientation.Store  ( rStream, aOrientation.GetVersion( fileVersion ) );
    aMargin.Store       ( rStream, aMargin.GetVersion( fileVersion ) );
    aLinebreak.Store    ( rStream, aLinebreak.GetVersion( fileVersion ) );
    // Rotation ab SO5
    aRotateAngle.Store  ( rStream, aRotateAngle.GetVersion( fileVersion ) );
    aRotateMode.Store   ( rStream, aRotateMode.GetVersion( fileVersion ) );

    // --- from 680/dr25 on: store strings as UTF-8
    aNumFormat.Save( rStream, RTL_TEXTENCODING_UTF8 );

    return (rStream.GetError() == 0);
}

ScAutoFormatData::ScAutoFormatData()
{
    nStrResId = USHRT_MAX;

    bIncludeValueFormat =
    bIncludeFont =
    bIncludeJustify =
    bIncludeFrame =
    bIncludeBackground =
    bIncludeWidthHeight = true;

    ppDataField = new ScAutoFormatDataField*[ 16 ];
    for( sal_uInt16 nIndex = 0; nIndex < 16; ++nIndex )
        ppDataField[ nIndex ] = new ScAutoFormatDataField;
}

ScAutoFormatData::ScAutoFormatData( const ScAutoFormatData& rData ) :
        aName( rData.aName ),
        nStrResId( rData.nStrResId ),
        bIncludeFont( rData.bIncludeFont ),
        bIncludeJustify( rData.bIncludeJustify ),
        bIncludeFrame( rData.bIncludeFrame ),
        bIncludeBackground( rData.bIncludeBackground ),
        bIncludeValueFormat( rData.bIncludeValueFormat ),
        bIncludeWidthHeight( rData.bIncludeWidthHeight )
{
    ppDataField = new ScAutoFormatDataField*[ 16 ];
    for( sal_uInt16 nIndex = 0; nIndex < 16; ++nIndex )
        ppDataField[ nIndex ] = new ScAutoFormatDataField( rData.GetField( nIndex ) );
}

ScAutoFormatData::~ScAutoFormatData()
{
    for( sal_uInt16 nIndex = 0; nIndex < 16; ++nIndex )
        delete ppDataField[ nIndex ];
    delete[] ppDataField;
}

ScAutoFormatDataField& ScAutoFormatData::GetField( sal_uInt16 nIndex )
{
    OSL_ENSURE( nIndex < 16, "ScAutoFormatData::GetField - illegal index" );
    OSL_ENSURE( ppDataField && ppDataField[ nIndex ], "ScAutoFormatData::GetField - no data" );
    return *ppDataField[ nIndex ];
}

const ScAutoFormatDataField& ScAutoFormatData::GetField( sal_uInt16 nIndex ) const
{
    OSL_ENSURE( nIndex < 16, "ScAutoFormatData::GetField - illegal index" );
    OSL_ENSURE( ppDataField && ppDataField[ nIndex ], "ScAutoFormatData::GetField - no data" );
    return *ppDataField[ nIndex ];
}

const SfxPoolItem* ScAutoFormatData::GetItem( sal_uInt16 nIndex, sal_uInt16 nWhich ) const
{
    const ScAutoFormatDataField& rField = GetField( nIndex );
    switch( nWhich )
    {
        case ATTR_FONT:             return &rField.GetFont();
        case ATTR_FONT_HEIGHT:      return &rField.GetHeight();
        case ATTR_FONT_WEIGHT:      return &rField.GetWeight();
        case ATTR_FONT_POSTURE:     return &rField.GetPosture();
        case ATTR_CJK_FONT:         return &rField.GetCJKFont();
        case ATTR_CJK_FONT_HEIGHT:  return &rField.GetCJKHeight();
        case ATTR_CJK_FONT_WEIGHT:  return &rField.GetCJKWeight();
        case ATTR_CJK_FONT_POSTURE: return &rField.GetCJKPosture();
        case ATTR_CTL_FONT:         return &rField.GetCTLFont();
        case ATTR_CTL_FONT_HEIGHT:  return &rField.GetCTLHeight();
        case ATTR_CTL_FONT_WEIGHT:  return &rField.GetCTLWeight();
        case ATTR_CTL_FONT_POSTURE: return &rField.GetCTLPosture();
        case ATTR_FONT_UNDERLINE:   return &rField.GetUnderline();
        case ATTR_FONT_OVERLINE:    return &rField.GetOverline();
        case ATTR_FONT_CROSSEDOUT:  return &rField.GetCrossedOut();
        case ATTR_FONT_CONTOUR:     return &rField.GetContour();
        case ATTR_FONT_SHADOWED:    return &rField.GetShadowed();
        case ATTR_FONT_COLOR:       return &rField.GetColor();
        case ATTR_BORDER:           return &rField.GetBox();
        case ATTR_BORDER_TLBR:      return &rField.GetTLBR();
        case ATTR_BORDER_BLTR:      return &rField.GetBLTR();
        case ATTR_BACKGROUND:       return &rField.GetBackground();
        case ATTR_HOR_JUSTIFY:      return &rField.GetHorJustify();
        case ATTR_VER_JUSTIFY:      return &rField.GetVerJustify();
        case ATTR_STACKED:          return &rField.GetStacked();
        case ATTR_MARGIN:           return &rField.GetMargin();
        case ATTR_LINEBREAK:        return &rField.GetLinebreak();
        case ATTR_ROTATE_VALUE:     return &rField.GetRotateAngle();
        case ATTR_ROTATE_MODE:      return &rField.GetRotateMode();
    }
    return nullptr;
}

void ScAutoFormatData::PutItem( sal_uInt16 nIndex, const SfxPoolItem& rItem )
{
    ScAutoFormatDataField& rField = GetField( nIndex );
    switch( rItem.Which() )
    {
        case ATTR_FONT:             rField.SetFont( static_cast<const SvxFontItem&>(rItem) );              break;
        case ATTR_FONT_HEIGHT:      rField.SetHeight( static_cast<const SvxFontHeightItem&>(rItem) );      break;
        case ATTR_FONT_WEIGHT:      rField.SetWeight( static_cast<const SvxWeightItem&>(rItem) );          break;
        case ATTR_FONT_POSTURE:     rField.SetPosture( static_cast<const SvxPostureItem&>(rItem) );        break;
        case ATTR_CJK_FONT:         rField.SetCJKFont( static_cast<const SvxFontItem&>(rItem) );           break;
        case ATTR_CJK_FONT_HEIGHT:  rField.SetCJKHeight( static_cast<const SvxFontHeightItem&>(rItem) );   break;
        case ATTR_CJK_FONT_WEIGHT:  rField.SetCJKWeight( static_cast<const SvxWeightItem&>(rItem) );       break;
        case ATTR_CJK_FONT_POSTURE: rField.SetCJKPosture( static_cast<const SvxPostureItem&>(rItem) );     break;
        case ATTR_CTL_FONT:         rField.SetCTLFont( static_cast<const SvxFontItem&>(rItem) );           break;
        case ATTR_CTL_FONT_HEIGHT:  rField.SetCTLHeight( static_cast<const SvxFontHeightItem&>(rItem) );   break;
        case ATTR_CTL_FONT_WEIGHT:  rField.SetCTLWeight( static_cast<const SvxWeightItem&>(rItem) );       break;
        case ATTR_CTL_FONT_POSTURE: rField.SetCTLPosture( static_cast<const SvxPostureItem&>(rItem) );     break;
        case ATTR_FONT_UNDERLINE:   rField.SetUnderline( static_cast<const SvxUnderlineItem&>(rItem) );    break;
        case ATTR_FONT_OVERLINE:    rField.SetOverline( static_cast<const SvxOverlineItem&>(rItem) );      break;
        case ATTR_FONT_CROSSEDOUT:  rField.SetCrossedOut( static_cast<const SvxCrossedOutItem&>(rItem) );  break;
        case ATTR_FONT_CONTOUR:     rField.SetContour( static_cast<const SvxContourItem&>(rItem) );        break;
        case ATTR_FONT_SHADOWED:    rField.SetShadowed( static_cast<const SvxShadowedItem&>(rItem) );      break;
        case ATTR_FONT_COLOR:       rField.SetColor( static_cast<const SvxColorItem&>(rItem) );            break;
        case ATTR_BORDER:           rField.SetBox( static_cast<const SvxBoxItem&>(rItem) );                break;
        case ATTR_BORDER_TLBR:      rField.SetTLBR( static_cast<const SvxLineItem&>(rItem) );              break;
        case ATTR_BORDER_BLTR:      rField.SetBLTR( static_cast<const SvxLineItem&>(rItem) );              break;
        case ATTR_BACKGROUND:       rField.SetBackground( static_cast<const SvxBrushItem&>(rItem) );       break;
        case ATTR_HOR_JUSTIFY:      rField.SetHorJustify( static_cast<const SvxHorJustifyItem&>(rItem) );  break;
        case ATTR_VER_JUSTIFY:      rField.SetVerJustify( static_cast<const SvxVerJustifyItem&>(rItem) );  break;
        case ATTR_STACKED:          rField.SetStacked( static_cast<const SfxBoolItem&>(rItem) );           break;
        case ATTR_MARGIN:           rField.SetMargin( static_cast<const SvxMarginItem&>(rItem) );          break;
        case ATTR_LINEBREAK:        rField.SetLinebreak( static_cast<const SfxBoolItem&>(rItem) );         break;
        case ATTR_ROTATE_VALUE:     rField.SetRotateAngle( static_cast<const SfxInt32Item&>(rItem) );      break;
        case ATTR_ROTATE_MODE:      rField.SetRotateMode( static_cast<const SvxRotateModeItem&>(rItem) );  break;
    }
}

void ScAutoFormatData::CopyItem( sal_uInt16 nToIndex, sal_uInt16 nFromIndex, sal_uInt16 nWhich )
{
    const SfxPoolItem* pItem = GetItem( nFromIndex, nWhich );
    if( pItem )
        PutItem( nToIndex, *pItem );
}

const ScNumFormatAbbrev& ScAutoFormatData::GetNumFormat( sal_uInt16 nIndex ) const
{
    return GetField( nIndex ).GetNumFormat();
}

bool ScAutoFormatData::IsEqualData( sal_uInt16 nIndex1, sal_uInt16 nIndex2 ) const
{
    bool bEqual = true;
    const ScAutoFormatDataField& rField1 = GetField( nIndex1 );
    const ScAutoFormatDataField& rField2 = GetField( nIndex2 );

    if( bIncludeValueFormat )
    {
        bEqual = bEqual
            && (rField1.GetNumFormat()      == rField2.GetNumFormat());
    }
    if( bIncludeFont )
    {
        bEqual = bEqual
            && (rField1.GetFont()           == rField2.GetFont())
            && (rField1.GetHeight()         == rField2.GetHeight())
            && (rField1.GetWeight()         == rField2.GetWeight())
            && (rField1.GetPosture()        == rField2.GetPosture())
            && (rField1.GetCJKFont()        == rField2.GetCJKFont())
            && (rField1.GetCJKHeight()      == rField2.GetCJKHeight())
            && (rField1.GetCJKWeight()      == rField2.GetCJKWeight())
            && (rField1.GetCJKPosture()     == rField2.GetCJKPosture())
            && (rField1.GetCTLFont()        == rField2.GetCTLFont())
            && (rField1.GetCTLHeight()      == rField2.GetCTLHeight())
            && (rField1.GetCTLWeight()      == rField2.GetCTLWeight())
            && (rField1.GetCTLPosture()     == rField2.GetCTLPosture())
            && (rField1.GetUnderline()      == rField2.GetUnderline())
            && (rField1.GetOverline()       == rField2.GetOverline())
            && (rField1.GetCrossedOut()     == rField2.GetCrossedOut())
            && (rField1.GetContour()        == rField2.GetContour())
            && (rField1.GetShadowed()       == rField2.GetShadowed())
            && (rField1.GetColor()          == rField2.GetColor());
    }
    if( bIncludeJustify )
    {
        bEqual = bEqual
            && (rField1.GetHorJustify()     == rField2.GetHorJustify())
            && (rField1.GetVerJustify()     == rField2.GetVerJustify())
            && (rField1.GetStacked()        == rField2.GetStacked())
            && (rField1.GetLinebreak()      == rField2.GetLinebreak())
            && (rField1.GetMargin()         == rField2.GetMargin())
            && (rField1.GetRotateAngle()    == rField2.GetRotateAngle())
            && (rField1.GetRotateMode()     == rField2.GetRotateMode());
    }
    if( bIncludeFrame )
    {
        bEqual = bEqual
            && (rField1.GetBox()            == rField2.GetBox())
            && (rField1.GetTLBR()           == rField2.GetTLBR())
            && (rField1.GetBLTR()           == rField2.GetBLTR());
    }
    if( bIncludeBackground )
    {
        bEqual = bEqual
            && (rField1.GetBackground()     == rField2.GetBackground());
    }
    return bEqual;
}

void ScAutoFormatData::FillToItemSet( sal_uInt16 nIndex, SfxItemSet& rItemSet, ScDocument& rDoc ) const
{
    const ScAutoFormatDataField& rField = GetField( nIndex );

    if( bIncludeValueFormat )
    {
        ScNumFormatAbbrev& rNumFormat = (ScNumFormatAbbrev&)rField.GetNumFormat();
        SfxUInt32Item aValueFormat( ATTR_VALUE_FORMAT, 0 );
        aValueFormat.SetValue( rNumFormat.GetFormatIndex( *rDoc.GetFormatTable() ) );
        rItemSet.Put( aValueFormat );
        rItemSet.Put( SvxLanguageItem( rNumFormat.GetLanguage(), ATTR_LANGUAGE_FORMAT ) );
    }
    if( bIncludeFont )
    {
        rItemSet.Put( rField.GetFont() );
        rItemSet.Put( rField.GetHeight() );
        rItemSet.Put( rField.GetWeight() );
        rItemSet.Put( rField.GetPosture() );
        // do not insert empty CJK font
        const SvxFontItem& rCJKFont = rField.GetCJKFont();
        if (!rCJKFont.GetStyleName().isEmpty())
        {
            rItemSet.Put( rCJKFont );
            rItemSet.Put( rField.GetCJKHeight() );
            rItemSet.Put( rField.GetCJKWeight() );
            rItemSet.Put( rField.GetCJKPosture() );
        }
        else
        {
            rItemSet.Put( rField.GetHeight(), ATTR_CJK_FONT_HEIGHT );
            rItemSet.Put( rField.GetWeight(), ATTR_CJK_FONT_WEIGHT );
            rItemSet.Put( rField.GetPosture(), ATTR_CJK_FONT_POSTURE );
        }
        // do not insert empty CTL font
        const SvxFontItem& rCTLFont = rField.GetCTLFont();
        if (!rCTLFont.GetStyleName().isEmpty())
        {
            rItemSet.Put( rCTLFont );
            rItemSet.Put( rField.GetCTLHeight() );
            rItemSet.Put( rField.GetCTLWeight() );
            rItemSet.Put( rField.GetCTLPosture() );
        }
        else
        {
            rItemSet.Put( rField.GetHeight(), ATTR_CTL_FONT_HEIGHT );
            rItemSet.Put( rField.GetWeight(), ATTR_CTL_FONT_WEIGHT );
            rItemSet.Put( rField.GetPosture(), ATTR_CTL_FONT_POSTURE );
        }
        rItemSet.Put( rField.GetUnderline() );
        rItemSet.Put( rField.GetOverline() );
        rItemSet.Put( rField.GetCrossedOut() );
        rItemSet.Put( rField.GetContour() );
        rItemSet.Put( rField.GetShadowed() );
        rItemSet.Put( rField.GetColor() );
    }
    if( bIncludeJustify )
    {
        rItemSet.Put( rField.GetHorJustify() );
        rItemSet.Put( rField.GetVerJustify() );
        rItemSet.Put( rField.GetStacked() );
        rItemSet.Put( rField.GetLinebreak() );
        rItemSet.Put( rField.GetMargin() );
        rItemSet.Put( rField.GetRotateAngle() );
        rItemSet.Put( rField.GetRotateMode() );
    }
    if( bIncludeFrame )
    {
        rItemSet.Put( rField.GetBox() );
        rItemSet.Put( rField.GetTLBR() );
        rItemSet.Put( rField.GetBLTR() );
    }
    if( bIncludeBackground )
        rItemSet.Put( rField.GetBackground() );
}

void ScAutoFormatData::GetFromItemSet( sal_uInt16 nIndex, const SfxItemSet& rItemSet, const ScNumFormatAbbrev& rNumFormat )
{
    ScAutoFormatDataField& rField = GetField( nIndex );

    rField.SetNumFormat     ( rNumFormat);
    rField.SetFont          ( static_cast<const SvxFontItem&>          (rItemSet.Get( ATTR_FONT )) );
    rField.SetHeight        ( static_cast<const SvxFontHeightItem&>    (rItemSet.Get( ATTR_FONT_HEIGHT )) );
    rField.SetWeight        ( static_cast<const SvxWeightItem&>        (rItemSet.Get( ATTR_FONT_WEIGHT )) );
    rField.SetPosture       ( static_cast<const SvxPostureItem&>       (rItemSet.Get( ATTR_FONT_POSTURE )) );
    rField.SetCJKFont       ( static_cast<const SvxFontItem&>          (rItemSet.Get( ATTR_CJK_FONT )) );
    rField.SetCJKHeight     ( static_cast<const SvxFontHeightItem&>    (rItemSet.Get( ATTR_CJK_FONT_HEIGHT )) );
    rField.SetCJKWeight     ( static_cast<const SvxWeightItem&>        (rItemSet.Get( ATTR_CJK_FONT_WEIGHT )) );
    rField.SetCJKPosture    ( static_cast<const SvxPostureItem&>       (rItemSet.Get( ATTR_CJK_FONT_POSTURE )) );
    rField.SetCTLFont       ( static_cast<const SvxFontItem&>          (rItemSet.Get( ATTR_CTL_FONT )) );
    rField.SetCTLHeight     ( static_cast<const SvxFontHeightItem&>    (rItemSet.Get( ATTR_CTL_FONT_HEIGHT )) );
    rField.SetCTLWeight     ( static_cast<const SvxWeightItem&>        (rItemSet.Get( ATTR_CTL_FONT_WEIGHT )) );
    rField.SetCTLPosture    ( static_cast<const SvxPostureItem&>       (rItemSet.Get( ATTR_CTL_FONT_POSTURE )) );
    rField.SetUnderline     ( static_cast<const SvxUnderlineItem&>     (rItemSet.Get( ATTR_FONT_UNDERLINE )) );
    rField.SetOverline      ( static_cast<const SvxOverlineItem&>      (rItemSet.Get( ATTR_FONT_OVERLINE )) );
    rField.SetCrossedOut    ( static_cast<const SvxCrossedOutItem&>    (rItemSet.Get( ATTR_FONT_CROSSEDOUT )) );
    rField.SetContour       ( static_cast<const SvxContourItem&>       (rItemSet.Get( ATTR_FONT_CONTOUR )) );
    rField.SetShadowed      ( static_cast<const SvxShadowedItem&>      (rItemSet.Get( ATTR_FONT_SHADOWED )) );
    rField.SetColor         ( static_cast<const SvxColorItem&>         (rItemSet.Get( ATTR_FONT_COLOR )) );
    rField.SetTLBR          ( static_cast<const SvxLineItem&>          (rItemSet.Get( ATTR_BORDER_TLBR )) );
    rField.SetBLTR          ( static_cast<const SvxLineItem&>          (rItemSet.Get( ATTR_BORDER_BLTR )) );
    rField.SetHorJustify    ( static_cast<const SvxHorJustifyItem&>    (rItemSet.Get( ATTR_HOR_JUSTIFY )) );
    rField.SetVerJustify    ( static_cast<const SvxVerJustifyItem&>    (rItemSet.Get( ATTR_VER_JUSTIFY )) );
    rField.SetStacked       ( static_cast<const SfxBoolItem&>          (rItemSet.Get( ATTR_STACKED )) );
    rField.SetLinebreak     ( static_cast<const SfxBoolItem&>          (rItemSet.Get( ATTR_LINEBREAK )) );
    rField.SetMargin        ( static_cast<const SvxMarginItem&>        (rItemSet.Get( ATTR_MARGIN )) );
    rField.SetBackground    ( static_cast<const SvxBrushItem&>         (rItemSet.Get( ATTR_BACKGROUND )) );
    rField.SetRotateAngle   ( static_cast<const SfxInt32Item&>         (rItemSet.Get( ATTR_ROTATE_VALUE )) );
    rField.SetRotateMode    ( static_cast<const SvxRotateModeItem&>    (rItemSet.Get( ATTR_ROTATE_MODE )) );
}

bool ScAutoFormatData::Load( SvStream& rStream, const ScAfVersions& rVersions )
{
    sal_uInt16  nVer = 0;
    rStream.ReadUInt16( nVer );
    bool bRet = 0 == rStream.GetError();
    if( bRet && (nVer == AUTOFORMAT_DATA_ID_X ||
            (AUTOFORMAT_DATA_ID_504 <= nVer && nVer <= AUTOFORMAT_DATA_ID)) )
    {
        // --- from 680/dr25 on: store strings as UTF-8
        if (nVer >= AUTOFORMAT_ID_680DR25)
        {
            aName = read_uInt16_lenPrefixed_uInt8s_ToOUString(rStream,
                RTL_TEXTENCODING_UTF8);
        }
        else
            aName = rStream.ReadUniOrByteString( rStream.GetStreamCharSet() );

        if( AUTOFORMAT_DATA_ID_552 <= nVer )
        {
            rStream.ReadUInt16( nStrResId );
            sal_uInt16 nId = RID_SVXSTR_TBLAFMT_BEGIN + nStrResId;
            if( RID_SVXSTR_TBLAFMT_BEGIN <= nId &&
                nId < RID_SVXSTR_TBLAFMT_END )
            {
                aName = SVX_RESSTR( nId );
            }
            else
                nStrResId = USHRT_MAX;
        }

        bool b;
        rStream.ReadCharAsBool( b ); bIncludeFont = b;
        rStream.ReadCharAsBool( b ); bIncludeJustify = b;
        rStream.ReadCharAsBool( b ); bIncludeFrame = b;
        rStream.ReadCharAsBool( b ); bIncludeBackground = b;
        rStream.ReadCharAsBool( b ); bIncludeValueFormat = b;
        rStream.ReadCharAsBool( b ); bIncludeWidthHeight = b;

        if (nVer >= AUTOFORMAT_DATA_ID_31005)
            rStream >> m_swFields;

        bRet = 0 == rStream.GetError();
        for( sal_uInt16 i = 0; bRet && i < 16; ++i )
            bRet = GetField( i ).Load( rStream, rVersions, nVer );
    }
    else
        bRet = false;
    return bRet;
}

bool ScAutoFormatData::Save(SvStream& rStream, sal_uInt16 fileVersion)
{
    sal_uInt16 nVal = AUTOFORMAT_DATA_ID;
    rStream.WriteUInt16( nVal );
    // --- from 680/dr25 on: store strings as UTF-8
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStream, aName, RTL_TEXTENCODING_UTF8);

    rStream.WriteUInt16( nStrResId );
    rStream.WriteBool( bIncludeFont );
    rStream.WriteBool( bIncludeJustify );
    rStream.WriteBool( bIncludeFrame );
    rStream.WriteBool( bIncludeBackground );
    rStream.WriteBool( bIncludeValueFormat );
    rStream.WriteBool( bIncludeWidthHeight );

    if (fileVersion >= SOFFICE_FILEFORMAT_50)
        WriteAutoFormatSwBlob( rStream, m_swFields );

    bool bRet = 0 == rStream.GetError();
    for (sal_uInt16 i = 0; bRet && (i < 16); i++)
        bRet = GetField( i ).Save( rStream, fileVersion );

    return bRet;
}

ScAutoFormat::ScAutoFormat() :
    mbSaveLater(false)
{
    //  create default autoformat
    ScAutoFormatData* pData = new ScAutoFormatData;
    OUString aName(ScGlobal::GetRscString(STR_STYLENAME_STANDARD));
    pData->SetName(aName);

    //  default font, default height
    vcl::Font aStdFont = OutputDevice::GetDefaultFont(
        DefaultFontType::LATIN_SPREADSHEET, LANGUAGE_ENGLISH_US, GetDefaultFontFlags::OnlyOne );
    SvxFontItem aFontItem(
        aStdFont.GetFamily(), aStdFont.GetFamilyName(), aStdFont.GetStyleName(),
        aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_FONT );

    aStdFont = OutputDevice::GetDefaultFont(
        DefaultFontType::CJK_SPREADSHEET, LANGUAGE_ENGLISH_US, GetDefaultFontFlags::OnlyOne );
    SvxFontItem aCJKFontItem(
        aStdFont.GetFamily(), aStdFont.GetFamilyName(), aStdFont.GetStyleName(),
        aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_CJK_FONT );

    aStdFont = OutputDevice::GetDefaultFont(
        DefaultFontType::CTL_SPREADSHEET, LANGUAGE_ENGLISH_US, GetDefaultFontFlags::OnlyOne );
    SvxFontItem aCTLFontItem(
        aStdFont.GetFamily(), aStdFont.GetFamilyName(), aStdFont.GetStyleName(),
        aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_CTL_FONT );

    SvxFontHeightItem aHeight( 200, 100, ATTR_FONT_HEIGHT );      // 10 pt;

    //  black thin border
    Color aBlack( COL_BLACK );
    ::editeng::SvxBorderLine aLine( &aBlack, DEF_LINE_WIDTH_0 );
    SvxBoxItem aBox( ATTR_BORDER );
    aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);
    aBox.SetLine(&aLine, SvxBoxItemLine::TOP);
    aBox.SetLine(&aLine, SvxBoxItemLine::RIGHT);
    aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);

    Color aWhite(COL_WHITE);
    Color aBlue(COL_BLUE);
    SvxColorItem aWhiteText( aWhite, ATTR_FONT_COLOR );
    SvxColorItem aBlackText( aBlack, ATTR_FONT_COLOR );
    SvxBrushItem aBlueBack( aBlue, ATTR_BACKGROUND );
    SvxBrushItem aWhiteBack( aWhite, ATTR_BACKGROUND );
    SvxBrushItem aGray70Back( Color(0x4d, 0x4d, 0x4d), ATTR_BACKGROUND );
    SvxBrushItem aGray20Back( Color(0xcc, 0xcc, 0xcc), ATTR_BACKGROUND );

    for (sal_uInt16 i=0; i<16; i++)
    {
        pData->PutItem( i, aBox );
        pData->PutItem( i, aFontItem );
        pData->PutItem( i, aCJKFontItem );
        pData->PutItem( i, aCTLFontItem );
        aHeight.SetWhich( ATTR_FONT_HEIGHT );
        pData->PutItem( i, aHeight );
        aHeight.SetWhich( ATTR_CJK_FONT_HEIGHT );
        pData->PutItem( i, aHeight );
        aHeight.SetWhich( ATTR_CTL_FONT_HEIGHT );
        pData->PutItem( i, aHeight );
        if (i<4)                                    // top: white on blue
        {
            pData->PutItem( i, aWhiteText );
            pData->PutItem( i, aBlueBack );
        }
        else if ( i%4 == 0 )                        // left: white on gray70
        {
            pData->PutItem( i, aWhiteText );
            pData->PutItem( i, aGray70Back );
        }
        else if ( i%4 == 3 || i >= 12 )             // right and bottom: black on gray20
        {
            pData->PutItem( i, aBlackText );
            pData->PutItem( i, aGray20Back );
        }
        else                                        // center: black on white
        {
            pData->PutItem( i, aBlackText );
            pData->PutItem( i, aWhiteBack );
        }
    }

    insert(pData);
}

bool DefaultFirstEntry::operator() (const OUString& left, const OUString& right) const
{
    OUString aStrStandard(ScGlobal::GetRscString(STR_STYLENAME_STANDARD));
    if ( ScGlobal::GetpTransliteration()->isEqual( left, aStrStandard ) )
        return true;
    if ( ScGlobal::GetpTransliteration()->isEqual( right, aStrStandard ) )
        return false;
    return ScGlobal::GetCollator()->compareString( left, right) < 0;
}

ScAutoFormat::ScAutoFormat(const ScAutoFormat& r) :
    maData(r.maData),
    mbSaveLater(false) {}

ScAutoFormat::~ScAutoFormat()
{
    //  When modified via StarOne then only the SaveLater flag is set and no saving is done.
    //  If the flag is set then save now.

    if (mbSaveLater)
        Save();
}

void ScAutoFormat::SetSaveLater( bool bSet )
{
    mbSaveLater = bSet;
}

const ScAutoFormatData* ScAutoFormat::findByIndex(size_t nIndex) const
{
    if (nIndex >= maData.size())
        return nullptr;

    MapType::const_iterator it = maData.begin();
    std::advance(it, nIndex);
    return it->second;
}

ScAutoFormatData* ScAutoFormat::findByIndex(size_t nIndex)
{
    if (nIndex >= maData.size())
        return nullptr;

    MapType::iterator it = maData.begin();
    std::advance(it, nIndex);
    return it->second;
}

ScAutoFormat::iterator ScAutoFormat::find(const ScAutoFormatData* pData)
{
    MapType::iterator it = maData.begin(), itEnd = maData.end();
    for (; it != itEnd; ++it)
    {
        if (it->second == pData)
            return it;
    }
    return itEnd;
}

ScAutoFormat::iterator ScAutoFormat::find(const OUString& rName)
{
    return maData.find(rName);
}

bool ScAutoFormat::insert(ScAutoFormatData* pNew)
{
    OUString aName = pNew->GetName();
    return maData.insert(aName, pNew).second;
}

void ScAutoFormat::erase(const iterator& it)
{
    maData.erase(it);
}

size_t ScAutoFormat::size() const
{
    return maData.size();
}

ScAutoFormat::const_iterator ScAutoFormat::begin() const
{
    return maData.begin();
}

ScAutoFormat::const_iterator ScAutoFormat::end() const
{
    return maData.end();
}

ScAutoFormat::iterator ScAutoFormat::begin()
{
    return maData.begin();
}

ScAutoFormat::iterator ScAutoFormat::end()
{
    return maData.end();
}

bool ScAutoFormat::Load()
{
    INetURLObject aURL;
    SvtPathOptions aPathOpt;
    aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
    aURL.setFinalSlash();
    aURL.Append( sAutoTblFmtName );

    SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), StreamMode::READ );
    SvStream* pStream = aMedium.GetInStream();
    bool bRet = (pStream && pStream->GetError() == 0);
    if (bRet)
    {
        SvStream& rStream = *pStream;
        // Attention: A common header has to be read
        sal_uInt16 nVal = 0;
        rStream.ReadUInt16( nVal );
        bRet = 0 == rStream.GetError();

        if (bRet)
        {
            if( nVal == AUTOFORMAT_ID_358 ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                sal_uInt16 nFileVers = SOFFICE_FILEFORMAT_40;
                sal_uInt8 nChrSet, nCnt;
                long nPos = rStream.Tell();
                rStream.ReadUChar( nCnt ).ReadUChar( nChrSet );
                if( rStream.Tell() != sal_uLong(nPos + nCnt) )
                {
                    OSL_FAIL( "header contains more/newer data" );
                    rStream.Seek( nPos + nCnt );
                }
                rStream.SetStreamCharSet( GetSOLoadTextEncoding( nChrSet ) );
                rStream.SetVersion( nFileVers );
            }

            if( nVal == AUTOFORMAT_ID_358 || nVal == AUTOFORMAT_ID_X ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                m_aVersions.Load( rStream, nVal );        // Item-Versionen

                ScAutoFormatData* pData;
                sal_uInt16 nAnz = 0;
                rStream.ReadUInt16( nAnz );
                bRet = (rStream.GetError() == 0);
                for (sal_uInt16 i=0; bRet && (i < nAnz); i++)
                {
                    pData = new ScAutoFormatData();
                    bRet = pData->Load(rStream, m_aVersions);
                    insert(pData);
                }
            }
            else
                bRet = false;
        }
    }
    mbSaveLater = false;
    return bRet;
}

bool ScAutoFormat::Save()
{
    INetURLObject aURL;
    SvtPathOptions aPathOpt;
    aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
    aURL.setFinalSlash();
    aURL.Append(sAutoTblFmtName);

    SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), StreamMode::WRITE );
    SvStream* pStream = aMedium.GetOutStream();
    bool bRet = (pStream && pStream->GetError() == 0);
    if (bRet)
    {
        const sal_uInt16 fileVersion = SOFFICE_FILEFORMAT_50;
        SvStream& rStream = *pStream;
        rStream.SetVersion( fileVersion );

        // Attention: A common header has to be saved
        sal_uInt16 nVal = AUTOFORMAT_ID;
        rStream.WriteUInt16( nVal )
               .WriteUChar( 2 )         // Number of chars of the header including this
               .WriteUChar( ::GetSOStoreTextEncoding(
                    osl_getThreadTextEncoding() ) );
        m_aVersions.Write(rStream, fileVersion);

        bRet &= (rStream.GetError() == 0);

        rStream.WriteUInt16( maData.size() - 1 );
        bRet &= (rStream.GetError() == 0);
        MapType::iterator it = maData.begin(), itEnd = maData.end();
        if (it != itEnd)
        {
            for (++it; bRet && it != itEnd; ++it) // Skip the first item.
            {
                bRet &= it->second->Save(rStream, fileVersion);
            }
        }

        rStream.Flush();

        aMedium.Commit();
    }
    mbSaveLater = false;
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
