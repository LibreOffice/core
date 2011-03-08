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
    std::vector< AquaSalMenuItem* >     maItems;

public: // for OOStatusView
    struct MenuBarButtonEntry
    {
        SalMenuButtonItem       maButton;
        NSImage*                mpNSImage;      // cached image
        NSString*               mpToolTipString;

        MenuBarButtonEntry() : mpNSImage( nil ), mpToolTipString( nil ) {}
        MenuBarButtonEntry( const SalMenuButtonItem& i_rItem )
        : maButton( i_rItem), mpNSImage( nil ), mpToolTipString( nil ) {}
    };
private:
    std::vector< MenuBarButtonEntry >   maButtons;

    MenuBarButtonEntry* findButtonItem( USHORT i_nItemId );
    void releaseButtonEntry( MenuBarButtonEntry& i_rEntry );
    static void statusLayout();
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
    virtual bool ShowNativePopupMenu(FloatingWindow * pWin, const Rectangle& rRect, ULONG nFlags);
    virtual bool AddMenuBarButton( const SalMenuButtonItem& );
    virtual void RemoveMenuBarButton( USHORT nId );
    virtual Rectangle GetMenuBarButtonRectPixel( USHORT i_nItemId, SalFrame* i_pReferenceFrame );

    int getItemIndexByPos( USHORT nPos ) const;
    const AquaSalFrame* getFrame() const;

    void setMainMenu();
    static void unsetMainMenu();
    static void setDefaultMenu();
    static void enableMainMenu( bool bEnable );
    static void addFallbackMenuItem( NSMenuItem* NewItem );
    static void removeFallbackMenuItem( NSMenuItem* pOldItem );

    const std::vector< MenuBarButtonEntry >& getButtons() const { return maButtons; }

    bool                    mbMenuBar;          // true - Menubar, false - Menu
    NSMenu*                 mpMenu;             // The Carbon reference to this menu
    Menu*                   mpVCLMenu;          // the corresponding vcl Menu object
    const AquaSalFrame*     mpFrame;            // the frame to dispatch the menu events to
    AquaSalMenu*            mpParentSalMenu;    // the parent menu that contains us (and perhaps has a frame)

    static const AquaSalMenu* pCurrentMenuBar;

};

class AquaSalMenuItem : public SalMenuItem
{
public:
    AquaSalMenuItem( const SalItemParams* );
    virtual ~AquaSalMenuItem();

    USHORT              mnId;                 // Item ID
    Menu*               mpVCLMenu;            // VCL Menu into which this MenuItem is inserted
    AquaSalMenu*        mpParentMenu;         // The menu in which this menu item is inserted
    AquaSalMenu*        mpSubMenu;            // Sub menu of this item (if defined)
    NSMenuItem*         mpMenuItem;           // The NSMenuItem
};

#endif // _SV_SALMENU_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
