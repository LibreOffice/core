/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: menubtn.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_MENUBTN_HXX
#define _SV_MENUBTN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/button.hxx>

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
