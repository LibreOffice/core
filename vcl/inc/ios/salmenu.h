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
#include <UIKit/UIKit.h>
#include "postmac.h"

#include "salmenu.hxx"

#include <vector>

class IosSalFrame;
class IosSalMenuItem;

class IosSalMenu : public SalMenu
{
    std::vector< IosSalMenuItem* >     maItems;

public: // for OOStatusView
    struct MenuBarButtonEntry
    {
        SalMenuButtonItem       maButton;
        UIImage*                mpUIImage;      // cached image
        NSString*               mpToolTipString;

        MenuBarButtonEntry() : mpUIImage( nil ), mpToolTipString( nil ) {}
        MenuBarButtonEntry( const SalMenuButtonItem& i_rItem )
        : maButton( i_rItem), mpUIImage( nil ), mpToolTipString( nil ) {}
    };
private:
    std::vector< MenuBarButtonEntry >   maButtons;

    MenuBarButtonEntry* findButtonItem( sal_uInt16 i_nItemId );
    void releaseButtonEntry( MenuBarButtonEntry& i_rEntry );
    static void statusLayout();
public:
    IosSalMenu( bool bMenuBar );
    virtual ~IosSalMenu();

    virtual sal_Bool VisibleMenuBar();  // must return TRUE to actually DISPLAY native menu bars
                                    // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    virtual void RemoveItem( unsigned nPos );
    virtual void SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    virtual void SetFrame( const SalFrame* pFrame );
    virtual void CheckItem( unsigned nPos, sal_Bool bCheck );
    virtual void EnableItem( unsigned nPos, sal_Bool bEnable );
    virtual void SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const XubString& rText );
    virtual void SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage);
    virtual void GetSystemMenuData( SystemMenuData* pData );
    virtual bool ShowNativePopupMenu(FloatingWindow * pWin, const Rectangle& rRect, sal_uLong nFlags);
    virtual bool AddMenuBarButton( const SalMenuButtonItem& );
    virtual void RemoveMenuBarButton( sal_uInt16 nId );
    virtual Rectangle GetMenuBarButtonRectPixel( sal_uInt16 i_nItemId, SalFrame* i_pReferenceFrame );

    int getItemIndexByPos( sal_uInt16 nPos ) const;
    const IosSalFrame* getFrame() const;

    void setMainMenu();
    static void unsetMainMenu();
    static void setDefaultMenu();
    static void enableMainMenu( bool bEnable );
    static void addFallbackMenuItem( UIMenuItem* NewItem );
    static void removeFallbackMenuItem( UIMenuItem* pOldItem );

    const std::vector< MenuBarButtonEntry >& getButtons() const { return maButtons; }

    bool                    mbMenuBar;          // true - Menubar, false - Menu
    UIMenuController*       mpMenu;
    Menu*                   mpVCLMenu;          // the corresponding vcl Menu object
    const IosSalFrame*      mpFrame;            // the frame to dispatch the menu events to
    IosSalMenu*             mpParentSalMenu;    // the parent menu that contains us (and perhaps has a frame)

    static const IosSalMenu* pCurrentMenuBar;

};

class IosSalMenuItem : public SalMenuItem
{
public:
    IosSalMenuItem( const SalItemParams* );
    virtual ~IosSalMenuItem();

    sal_uInt16          mnId;                 // Item ID
    Menu*               mpVCLMenu;            // VCL Menu into which this MenuItem is inserted
    IosSalMenu*         mpParentMenu;         // The menu in which this menu item is inserted
    IosSalMenu*         mpSubMenu;            // Sub menu of this item (if defined)
    UIMenuItem*         mpMenuItem;           // The UIMenuItem
};

#endif // _SV_SALMENU_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
