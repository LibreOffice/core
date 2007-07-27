/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salmenu.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-27 07:43:42 $
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

#ifndef _SV_SALMENU_H
#define _SV_SALMENU_H

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifndef _SV_SALMENU_HXX
#include <vcl/salmenu.hxx>
#endif


class AquaSalMenu : public SalMenu
{
public:
    AquaSalMenu() {}
    virtual ~AquaSalMenu();

    virtual BOOL VisibleMenuBar();  // must return TRUE to actually DISPLAY native menu bars
                                    // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    virtual void RemoveItem( unsigned nPos );
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    virtual void SetFrame( const SalFrame* pFrame );
    virtual void CheckItem( unsigned nPos, BOOL bCheck );
    virtual void EnableItem( unsigned nPos, BOOL bEnable );
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText );
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage);
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName );
    virtual void GetSystemMenuData( SystemMenuData* pData );

    XubString mText;   // Title of this menu
    MenuRef mrMenuRef; // The Carbon reference to this menu
    BOOL mbMenuBar;    // TRUE - Menubar, FALSE - Menu
};

class AquaSalMenuItem : public SalMenuItem
{
public:
    AquaSalMenuItem() {}
    virtual ~AquaSalMenuItem();

    USHORT mnId;                        // Item ID
    Menu *mpMenu;                       // Menu into which this MenuItem is inserted
    SalMenu *mpSubMenu;                 // Sub menu of this item (if defined)
    XubString mText;                    // Title of this menu item
    MenuRef mrParentMenuRef;            // The menu in which this menu item is inserted

    MenuItemIndex mnMenuItemIndex;      // The menu index of this menu item in mpMenu
                                        // It is 1 based, so the first menu
                                        // item's MenuItemIndex in the menu has value 1

    MenuItemAttributes maMenuAttributes;

    Bitmap maBitmap;            // item image
};

#endif // _SV_SALMENU_H
