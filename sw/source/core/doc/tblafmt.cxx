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

#include <tools/resid.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/docfile.hxx>
#include <svl/urihelper.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <swtable.hxx>
#include <swtblfmt.hxx>
#include <com/sun/star/text/VertOrientation.hpp>
#include <swtypes.hxx>
#include <doc.hxx>
#include <poolfmt.hxx>
#include <tblafmt.hxx>
#include <cellatr.hxx>
#include <SwStyleNameMapper.hxx>
#include <hintids.hxx>
#include <fmtornt.hxx>
#include <editsh.hxx>

#include <boost/noncopyable.hpp>

#include <memory>
#include <vector>

/*
 * XXX: BIG RED NOTICE! Changes MUST be binary file format compatible and MUST
 * be synchronized with Calc's ScAutoFormat sc/source/core/tool/autoform.cxx
 */

using ::editeng::SvxBorderLine;

// until SO5PF
const sal_uInt16 AUTOFORMAT_ID_X        = 9501;
const sal_uInt16 AUTOFORMAT_ID_358      = 9601;
const sal_uInt16 AUTOFORMAT_DATA_ID_X   = 9502;

// from SO5
//! In follow-up versions these IDs' values need to increase
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

// --- from DEV300/overline2 on: overline
const sal_uInt16 AUTOFORMAT_ID_300OVRLN      = 10031;
const sal_uInt16 AUTOFORMAT_DATA_ID_300OVRLN = 10032;

// --- Bug fix to fdo#31005: Table Autoformats does not save/apply all properties (Writer and Calc)
const sal_uInt16 AUTOFORMAT_ID_31005      = 10041;
const sal_uInt16 AUTOFORMAT_DATA_ID_31005 = 10042;

// current version
const sal_uInt16 AUTOFORMAT_ID          = AUTOFORMAT_ID_31005;
const sal_uInt16 AUTOFORMAT_DATA_ID     = AUTOFORMAT_DATA_ID_31005;
const sal_uInt16 AUTOFORMAT_FILE_VERSION= SOFFICE_FILEFORMAT_50;

SwBoxAutoFormat* SwTableAutoFormat::pDfltBoxAutoFormat = nullptr;

#define AUTOTABLE_FORMAT_NAME "autotbl.fmt"

namespace
{
    /// Begins a writer-specific data block. Call before serializing any writer-specific properties.
    sal_uInt64 BeginSwBlock(SvStream& rStream)
    {
        // We need to write down the offset of the end of the writer-specific data, so that
        // calc can skip it. We'll only have that value after writing the data, so we
        // write a placeholder value first, write the data, then jump back and write the
        // real offset.

        // Note that we explicitly use sal_uInt64 instead of sal_Size (which can be 32
        // or 64 depending on platform) to ensure 64-bit portability on this front. I don't
        // actually know if autotbl.fmt as a whole is portable, since that requires all serialization
        // logic to be written with portability in mind.
        sal_uInt64 whereToWriteEndOfSwBlock = rStream.Tell();

        sal_uInt64 endOfSwBlock = 0;
        rStream.WriteUInt64( endOfSwBlock );

        return whereToWriteEndOfSwBlock;
    }

    /// Ends a writer-specific data block. Call after serializing writer-specific properties.
    /// Closes a corresponding BeginSwBlock call.
    void EndSwBlock(SvStream& rStream, sal_uInt64 whereToWriteEndOfSwBlock)
    {
        sal_uInt64 endOfSwBlock = rStream.Tell();
        rStream.Seek(whereToWriteEndOfSwBlock);
        rStream.WriteUInt64( endOfSwBlock );
        rStream.Seek(endOfSwBlock);
    }

    /**
    Helper class for writer-specific blocks. Begins a writer-specific block on construction,
    and closes it on destruction.

    See also: BeginSwBlock and EndSwBlock.
    */
    class WriterSpecificAutoFormatBlock : ::boost::noncopyable
    {
    public:
        explicit WriterSpecificAutoFormatBlock(SvStream &rStream) : _rStream(rStream)
        {
            _whereToWriteEndOfBlock = BeginSwBlock(rStream);
        }

        ~WriterSpecificAutoFormatBlock()
        {
            EndSwBlock(_rStream, _whereToWriteEndOfBlock);
        }

    private:
        SvStream &_rStream;
        sal_uInt64 _whereToWriteEndOfBlock;
    };

    /// Checks whether a writer-specific block exists (i.e. size is not zero)
    bool WriterSpecificBlockExists(SvStream &stream)
    {
        sal_uInt64 endOfSwBlock = 0;
        stream.ReadUInt64( endOfSwBlock );

        // end-of-block pointing to itself indicates a zero-size block.
        return endOfSwBlock != stream.Tell();
    }
}

// Struct with version numbers of the Items

struct SwAfVersions
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
    sal_uInt16 m_nTextOrientationVersion;
    sal_uInt16 m_nVerticalAlignmentVersion;

    sal_uInt16 nHorJustifyVersion;
    sal_uInt16 nVerJustifyVersion;
    sal_uInt16 nOrientationVersion;
    sal_uInt16 nMarginVersion;
    sal_uInt16 nBoolVersion;
    sal_uInt16 nInt32Version;
    sal_uInt16 nRotateModeVersion;

    sal_uInt16 nNumFormatVersion;

    SwAfVersions();
    void Load( SvStream& rStream, sal_uInt16 nVer );
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
    m_nTextOrientationVersion(0),
    m_nVerticalAlignmentVersion(0),
    nHorJustifyVersion(0),
    nVerJustifyVersion(0),
    nOrientationVersion(0),
    nMarginVersion(0),
    nBoolVersion(0),
    nInt32Version(0),
    nRotateModeVersion(0),
    nNumFormatVersion(0)
{
}

void SwAfVersions::Load( SvStream& rStream, sal_uInt16 nVer )
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
    if (nVer >= AUTOFORMAT_ID_31005 && WriterSpecificBlockExists(rStream))
    {
        rStream.ReadUInt16( m_nTextOrientationVersion );
        rStream.ReadUInt16( m_nVerticalAlignmentVersion );
    }

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
    rStream.ReadUInt16( nNumFormatVersion );
}

SwBoxAutoFormat::SwBoxAutoFormat()
    : m_aFont( *static_cast<const SvxFontItem*>(GetDfltAttr( RES_CHRATR_FONT )) ),
    m_aHeight( 240, 100, RES_CHRATR_FONTSIZE ),
    m_aWeight( WEIGHT_NORMAL, RES_CHRATR_WEIGHT ),
    m_aPosture( ITALIC_NONE, RES_CHRATR_POSTURE ),

    m_aCJKFont( *static_cast<const SvxFontItem*>(GetDfltAttr( RES_CHRATR_CJK_FONT )) ),
    m_aCJKHeight( 240, 100, RES_CHRATR_CJK_FONTSIZE ),
    m_aCJKWeight( WEIGHT_NORMAL, RES_CHRATR_CJK_WEIGHT ),
    m_aCJKPosture( ITALIC_NONE, RES_CHRATR_CJK_POSTURE ),

    m_aCTLFont( *static_cast<const SvxFontItem*>(GetDfltAttr( RES_CHRATR_CTL_FONT )) ),
    m_aCTLHeight( 240, 100, RES_CHRATR_CTL_FONTSIZE ),
    m_aCTLWeight( WEIGHT_NORMAL, RES_CHRATR_CTL_WEIGHT ),
    m_aCTLPosture( ITALIC_NONE, RES_CHRATR_CTL_POSTURE ),

    m_aUnderline( UNDERLINE_NONE, RES_CHRATR_UNDERLINE ),
    m_aOverline( UNDERLINE_NONE, RES_CHRATR_OVERLINE ),
    m_aCrossedOut( STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT ),
    m_aContour( false, RES_CHRATR_CONTOUR ),
    m_aShadowed( false, RES_CHRATR_SHADOWED ),
    m_aColor( RES_CHRATR_COLOR ),
    m_aBox( RES_BOX ),
    m_aTLBR( 0 ),
    m_aBLTR( 0 ),
    m_aBackground( RES_BACKGROUND ),
    m_aAdjust( SVX_ADJUST_LEFT, RES_PARATR_ADJUST ),
    m_aTextOrientation(FRMDIR_ENVIRONMENT, RES_FRAMEDIR),
    m_aVerticalAlignment(0, css::text::VertOrientation::NONE, css::text::RelOrientation::FRAME),
    m_aHorJustify( SVX_HOR_JUSTIFY_STANDARD, 0),
    m_aVerJustify( SVX_VER_JUSTIFY_STANDARD, 0),
    m_aStacked( 0 ),
    m_aMargin( 0 ),
    m_aLinebreak( 0 ),
    m_aRotateAngle( 0 ),

// FIXME - add attribute IDs for the diagonal line items
//    aTLBR( RES_... ),
//    aBLTR( RES_... ),
    m_aRotateMode( SVX_ROTATE_MODE_STANDARD, 0 )
{
    m_eSysLanguage = m_eNumFormatLanguage = ::GetAppLanguage();
    m_aBox.SetDistance( 55 );
}

SwBoxAutoFormat::SwBoxAutoFormat( const SwBoxAutoFormat& rNew )
    : m_aFont( rNew.m_aFont ),
    m_aHeight( rNew.m_aHeight ),
    m_aWeight( rNew.m_aWeight ),
    m_aPosture( rNew.m_aPosture ),
    m_aCJKFont( rNew.m_aCJKFont ),
    m_aCJKHeight( rNew.m_aCJKHeight ),
    m_aCJKWeight( rNew.m_aCJKWeight ),
    m_aCJKPosture( rNew.m_aCJKPosture ),
    m_aCTLFont( rNew.m_aCTLFont ),
    m_aCTLHeight( rNew.m_aCTLHeight ),
    m_aCTLWeight( rNew.m_aCTLWeight ),
    m_aCTLPosture( rNew.m_aCTLPosture ),
    m_aUnderline( rNew.m_aUnderline ),
    m_aOverline( rNew.m_aOverline ),
    m_aCrossedOut( rNew.m_aCrossedOut ),
    m_aContour( rNew.m_aContour ),
    m_aShadowed( rNew.m_aShadowed ),
    m_aColor( rNew.m_aColor ),
    m_aBox( rNew.m_aBox ),
    m_aTLBR( rNew.m_aTLBR ),
    m_aBLTR( rNew.m_aBLTR ),
    m_aBackground( rNew.m_aBackground ),
    m_aAdjust( rNew.m_aAdjust ),
    m_aTextOrientation(rNew.m_aTextOrientation),
    m_aVerticalAlignment(rNew.m_aVerticalAlignment),
    m_aHorJustify( rNew.m_aHorJustify ),
    m_aVerJustify( rNew.m_aVerJustify ),
    m_aStacked( rNew.m_aStacked ),
    m_aMargin( rNew.m_aMargin ),
    m_aLinebreak( rNew.m_aLinebreak ),
    m_aRotateAngle( rNew.m_aRotateAngle ),
    m_aRotateMode( rNew.m_aRotateMode ),
    m_sNumFormatString( rNew.m_sNumFormatString ),
    m_eSysLanguage( rNew.m_eSysLanguage ),
    m_eNumFormatLanguage( rNew.m_eNumFormatLanguage )
{
}

SwBoxAutoFormat::~SwBoxAutoFormat()
{
}

SwBoxAutoFormat& SwBoxAutoFormat::operator=( const SwBoxAutoFormat& rNew )
{
    m_aFont = rNew.m_aFont;
    m_aHeight = rNew.m_aHeight;
    m_aWeight = rNew.m_aWeight;
    m_aPosture = rNew.m_aPosture;
    m_aCJKFont = rNew.m_aCJKFont;
    m_aCJKHeight = rNew.m_aCJKHeight;
    m_aCJKWeight = rNew.m_aCJKWeight;
    m_aCJKPosture = rNew.m_aCJKPosture;
    m_aCTLFont = rNew.m_aCTLFont;
    m_aCTLHeight = rNew.m_aCTLHeight;
    m_aCTLWeight = rNew.m_aCTLWeight;
    m_aCTLPosture = rNew.m_aCTLPosture;
    m_aUnderline = rNew.m_aUnderline;
    m_aOverline = rNew.m_aOverline;
    m_aCrossedOut = rNew.m_aCrossedOut;
    m_aContour = rNew.m_aContour;
    m_aShadowed = rNew.m_aShadowed;
    m_aColor = rNew.m_aColor;
    SetAdjust( rNew.m_aAdjust );
    m_aTextOrientation = rNew.m_aTextOrientation;
    m_aVerticalAlignment = rNew.m_aVerticalAlignment;
    m_aBox = rNew.m_aBox;
    m_aTLBR = rNew.m_aTLBR;
    m_aBLTR = rNew.m_aBLTR;
    m_aBackground = rNew.m_aBackground;

    m_aHorJustify = rNew.m_aHorJustify;
    m_aVerJustify = rNew.m_aVerJustify;
    m_aStacked.SetValue( rNew.m_aStacked.GetValue() );
    m_aMargin = rNew.m_aMargin;
    m_aLinebreak.SetValue( rNew.m_aLinebreak.GetValue() );
    m_aRotateAngle.SetValue( rNew.m_aRotateAngle.GetValue() );
    m_aRotateMode.SetValue( rNew.m_aRotateMode.GetValue() );

    m_sNumFormatString = rNew.m_sNumFormatString;
    m_eSysLanguage = rNew.m_eSysLanguage;
    m_eNumFormatLanguage = rNew.m_eNumFormatLanguage;

    return *this;
}

#define READ( aItem, aItemType, nVers )\
    pNew = aItem.Create(rStream, nVers ); \
    aItem = *static_cast<aItemType*>(pNew); \
    delete pNew;

bool SwBoxAutoFormat::Load( SvStream& rStream, const SwAfVersions& rVersions, sal_uInt16 nVer )
{
    SfxPoolItem* pNew;
    SvxOrientationItem aOrientation( SVX_ORIENTATION_STANDARD, 0);

    READ( m_aFont,        SvxFontItem            , rVersions.nFontVersion)

    if( rStream.GetStreamCharSet() == m_aFont.GetCharSet() )
        m_aFont.SetCharSet(::osl_getThreadTextEncoding());

    READ( m_aHeight,      SvxFontHeightItem  , rVersions.nFontHeightVersion)
    READ( m_aWeight,      SvxWeightItem      , rVersions.nWeightVersion)
    READ( m_aPosture,     SvxPostureItem     , rVersions.nPostureVersion)
    // --- from 641 on: CJK and CTL font settings
    if( AUTOFORMAT_DATA_ID_641 <= nVer )
    {
        READ( m_aCJKFont,                        SvxFontItem         , rVersions.nFontVersion)
        READ( m_aCJKHeight,       SvxFontHeightItem   , rVersions.nFontHeightVersion)
        READ( m_aCJKWeight,     SvxWeightItem       , rVersions.nWeightVersion)
        READ( m_aCJKPosture,   SvxPostureItem      , rVersions.nPostureVersion)
        READ( m_aCTLFont,                        SvxFontItem         , rVersions.nFontVersion)
        READ( m_aCTLHeight,        SvxFontHeightItem   , rVersions.nFontHeightVersion)
        READ( m_aCTLWeight,       SvxWeightItem       , rVersions.nWeightVersion)
        READ( m_aCTLPosture,   SvxPostureItem      , rVersions.nPostureVersion)
    }
    READ( m_aUnderline,   SvxUnderlineItem   , rVersions.nUnderlineVersion)
    if( nVer >= AUTOFORMAT_DATA_ID_300OVRLN )
    {
        READ( m_aOverline,       SvxOverlineItem     , rVersions.nOverlineVersion)
    }
    READ( m_aCrossedOut,  SvxCrossedOutItem  , rVersions.nCrossedOutVersion)
    READ( m_aContour,     SvxContourItem     , rVersions.nContourVersion)
    READ( m_aShadowed,    SvxShadowedItem       , rVersions.nShadowedVersion)
    READ( m_aColor,       SvxColorItem       , rVersions.nColorVersion)

    READ( m_aBox,         SvxBoxItem         , rVersions.nBoxVersion)

    // --- from 680/dr14 on: diagonal frame lines
    if( nVer >= AUTOFORMAT_DATA_ID_680DR14 )
    {
        READ( m_aTLBR, SvxLineItem, rVersions.nLineVersion)
        READ( m_aBLTR, SvxLineItem, rVersions.nLineVersion)
    }

    READ( m_aBackground,  SvxBrushItem        , rVersions.nBrushVersion)

    pNew = m_aAdjust.Create(rStream, rVersions.nAdjustVersion );
    SetAdjust( *static_cast<SvxAdjustItem*>(pNew) );
    delete pNew;

    if (nVer >= AUTOFORMAT_DATA_ID_31005 && WriterSpecificBlockExists(rStream))
    {
        READ(m_aTextOrientation, SvxFrameDirectionItem, rVersions.m_nTextOrientationVersion);
        READ(m_aVerticalAlignment, SwFormatVertOrient, rVersions.m_nVerticalAlignmentVersion);
    }

    READ( m_aHorJustify,  SvxHorJustifyItem , rVersions.nHorJustifyVersion)
    READ( m_aVerJustify,  SvxVerJustifyItem   , rVersions.nVerJustifyVersion)

    READ( aOrientation, SvxOrientationItem  , rVersions.nOrientationVersion)
    READ( m_aMargin, SvxMarginItem       , rVersions.nMarginVersion)

    pNew = m_aLinebreak.Create(rStream, rVersions.nBoolVersion );
    m_aLinebreak.SetValue( static_cast<SfxBoolItem*>(pNew)->GetValue() );
    delete pNew;

    if ( nVer >= AUTOFORMAT_DATA_ID_504 )
    {
        pNew = m_aRotateAngle.Create( rStream, rVersions.nInt32Version );
        m_aRotateAngle.SetValue( static_cast<SfxInt32Item*>(pNew)->GetValue() );
        delete pNew;
        pNew = m_aRotateMode.Create( rStream, rVersions.nRotateModeVersion );
        m_aRotateMode.SetValue( static_cast<SvxRotateModeItem*>(pNew)->GetValue() );
        delete pNew;
    }

    if( 0 == rVersions.nNumFormatVersion )
    {
        sal_uInt16 eSys, eLge;
        // --- from 680/dr25 on: store strings as UTF-8
        rtl_TextEncoding eCharSet = (nVer >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        m_sNumFormatString = rStream.ReadUniOrByteString( eCharSet );
        rStream.ReadUInt16( eSys ).ReadUInt16( eLge );
        m_eSysLanguage = (LanguageType) eSys;
        m_eNumFormatLanguage = (LanguageType) eLge;
        if ( m_eSysLanguage == LANGUAGE_SYSTEM )      // from old versions (Calc)
            m_eSysLanguage = ::GetAppLanguage();
    }

    m_aStacked.SetValue( aOrientation.IsStacked() );
    m_aRotateAngle.SetValue( aOrientation.GetRotation( m_aRotateAngle.GetValue() ) );

    return 0 == rStream.GetError();
}

bool SwBoxAutoFormat::Save( SvStream& rStream, sal_uInt16 fileVersion ) const
{
    SvxOrientationItem aOrientation( m_aRotateAngle.GetValue(), m_aStacked.GetValue(), 0 );

    m_aFont.Store( rStream, m_aFont.GetVersion(fileVersion)  );
    m_aHeight.Store( rStream, m_aHeight.GetVersion(fileVersion) );
    m_aWeight.Store( rStream, m_aWeight.GetVersion(fileVersion) );
    m_aPosture.Store( rStream, m_aPosture.GetVersion(fileVersion) );
    m_aCJKFont.Store( rStream, m_aCJKFont.GetVersion(fileVersion)  );
    m_aCJKHeight.Store( rStream, m_aCJKHeight.GetVersion(fileVersion) );
    m_aCJKWeight.Store( rStream, m_aCJKWeight.GetVersion(fileVersion) );
    m_aCJKPosture.Store( rStream, m_aCJKPosture.GetVersion(fileVersion) );
    m_aCTLFont.Store( rStream, m_aCTLFont.GetVersion(fileVersion)  );
    m_aCTLHeight.Store( rStream, m_aCTLHeight.GetVersion(fileVersion) );
    m_aCTLWeight.Store( rStream, m_aCTLWeight.GetVersion(fileVersion) );
    m_aCTLPosture.Store( rStream, m_aCTLPosture.GetVersion(fileVersion) );
    m_aUnderline.Store( rStream, m_aUnderline.GetVersion(fileVersion) );
    m_aOverline.Store( rStream, m_aOverline.GetVersion(fileVersion) );
    m_aCrossedOut.Store( rStream, m_aCrossedOut.GetVersion(fileVersion) );
    m_aContour.Store( rStream, m_aContour.GetVersion(fileVersion) );
    m_aShadowed.Store( rStream, m_aShadowed.GetVersion(fileVersion) );
    m_aColor.Store( rStream, m_aColor.GetVersion(fileVersion) );
    m_aBox.Store( rStream, m_aBox.GetVersion(fileVersion) );
    m_aTLBR.Store( rStream, m_aTLBR.GetVersion(fileVersion) );
    m_aBLTR.Store( rStream, m_aBLTR.GetVersion(fileVersion) );
    m_aBackground.Store( rStream, m_aBackground.GetVersion(fileVersion) );

    m_aAdjust.Store( rStream, m_aAdjust.GetVersion(fileVersion) );
    if (fileVersion >= SOFFICE_FILEFORMAT_50)
    {
        WriterSpecificAutoFormatBlock block(rStream);

        m_aTextOrientation.Store(rStream, m_aTextOrientation.GetVersion(fileVersion));
        m_aVerticalAlignment.Store(rStream, m_aVerticalAlignment.GetVersion(fileVersion));
    }

    m_aHorJustify.Store( rStream, m_aHorJustify.GetVersion(fileVersion) );
    m_aVerJustify.Store( rStream, m_aVerJustify.GetVersion(fileVersion) );
    aOrientation.Store( rStream, aOrientation.GetVersion(fileVersion) );
    m_aMargin.Store( rStream, m_aMargin.GetVersion(fileVersion) );
    m_aLinebreak.Store( rStream, m_aLinebreak.GetVersion(fileVersion) );
    // Calc Rotation from SO5
    m_aRotateAngle.Store( rStream, m_aRotateAngle.GetVersion(fileVersion) );
    m_aRotateMode.Store( rStream, m_aRotateMode.GetVersion(fileVersion) );

    // --- from 680/dr25 on: store strings as UTF-8
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStream, m_sNumFormatString,
        RTL_TEXTENCODING_UTF8);
    rStream.WriteUInt16( m_eSysLanguage ).WriteUInt16( m_eNumFormatLanguage );

    return 0 == rStream.GetError();
}

bool SwBoxAutoFormat::SaveVersionNo( SvStream& rStream, sal_uInt16 fileVersion ) const
{
    rStream.WriteUInt16( m_aFont.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aHeight.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aWeight.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aPosture.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aUnderline.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aOverline.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aCrossedOut.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aContour.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aShadowed.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aColor.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aBox.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aTLBR.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aBackground.GetVersion( fileVersion ) );

    rStream.WriteUInt16( m_aAdjust.GetVersion( fileVersion ) );

    if (fileVersion >= SOFFICE_FILEFORMAT_50)
    {
        WriterSpecificAutoFormatBlock block(rStream);

        rStream.WriteUInt16( m_aTextOrientation.GetVersion(fileVersion) );
        rStream.WriteUInt16( m_aVerticalAlignment.GetVersion(fileVersion) );
    }

    rStream.WriteUInt16( m_aHorJustify.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aVerJustify.GetVersion( fileVersion ) );
    rStream.WriteUInt16( SvxOrientationItem(SVX_ORIENTATION_STANDARD, 0).GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aMargin.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aLinebreak.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aRotateAngle.GetVersion( fileVersion ) );
    rStream.WriteUInt16( m_aRotateMode.GetVersion( fileVersion ) );

    rStream.WriteUInt16( 0 );       // NumberFormat

    return 0 == rStream.GetError();
}

SwTableAutoFormat::SwTableAutoFormat( const OUString& rName )
    : m_aName( rName )
    , nStrResId( USHRT_MAX )
    , m_aBreak( SVX_BREAK_NONE, RES_BREAK )
    , m_aKeepWithNextPara( false, RES_KEEP )
    , m_aRepeatHeading( 0 )
    , m_bLayoutSplit( true )
    , m_bRowSplit( true )
    , m_bCollapsingBorders(true)
    , m_aShadow( RES_SHADOW )
{
    bInclFont = true;
    bInclJustify = true;
    bInclFrame = true;
    bInclBackground = true;
    bInclValueFormat = true;
    bInclWidthHeight = true;

    memset( aBoxAutoFormat, 0, sizeof( aBoxAutoFormat ) );
}

SwTableAutoFormat::SwTableAutoFormat( const SwTableAutoFormat& rNew )
    : m_aBreak( rNew.m_aBreak )
    , m_aKeepWithNextPara( false, RES_KEEP )
    , m_aShadow( RES_SHADOW )
{
    for( sal_uInt8 n = 0; n < 16; ++n )
        aBoxAutoFormat[ n ] = nullptr;
    *this = rNew;
}

SwTableAutoFormat& SwTableAutoFormat::operator=( const SwTableAutoFormat& rNew )
{
    if (&rNew == this)
        return *this;

    for( sal_uInt8 n = 0; n < 16; ++n )
    {
        if( aBoxAutoFormat[ n ] )
            delete aBoxAutoFormat[ n ];

        SwBoxAutoFormat* pFormat = rNew.aBoxAutoFormat[ n ];
        if( pFormat )      // if is set -> copy
            aBoxAutoFormat[ n ] = new SwBoxAutoFormat( *pFormat );
        else            // else default
            aBoxAutoFormat[ n ] = nullptr;
    }

    m_aName = rNew.m_aName;
    nStrResId = rNew.nStrResId;
    bInclFont = rNew.bInclFont;
    bInclJustify = rNew.bInclJustify;
    bInclFrame = rNew.bInclFrame;
    bInclBackground = rNew.bInclBackground;
    bInclValueFormat = rNew.bInclValueFormat;
    bInclWidthHeight = rNew.bInclWidthHeight;

    m_aBreak = rNew.m_aBreak;
    m_aPageDesc = rNew.m_aPageDesc;
    m_aKeepWithNextPara = rNew.m_aKeepWithNextPara;
    m_aRepeatHeading = rNew.m_aRepeatHeading;
    m_bLayoutSplit = rNew.m_bLayoutSplit;
    m_bRowSplit = rNew.m_bRowSplit;
    m_bCollapsingBorders = rNew.m_bCollapsingBorders;
    m_aShadow = rNew.m_aShadow;

    return *this;
}

SwTableAutoFormat::~SwTableAutoFormat()
{
    SwBoxAutoFormat** ppFormat = aBoxAutoFormat;
    for( sal_uInt8 n = 0; n < 16; ++n, ++ppFormat )
        if( *ppFormat )
            delete *ppFormat;
}

void SwTableAutoFormat::SetBoxFormat( const SwBoxAutoFormat& rNew, sal_uInt8 nPos )
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    SwBoxAutoFormat* pFormat = aBoxAutoFormat[ nPos ];
    if( pFormat )      // if is set -> copy
        *aBoxAutoFormat[ nPos ] = rNew;
    else            // else set anew
        aBoxAutoFormat[ nPos ] = new SwBoxAutoFormat( rNew );
}

const SwBoxAutoFormat& SwTableAutoFormat::GetBoxFormat( sal_uInt8 nPos ) const
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    SwBoxAutoFormat* pFormat = aBoxAutoFormat[ nPos ];
    if( pFormat )      // if is set -> copy
        return *pFormat;
    else            // else return the default
    {
        // If it doesn't exist yet:
        if( !pDfltBoxAutoFormat )
            pDfltBoxAutoFormat = new SwBoxAutoFormat;
        return *pDfltBoxAutoFormat;
    }
}

void SwTableAutoFormat::UpdateFromSet( sal_uInt8 nPos,
                                    const SfxItemSet& rSet,
                                    UpdateFlags eFlags,
                                    SvNumberFormatter* pNFormatr)
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    SwBoxAutoFormat* pFormat = aBoxAutoFormat[ nPos ];
    if( !pFormat )     // if is set -> copy
    {
        pFormat = new SwBoxAutoFormat;
        aBoxAutoFormat[ nPos ] = pFormat;
    }

    if( UPDATE_CHAR & eFlags )
    {
        pFormat->SetFont( static_cast<const SvxFontItem&>(rSet.Get( RES_CHRATR_FONT )) );
        pFormat->SetHeight( static_cast<const SvxFontHeightItem&>(rSet.Get( RES_CHRATR_FONTSIZE )) );
        pFormat->SetWeight( static_cast<const SvxWeightItem&>(rSet.Get( RES_CHRATR_WEIGHT )) );
        pFormat->SetPosture( static_cast<const SvxPostureItem&>(rSet.Get( RES_CHRATR_POSTURE )) );
        pFormat->SetCJKFont( static_cast<const SvxFontItem&>(rSet.Get( RES_CHRATR_CJK_FONT )) );
        pFormat->SetCJKHeight( static_cast<const SvxFontHeightItem&>(rSet.Get( RES_CHRATR_CJK_FONTSIZE )) );
        pFormat->SetCJKWeight( static_cast<const SvxWeightItem&>(rSet.Get( RES_CHRATR_CJK_WEIGHT )) );
        pFormat->SetCJKPosture( static_cast<const SvxPostureItem&>(rSet.Get( RES_CHRATR_CJK_POSTURE )) );
        pFormat->SetCTLFont( static_cast<const SvxFontItem&>(rSet.Get( RES_CHRATR_CTL_FONT )) );
        pFormat->SetCTLHeight( static_cast<const SvxFontHeightItem&>(rSet.Get( RES_CHRATR_CTL_FONTSIZE )) );
        pFormat->SetCTLWeight( static_cast<const SvxWeightItem&>(rSet.Get( RES_CHRATR_CTL_WEIGHT )) );
        pFormat->SetCTLPosture( static_cast<const SvxPostureItem&>(rSet.Get( RES_CHRATR_CTL_POSTURE )) );
        pFormat->SetUnderline( static_cast<const SvxUnderlineItem&>(rSet.Get( RES_CHRATR_UNDERLINE )) );
        pFormat->SetOverline( static_cast<const SvxOverlineItem&>(rSet.Get( RES_CHRATR_OVERLINE )) );
        pFormat->SetCrossedOut( static_cast<const SvxCrossedOutItem&>(rSet.Get( RES_CHRATR_CROSSEDOUT )) );
        pFormat->SetContour( static_cast<const SvxContourItem&>(rSet.Get( RES_CHRATR_CONTOUR )) );
        pFormat->SetShadowed( static_cast<const SvxShadowedItem&>(rSet.Get( RES_CHRATR_SHADOWED )) );
        pFormat->SetColor( static_cast<const SvxColorItem&>(rSet.Get( RES_CHRATR_COLOR )) );
        pFormat->SetAdjust( static_cast<const SvxAdjustItem&>(rSet.Get( RES_PARATR_ADJUST )) );
    }
    if( UPDATE_BOX & eFlags )
    {
        pFormat->SetBox( static_cast<const SvxBoxItem&>(rSet.Get( RES_BOX )) );
// FIXME - add attribute IDs for the diagonal line items
//        pFormat->SetTLBR( (SvxLineItem&)rSet.Get( RES_... ) );
//        pFormat->SetBLTR( (SvxLineItem&)rSet.Get( RES_... ) );
        pFormat->SetBackground( static_cast<const SvxBrushItem&>(rSet.Get( RES_BACKGROUND )) );
        pFormat->SetTextOrientation(static_cast<const SvxFrameDirectionItem&>(rSet.Get(RES_FRAMEDIR)));
        pFormat->SetVerticalAlignment(static_cast<const SwFormatVertOrient&>(rSet.Get(RES_VERT_ORIENT)));

        const SwTableBoxNumFormat* pNumFormatItem;
        const SvNumberformat* pNumFormat = nullptr;
        if( SfxItemState::SET == rSet.GetItemState( RES_BOXATR_FORMAT, true,
            reinterpret_cast<const SfxPoolItem**>(&pNumFormatItem) ) && pNFormatr &&
            nullptr != (pNumFormat = pNFormatr->GetEntry( pNumFormatItem->GetValue() )) )
            pFormat->SetValueFormat( pNumFormat->GetFormatstring(),
                                    pNumFormat->GetLanguage(),
                                    ::GetAppLanguage());
        else
        {
            // default
            pFormat->SetValueFormat( OUString(), LANGUAGE_SYSTEM,
                                  ::GetAppLanguage() );
        }
    }

    // we cannot handle the rest, that's specific to StarCalc
}

void SwTableAutoFormat::UpdateToSet(sal_uInt8 nPos, SfxItemSet& rSet,
                                 UpdateFlags eFlags, SvNumberFormatter* pNFormatr) const
{
    const SwBoxAutoFormat& rChg = GetBoxFormat( nPos );

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
            if (!rCJKFont.GetStyleName().isEmpty())
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
            if (!rCTLFont.GetStyleName().isEmpty())
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

        rSet.Put(rChg.GetTextOrientation());
        rSet.Put(rChg.GetVerticalAlignment());

        if( IsValueFormat() && pNFormatr )
        {
            OUString sFormat;
            LanguageType eLng, eSys;
            rChg.GetValueFormat( sFormat, eLng, eSys );
            if( !sFormat.isEmpty() )
            {
                short nType;
                bool bNew;
                sal_Int32 nCheckPos;
                sal_uInt32 nKey = pNFormatr->GetIndexPuttingAndConverting( sFormat, eLng,
                                                                        eSys, nType, bNew, nCheckPos);
                rSet.Put( SwTableBoxNumFormat( nKey ));
            }
            else
                rSet.ClearItem( RES_BOXATR_FORMAT );
        }
    }

    // we cannot handle the rest, that's specific to StarCalc
}

void SwTableAutoFormat::RestoreTableProperties(SwTable &table) const
{
    SwTableFormat* pFormat = table.GetFrameFormat();
    if (!pFormat)
        return;

    SwDoc *pDoc = pFormat->GetDoc();
    if (!pDoc)
        return;

    SfxItemSet rSet(pDoc->GetAttrPool(), aTableSetRange);

    rSet.Put(m_aBreak);
    rSet.Put(m_aPageDesc);
    rSet.Put(SwFormatLayoutSplit(m_bLayoutSplit));
    rSet.Put(SfxBoolItem(RES_COLLAPSING_BORDERS, m_bCollapsingBorders));
    rSet.Put(m_aKeepWithNextPara);
    rSet.Put(m_aShadow);

    pFormat->SetFormatAttr(rSet);

    SwEditShell *pShell = pDoc->GetEditShell();
    pDoc->SetRowSplit(*pShell->getShellCursor(false), SwFormatRowSplit(m_bRowSplit));

    table.SetRowsToRepeat(m_aRepeatHeading);
}

void SwTableAutoFormat::StoreTableProperties(const SwTable &table)
{
    SwTableFormat* pFormat = table.GetFrameFormat();
    if (!pFormat)
        return;

    SwDoc *pDoc = pFormat->GetDoc();
    if (!pDoc)
        return;

    SwEditShell *pShell = pDoc->GetEditShell();
    SwFormatRowSplit *pRowSplit = nullptr;
    SwDoc::GetRowSplit(*pShell->getShellCursor(false), pRowSplit);
    m_bRowSplit = pRowSplit && pRowSplit->GetValue();
    delete pRowSplit;
    pRowSplit = nullptr;

    const SfxItemSet &rSet = pFormat->GetAttrSet();

    m_aBreak = static_cast<const SvxFormatBreakItem&>(rSet.Get(RES_BREAK));
    m_aPageDesc = static_cast<const SwFormatPageDesc&>(rSet.Get(RES_PAGEDESC));
    const SwFormatLayoutSplit &layoutSplit = static_cast<const SwFormatLayoutSplit&>(rSet.Get(RES_LAYOUT_SPLIT));
    m_bLayoutSplit = layoutSplit.GetValue();
    m_bCollapsingBorders = static_cast<const SfxBoolItem&>(rSet.Get(RES_COLLAPSING_BORDERS)).GetValue();

    m_aKeepWithNextPara = static_cast<const SvxFormatKeepItem&>(rSet.Get(RES_KEEP));
    m_aRepeatHeading = table.GetRowsToRepeat();
    m_aShadow = static_cast<const SvxShadowItem&>(rSet.Get(RES_SHADOW));
}

bool SwTableAutoFormat::Load( SvStream& rStream, const SwAfVersions& rVersions )
{
    sal_uInt16  nVal = 0;
    rStream.ReadUInt16( nVal );
    bool bRet = 0 == rStream.GetError();

    if( bRet && (nVal == AUTOFORMAT_DATA_ID_X ||
            (AUTOFORMAT_DATA_ID_504 <= nVal && nVal <= AUTOFORMAT_DATA_ID)) )
    {
        bool b;
        // --- from 680/dr25 on: store strings as UTF-8
        rtl_TextEncoding eCharSet = (nVal >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        m_aName = rStream.ReadUniOrByteString( eCharSet );
        if( AUTOFORMAT_DATA_ID_552 <= nVal )
        {
            rStream.ReadUInt16( nStrResId );
            sal_uInt16 nId = RID_SVXSTR_TBLAFMT_BEGIN + nStrResId;
            if( RID_SVXSTR_TBLAFMT_BEGIN <= nId &&
                nId < RID_SVXSTR_TBLAFMT_END )
            {
                m_aName = SVX_RESSTR( nId );
            }
            else
                nStrResId = USHRT_MAX;
        }
        rStream.ReadCharAsBool( b ); bInclFont = b;
        rStream.ReadCharAsBool( b ); bInclJustify = b;
        rStream.ReadCharAsBool( b ); bInclFrame = b;
        rStream.ReadCharAsBool( b ); bInclBackground = b;
        rStream.ReadCharAsBool( b ); bInclValueFormat = b;
        rStream.ReadCharAsBool( b ); bInclWidthHeight = b;

        if (nVal >= AUTOFORMAT_DATA_ID_31005 && WriterSpecificBlockExists(rStream))
        {
            SfxPoolItem* pNew = nullptr;

            READ(m_aBreak, SvxFormatBreakItem, AUTOFORMAT_FILE_VERSION);
            READ(m_aPageDesc, SwFormatPageDesc, AUTOFORMAT_FILE_VERSION);
            READ(m_aKeepWithNextPara, SvxFormatKeepItem, AUTOFORMAT_FILE_VERSION);

            rStream.ReadUInt16( m_aRepeatHeading ).ReadCharAsBool( m_bLayoutSplit ).ReadCharAsBool( m_bRowSplit ).ReadCharAsBool( m_bCollapsingBorders );

            READ(m_aShadow, SvxShadowItem, AUTOFORMAT_FILE_VERSION);
        }

        bRet = 0 == rStream.GetError();

        for( sal_uInt8 i = 0; bRet && i < 16; ++i )
        {
            SwBoxAutoFormat* pFormat = new SwBoxAutoFormat;
            bRet = pFormat->Load( rStream, rVersions, nVal );
            if( bRet )
                aBoxAutoFormat[ i ] = pFormat;
            else
            {
                delete pFormat;
                break;
            }
        }
    }
    return bRet;
}

bool SwTableAutoFormat::Save( SvStream& rStream, sal_uInt16 fileVersion ) const
{
    sal_uInt16 nVal = AUTOFORMAT_DATA_ID;
    rStream.WriteUInt16( nVal );
    // --- from 680/dr25 on: store strings as UTF-8
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStream, m_aName,
        RTL_TEXTENCODING_UTF8 );
    rStream.WriteUInt16( nStrResId );
    rStream.WriteBool( bInclFont );
    rStream.WriteBool( bInclJustify );
    rStream.WriteBool( bInclFrame );
    rStream.WriteBool( bInclBackground );
    rStream.WriteBool( bInclValueFormat );
    rStream.WriteBool( bInclWidthHeight );

    {
        WriterSpecificAutoFormatBlock block(rStream);

        m_aBreak.Store(rStream, m_aBreak.GetVersion(fileVersion));
        m_aPageDesc.Store(rStream, m_aPageDesc.GetVersion(fileVersion));
        m_aKeepWithNextPara.Store(rStream, m_aKeepWithNextPara.GetVersion(fileVersion));
        rStream.WriteUInt16( m_aRepeatHeading ).WriteBool( m_bLayoutSplit ).WriteBool( m_bRowSplit ).WriteBool( m_bCollapsingBorders );
        m_aShadow.Store(rStream, m_aShadow.GetVersion(fileVersion));
    }

    bool bRet = 0 == rStream.GetError();

    for( int i = 0; bRet && i < 16; ++i )
    {
        SwBoxAutoFormat* pFormat = aBoxAutoFormat[ i ];
        if( !pFormat )     // if not set -> write default
        {
            // If it doesn't exist yet:
            if( !pDfltBoxAutoFormat )
                pDfltBoxAutoFormat = new SwBoxAutoFormat;
            pFormat = pDfltBoxAutoFormat;
        }
        bRet = pFormat->Save( rStream, fileVersion );
    }
    return bRet;
}

struct SwTableAutoFormatTable::Impl
{
    std::vector<std::unique_ptr<SwTableAutoFormat>> m_AutoFormats;
};

size_t SwTableAutoFormatTable::size() const
{
    return m_pImpl->m_AutoFormats.size();
}

SwTableAutoFormat const& SwTableAutoFormatTable::operator[](size_t const i) const
{
    return *m_pImpl->m_AutoFormats[i];
}
SwTableAutoFormat      & SwTableAutoFormatTable::operator[](size_t const i)
{
    return *m_pImpl->m_AutoFormats[i];
}

void SwTableAutoFormatTable::AddAutoFormat(const SwTableAutoFormat& rTableStyle)
{
    // don't insert when we already have style of this name
    if (FindAutoFormat(rTableStyle.GetName()))
        return;

    InsertAutoFormat(size(), std::unique_ptr<SwTableAutoFormat>(new SwTableAutoFormat(rTableStyle)));
}

void SwTableAutoFormatTable::InsertAutoFormat(size_t const i, std::unique_ptr<SwTableAutoFormat> pFormat)
{
    m_pImpl->m_AutoFormats.insert(m_pImpl->m_AutoFormats.begin() + i, std::move(pFormat));
}

void SwTableAutoFormatTable::EraseAutoFormat(size_t const i)
{
    m_pImpl->m_AutoFormats.erase(m_pImpl->m_AutoFormats.begin() + i);
}

std::unique_ptr<SwTableAutoFormat> SwTableAutoFormatTable::ReleaseAutoFormat(size_t const i)
{
    auto const iter(m_pImpl->m_AutoFormats.begin() + i);
    std::unique_ptr<SwTableAutoFormat> pRet(std::move(*iter));
    m_pImpl->m_AutoFormats.erase(iter);
    return pRet;
}

SwTableAutoFormat* SwTableAutoFormatTable::FindAutoFormat(const OUString& rName) const
{
    for (auto &rFormat : m_pImpl->m_AutoFormats)
    {
        if (rFormat->GetName() == rName)
            return rFormat.get();
    }

    return nullptr;
}

SwTableAutoFormatTable::~SwTableAutoFormatTable()
{
}

SwTableAutoFormatTable::SwTableAutoFormatTable()
    : m_pImpl(new Impl)
{
    OUString sNm;
    std::unique_ptr<SwTableAutoFormat> pNew(new SwTableAutoFormat(
                SwStyleNameMapper::GetUIName(RES_POOLCOLL_STANDARD, sNm)));

    SwBoxAutoFormat aNew;

    sal_uInt8 i;

    Color aColor( COL_BLUE );
    SvxBrushItem aBrushItem( aColor, RES_BACKGROUND );
    aNew.SetBackground( aBrushItem );
    aNew.SetColor( SvxColorItem(Color( COL_WHITE ), RES_CHRATR_COLOR) );

    for( i = 0; i < 4; ++i )
        pNew->SetBoxFormat( aNew, i );

    // 70% gray
    aBrushItem.SetColor( RGB_COLORDATA( 0x4d, 0x4d, 0x4d ) );
    aNew.SetBackground( aBrushItem );
    for( i = 4; i <= 12; i += 4 )
        pNew->SetBoxFormat( aNew, i );

    // 20% gray
    aBrushItem.SetColor( RGB_COLORDATA( 0xcc, 0xcc, 0xcc ) );
    aNew.SetBackground( aBrushItem );
    aColor.SetColor( COL_BLACK );
    aNew.SetColor( SvxColorItem( aColor, RES_CHRATR_COLOR) );
    for( i = 7; i <= 15; i += 4 )
        pNew->SetBoxFormat( aNew, i );
    for( i = 13; i <= 14; ++i )
        pNew->SetBoxFormat( aNew, i );

    aBrushItem.SetColor( Color( COL_WHITE ) );
    aNew.SetBackground( aBrushItem );
    for( i = 5; i <= 6; ++i )
        pNew->SetBoxFormat( aNew, i );
    for( i = 9; i <= 10; ++i )
        pNew->SetBoxFormat( aNew, i );

    SvxBoxItem aBox( RES_BOX );
    aBox.SetDistance( 55 );
    SvxBorderLine aLn( &aColor, DEF_LINE_WIDTH_0 );
    aBox.SetLine( &aLn, SvxBoxItemLine::LEFT );
    aBox.SetLine( &aLn, SvxBoxItemLine::BOTTOM );

    for( i = 0; i <= 15; ++i )
    {
        aBox.SetLine( i <= 3 ? &aLn : nullptr, SvxBoxItemLine::TOP );
        aBox.SetLine( (3 == ( i & 3 )) ? &aLn : nullptr, SvxBoxItemLine::RIGHT );
        const_cast<SwBoxAutoFormat&>(pNew->GetBoxFormat( i )).SetBox( aBox );
    }

    m_pImpl->m_AutoFormats.push_back(std::move(pNew));
}

bool SwTableAutoFormatTable::Load()
{
    bool bRet = false;
    OUString sNm(AUTOTABLE_FORMAT_NAME);
    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sNm ))
    {
        SfxMedium aStream( sNm, STREAM_STD_READ );
        bRet = Load( *aStream.GetInStream() );
    }
    else
        bRet = false;
    return bRet;
}

bool SwTableAutoFormatTable::Save() const
{
    SvtPathOptions aPathOpt;
    const OUString sNm( aPathOpt.GetUserConfigPath() + "/" AUTOTABLE_FORMAT_NAME );
    SfxMedium aStream(sNm, STREAM_STD_WRITE );
    return Save( *aStream.GetOutStream() ) && aStream.Commit();
}

bool SwTableAutoFormatTable::Load( SvStream& rStream )
{
    bool bRet = 0 == rStream.GetError();
    if (bRet)
    {
        // Attention: We need to read a general Header here
        sal_uInt16 nVal = 0;
        rStream.ReadUInt16( nVal );
        bRet = 0 == rStream.GetError();

        if( bRet )
        {
            SwAfVersions aVersions;

            // Default version is 5.0, unless we detect an old format ID.
            sal_uInt16 nFileVers = SOFFICE_FILEFORMAT_50;
            if(nVal < AUTOFORMAT_ID_31005)
                nFileVers = SOFFICE_FILEFORMAT_40;

            if( nVal == AUTOFORMAT_ID_358 ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                sal_uInt8 nChrSet, nCnt;
                long nPos = rStream.Tell();
                rStream.ReadUChar( nCnt ).ReadUChar( nChrSet );
                if( rStream.Tell() != sal_uLong(nPos + nCnt) )
                {
                    OSL_ENSURE( false, "The Header contains more or newer Data" );
                    rStream.Seek( nPos + nCnt );
                }
                rStream.SetStreamCharSet( (rtl_TextEncoding)nChrSet );
                rStream.SetVersion( nFileVers );
            }

            if( nVal == AUTOFORMAT_ID_358 || nVal == AUTOFORMAT_ID_X ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                aVersions.Load( rStream, nVal );        // Item versions

                sal_uInt16 nCount = 0;
                rStream.ReadUInt16( nCount );

                bRet = 0 == rStream.GetError();
                if (bRet)
                {
                    const size_t nMinRecordSize = sizeof(sal_uInt16);
                    const size_t nMaxRecords = rStream.remainingSize() / nMinRecordSize;
                    if (nCount > nMaxRecords)
                    {
                        SAL_WARN("sw.core", "Parsing error: " << nMaxRecords <<
                                 " max possible entries, but " << nCount << " claimed, truncating");
                        nCount = nMaxRecords;
                    }
                    for (sal_uInt16 i = 0; i < nCount; ++i)
                    {
                        std::unique_ptr<SwTableAutoFormat> pNew(
                            new SwTableAutoFormat( OUString() ));
                        bRet = pNew->Load( rStream, aVersions );
                        if( bRet )
                        {
                            m_pImpl->m_AutoFormats.push_back(std::move(pNew));
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                bRet = false;
            }
        }
    }
    return bRet;
}

bool SwTableAutoFormatTable::Save( SvStream& rStream ) const
{
    bool bRet = 0 == rStream.GetError();
    if (bRet)
    {
        rStream.SetVersion(AUTOFORMAT_FILE_VERSION);

        // Attention: We need to save a general Header here
        sal_uInt16 nVal = AUTOFORMAT_ID;
        rStream.WriteUInt16( nVal )
               .WriteUChar( 2 ) // Character count of the Header including this value
               .WriteUChar( GetStoreCharSet( ::osl_getThreadTextEncoding() ) );

        bRet = 0 == rStream.GetError();
        if (!bRet)
            return false;

        // Write this version number for all attributes
        m_pImpl->m_AutoFormats[0]->GetBoxFormat(0).SaveVersionNo(
                rStream, AUTOFORMAT_FILE_VERSION);

        rStream.WriteUInt16( m_pImpl->m_AutoFormats.size() - 1 );
        bRet = 0 == rStream.GetError();

        for (sal_uInt16 i = 1; bRet && i < m_pImpl->m_AutoFormats.size(); ++i)
        {
            SwTableAutoFormat const& rFormat = *m_pImpl->m_AutoFormats[i];
            bRet = rFormat.Save(rStream, AUTOFORMAT_FILE_VERSION);
        }
    }
    rStream.Flush();
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
