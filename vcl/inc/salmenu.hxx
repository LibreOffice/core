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

#ifndef _SV_SALMENU_HXX
#define _SV_SALMENU_HXX

#include <vcl/sv.h>
#include "vcl/dllapi.h"
#include <vcl/menu.hxx>
#include <vcl/keycod.hxx>
#include <vcl/image.hxx>

struct SystemMenuData;
class FloatingWindow;
class SalFrame;

struct SalItemParams
{
    sal_uInt16          nId;                    // item Id
    MenuItemType    eType;                  // MenuItem-Type
    MenuItemBits    nBits;                  // MenuItem-Bits
    Menu*           pMenu;                  // Pointer to Menu
    XubString       aText;                  // Menu-Text
    Image           aImage;                 // Image
};


struct SalMenuButtonItem
{
    sal_uInt16              mnId;
    Image               maImage;
    rtl::OUString       maToolTipText;

    SalMenuButtonItem() : mnId( 0 ) {}
    SalMenuButtonItem( sal_uInt16 i_nId, const Image& rImg, const rtl::OUString& i_rTTText = rtl::OUString() )
    : mnId( i_nId ), maImage( rImg ), maToolTipText( i_rTTText ) {}
};

class VCL_PLUGIN_PUBLIC SalMenuItem
{
public:
    SalMenuItem() {}
    virtual ~SalMenuItem();
};

class VCL_PLUGIN_PUBLIC SalMenu
{
public:
    SalMenu() {}
    virtual ~SalMenu();

    virtual sal_Bool VisibleMenuBar() = 0;  // must return sal_True to actually DISPLAY native menu bars
                            // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos ) = 0;
    virtual void RemoveItem( unsigned nPos ) = 0;
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos ) = 0;
    virtual void SetFrame( const SalFrame* pFrame ) = 0;
    virtual void CheckItem( unsigned nPos, sal_Bool bCheck ) = 0;
    virtual void EnableItem( unsigned nPos, sal_Bool bEnable ) = 0;
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )= 0;
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage ) = 0;
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName ) = 0;
    virtual void GetSystemMenuData( SystemMenuData* pData ) = 0;
    virtual bool ShowNativePopupMenu(FloatingWindow * pWin, const Rectangle& rRect, sal_uLong nFlags);
    virtual bool AddMenuBarButton( const SalMenuButtonItem& ); // return false if not implemented or failure
    virtual void RemoveMenuBarButton( sal_uInt16 nId );

    // return an empty rectangle if not implemented
    // return Rectangle( Point( -1, -1 ), Size( 1, 1 ) ) if menu bar buttons implemented
    // but rectangle cannot be determined
    virtual Rectangle GetMenuBarButtonRectPixel( sal_uInt16 i_nItemId, SalFrame* i_pReferenceFrame );
};


#endif // _SV_SALMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
