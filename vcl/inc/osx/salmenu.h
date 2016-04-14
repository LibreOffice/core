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

#ifndef INCLUDED_VCL_INC_OSX_SALMENU_H
#define INCLUDED_VCL_INC_OSX_SALMENU_H

#include "premac.h"
#include <Cocoa/Cocoa.h>
#include "postmac.h"

#include "salmenu.hxx"

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

    MenuBarButtonEntry* findButtonItem( sal_uInt16 i_nItemId );
    static void statusLayout();
public:
    AquaSalMenu( bool bMenuBar );
    virtual ~AquaSalMenu() override;

    virtual bool VisibleMenuBar() override;
    // must return true to actually display native menu bars
    // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos ) override;
    virtual void RemoveItem( unsigned nPos ) override;
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos ) override;
    virtual void SetFrame( const SalFrame* pFrame ) override;
    virtual void CheckItem( unsigned nPos, bool bCheck ) override;
    virtual void EnableItem( unsigned nPos, bool bEnable ) override;
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const OUString& rText ) override;
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage) override;
    virtual void SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const vcl::KeyCode& rKeyCode, const OUString& rKeyName ) override;
    virtual void GetSystemMenuData( SystemMenuData* pData ) override;
    virtual bool ShowNativePopupMenu(FloatingWindow * pWin, const Rectangle& rRect, FloatWinPopupFlags nFlags) override;
    virtual bool AddMenuBarButton( const SalMenuButtonItem& ) override;
    virtual void RemoveMenuBarButton( sal_uInt16 nId ) override;
    virtual Rectangle GetMenuBarButtonRectPixel( sal_uInt16 i_nItemId, SalFrame* i_pReferenceFrame ) override;

    int getItemIndexByPos( sal_uInt16 nPos ) const;
    const AquaSalFrame* getFrame() const;

    void setMainMenu();
    static void unsetMainMenu();
    static void setDefaultMenu();
    static void enableMainMenu( bool bEnable );
    static void addFallbackMenuItem( NSMenuItem* NewItem );
    static void removeFallbackMenuItem( NSMenuItem* pOldItem );

    bool IsSeparatorAt( unsigned nPos );
    static bool IsItemSeparator( AquaSalMenuItem * pAquaSalMenuItem );

    const std::vector< MenuBarButtonEntry >& getButtons() const { return maButtons; }

    bool                    mbMenuBar;          // true - Menubar, false - Menu
    NSMenu*                 mpMenu;             // The Carbon reference to this menu
    VclPtr<Menu>            mpVCLMenu;          // the corresponding vcl Menu object
    const AquaSalFrame*     mpFrame;            // the frame to dispatch the menu events to
    AquaSalMenu*            mpParentSalMenu;    // the parent menu that contains us (and perhaps has a frame)

    static const AquaSalMenu* pCurrentMenuBar;

};

class AquaSalMenuItem : public SalMenuItem
{
public:
    AquaSalMenuItem( const SalItemParams* );
    virtual ~AquaSalMenuItem() override;

    sal_uInt16          mnId;                 // Item ID
    VclPtr<Menu>        mpVCLMenu;            // VCL Menu into which this MenuItem is inserted
    AquaSalMenu*        mpParentMenu;         // The menu in which this menu item is inserted
    AquaSalMenu*        mpSubMenu;            // Sub menu of this item (if defined)
    NSMenuItem*         mpMenuItem;           // The NSMenuItem
};

#endif // INCLUDED_VCL_INC_OSX_SALMENU_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
