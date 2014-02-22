/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    mnDDStyle       = PUSHBUTTON_DROPDOWN_MENUBUTTON;

    mpMenuTimer     = NULL;
    mpMenu          = NULL;
    mpOwnMenu       = NULL;
    mnCurItemId     = 0;
    mnMenuMode      = 0;
}

void MenuButton::ImplInit( Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;

    PushButton::ImplInit( pParent, nStyle );
    EnableRTL( Application::GetSettings().GetLayoutRTL() );
}

void MenuButton::ImplExecuteMenu()
{
    Activate();

    if ( mpMenu )
    {
        Point aPos( 0, 1 );
        Size aSize = GetSizePixel();
        Rectangle aRect( aPos, aSize );
        SetPressed( true );
        EndSelection();
        mnCurItemId = mpMenu->Execute( this, aRect, POPUPMENU_EXECUTE_DOWN );
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


MenuButton::MenuButton( Window* pParent, WinBits nWinBits )
    : PushButton( WINDOW_MENUBUTTON )
{
    ImplInitMenuButtonData();
    ImplInit( pParent, nWinBits );
}

MenuButton::MenuButton( Window* pParent, const ResId& rResId )
    : PushButton( WINDOW_MENUBUTTON )
{
    ImplInitMenuButtonData();
    rResId.SetRT( RSC_MENUBUTTON );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

void MenuButton::ImplLoadRes( const ResId& rResId )
{
    Control::ImplLoadRes( rResId );

    sal_uLong nObjMask = ReadLongRes();

    if ( RSCMENUBUTTON_MENU & nObjMask )
    {
        mpOwnMenu = new PopupMenu( ResId( (RSHEADER_TYPE*)GetClassRes(), *rResId.GetResMgr() ) );
        SetPopupMenu( mpOwnMenu );
        IncrementRes( GetObjSizeRes( (RSHEADER_TYPE*)GetClassRes() ) );
    }
}

MenuButton::~MenuButton()
{
    delete mpMenuTimer;
    delete mpOwnMenu;
}

IMPL_LINK_NOARG(MenuButton, ImplMenuTimeoutHdl)
{
    
    if ( IsTracking() )
    {
        if ( !(GetStyle() & WB_NOPOINTERFOCUS) )
            GrabFocus();
        ImplExecuteMenu();
    }

    return 0;
}


void MenuButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bExecute = true;
    if ( mnMenuMode & MENUBUTTON_MENUMODE_TIMED )
    {
        
        if( mnDDStyle != PUSHBUTTON_DROPDOWN_MENUBUTTON || 
            rMEvt.GetPosPixel().X() <= ImplGetSeparatorX() )
        {
            if ( !mpMenuTimer )
            {
                mpMenuTimer = new Timer;
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
            ImplExecuteMenu();
        }
    }
}


void MenuButton::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aKeyCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aKeyCode.GetCode();
    if ( (nCode == KEY_DOWN) && aKeyCode.IsMod2() )
        ImplExecuteMenu();
    else if ( !(mnMenuMode & MENUBUTTON_MENUMODE_TIMED) &&
              !aKeyCode.GetModifier() &&
              ((nCode == KEY_RETURN) || (nCode == KEY_SPACE)) )
        ImplExecuteMenu();
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
    
    mnMenuMode = nMode;
}

void MenuButton::SetPopupMenu( PopupMenu* pNewMenu )
{
    if (pNewMenu == mpMenu)
        return;

    mpMenu = pNewMenu;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
