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

#include <svl/intitem.hxx>
#include <vcl/toolbox.hxx>

#include <strings.hrc>

#include <diactrl.hxx>

#include <sdresid.hxx>
#include <app.hrc>

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>

using namespace ::com::sun::star;

SFX_IMPL_TOOLBOX_CONTROL( SdTbxCtlDiaPages,  SfxUInt16Item )

// SdPagesField
SdPagesField::SdPagesField( vcl::Window* pParent,
                            const uno::Reference< frame::XFrame >& rFrame ) :
    SvxMetricField  ( pParent, rFrame ),
    m_xFrame        ( rFrame )
{
    OUString aStr( SdResId( STR_SLIDE_PLURAL ) );
//TODO    SetCustomUnitText( aStr );

    // set parameter of MetricFields
    m_xWidget->set_digits(0);
    m_xWidget->set_unit(FieldUnit::CUSTOM);
    m_xWidget->set_range(1, 15, FieldUnit::CUSTOM);
    m_xWidget->set_increments(1, 5, FieldUnit::CUSTOM);

    SetSizePixel(m_xWidget->get_preferred_size());
}

SdPagesField::~SdPagesField()
{
}

void SdPagesField::UpdatePagesField( const SfxUInt16Item* pItem )
{
    if( pItem )
    {
        long nValue = static_cast<long>(pItem->GetValue());
        m_xWidget->set_value(nValue, FieldUnit::CUSTOM);
#if 0
        if( nValue == 1 )
            SetCustomUnitText( SdResId( STR_SLIDE_SINGULAR ) );
        else
            SetCustomUnitText( SdResId( STR_SLIDE_PLURAL ) );
#endif
    }
    else
        m_xWidget->set_text(OUString());
}

void SdPagesField::Modify()
{
    SfxUInt16Item aItem(SID_PAGES_PER_ROW, m_xWidget->get_value(FieldUnit::CUSTOM));

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

SdTbxCtlDiaPages::~SdTbxCtlDiaPages()
{
}

void SdTbxCtlDiaPages::StateChanged( sal_uInt16,
                SfxItemState eState, const SfxPoolItem* pState )
{
    SdPagesField* pFld = static_cast<SdPagesField*>( GetToolBox().GetItemWindow( GetId() ) );
    DBG_ASSERT( pFld, "Window not found" );

    if ( eState == SfxItemState::DISABLED )
    {
        pFld->set_sensitive(false);
    }
    else
    {
        pFld->set_sensitive(true);

        const SfxUInt16Item* pItem = nullptr;
        if ( eState == SfxItemState::DEFAULT )
        {
            pItem = dynamic_cast< const SfxUInt16Item* >( pState );
            DBG_ASSERT( pItem, "sd::SdTbxCtlDiaPages::StateChanged(), wrong item type!" );
        }

        pFld->UpdatePagesField( pItem );
    }
}

VclPtr<vcl::Window> SdTbxCtlDiaPages::CreateItemWindow( vcl::Window* pParent )
{
    VclPtr<SdPagesField> pWindow = VclPtr<SdPagesField>::Create(pParent, m_xFrame);
    pWindow->Show();

    return pWindow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
