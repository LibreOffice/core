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


#include <tools/shl.hxx>
#include <vcl/status.hxx>
#include <vcl/menu.hxx>
#include <vcl/image.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <svl/eitem.hxx>
#include <xmlsecurity/stbcontrl.hxx>

#define PAINT_OFFSET    5

SFX_IMPL_STATUSBAR_CONTROL( XmlSecStatusBarControl, SfxBoolItem );


struct XmlSecStatusBarControl::XmlSecStatusBarControl_Impl
{
    Point       maPos;
    Size        maSize;
    bool        mbSigned;
    Image       maImage;
};


XmlSecStatusBarControl::XmlSecStatusBarControl( sal_uInt16 _nId, StatusBar& _rStb, SfxBindings& _rBind )
    :SfxStatusBarControl( _nId, _rStb, _rBind )

    ,mpImpl( new XmlSecStatusBarControl_Impl )
{
    mpImpl->mbSigned = false;
}

XmlSecStatusBarControl::~XmlSecStatusBarControl()
{
    delete mpImpl;
}

void XmlSecStatusBarControl::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    GetStatusBar().SetHelpText( GetId(), String() );    // necessary ?
    GetStatusBar().SetHelpId( GetId(), nSID );          // necessary ?

    if( SFX_ITEM_AVAILABLE != eState )
    {
        mpImpl->mbSigned = false;
    }
    else if( pState->ISA( SfxBoolItem ) )
    {
        mpImpl->mbSigned = ( ( SfxBoolItem* ) pState )->GetValue();
    }
    else
    {
        SAL_WARN( "xmlsecurity.dialogs", "+XmlSecStatusBarControl::StateChanged(): invalid item type" );
        mpImpl->mbSigned = false;
    }

    if( GetStatusBar().AreItemsVisible() )              // necessary ?
        GetStatusBar().SetItemData( GetId(), 0 );

    GetStatusBar().SetItemText( GetId(), String() );    // necessary ?
}

void XmlSecStatusBarControl::Command( const CommandEvent& rCEvt )
{
    // can / has to be done when integrated in Office!
    SfxStatusBarControl::Command( rCEvt );
}

void XmlSecStatusBarControl::Paint( const UserDrawEvent& rUsrEvt )
{
    OutputDevice*       pDev = rUsrEvt.GetDevice();
    DBG_ASSERT( pDev, "-XmlSecStatusBarControl::Paint(): no Output Device... this will lead to nirvana..." );
    const Rectangle&    rRect = rUsrEvt.GetRect();
    StatusBar&          rBar = GetStatusBar();
    Point               aItemPos = rBar.GetItemTextPos( GetId() );
    Color               aOldLineColor = pDev->GetLineColor();
    Color               aOldFillColor = pDev->GetFillColor();

    // just 4 testing until we've got a bitmap
    pDev->SetLineColor();
    pDev->SetFillColor( pDev->GetBackground().GetColor() );

    String              s( String::CreateFromAscii( mpImpl->mbSigned? "X" : "-" ) );
    pDev->DrawRect( rRect );
    pDev->DrawText( Point( rRect.Left() + rRect.GetWidth() / 2 - pDev->GetTextWidth( s ) / 2, aItemPos.Y() ), s );

    pDev->SetLineColor( aOldLineColor );
    pDev->SetFillColor( aOldFillColor );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
