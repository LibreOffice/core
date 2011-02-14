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

#ifndef _SV_SALMENU_H
#define _SV_SALMENU_H

#include <vcl/bitmap.hxx>
#include <salmenu.hxx>

class WinSalMenu : public SalMenu
{
public:
    WinSalMenu();
    virtual ~WinSalMenu();
    virtual sal_Bool VisibleMenuBar();  // must return TRUE to actually DISPLAY native menu bars
                            // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    virtual void RemoveItem( unsigned nPos );
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    virtual void SetFrame( const SalFrame* pFrame );
    virtual void CheckItem( unsigned nPos, sal_Bool bCheck );
    virtual void EnableItem( unsigned nPos, sal_Bool bEnable );
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText );
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage );
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName );
    virtual void GetSystemMenuData( SystemMenuData* pData );

    HMENU mhMenu;           // the menu handle
    sal_Bool  mbMenuBar;        // true for menu bars
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
    XubString mText;        // the item text
    XubString mAccelText;   // the accelerator string
    Bitmap    maBitmap;     // item image
    int       mnId;         // item id
    WinSalMenu*  mpSalMenu;    // the menu where this item is inserted
};

#endif // _SV_SALMENU_H

