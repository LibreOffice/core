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

#include <vcl/dockwin.hxx>
#include <vcl/event.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/menu.hxx>
#include <vcl/timer.hxx>
#include <vcl/toolkit/menubtn.hxx>
#include <vcl/settings.hxx>
#include <vcl/uitest/uiobject.hxx>
#include <vcl/uitest/logger.hxx>
#include <vcl/uitest/eventdescription.hxx>
#include <menutogglebutton.hxx>

namespace
{
void collectUIInformation( const OUString& aID, const OUString& aevent , const OUString& akey , const OUString& avalue)
{
    EventDescription aDescription;
    aDescription.aID = aID;
    aDescription.aParameters = {{ akey ,  avalue}};
    aDescription.aAction = aevent;
    aDescription.aParent = "MainWindow";
    aDescription.aKeyWord = "MenuButton";
    UITestLogger::getInstance().logEvent(aDescription);
}
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
    mbStartingMenu = true;

    Activate();

    if (!mpMenu && !mpFloatingWindow)
    {
        mbStartingMenu = false;
        return;
    }

    Size aSize = GetSizePixel();
    SetPressed( true );
    EndSelection();
    if (mpMenu)
    {
        Point aPos(0, 1);
        tools::Rectangle aRect(aPos, aSize );
        mpMenu->Execute(this, aRect, PopupMenuFlags::ExecuteDown);

        if (IsDisposed())
            return;

        mnCurItemId = mpMenu->GetCurItemId();
        msCurItemIdent = mpMenu->GetCurItemIdent();
    }
    else
    {
        Point aPos(GetParent()->OutputToScreenPixel(GetPosPixel()));
        tools::Rectangle aRect(aPos, aSize );
        FloatWinPopupFlags nFlags = FloatWinPopupFlags::Down | FloatWinPopupFlags::GrabFocus;
        if (mpFloatingWindow->GetType() == WindowType::FLOATINGWINDOW)
            static_cast<FloatingWindow*>(mpFloatingWindow.get())->StartPopupMode(aRect, nFlags);
        else
        {
            mpFloatingWindow->EnableDocking();
            vcl::Window::GetDockingManager()->StartPopupMode(mpFloatingWindow, aRect, nFlags);
        }
    }

    mbStartingMenu = false;

    SetPressed(false);
    OUString aID = get_id(); // tdf#136678 take a copy if we are destroyed by Select callback
    if (mnCurItemId)
    {
        Select();
        mnCurItemId = 0;
        msCurItemIdent.clear();
    }
    collectUIInformation(aID,"OPENLIST","","");
}

void MenuButton::CancelMenu()
{
    if (!mpMenu && !mpFloatingWindow)
        return;

    if (mpMenu)
    {
        mpMenu->EndExecute();
    }
    else
    {
        if (mpFloatingWindow->GetType() == WindowType::FLOATINGWINDOW)
            static_cast<FloatingWindow*>(mpFloatingWindow.get())->EndPopupMode();
        else
            vcl::Window::GetDockingManager()->EndPopupMode(mpFloatingWindow);
    }
    collectUIInformation(get_id(),"CLOSELIST","","");
}

bool MenuButton::InPopupMode() const
{
    if (mbStartingMenu)
        return true;

    if (!mpMenu && !mpFloatingWindow)
        return false;

    if (mpMenu)
       return PopupMenu::GetActivePopupMenu() == mpMenu;
    else
    {
        if (mpFloatingWindow->GetType() == WindowType::FLOATINGWINDOW)
            return static_cast<const FloatingWindow*>(mpFloatingWindow.get())->IsInPopupMode();
        else
            return vcl::Window::GetDockingManager()->IsInPopupMode(mpFloatingWindow);
    }
}

MenuButton::MenuButton( vcl::Window* pParent, WinBits nWinBits )
    : PushButton(WindowType::MENUBUTTON)
    , mnCurItemId(0)
    , mbDelayMenu(false)
    , mbStartingMenu(false)
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
    mpMenuTimer.reset();
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
        if( rMEvt.GetPosPixel().X() <= ImplGetSeparatorX() )
        {
            if ( !mpMenuTimer )
            {
                mpMenuTimer.reset(new Timer("MenuTimer"));
                mpMenuTimer->SetInvokeHandler( LINK( this, MenuButton, ImplMenuTimeoutHdl ) );
            }

            mpMenuTimer->SetTimeout( MouseSettings::GetActionDelay() );
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
    if (mnCurItemId)
        collectUIInformation(get_id(),"OPENFROMLIST","POS",OUString::number(mnCurItemId));

    maSelectHdl.Call( this );
}

void MenuButton::SetPopupMenu(PopupMenu* pNewMenu)
{
    if (pNewMenu == mpMenu)
        return;

    mpMenu = pNewMenu;
}

void MenuButton::SetPopover(Window* pWindow)
{
    if (pWindow == mpFloatingWindow)
        return;

    mpFloatingWindow = pWindow;
}


FactoryFunction MenuButton::GetUITestFactory() const
{
    return MenuButtonUIObject::create;
}

void MenuButton::SetCurItemId(){
    mnCurItemId = mpMenu->GetCurItemId();
    msCurItemIdent = mpMenu->GetCurItemIdent();
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

bool MenuToggleButton::GetActive() const
{
    return mbIsActive;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
