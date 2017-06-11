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


#include <vcl/status.hxx>
#include <vcl/builder.hxx>
#include <vcl/menu.hxx>
#include <vcl/image.hxx>
#include <sfx2/signaturestate.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/sfxsids.hrc>

#include <svl/intitem.hxx>

#include <svl/eitem.hxx>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include "svx/xmlsecctrl.hxx"
#include <tools/urlobj.hxx>
#include "bitmaps.hlst"

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

    mpImpl->maImage             = Image(BitmapEx(RID_SVXBMP_SIGNET));
    mpImpl->maImageBroken       = Image(BitmapEx(RID_SVXBMP_SIGNET_BROKEN));
    mpImpl->maImageNotValidated = Image(BitmapEx(RID_SVXBMP_SIGNET_NOTVALIDATED));
}

XmlSecStatusBarControl::~XmlSecStatusBarControl()
{
}

void XmlSecStatusBarControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )
{
    if( SfxItemState::DEFAULT != eState )
    {
        mpImpl->mnState = SignatureState::UNKNOWN;
    }
    else if( dynamic_cast< const SfxUInt16Item* >(pState) !=  nullptr )
    {
        mpImpl->mnState = static_cast<SignatureState>(static_cast<const SfxUInt16Item*>(pState)->GetValue());
    }
    else
    {
        SAL_WARN( "svx.stbcrtls", "+XmlSecStatusBarControl::StateChanged(): invalid item type" );
        mpImpl->mnState = SignatureState::UNKNOWN;
    }

    if( GetStatusBar().AreItemsVisible() )              // necessary ?
        GetStatusBar().SetItemData( GetId(), nullptr );

    GetStatusBar().SetItemText( GetId(), "" );    // necessary ?

    const char* pResId = RID_SVXSTR_XMLSEC_NO_SIG;
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
        VclBuilder aBuilder(nullptr, VclBuilderContainer::getUIRootDir(), "svx/ui/xmlsecstatmenu.ui", "");
        VclPtr<PopupMenu> aPopupMenu(aBuilder.get_menu("menu"));
        if (aPopupMenu->Execute(&GetStatusBar(), rCEvt.GetMousePosPixel()))
        {
            css::uno::Any a;
            SfxUInt16Item aState( GetSlotId(), 0 );
            INetURLObject aObj( m_aCommandURL );

            css::uno::Sequence< css::beans::PropertyValue > aArgs( 1 );
            aArgs[0].Name  = aObj.GetURLPath();
            aState.QueryValue( a );
            aArgs[0].Value = a;

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

    long yOffset = (aRect.GetHeight() - mpImpl->maImage.GetSizePixel().Height()) / 2;

    if( mpImpl->mnState == SignatureState::OK )
    {
        aRect.Top() += yOffset;
        pDev->DrawImage( aRect.TopLeft(), mpImpl->maImage );
    }
    else if( mpImpl->mnState == SignatureState::BROKEN )
    {
        aRect.Top() += yOffset;
        pDev->DrawImage( aRect.TopLeft(), mpImpl->maImageBroken );
    }
    else if( mpImpl->mnState == SignatureState::NOTVALIDATED
        || mpImpl->mnState == SignatureState::PARTIAL_OK)
    {
        aRect.Top() += yOffset;
        pDev->DrawImage( aRect.TopLeft(), mpImpl->maImageNotValidated );
    }
    else
        pDev->DrawRect( aRect );

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
