/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salmenu.h,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:55:25 $
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

#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"

#include "vcl/sv.h"
#include "vcl/salmenu.hxx"

#include <vector>

class AquaSalFrame;
class AquaSalMenuItem;

class AquaSalMenu : public SalMenu
{
public:
    AquaSalMenu( bool bMenuBar );
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

    int getItemIndexByPos( USHORT nPos ) const;
    const AquaSalFrame* getFrame() const;

    void setMainMenu();
    void unsetMainMenu();
    static void setDefaultMenu();
    static void enableMainMenu( bool bEnable );
    static void addFallbackMenuItem( NSMenuItem* NewItem );
    static void removeFallbackMenuItem( NSMenuItem* pOldItem );

    bool                    mbMenuBar;          // true - Menubar, false - Menu
    NSMenu*                 mpMenu;             // The Carbon reference to this menu
    Menu*                   mpVCLMenu;          // the corresponding vcl Menu object
    const AquaSalFrame*     mpFrame;            // the frame to dispatch the menu events to
    AquaSalMenu*            mpParentSalMenu;    // the parent menu that contains us (and perhaps has a frame)

    static const AquaSalMenu* pCurrentMenuBar;

    std::vector< AquaSalMenuItem* > maItems;
};

#endif // _SV_SALMENU_H
