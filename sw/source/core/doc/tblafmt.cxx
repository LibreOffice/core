/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sw.hxx"


#include <tools/resid.hxx>
#include <tools/stream.hxx>
#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/docfile.hxx>
#include <svl/urihelper.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#define READ_OLDVERS        // erstmal noch alte Versionen lesen
#include <swtypes.hxx>
#include <doc.hxx>
#include <poolfmt.hxx>
#include <tblafmt.hxx>
#include <cellatr.hxx>
#include <SwStyleNameMapper.hxx>

// bis SO5PF
const USHORT AUTOFORMAT_ID_X        = 9501;
const USHORT AUTOFORMAT_ID_358      = 9601;
const USHORT AUTOFORMAT_DATA_ID_X   = 9502;

// ab SO5
//! in nachfolgenden Versionen muss der Betrag dieser IDs groesser sein
const USHORT AUTOFORMAT_ID_504      = 9801;
const USHORT AUTOFORMAT_DATA_ID_504 = 9802;

const USHORT AUTOFORMAT_ID_552      = 9901;
const USHORT AUTOFORMAT_DATA_ID_552 = 9902;

// --- from 641 on: CJK and CTL font settings
const USHORT AUTOFORMAT_ID_641      = 10001;
const USHORT AUTOFORMAT_DATA_ID_641 = 10002;

// --- from 680/dr14 on: diagonal frame lines
const USHORT AUTOFORMAT_ID_680DR14      = 10011;
const USHORT AUTOFORMAT_DATA_ID_680DR14 = 10012;

// --- from 680/dr25 on: store strings as UTF-8
const USHORT AUTOFORMAT_ID_680DR25      = 10021;
const USHORT AUTOFORMAT_DATA_ID_680DR25 = 10022;

// --- from DEV300/overline2 on: overline
const USHORT AUTOFORMAT_ID_300OVRLN      = 10031;
const USHORT AUTOFORMAT_DATA_ID_300OVRLN = 10032;

// current version
const USHORT AUTOFORMAT_ID          = AUTOFORMAT_ID_300OVRLN;
const USHORT AUTOFORMAT_DATA_ID     = AUTOFORMAT_DATA_ID_300OVRLN;


#ifdef READ_OLDVERS
const USHORT AUTOFORMAT_OLD_ID      = 8201;
const USHORT AUTOFORMAT_OLD_ID1     = 8301;
const USHORT AUTOFORMAT_OLD_DATA_ID = 8202;
#endif


SwBoxAutoFmt* SwTableAutoFmt::pDfltBoxAutoFmt = 0;

#define sAutoTblFmtName "autotbl.fmt"

// SwTable Auto-Format-Tabelle
SV_IMPL_PTRARR( _SwTableAutoFmtTbl, SwTableAutoFmt* )


//  Struct mit Versionsnummern der Items

struct SwAfVersions
{
public:
    USHORT nFontVersion;
    USHORT nFontHeightVersion;
    USHORT nWeightVersion;
    USHORT nPostureVersion;
    USHORT nUnderlineVersion;
    USHORT nOverlineVersion;
    USHORT nCrossedOutVersion;
    USHORT nContourVersion;
    USHORT nShadowedVersion;
    USHORT nColorVersion;
    USHORT nBoxVersion;
    USHORT nLineVersion;
    USHORT nBrushVersion;

    USHORT nAdjustVersion;

    USHORT nHorJustifyVersion;
    USHORT nVerJustifyVersion;
    USHORT nOrientationVersion;
    USHORT nMarginVersion;
    USHORT nBoolVersion;
    USHORT nInt32Version;
    USHORT nRotateModeVersion;

    USHORT nNumFmtVersion;

    SwAfVersions();
    void Load( SvStream& rStream, USHORT nVer );
};

SwAfVersions::SwAfVersions() :
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

void SwAfVersions::Load( SvStream& rStream, USHORT nVer )
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

//  ---------------------------------------------------------------------------

SwBoxAutoFmt::SwBoxAutoFmt()
    : aFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT ) ),
    aHeight( 240, 100, RES_CHRATR_FONTSIZE ),
    aWeight( WEIGHT_NORMAL, RES_CHRATR_WEIGHT ),
    aPosture( ITALIC_NONE, RES_CHRATR_POSTURE ),

    aCJKFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_CJK_FONT ) ),
    aCJKHeight( 240, 100, RES_CHRATR_CJK_FONTSIZE ),
    aCJKWeight( WEIGHT_NORMAL, RES_CHRATR_CJK_WEIGHT ),
    aCJKPosture( ITALIC_NONE, RES_CHRATR_CJK_POSTURE ),

    aCTLFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_CTL_FONT ) ),
    aCTLHeight( 240, 100, RES_CHRATR_CTL_FONTSIZE ),
    aCTLWeight( WEIGHT_NORMAL, RES_CHRATR_CTL_WEIGHT ),
    aCTLPosture( ITALIC_NONE, RES_CHRATR_CTL_POSTURE ),

    aUnderline( UNDERLINE_NONE, RES_CHRATR_UNDERLINE ),
    aOverline( UNDERLINE_NONE, RES_CHRATR_OVERLINE ),
    aCrossedOut( STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT ),
    aContour( sal_False, RES_CHRATR_CONTOUR ),
    aShadowed( sal_False, RES_CHRATR_SHADOWED ),
    aColor( RES_CHRATR_COLOR ),
    aBox( RES_BOX ),
    aTLBR( 0 ),
    aBLTR( 0 ),
    aBackground( RES_BACKGROUND ),
    aAdjust( SVX_ADJUST_LEFT, RES_PARATR_ADJUST ),
    aHorJustify( SVX_HOR_JUSTIFY_STANDARD, 0),
    aVerJustify( SVX_VER_JUSTIFY_STANDARD, 0),
    aStacked( 0 ),
    aMargin( 0 ),
    aLinebreak( 0 ),
    aRotateAngle( 0 ),

// FIXME - add attribute IDs for the diagonal line items
//    aTLBR( RES_... ),
//    aBLTR( RES_... ),
    aRotateMode( SVX_ROTATE_MODE_STANDARD, 0 )
{
    eSysLanguage = eNumFmtLanguage = static_cast<LanguageType>(::GetAppLanguage());
    aBox.SetDistance( 55 );
}


SwBoxAutoFmt::SwBoxAutoFmt( const SwBoxAutoFmt& rNew )
    : aFont( rNew.aFont ),
    aHeight( rNew.aHeight ),
    aWeight( rNew.aWeight ),
    aPosture( rNew.aPosture ),
    aCJKFont( rNew.aCJKFont ),
    aCJKHeight( rNew.aCJKHeight ),
    aCJKWeight( rNew.aCJKWeight ),
    aCJKPosture( rNew.aCJKPosture ),
    aCTLFont( rNew.aCTLFont ),
    aCTLHeight( rNew.aCTLHeight ),
    aCTLWeight( rNew.aCTLWeight ),
    aCTLPosture( rNew.aCTLPosture ),
    aUnderline( rNew.aUnderline ),
    aOverline( rNew.aOverline ),
    aCrossedOut( rNew.aCrossedOut ),
    aContour( rNew.aContour ),
    aShadowed( rNew.aShadowed ),
    aColor( rNew.aColor ),
    aBox( rNew.aBox ),
    aTLBR( rNew.aTLBR ),
    aBLTR( rNew.aBLTR ),
    aBackground( rNew.aBackground ),
    aAdjust( rNew.aAdjust ),
    aHorJustify( rNew.aHorJustify ),
    aVerJustify( rNew.aVerJustify ),
    aStacked( rNew.aStacked ),
    aMargin( rNew.aMargin ),
    aLinebreak( rNew.aLinebreak ),
    aRotateAngle( rNew.aRotateAngle ),
    aRotateMode( rNew.aRotateMode ),
    sNumFmtString( rNew.sNumFmtString ),
    eSysLanguage( rNew.eSysLanguage ),
    eNumFmtLanguage( rNew.eNumFmtLanguage )
{
}


SwBoxAutoFmt::~SwBoxAutoFmt()
{
}

SwBoxAutoFmt& SwBoxAutoFmt::operator=( const SwBoxAutoFmt& rNew )
{
    aFont = rNew.aFont;
    aHeight = rNew.aHeight;
    aWeight = rNew.aWeight;
    aPosture = rNew.aPosture;
    aCJKFont = rNew.aCJKFont;
    aCJKHeight = rNew.aCJKHeight;
    aCJKWeight = rNew.aCJKWeight;
    aCJKPosture = rNew.aCJKPosture;
    aCTLFont = rNew.aCTLFont;
    aCTLHeight = rNew.aCTLHeight;
    aCTLWeight = rNew.aCTLWeight;
    aCTLPosture = rNew.aCTLPosture;
    aUnderline = rNew.aUnderline;
    aOverline = rNew.aOverline;
    aCrossedOut = rNew.aCrossedOut;
    aContour = rNew.aContour;
    aShadowed = rNew.aShadowed;
    aColor = rNew.aColor;
    SetAdjust( rNew.aAdjust );
    aBox = rNew.aBox;
    aTLBR = rNew.aTLBR;
    aBLTR = rNew.aBLTR;
    aBackground = rNew.aBackground;

    aHorJustify = rNew.aHorJustify;
    aVerJustify = rNew.aVerJustify;
    aStacked.SetValue( rNew.aStacked.GetValue() );
    aMargin = rNew.aMargin;
    aLinebreak.SetValue( rNew.aLinebreak.GetValue() );
    aRotateAngle.SetValue( rNew.aRotateAngle.GetValue() );
    aRotateMode.SetValue( rNew.aRotateMode.GetValue() );

    sNumFmtString = rNew.sNumFmtString;
    eSysLanguage = rNew.eSysLanguage;
    eNumFmtLanguage = rNew.eNumFmtLanguage;

    return *this;
}


#define READ( aItem, aItemType, nVers )\
    pNew = aItem.Create(rStream, nVers ); \
    aItem = *(aItemType*)pNew; \
    delete pNew;

BOOL SwBoxAutoFmt::Load( SvStream& rStream, const SwAfVersions& rVersions, USHORT nVer )
{
    SfxPoolItem* pNew;
    SvxOrientationItem aOrientation( SVX_ORIENTATION_STANDARD, 0);

    READ( aFont,        SvxFontItem            , rVersions.nFontVersion)

    if( rStream.GetStreamCharSet() == aFont.GetCharSet() )
        aFont.GetCharSet() = ::gsl_getSystemTextEncoding();

    READ( aHeight,      SvxFontHeightItem  , rVersions.nFontHeightVersion)
    READ( aWeight,      SvxWeightItem      , rVersions.nWeightVersion)
    READ( aPosture,     SvxPostureItem     , rVersions.nPostureVersion)
    // --- from 641 on: CJK and CTL font settings
    if( AUTOFORMAT_DATA_ID_641 <= nVer )
    {
        READ( aCJKFont,                        SvxFontItem         , rVersions.nFontVersion)
        READ( aCJKHeight,       SvxFontHeightItem   , rVersions.nFontHeightVersion)
        READ( aCJKWeight,     SvxWeightItem       , rVersions.nWeightVersion)
        READ( aCJKPosture,   SvxPostureItem      , rVersions.nPostureVersion)
        READ( aCTLFont,                        SvxFontItem         , rVersions.nFontVersion)
        READ( aCTLHeight,        SvxFontHeightItem   , rVersions.nFontHeightVersion)
        READ( aCTLWeight,       SvxWeightItem       , rVersions.nWeightVersion)
        READ( aCTLPosture,   SvxPostureItem      , rVersions.nPostureVersion)
    }
    READ( aUnderline,   SvxUnderlineItem   , rVersions.nUnderlineVersion)
    if( nVer >= AUTOFORMAT_DATA_ID_300OVRLN )
    {
        READ( aOverline,       SvxOverlineItem     , rVersions.nOverlineVersion)
    }
    READ( aCrossedOut,  SvxCrossedOutItem  , rVersions.nCrossedOutVersion)
    READ( aContour,     SvxContourItem     , rVersions.nContourVersion)
    READ( aShadowed,    SvxShadowedItem       , rVersions.nShadowedVersion)
    READ( aColor,       SvxColorItem       , rVersions.nColorVersion)

    READ( aBox,         SvxBoxItem         , rVersions.nBoxVersion)

    // --- from 680/dr14 on: diagonal frame lines
    if( nVer >= AUTOFORMAT_DATA_ID_680DR14 )
    {
        READ( aTLBR, SvxLineItem, rVersions.nLineVersion)
        READ( aBLTR, SvxLineItem, rVersions.nLineVersion)
    }

    READ( aBackground,  SvxBrushItem        , rVersions.nBrushVersion)

    pNew = aAdjust.Create(rStream, rVersions.nAdjustVersion );
    SetAdjust( *(SvxAdjustItem*)pNew );
    delete pNew;

    READ( aHorJustify,  SvxHorJustifyItem , rVersions.nHorJustifyVersion)
    READ( aVerJustify,  SvxVerJustifyItem   , rVersions.nVerJustifyVersion)
    READ( aOrientation, SvxOrientationItem  , rVersions.nOrientationVersion)
    READ( aMargin, SvxMarginItem       , rVersions.nMarginVersion)

    pNew = aLinebreak.Create(rStream, rVersions.nBoolVersion );
    aLinebreak.SetValue( ((SfxBoolItem*)pNew)->GetValue() );
    delete pNew;

    if ( nVer >= AUTOFORMAT_DATA_ID_504 )
    {
        pNew = aRotateAngle.Create( rStream, rVersions.nInt32Version );
        aRotateAngle.SetValue( ((SfxInt32Item*)pNew)->GetValue() );
        delete pNew;
        pNew = aRotateMode.Create( rStream, rVersions.nRotateModeVersion );
        aRotateMode.SetValue( ((SvxRotateModeItem*)pNew)->GetValue() );
        delete pNew;
    }

    if( 0 == rVersions.nNumFmtVersion )
    {
        USHORT eSys, eLge;
        // --- from 680/dr25 on: store strings as UTF-8
        CharSet eCharSet = (nVer >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        rStream.ReadByteString( sNumFmtString, eCharSet )
                >> eSys >> eLge;
        eSysLanguage = (LanguageType) eSys;
        eNumFmtLanguage = (LanguageType) eLge;
        if ( eSysLanguage == LANGUAGE_SYSTEM )      // von alten Versionen (Calc)
            eSysLanguage = static_cast<LanguageType>(::GetAppLanguage());
    }

    aStacked.SetValue( aOrientation.IsStacked() );
    aRotateAngle.SetValue( aOrientation.GetRotation( aRotateAngle.GetValue() ) );

    return 0 == rStream.GetError();
}

#ifdef READ_OLDVERS

BOOL SwBoxAutoFmt::LoadOld( SvStream& rStream, USHORT aLoadVer[] )
{
    SfxPoolItem* pNew;
    READ( aFont,        SvxFontItem         , 0)

    if( rStream.GetStreamCharSet() == aFont.GetCharSet() )
        aFont.GetCharSet() = ::gsl_getSystemTextEncoding();

    READ( aHeight,      SvxFontHeightItem   , 1)
    READ( aWeight,      SvxWeightItem       , 2)
    READ( aPosture,     SvxPostureItem      , 3)
    READ( aUnderline,   SvxUnderlineItem    , 4)
    READ( aCrossedOut,  SvxCrossedOutItem   , 5)
    READ( aContour,     SvxContourItem      , 6)
    READ( aShadowed,    SvxShadowedItem     , 7)
    READ( aColor,       SvxColorItem        , 8)

    pNew = aAdjust.Create(rStream, aLoadVer[ 9 ] );
    SetAdjust( *(SvxAdjustItem*)pNew );
    delete pNew;

    READ( aBox,         SvxBoxItem          , 10)
    READ( aBackground,  SvxBrushItem        , 11)

    return 0 == rStream.GetError();
}

#endif


BOOL SwBoxAutoFmt::Save( SvStream& rStream ) const
{
    SvxOrientationItem aOrientation( aRotateAngle.GetValue(), aStacked.GetValue(), 0 );

    aFont.Store( rStream, aFont.GetVersion(SOFFICE_FILEFORMAT_40)  );
    aHeight.Store( rStream, aHeight.GetVersion(SOFFICE_FILEFORMAT_40) );
    aWeight.Store( rStream, aWeight.GetVersion(SOFFICE_FILEFORMAT_40) );
    aPosture.Store( rStream, aPosture.GetVersion(SOFFICE_FILEFORMAT_40) );
    aCJKFont.Store( rStream, aCJKFont.GetVersion(SOFFICE_FILEFORMAT_40)  );
    aCJKHeight.Store( rStream, aCJKHeight.GetVersion(SOFFICE_FILEFORMAT_40) );
    aCJKWeight.Store( rStream, aCJKWeight.GetVersion(SOFFICE_FILEFORMAT_40) );
    aCJKPosture.Store( rStream, aCJKPosture.GetVersion(SOFFICE_FILEFORMAT_40) );
    aCTLFont.Store( rStream, aCTLFont.GetVersion(SOFFICE_FILEFORMAT_40)  );
    aCTLHeight.Store( rStream, aCTLHeight.GetVersion(SOFFICE_FILEFORMAT_40) );
    aCTLWeight.Store( rStream, aCTLWeight.GetVersion(SOFFICE_FILEFORMAT_40) );
    aCTLPosture.Store( rStream, aCTLPosture.GetVersion(SOFFICE_FILEFORMAT_40) );
    aUnderline.Store( rStream, aUnderline.GetVersion(SOFFICE_FILEFORMAT_40) );
    aOverline.Store( rStream, aOverline.GetVersion(SOFFICE_FILEFORMAT_40) );
    aCrossedOut.Store( rStream, aCrossedOut.GetVersion(SOFFICE_FILEFORMAT_40) );
    aContour.Store( rStream, aContour.GetVersion(SOFFICE_FILEFORMAT_40) );
    aShadowed.Store( rStream, aShadowed.GetVersion(SOFFICE_FILEFORMAT_40) );
    aColor.Store( rStream, aColor.GetVersion(SOFFICE_FILEFORMAT_40) );
    aBox.Store( rStream, aBox.GetVersion(SOFFICE_FILEFORMAT_40) );
    aTLBR.Store( rStream, aTLBR.GetVersion(SOFFICE_FILEFORMAT_40) );
    aBLTR.Store( rStream, aBLTR.GetVersion(SOFFICE_FILEFORMAT_40) );
    aBackground.Store( rStream, aBackground.GetVersion(SOFFICE_FILEFORMAT_40) );

    aAdjust.Store( rStream, aAdjust.GetVersion(SOFFICE_FILEFORMAT_40) );

    aHorJustify.Store( rStream, aHorJustify.GetVersion(SOFFICE_FILEFORMAT_40) );
    aVerJustify.Store( rStream, aVerJustify.GetVersion(SOFFICE_FILEFORMAT_40) );
    aOrientation.Store( rStream, aOrientation.GetVersion(SOFFICE_FILEFORMAT_40) );
    aMargin.Store( rStream, aMargin.GetVersion(SOFFICE_FILEFORMAT_40) );
    aLinebreak.Store( rStream, aLinebreak.GetVersion(SOFFICE_FILEFORMAT_40) );
    // Calc Rotation ab SO5
    aRotateAngle.Store( rStream, aRotateAngle.GetVersion(SOFFICE_FILEFORMAT_40) );
    aRotateMode.Store( rStream, aRotateMode.GetVersion(SOFFICE_FILEFORMAT_40) );

    // --- from 680/dr25 on: store strings as UTF-8
    rStream.WriteByteString( sNumFmtString, RTL_TEXTENCODING_UTF8 )
        << (USHORT)eSysLanguage << (USHORT)eNumFmtLanguage;

    return 0 == rStream.GetError();
}


BOOL SwBoxAutoFmt::SaveVerionNo( SvStream& rStream ) const
{
    rStream << aFont.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aHeight.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aWeight.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aPosture.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aUnderline.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aOverline.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aCrossedOut.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aContour.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aShadowed.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aColor.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aBox.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aTLBR.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aBackground.GetVersion( SOFFICE_FILEFORMAT_40 );

    rStream << aAdjust.GetVersion( SOFFICE_FILEFORMAT_40 );

    rStream << aHorJustify.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aVerJustify.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << SvxOrientationItem(SVX_ORIENTATION_STANDARD, 0).GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aMargin.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aLinebreak.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aRotateAngle.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aRotateMode.GetVersion( SOFFICE_FILEFORMAT_40 );

    rStream << (USHORT)0;       // NumberFormat

    return 0 == rStream.GetError();
}

/*  */


SwTableAutoFmt::SwTableAutoFmt( const String& rName )
    : aName( rName ), nStrResId( USHRT_MAX )
{
    bInclFont = TRUE;
    bInclJustify = TRUE;
    bInclFrame = TRUE;
    bInclBackground = TRUE;
    bInclValueFormat = TRUE;
    bInclWidthHeight = TRUE;

    memset( aBoxAutoFmt, 0, sizeof( aBoxAutoFmt ) );
}


SwTableAutoFmt::SwTableAutoFmt( const SwTableAutoFmt& rNew )
{
    for( BYTE n = 0; n < 16; ++n )
        aBoxAutoFmt[ n ] = 0;
    *this = rNew;
}

SwTableAutoFmt& SwTableAutoFmt::operator=( const SwTableAutoFmt& rNew )
{
    for( BYTE n = 0; n < 16; ++n )
    {
        if( aBoxAutoFmt[ n ] )
            delete aBoxAutoFmt[ n ];

        SwBoxAutoFmt* pFmt = rNew.aBoxAutoFmt[ n ];
        if( pFmt )      // ist gesetzt -> kopieren
            aBoxAutoFmt[ n ] = new SwBoxAutoFmt( *pFmt );
        else            // sonst default
            aBoxAutoFmt[ n ] = 0;
    }

    aName = rNew.aName;
    nStrResId = rNew.nStrResId;
    bInclFont = rNew.bInclFont;
    bInclJustify = rNew.bInclJustify;
    bInclFrame = rNew.bInclFrame;
    bInclBackground = rNew.bInclBackground;
    bInclValueFormat = rNew.bInclValueFormat;
    bInclWidthHeight = rNew.bInclWidthHeight;

    return *this;
}


SwTableAutoFmt::~SwTableAutoFmt()
{
    SwBoxAutoFmt** ppFmt = aBoxAutoFmt;
    for( BYTE n = 0; n < 16; ++n, ++ppFmt )
        if( *ppFmt )
            delete *ppFmt;
}


void SwTableAutoFmt::SetBoxFmt( const SwBoxAutoFmt& rNew, BYTE nPos )
{
    OSL_ENSURE( nPos < 16, "falscher Bereich" );

    SwBoxAutoFmt* pFmt = aBoxAutoFmt[ nPos ];
    if( pFmt )      // ist gesetzt -> kopieren
        *aBoxAutoFmt[ nPos ] = rNew;
    else            // sonst neu setzen
        aBoxAutoFmt[ nPos ] = new SwBoxAutoFmt( rNew );
}


const SwBoxAutoFmt& SwTableAutoFmt::GetBoxFmt( BYTE nPos ) const
{
    OSL_ENSURE( nPos < 16, "falscher Bereich" );

    SwBoxAutoFmt* pFmt = aBoxAutoFmt[ nPos ];
    if( pFmt )      // ist gesetzt -> kopieren
        return *pFmt;
    else            // sonst den default returnen
    {
        // falls noch nicht vorhanden:
        if( !pDfltBoxAutoFmt )
            pDfltBoxAutoFmt = new SwBoxAutoFmt;
        return *pDfltBoxAutoFmt;
    }
}



SwBoxAutoFmt& SwTableAutoFmt::UpdateFromSet( BYTE nPos,
                                            const SfxItemSet& rSet,
                                            UpdateFlags eFlags,
                                            SvNumberFormatter* pNFmtr )
{
    OSL_ENSURE( nPos < 16, "falscher Bereich" );

    SwBoxAutoFmt* pFmt = aBoxAutoFmt[ nPos ];
    if( !pFmt )     // ist gesetzt -> kopieren
    {
        pFmt = new SwBoxAutoFmt;
        aBoxAutoFmt[ nPos ] = pFmt;
    }

    if( UPDATE_CHAR & eFlags )
    {
        pFmt->SetFont( (SvxFontItem&)rSet.Get( RES_CHRATR_FONT ) );
        pFmt->SetHeight( (SvxFontHeightItem&)rSet.Get( RES_CHRATR_FONTSIZE ) );
        pFmt->SetWeight( (SvxWeightItem&)rSet.Get( RES_CHRATR_WEIGHT ) );
        pFmt->SetPosture( (SvxPostureItem&)rSet.Get( RES_CHRATR_POSTURE ) );
        pFmt->SetCJKFont( (SvxFontItem&)rSet.Get( RES_CHRATR_CJK_FONT ) );
        pFmt->SetCJKHeight( (SvxFontHeightItem&)rSet.Get( RES_CHRATR_CJK_FONTSIZE ) );
        pFmt->SetCJKWeight( (SvxWeightItem&)rSet.Get( RES_CHRATR_CJK_WEIGHT ) );
        pFmt->SetCJKPosture( (SvxPostureItem&)rSet.Get( RES_CHRATR_CJK_POSTURE ) );
        pFmt->SetCTLFont( (SvxFontItem&)rSet.Get( RES_CHRATR_CTL_FONT ) );
        pFmt->SetCTLHeight( (SvxFontHeightItem&)rSet.Get( RES_CHRATR_CTL_FONTSIZE ) );
        pFmt->SetCTLWeight( (SvxWeightItem&)rSet.Get( RES_CHRATR_CTL_WEIGHT ) );
        pFmt->SetCTLPosture( (SvxPostureItem&)rSet.Get( RES_CHRATR_CTL_POSTURE ) );
        pFmt->SetUnderline( (SvxUnderlineItem&)rSet.Get( RES_CHRATR_UNDERLINE ) );
        pFmt->SetOverline( (SvxOverlineItem&)rSet.Get( RES_CHRATR_OVERLINE ) );
        pFmt->SetCrossedOut( (SvxCrossedOutItem&)rSet.Get( RES_CHRATR_CROSSEDOUT ) );
        pFmt->SetContour( (SvxContourItem&)rSet.Get( RES_CHRATR_CONTOUR ) );
        pFmt->SetShadowed( (SvxShadowedItem&)rSet.Get( RES_CHRATR_SHADOWED ) );
        pFmt->SetColor( (SvxColorItem&)rSet.Get( RES_CHRATR_COLOR ) );
        pFmt->SetAdjust( (SvxAdjustItem&)rSet.Get( RES_PARATR_ADJUST ) );
    }
    if( UPDATE_BOX & eFlags )
    {
        pFmt->SetBox( (SvxBoxItem&)rSet.Get( RES_BOX ) );
// FIXME - add attribute IDs for the diagonal line items
//        pFmt->SetTLBR( (SvxLineItem&)rSet.Get( RES_... ) );
//        pFmt->SetBLTR( (SvxLineItem&)rSet.Get( RES_... ) );
        pFmt->SetBackground( (SvxBrushItem&)rSet.Get( RES_BACKGROUND ) );

        const SwTblBoxNumFormat* pNumFmtItem;
        const SvNumberformat* pNumFormat = 0;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMAT, TRUE,
            (const SfxPoolItem**)&pNumFmtItem ) && pNFmtr &&
            0 != (pNumFormat = pNFmtr->GetEntry( pNumFmtItem->GetValue() )) )
            pFmt->SetValueFormat( ((SvNumberformat*)pNumFormat)->GetFormatstring(),
                                    pNumFormat->GetLanguage(),
                                    static_cast<LanguageType>(::GetAppLanguage()));
        else
        {
            // defaulten
            pFmt->SetValueFormat( aEmptyStr, LANGUAGE_SYSTEM,
                                  static_cast<LanguageType>(::GetAppLanguage() ));
        }
    }
    // den Rest koennen wir nicht, StarCalc spezifisch

    return *pFmt;
}


void SwTableAutoFmt::UpdateToSet( BYTE nPos, SfxItemSet& rSet,
                UpdateFlags eFlags, SvNumberFormatter* pNFmtr ) const
{
    const SwBoxAutoFmt& rChg = GetBoxFmt( nPos );

    if( UPDATE_CHAR & eFlags )
    {
        if( IsFont() )
        {
            rSet.Put( rChg.GetFont() );
            rSet.Put( rChg.GetHeight() );
            rSet.Put( rChg.GetWeight() );
            rSet.Put( rChg.GetPosture() );
            // do not insert empty CJK font
            const SvxFontItem& rCJKFont = rChg.GetCJKFont();
            if( rCJKFont.GetStyleName().Len() )
            {
                rSet.Put( rChg.GetCJKFont() );
                rSet.Put( rChg.GetCJKHeight() );
                rSet.Put( rChg.GetCJKWeight() );
                rSet.Put( rChg.GetCJKPosture() );
            }
            else
            {
                rSet.Put( rChg.GetHeight(), RES_CHRATR_CJK_FONTSIZE );
                rSet.Put( rChg.GetWeight(), RES_CHRATR_CJK_WEIGHT );
                rSet.Put( rChg.GetPosture(), RES_CHRATR_CJK_POSTURE );
            }
            // do not insert empty CTL font
            const SvxFontItem& rCTLFont = rChg.GetCTLFont();
            if( rCTLFont.GetStyleName().Len() )
            {
                rSet.Put( rChg.GetCTLFont() );
                rSet.Put( rChg.GetCTLHeight() );
                rSet.Put( rChg.GetCTLWeight() );
                rSet.Put( rChg.GetCTLPosture() );
            }
            else
            {
                rSet.Put( rChg.GetHeight(), RES_CHRATR_CTL_FONTSIZE );
                rSet.Put( rChg.GetWeight(), RES_CHRATR_CTL_WEIGHT );
                rSet.Put( rChg.GetPosture(), RES_CHRATR_CTL_POSTURE );
            }
            rSet.Put( rChg.GetUnderline() );
            rSet.Put( rChg.GetOverline() );
            rSet.Put( rChg.GetCrossedOut() );
            rSet.Put( rChg.GetContour() );
            rSet.Put( rChg.GetShadowed() );
            rSet.Put( rChg.GetColor() );
        }
        if( IsJustify() )
            rSet.Put( rChg.GetAdjust() );
    }

    if( UPDATE_BOX & eFlags )
    {
        if( IsFrame() )
        {
            rSet.Put( rChg.GetBox() );
// FIXME - uncomment the lines to put the diagonal line items
//            rSet.Put( rChg.GetTLBR() );
//            rSet.Put( rChg.GetBLTR() );
        }
        if( IsBackground() )
            rSet.Put( rChg.GetBackground() );

        if( IsValueFormat() && pNFmtr )
        {
            String sFmt; LanguageType eLng, eSys;
            rChg.GetValueFormat( sFmt, eLng, eSys );
            if( sFmt.Len() )
            {
                short nType;
                BOOL bNew;
                xub_StrLen nCheckPos;
                sal_uInt32 nKey = pNFmtr->GetIndexPuttingAndConverting( sFmt, eLng,
                        eSys, nType, bNew, nCheckPos);
                rSet.Put( SwTblBoxNumFormat( nKey ));
            }
            else
                rSet.ClearItem( RES_BOXATR_FORMAT );
        }
    }

    // den Rest koennen wir nicht, StarCalc spezifisch
}


BOOL SwTableAutoFmt::Load( SvStream& rStream, const SwAfVersions& rVersions )
{
    BOOL    bRet = TRUE;
    USHORT  nVal = 0;
    rStream >> nVal;
    bRet = 0 == rStream.GetError();

    if( bRet && (nVal == AUTOFORMAT_DATA_ID_X ||
            (AUTOFORMAT_DATA_ID_504 <= nVal && nVal <= AUTOFORMAT_DATA_ID)) )
    {
        BOOL b;
        // --- from 680/dr25 on: store strings as UTF-8
        CharSet eCharSet = (nVal >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        rStream.ReadByteString( aName, eCharSet );
        if( AUTOFORMAT_DATA_ID_552 <= nVal )
        {
            rStream >> nStrResId;
            USHORT nId = RID_SVXSTR_TBLAFMT_BEGIN + nStrResId;
            if( RID_SVXSTR_TBLAFMT_BEGIN <= nId &&
                nId < RID_SVXSTR_TBLAFMT_END )
            {
                aName = SVX_RESSTR( nId );
            }
            else
                nStrResId = USHRT_MAX;
        }
        rStream >> b; bInclFont = b;
        rStream >> b; bInclJustify = b;
        rStream >> b; bInclFrame = b;
        rStream >> b; bInclBackground = b;
        rStream >> b; bInclValueFormat = b;
        rStream >> b; bInclWidthHeight = b;

        bRet = 0 == rStream.GetError();

        for( BYTE i = 0; i < 16; ++i )
        {
            SwBoxAutoFmt* pFmt = new SwBoxAutoFmt;
            bRet = pFmt->Load( rStream, rVersions, nVal );
            if( bRet )
                aBoxAutoFmt[ i ] = pFmt;
            else
            {
                delete pFmt;
                break;
            }
        }
    }
    return bRet;
}

#ifdef READ_OLDVERS

BOOL SwTableAutoFmt::LoadOld( SvStream& rStream, USHORT aLoadVer[] )
{
    BOOL    bRet = TRUE;
    USHORT  nVal = 0;
    rStream >> nVal;
    bRet = 0 == rStream.GetError();

    if( bRet && ( AUTOFORMAT_OLD_DATA_ID == nVal ))
    {
        BOOL b;
        rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
        rStream >> b; bInclFont = b;
        rStream >> b; bInclJustify = b;
        rStream >> b; bInclFrame = b;
        rStream >> b; bInclBackground = b;
        bRet = (rStream.GetError() == 0);

        for( int i = 0; i < 16; i++)
        {
            SwBoxAutoFmt* pFmt = new SwBoxAutoFmt;
            bRet = pFmt->LoadOld( rStream, aLoadVer );
            if( bRet )
                aBoxAutoFmt[ i ] = pFmt;
            else
            {
                delete pFmt;
                break;
            }
        }
    }
    return bRet;
}
#endif


BOOL SwTableAutoFmt::Save( SvStream& rStream ) const
{
    USHORT nVal = AUTOFORMAT_DATA_ID;
    BOOL b;
    rStream << nVal;
    // --- from 680/dr25 on: store strings as UTF-8
    rStream.WriteByteString( aName, RTL_TEXTENCODING_UTF8 );
    rStream << nStrResId;
    rStream << ( b = bInclFont );
    rStream << ( b = bInclJustify );
    rStream << ( b = bInclFrame );
    rStream << ( b = bInclBackground );
    rStream << ( b = bInclValueFormat );
    rStream << ( b = bInclWidthHeight );

    BOOL bRet = 0 == rStream.GetError();

    for( int i = 0; bRet && i < 16; ++i )
    {
        SwBoxAutoFmt* pFmt = aBoxAutoFmt[ i ];
        if( !pFmt )     // nicht gesetzt -> default schreiben
        {
            // falls noch nicht vorhanden:
            if( !pDfltBoxAutoFmt )
                pDfltBoxAutoFmt = new SwBoxAutoFmt;
            pFmt = pDfltBoxAutoFmt;
        }
        bRet = pFmt->Save( rStream );
    }
    return bRet;
}


SwTableAutoFmtTbl::SwTableAutoFmtTbl()
{
    String sNm;
    SwTableAutoFmt* pNew = new SwTableAutoFmt(
                            SwStyleNameMapper::GetUIName( RES_POOLCOLL_STANDARD, sNm ) );

    SwBoxAutoFmt aNew;

    BYTE i;

    Color aColor( COL_BLUE );
    SvxBrushItem aBrushItem( aColor, RES_BACKGROUND );
    aNew.SetBackground( aBrushItem );
    aNew.SetColor( SvxColorItem(Color( COL_WHITE ), RES_CHRATR_COLOR) );

    for( i = 0; i < 4; ++i )
        pNew->SetBoxFmt( aNew, i );

    // 70% Grau
    aBrushItem.SetColor( RGB_COLORDATA( 0x4d, 0x4d, 0x4d ) );
    aNew.SetBackground( aBrushItem );
    for( i = 4; i <= 12; i += 4 )
        pNew->SetBoxFmt( aNew, i );

    // 20% Grau
    aBrushItem.SetColor( RGB_COLORDATA( 0xcc, 0xcc, 0xcc ) );
    aNew.SetBackground( aBrushItem );
    aColor.SetColor( COL_BLACK );
    aNew.SetColor( SvxColorItem( aColor, RES_CHRATR_COLOR) );
    for( i = 7; i <= 15; i += 4 )
        pNew->SetBoxFmt( aNew, i );
    for( i = 13; i <= 14; ++i )
        pNew->SetBoxFmt( aNew, i );

    aBrushItem.SetColor( Color( COL_WHITE ) );
    aNew.SetBackground( aBrushItem );
    for( i = 5; i <= 6; ++i )
        pNew->SetBoxFmt( aNew, i );
    for( i = 9; i <= 10; ++i )
        pNew->SetBoxFmt( aNew, i );


    SvxBoxItem aBox( RES_BOX );
    aBox.SetDistance( 55 );
    SvxBorderLine aLn( &aColor, DEF_LINE_WIDTH_0 );
    aBox.SetLine( &aLn, BOX_LINE_LEFT );
    aBox.SetLine( &aLn, BOX_LINE_BOTTOM );

    for( i = 0; i <= 15; ++i )
    {
        aBox.SetLine( i <= 3 ? &aLn : 0, BOX_LINE_TOP );
        aBox.SetLine( (3 == ( i & 3 )) ? &aLn : 0, BOX_LINE_RIGHT );
        ((SwBoxAutoFmt&)pNew->GetBoxFmt( i )).SetBox( aBox );
    }

    Insert( pNew, Count() );
}

BOOL SwTableAutoFmtTbl::Load()
{
    BOOL bRet = FALSE;
    String sNm( String::CreateFromAscii(
                RTL_CONSTASCII_STRINGPARAM( sAutoTblFmtName )));
    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sNm, SvtPathOptions::PATH_USERCONFIG ))
    {
        SfxMedium aStream( sNm, STREAM_STD_READ, TRUE );
        bRet = Load( *aStream.GetInStream() );
    }
    else
        bRet = FALSE;
    return bRet;
}

BOOL SwTableAutoFmtTbl::Save() const
{
    SvtPathOptions aPathOpt;
    String sNm( aPathOpt.GetUserConfigPath() );
    sNm += INET_PATH_TOKEN;
    sNm.AppendAscii( RTL_CONSTASCII_STRINGPARAM( sAutoTblFmtName ));
    SfxMedium aStream(sNm, STREAM_STD_WRITE, TRUE );
    return Save( *aStream.GetOutStream() ) && aStream.Commit();
}

BOOL SwTableAutoFmtTbl::Load( SvStream& rStream )
{
    BOOL bRet = 0 == rStream.GetError();
    if (bRet)
    {
        // Achtung hier muss ein allgemeiner Header gelesen werden
        USHORT nVal = 0;
        rStream >> nVal;
        bRet = 0 == rStream.GetError();

        if( bRet )
        {
            SwAfVersions aVersions;

            if( nVal == AUTOFORMAT_ID_358 ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                UINT16 nFileVers = SOFFICE_FILEFORMAT_40;
                BYTE nChrSet, nCnt;
                long nPos = rStream.Tell();
                rStream >> nCnt >> nChrSet;
//              if( 4 <= nCnt )
//                  rStream >> nFileVers;
                if( rStream.Tell() != ULONG(nPos + nCnt) )
                {
                    OSL_ENSURE( !this, "Der Header enthaelt mehr/neuere Daten" );
                    rStream.Seek( nPos + nCnt );
                }
                rStream.SetStreamCharSet( (CharSet)nChrSet );
                rStream.SetVersion( nFileVers );
            }

            if( nVal == AUTOFORMAT_ID_358 || nVal == AUTOFORMAT_ID_X ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                aVersions.Load( rStream, nVal );        // Item-Versionen

                SwTableAutoFmt* pNew;
                USHORT nAnz = 0;
                rStream >> nAnz;

                bRet = 0 == rStream.GetError();

                for( USHORT i = 0; i < nAnz; ++i )
                {
                    pNew = new SwTableAutoFmt( aEmptyStr );
                    bRet = pNew->Load( rStream, aVersions );
                    if( bRet )
                    {
                        Insert( pNew, Count() );
                    }
                    else
                    {
                        delete pNew;
                        break;
                    }
                }
            }
#ifdef READ_OLDVERS
            else if( AUTOFORMAT_OLD_ID == nVal || AUTOFORMAT_OLD_ID1 == nVal )
            {
                SwTableAutoFmt* pNew;
                USHORT nAnz = 0;
                rStream >> nAnz;

                USHORT aArr[ 12 ];
                memset( aArr, 0, 12 * sizeof( USHORT ) );
                if( AUTOFORMAT_OLD_ID1 == nVal )
                    for( USHORT n = 0; n < 12; ++n )
                        rStream >> aArr[ n ];

                bRet = 0 == rStream.GetError();

                for( USHORT i = 0; i < nAnz; ++i )
                {
                    pNew = new SwTableAutoFmt( aEmptyStr );
                    bRet = pNew->LoadOld( rStream, aArr );
                    if( bRet )
                    {
                        Insert( pNew, Count() );
                    }
                    else
                    {
                        delete pNew;
                        break;
                    }
                }
            }
#endif
        }
    }
    return bRet;
}


BOOL SwTableAutoFmtTbl::Save( SvStream& rStream ) const
{
    BOOL bRet = 0 == rStream.GetError();
    if (bRet)
    {
        rStream.SetVersion( SOFFICE_FILEFORMAT_40 );

        // Achtung hier muss ein allgemeiner Header gespeichert werden
        USHORT nVal = AUTOFORMAT_ID;
        rStream << nVal
                << (BYTE)2      // Anzahl von Zeichen des Headers incl. diesem
                << (BYTE)GetStoreCharSet( ::gsl_getSystemTextEncoding() );

        bRet = 0 == rStream.GetError();

        //-----------------------------------------------------------
        // die VersionsNummer fuer alle Attribute schreiben
        (*this)[ 0 ]->GetBoxFmt( 0 ).SaveVerionNo( rStream );

        rStream << (USHORT)(Count() - 1);
        bRet = 0 == rStream.GetError();

        for( USHORT i = 1; bRet && i < Count(); ++i )
        {
            SwTableAutoFmt* pFmt = (*this)[ i ];
            bRet = pFmt->Save( rStream );
        }
    }
    rStream.Flush();
    return bRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
