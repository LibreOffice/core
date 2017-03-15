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

#include <config_dbus.h>
#include <config_gio.h>

#include <vector>
#if ENABLE_GIO
#include <gio/gio.h>
#endif

#include <salmenu.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <vcl/idle.hxx>

#if ENABLE_DBUS && ENABLE_GIO && \
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
    Idle                            maUpdateMenuBarIdle;

    bool                            mbMenuBar;
    bool                            mbNeedsUpdate;
    bool                            mbReturnFocusToDocument;
    bool                            mbAddedGrab;
    GtkWidget*                      mpMenuBarContainerWidget;
    GtkWidget*                      mpMenuBarWidget;
    GtkWidget*                      mpCloseButton;
    Menu*                           mpVCLMenu;
    GtkSalMenu*                     mpParentSalMenu;
    GtkSalFrame*                    mpFrame;

    // GMenuModel and GActionGroup attributes
    GMenuModel*                     mpMenuModel;
    GActionGroup*                   mpActionGroup;

    void                        ImplUpdate(bool bRecurse, bool bRemoveDisabledEntries);
    void                        ActivateAllSubmenus(Menu* pMenuBar);

    DECL_LINK_TYPED(MenuBarHierarchyChangeHandler, Idle*, void);

public:
    GtkSalMenu( bool bMenuBar );
    virtual ~GtkSalMenu();

    virtual bool                VisibleMenuBar() override;   // must return TRUE to actually DISPLAY native menu bars
                                                    // otherwise only menu messages are processed (eg, OLE on Windows)

    virtual void                InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos ) override;
    virtual void                RemoveItem( unsigned nPos ) override;
    virtual void                SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos ) override;
    virtual void                SetFrame( const SalFrame* pFrame ) override;
    const GtkSalFrame*          GetFrame() const;
    virtual void                CheckItem( unsigned nPos, bool bCheck ) override;
    virtual void                EnableItem( unsigned nPos, bool bEnable ) override;
    virtual void                ShowItem( unsigned nPos, bool bShow ) override;
    virtual void                SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const OUString& rText ) override;
    virtual void                SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage) override;
    virtual void                SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const vcl::KeyCode& rKeyCode, const OUString& rKeyName ) override;
    virtual void                GetSystemMenuData( SystemMenuData* pData ) override;

    void                        SetMenu( Menu* pMenu ) { mpVCLMenu = pMenu; }
    Menu*                       GetMenu() { return mpVCLMenu; }
    void                        SetMenuModel(GMenuModel* pMenuModel);
    unsigned                    GetItemCount() { return maItems.size(); }
    GtkSalMenuItem*             GetItemAtPos( unsigned nPos ) { return maItems[ nPos ]; }
    void                        SetActionGroup( GActionGroup* pActionGroup ) { mpActionGroup = pActionGroup; }
    bool                        IsItemVisible( unsigned nPos );

    void                        NativeSetItemText( unsigned nSection, unsigned nItemPos, const OUString& rText );
    void                        NativeSetItemIcon( unsigned nSection, unsigned nItemPos, const Image& rImage );
    bool                        NativeSetItemCommand( unsigned nSection,
                                                      unsigned nItemPos,
                                                      sal_uInt16 nId,
                                                      const gchar* aCommand,
                                                      MenuItemBits nBits,
                                                      bool bChecked,
                                                      bool bIsSubmenu );
    void                        NativeSetEnableItem( gchar* aCommand, gboolean bEnable );
    void                        NativeCheckItem( unsigned nSection, unsigned nItemPos, MenuItemBits bits, gboolean bCheck );
    void                        NativeSetAccelerator( unsigned nSection, unsigned nItemPos, const vcl::KeyCode& rKeyCode, const OUString& rKeyName );

    static void                 DispatchCommand(const gchar* pMenuCommand);
    static void                 Activate(const gchar* pMenuCommand);
    static void                 Deactivate(const gchar* pMenuCommand);
    void                        EnableUnity(bool bEnable);
    virtual void                ShowMenuBar( bool bVisible ) override;
    bool                        PrepUpdate();
    virtual void                Update() override;  // Update this menu only.
    // Update full menu hierarchy from this menu.
    void                        UpdateFull () { ActivateAllSubmenus(mpVCLMenu); Update(); }
    // Clear ActionGroup and MenuModel from full menu hierarchy
    void                        ClearActionGroupAndMenuModel();
    GtkSalMenu*                 GetTopLevel();
    void                        SetNeedsUpdate();

    void CreateMenuBarWidget();
    void DestroyMenuBarWidget();
    gboolean SignalKey(GdkEventKey* pEvent);
    void ReturnFocus();

    virtual bool ShowNativePopupMenu(FloatingWindow * pWin, const Rectangle& rRect, FloatWinPopupFlags nFlags) override;
    virtual void ShowCloseButton(bool bShow) override;
    virtual bool CanGetFocus() const override;
    virtual bool TakeFocus() override;
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
