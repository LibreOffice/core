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

#ifndef INCLUDED_VCL_MENUBTN_HXX
#define INCLUDED_VCL_MENUBTN_HXX

#include <vcl/button.hxx>
#include <vcl/dllapi.h>

class Timer;
class PopupMenu;
class VclBuilder;
class VclSimpleEvent;

#define MENUBUTTON_MENUMODE_TIMED       ((sal_uInt16)0x0001)

class VCL_DLLPUBLIC MenuButton : public PushButton
{
private:
    friend class VclBuilder;

    Rectangle       maFocusRect;
    Timer*          mpMenuTimer;
    PopupMenu*      mpOwnMenu;
    PopupMenu*      mpMenu;
    sal_uInt16      mnCurItemId;
    sal_uInt16      mnMenuMode;
    Link            maActivateHdl;
    Link            maSelectHdl;

    SAL_DLLPRIVATE void    ImplInitMenuButtonData();
    SAL_DLLPRIVATE void    ImplExecuteMenu();
    DECL_DLLPRIVATE_LINK(  ImplMenuTimeoutHdl, void* );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         MenuButton( const MenuButton & );
    SAL_DLLPRIVATE         MenuButton& operator=( const MenuButton & );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplLoadRes( const ResId& rResId );

public:
    explicit        MenuButton( Window* pParent, WinBits nStyle = 0 );
    explicit        MenuButton( Window* pParent, const ResId& );
    virtual         ~MenuButton();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;

    virtual void    Activate() SAL_OVERRIDE;
    virtual void    Select();

    void            SetMenuMode( sal_uInt16 nMode );
    sal_uInt16      GetMenuMode() const { return mnMenuMode; }

    void            SetPopupMenu( PopupMenu* pNewMenu );
    PopupMenu*      GetPopupMenu() const { return mpMenu; }

    sal_uInt16      GetCurItemId() const { return mnCurItemId; }
    OString         GetCurItemIdent() const;
    void            SetCurItemId( sal_uInt16 nItemId ) { mnCurItemId = nItemId; }

    void            SetActivateHdl( const Link& rLink ) { maActivateHdl = rLink; }
    const Link&     GetActivateHdl() const              { return maActivateHdl; }
    void            SetSelectHdl( const Link& rLink )   { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const                { return maSelectHdl; }
};

#endif // INCLUDED_VCL_MENUBTN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
