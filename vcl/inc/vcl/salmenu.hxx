/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salmenu.hxx,v $
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

#ifndef _SV_SALMENU_HXX
#define _SV_SALMENU_HXX

#include <vcl/sv.h>
#include "vcl/dllapi.h"
#include <vcl/menu.hxx>
#include <vcl/keycod.hxx>
#include <vcl/image.hxx>

struct SystemMenuData;
class SalFrame;

struct SalItemParams
{
    USHORT          nId;                    // item Id
    MenuItemType    eType;                  // MenuItem-Type
    MenuItemBits    nBits;                  // MenuItem-Bits
    Menu*           pMenu;                  // Pointer to Menu
    XubString       aText;                  // Menu-Text
    Image           aImage;                 // Image
};


class VCL_DLLPUBLIC SalMenuItem
{
public:
    SalMenuItem() {}
    virtual ~SalMenuItem();
};

class VCL_DLLPUBLIC SalMenu
{
public:
    SalMenu() {}
    virtual ~SalMenu();

    virtual BOOL VisibleMenuBar() = 0;  // must return TRUE to actually DISPLAY native menu bars
                            // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos ) = 0;
    virtual void RemoveItem( unsigned nPos ) = 0;
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos ) = 0;
    virtual void SetFrame( const SalFrame* pFrame ) = 0;
    virtual void CheckItem( unsigned nPos, BOOL bCheck ) = 0;
    virtual void EnableItem( unsigned nPos, BOOL bEnable ) = 0;
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText )= 0;
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage ) = 0;
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName ) = 0;
    virtual void GetSystemMenuData( SystemMenuData* pData ) = 0;
};


#endif // _SV_SALMENU_HXX

