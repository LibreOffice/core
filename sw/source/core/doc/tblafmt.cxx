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

#define READ( aItem, aItemType, nVers )\
    pNew = aItem.Create(rStream, nVers ); \
    aItem = *static_cast<aItemType*>(pNew); \
    delete pNew;

bool SwTableBoxFormat::Load( SvStream& rStream, const SwAfVersions& rVersions, sal_uInt16 nVer )
{
    SfxPoolItem* pNew;

    SvxFontItem aFont = SvxFontItem( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT ) );
    READ( aFont, SvxFontItem, rVersions.nFontVersion )
    if( rStream.GetStreamCharSet() == aFont.GetCharSet() )
        aFont.SetCharSet( ::osl_getThreadTextEncoding() );
    SetFont( aFont );

    SvxFontHeightItem aHeight = SvxFontHeightItem( 240, 100, RES_CHRATR_FONTSIZE );
    READ( aHeight, SvxFontHeightItem, rVersions.nFontHeightVersion )
    SetHeight( aHeight );

    SvxWeightItem aWeight = SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
    READ( aWeight, SvxWeightItem, rVersions.nWeightVersion )
    SetWeight( aWeight );

    SvxPostureItem aPosture = SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE );
    READ( aPosture, SvxPostureItem, rVersions.nPostureVersion )
    SetPosture( aPosture );

    // --- from 641 on: CJK and CTL font settings
    if( AUTOFORMAT_DATA_ID_641 <= nVer )
    {
        SvxFontItem aCJKFont = SvxFontItem( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT ) );
        READ( aCJKFont, SvxFontItem, rVersions.nFontVersion )
        SetCJKFont( aCJKFont );

        SvxFontHeightItem aCJKHeight = SvxFontHeightItem( 240, 100, RES_CHRATR_FONTSIZE );
        READ( aCJKHeight, SvxFontHeightItem, rVersions.nFontHeightVersion )
        SetCJKHeight( aCJKHeight );

        SvxWeightItem aCJKWeight = SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
        READ( aCJKWeight, SvxWeightItem, rVersions.nWeightVersion )
        SetCJKWeight( aCJKWeight );

        SvxPostureItem aCJKPosture = SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE );
        READ( aCJKPosture, SvxPostureItem, rVersions.nPostureVersion )
        SetCJKPosture( aCJKPosture );


        SvxFontItem aCTLFont = SvxFontItem( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT ) );
        READ( aCTLFont, SvxFontItem, rVersions.nFontVersion )
        SetCTLFont( aCTLFont );

        SvxFontHeightItem aCTLHeight = SvxFontHeightItem( 240, 100, RES_CHRATR_FONTSIZE );
        READ( aCTLHeight, SvxFontHeightItem, rVersions.nFontHeightVersion )
        SetCTLHeight( aCTLHeight );

        SvxWeightItem aCTLWeight = SvxWeightItem( WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
        READ( aCTLWeight, SvxWeightItem, rVersions.nWeightVersion )
        SetCTLWeight( aCTLWeight );

        SvxPostureItem aCTLPosture = SvxPostureItem( ITALIC_NONE, RES_CHRATR_POSTURE );
        READ( aCTLPosture, SvxPostureItem, rVersions.nPostureVersion )
        SetCTLPosture( aCTLPosture );
    }

    SvxUnderlineItem aUnderline = SvxUnderlineItem( UNDERLINE_NONE, RES_CHRATR_UNDERLINE );
    READ( aUnderline, SvxUnderlineItem, rVersions.nUnderlineVersion )
    SetUnderline( aUnderline );

    if( nVer >= AUTOFORMAT_DATA_ID_300OVRLN )
    {
        SvxOverlineItem aOverline = SvxOverlineItem( UNDERLINE_NONE, RES_CHRATR_OVERLINE );
        READ( aOverline, SvxOverlineItem, rVersions.nOverlineVersion )
        SetOverline( aOverline );
    }

    SvxCrossedOutItem aCrossedOut = SvxCrossedOutItem( STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT );
    READ( aCrossedOut, SvxCrossedOutItem, rVersions.nCrossedOutVersion )
    SetCrossedOut( aCrossedOut );

    SvxContourItem aContour = SvxContourItem( sal_False, RES_CHRATR_CONTOUR );
    READ( aContour, SvxContourItem, rVersions.nContourVersion )
    SetContour( aContour );

    SvxShadowedItem aShadowed = SvxShadowedItem( sal_False, RES_CHRATR_SHADOWED );
    READ( aShadowed, SvxShadowedItem, rVersions.nShadowedVersion )
    SetShadowed( aShadowed );

    SvxColorItem aColor = SvxColorItem( RES_CHRATR_COLOR );
    READ( aColor, SvxColorItem, rVersions.nColorVersion )
    SetColor( aColor );

    SvxBoxItem aBox = SvxBoxItem( RES_BOX );
    READ( aBox, SvxBoxItem, rVersions.nBoxVersion )
    SetBox( aBox );

    // Unused, but needed to be loaded to not mess the other styles
    if( nVer >= AUTOFORMAT_DATA_ID_680DR14 )
    {
        SvxLineItem aTLBR( 0 );
        READ( aTLBR, SvxLineItem, rVersions.nLineVersion)

        SvxLineItem aBLTR( 0 );
        READ( aBLTR, SvxLineItem, rVersions.nLineVersion)
    }
    // <- close

    SvxBrushItem aBackground = SvxBrushItem( RES_BACKGROUND );
    READ( aBackground, SvxBrushItem, rVersions.nBrushVersion )
    SetBackground( aBackground );

    SvxAdjustItem aAdjust = SvxAdjustItem( SVX_ADJUST_LEFT, RES_PARATR_ADJUST );
    pNew = aAdjust.Create( rStream, rVersions.nAdjustVersion );
    SetAdjust( *(SvxAdjustItem*)pNew );
    delete pNew;

    if (nVer >= AUTOFORMAT_DATA_ID_31005 && WriterSpecificBlockExists(rStream))
    {
        SvxFrameDirectionItem aTextOrientation = SvxFrameDirectionItem( FRMDIR_ENVIRONMENT, RES_FRAMEDIR );
        READ( aTextOrientation, SvxFrameDirectionItem, rVersions.m_nTextOrientationVersion )
        SetTextOrientation( aTextOrientation );

        SwFormatVertOrient aVerticalAlignment = SwFormatVertOrient( 0, com::sun::star::text::VertOrientation::NONE, com::sun::star::text::RelOrientation::FRAME );
        READ( aVerticalAlignment, SwFormatVertOrient, rVersions.m_nVerticalAlignmentVersion );
        SetVerticalAlignment( aVerticalAlignment );
    }

    // Unused, but needed to be loaded to not mess the other styles
    SvxHorJustifyItem aHorJustify( SVX_HOR_JUSTIFY_STANDARD, 0 );
    READ( aHorJustify, SvxHorJustifyItem, rVersions.nHorJustifyVersion )

    SvxVerJustifyItem aVerJustify( SVX_VER_JUSTIFY_STANDARD, 0 );
    READ( aVerJustify, SvxVerJustifyItem, rVersions.nVerJustifyVersion )

    SvxOrientationItem aOrientation( SVX_ORIENTATION_STANDARD, 0 );
    READ( aOrientation, SvxOrientationItem, rVersions.nOrientationVersion )

    SvxMarginItem aMargin( 0 );
    READ( aMargin, SvxMarginItem, rVersions.nMarginVersion )

    SfxBoolItem aLinebreak( 0 );
    pNew = aLinebreak.Create(rStream, rVersions.nBoolVersion );
    aLinebreak.SetValue( ((SfxBoolItem*)pNew)->GetValue() );
    delete pNew;

    if ( nVer >= AUTOFORMAT_DATA_ID_504 )
    {
        SfxInt32Item aRotateAngle( 0 );
        pNew = aRotateAngle.Create( rStream, rVersions.nInt32Version );
        aRotateAngle.SetValue( ((SfxInt32Item*)pNew)->GetValue() );
        delete pNew;

        SvxRotateModeItem aRotateMode( SVX_ROTATE_MODE_STANDARD, 0 );
        pNew = aRotateMode.Create( rStream, rVersions.nRotateModeVersion );
        aRotateMode.SetValue( ((SvxRotateModeItem*)pNew)->GetValue() );
        delete pNew;
    }
    // <- close

    if( 0 == rVersions.nNumFormatVersion )
    {
        sal_uInt16 eSys, eLge;
        // --- from 680/dr25 on: store strings as UTF-8
        rtl_TextEncoding eCharSet = (nVer >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        sNumFormatString = rStream.ReadUniOrByteString( eCharSet );
        rStream.ReadUInt16( eSys ).ReadUInt16( eLge );
        eSysLanguage = (LanguageType) eSys;
        eNumFormatLanguage = (LanguageType) eLge;
        if ( eSysLanguage == LANGUAGE_SYSTEM )      // from old versions (Calc)
            eSysLanguage = ::GetAppLanguage();
    }

    return 0 == rStream.GetError();
}

bool SwTableBoxFormat::Save( SvStream& rStream, sal_uInt16 fileVersion ) const
{
    SvxFontItem aFont = GetFont();
    aFont.Store( rStream, aFont.GetVersion(fileVersion) );

    SvxFontHeightItem aHeight = GetHeight();
    aHeight.Store( rStream, aHeight.GetVersion(fileVersion) );

    SvxWeightItem aWeight = GetWeight();
    aWeight.Store( rStream, aWeight.GetVersion(fileVersion) );

    SvxPostureItem aPosture = GetPosture();
    aPosture.Store( rStream, aPosture.GetVersion(fileVersion) );

    SvxFontItem aCJKFont = GetCJKFont();
    aCJKFont.Store( rStream, aCJKFont.GetVersion(fileVersion) );

    SvxFontHeightItem aCJKHeight = GetCJKHeight();
    aCJKHeight.Store( rStream, aCJKHeight.GetVersion(fileVersion) );

    SvxWeightItem aCJKWeight = GetCJKWeight();
    aCJKWeight.Store( rStream, aCJKWeight.GetVersion(fileVersion) );

    SvxPostureItem aCJKPosture = GetCJKPosture();
    aCJKPosture.Store( rStream, aCJKPosture.GetVersion(fileVersion) );

    SvxFontItem aCTLFont = GetCTLFont();
    aCTLFont.Store( rStream, aCTLFont.GetVersion(fileVersion) );

    SvxFontHeightItem aCTLHeight = GetCTLHeight();
    aCTLHeight.Store( rStream, aCTLHeight.GetVersion(fileVersion) );

    SvxWeightItem aCTLWeight = GetCTLWeight();
    aCTLWeight.Store( rStream, aCTLWeight.GetVersion(fileVersion) );

    SvxPostureItem aCTLPosture = GetCTLPosture();
    aCTLPosture.Store( rStream, aCTLPosture.GetVersion(fileVersion) );

    SvxUnderlineItem aUnderline = GetUnderline();
    aUnderline.Store( rStream, aUnderline.GetVersion(fileVersion) );

    SvxOverlineItem aOverline = GetOverline();
    aOverline.Store( rStream, aOverline.GetVersion(fileVersion) );

    SvxCrossedOutItem aCrossedOut = GetCrossedOut();
    aCrossedOut.Store( rStream, aCrossedOut.GetVersion(fileVersion) );

    SvxContourItem aContour = GetContour();
    aContour.Store( rStream, aContour.GetVersion(fileVersion) );

    SvxShadowedItem aShadowed = GetShadowed();
    aShadowed.Store( rStream, aShadowed.GetVersion(fileVersion) );

    SvxColorItem aColor = GetColor();
    aColor.Store( rStream, aColor.GetVersion(fileVersion) );

    SvxBoxItem aBox = GetBox();
    aBox.Store( rStream, aBox.GetVersion(fileVersion) );

    SvxBrushItem aBackground = makeBackgroundBrushItem();
    aBackground.Store( rStream, aBackground.GetVersion(fileVersion) );

    SvxAdjustItem aAdjust = GetAdjust();
    aAdjust.Store( rStream, aAdjust.GetVersion(fileVersion) );

    if (fileVersion >= SOFFICE_FILEFORMAT_50)
    {
        WriterSpecificAutoFormatBlock block(rStream);

        SvxFrameDirectionItem aTextOrientation = GetTextOrientation();
        aTextOrientation.Store(rStream, aTextOrientation.GetVersion(fileVersion));

        SwFormatVertOrient aVerticalAlignment = GetVerticalAlignment();
        aVerticalAlignment.Store(rStream, aVerticalAlignment.GetVersion(fileVersion));
    }

    // --- from 680/dr25 on: store strings as UTF-8
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStream, sNumFormatString,
        RTL_TEXTENCODING_UTF8);
    rStream.WriteUInt16( eSysLanguage ).WriteUInt16( eNumFormatLanguage );

    return 0 == rStream.GetError();
}

bool SwTableBoxFormat::SaveVersionNo( SvStream& rStream, sal_uInt16 fileVersion ) const
{
    rStream.WriteUInt16(GetFont().GetVersion(fileVersion));
    rStream.WriteUInt16(GetHeight().GetVersion(fileVersion));
    rStream.WriteUInt16(GetWeight().GetVersion(fileVersion));
    rStream.WriteUInt16(GetPosture().GetVersion(fileVersion));
    rStream.WriteUInt16(GetUnderline().GetVersion(fileVersion));
    rStream.WriteUInt16(GetOverline().GetVersion(fileVersion));
    rStream.WriteUInt16(GetCrossedOut().GetVersion(fileVersion));
    rStream.WriteUInt16(GetContour().GetVersion(fileVersion));
    rStream.WriteUInt16(GetShadowed().GetVersion(fileVersion));
    rStream.WriteUInt16(GetColor().GetVersion(fileVersion));
    rStream.WriteUInt16(GetBox().GetVersion(fileVersion));
    rStream.WriteUInt16(makeBackgroundBrushItem().GetVersion(fileVersion));
    rStream.WriteUInt16(GetAdjust().GetVersion(fileVersion));

    if (fileVersion >= SOFFICE_FILEFORMAT_50)
    {
        WriterSpecificAutoFormatBlock block(rStream);

        rStream.WriteUInt16(GetTextOrientation().GetVersion(fileVersion));
        rStream.WriteUInt16(GetVerticalAlignment().GetVersion(fileVersion));
    }

    rStream.WriteUInt16(0);       // NumberFormat

    return 0 == rStream.GetError();
}

SwTableAutoFormat::SwTableAutoFormat( const OUString& rName, SwTableFormat* pTableStyle )
    : m_pTableStyle( pTableStyle )
    , nStrResId( USHRT_MAX )
{
    bInclFont = true;
    bInclJustify = true;
    bInclFrame = true;
    bInclBackground = true;
    bInclValueFormat = true;
    bInclWidthHeight = true;

    m_pTableStyle->SetName( rName );
}

SwTableAutoFormat::SwTableAutoFormat( const SwTableAutoFormat& rNew )
{
    *this = rNew;
}

SwTableAutoFormat& SwTableAutoFormat::operator=( const SwTableAutoFormat& rNew )
{
    if (&rNew == this)
        return *this;

    m_pTableStyle = rNew.m_pTableStyle;
    nStrResId = rNew.nStrResId;
    bInclFont = rNew.bInclFont;
    bInclJustify = rNew.bInclJustify;
    bInclFrame = rNew.bInclFrame;
    bInclBackground = rNew.bInclBackground;
    bInclValueFormat = rNew.bInclValueFormat;
    bInclWidthHeight = rNew.bInclWidthHeight;

    return *this;
}

void SwTableAutoFormat::SetBoxFormat( const SwTableBoxFormat& rNew, sal_uInt8 nPos )
{
    m_pTableStyle->SetBoxFormat( rNew, nPos );
}


SwTableBoxFormat* SwTableAutoFormat::GetBoxFormat( sal_uInt8 nPos ) const
{
    return m_pTableStyle->GetBoxFormat( nPos );
}


void SwTableAutoFormat::UpdateFromSet( sal_uInt8 nPos,
                                    const SfxItemSet& rSet,
                                    UpdateFlags eFlags,
                                    SvNumberFormatter* pNFormatr)
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    SwTableBoxFormat* pFormat = GetBoxFormat( nPos );

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
        const SvNumberformat* pNumFormat = 0;
        if( SfxItemState::SET == rSet.GetItemState( RES_BOXATR_FORMAT, true,
            reinterpret_cast<const SfxPoolItem**>(&pNumFormatItem) ) && pNFormatr &&
            0 != (pNumFormat = pNFormatr->GetEntry( pNumFormatItem->GetValue() )) )
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
    const SwTableBoxFormat& rChg = *GetBoxFormat( nPos );

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
            rSet.Put( rChg.makeBackgroundBrushItem() );

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
    m_pTableStyle->RestoreTableProperties( table );
}

void SwTableAutoFormat::StoreTableProperties(const SwTable &table)
{
    m_pTableStyle->StoreTableProperties( table );
}

bool SwTableFormat::Load( SvStream& rStream, const SwAfVersions& rVersions, SwDoc* pDoc, sal_uInt16 nVal )
{
    sal_Bool bRet = 0 == rStream.GetError();

    if (nVal >= AUTOFORMAT_DATA_ID_31005 && WriterSpecificBlockExists(rStream))
    {
        SfxPoolItem* pNew = 0;

        SvxFormatBreakItem aBreak = SvxFormatBreakItem( SVX_BREAK_NONE, RES_BREAK );
        READ( aBreak, SvxFormatBreakItem, AUTOFORMAT_FILE_VERSION );
        SetBreak( aBreak );

        SwFormatPageDesc aPageDesc;
        READ( aPageDesc, SwFormatPageDesc, AUTOFORMAT_FILE_VERSION );
        SetPageDesc( aPageDesc );

        SvxFormatKeepItem aKeepWithNextPara = SvxFormatKeepItem( sal_False, RES_KEEP );
        READ( aKeepWithNextPara, SvxFormatKeepItem, AUTOFORMAT_FILE_VERSION );
        SetKeepWithNextPara( aKeepWithNextPara );

        sal_uInt16 aRepeatHeading;
        bool bLayoutSplit;
        bool bRowSplit;
        bool bCollapsingBorders;
        rStream.ReadUInt16(aRepeatHeading).ReadCharAsBool(bLayoutSplit).ReadCharAsBool(bRowSplit).ReadCharAsBool(bCollapsingBorders);
        SetRepeatHeading( aRepeatHeading );
        SetRowSplit( bRowSplit );
        SetLayoutSplit( bLayoutSplit );
        SetCollapsingBorders( bCollapsingBorders );

        SvxShadowItem aShadow = SvxShadowItem( RES_SHADOW );
        READ( aShadow, SvxShadowItem, AUTOFORMAT_FILE_VERSION );
        SetShadow( aShadow );
    }

    bRet = 0 == rStream.GetError();

    for( sal_uInt8 i = 0; bRet && i < 16; ++i )
    {
        SwTableBoxFormat* pFormat = pDoc->MakeTableBoxFormat();

        bRet = pFormat->Load( rStream, rVersions, nVal );
        if( bRet )
            SetBoxFormat( *pFormat, i );
        else
        {
            delete pFormat;
            break;
        }
    }

    return bRet;
}

SwTableAutoFormat* SwTableAutoFormat::Load( SvStream& rStream, const SwAfVersions& rVersions, SwDoc* pDoc )
{
    SwTableAutoFormat* pRet = NULL;

    sal_uInt16  nVal = 0;
    rStream.ReadUInt16( nVal );
    bool bRet = 0 == rStream.GetError();

    if( bRet && (nVal == AUTOFORMAT_DATA_ID_X ||
            (AUTOFORMAT_DATA_ID_504 <= nVal && nVal <= AUTOFORMAT_DATA_ID)) )
    {
        bool b;
        // --- from 680/dr25 on: store strings as UTF-8
        rtl_TextEncoding eCharSet = (nVal >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        OUString aName = rStream.ReadUniOrByteString( eCharSet );
        sal_uInt16 nStrResId = USHRT_MAX;
        if( AUTOFORMAT_DATA_ID_552 <= nVal )
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

        // FIXME Yuk! we are creating the table styles ATM, but in the targetted
        // ideal, the table styles are created with the document
        SwTableFormat* pStyle = pDoc->FindTableFormatByName(aName);
        if ( !pStyle )
            pStyle = pDoc->MakeTableFrameFormat(aName, NULL);
        pRet = new SwTableAutoFormat( aName, pStyle );

        pRet->nStrResId = nStrResId;

        rStream.ReadCharAsBool( b ); pRet->bInclFont = b;
        rStream.ReadCharAsBool( b ); pRet->bInclJustify = b;
        rStream.ReadCharAsBool( b ); pRet->bInclFrame = b;
        rStream.ReadCharAsBool( b ); pRet->bInclBackground = b;
        rStream.ReadCharAsBool( b ); pRet->bInclValueFormat = b;
        rStream.ReadCharAsBool( b ); pRet->bInclWidthHeight = b;

        bRet = pStyle->Load( rStream, rVersions, pDoc, nVal );
    }
    if ( !bRet )
    {
        delete pRet;
        pRet = NULL;
    }
    return pRet;
}

bool SwTableAutoFormat::Save( SvStream& rStream, sal_uInt16 fileVersion ) const
{
    sal_uInt16 nVal = AUTOFORMAT_DATA_ID;
    rStream.WriteUInt16( nVal );
    // --- from 680/dr25 on: store strings as UTF-8
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStream, GetName(),
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

        SvxFormatBreakItem m_aBreak = m_pTableStyle->GetBreak();
        m_aBreak.Store(rStream, m_aBreak.GetVersion(fileVersion));

        SwFormatPageDesc m_aPageDesc = m_pTableStyle->GetPageDesc();
        m_aPageDesc.Store(rStream, m_aPageDesc.GetVersion(fileVersion));

        SvxFormatKeepItem m_aKeepWithNextPara = m_pTableStyle->GetKeepWithNextPara();
        m_aKeepWithNextPara.Store(rStream, m_aKeepWithNextPara.GetVersion(fileVersion));

        rStream.WriteUInt16(m_pTableStyle->GetRepeatHeading());
        rStream.WriteBool(m_pTableStyle->GetLayoutSplit());
        rStream.WriteBool(m_pTableStyle->GetRowSplit());
        rStream.WriteBool(m_pTableStyle->GetCollapsingBorders());

        SvxShadowItem m_aShadow = m_pTableStyle->GetShadow();
        m_aShadow.Store(rStream, m_aShadow.GetVersion(fileVersion));
    }

    bool bRet = 0 == rStream.GetError();

    for( int i = 0; bRet && i < 16; ++i )
    {
        SwTableBoxFormat* pFormat = GetBoxFormat( i );

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

void
SwTableAutoFormatTable::InsertAutoFormat(size_t const i, std::unique_ptr<SwTableAutoFormat> pFormat)
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

SwTableAutoFormatTable::~SwTableAutoFormatTable()
{
}

SwTableAutoFormatTable::SwTableAutoFormatTable(SwDoc* pDoc)
    : m_pImpl(new Impl)
    , m_pDoc(pDoc)
{
    OUString sNm;
    // FIXME Yuk! we are creating the table styles ATM, but in the targetted
    // ideal, the table styles are created with the document
    sNm = SwStyleNameMapper::GetUIName( RES_POOLCOLL_STANDARD, sNm );
    SwTableFormat* pStyle = pDoc->FindTableFormatByName(sNm);
    if ( !pStyle )
        pStyle = pDoc->MakeTableFrameFormat(sNm, NULL);
    std::unique_ptr<SwTableAutoFormat> pNewTableAutoFormat(new SwTableAutoFormat(sNm, pStyle));

    SwTableBoxFormat* pNewBoxFormat = pDoc->MakeTableBoxFormat();

    sal_uInt8 i;

    Color aColor( COL_BLUE );
    SvxBrushItem aBrushItem( aColor, RES_BACKGROUND );
    pNewBoxFormat->SetBackground( aBrushItem );
    pNewBoxFormat->SetColor( SvxColorItem(Color( COL_WHITE ), RES_CHRATR_COLOR) );

    for( i = 0; i < 4; ++i )
        pNewTableAutoFormat->SetBoxFormat( *pNewBoxFormat, i );

    // 70% gray
    aBrushItem.SetColor( RGB_COLORDATA( 0x4d, 0x4d, 0x4d ) );
    pNewBoxFormat->SetBackground( aBrushItem );
    for( i = 4; i <= 12; i += 4 )
        pNewTableAutoFormat->SetBoxFormat( *pNewBoxFormat, i );

    // 20% gray
    aBrushItem.SetColor( RGB_COLORDATA( 0xcc, 0xcc, 0xcc ) );
    pNewBoxFormat->SetBackground( aBrushItem );
    aColor.SetColor( COL_BLACK );
    pNewBoxFormat->SetColor( SvxColorItem( aColor, RES_CHRATR_COLOR) );
    for( i = 7; i <= 15; i += 4 )
        pNewTableAutoFormat->SetBoxFormat( *pNewBoxFormat, i );
    for( i = 13; i <= 14; ++i )
        pNewTableAutoFormat->SetBoxFormat( *pNewBoxFormat, i );

    aBrushItem.SetColor( Color( COL_WHITE ) );
    pNewBoxFormat->SetBackground( aBrushItem );
    for( i = 5; i <= 6; ++i )
        pNewTableAutoFormat->SetBoxFormat( *pNewBoxFormat, i );
    for( i = 9; i <= 10; ++i )
        pNewTableAutoFormat->SetBoxFormat( *pNewBoxFormat, i );


    SvxBoxItem aBox( RES_BOX );
    aBox.SetDistance( 55 );
    SvxBorderLine aLn( &aColor, DEF_LINE_WIDTH_0 );
    aBox.SetLine( &aLn, SvxBoxItemLine::LEFT );
    aBox.SetLine( &aLn, SvxBoxItemLine::BOTTOM );

    for( i = 0; i <= 15; ++i )
    {
        aBox.SetLine( i <= 3 ? &aLn : 0, SvxBoxItemLine::TOP );
        aBox.SetLine( (3 == ( i & 3 )) ? &aLn : 0, SvxBoxItemLine::RIGHT );
        pNewTableAutoFormat->GetBoxFormat( i )->SetBox( aBox );
    }

    m_pImpl->m_AutoFormats.push_back(std::move(pNewTableAutoFormat));
}

bool SwTableAutoFormatTable::Load()
{
    bool bRet = false;
    OUString sNm(AUTOTABLE_FORMAT_NAME);
    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sNm, SvtPathOptions::PATH_USERCONFIG ))
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
                        std::unique_ptr<SwTableAutoFormat> pNew(SwTableAutoFormat::Load(rStream, aVersions, m_pDoc));
                        if (pNew)
                        {
                            m_pImpl->m_AutoFormats.push_back(std::move(pNew));
                        }
                        else
                        {
                            bRet = false;
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
        m_pImpl->m_AutoFormats[0]->GetBoxFormat(0)->SaveVersionNo(
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
