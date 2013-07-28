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

#include "tp_TitleRotation.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "chartview/ChartSfxItemIds.hxx"
#include "HelpIds.hrc"
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

// header for class SfxInt32Item
#include <svl/intitem.hxx>

namespace chart
{

SchAlignmentTabPage::SchAlignmentTabPage(Window* pWindow,
                                         const SfxItemSet& rInAttrs, bool bWithRotation) :
    SfxTabPage(pWindow, SchResId(TP_ALIGNMENT), rInAttrs),
    aFlAlign        ( this, SchResId( FL_ALIGN ) ),
    aCtrlDial       ( this, SchResId( CTR_DIAL ) ),
    aFtRotate       ( this, SchResId( FT_DEGREES ) ),
    aNfRotate       ( this, SchResId( NF_ORIENT ) ),
    aCbStacked      ( this, SchResId( BTN_TXTSTACKED ) ),
    aOrientHlp      ( aCtrlDial, aNfRotate, aCbStacked ),
    aFtTextDirection( this, SchResId( FT_TEXTDIR ) ),
    aLbTextDirection( this, SchResId( LB_TEXTDIR ), &aFtTextDirection )
{
    FreeResource();

    aCbStacked.EnableTriState( sal_False );
    aOrientHlp.AddDependentWindow( aFtRotate, STATE_CHECK );

    if( !bWithRotation )
    {
        aOrientHlp.Hide();
        Point aMove( 0, aCtrlDial.GetPosPixel().Y() - aFtTextDirection.GetPosPixel().Y() );
        aFtTextDirection.SetPosPixel( aFtTextDirection.GetPosPixel() + aMove );
        aLbTextDirection.SetPosPixel( aLbTextDirection.GetPosPixel() + aMove );

        aLbTextDirection.SetHelpId( HID_SCH_TEXTDIRECTION_EQUATION );
    }
}

SchAlignmentTabPage::~SchAlignmentTabPage()
{
}

SfxTabPage* SchAlignmentTabPage::Create(Window* pParent,
                                        const SfxItemSet& rInAttrs)
{
    return new SchAlignmentTabPage(pParent, rInAttrs);
}

SfxTabPage* SchAlignmentTabPage::CreateWithoutRotation(Window* pParent,
                                        const SfxItemSet& rInAttrs)
{
    return new SchAlignmentTabPage(pParent, rInAttrs, false);
}

sal_Bool SchAlignmentTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    //Since 04/1998 text can be rotated by an arbitrary angle: SCHATTR_TEXT_DEGREES
    bool bStacked = aOrientHlp.GetStackedState() == STATE_CHECK;
    rOutAttrs.Put( SfxBoolItem( SCHATTR_TEXT_STACKED, bStacked ) );

    sal_Int32 nDegrees = bStacked ? 0 : aCtrlDial.GetRotation();
    rOutAttrs.Put( SfxInt32Item( SCHATTR_TEXT_DEGREES, nDegrees ) );

    SvxFrameDirection aDirection( aLbTextDirection.GetSelectEntryValue() );
    rOutAttrs.Put( SfxInt32Item( EE_PARA_WRITINGDIR, aDirection ) );

    return sal_True;
}

void SchAlignmentTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem* pItem = GetItem( rInAttrs, SCHATTR_TEXT_DEGREES );

    sal_Int32 nDegrees = pItem ? ((const SfxInt32Item*)pItem)->GetValue() : 0;
    aCtrlDial.SetRotation( nDegrees );

    pItem = GetItem( rInAttrs, SCHATTR_TEXT_STACKED );
    bool bStacked = pItem && ((const SfxBoolItem*)pItem)->GetValue();
    aOrientHlp.SetStackedState( bStacked ? STATE_CHECK : STATE_NOCHECK );

    if( rInAttrs.GetItemState(EE_PARA_WRITINGDIR, sal_True, &pItem) == SFX_ITEM_SET)
        aLbTextDirection.SelectEntryValue( SvxFrameDirection(((const SvxFrameDirectionItem*)pItem)->GetValue()) );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
