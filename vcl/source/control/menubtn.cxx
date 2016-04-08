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

#include <tools/rc.h>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/menu.hxx>
#include <vcl/timer.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

void MenuButton::ImplInitMenuButtonData()
{
    mnDDStyle       = PushButtonDropdownStyle::MenuButton;

    mpMenuTimer     = nullptr;
    mpMenu          = nullptr;
    mpOwnMenu       = nullptr;
    mnCurItemId     = 0;
    mnMenuMode      = 0;
}

void MenuButton::ImplInit( vcl::Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;

    PushButton::ImplInit( pParent, nStyle );
    EnableRTL( AllSettings::GetLayoutRTL() );
}

void MenuButton::ExecuteMenu()
{
    Activate();

    if ( mpMenu )
    {
        Point aPos( 0, 1 );
        Size aSize = GetSizePixel();
        Rectangle aRect( aPos, aSize );
        SetPressed( true );
        EndSelection();
        mnCurItemId = mpMenu->Execute( this, aRect, PopupMenuFlags::ExecuteDown );
        SetPressed( false );
        if ( mnCurItemId )
        {
            Select();
            mnCurItemId = 0;
        }
    }
}

OString MenuButton::GetCurItemIdent() const
{
    return (mnCurItemId && mpMenu) ?
        mpMenu->GetItemIdent(mnCurItemId) : OString();
}

MenuButton::MenuButton( vcl::Window* pParent, WinBits nWinBits )
    : PushButton( WINDOW_MENUBUTTON )
{
    ImplInitMenuButtonData();
    ImplInit( pParent, nWinBits );
}

MenuButton::~MenuButton()
{
    disposeOnce();
}

void MenuButton::dispose()
{
    delete mpMenuTimer;
    delete mpOwnMenu;
    PushButton::dispose();
}

IMPL_LINK_NOARG_TYPED(MenuButton, ImplMenuTimeoutHdl, Timer *, void)
{
    // See if Button Tracking is still active, as it could've been cancelled earlier
    if ( IsTracking() )
    {
        if ( !(GetStyle() & WB_NOPOINTERFOCUS) )
            GrabFocus();
        ExecuteMenu();
    }
}

void MenuButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bExecute = true;
    if ( mnMenuMode & MENUBUTTON_MENUMODE_TIMED )
    {
        // If the separated dropdown symbol is not hit, delay the popup execution
        if( mnDDStyle != PushButtonDropdownStyle::MenuButton || // no separator at all
            rMEvt.GetPosPixel().X() <= ImplGetSeparatorX() )
        {
            if ( !mpMenuTimer )
            {
                mpMenuTimer = new Timer("MenuTimer");
                mpMenuTimer->SetTimeoutHdl( LINK( this, MenuButton, ImplMenuTimeoutHdl ) );
            }

            mpMenuTimer->SetTimeout( GetSettings().GetMouseSettings().GetActionDelay() );
            mpMenuTimer->Start();

            PushButton::MouseButtonDown( rMEvt );
            bExecute = false;
        }
    }
    if( bExecute )
    {
        if ( PushButton::ImplHitTestPushButton( this, rMEvt.GetPosPixel() ) )
        {
            if ( !(GetStyle() & WB_NOPOINTERFOCUS) )
                GrabFocus();
            ExecuteMenu();
        }
    }
}

void MenuButton::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aKeyCode.GetCode();
    if ( (nCode == KEY_DOWN) && aKeyCode.IsMod2() )
        ExecuteMenu();
    else if ( !(mnMenuMode & MENUBUTTON_MENUMODE_TIMED) &&
              !aKeyCode.GetModifier() &&
              ((nCode == KEY_RETURN) || (nCode == KEY_SPACE)) )
        ExecuteMenu();
    else
        PushButton::KeyInput( rKEvt );
}

void MenuButton::Activate()
{
    maActivateHdl.Call( this );
}

void MenuButton::Select()
{
    maSelectHdl.Call( this );
}

void MenuButton::SetMenuMode( sal_uInt16 nMode )
{
    // FIXME: It's better to not inline this for 5.1; in 6.0 we can make it inline, however
    mnMenuMode = nMode;
}

void MenuButton::SetPopupMenu( PopupMenu* pNewMenu )
{
    if (pNewMenu == mpMenu)
        return;

    mpMenu = pNewMenu;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
