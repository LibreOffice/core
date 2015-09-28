/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_INC_UNX_GTK_GTKSALMENU_HXX
#define INCLUDED_VCL_INC_UNX_GTK_GTKSALMENU_HXX

#include <vector>
#ifdef ENABLE_GIO
#include <gio/gio.h>
#endif

#include <unx/salmenu.h>
#include <unx/gtk/gtkframe.hxx>

#if defined(ENABLE_DBUS) && defined(ENABLE_GIO) && \
    (GLIB_MAJOR_VERSION > 2 || GLIB_MINOR_VERSION >= 36)
#  define ENABLE_GMENU_INTEGRATION
#  include <unx/gtk/glomenu.h>
#  include <unx/gtk/gloactiongroup.h>
#else
#  if !(GLIB_MAJOR_VERSION > 2 || GLIB_MINOR_VERSION >= 32)
     typedef void GMenuModel;
#  endif
#  if !(GLIB_MAJOR_VERSION > 2 || GLIB_MINOR_VERSION >= 28)
     typedef void GActionGroup;
#  endif
#endif

class MenuItemList;
class GtkSalMenuItem;

class GtkSalMenu : public SalMenu
{
private:
    std::vector< GtkSalMenuItem* >  maItems;

    bool                        mbMenuBar;
    Menu*                           mpVCLMenu;
    GtkSalMenu*                     mpParentSalMenu;
    const GtkSalFrame*              mpFrame;

    // GMenuModel and GActionGroup attributes
    GMenuModel*                     mpMenuModel;
    GActionGroup*                   mpActionGroup;

    GtkSalMenu*                 GetMenuForItemCommand( gchar* aCommand, gboolean bGetSubmenu );
    void                        ImplUpdate( gboolean bRecurse );
    void                        ActivateAllSubmenus(MenuBar* pMenuBar);

public:
    GtkSalMenu( bool bMenuBar );
    virtual ~GtkSalMenu();

    virtual bool                VisibleMenuBar() SAL_OVERRIDE;   // must return TRUE to actually DISPLAY native menu bars
                                                    // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void                InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos ) SAL_OVERRIDE;
    virtual void                RemoveItem( unsigned nPos ) SAL_OVERRIDE;
    virtual void                SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos ) SAL_OVERRIDE;
    virtual void                SetFrame( const SalFrame* pFrame ) SAL_OVERRIDE;
    const GtkSalFrame*          GetFrame() const;
    virtual void                CheckItem( unsigned nPos, bool bCheck ) SAL_OVERRIDE;
    virtual void                EnableItem( unsigned nPos, bool bEnable ) SAL_OVERRIDE;
    virtual void                ShowItem( unsigned nPos, bool bShow ) SAL_OVERRIDE;
    virtual void                SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const OUString& rText ) SAL_OVERRIDE;
    virtual void                SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage) SAL_OVERRIDE;
    virtual void                SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const vcl::KeyCode& rKeyCode, const OUString& rKeyName ) SAL_OVERRIDE;
    virtual void                GetSystemMenuData( SystemMenuData* pData ) SAL_OVERRIDE;

    void                        SetMenu( Menu* pMenu ) { mpVCLMenu = pMenu; }
    Menu*                       GetMenu() { return mpVCLMenu; }
    void                        SetMenuModel( GMenuModel* pMenuModel ) { mpMenuModel = pMenuModel; }
    GMenuModel*                 GetMenuModel() { return mpMenuModel; }
    unsigned                    GetItemCount() { return maItems.size(); }
    GtkSalMenuItem*             GetItemAtPos( unsigned nPos ) { return maItems[ nPos ]; }
    void                        SetActionGroup( GActionGroup* pActionGroup ) { mpActionGroup = pActionGroup; }
    GActionGroup*               GetActionGroup() { return mpActionGroup; }
    bool                        IsItemVisible( unsigned nPos );

    void                        NativeSetItemText( unsigned nSection, unsigned nItemPos, const OUString& rText );
    void                        NativeSetItemCommand( unsigned nSection,
                                                      unsigned nItemPos,
                                                      sal_uInt16 nId,
                                                      const gchar* aCommand,
                                                      MenuItemBits nBits,
                                                      gboolean bChecked,
                                                      gboolean bIsSubmenu );
    void                        NativeSetEnableItem( gchar* aCommand, gboolean bEnable );
    void                        NativeCheckItem( unsigned nSection, unsigned nItemPos, MenuItemBits bits, gboolean bCheck );
    void                        NativeSetAccelerator( unsigned nSection, unsigned nItemPos, const vcl::KeyCode& rKeyCode, const OUString& rKeyName );

    void                        DispatchCommand( gint itemId, const gchar* aCommand );
    void                        Activate();
    void                        Deactivate( const gchar* aMenuCommand );
    void                        Display( bool bVisible );
    bool                        PrepUpdate();
    void                        Update();           // Update this menu only.
    void                        UpdateFull();       // Update full menu hierarchy from this menu.
};

class GtkSalMenuItem : public SalMenuItem
{
public:
    GtkSalMenuItem( const SalItemParams* );
    virtual ~GtkSalMenuItem();

    sal_uInt16          mnId;               // Item ID
    MenuItemType        mnType;             // Item type
    bool                mbVisible;          // Item visibility.
    Menu*               mpVCLMenu;          // VCL Menu into which this menu item is inserted
    GtkSalMenu*         mpParentMenu;       // The menu into which this menu item is inserted
    GtkSalMenu*         mpSubMenu;          // Submenu of this item (if defined)
};

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKSALMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
