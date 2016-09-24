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

#ifndef INCLUDED_VCL_INC_WIN_SALMENU_H
#define INCLUDED_VCL_INC_WIN_SALMENU_H

#include <vcl/bitmap.hxx>
#include <salmenu.hxx>

class WinSalMenu : public SalMenu
{
public:
    WinSalMenu();
    virtual ~WinSalMenu();
    virtual bool VisibleMenuBar() override;  // must return TRUE to actually DISPLAY native menu bars
                            // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos ) override;
    virtual void RemoveItem( unsigned nPos ) override;
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos ) override;
    virtual void SetFrame( const SalFrame* pFrame ) override;
    virtual void CheckItem( unsigned nPos, bool bCheck ) override;
    virtual void EnableItem( unsigned nPos, bool bEnable ) override;
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const OUString& rText ) override;
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage ) override;
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const vcl::KeyCode& rKeyCode, const OUString& rKeyName ) override;
    virtual void GetSystemMenuData( SystemMenuData* pData ) override;

    HMENU mhMenu;           // the menu handle
    bool  mbMenuBar;        // true for menu bars
    HWND  mhWnd;            // the window handle where the menubar is attached, may be NULL
    WinSalMenu *mpParentMenu;  // the parent menu
};

class WinSalMenuItem : public SalMenuItem
{
public:
    WinSalMenuItem();
    virtual ~WinSalMenuItem();

    MENUITEMINFOW mInfo;
    void*     mpMenu;       // pointer to corresponding VCL menu
    OUString  mText;        // the item text
    OUString  mAccelText;   // the accelerator string
    Bitmap    maBitmap;     // item image
    int       mnId;         // item id
    WinSalMenu*  mpSalMenu;    // the menu where this item is inserted
};

#endif // INCLUDED_VCL_INC_WIN_SALMENU_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
