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
#include <vcl/floatwin.hxx>
#include <vcl/menu.hxx>
#include <vcl/timer.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

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

    if (!mpMenu && !mpFloatingWindow)
        return;

    Size aSize = GetSizePixel();
    SetPressed( true );
    EndSelection();
    if (mpMenu)
    {
        Point aPos(0, 1);
        Rectangle aRect(aPos, aSize );
        mnCurItemId = mpMenu->Execute(this, aRect, PopupMenuFlags::ExecuteDown);
    }
    else
    {
        Point aPos(GetParent()->OutputToScreenPixel(GetPosPixel()));
        Rectangle aRect(aPos, aSize );
        mpFloatingWindow->StartPopupMode(aRect, FloatWinPopupFlags::Down | FloatWinPopupFlags::GrabFocus);
    }
    SetPressed(false);
    if (mnCurItemId)
    {
        Select();
        mnCurItemId = 0;
    }
}

OString MenuButton::GetCurItemIdent() const
{
    return (mnCurItemId && mpMenu) ?
        mpMenu->GetItemIdent(mnCurItemId) : OString();
}

MenuButton::MenuButton( vcl::Window* pParent, WinBits nWinBits )
    : PushButton(WindowType::MENUBUTTON)
    , mpMenuTimer(nullptr)
    , mnCurItemId(0)
    , mbDelayMenu(false)
{
    mnDDStyle = PushButtonDropdownStyle::MenuButton;
    ImplInit(pParent, nWinBits);
}

MenuButton::~MenuButton()
{
    disposeOnce();
}

void MenuButton::dispose()
{
    delete mpMenuTimer;
    mpFloatingWindow.clear();
    mpMenu.clear();
    PushButton::dispose();
}

IMPL_LINK_NOARG(MenuButton, ImplMenuTimeoutHdl, Timer *, void)
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
    if (mbDelayMenu)
    {
        // If the separated dropdown symbol is not hit, delay the popup execution
        if( mnDDStyle == PushButtonDropdownStyle::Toolbox || // no separator at all
            rMEvt.GetPosPixel().X() <= ImplGetSeparatorX() )
        {
            if ( !mpMenuTimer )
            {
                mpMenuTimer = new Timer("MenuTimer");
                mpMenuTimer->SetInvokeHandler( LINK( this, MenuButton, ImplMenuTimeoutHdl ) );
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
    else if ( !mbDelayMenu &&
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

void MenuButton::SetPopupMenu(PopupMenu* pNewMenu)
{
    if (pNewMenu == mpMenu)
        return;

    mpMenu = pNewMenu;
}

void MenuButton::SetPopover(FloatingWindow* pFloatingWindow)
{
    if (pFloatingWindow == mpFloatingWindow)
        return;

    mpFloatingWindow = pFloatingWindow;
}

//class MenuToggleButton ----------------------------------------------------

MenuToggleButton::MenuToggleButton( vcl::Window* pParent, WinBits nWinBits )
    : MenuButton( pParent, nWinBits )
{
}

MenuToggleButton::~MenuToggleButton()
{
    disposeOnce();
}

void MenuToggleButton::SetActive( bool bSel )
{
    mbIsActive = bSel;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
