/*************************************************************************
 *
 *  $RCSfile: salmenu.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-20 13:03:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_SALMENU_H
#define _SV_SALMENU_H

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _SV_SALMENU_HXX
#include <salmenu.hxx>
#endif


class WinSalMenu : public SalMenu
{
public:
    WinSalMenu();
    virtual ~WinSalMenu();
    virtual BOOL VisibleMenuBar();  // must return TRUE to actually DISPLAY native menu bars
                            // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    virtual void RemoveItem( unsigned nPos );
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    virtual void SetFrame( const SalFrame* pFrame );
    virtual void CheckItem( unsigned nPos, BOOL bCheck );
    virtual void EnableItem( unsigned nPos, BOOL bEnable );
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText );
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage );
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const XubString& rKeyName );
    virtual void GetSystemMenuData( SystemMenuData* pData );

    HMENU mhMenu;           // the menu handle
    BOOL  mbMenuBar;        // true for menu bars
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

