/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <tools/rc.h>
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/menu.hxx>
#include <vcl/timer.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/svapp.hxx>

// =======================================================================

void MenuButton::ImplInitMenuButtonData()
{
    mnDDStyle       = PUSHBUTTON_DROPDOWN_MENUBUTTON;

    mpMenuTimer     = NULL;
    mpMenu          = NULL;
    mpOwnMenu       = NULL;
    mnCurItemId     = 0;
    mnMenuMode      = 0;
}

// -----------------------------------------------------------------------

void MenuButton::ImplInit( Window* pParent, WinBits nStyle )
{
    if ( !(nStyle & WB_NOTABSTOP) )
        nStyle |= WB_TABSTOP;

    PushButton::ImplInit( pParent, nStyle );
    EnableRTL( Application::GetSettings().GetLayoutRTL() );
}

// -----------------------------------------------------------------------

void MenuButton::ImplExecuteMenu()
{
    Activate();

    if ( mpMenu )
    {
        Point aPos( 0, 1 );
        Size aSize = GetSizePixel();
        Rectangle aRect( aPos, aSize );
        SetPressed( sal_True );
        EndSelection();
        mnCurItemId = mpMenu->Execute( this, aRect, POPUPMENU_EXECUTE_DOWN );
        SetPressed( sal_False );
        if ( mnCurItemId )
        {
            Select();
            mnCurItemId = 0;
        }
    }
}

// -----------------------------------------------------------------------

MenuButton::MenuButton( Window* pParent, WinBits nWinBits ) :
    PushButton( WINDOW_MENUBUTTON )
{
    ImplInitMenuButtonData();
    ImplInit( pParent, nWinBits );
}

// -----------------------------------------------------------------------

MenuButton::MenuButton( Window* pParent, const ResId& rResId ) :
    PushButton( WINDOW_MENUBUTTON )
{
    ImplInitMenuButtonData();
    rResId.SetRT( RSC_MENUBUTTON );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

MenuButton::~MenuButton()
{
    delete mpMenuTimer;
    delete mpOwnMenu;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(MenuButton, ImplMenuTimeoutHdl)
{
    // Abfragen, ob Button-Benutzung noch aktiv ist, da diese ja auch
    // vorher abgebrochen wurden sein koennte
    if ( IsTracking() )
    {
        if ( !(GetStyle() & WB_NOPOINTERFOCUS) )
            GrabFocus();
        ImplExecuteMenu();
    }

    return 0;
}

// -----------------------------------------------------------------------

void MenuButton::MouseButtonDown( const MouseEvent& rMEvt )
{
    bool bExecute = true;
    if ( mnMenuMode & MENUBUTTON_MENUMODE_TIMED )
    {
        // if the separated dropdown symbol is not hit, delay the popup execution
        if( mnDDStyle != PUSHBUTTON_DROPDOWN_MENUBUTTON || // no separator at all
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

// -----------------------------------------------------------------------

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

// -----------------------------------------------------------------------

void MenuButton::Activate()
{
    maActivateHdl.Call( this );
}

// -----------------------------------------------------------------------

void MenuButton::Select()
{
    maSelectHdl.Call( this );
}

// -----------------------------------------------------------------------

void MenuButton::SetMenuMode( sal_uInt16 nMode )
{
    // Fuer die 5.1-Auslieferung besser noch nicht inline, ansonsten kann
    // diese Funktion zur 6.0 inline werden
    mnMenuMode = nMode;
}

// -----------------------------------------------------------------------

void MenuButton::SetPopupMenu( PopupMenu* pNewMenu )
{
    // Fuer die 5.1-Auslieferung besser noch nicht inline, ansonsten kann
    // diese Funktion zur 6.0 inline werden
    mpMenu = pNewMenu;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
