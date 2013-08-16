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

#include <com/sun/star/presentation/FadeEffect.hpp>

#include <svx/dialogs.hrc>

#include "sdattr.hxx"
#include "strings.hrc"

#include "diactrl.hxx"

#include "sdresid.hxx"
#include "app.hrc"
#include "res_bmp.hrc"
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/app.hxx>

using namespace ::com::sun::star;

SFX_IMPL_TOOLBOX_CONTROL( SdTbxCtlDiaPages,  SfxUInt16Item )


//========================================================================
// SdPagesField
//========================================================================

SdPagesField::SdPagesField( Window* pParent,
                            const uno::Reference< frame::XFrame >& rFrame,
                            WinBits nBits ) :
    SvxMetricField  ( pParent, rFrame, nBits ),
    m_xFrame        ( rFrame )
{
    OUString aStr( SD_RESSTR( STR_SLIDE_PLURAL ) );
    SetCustomUnitText( aStr );

    // set size
    aStr += "XXX";
    Size aSize( GetTextWidth( aStr )+20, GetTextHeight()+6 );


    SetSizePixel( aSize );

    // set parameter of MetricFields
    SetUnit( FUNIT_CUSTOM );
    SetMin( 1 );
    SetFirst( 1 );
    SetMax( 15 );
    SetLast( 15 );
    SetSpinSize( 1 );
    SetDecimalDigits( 0 );
    Show();
}

// -----------------------------------------------------------------------

SdPagesField::~SdPagesField()
{
}

// -----------------------------------------------------------------------

void SdPagesField::UpdatePagesField( const SfxUInt16Item* pItem )
{
    if( pItem )
    {
        long nValue = (long) pItem->GetValue();
        SetValue( nValue );
        if( nValue == 1 )
            SetCustomUnitText( SD_RESSTR( STR_SLIDE_SINGULAR ) );
        else
            SetCustomUnitText( SD_RESSTR( STR_SLIDE_PLURAL ) );
    }
    else
        SetText( OUString() );
}

// -----------------------------------------------------------------------

void SdPagesField::Modify()
{
    SfxUInt16Item aItem( SID_PAGES_PER_ROW, (sal_uInt16) GetValue() );

    ::uno::Any a;
    ::uno::Sequence< ::beans::PropertyValue > aArgs( 1 );
    aArgs[0].Name   = "PagesPerRow";
    aItem.QueryValue( a );
    aArgs[0].Value  = a;
    SfxToolBoxControl::Dispatch( ::uno::Reference< ::frame::XDispatchProvider >( m_xFrame->getController(), ::uno::UNO_QUERY ),
                                 ".uno:PagesPerRow",
                                 aArgs );
}

SdTbxCtlDiaPages::SdTbxCtlDiaPages( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

//========================================================================

SdTbxCtlDiaPages::~SdTbxCtlDiaPages()
{
}

//========================================================================

void SdTbxCtlDiaPages::StateChanged( sal_uInt16,
                SfxItemState eState, const SfxPoolItem* pState )
{
    SdPagesField* pFld = (SdPagesField*) GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pFld, "Window not found" );

    if ( eState == SFX_ITEM_DISABLED )
    {
        pFld->Disable();
        pFld->SetText( OUString() );
    }
    else
    {
        pFld->Enable();

        const SfxUInt16Item* pItem = 0;
        if ( eState == SFX_ITEM_AVAILABLE )
        {
            pItem = dynamic_cast< const SfxUInt16Item* >( pState );
            DBG_ASSERT( pItem, "sd::SdTbxCtlDiaPages::StateChanged(), wrong item type!" );
        }

        pFld->UpdatePagesField( pItem );
    }
}

//========================================================================

Window* SdTbxCtlDiaPages::CreateItemWindow( Window* pParent )
{
    return new SdPagesField( pParent, m_xFrame );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
