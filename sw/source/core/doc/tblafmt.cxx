/*************************************************************************
 *
 *  $RCSfile: tblafmt.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-06 13:06:03 $
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


#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER


#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SFXDOCFILE_HXX
#include <sfx2/docfile.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _ZFORLIST_HXX //autogen
#include <svtools/zforlist.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _SFX_INIMGR_HXX
#include <sfx2/inimgr.hxx>
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX
#include <svx/dialmgr.hxx>
#endif
#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif

#define READ_OLDVERS        // erstmal noch alte Versionen lesen

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _TBLAFMT_HXX
#include <tblafmt.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif


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

// aktuelle Version
const USHORT AUTOFORMAT_ID          = AUTOFORMAT_ID_552;
const USHORT AUTOFORMAT_DATA_ID     = AUTOFORMAT_DATA_ID_552;


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
    USHORT nCrossedOutVersion;
    USHORT nContourVersion;
    USHORT nShadowedVersion;
    USHORT nColorVersion;
    USHORT nBoxVersion;
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
    nCrossedOutVersion(0),
    nContourVersion(0),
    nShadowedVersion(0),
    nColorVersion(0),
    nBoxVersion(0),
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
    rStream >> nCrossedOutVersion;
    rStream >> nContourVersion;
    rStream >> nShadowedVersion;
    rStream >> nColorVersion;
    rStream >> nBoxVersion;
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
    aRotateMode( SVX_ROTATE_MODE_STANDARD, 0 )
{
    eSysLanguage = eNumFmtLanguage = ::GetSystemLanguage();
    aBox.SetDistance( 55 );
}


SwBoxAutoFmt::SwBoxAutoFmt( const SwBoxAutoFmt& rNew )
    : aFont( rNew.aFont ),
    aHeight( rNew.aHeight ),
    aWeight( rNew.aWeight ),
    aPosture( rNew.aPosture ),
    aUnderline( rNew.aUnderline ),
    aCrossedOut( rNew.aCrossedOut ),
    aContour( rNew.aContour ),
    aShadowed( rNew.aShadowed ),
    aColor( rNew.aColor ),
    aAdjust( rNew.aAdjust ),
    aBox( rNew.aBox ),
    aBackground( rNew.aBackground ),
    aHorJustify( rNew.aHorJustify ),
    aVerJustify( rNew.aVerJustify ),
    aOrientation( rNew.aOrientation ),
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

#if USED

int SwBoxAutoFmt::operator==( const SwBoxAutoFmt& rCmp ) const
{
    return  aFont == rCmp.aFont &&
            aHeight == rCmp.aHeight &&
            aWeight == rCmp.aWeight &&
            aPosture == rCmp.aPosture &&
            aUnderline == rCmp.aUnderline &&
            aCrossedOut == rCmp.aCrossedOut &&
            aContour == rCmp.aContour &&
            aShadowed == rCmp.aShadowed &&
            aColor == rCmp.aColor &&
            aAdjust == rCmp.aAdjust &&
            aBox == rCmp.aBox &&
            aBackground == rCmp.aBackground;
}
#endif


SwBoxAutoFmt& SwBoxAutoFmt::operator=( const SwBoxAutoFmt& rNew )
{
    aFont = rNew.aFont;
    aHeight = rNew.aHeight;
    aWeight = rNew.aWeight;
    aPosture = rNew.aPosture;
    aUnderline = rNew.aUnderline;
    aCrossedOut = rNew.aCrossedOut;
    aContour = rNew.aContour;
    aShadowed = rNew.aShadowed;
    aColor = rNew.aColor;
    SetAdjust( rNew.aAdjust );
    aBox = rNew.aBox;
    aBackground = rNew.aBackground;

    aHorJustify = rNew.aHorJustify;
    aVerJustify = rNew.aVerJustify;
    aOrientation = rNew.aOrientation;
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
    READ( aFont,        SvxFontItem         , rVersions.nFontVersion)

    if( rStream.GetStreamCharSet() == aFont.GetCharSet() )
        aFont.GetCharSet() = ::gsl_getSystemTextEncoding();

    READ( aHeight,      SvxFontHeightItem   , rVersions.nFontHeightVersion)
    READ( aWeight,      SvxWeightItem       , rVersions.nWeightVersion)
    READ( aPosture,     SvxPostureItem      , rVersions.nPostureVersion)
    READ( aUnderline,   SvxUnderlineItem    , rVersions.nUnderlineVersion)
    READ( aCrossedOut,  SvxCrossedOutItem   , rVersions.nCrossedOutVersion)
    READ( aContour,     SvxContourItem      , rVersions.nContourVersion)
    READ( aShadowed,    SvxShadowedItem     , rVersions.nShadowedVersion)
    READ( aColor,       SvxColorItem        , rVersions.nColorVersion)

    READ( aBox,         SvxBoxItem          , rVersions.nBoxVersion)
    READ( aBackground,  SvxBrushItem        , rVersions.nBrushVersion)

    pNew = aAdjust.Create(rStream, rVersions.nAdjustVersion );
    SetAdjust( *(SvxAdjustItem*)pNew );
    delete pNew;

    READ( aHorJustify,  SvxHorJustifyItem   , rVersions.nHorJustifyVersion)
    READ( aVerJustify,  SvxVerJustifyItem   , rVersions.nVerJustifyVersion)
    READ( aOrientation, SvxOrientationItem  , rVersions.nOrientationVersion)
    READ( aMargin,      SvxMarginItem       , rVersions.nMarginVersion)

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
        rStream.ReadByteString( sNumFmtString, rStream.GetStreamCharSet() )
                >> eSys >> eLge;
        eSysLanguage = (LanguageType) eSys;
        eNumFmtLanguage = (LanguageType) eLge;
        if ( eSysLanguage == LANGUAGE_SYSTEM )      // von alten Versionen (Calc)
            eSysLanguage = ::GetSystemLanguage();
    }

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
    aFont.Store( rStream, aFont.GetVersion(SOFFICE_FILEFORMAT_40)  );
    aHeight.Store( rStream, aHeight.GetVersion(SOFFICE_FILEFORMAT_40) );
    aWeight.Store( rStream, aWeight.GetVersion(SOFFICE_FILEFORMAT_40) );
    aPosture.Store( rStream, aPosture.GetVersion(SOFFICE_FILEFORMAT_40) );
    aUnderline.Store( rStream, aUnderline.GetVersion(SOFFICE_FILEFORMAT_40) );
    aCrossedOut.Store( rStream, aCrossedOut.GetVersion(SOFFICE_FILEFORMAT_40) );
    aContour.Store( rStream, aContour.GetVersion(SOFFICE_FILEFORMAT_40) );
    aShadowed.Store( rStream, aShadowed.GetVersion(SOFFICE_FILEFORMAT_40) );
    aColor.Store( rStream, aColor.GetVersion(SOFFICE_FILEFORMAT_40) );
    aBox.Store( rStream, aBox.GetVersion(SOFFICE_FILEFORMAT_40) );
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

    rStream.WriteByteString( sNumFmtString, rStream.GetStreamCharSet() )
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
    rStream << aCrossedOut.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aContour.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aShadowed.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aColor.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aBox.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aBackground.GetVersion( SOFFICE_FILEFORMAT_40 );

    rStream << aAdjust.GetVersion( SOFFICE_FILEFORMAT_40 );

    rStream << aHorJustify.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aVerJustify.GetVersion( SOFFICE_FILEFORMAT_40 );
    rStream << aOrientation.GetVersion( SOFFICE_FILEFORMAT_40 );
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
    ASSERT( 0 <= nPos && nPos < 16, "falscher Bereich" );

    SwBoxAutoFmt* pFmt = aBoxAutoFmt[ nPos ];
    if( pFmt )      // ist gesetzt -> kopieren
        *aBoxAutoFmt[ nPos ] = rNew;
    else            // sonst neu setzen
        aBoxAutoFmt[ nPos ] = new SwBoxAutoFmt( rNew );
}


const SwBoxAutoFmt& SwTableAutoFmt::GetBoxFmt( BYTE nPos ) const
{
    ASSERT( 0 <= nPos && nPos < 16, "falscher Bereich" );

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
    ASSERT( 0 <= nPos && nPos < 16, "falscher Bereich" );

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
        pFmt->SetUnderline( (SvxUnderlineItem&)rSet.Get( RES_CHRATR_UNDERLINE ) );
        pFmt->SetCrossedOut( (SvxCrossedOutItem&)rSet.Get( RES_CHRATR_CROSSEDOUT ) );
        pFmt->SetContour( (SvxContourItem&)rSet.Get( RES_CHRATR_CONTOUR ) );
        pFmt->SetShadowed( (SvxShadowedItem&)rSet.Get( RES_CHRATR_SHADOWED ) );
        pFmt->SetColor( (SvxColorItem&)rSet.Get( RES_CHRATR_COLOR ) );
        pFmt->SetAdjust( (SvxAdjustItem&)rSet.Get( RES_PARATR_ADJUST ) );
    }
    if( UPDATE_BOX & eFlags )
    {
        pFmt->SetBox( (SvxBoxItem&)rSet.Get( RES_BOX ) );
        pFmt->SetBackground( (SvxBrushItem&)rSet.Get( RES_BACKGROUND ) );

        const SwTblBoxNumFormat* pNumFmtItem;
        const SvNumberformat* pNumFormat = 0;
        if( SFX_ITEM_SET == rSet.GetItemState( RES_BOXATR_FORMAT, TRUE,
            (const SfxPoolItem**)&pNumFmtItem ) && pNFmtr &&
            0 != (pNumFormat = pNFmtr->GetEntry( pNumFmtItem->GetValue() )) )
            pFmt->SetValueFormat( ((SvNumberformat*)pNumFormat)->GetFormatstring(),
                                    pNumFormat->GetLanguage(),
                                    ::GetSystemLanguage() );
        else
        {
            // defaulten
            pFmt->SetValueFormat( aEmptyStr, LANGUAGE_SYSTEM,
                                    ::GetSystemLanguage() );
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
            rSet.Put( rChg.GetUnderline() );
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
            rSet.Put( rChg.GetBox() );
        if( IsBackground() )
            rSet.Put( rChg.GetBackground() );

        if( IsValueFormat() && pNFmtr )
        {
            String sFmt; LanguageType eLng, eSys;
            rChg.GetValueFormat( sFmt, eLng, eSys );
            if( sFmt.Len() )
            {
                ULONG nKey = 0;
                if ( eLng == LANGUAGE_SYSTEM && eSys != ::GetSystemLanguage() )
                {
                    //  #53381# wenn System beim Speichern etwas anderes war,
                    //  muss konvertiert werden (geht nur mit eingebauten Formaten)
                    ULONG nOrig = pNFmtr->GetEntryKey( sFmt, eSys );
                    if ( nOrig != NUMBERFORMAT_ENTRY_NOT_FOUND )
                        nKey = pNFmtr->GetFormatForLanguageIfBuiltIn( nOrig,
                                        ::GetSystemLanguage() );
                }
                else    // sonst einfach suchen oder anlegen
                {
                    nKey = pNFmtr->GetEntryKey( sFmt, eLng );
                    if( NUMBERFORMAT_ENTRY_NOT_FOUND == nKey )
                    {
                        xub_StrLen nCheckPos;
                        short nType;
                        pNFmtr->PutEntry( sFmt, nCheckPos, nType, nKey, eLng );
                    }
                }
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
        rStream.ReadByteString( aName, rStream.GetStreamCharSet() );
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
    rStream.WriteByteString( aName, rStream.GetStreamCharSet() );
    if( USHRT_MAX == nStrResId && 0 != SFX_INIMANAGER()->Get(
            SFX_GROUP_WORKINGSET_IMPL, String::CreateFromAscii(
            RTL_CONSTASCII_STRINGPARAM("SaveTableAutoFmtNameId" ))).ToInt32())
    {
        // check Name for ResId
        for( USHORT nId = RID_SVXSTR_TBLAFMT_BEGIN;
                    RID_SVXSTR_TBLAFMT_END > nId; ++nId )
        {
            String s( SVX_RES( nId ) );
            if( s == aName )
            {
                SwTableAutoFmt* pThis = (SwTableAutoFmt*)this;
                pThis->nStrResId = nId - RID_SVXSTR_TBLAFMT_BEGIN;
                break;
            }
        }
    }

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
                            SwDoc::GetPoolNm( RES_POOLCOLL_STANDARD, sNm ) );

    SwBoxAutoFmt aNew;

    BYTE i;

    Color aColor( COL_BLUE );
    SvxBrushItem aBrushItem( aColor );
    aNew.SetBackground( aBrushItem );
    aNew.SetColor( Color( COL_WHITE ));

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
    aNew.SetColor( aColor );
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


    SvxBoxItem aBox;
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
    if( SFX_INIMANAGER()->SearchFile( sNm, SFX_KEY_USERCONFIG_PATH ))
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
    String sNm( URIHelper::SmartRelToAbs( aPathOpt.GetUserConfigPath() ));
    sNm += INET_PATH_TOKEN;
    sNm.AppendAscii( RTL_CONSTASCII_STRINGPARAM( sAutoTblFmtName ));
    SfxMedium aStream(sNm, STREAM_STD_WRITE, TRUE );
    return Save( *aStream.GetOutStream() );
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
                    ASSERT( !this, "Der Header enthaelt mehr/neuere Daten" );
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
                << (BYTE)GetStoreCharSet( ::gsl_getSystemTextEncoding(),
                                              SOFFICE_FILEFORMAT_40 );
//              << (BYTE)4      // Anzahl von Zeichen des Headers incl. diesem
//              << (BYTE)::GetSystemCharSet()
//              << (UNIT16)SOFFICE_FILEFORMAT_NOW;
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



