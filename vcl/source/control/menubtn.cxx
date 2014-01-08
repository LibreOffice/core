/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/menu.hxx>
#include <vcl/timer.hxx>
#include <vcl/menubtn.hxx>



// =======================================================================

#define IMAGEBUTTON_BORDER_OFF1     11
#define IMAGEBUTTON_BORDER_OFF2     16

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
    if ( mpMenuTimer )
        delete mpMenuTimer;
    if ( mpOwnMenu )
        delete mpOwnMenu;
}

// -----------------------------------------------------------------------

IMPL_LINK( MenuButton, ImplMenuTimeoutHdl, Timer*, EMPTYARG )
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
        // if the separated dropdown symbol is hit,
        // execute the popup immediately
        if ( ImplGetSymbolRect().IsEmpty() ||
            ( PushButton::ImplHitTestPushButton( this, rMEvt.GetPosPixel() ) &&
            rMEvt.GetPosPixel().X() < ImplGetSymbolRect().Left() ) )
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
