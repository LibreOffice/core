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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#define READ_OLDVERS

#include "autoform.hxx"

#include <sfx2/app.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/itemset.hxx>
#include <tools/shl.hxx>
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

//------------------------------------------------------------------------

const sal_Char *linker_dummy = "";

//  Standard-Name ist jetzt STR_STYLENAME_STANDARD (wie Vorlagen)
//static const sal_Char __FAR_DATA cStandardName[] = "Standard";

static const sal_Char __FAR_DATA sAutoTblFmtName[] = "autotbl.fmt";

// bis SO5PF
const sal_uInt16 AUTOFORMAT_ID_X        = 9501;
const sal_uInt16 AUTOFORMAT_ID_358      = 9601;
const sal_uInt16 AUTOFORMAT_DATA_ID_X   = 9502;

// ab SO5
//! in nachfolgenden Versionen muss der Betrag dieser IDs groesser sein
const sal_uInt16 AUTOFORMAT_ID_504      = 9801;
const sal_uInt16 AUTOFORMAT_DATA_ID_504 = 9802;

const sal_uInt16 AUTOFORMAT_ID_552      = 9901;
const sal_uInt16 AUTOFORMAT_DATA_ID_552 = 9902;

// --- from 641 on: CJK and CTL font settings
const sal_uInt16 AUTOFORMAT_ID_641      = 10001;
const sal_uInt16 AUTOFORMAT_DATA_ID_641 = 10002;

// --- from 680/dr14 on: diagonal frame lines
const sal_uInt16 AUTOFORMAT_ID_680DR14      = 10011;
const sal_uInt16 AUTOFORMAT_DATA_ID_680DR14 = 10012;

// --- from 680/dr25 on: #21549# store strings as UTF-8
const sal_uInt16 AUTOFORMAT_ID_680DR25      = 10021;
const sal_uInt16 AUTOFORMAT_DATA_ID_680DR25 = 10022;

// --- from DEV300/overline2 on: #5991# overline support
const sal_uInt16 AUTOFORMAT_ID_300OVRLN      = 10031;
const sal_uInt16 AUTOFORMAT_DATA_ID_300OVRLN = 10032;

// aktuelle Version
const sal_uInt16 AUTOFORMAT_ID          = AUTOFORMAT_ID_300OVRLN;
const sal_uInt16 AUTOFORMAT_DATA_ID     = AUTOFORMAT_DATA_ID_300OVRLN;


#ifdef READ_OLDVERS
const sal_uInt16 AUTOFORMAT_OLD_ID_OLD  = 4201;
const sal_uInt16 AUTOFORMAT_OLD_DATA_ID = 4202;
const sal_uInt16 AUTOFORMAT_OLD_ID_NEW  = 4203;
#endif


//  Struct mit Versionsnummern der Items

struct ScAfVersions
{
public:
    sal_uInt16 nFontVersion;
    sal_uInt16 nFontHeightVersion;
    sal_uInt16 nWeightVersion;
    sal_uInt16 nPostureVersion;
    sal_uInt16 nUnderlineVersion;
    sal_uInt16 nOverlineVersion;
    sal_uInt16 nCrossedOutVersion;
    sal_uInt16 nContourVersion;
    sal_uInt16 nShadowedVersion;
    sal_uInt16 nColorVersion;
    sal_uInt16 nBoxVersion;
    sal_uInt16 nLineVersion;
    sal_uInt16 nBrushVersion;

    sal_uInt16 nAdjustVersion;

    sal_uInt16 nHorJustifyVersion;
    sal_uInt16 nVerJustifyVersion;
    sal_uInt16 nOrientationVersion;
    sal_uInt16 nMarginVersion;
    sal_uInt16 nBoolVersion;
    sal_uInt16 nInt32Version;
    sal_uInt16 nRotateModeVersion;

    sal_uInt16 nNumFmtVersion;

    ScAfVersions();
    void Load( SvStream& rStream, sal_uInt16 nVer );
    static void Write(SvStream& rStream);
};

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
    rStream >> nFontVersion;
    rStream >> nFontHeightVersion;
    rStream >> nWeightVersion;
    rStream >> nPostureVersion;
    rStream >> nUnderlineVersion;
    if ( nVer >= AUTOFORMAT_ID_300OVRLN )
        rStream >> nOverlineVersion;
    rStream >> nCrossedOutVersion;
    rStream >> nContourVersion;
    rStream >> nShadowedVersion;
    rStream >> nColorVersion;
    rStream >> nBoxVersion;
    if ( nVer >= AUTOFORMAT_ID_680DR14 )
        rStream >> nLineVersion;
    rStream >> nBrushVersion;
    rStream >> nAdjustVersion;
    rStream >> nHorJustifyVersion;
    rStream >> nVerJustifyVersion;
    rStream >> nOrientationVersion;
    rStream >> nMarginVersion;
    rStream >> nBoolVersion;
    if ( nVer >= AUTOFORMAT_ID_504 )
    {
        rStream >> nInt32Version;
        rStream >> nRotateModeVersion;
    }
    rStream >> nNumFmtVersion;
}

void ScAfVersions::Write(SvStream& rStream)
{
    rStream << SvxFontItem(ATTR_FONT).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxFontHeightItem(240, 100, ATTR_FONT_HEIGHT).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxWeightItem(WEIGHT_NORMAL, ATTR_FONT_WEIGHT).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxPostureItem(ITALIC_NONE, ATTR_FONT_POSTURE).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxUnderlineItem(UNDERLINE_NONE, ATTR_FONT_UNDERLINE).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxOverlineItem(UNDERLINE_NONE, ATTR_FONT_OVERLINE).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxCrossedOutItem(STRIKEOUT_NONE, ATTR_FONT_CROSSEDOUT).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxContourItem(sal_False, ATTR_FONT_CONTOUR).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxShadowedItem(sal_False, ATTR_FONT_SHADOWED).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxColorItem(ATTR_FONT_COLOR).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxBoxItem(ATTR_BORDER).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxLineItem(SID_FRAME_LINESTYLE).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxBrushItem(ATTR_BACKGROUND).GetVersion(SOFFICE_FILEFORMAT_40);

    rStream << SvxAdjustItem(SVX_ADJUST_LEFT, 0).GetVersion(SOFFICE_FILEFORMAT_40);

    rStream << SvxHorJustifyItem(SVX_HOR_JUSTIFY_STANDARD, ATTR_HOR_JUSTIFY).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxVerJustifyItem(SVX_VER_JUSTIFY_STANDARD, ATTR_VER_JUSTIFY).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxOrientationItem(SVX_ORIENTATION_STANDARD, 0).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxMarginItem(ATTR_MARGIN).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SfxBoolItem(ATTR_LINEBREAK).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SfxInt32Item(ATTR_ROTATE_VALUE).GetVersion(SOFFICE_FILEFORMAT_40);
    rStream << SvxRotateModeItem(SVX_ROTATE_MODE_STANDARD,0).GetVersion(SOFFICE_FILEFORMAT_40);

    rStream << (sal_uInt16)0;       // Num-Format
}

//  ---------------------------------------------------------------------------

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
    aContour( sal_False, ATTR_FONT_CONTOUR ),
    aShadowed( sal_False, ATTR_FONT_SHADOWED ),
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
    aItem = *(ItemType*)pNew;               \
    delete pNew;

sal_Bool ScAutoFormatDataField::Load( SvStream& rStream, const ScAfVersions& rVersions, sal_uInt16 nVer )
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
    SetAdjust( *(SvxAdjustItem*)pNew );
    delete pNew;

    READ( aHorJustify,   SvxHorJustifyItem,  rVersions.nHorJustifyVersion)
    READ( aVerJustify,   SvxVerJustifyItem,  rVersions.nVerJustifyVersion)
    READ( aOrientation,  SvxOrientationItem, rVersions.nOrientationVersion)
    READ( aMargin,       SvxMarginItem,      rVersions.nMarginVersion)

    pNew = aLinebreak.Create( rStream, rVersions.nBoolVersion );
    SetLinebreak( *(SfxBoolItem*)pNew );
    delete pNew;

    if ( nVer >= AUTOFORMAT_DATA_ID_504 )
    {
        pNew = aRotateAngle.Create( rStream, rVersions.nInt32Version );
        SetRotateAngle( *(SfxInt32Item*)pNew );
        delete pNew;
        pNew = aRotateMode.Create( rStream, rVersions.nRotateModeVersion );
        SetRotateMode( *(SvxRotateModeItem*)pNew );
        delete pNew;
    }

    if( 0 == rVersions.nNumFmtVersion )
    {
        // --- from 680/dr25 on: #21549# store strings as UTF-8
        CharSet eCharSet = (nVer >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        aNumFormat.Load( rStream, eCharSet );
    }

    //  adjust charset in font
    CharSet eSysSet = gsl_getSystemTextEncoding();
    CharSet eSrcSet = rStream.GetStreamCharSet();
    if( eSrcSet != eSysSet && aFont.GetCharSet() == eSrcSet )
        aFont.SetCharSet(eSysSet);

    aStacked.SetValue( aOrientation.IsStacked() );
    aRotateAngle.SetValue( aOrientation.GetRotation( aRotateAngle.GetValue() ) );

    return (rStream.GetError() == 0);
}

#ifdef READ_OLDVERS
sal_Bool ScAutoFormatDataField::LoadOld( SvStream& rStream, const ScAfVersions& rVersions )
{
    SfxPoolItem* pNew;
    SvxOrientationItem aOrientation( SVX_ORIENTATION_STANDARD, 0 );

    aNumFormat.Load(rStream, rStream.GetStreamCharSet());

    READ( aFont,        SvxFontItem,        rVersions.nFontVersion)
    READ( aHeight,      SvxFontHeightItem,  rVersions.nFontHeightVersion)
    READ( aWeight,      SvxWeightItem,      rVersions.nWeightVersion)
    READ( aPosture,     SvxPostureItem,     rVersions.nPostureVersion)
    READ( aUnderline,   SvxUnderlineItem,   rVersions.nUnderlineVersion)
    READ( aCrossedOut,  SvxCrossedOutItem,  rVersions.nCrossedOutVersion)
    READ( aContour,     SvxContourItem,     rVersions.nContourVersion)
    READ( aShadowed,    SvxShadowedItem,    rVersions.nShadowedVersion)
    READ( aColor,       SvxColorItem,       rVersions.nColorVersion)
    READ( aHorJustify,  SvxHorJustifyItem,  rVersions.nHorJustifyVersion)
    READ( aVerJustify,  SvxVerJustifyItem,  rVersions.nVerJustifyVersion)
    READ( aOrientation, SvxOrientationItem, rVersions.nOrientationVersion)
    pNew = aLinebreak.Create( rStream, rVersions.nBoolVersion );
    SetLinebreak( *(SfxBoolItem*)pNew );
    delete pNew;
    READ( aMargin,      SvxMarginItem,      rVersions.nMarginVersion)
    READ( aBox,         SvxBoxItem,         rVersions.nBoxVersion)
    READ( aBackground,  SvxBrushItem,       rVersions.nBrushVersion)

    aStacked.SetValue( aOrientation.IsStacked() );
    aRotateAngle.SetValue( aOrientation.GetRotation( aRotateAngle.GetValue() ) );

    return (rStream.GetError() == 0);
}
#endif

sal_Bool ScAutoFormatDataField::Save( SvStream& rStream )
{
    SvxOrientationItem aOrientation( aRotateAngle.GetValue(), aStacked.GetValue(), 0 );

    aFont.Store         ( rStream, aFont.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aHeight.Store       ( rStream, aHeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aWeight.Store       ( rStream, aWeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aPosture.Store      ( rStream, aPosture.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    // --- from 641 on: CJK and CTL font settings
    aCJKFont.Store      ( rStream, aCJKFont.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aCJKHeight.Store    ( rStream, aCJKHeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aCJKWeight.Store    ( rStream, aCJKWeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aCJKPosture.Store   ( rStream, aCJKPosture.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aCTLFont.Store      ( rStream, aCTLFont.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aCTLHeight.Store    ( rStream, aCTLHeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aCTLWeight.Store    ( rStream, aCTLWeight.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aCTLPosture.Store   ( rStream, aCTLPosture.GetVersion( SOFFICE_FILEFORMAT_40 ) );

    aUnderline.Store    ( rStream, aUnderline.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    // --- from DEV300/overline2 on: overline support
    aOverline.Store     ( rStream, aOverline.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aCrossedOut.Store   ( rStream, aCrossedOut.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aContour.Store      ( rStream, aContour.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aShadowed.Store     ( rStream, aShadowed.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aColor.Store        ( rStream, aColor.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aBox.Store          ( rStream, aBox.GetVersion( SOFFICE_FILEFORMAT_40 ) );

    // --- from 680/dr14 on: diagonal frame lines
    aTLBR.Store         ( rStream, aTLBR.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aBLTR.Store         ( rStream, aBLTR.GetVersion( SOFFICE_FILEFORMAT_40 ) );

    aBackground.Store   ( rStream, aBackground.GetVersion( SOFFICE_FILEFORMAT_40 ) );

    aAdjust.Store       ( rStream, aAdjust.GetVersion( SOFFICE_FILEFORMAT_40 ) );

    aHorJustify.Store   ( rStream, aHorJustify.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aVerJustify.Store   ( rStream, aVerJustify.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aOrientation.Store  ( rStream, aOrientation.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aMargin.Store       ( rStream, aMargin.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aLinebreak.Store    ( rStream, aLinebreak.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    // Rotation ab SO5
    aRotateAngle.Store  ( rStream, aRotateAngle.GetVersion( SOFFICE_FILEFORMAT_40 ) );
    aRotateMode.Store   ( rStream, aRotateMode.GetVersion( SOFFICE_FILEFORMAT_40 ) );

    // --- from 680/dr25 on: #21549# store strings as UTF-8
    aNumFormat.Save( rStream, RTL_TEXTENCODING_UTF8 );

    return (rStream.GetError() == 0);
}


//  ---------------------------------------------------------------------------

ScAutoFormatData::ScAutoFormatData()
{
    nStrResId = USHRT_MAX;

    bIncludeValueFormat =
    bIncludeFont =
    bIncludeJustify =
    bIncludeFrame =
    bIncludeBackground =
    bIncludeWidthHeight = sal_True;

    ppDataField = new ScAutoFormatDataField*[ 16 ];
    for( sal_uInt16 nIndex = 0; nIndex < 16; ++nIndex )
        ppDataField[ nIndex ] = new ScAutoFormatDataField;
}

ScAutoFormatData::ScAutoFormatData( const ScAutoFormatData& rData ) :
        ScDataObject(),
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
    DBG_ASSERT( nIndex < 16, "ScAutoFormatData::GetField - illegal index" );
    DBG_ASSERT( ppDataField && ppDataField[ nIndex ], "ScAutoFormatData::GetField - no data" );
    return *ppDataField[ nIndex ];
}

const ScAutoFormatDataField& ScAutoFormatData::GetField( sal_uInt16 nIndex ) const
{
    DBG_ASSERT( nIndex < 16, "ScAutoFormatData::GetField - illegal index" );
    DBG_ASSERT( ppDataField && ppDataField[ nIndex ], "ScAutoFormatData::GetField - no data" );
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
    return NULL;
}

void ScAutoFormatData::PutItem( sal_uInt16 nIndex, const SfxPoolItem& rItem )
{
    ScAutoFormatDataField& rField = GetField( nIndex );
    switch( rItem.Which() )
    {
        case ATTR_FONT:             rField.SetFont( (const SvxFontItem&)rItem );              break;
        case ATTR_FONT_HEIGHT:      rField.SetHeight( (const SvxFontHeightItem&)rItem );      break;
        case ATTR_FONT_WEIGHT:      rField.SetWeight( (const SvxWeightItem&)rItem );          break;
        case ATTR_FONT_POSTURE:     rField.SetPosture( (const SvxPostureItem&)rItem );        break;
        case ATTR_CJK_FONT:         rField.SetCJKFont( (const SvxFontItem&)rItem );           break;
        case ATTR_CJK_FONT_HEIGHT:  rField.SetCJKHeight( (const SvxFontHeightItem&)rItem );   break;
        case ATTR_CJK_FONT_WEIGHT:  rField.SetCJKWeight( (const SvxWeightItem&)rItem );       break;
        case ATTR_CJK_FONT_POSTURE: rField.SetCJKPosture( (const SvxPostureItem&)rItem );     break;
        case ATTR_CTL_FONT:         rField.SetCTLFont( (const SvxFontItem&)rItem );           break;
        case ATTR_CTL_FONT_HEIGHT:  rField.SetCTLHeight( (const SvxFontHeightItem&)rItem );   break;
        case ATTR_CTL_FONT_WEIGHT:  rField.SetCTLWeight( (const SvxWeightItem&)rItem );       break;
        case ATTR_CTL_FONT_POSTURE: rField.SetCTLPosture( (const SvxPostureItem&)rItem );     break;
        case ATTR_FONT_UNDERLINE:   rField.SetUnderline( (const SvxUnderlineItem&)rItem );    break;
        case ATTR_FONT_OVERLINE:    rField.SetOverline( (const SvxOverlineItem&)rItem );      break;
        case ATTR_FONT_CROSSEDOUT:  rField.SetCrossedOut( (const SvxCrossedOutItem&)rItem );  break;
        case ATTR_FONT_CONTOUR:     rField.SetContour( (const SvxContourItem&)rItem );        break;
        case ATTR_FONT_SHADOWED:    rField.SetShadowed( (const SvxShadowedItem&)rItem );      break;
        case ATTR_FONT_COLOR:       rField.SetColor( (const SvxColorItem&)rItem );            break;
        case ATTR_BORDER:           rField.SetBox( (const SvxBoxItem&)rItem );                break;
        case ATTR_BORDER_TLBR:      rField.SetTLBR( (const SvxLineItem&)rItem );              break;
        case ATTR_BORDER_BLTR:      rField.SetBLTR( (const SvxLineItem&)rItem );              break;
        case ATTR_BACKGROUND:       rField.SetBackground( (const SvxBrushItem&)rItem );       break;
        case ATTR_HOR_JUSTIFY:      rField.SetHorJustify( (const SvxHorJustifyItem&)rItem );  break;
        case ATTR_VER_JUSTIFY:      rField.SetVerJustify( (const SvxVerJustifyItem&)rItem );  break;
        case ATTR_STACKED:          rField.SetStacked( (const SfxBoolItem&)rItem );           break;
        case ATTR_MARGIN:           rField.SetMargin( (const SvxMarginItem&)rItem );          break;
        case ATTR_LINEBREAK:        rField.SetLinebreak( (const SfxBoolItem&)rItem );         break;
        case ATTR_ROTATE_VALUE:     rField.SetRotateAngle( (const SfxInt32Item&)rItem );      break;
        case ATTR_ROTATE_MODE:      rField.SetRotateMode( (const SvxRotateModeItem&)rItem );  break;
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

sal_Bool ScAutoFormatData::IsEqualData( sal_uInt16 nIndex1, sal_uInt16 nIndex2 ) const
{
    sal_Bool bEqual = sal_True;
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
        // #103065# do not insert empty CJK font
        const SvxFontItem& rCJKFont = rField.GetCJKFont();
        if( rCJKFont.GetStyleName().Len() )
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
        // #103065# do not insert empty CTL font
        const SvxFontItem& rCTLFont = rField.GetCTLFont();
        if( rCTLFont.GetStyleName().Len() )
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
    rField.SetFont          ( (const SvxFontItem&)          rItemSet.Get( ATTR_FONT ) );
    rField.SetHeight        ( (const SvxFontHeightItem&)    rItemSet.Get( ATTR_FONT_HEIGHT ) );
    rField.SetWeight        ( (const SvxWeightItem&)        rItemSet.Get( ATTR_FONT_WEIGHT ) );
    rField.SetPosture       ( (const SvxPostureItem&)       rItemSet.Get( ATTR_FONT_POSTURE ) );
    rField.SetCJKFont       ( (const SvxFontItem&)          rItemSet.Get( ATTR_CJK_FONT ) );
    rField.SetCJKHeight     ( (const SvxFontHeightItem&)    rItemSet.Get( ATTR_CJK_FONT_HEIGHT ) );
    rField.SetCJKWeight     ( (const SvxWeightItem&)        rItemSet.Get( ATTR_CJK_FONT_WEIGHT ) );
    rField.SetCJKPosture    ( (const SvxPostureItem&)       rItemSet.Get( ATTR_CJK_FONT_POSTURE ) );
    rField.SetCTLFont       ( (const SvxFontItem&)          rItemSet.Get( ATTR_CTL_FONT ) );
    rField.SetCTLHeight     ( (const SvxFontHeightItem&)    rItemSet.Get( ATTR_CTL_FONT_HEIGHT ) );
    rField.SetCTLWeight     ( (const SvxWeightItem&)        rItemSet.Get( ATTR_CTL_FONT_WEIGHT ) );
    rField.SetCTLPosture    ( (const SvxPostureItem&)       rItemSet.Get( ATTR_CTL_FONT_POSTURE ) );
    rField.SetUnderline     ( (const SvxUnderlineItem&)     rItemSet.Get( ATTR_FONT_UNDERLINE ) );
    rField.SetOverline      ( (const SvxOverlineItem&)      rItemSet.Get( ATTR_FONT_OVERLINE ) );
    rField.SetCrossedOut    ( (const SvxCrossedOutItem&)    rItemSet.Get( ATTR_FONT_CROSSEDOUT ) );
    rField.SetContour       ( (const SvxContourItem&)       rItemSet.Get( ATTR_FONT_CONTOUR ) );
    rField.SetShadowed      ( (const SvxShadowedItem&)      rItemSet.Get( ATTR_FONT_SHADOWED ) );
    rField.SetColor         ( (const SvxColorItem&)         rItemSet.Get( ATTR_FONT_COLOR ) );
    rField.SetTLBR          ( (const SvxLineItem&)          rItemSet.Get( ATTR_BORDER_TLBR ) );
    rField.SetBLTR          ( (const SvxLineItem&)          rItemSet.Get( ATTR_BORDER_BLTR ) );
    rField.SetHorJustify    ( (const SvxHorJustifyItem&)    rItemSet.Get( ATTR_HOR_JUSTIFY ) );
    rField.SetVerJustify    ( (const SvxVerJustifyItem&)    rItemSet.Get( ATTR_VER_JUSTIFY ) );
    rField.SetStacked       ( (const SfxBoolItem&)          rItemSet.Get( ATTR_STACKED ) );
    rField.SetLinebreak     ( (const SfxBoolItem&)          rItemSet.Get( ATTR_LINEBREAK ) );
    rField.SetMargin        ( (const SvxMarginItem&)        rItemSet.Get( ATTR_MARGIN ) );
    rField.SetBackground    ( (const SvxBrushItem&)         rItemSet.Get( ATTR_BACKGROUND ) );
    rField.SetRotateAngle   ( (const SfxInt32Item&)         rItemSet.Get( ATTR_ROTATE_VALUE ) );
    rField.SetRotateMode    ( (const SvxRotateModeItem&)    rItemSet.Get( ATTR_ROTATE_MODE ) );
}

sal_Bool ScAutoFormatData::Load( SvStream& rStream, const ScAfVersions& rVersions )
{
    sal_Bool    bRet = sal_True;
    sal_uInt16  nVer = 0;
    rStream >> nVer;
    bRet = 0 == rStream.GetError();
    if( bRet && (nVer == AUTOFORMAT_DATA_ID_X ||
            (AUTOFORMAT_DATA_ID_504 <= nVer && nVer <= AUTOFORMAT_DATA_ID)) )
    {
        // --- from 680/dr25 on: #21549# store strings as UTF-8
        CharSet eCharSet = (nVer >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        rStream.ReadByteString( aName, eCharSet );
        if( AUTOFORMAT_DATA_ID_552 <= nVer )
        {
            rStream >> nStrResId;
            sal_uInt16 nId = RID_SVXSTR_TBLAFMT_BEGIN + nStrResId;
            if( RID_SVXSTR_TBLAFMT_BEGIN <= nId &&
                nId < RID_SVXSTR_TBLAFMT_END )
            {
                aName = SVX_RESSTR( nId );
            }
            else
                nStrResId = USHRT_MAX;
        }

        sal_Bool b;
        rStream >> b; bIncludeFont = b;
        rStream >> b; bIncludeJustify = b;
        rStream >> b; bIncludeFrame = b;
        rStream >> b; bIncludeBackground = b;
        rStream >> b; bIncludeValueFormat = b;
        rStream >> b; bIncludeWidthHeight = b;

        bRet = 0 == rStream.GetError();
        for( sal_uInt16 i = 0; bRet && i < 16; ++i )
            bRet = GetField( i ).Load( rStream, rVersions, nVer );
    }
    else
        bRet = sal_False;
    return bRet;
}

#ifdef READ_OLDVERS
sal_Bool ScAutoFormatData::LoadOld( SvStream& rStream, const ScAfVersions& rVersions )
{
    sal_Bool    bRet = sal_True;
    sal_uInt16  nVal = 0;
    rStream >> nVal;
    bRet = (rStream.GetError() == 0);
    if (bRet && (nVal == AUTOFORMAT_OLD_DATA_ID))
    {
        rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
        sal_Bool b;
        rStream >> b; bIncludeFont = b;
        rStream >> b; bIncludeJustify = b;
        rStream >> b; bIncludeFrame = b;
        rStream >> b; bIncludeBackground = b;
        rStream >> b; bIncludeValueFormat = b;
        rStream >> b; bIncludeWidthHeight = b;

        bRet = 0 == rStream.GetError();
        for (sal_uInt16 i=0; bRet && i < 16; i++)
            bRet = GetField( i ).LoadOld( rStream, rVersions );
    }
    else
        bRet = sal_False;
    return bRet;
}
#endif

sal_Bool ScAutoFormatData::Save(SvStream& rStream)
{
    sal_uInt16 nVal = AUTOFORMAT_DATA_ID;
    sal_Bool b;
    rStream << nVal;
    // --- from 680/dr25 on: #21549# store strings as UTF-8
    rStream.WriteByteString( aName, RTL_TEXTENCODING_UTF8 );

#if 0
    //  This was an internal flag to allow creating AutoFormats with localized names

    if ( USHRT_MAX == nStrResId )
    {
        String aIniVal( SFX_APP()->GetIniManager()->Get(
            SFX_GROUP_WORKINGSET_IMPL,
            String( RTL_CONSTASCII_USTRINGPARAM( "SaveTableAutoFmtNameId" ))));
        if( 0 != aIniVal.ToInt32() )
        {
            // check Name for ResId
            for( sal_uInt16 nId = RID_SVXSTR_TBLAFMT_BEGIN;
                        RID_SVXSTR_TBLAFMT_END > nId; ++nId )
            {
                String s( SVX_RES( nId ) );
                if( s == aName )
                {
                    nStrResId = nId - RID_SVXSTR_TBLAFMT_BEGIN;
                    break;
                }
            }
        }
    }
#endif

    rStream << nStrResId;
    rStream << ( b = bIncludeFont );
    rStream << ( b = bIncludeJustify );
    rStream << ( b = bIncludeFrame );
    rStream << ( b = bIncludeBackground );
    rStream << ( b = bIncludeValueFormat );
    rStream << ( b = bIncludeWidthHeight );

    sal_Bool bRet = 0 == rStream.GetError();
    for (sal_uInt16 i = 0; bRet && (i < 16); i++)
        bRet = GetField( i ).Save( rStream );

    return bRet;
}

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

ScAutoFormat::ScAutoFormat(sal_uInt16 nLim, sal_uInt16 nDel, sal_Bool bDup):
    ScSortedCollection        (nLim, nDel, bDup),
    bSaveLater              (sal_False)
{
    //  create default autoformat
    ScAutoFormatData* pData = new ScAutoFormatData;
    String aName(ScGlobal::GetRscString(STR_STYLENAME_STANDARD));
    pData->SetName(aName);

    //  default font, default height
    Font aStdFont = OutputDevice::GetDefaultFont(
        DEFAULTFONT_LATIN_SPREADSHEET, LANGUAGE_ENGLISH_US, DEFAULTFONT_FLAGS_ONLYONE );
    SvxFontItem aFontItem(
        aStdFont.GetFamily(), aStdFont.GetName(), aStdFont.GetStyleName(),
        aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_FONT );

    aStdFont = OutputDevice::GetDefaultFont(
        DEFAULTFONT_CJK_SPREADSHEET, LANGUAGE_ENGLISH_US, DEFAULTFONT_FLAGS_ONLYONE );
    SvxFontItem aCJKFontItem(
        aStdFont.GetFamily(), aStdFont.GetName(), aStdFont.GetStyleName(),
        aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_CJK_FONT );

    aStdFont = OutputDevice::GetDefaultFont(
        DEFAULTFONT_CTL_SPREADSHEET, LANGUAGE_ENGLISH_US, DEFAULTFONT_FLAGS_ONLYONE );
    SvxFontItem aCTLFontItem(
        aStdFont.GetFamily(), aStdFont.GetName(), aStdFont.GetStyleName(),
        aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_CTL_FONT );

    SvxFontHeightItem aHeight( 200, 100, ATTR_FONT_HEIGHT );      // 10 pt;

    //  black thin border
    Color aBlack( COL_BLACK );
    SvxBorderLine aLine( &aBlack, DEF_LINE_WIDTH_0 );
    SvxBoxItem aBox( ATTR_BORDER );
    aBox.SetLine(&aLine, BOX_LINE_LEFT);
    aBox.SetLine(&aLine, BOX_LINE_TOP);
    aBox.SetLine(&aLine, BOX_LINE_RIGHT);
    aBox.SetLine(&aLine, BOX_LINE_BOTTOM);

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

    Insert(pData);
}

ScAutoFormat::ScAutoFormat(const ScAutoFormat& rAutoFormat) :
    ScSortedCollection (rAutoFormat),
    bSaveLater       (sal_False)
{}

ScAutoFormat::~ScAutoFormat()
{
    //  Bei Aenderungen per StarOne wird nicht sofort gespeichert, sondern zuerst nur
    //  das SaveLater Flag gesetzt. Wenn das Flag noch gesetzt ist, jetzt speichern.

    if (bSaveLater)
        Save();
}

void ScAutoFormat::SetSaveLater( sal_Bool bSet )
{
    bSaveLater = bSet;
}

short ScAutoFormat::Compare(ScDataObject* pKey1, ScDataObject* pKey2) const
{
    String aStr1;
    String aStr2;
    ((ScAutoFormatData*)pKey1)->GetName(aStr1);
    ((ScAutoFormatData*)pKey2)->GetName(aStr2);
    String aStrStandard = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);
    if ( ScGlobal::GetpTransliteration()->isEqual( aStr1, aStrStandard ) )
        return -1;
    if ( ScGlobal::GetpTransliteration()->isEqual( aStr2, aStrStandard ) )
        return 1;
    return (short) ScGlobal::GetpTransliteration()->compareString( aStr1, aStr2 );
}

sal_Bool ScAutoFormat::Load()
{
    sal_Bool bRet = sal_True;

    INetURLObject aURL;
    SvtPathOptions aPathOpt;
    aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
    aURL.setFinalSlash();
    aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( sAutoTblFmtName ) ) );

    SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), STREAM_READ, sal_True );
    SvStream* pStream = aMedium.GetInStream();
    bRet = (pStream && pStream->GetError() == 0);
    if (bRet)
    {
        SvStream& rStream = *pStream;
        // Achtung hier muss ein allgemeiner Header gelesen werden
        sal_uInt16 nVal = 0;
        rStream >> nVal;
        bRet = 0 == rStream.GetError();

        ScAfVersions aVersions;

        if (bRet)
        {
            if( nVal == AUTOFORMAT_ID_358 ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                sal_uInt16 nFileVers = SOFFICE_FILEFORMAT_40;
                sal_uInt8 nChrSet, nCnt;
                long nPos = rStream.Tell();
                rStream >> nCnt >> nChrSet;
//              if( 4 <= nCnt )
//                  rStream >> nFileVers;
                if( rStream.Tell() != sal_uLong(nPos + nCnt) )
                {
                    DBG_ERRORFILE( "Der Header enthaelt mehr/neuere Daten" );
                    rStream.Seek( nPos + nCnt );
                }
                rStream.SetStreamCharSet( GetSOLoadTextEncoding( nChrSet, nFileVers ) );
                rStream.SetVersion( nFileVers );
            }

            if( nVal == AUTOFORMAT_ID_358 || nVal == AUTOFORMAT_ID_X ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                aVersions.Load( rStream, nVal );        // Item-Versionen

                ScAutoFormatData* pData;
                sal_uInt16 nAnz = 0;
                rStream >> nAnz;
                bRet = (rStream.GetError() == 0);
                for (sal_uInt16 i=0; bRet && (i < nAnz); i++)
                {
                    pData = new ScAutoFormatData();
                    bRet = pData->Load(rStream, aVersions);
                    Insert(pData);
                }
            }
#ifdef READ_OLDVERS
            else
            {
                if( AUTOFORMAT_OLD_ID_NEW == nVal )
                {
                    // alte Version der Versions laden
                    rStream >> aVersions.nFontVersion;
                    rStream >> aVersions.nFontHeightVersion;
                    rStream >> aVersions.nWeightVersion;
                    rStream >> aVersions.nPostureVersion;
                    rStream >> aVersions.nUnderlineVersion;
                    rStream >> aVersions.nCrossedOutVersion;
                    rStream >> aVersions.nContourVersion;
                    rStream >> aVersions.nShadowedVersion;
                    rStream >> aVersions.nColorVersion;
                    rStream >> aVersions.nHorJustifyVersion;
                    rStream >> aVersions.nVerJustifyVersion;
                    rStream >> aVersions.nOrientationVersion;
                    rStream >> aVersions.nBoolVersion;
                    rStream >> aVersions.nMarginVersion;
                    rStream >> aVersions.nBoxVersion;
                    rStream >> aVersions.nBrushVersion;
                }
                if( AUTOFORMAT_OLD_ID_OLD == nVal ||
                    AUTOFORMAT_OLD_ID_NEW == nVal )
                {
                    ScAutoFormatData* pData;
                    sal_uInt16 nAnz = 0;
                    rStream >> nAnz;
                    bRet = 0 == rStream.GetError();
                    for( sal_uInt16 i=0; bRet && (i < nAnz); ++i )
                    {
                        pData = new ScAutoFormatData();
                        bRet = pData->LoadOld( rStream, aVersions );
                        Insert( pData );
                    }
                }
                else
                    bRet = sal_False;
            }
#endif
        }
    }
    bSaveLater = sal_False;
    return bRet;
}

sal_Bool ScAutoFormat::Save()
{
    sal_Bool bRet = sal_True;

    INetURLObject aURL;
    SvtPathOptions aPathOpt;
    aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
    aURL.setFinalSlash();
    aURL.Append( String( RTL_CONSTASCII_USTRINGPARAM( sAutoTblFmtName ) ) );

    SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), STREAM_WRITE, sal_True );
    SvStream* pStream = aMedium.GetOutStream();
    bRet = (pStream && pStream->GetError() == 0);
    if (bRet)
    {
        SvStream& rStream = *pStream;
        rStream.SetVersion( SOFFICE_FILEFORMAT_40 );

        // Achtung hier muss ein allgemeiner Header gespeichert werden
        sal_uInt16 nVal = AUTOFORMAT_ID;
        rStream << nVal
                << (sal_uInt8)2         // Anzahl von Zeichen des Headers incl. diesem
                << (sal_uInt8)::GetSOStoreTextEncoding(
                    gsl_getSystemTextEncoding(), sal::static_int_cast<sal_uInt16>(rStream.GetVersion()) );
//              << (sal_uInt8)4         // Anzahl von Zeichen des Headers incl. diesem
//              << (sal_uInt8)::GetStoreCharSet(::GetSystemCharSet())
//              << (UNIT16)SOFFICE_FILEFORMAT_NOW;
        ScAfVersions::Write(rStream);           // Item-Versionen

        bRet = (rStream.GetError() == 0);
        //-----------------------------------------------------------
        rStream << (sal_uInt16)(nCount - 1);
        bRet = (rStream.GetError() == 0);
        for (sal_uInt16 i=1; bRet && (i < nCount); i++)
            bRet = ((ScAutoFormatData*)pItems[i])->Save(rStream);
        rStream.Flush();

        aMedium.Commit();
    }
    bSaveLater = sal_False;
    return bRet;
}

sal_uInt16 ScAutoFormat::FindIndexPerName( const String& rName ) const
{
    String              aName;

    for( sal_uInt16 i=0; i<nCount ; i++ )
    {
        ScAutoFormatData* pItem = (ScAutoFormatData*)pItems[i];
        pItem->GetName( aName );

        if( aName == rName )
            return i;
    }

    return 0;
}




