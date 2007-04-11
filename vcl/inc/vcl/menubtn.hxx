/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: menubtn.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:00:25 $
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

#ifndef _SV_MENUBTN_HXX
#define _SV_MENUBTN_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

class Timer;
class PopupMenu;

// --------------------
// - MenuButton-Types -
// --------------------

#define MENUBUTTON_MENUMODE_TIMED       ((USHORT)0x0001)

// --------------
// - MenuButton -
// --------------

class VCL_DLLPUBLIC MenuButton : public PushButton
{
private:
    Rectangle       maFocusRect;
    Timer*          mpMenuTimer;
    PopupMenu*      mpOwnMenu;
    PopupMenu*      mpMenu;
    USHORT          mnCurItemId;
    USHORT          mnMenuMode;
    Link            maActivateHdl;
    Link            maSelectHdl;

    SAL_DLLPRIVATE void    ImplInitMenuButtonData();
    SAL_DLLPRIVATE void    ImplExecuteMenu();
    DECL_DLLPRIVATE_LINK(  ImplMenuTimeoutHdl, Timer* );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         MenuButton( const MenuButton & );
    SAL_DLLPRIVATE         MenuButton& operator=( const MenuButton & );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplLoadRes( const ResId& rResId );

public:
                    MenuButton( Window* pParent, WinBits nStyle = 0 );
                    MenuButton( Window* pParent, const ResId& rResId );
                    ~MenuButton();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );

    virtual void    Activate();
    virtual void    Select();

    void            SetMenuMode( USHORT nMode );
    USHORT          GetMenuMode() const { return mnMenuMode; }

    void            SetPopupMenu( PopupMenu* pNewMenu );
    PopupMenu*      GetPopupMenu() const { return mpMenu; }

    USHORT          GetCurItemId() const { return mnCurItemId; }

    void            SetActivateHdl( const Link& rLink ) { maActivateHdl = rLink; }
    const Link&     GetActivateHdl() const              { return maActivateHdl; }
    void            SetSelectHdl( const Link& rLink )   { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const                { return maSelectHdl; }
};

#endif  // _SV_MENUBTN_HXX
