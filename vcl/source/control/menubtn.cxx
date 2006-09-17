/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: menubtn.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:54:28 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_RC_H
#include <tools/rc.h>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_MENUBTN_HXX
#include <menubtn.hxx>
#endif



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
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        if ( !((GetStyle() & (WB_RECTSTYLE | WB_SMALLSTYLE)) ||
             !(rStyleSettings.GetOptions() & STYLE_OPTION_MACSTYLE)) )
        {
            aRect.Left()    += 2;
            aRect.Top()     += 2;
            aRect.Right()   -= 2;
            aRect.Bottom()  -= 2;
        }
        SetPressed( TRUE );
        EndSelection();
        mnCurItemId = mpMenu->Execute( this, aRect, POPUPMENU_EXECUTE_DOWN );
        SetPressed( FALSE );
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

    ULONG nObjMask = ReadLongRes();

    if ( RSCMENUBUTTON_MENU & nObjMask )
    {
        mpOwnMenu = new PopupMenu( ResId( (RSHEADER_TYPE*)GetClassRes() ) );
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
    if ( mnMenuMode & MENUBUTTON_MENUMODE_TIMED )
    {
        if ( !mpMenuTimer )
        {
            mpMenuTimer = new Timer;
            mpMenuTimer->SetTimeoutHdl( LINK( this, MenuButton, ImplMenuTimeoutHdl ) );
        }

        mpMenuTimer->SetTimeout( GetSettings().GetMouseSettings().GetActionDelay() );
        mpMenuTimer->Start();

        PushButton::MouseButtonDown( rMEvt );
    }
    else
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
    USHORT nCode = aKeyCode.GetCode();
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

void MenuButton::SetMenuMode( USHORT nMode )
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
