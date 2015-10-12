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
    Link<MenuButton*,void> maActivateHdl;
    Link<MenuButton*,void> maSelectHdl;

    SAL_DLLPRIVATE void    ImplInitMenuButtonData();
    DECL_DLLPRIVATE_LINK_TYPED( ImplMenuTimeoutHdl, Timer*, void );

                           MenuButton( const MenuButton & ) = delete;
                           MenuButton& operator=( const MenuButton & ) = delete;

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle );

public:
    explicit        MenuButton( vcl::Window* pParent, WinBits nStyle = 0 );
    virtual         ~MenuButton();
    virtual void    dispose() SAL_OVERRIDE;

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;

    virtual void    Activate() SAL_OVERRIDE;
    virtual void    Select();

    void            ExecuteMenu();

    void            SetMenuMode( sal_uInt16 nMode );

    void            SetPopupMenu( PopupMenu* pNewMenu );
    PopupMenu*      GetPopupMenu() const { return mpMenu; }

    sal_uInt16      GetCurItemId() const { return mnCurItemId; }
    OString         GetCurItemIdent() const;

    void            SetActivateHdl( const Link<MenuButton *, void>& rLink ) { maActivateHdl = rLink; }
    void            SetSelectHdl( const Link<MenuButton *, void>& rLink ) { maSelectHdl = rLink; }
};

#endif // INCLUDED_VCL_MENUBTN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
