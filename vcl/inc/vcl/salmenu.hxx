/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salmenu.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:01:31 $
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

#ifndef _SV_SALMENU_HXX
#define _SV_SALMENU_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _VCL_DLLAPI_H
#include "vcl/dllapi.h"
#endif

#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _SV_KEYCODE_HXX
#include <vcl/keycod.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif

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

