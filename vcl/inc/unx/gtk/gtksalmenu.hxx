/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * Copyright © 2011 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
 * USA.
 *
 * Author: Antonio Fernández <antonio.fernandez@aentos.es>
 */

#ifndef GTKSALMENU_HXX
#define GTKSALMENU_HXX

#include <vcl/sv.h>
#include <vcl/bitmap.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unx/salmenu.h>

#include <gio/gio.h>

#include "glomenu.h"
#include "gloactiongroup.h"


class MenuItemList;
class GtkSalMenuItem;

class GtkSalMenu : public SalMenu
{
private:
    std::vector< GtkSalMenuItem* >  maItems;

    sal_Bool                        mbMenuBar;
    sal_Bool                        mbVisible;
    Menu*                           mpVCLMenu;
    GtkSalMenu*                     mpParentSalMenu;
    const GtkSalFrame*              mpFrame;

    sal_uInt32                      mWatcherId;

    // GMenuModel and GActionGroup attributes
    GMenuModel*                     mpMenuModel;
    GActionGroup*                   mpActionGroup;

    GtkSalMenu*                 GetMenuForItemCommand( gchar* aCommand, gboolean bGetSubmenu );

public:
    GtkSalMenu( sal_Bool bMenuBar );
    virtual ~GtkSalMenu();

    virtual sal_Bool            VisibleMenuBar();   // must return TRUE to actually DISPLAY native menu bars
                                                    // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void                InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos );
    virtual void                RemoveItem( unsigned nPos );
    virtual void                SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos );
    virtual void                SetFrame( const SalFrame* pFrame );
    virtual const GtkSalFrame*  GetFrame() const;
    virtual void                CheckItem( unsigned nPos, sal_Bool bCheck );
    virtual void                EnableItem( unsigned nPos, sal_Bool bEnable );
    virtual void                SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& rText );
    virtual void                SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage);
    virtual void                SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const rtl::OUString& rKeyName );
    virtual void                GetSystemMenuData( SystemMenuData* pData );
    virtual void                SetItemCommand( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& aCommandStr );
    virtual void                Freeze();

    virtual void                SetMenu( Menu* pMenu ) { mpVCLMenu = pMenu; }
    virtual Menu*               GetMenu() { return mpVCLMenu; }
    virtual GtkSalMenu*         GetParentSalMenu() { return mpParentSalMenu; }
    virtual void                SetMenuModel( GMenuModel* pMenuModel ) { mpMenuModel = pMenuModel; }
    virtual GMenuModel*         GetMenuModel() { return mpMenuModel; }
    virtual unsigned            GetItemCount() { return maItems.size(); }
    virtual GtkSalMenuItem*     GetItemAtPos( unsigned nPos ) { return maItems[ nPos ]; }
    virtual void                SetActionGroup( GActionGroup* pActionGroup ) { mpActionGroup = pActionGroup; }
    virtual GActionGroup*       GetActionGroup() { return mpActionGroup; }

    void                        NativeSetItemText( unsigned nSection, unsigned nItemPos, const rtl::OUString& rText );
    void                        NativeSetItemCommand( unsigned nSection,
                                                      unsigned nItemPos,
                                                      sal_uInt16 nId,
                                                      const gchar* aCommand,
                                                      MenuItemBits nBits,
                                                      gboolean bChecked,
                                                      gboolean bIsSubmenu );
    void                        NativeSetEnableItem( gchar* aCommand, gboolean bEnable );
    void                        NativeCheckItem( unsigned nSection, unsigned nItemPos, MenuItemBits bits, gboolean bCheck );
    void                        NativeSetAccelerator( unsigned nSection, unsigned nItemPos, const KeyCode& rKeyCode, const rtl::OUString& rKeyName );

    void                        DispatchCommand( gint itemId, const gchar* aCommand );
    void                        Activate( const gchar* aMenuCommand );
    void                        Deactivate( const gchar* aMenuCommand );
};

class GtkSalMenuItem : public SalMenuItem
{
public:
    GtkSalMenuItem( const SalItemParams* );
    virtual ~GtkSalMenuItem();

    sal_uInt16          mnId;               // Item ID
    MenuItemType        mnType;             // Item type
    Menu*               mpVCLMenu;          // VCL Menu into which this MenuItem is inserted
    GtkSalMenu*         mpParentMenu;       // The menu in which this menu item is inserted
    GtkSalMenu*         mpSubMenu;          // Sub menu of this item (if defined)
};

#endif // GTKSALMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
