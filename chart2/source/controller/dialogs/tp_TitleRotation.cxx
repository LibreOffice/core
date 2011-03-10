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
#include "precompiled_chart2.hxx"
#include "tp_TitleRotation.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "chartview/ChartSfxItemIds.hxx"
#include "HelpIds.hrc"
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>

// header for class SfxInt32Item
#include <svl/intitem.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

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

//.............................................................................
} //namespace chart
//.............................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
