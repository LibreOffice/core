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
#include <editeng/legacyitem.hxx>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
