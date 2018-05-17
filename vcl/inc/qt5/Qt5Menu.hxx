/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <salmenu.hxx>

class MenuItemList;
class QMenuBar;
class Qt5MenuItem;
class Qt5Frame;

class Qt5Menu : public SalMenu
{
private:
    std::vector< Qt5MenuItem* >  maItems;
    VclPtr<Menu>                 mpVCLMenu;
    Qt5Menu*                     mpParentSalMenu;
    Qt5Frame*                    mpFrame;
    bool                         mbMenuBar;

    QMenuBar*                    mpQMenuBar;

    void ActivateAllSubMenus( Menu* pMenuBar );
    void Update();

    void NativeItemText( OUString &rItemText );

public:
    Qt5Menu( bool bMenuBar );
    virtual ~Qt5Menu() override;

    virtual bool                VisibleMenuBar() override;   // must return TRUE to actually DISPLAY native menu bars

    virtual void                InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos ) override;
    virtual void                RemoveItem( unsigned nPos ) override;
    virtual void                SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos ) override;
    virtual void                SetFrame( const SalFrame* pFrame ) override;
    const Qt5Frame*             GetFrame() const;
    virtual void                CheckItem( unsigned nPos, bool bCheck ) override;
    virtual void                EnableItem( unsigned nPos, bool bEnable ) override;
    virtual void                ShowItem( unsigned nPos, bool bShow ) override;
    virtual void                SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const OUString& rText ) override;
    virtual void                SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage) override;
    virtual void                SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const vcl::KeyCode& rKeyCode, const OUString& rKeyName ) override;
    virtual void                GetSystemMenuData( SystemMenuData* pData ) override;

    void                        SetMenu( Menu* pMenu ) { mpVCLMenu = pMenu; }
    Menu*                       GetMenu() { return mpVCLMenu; }
    unsigned                    GetItemCount() { return maItems.size(); }
    Qt5MenuItem*                GetItemAtPos( unsigned nPos ) { return maItems[ nPos ]; }
};

class Qt5MenuItem : public SalMenuItem
{
public:
    Qt5MenuItem( const SalItemParams* );
    virtual ~Qt5MenuItem() override;

    sal_uInt16          mnId;               // Item ID
    MenuItemType        mnType;             // Item type
    bool                mbVisible;          // Item visibility.
    Menu*               mpVCLMenu;
    Qt5Menu*            mpParentMenu;       // The menu into which this menu item is inserted
    Qt5Menu*            mpSubMenu;          // Submenu of this item (if defined)
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
