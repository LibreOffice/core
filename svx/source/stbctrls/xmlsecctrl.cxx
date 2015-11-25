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

#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include "svx/xmlsecctrl.hxx"
#include <tools/urlobj.hxx>

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

    mpImpl->maImage             = Image( SVX_RES( RID_SVXBMP_SIGNET              ) );
    mpImpl->maImageBroken       = Image( SVX_RES( RID_SVXBMP_SIGNET_BROKEN       ) );
    mpImpl->maImageNotValidated = Image( SVX_RES( RID_SVXBMP_SIGNET_NOTVALIDATED ) );

    if (_rStb.GetDPIScaleFactor() > 1)
    {
        Image arr[3] = {mpImpl->maImage, mpImpl->maImageBroken, mpImpl->maImageNotValidated};

        for (int i = 0; i < 3; i++)
        {
            BitmapEx b = arr[i].GetBitmapEx();
            b.Scale(_rStb.GetDPIScaleFactor(), _rStb.GetDPIScaleFactor(), BmpScaleFlag::Fast);
            arr[i] = Image(b);
        }

        mpImpl->maImage = arr[0];
        mpImpl->maImageBroken = arr[1];
        mpImpl->maImageNotValidated = arr[2];
    }

}

XmlSecStatusBarControl::~XmlSecStatusBarControl()
{
    delete mpImpl;
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

    sal_uInt16 nResId = RID_SVXSTR_XMLSEC_NO_SIG;
    if ( mpImpl->mnState == SignatureState::OK )
        nResId = RID_SVXSTR_XMLSEC_SIG_OK;
    else if ( mpImpl->mnState == SignatureState::BROKEN )
        nResId = RID_SVXSTR_XMLSEC_SIG_NOT_OK;
    else if ( mpImpl->mnState == SignatureState::NOTVALIDATED )
        nResId = RID_SVXSTR_XMLSEC_SIG_OK_NO_VERIFY;
    else if ( mpImpl->mnState == SignatureState::PARTIAL_OK )
        nResId = RID_SVXSTR_XMLSEC_SIG_CERT_OK_PARTIAL_SIG;

    GetStatusBar().SetQuickHelpText( GetId(), SVX_RESSTR( nResId ) );
}

void XmlSecStatusBarControl::Command( const CommandEvent& rCEvt )
{
    if( rCEvt.GetCommand() == CommandEventId::ContextMenu )
    {
        PopupMenu aPopupMenu( ResId( RID_SVXMNU_XMLSECSTATBAR, DIALOG_MGR() ) );
        if( aPopupMenu.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel() ) )
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

    Rectangle           aRect = rUsrEvt.GetRect();
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
