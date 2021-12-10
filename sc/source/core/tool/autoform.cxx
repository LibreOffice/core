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
#include <autoform.hxx>

#include <sal/log.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/intitem.hxx>
#include <svl/itemset.hxx>
#include <vcl/outdev.hxx>
#include <svx/algitem.hxx>
#include <svx/dialmgr.hxx>
#include <svx/rotmodit.hxx>
#include <svx/strings.hrc>
#include <editeng/adjustitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/fileformat.h>
#include <unotools/collatorwrapper.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <tools/tenccvt.hxx>
#include <osl/diagnose.h>
#include <osl/thread.hxx>

#include <attrib.hxx>
#include <globstr.hrc>
#include <scitems.hxx>
#include <scresid.hxx>
#include <document.hxx>

/*
 * XXX: BIG RED NOTICE! Changes MUST be binary file format compatible and MUST
 * be synchronized with Writer's SwTableAutoFmtTbl sw/source/core/doc/tblafmt.cxx
 */

constexpr OUStringLiteral sAutoTblFmtName = u"autotbl.fmt";

// till SO5PF
const sal_uInt16 AUTOFORMAT_ID_X        = 9501;
const sal_uInt16 AUTOFORMAT_ID_358      = 9601;
const sal_uInt16 AUTOFORMAT_DATA_ID_X   = 9502;

// from SO5 on
// in following versions the value of the IDs must be higher
const sal_uInt16 AUTOFORMAT_ID_504      = 9801;
const sal_uInt16 AUTOFORMAT_DATA_ID_504 = 9802;

const sal_uInt16 AUTOFORMAT_DATA_ID_552 = 9902;

// --- from 680/dr25 on: store strings as UTF-8
const sal_uInt16 AUTOFORMAT_ID_680DR25      = 10021;

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
            blob.pData.reset(new sal_uInt8[blobSize]);
            blob.size = static_cast<std::size_t>(blobSize);
            stream.ReadBytes(blob.pData.get(), blob.size);
        }

        return stream;
    }

    /// Write an AutoFormatSwBlob to stream.
    SvStream& WriteAutoFormatSwBlob(SvStream &stream, const AutoFormatSwBlob &blob)
    {
        const sal_uInt64 endOfBlob = stream.Tell() + sizeof(sal_uInt64) + blob.size;
        stream.WriteUInt64( endOfBlob );
        if (blob.size)
            stream.WriteBytes(blob.pData.get(), blob.size);

        return stream;
    }
}

ScAfVersions::ScAfVersions()
{
}

void ScAfVersions::Load( SvStream& rStream, sal_uInt16 nVer )
{
    LoadBlockA(rStream, nVer);
    if (nVer >= AUTOFORMAT_ID_31005)
    {
        rStream >> swVersions;
    }
    LoadBlockB(rStream, nVer);
}

void ScAfVersions::Write(SvStream& rStream, sal_uInt16 fileVersion)
{
    AutoFormatVersions::WriteBlockA(rStream, fileVersion);

    if (fileVersion >= SOFFICE_FILEFORMAT_50)
    {
        WriteAutoFormatSwBlob( rStream, swVersions );
    }

    AutoFormatVersions::WriteBlockB(rStream, fileVersion);
}

ScAutoFormatDataField::ScAutoFormatDataField()
{
    // need to set default instances for base class AutoFormatBase here
    // due to resource defines (e.g. ATTR_FONT) which are not available
    // in svx and different in the different usages of derivations
    m_aFont = std::make_unique<SvxFontItem>(ATTR_FONT);
    m_aHeight = std::make_unique<SvxFontHeightItem>(240, 100, ATTR_FONT_HEIGHT);
    m_aWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, ATTR_FONT_WEIGHT);
    m_aPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, ATTR_FONT_POSTURE);
    m_aCJKFont = std::make_unique<SvxFontItem>(ATTR_CJK_FONT);
    m_aCJKHeight = std::make_unique<SvxFontHeightItem>(240, 100, ATTR_CJK_FONT_HEIGHT);
    m_aCJKWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, ATTR_CJK_FONT_WEIGHT);
    m_aCJKPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, ATTR_CJK_FONT_POSTURE);
    m_aCTLFont = std::make_unique<SvxFontItem>(ATTR_CTL_FONT);
    m_aCTLHeight = std::make_unique<SvxFontHeightItem>(240, 100, ATTR_CTL_FONT_HEIGHT);
    m_aCTLWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, ATTR_CTL_FONT_WEIGHT);
    m_aCTLPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, ATTR_CTL_FONT_POSTURE);
    m_aUnderline = std::make_unique<SvxUnderlineItem>(LINESTYLE_NONE,ATTR_FONT_UNDERLINE);
    m_aOverline = std::make_unique<SvxOverlineItem>(LINESTYLE_NONE,ATTR_FONT_OVERLINE);
    m_aCrossedOut = std::make_unique<SvxCrossedOutItem>(STRIKEOUT_NONE, ATTR_FONT_CROSSEDOUT);
    m_aContour = std::make_unique<SvxContourItem>(false, ATTR_FONT_CONTOUR);
    m_aShadowed = std::make_unique<SvxShadowedItem>(false, ATTR_FONT_SHADOWED);
    m_aColor = std::make_unique<SvxColorItem>(ATTR_FONT_COLOR);
    m_aBox = std::make_unique<SvxBoxItem>(ATTR_BORDER);
    m_aTLBR = std::make_unique<SvxLineItem>(ATTR_BORDER_TLBR);
    m_aBLTR = std::make_unique<SvxLineItem>(ATTR_BORDER_BLTR);
    m_aBackground = std::make_unique<SvxBrushItem>(ATTR_BACKGROUND);
    m_aAdjust = std::make_unique<SvxAdjustItem>(SvxAdjust::Left, 0);
    m_aHorJustify = std::make_unique<SvxHorJustifyItem>(SvxCellHorJustify::Standard, ATTR_HOR_JUSTIFY);
    m_aVerJustify = std::make_unique<SvxVerJustifyItem>(SvxCellVerJustify::Standard, ATTR_VER_JUSTIFY);
    m_aStacked = std::make_unique<ScVerticalStackCell>();
    m_aMargin = std::make_unique<SvxMarginItem>(ATTR_MARGIN);
    m_aLinebreak = std::make_unique<ScLineBreakCell>();
    m_aRotateAngle = std::make_unique<ScRotateValueItem>(0_deg100);
    m_aRotateMode = std::make_unique<SvxRotateModeItem>(SVX_ROTATE_MODE_STANDARD, ATTR_ROTATE_MODE);
}

ScAutoFormatDataField::ScAutoFormatDataField( const ScAutoFormatDataField& rCopy )
:   AutoFormatBase(rCopy),
     // m_swFields was not copied in original, needed?
    aNumFormat( rCopy.aNumFormat )
{
}

ScAutoFormatDataField::~ScAutoFormatDataField()
{
}

bool ScAutoFormatDataField::Load( SvStream& rStream, const ScAfVersions& rVersions, sal_uInt16 nVer )
{
    LoadBlockA( rStream, rVersions, nVer );

    if (nVer >= AUTOFORMAT_DATA_ID_31005)
    {
        rStream >> m_swFields;
    }

    LoadBlockB( rStream, rVersions, nVer );

    if( 0 == rVersions.nNumFormatVersion )
    {
        // --- from 680/dr25 on: store strings as UTF-8
        rtl_TextEncoding eCharSet = (nVer >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        aNumFormat.Load( rStream, eCharSet );
    }

    //  adjust charset in font
    rtl_TextEncoding eSysSet = osl_getThreadTextEncoding();
    rtl_TextEncoding eSrcSet = rStream.GetStreamCharSet();
    if( eSrcSet != eSysSet && m_aFont->GetCharSet() == eSrcSet )
        m_aFont->SetCharSet(eSysSet);

    return (rStream.GetError() == ERRCODE_NONE);
}

bool ScAutoFormatDataField::Save( SvStream& rStream, sal_uInt16 fileVersion )
{
    SaveBlockA( rStream, fileVersion );

    if (fileVersion >= SOFFICE_FILEFORMAT_50)
    {
        WriteAutoFormatSwBlob( rStream, m_swFields );
    }

    SaveBlockB( rStream, fileVersion );

    // --- from 680/dr25 on: store strings as UTF-8
    aNumFormat.Save( rStream, RTL_TEXTENCODING_UTF8 );

    return (rStream.GetError() == ERRCODE_NONE);
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

    for( sal_uInt16 nIndex = 0; nIndex < 16; ++nIndex )
        ppDataField[ nIndex ].reset( new ScAutoFormatDataField );
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
    for( sal_uInt16 nIndex = 0; nIndex < 16; ++nIndex )
        ppDataField[ nIndex ].reset( new ScAutoFormatDataField( rData.GetField( nIndex ) ) );
}

ScAutoFormatData::~ScAutoFormatData()
{
}

ScAutoFormatDataField& ScAutoFormatData::GetField( sal_uInt16 nIndex )
{
    OSL_ENSURE( nIndex < 16, "ScAutoFormatData::GetField - illegal index" );
    OSL_ENSURE( ppDataField[ nIndex ], "ScAutoFormatData::GetField - no data" );
    return *ppDataField[ nIndex ];
}

const ScAutoFormatDataField& ScAutoFormatData::GetField( sal_uInt16 nIndex ) const
{
    OSL_ENSURE( nIndex < 16, "ScAutoFormatData::GetField - illegal index" );
    OSL_ENSURE( ppDataField[ nIndex ], "ScAutoFormatData::GetField - no data" );
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
        case ATTR_FONT:             rField.SetFont( rItem.StaticWhichCast(ATTR_FONT) );                    break;
        case ATTR_FONT_HEIGHT:      rField.SetHeight( rItem.StaticWhichCast(ATTR_FONT_HEIGHT) );           break;
        case ATTR_FONT_WEIGHT:      rField.SetWeight( rItem.StaticWhichCast(ATTR_FONT_WEIGHT) );           break;
        case ATTR_FONT_POSTURE:     rField.SetPosture( rItem.StaticWhichCast(ATTR_FONT_POSTURE) );         break;
        case ATTR_CJK_FONT:         rField.SetCJKFont( rItem.StaticWhichCast(ATTR_CJK_FONT) );             break;
        case ATTR_CJK_FONT_HEIGHT:  rField.SetCJKHeight( rItem.StaticWhichCast(ATTR_CJK_FONT_HEIGHT) );    break;
        case ATTR_CJK_FONT_WEIGHT:  rField.SetCJKWeight( rItem.StaticWhichCast(ATTR_CJK_FONT_WEIGHT) );    break;
        case ATTR_CJK_FONT_POSTURE: rField.SetCJKPosture( rItem.StaticWhichCast(ATTR_CJK_FONT_POSTURE) );  break;
        case ATTR_CTL_FONT:         rField.SetCTLFont( rItem.StaticWhichCast(ATTR_CTL_FONT) );             break;
        case ATTR_CTL_FONT_HEIGHT:  rField.SetCTLHeight( rItem.StaticWhichCast(ATTR_CTL_FONT_HEIGHT) );    break;
        case ATTR_CTL_FONT_WEIGHT:  rField.SetCTLWeight( rItem.StaticWhichCast(ATTR_CTL_FONT_WEIGHT) );    break;
        case ATTR_CTL_FONT_POSTURE: rField.SetCTLPosture( rItem.StaticWhichCast(ATTR_CTL_FONT_POSTURE) );  break;
        case ATTR_FONT_UNDERLINE:   rField.SetUnderline( rItem.StaticWhichCast(ATTR_FONT_UNDERLINE) );     break;
        case ATTR_FONT_OVERLINE:    rField.SetOverline( rItem.StaticWhichCast(ATTR_FONT_OVERLINE) );       break;
        case ATTR_FONT_CROSSEDOUT:  rField.SetCrossedOut( rItem.StaticWhichCast(ATTR_FONT_CROSSEDOUT) );   break;
        case ATTR_FONT_CONTOUR:     rField.SetContour( rItem.StaticWhichCast(ATTR_FONT_CONTOUR) );         break;
        case ATTR_FONT_SHADOWED:    rField.SetShadowed( rItem.StaticWhichCast(ATTR_FONT_SHADOWED) );       break;
        case ATTR_FONT_COLOR:       rField.SetColor( rItem.StaticWhichCast(ATTR_FONT_COLOR) );             break;
        case ATTR_BORDER:           rField.SetBox( rItem.StaticWhichCast(ATTR_BORDER) );                   break;
        case ATTR_BORDER_TLBR:      rField.SetTLBR( rItem.StaticWhichCast(ATTR_BORDER_TLBR) );             break;
        case ATTR_BORDER_BLTR:      rField.SetBLTR( rItem.StaticWhichCast(ATTR_BORDER_BLTR) );             break;
        case ATTR_BACKGROUND:       rField.SetBackground( rItem.StaticWhichCast(ATTR_BACKGROUND) );        break;
        case ATTR_HOR_JUSTIFY:      rField.SetHorJustify( rItem.StaticWhichCast(ATTR_HOR_JUSTIFY) );       break;
        case ATTR_VER_JUSTIFY:      rField.SetVerJustify( rItem.StaticWhichCast(ATTR_VER_JUSTIFY) );       break;
        case ATTR_STACKED:          rField.SetStacked( rItem.StaticWhichCast(ATTR_STACKED) );              break;
        case ATTR_MARGIN:           rField.SetMargin( rItem.StaticWhichCast(ATTR_MARGIN) );                break;
        case ATTR_LINEBREAK:        rField.SetLinebreak( rItem.StaticWhichCast(ATTR_LINEBREAK) );          break;
        case ATTR_ROTATE_VALUE:     rField.SetRotateAngle( rItem.StaticWhichCast(ATTR_ROTATE_VALUE) );     break;
        case ATTR_ROTATE_MODE:      rField.SetRotateMode( rItem.StaticWhichCast(ATTR_ROTATE_MODE) );       break;
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

void ScAutoFormatData::FillToItemSet( sal_uInt16 nIndex, SfxItemSet& rItemSet, const ScDocument& rDoc ) const
{
    const ScAutoFormatDataField& rField = GetField( nIndex );

    if( bIncludeValueFormat )
    {
        ScNumFormatAbbrev& rNumFormat = const_cast<ScNumFormatAbbrev&>(rField.GetNumFormat());
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
            SvxFontHeightItem aFontHeightItem(rField.GetHeight());
            aFontHeightItem.SetWhich(ATTR_CJK_FONT_HEIGHT);
            rItemSet.Put( aFontHeightItem );
            SvxWeightItem aWeightItem(rField.GetWeight());
            aWeightItem.SetWhich(ATTR_CJK_FONT_WEIGHT);
            rItemSet.Put( aWeightItem );
            SvxPostureItem aPostureItem(rField.GetPosture());
            aPostureItem.SetWhich(ATTR_CJK_FONT_POSTURE);
            rItemSet.Put( aPostureItem );
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
            SvxFontHeightItem aFontHeightItem(rField.GetHeight());
            aFontHeightItem.SetWhich(ATTR_CTL_FONT_HEIGHT);
            rItemSet.Put( aFontHeightItem );
            SvxWeightItem aWeightItem(rField.GetWeight());
            aWeightItem.SetWhich(ATTR_CTL_FONT_WEIGHT);
            rItemSet.Put( aWeightItem );
            SvxPostureItem aPostureItem(rField.GetPosture());
            aPostureItem.SetWhich(ATTR_CTL_FONT_POSTURE);
            rItemSet.Put( aPostureItem );
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
    rField.SetFont          ( rItemSet.Get( ATTR_FONT ) );
    rField.SetHeight        ( rItemSet.Get( ATTR_FONT_HEIGHT ) );
    rField.SetWeight        ( rItemSet.Get( ATTR_FONT_WEIGHT ) );
    rField.SetPosture       ( rItemSet.Get( ATTR_FONT_POSTURE ) );
    rField.SetCJKFont       ( rItemSet.Get( ATTR_CJK_FONT ) );
    rField.SetCJKHeight     ( rItemSet.Get( ATTR_CJK_FONT_HEIGHT ) );
    rField.SetCJKWeight     ( rItemSet.Get( ATTR_CJK_FONT_WEIGHT ) );
    rField.SetCJKPosture    ( rItemSet.Get( ATTR_CJK_FONT_POSTURE ) );
    rField.SetCTLFont       ( rItemSet.Get( ATTR_CTL_FONT ) );
    rField.SetCTLHeight     ( rItemSet.Get( ATTR_CTL_FONT_HEIGHT ) );
    rField.SetCTLWeight     ( rItemSet.Get( ATTR_CTL_FONT_WEIGHT ) );
    rField.SetCTLPosture    ( rItemSet.Get( ATTR_CTL_FONT_POSTURE ) );
    rField.SetUnderline     ( rItemSet.Get( ATTR_FONT_UNDERLINE ) );
    rField.SetOverline      ( rItemSet.Get( ATTR_FONT_OVERLINE ) );
    rField.SetCrossedOut    ( rItemSet.Get( ATTR_FONT_CROSSEDOUT ) );
    rField.SetContour       ( rItemSet.Get( ATTR_FONT_CONTOUR ) );
    rField.SetShadowed      ( rItemSet.Get( ATTR_FONT_SHADOWED ) );
    rField.SetColor         ( rItemSet.Get( ATTR_FONT_COLOR ) );
    rField.SetTLBR          ( rItemSet.Get( ATTR_BORDER_TLBR ) );
    rField.SetBLTR          ( rItemSet.Get( ATTR_BORDER_BLTR ) );
    rField.SetHorJustify    ( rItemSet.Get( ATTR_HOR_JUSTIFY ) );
    rField.SetVerJustify    ( rItemSet.Get( ATTR_VER_JUSTIFY ) );
    rField.SetStacked       ( rItemSet.Get( ATTR_STACKED ) );
    rField.SetLinebreak     ( rItemSet.Get( ATTR_LINEBREAK ) );
    rField.SetMargin        ( rItemSet.Get( ATTR_MARGIN ) );
    rField.SetBackground    ( rItemSet.Get( ATTR_BACKGROUND ) );
    rField.SetRotateAngle   ( rItemSet.Get( ATTR_ROTATE_VALUE ) );
    rField.SetRotateMode    ( rItemSet.Get( ATTR_ROTATE_MODE ) );
}

const TranslateId RID_SVXSTR_TBLAFMT[] =
{
    RID_SVXSTR_TBLAFMT_3D,
    RID_SVXSTR_TBLAFMT_BLACK1,
    RID_SVXSTR_TBLAFMT_BLACK2,
    RID_SVXSTR_TBLAFMT_BLUE,
    RID_SVXSTR_TBLAFMT_BROWN,
    RID_SVXSTR_TBLAFMT_CURRENCY,
    RID_SVXSTR_TBLAFMT_CURRENCY_3D,
    RID_SVXSTR_TBLAFMT_CURRENCY_GRAY,
    RID_SVXSTR_TBLAFMT_CURRENCY_LAVENDER,
    RID_SVXSTR_TBLAFMT_CURRENCY_TURQUOISE,
    RID_SVXSTR_TBLAFMT_GRAY,
    RID_SVXSTR_TBLAFMT_GREEN,
    RID_SVXSTR_TBLAFMT_LAVENDER,
    RID_SVXSTR_TBLAFMT_RED,
    RID_SVXSTR_TBLAFMT_TURQUOISE,
    RID_SVXSTR_TBLAFMT_YELLOW,
    RID_SVXSTR_TBLAFMT_LO6_ACADEMIC,
    RID_SVXSTR_TBLAFMT_LO6_BOX_LIST_BLUE,
    RID_SVXSTR_TBLAFMT_LO6_BOX_LIST_GREEN,
    RID_SVXSTR_TBLAFMT_LO6_BOX_LIST_RED,
    RID_SVXSTR_TBLAFMT_LO6_BOX_LIST_YELLOW,
    RID_SVXSTR_TBLAFMT_LO6_ELEGANT,
    RID_SVXSTR_TBLAFMT_LO6_FINANCIAL,
    RID_SVXSTR_TBLAFMT_LO6_SIMPLE_GRID_COLUMNS,
    RID_SVXSTR_TBLAFMT_LO6_SIMPLE_GRID_ROWS,
    RID_SVXSTR_TBLAFMT_LO6_SIMPLE_LIST_SHADED
};

bool ScAutoFormatData::Load( SvStream& rStream, const ScAfVersions& rVersions )
{
    sal_uInt16  nVer = 0;
    rStream.ReadUInt16( nVer );
    bool bRet = ERRCODE_NONE == rStream.GetError();
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
            if (nStrResId < SAL_N_ELEMENTS(RID_SVXSTR_TBLAFMT))
                aName = SvxResId(RID_SVXSTR_TBLAFMT[nStrResId]);
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

        bRet = ERRCODE_NONE == rStream.GetError();
        for( sal_uInt16 i = 0; bRet && i < 16; ++i )
            bRet = GetField( i ).Load( rStream, rVersions, nVer );
    }
    else
        bRet = false;
    return bRet;
}

bool ScAutoFormatData::Save(SvStream& rStream, sal_uInt16 fileVersion)
{
    rStream.WriteUInt16( AUTOFORMAT_DATA_ID );
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

    bool bRet = ERRCODE_NONE == rStream.GetError();
    for (sal_uInt16 i = 0; bRet && (i < 16); i++)
        bRet = GetField( i ).Save( rStream, fileVersion );

    return bRet;
}

ScAutoFormat::ScAutoFormat() :
    mbSaveLater(false)
{
    //  create default autoformat
    std::unique_ptr<ScAutoFormatData> pData(new ScAutoFormatData);
    OUString aName(ScResId(STR_STYLENAME_STANDARD));
    pData->SetName(aName);

    //  default font, default height
    vcl::Font aStdFont = OutputDevice::GetDefaultFont(
        DefaultFontType::LATIN_SPREADSHEET, LANGUAGE_ENGLISH_US, GetDefaultFontFlags::OnlyOne );
    SvxFontItem aFontItem(
        aStdFont.GetFamilyType(), aStdFont.GetFamilyName(), aStdFont.GetStyleName(),
        aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_FONT );

    aStdFont = OutputDevice::GetDefaultFont(
        DefaultFontType::CJK_SPREADSHEET, LANGUAGE_ENGLISH_US, GetDefaultFontFlags::OnlyOne );
    SvxFontItem aCJKFontItem(
        aStdFont.GetFamilyType(), aStdFont.GetFamilyName(), aStdFont.GetStyleName(),
        aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_CJK_FONT );

    aStdFont = OutputDevice::GetDefaultFont(
        DefaultFontType::CTL_SPREADSHEET, LANGUAGE_ENGLISH_US, GetDefaultFontFlags::OnlyOne );
    SvxFontItem aCTLFontItem(
        aStdFont.GetFamilyType(), aStdFont.GetFamilyName(), aStdFont.GetStyleName(),
        aStdFont.GetPitch(), aStdFont.GetCharSet(), ATTR_CTL_FONT );

    SvxFontHeightItem aHeight( 200, 100, ATTR_FONT_HEIGHT );      // 10 pt;

    //  black thin border
    Color aBlack( COL_BLACK );
    ::editeng::SvxBorderLine aLine( &aBlack, SvxBorderLineWidth::VeryThin );
    SvxBoxItem aBox( ATTR_BORDER );
    aBox.SetLine(&aLine, SvxBoxItemLine::LEFT);
    aBox.SetLine(&aLine, SvxBoxItemLine::TOP);
    aBox.SetLine(&aLine, SvxBoxItemLine::RIGHT);
    aBox.SetLine(&aLine, SvxBoxItemLine::BOTTOM);

    Color aWhite(COL_WHITE);
    SvxColorItem aWhiteText( aWhite, ATTR_FONT_COLOR );
    SvxColorItem aBlackText( aBlack, ATTR_FONT_COLOR );
    SvxBrushItem aBlueBack( COL_BLUE, ATTR_BACKGROUND );
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

    insert(std::move(pData));
}

bool DefaultFirstEntry::operator() (const OUString& left, const OUString& right) const
{
    OUString aStrStandard(ScResId(STR_STYLENAME_STANDARD));
    if (ScGlobal::GetTransliteration().isEqual( left, right ) )
        return false;
    if ( ScGlobal::GetTransliteration().isEqual( left, aStrStandard ) )
        return true;
    if ( ScGlobal::GetTransliteration().isEqual( right, aStrStandard ) )
        return false;
    return ScGlobal::GetCollator().compareString( left, right) < 0;
}

void ScAutoFormat::SetSaveLater( bool bSet )
{
    mbSaveLater = bSet;
}

const ScAutoFormatData* ScAutoFormat::findByIndex(size_t nIndex) const
{
    if (nIndex >= m_Data.size())
        return nullptr;

    MapType::const_iterator it = m_Data.begin();
    std::advance(it, nIndex);
    return it->second.get();
}

ScAutoFormatData* ScAutoFormat::findByIndex(size_t nIndex)
{
    if (nIndex >= m_Data.size())
        return nullptr;

    MapType::iterator it = m_Data.begin();
    std::advance(it, nIndex);
    return it->second.get();
}

ScAutoFormat::iterator ScAutoFormat::find(const OUString& rName)
{
    return m_Data.find(rName);
}

ScAutoFormat::iterator ScAutoFormat::insert(std::unique_ptr<ScAutoFormatData> pNew)
{
    OUString aName = pNew->GetName();
    return m_Data.insert(std::make_pair(aName, std::move(pNew))).first;
}

void ScAutoFormat::erase(const iterator& it)
{
    m_Data.erase(it);
}

size_t ScAutoFormat::size() const
{
    return m_Data.size();
}

ScAutoFormat::const_iterator ScAutoFormat::begin() const
{
    return m_Data.begin();
}

ScAutoFormat::const_iterator ScAutoFormat::end() const
{
    return m_Data.end();
}

ScAutoFormat::iterator ScAutoFormat::begin()
{
    return m_Data.begin();
}

ScAutoFormat::iterator ScAutoFormat::end()
{
    return m_Data.end();
}

void ScAutoFormat::Load()
{
    INetURLObject aURL;
    SvtPathOptions aPathOpt;
    aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
    aURL.setFinalSlash();
    aURL.Append( sAutoTblFmtName );

    SfxMedium aMedium( aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::READ );
    SvStream* pStream = aMedium.GetInStream();
    bool bRet = (pStream && pStream->GetError() == ERRCODE_NONE);
    if (bRet)
    {
        SvStream& rStream = *pStream;
        // Attention: A common header has to be read
        sal_uInt16 nVal = 0;
        rStream.ReadUInt16( nVal );
        bRet = ERRCODE_NONE == rStream.GetError();

        if (bRet)
        {
            if( nVal == AUTOFORMAT_ID_358 ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                sal_uInt8 nChrSet, nCnt;
                sal_uInt64 nPos = rStream.Tell();
                rStream.ReadUChar( nCnt ).ReadUChar( nChrSet );
                if( rStream.Tell() != sal_uLong(nPos + nCnt) )
                {
                    OSL_FAIL( "header contains more/newer data" );
                    rStream.Seek( nPos + nCnt );
                }
                rStream.SetStreamCharSet( GetSOLoadTextEncoding( nChrSet ) );
                rStream.SetVersion( SOFFICE_FILEFORMAT_40 );
            }

            if( nVal == AUTOFORMAT_ID_358 || nVal == AUTOFORMAT_ID_X ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                m_aVersions.Load( rStream, nVal );        // item versions

                sal_uInt16 nCnt = 0;
                rStream.ReadUInt16( nCnt );
                bRet = (rStream.GetError() == ERRCODE_NONE);

                // there has to at least be a sal_uInt16 header
                const size_t nMaxRecords = rStream.remainingSize() / sizeof(sal_uInt16);
                if (nCnt > nMaxRecords)
                {
                    SAL_WARN("sc", "Parsing error: " << nMaxRecords <<
                             " max possible entries, but " << nCnt << " claimed, truncating");
                    nCnt = nMaxRecords;
                }

                for (sal_uInt16 i=0; bRet && (i < nCnt); i++)
                {
                    std::unique_ptr<ScAutoFormatData> pData(new ScAutoFormatData());
                    bRet = pData->Load(rStream, m_aVersions);
                    insert(std::move(pData));
                }
            }
        }
    }
    mbSaveLater = false;
}

bool ScAutoFormat::Save()
{
    INetURLObject aURL;
    SvtPathOptions aPathOpt;
    aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
    aURL.setFinalSlash();
    aURL.Append(sAutoTblFmtName);

    SfxMedium aMedium( aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE );
    SvStream* pStream = aMedium.GetOutStream();
    bool bRet = (pStream && pStream->GetError() == ERRCODE_NONE);
    if (bRet)
    {
        const sal_uInt16 fileVersion = SOFFICE_FILEFORMAT_50;
        SvStream& rStream = *pStream;
        rStream.SetVersion( fileVersion );

        // Attention: A common header has to be saved
        rStream.WriteUInt16( AUTOFORMAT_ID )
               .WriteUChar( 2 )         // Number of chars of the header including this
               .WriteUChar( ::GetSOStoreTextEncoding(
                    osl_getThreadTextEncoding() ) );
        m_aVersions.Write(rStream, fileVersion);

        bRet &= (rStream.GetError() == ERRCODE_NONE);

        rStream.WriteUInt16( m_Data.size() - 1 );
        bRet &= (rStream.GetError() == ERRCODE_NONE);
        MapType::iterator it = m_Data.begin(), itEnd = m_Data.end();
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
