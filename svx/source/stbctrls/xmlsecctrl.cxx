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

#include <sal/config.h>

#include <comphelper/propertyvalue.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/image.hxx>
#include <vcl/event.hxx>
#include <vcl/status.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <sfx2/signaturestate.hxx>
#include <sfx2/module.hxx>

#include <svl/intitem.hxx>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xmlsecctrl.hxx>
#include <tools/urlobj.hxx>
#include <bitmaps.hlst>
#include <sal/log.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>

SFX_IMPL_STATUSBAR_CONTROL( XmlSecStatusBarControl, SfxUInt16Item );

struct XmlSecStatusBarControl::XmlSecStatusBarControl_Impl
{
    SignatureState  mnState;
    Image       maImage;
    Image       maImageBroken;
    Image       maImageNotValidated;
};


XmlSecStatusBarControl::XmlSecStatusBarControl( sal_uInt16 _nSlotId,  sal_uInt16 _nId, StatusBar& _rStb )
    :SfxStatusBarControl( _nSlotId, _nId, _rStb )
    ,mpImpl( new XmlSecStatusBarControl_Impl )
{
    mpImpl->mnState = SignatureState::UNKNOWN;

    mpImpl->maImage             = Image(StockImage::Yes, RID_SVXBMP_SIGNET);
    mpImpl->maImageBroken       = Image(StockImage::Yes, RID_SVXBMP_SIGNET_BROKEN);
    mpImpl->maImageNotValidated = Image(StockImage::Yes, RID_SVXBMP_SIGNET_NOTVALIDATED);
}

XmlSecStatusBarControl::~XmlSecStatusBarControl()
{
}

void XmlSecStatusBarControl::StateChangedAtStatusBarControl( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )
{
    if( SfxItemState::DEFAULT != eState )
    {
        mpImpl->mnState = SignatureState::UNKNOWN;
    }
    else if( auto pUint16Item = dynamic_cast< const SfxUInt16Item* >(pState) )
    {
        mpImpl->mnState = static_cast<SignatureState>(pUint16Item->GetValue());
    }
    else
    {
        SAL_WARN( "svx.stbcrtls", "+XmlSecStatusBarControl::StateChangedAtStatusBarControl(): invalid item type" );
        mpImpl->mnState = SignatureState::UNKNOWN;
    }

    GetStatusBar().SetItemData( GetId(), nullptr ); // necessary ?

    GetStatusBar().SetItemText( GetId(), u""_ustr );    // necessary ?

    TranslateId pResId = RID_SVXSTR_XMLSEC_NO_SIG;
    if ( mpImpl->mnState == SignatureState::OK )
        pResId = RID_SVXSTR_XMLSEC_SIG_OK;
    else if ( mpImpl->mnState == SignatureState::BROKEN )
        pResId = RID_SVXSTR_XMLSEC_SIG_NOT_OK;
    else if ( mpImpl->mnState == SignatureState::NOTVALIDATED )
        pResId = RID_SVXSTR_XMLSEC_SIG_OK_NO_VERIFY;
    else if ( mpImpl->mnState == SignatureState::PARTIAL_OK )
        pResId = RID_SVXSTR_XMLSEC_SIG_CERT_OK_PARTIAL_SIG;

    GetStatusBar().SetQuickHelpText(GetId(), SvxResId(pResId));
}

void XmlSecStatusBarControl::Command( const CommandEvent& rCEvt )
{
    if( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        tools::Rectangle aRect(rCEvt.GetMousePosPixel(), Size(1, 1));
        weld::Window* pPopupParent = weld::GetPopupParent(GetStatusBar(), aRect);
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pPopupParent, u"svx/ui/xmlsecstatmenu.ui"_ustr));
        std::unique_ptr<weld::Menu> xPopMenu(xBuilder->weld_menu(u"menu"_ustr));
        if (!xPopMenu->popup_at_rect(pPopupParent, aRect).isEmpty())
        {
            css::uno::Any a;
            SfxUInt16Item aState( GetSlotId(), 0 );
            aState.QueryValue( a );
            INetURLObject aObj( m_aCommandURL );

            css::uno::Sequence< css::beans::PropertyValue > aArgs{ comphelper::makePropertyValue(
                aObj.GetURLPath(), a) };
            execute( aArgs );
        }
    }
    else
        SfxStatusBarControl::Command( rCEvt );
}

void XmlSecStatusBarControl::Paint( const UserDrawEvent& rUsrEvt )
{
    vcl::RenderContext* pDev = rUsrEvt.GetRenderContext();

    tools::Rectangle           aRect = rUsrEvt.GetRect();
    Color               aOldLineColor = pDev->GetLineColor();
    Color               aOldFillColor = pDev->GetFillColor();

    pDev->SetLineColor();
    pDev->SetFillColor( pDev->GetBackground().GetColor() );

    tools::Long yOffset = (aRect.GetHeight() - mpImpl->maImage.GetSizePixel().Height()) / 2;

    if( mpImpl->mnState == SignatureState::OK )
    {
        aRect.AdjustTop(yOffset );
        pDev->DrawImage( aRect.TopLeft(), mpImpl->maImage );
    }
    else if( mpImpl->mnState == SignatureState::BROKEN )
    {
        aRect.AdjustTop(yOffset );
        pDev->DrawImage( aRect.TopLeft(), mpImpl->maImageBroken );
    }
    else if( mpImpl->mnState == SignatureState::NOTVALIDATED
        || mpImpl->mnState == SignatureState::PARTIAL_OK)
    {
        aRect.AdjustTop(yOffset );
        pDev->DrawImage( aRect.TopLeft(), mpImpl->maImageNotValidated );
    }
    else
        pDev->DrawRect( aRect );

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
