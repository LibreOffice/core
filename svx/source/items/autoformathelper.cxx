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

#include <svx/autoformathelper.hxx>
#include <tools/stream.hxx>
#include <svl/legacyitem.hxx>
#include <editeng/legacyitem.hxx>
#include <svx/legacyitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/eitem.hxx>
#include <svx/algitem.hxx>
#include <svl/intitem.hxx>
#include <svx/rotmodit.hxx>
#include <osl/thread.h>

//////////////////////////////////////////////////////////////////////////////

AutoFormatVersions::AutoFormatVersions()
:   nFontVersion(0),
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
    nNumFormatVersion(0)
{
}

const sal_uInt16 AUTOFORMAT_ID_300OVRLN      = 10031;
const sal_uInt16 AUTOFORMAT_ID_680DR14      = 10011;
const sal_uInt16 AUTOFORMAT_ID_504      = 9801;

void AutoFormatVersions::LoadBlockA( SvStream& rStream, sal_uInt16 nVer )
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
}

void AutoFormatVersions::LoadBlockB( SvStream& rStream, sal_uInt16 nVer )
{
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

void AutoFormatVersions::WriteBlockA(SvStream& rStream, sal_uInt16 fileVersion)
{
    rStream.WriteUInt16(legacy::SvxFont::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxFontHeight::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxWeight::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxPosture::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxTextLine::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxTextLine::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxCrossedOut::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SfxBool::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SfxBool::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxColor::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxBox::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxLine::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxBrush::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxAdjust::GetVersion(fileVersion));
}

void AutoFormatVersions::WriteBlockB(SvStream& rStream, sal_uInt16 fileVersion)
{
    rStream.WriteUInt16(legacy::SvxHorJustify::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxVerJustify::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxOrientation::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxMargin::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SfxBool::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::CntInt32::GetVersion(fileVersion));
    rStream.WriteUInt16(legacy::SvxRotateMode::GetVersion(fileVersion));
    rStream.WriteUInt16( 0 );       // NumberFormat
}

//////////////////////////////////////////////////////////////////////////////

void AutoFormatBase::SetFont( const SvxFontItem& rNew )             { m_aFont.reset(rNew.Clone()); }
void AutoFormatBase::SetHeight( const SvxFontHeightItem& rNew )     { m_aHeight.reset(rNew.Clone()); }
void AutoFormatBase::SetWeight( const SvxWeightItem& rNew )         { m_aWeight.reset(rNew.Clone()); }
void AutoFormatBase::SetPosture( const SvxPostureItem& rNew )       { m_aPosture.reset(rNew.Clone()); }
void AutoFormatBase::SetCJKFont( const SvxFontItem& rNew )          { m_aCJKFont.reset(rNew.Clone()); }
void AutoFormatBase::SetCJKHeight( const SvxFontHeightItem& rNew )  { m_aCJKHeight.reset(rNew.Clone()); }
void AutoFormatBase::SetCJKWeight( const SvxWeightItem& rNew )      { m_aCJKWeight.reset(rNew.Clone()); }
void AutoFormatBase::SetCJKPosture( const SvxPostureItem& rNew )    { m_aCJKPosture.reset(rNew.Clone()); }
void AutoFormatBase::SetCTLFont( const SvxFontItem& rNew )          { m_aCTLFont.reset(rNew.Clone()); }
void AutoFormatBase::SetCTLHeight( const SvxFontHeightItem& rNew )  { m_aCTLHeight.reset(rNew.Clone()); }
void AutoFormatBase::SetCTLWeight( const SvxWeightItem& rNew )      { m_aCTLWeight.reset(rNew.Clone()); }
void AutoFormatBase::SetCTLPosture( const SvxPostureItem& rNew )    { m_aCTLPosture.reset(rNew.Clone()); }
void AutoFormatBase::SetUnderline( const SvxUnderlineItem& rNew )   { m_aUnderline.reset(rNew.Clone()); }
void AutoFormatBase::SetOverline( const SvxOverlineItem& rNew )     { m_aOverline.reset(rNew.Clone()); }
void AutoFormatBase::SetCrossedOut( const SvxCrossedOutItem& rNew ) { m_aCrossedOut.reset(rNew.Clone()); }
void AutoFormatBase::SetContour( const SvxContourItem& rNew )       { m_aContour.reset(rNew.Clone()); }
void AutoFormatBase::SetShadowed( const SvxShadowedItem& rNew )     { m_aShadowed.reset(rNew.Clone()); }
void AutoFormatBase::SetColor( const SvxColorItem& rNew )           { m_aColor.reset(rNew.Clone()); }
void AutoFormatBase::SetBox( const SvxBoxItem& rNew )               { m_aBox.reset(rNew.Clone()); }
void AutoFormatBase::SetTLBR( const SvxLineItem& rNew )             { m_aTLBR.reset(rNew.Clone()); }
void AutoFormatBase::SetBLTR( const SvxLineItem& rNew )             { m_aBLTR.reset(rNew.Clone()); }
void AutoFormatBase::SetBackground( const SvxBrushItem& rNew )      { m_aBackground.reset(rNew.Clone()); }
void AutoFormatBase::SetAdjust( const SvxAdjustItem& rNew )         { m_aAdjust.reset(rNew.Clone()); }
void AutoFormatBase::SetHorJustify( const SvxHorJustifyItem& rNew ) { m_aHorJustify.reset(rNew.Clone()); }
void AutoFormatBase::SetVerJustify( const SvxVerJustifyItem& rNew ) { m_aVerJustify.reset(rNew.Clone()); }
void AutoFormatBase::SetStacked( const SfxBoolItem& rNew ) { m_aStacked.reset(rNew.Clone()); }
void AutoFormatBase::SetMargin( const SvxMarginItem& rNew ) { m_aMargin.reset(rNew.Clone()); }
void AutoFormatBase::SetLinebreak( const SfxBoolItem& rNew ) { m_aLinebreak.reset(rNew.Clone()); }
void AutoFormatBase::SetRotateAngle( const SfxInt32Item& rNew ) { m_aRotateAngle.reset(rNew.Clone()); }
void AutoFormatBase::SetRotateMode( const SvxRotateModeItem& rNew ) { m_aRotateMode.reset(rNew.Clone()); }

AutoFormatBase::AutoFormatBase()
:   m_aFont(),
    m_aHeight(),
    m_aWeight(),
    m_aPosture(),
    m_aCJKFont(),
    m_aCJKHeight(),
    m_aCJKWeight(),
    m_aCJKPosture(),
    m_aCTLFont(),
    m_aCTLHeight(),
    m_aCTLWeight(),
    m_aCTLPosture(),
    m_aUnderline(),
    m_aOverline(),
    m_aCrossedOut(),
    m_aContour(),
    m_aShadowed(),
    m_aColor(),
    m_aBox(),
    m_aTLBR(),
    m_aBLTR(),
    m_aBackground(),
    m_aAdjust(),
    m_aHorJustify(),
    m_aVerJustify(),
    m_aStacked(),
    m_aMargin(),
    m_aLinebreak(),
    m_aRotateAngle(),
    m_aRotateMode()
{
}

AutoFormatBase::AutoFormatBase( const AutoFormatBase& rNew )
:   m_aFont(rNew.m_aFont->Clone()),
    m_aHeight(rNew.m_aHeight->Clone()),
    m_aWeight(rNew.m_aWeight->Clone()),
    m_aPosture(rNew.m_aPosture->Clone()),
    m_aCJKFont(rNew.m_aCJKFont->Clone()),
    m_aCJKHeight(rNew.m_aCJKHeight->Clone()),
    m_aCJKWeight(rNew.m_aCJKWeight->Clone()),
    m_aCJKPosture(rNew.m_aCJKPosture->Clone()),
    m_aCTLFont(rNew.m_aCTLFont->Clone()),
    m_aCTLHeight(rNew.m_aCTLHeight->Clone()),
    m_aCTLWeight(rNew.m_aCTLWeight->Clone()),
    m_aCTLPosture(rNew.m_aCTLPosture->Clone()),
    m_aUnderline(rNew.m_aUnderline->Clone()),
    m_aOverline(rNew.m_aOverline->Clone()),
    m_aCrossedOut(rNew.m_aCrossedOut->Clone()),
    m_aContour(rNew.m_aContour->Clone()),
    m_aShadowed(rNew.m_aShadowed->Clone()),
    m_aColor(rNew.m_aColor->Clone()),
    m_aBox(rNew.m_aBox->Clone()),
    m_aTLBR(rNew.m_aTLBR->Clone()),
    m_aBLTR(rNew.m_aBLTR->Clone()),
    m_aBackground(rNew.m_aBackground->Clone()),
    m_aAdjust(rNew.m_aAdjust->Clone()),
    m_aHorJustify(rNew.m_aHorJustify->Clone()),
    m_aVerJustify(rNew.m_aVerJustify->Clone()),
    m_aStacked(rNew.m_aStacked->Clone()),
    m_aMargin(rNew.m_aMargin->Clone()),
    m_aLinebreak(rNew.m_aLinebreak->Clone()),
    m_aRotateAngle(rNew.m_aRotateAngle->Clone()),
    m_aRotateMode(rNew.m_aRotateMode->Clone())
{
}

AutoFormatBase::~AutoFormatBase()
{
}

AutoFormatBase& AutoFormatBase::operator=(const AutoFormatBase& rRef)
{
    // check self-assignment
    if(this == &rRef)
    {
        return *this;
    }

    // copy local members - this will use ::Clone() on all involved Items
    SetFont(rRef.GetFont());
    SetHeight(rRef.GetHeight());
    SetWeight(rRef.GetWeight());
    SetPosture(rRef.GetPosture());
    SetCJKFont(rRef.GetCJKFont());
    SetCJKHeight(rRef.GetCJKHeight());
    SetCJKWeight(rRef.GetCJKWeight());
    SetCJKPosture(rRef.GetCJKPosture());
    SetCTLFont(rRef.GetCTLFont());
    SetCTLHeight(rRef.GetCTLHeight());
    SetCTLWeight(rRef.GetCTLWeight());
    SetCTLPosture(rRef.GetCTLPosture());
    SetUnderline(rRef.GetUnderline());
    SetOverline(rRef.GetOverline());
    SetCrossedOut(rRef.GetCrossedOut());
    SetContour(rRef.GetContour());
    SetShadowed(rRef.GetShadowed());
    SetColor(rRef.GetColor());
    SetBox(rRef.GetBox());
    SetTLBR(rRef.GetTLBR());
    SetBLTR(rRef.GetBLTR());
    SetBackground(rRef.GetBackground());
    SetAdjust(rRef.GetAdjust());
    SetHorJustify(rRef.GetHorJustify());
    SetVerJustify(rRef.GetVerJustify());
    SetStacked(rRef.GetStacked());
    SetMargin(rRef.GetMargin());
    SetLinebreak(rRef.GetLinebreak());
    SetRotateAngle(rRef.GetRotateAngle());
    SetRotateMode(rRef.GetRotateMode());

    return *this;
}

const sal_uInt16 AUTOFORMAT_DATA_ID_641 = 10002;
const sal_uInt16 AUTOFORMAT_DATA_ID_300OVRLN = 10032;
const sal_uInt16 AUTOFORMAT_DATA_ID_680DR14 = 10012;
const sal_uInt16 AUTOFORMAT_DATA_ID_504 = 9802;

bool AutoFormatBase::LoadBlockA( SvStream& rStream, const AutoFormatVersions& rVersions, sal_uInt16 nVer )
{
    legacy::SvxFont::Create(*m_aFont, rStream, rVersions.nFontVersion);

    if( rStream.GetStreamCharSet() == m_aFont->GetCharSet() )
    {
        m_aFont->SetCharSet(::osl_getThreadTextEncoding());
    }

    legacy::SvxFontHeight::Create(*m_aHeight, rStream, rVersions.nFontHeightVersion);
    legacy::SvxWeight::Create(*m_aWeight, rStream, rVersions.nWeightVersion);
    legacy::SvxPosture::Create(*m_aPosture, rStream, rVersions.nPostureVersion);

    // --- from 641 on: CJK and CTL font settings
    if( AUTOFORMAT_DATA_ID_641 <= nVer )
    {
        legacy::SvxFont::Create(*m_aCJKFont, rStream, rVersions.nFontVersion);
        legacy::SvxFontHeight::Create(*m_aCJKHeight, rStream, rVersions.nFontHeightVersion);
        legacy::SvxWeight::Create(*m_aCJKWeight, rStream, rVersions.nWeightVersion);
        legacy::SvxPosture::Create(*m_aCJKPosture, rStream, rVersions.nPostureVersion);

        legacy::SvxFont::Create(*m_aCTLFont, rStream, rVersions.nFontVersion);
        legacy::SvxFontHeight::Create(*m_aCTLHeight, rStream, rVersions.nFontHeightVersion);
        legacy::SvxWeight::Create(*m_aCTLWeight, rStream, rVersions.nWeightVersion);
        legacy::SvxPosture::Create(*m_aCTLPosture, rStream, rVersions.nPostureVersion);
    }

    legacy::SvxTextLine::Create(*m_aUnderline, rStream, rVersions.nUnderlineVersion);

    if( nVer >= AUTOFORMAT_DATA_ID_300OVRLN )
    {
        legacy::SvxTextLine::Create(*m_aOverline, rStream, rVersions.nOverlineVersion);
    }

    legacy::SvxCrossedOut::Create(*m_aCrossedOut, rStream, rVersions.nCrossedOutVersion);
    legacy::SfxBool::Create(*m_aContour, rStream, rVersions.nContourVersion);
    legacy::SfxBool::Create(*m_aShadowed, rStream, rVersions.nShadowedVersion);
    legacy::SvxColor::Create(*m_aColor, rStream, rVersions.nColorVersion);
    legacy::SvxBox::Create(*m_aBox, rStream, rVersions.nBoxVersion);

    // --- from 680/dr14 on: diagonal frame lines
    if( nVer >= AUTOFORMAT_DATA_ID_680DR14 )
    {
        legacy::SvxLine::Create(*m_aTLBR, rStream, rVersions.nLineVersion);
        legacy::SvxLine::Create(*m_aBLTR, rStream, rVersions.nLineVersion);
    }

    legacy::SvxBrush::Create(*m_aBackground, rStream, rVersions.nBrushVersion);
    legacy::SvxAdjust::Create(*m_aAdjust, rStream, rVersions.nAdjustVersion);

    return ERRCODE_NONE == rStream.GetError();
}

bool AutoFormatBase::LoadBlockB( SvStream& rStream, const AutoFormatVersions& rVersions, sal_uInt16 nVer )
{
    legacy::SvxHorJustify::Create(*m_aHorJustify, rStream, rVersions.nHorJustifyVersion);
    legacy::SvxVerJustify::Create(*m_aVerJustify, rStream, rVersions.nVerJustifyVersion);
    SvxOrientationItem aOrientation( SvxCellOrientation::Standard, 0);
    legacy::SvxOrientation::Create(aOrientation, rStream, rVersions.nOrientationVersion);
    legacy::SvxMargin::Create(*m_aMargin, rStream, rVersions.nMarginVersion);
    legacy::SfxBool::Create(*m_aLinebreak, rStream, rVersions.nBoolVersion);

    if ( nVer >= AUTOFORMAT_DATA_ID_504 )
    {
        legacy::CntInt32::Create(*m_aRotateAngle, rStream, rVersions.nInt32Version);
        legacy::SvxRotateMode::Create(*m_aRotateMode, rStream, rVersions.nRotateModeVersion);
    }

    m_aStacked->SetValue( aOrientation.IsStacked() );
    m_aRotateAngle->SetValue( aOrientation.GetRotation( Degree100(m_aRotateAngle->GetValue()) ).get() );

    return ERRCODE_NONE == rStream.GetError();
}

bool AutoFormatBase::SaveBlockA( SvStream& rStream, sal_uInt16 fileVersion ) const
{
    legacy::SvxFont::Store(*m_aFont, rStream, legacy::SvxFont::GetVersion(fileVersion));
    legacy::SvxFontHeight::Store(*m_aHeight, rStream, legacy::SvxFontHeight::GetVersion(fileVersion));
    legacy::SvxWeight::Store(*m_aWeight, rStream, legacy::SvxWeight::GetVersion(fileVersion));
    legacy::SvxPosture::Store(*m_aPosture, rStream, legacy::SvxPosture::GetVersion(fileVersion));

    // --- from 641 on: CJK and CTL font settings
    legacy::SvxFont::Store(*m_aCJKFont, rStream, legacy::SvxFont::GetVersion(fileVersion));
    legacy::SvxFontHeight::Store(*m_aCJKHeight, rStream, legacy::SvxFontHeight::GetVersion(fileVersion));
    legacy::SvxWeight::Store(*m_aCJKWeight, rStream, legacy::SvxWeight::GetVersion(fileVersion));
    legacy::SvxPosture::Store(*m_aCJKPosture, rStream, legacy::SvxPosture::GetVersion(fileVersion));

    legacy::SvxFont::Store(*m_aCTLFont, rStream, legacy::SvxFont::GetVersion(fileVersion));
    legacy::SvxFontHeight::Store(*m_aCTLHeight, rStream, legacy::SvxFontHeight::GetVersion(fileVersion));
    legacy::SvxWeight::Store(*m_aCTLWeight, rStream, legacy::SvxWeight::GetVersion(fileVersion));
    legacy::SvxPosture::Store(*m_aCTLPosture, rStream, legacy::SvxPosture::GetVersion(fileVersion));

    legacy::SvxTextLine::Store(*m_aUnderline, rStream, legacy::SvxTextLine::GetVersion(fileVersion));

    // --- from DEV300/overline2 on: overline support
    legacy::SvxTextLine::Store(*m_aOverline, rStream, legacy::SvxTextLine::GetVersion(fileVersion));
    legacy::SvxCrossedOut::Store(*m_aCrossedOut, rStream, legacy::SvxCrossedOut::GetVersion(fileVersion));
    legacy::SfxBool::Store(*m_aContour, rStream, legacy::SfxBool::GetVersion(fileVersion));
    legacy::SfxBool::Store(*m_aShadowed, rStream, legacy::SfxBool::GetVersion(fileVersion));
    legacy::SvxColor::Store(*m_aColor, rStream, legacy::SvxColor::GetVersion(fileVersion));
    legacy::SvxBox::Store(*m_aBox, rStream, legacy::SvxBox::GetVersion(fileVersion));

    // --- from 680/dr14 on: diagonal frame lines
    legacy::SvxLine::Store(*m_aTLBR, rStream, legacy::SvxLine::GetVersion(fileVersion));
    legacy::SvxLine::Store(*m_aBLTR, rStream, legacy::SvxLine::GetVersion(fileVersion));
    legacy::SvxBrush::Store(*m_aBackground, rStream, legacy::SvxBrush::GetVersion(fileVersion));
    legacy::SvxAdjust::Store(*m_aAdjust, rStream, legacy::SvxAdjust::GetVersion(fileVersion));

    return ERRCODE_NONE == rStream.GetError();
}

bool AutoFormatBase::SaveBlockB( SvStream& rStream, sal_uInt16 fileVersion ) const
{
    legacy::SvxHorJustify::Store(*m_aHorJustify, rStream, legacy::SvxHorJustify::GetVersion(fileVersion));
    legacy::SvxVerJustify::Store(*m_aVerJustify, rStream, legacy::SvxVerJustify::GetVersion(fileVersion));
    SvxOrientationItem aOrientation( Degree100(m_aRotateAngle->GetValue()), m_aStacked->GetValue(), 0 );
    legacy::SvxOrientation::Store(aOrientation, rStream, legacy::SvxOrientation::GetVersion(fileVersion));
    legacy::SvxMargin::Store(*m_aMargin, rStream, legacy::SvxMargin::GetVersion(fileVersion));
    legacy::SfxBool::Store(*m_aLinebreak, rStream, legacy::SfxBool::GetVersion(fileVersion));

    // Calc Rotation from SO5
    legacy::CntInt32::Store(*m_aRotateAngle, rStream, legacy::CntInt32::GetVersion(fileVersion));
    legacy::SvxRotateMode::Store(*m_aRotateMode, rStream, legacy::SvxRotateMode::GetVersion(fileVersion));

    return ERRCODE_NONE == rStream.GetError();
}

//////////////////////////////////////////////////////////////////////////////
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
