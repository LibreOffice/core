/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <config_dbus.h>
#include <config_gio.h>

#include <vector>
#if ENABLE_GIO
#include <gio/gio.h>
#endif

#include <salmenu.hxx>
#include <unx/gtk/gtkframe.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/idle.hxx>

#include <unx/gtk/glomenu.h>
#include <unx/gtk/gloactiongroup.h>

class MenuItemList;
class GtkSalMenuItem;

class GtkSalMenu final : public SalMenu
{
private:
    std::vector< GtkSalMenuItem* >  maItems;
    std::vector<std::pair<sal_uInt16, GtkWidget*>> maExtraButtons;
    Idle                            maUpdateMenuBarIdle;

    bool                            mbInActivateCallback;
    bool                            mbMenuBar;
    bool                            mbNeedsUpdate;
    bool                            mbReturnFocusToDocument;
    bool                            mbAddedGrab;
    /// Even setting  null icon on a menuitem can be expensive, so cache state to avoid that call
    bool                            mbHasNullItemIcon = true;
    GtkWidget*                      mpMenuBarContainerWidget;
    GtkWidget*                      mpMenuAllowShrinkWidget;
    GtkWidget*                      mpMenuBarWidget;
    GtkWidget*                      mpMenuWidget;
    GtkWidget*                      mpCloseButton;
    VclPtr<Menu>                    mpVCLMenu;
    GtkSalMenu*                     mpParentSalMenu;
    GtkSalFrame*                    mpFrame;

    // GMenuModel and GActionGroup attributes
    GMenuModel*                     mpMenuModel;
    GActionGroup*                   mpActionGroup;

    void                        ImplUpdate(bool bRecurse, bool bRemoveDisabledEntries);
    void                        ActivateAllSubmenus(Menu* pMenuBar);

    DECL_LINK(MenuBarHierarchyChangeHandler, Timer*, void);

    static GtkWidget* AddButton(GtkWidget *pImage);

public:
    GtkSalMenu( bool bMenuBar );
    virtual ~GtkSalMenu() override;

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

    void                        SetMenu( Menu* pMenu ) { mpVCLMenu = pMenu; }
    Menu*                       GetMenu() { return mpVCLMenu; }
    void                        SetMenuModel(GMenuModel* pMenuModel);
    unsigned                    GetItemCount() const { return maItems.size(); }
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
    void                        NativeSetEnableItem(const OString& sCommand, gboolean bEnable);
    void                        NativeCheckItem( unsigned nSection, unsigned nItemPos, MenuItemBits bits, gboolean bCheck );
    void                        NativeSetAccelerator( unsigned nSection, unsigned nItemPos, const vcl::KeyCode& rKeyCode, std::u16string_view rKeyName );

    static void                 DispatchCommand(const gchar* pMenuCommand);
    static void                 Activate(const gchar* pMenuCommand);
    static void                 Deactivate(const gchar* pMenuCommand);
    void                        EnableUnity(bool bEnable);
    virtual void                ShowMenuBar( bool bVisible ) override;
    bool                        PrepUpdate() const;
    virtual void                Update() override;  // Update this menu only.
    // Update full menu hierarchy from this menu.
    void                        UpdateFull () { ActivateAllSubmenus(mpVCLMenu); }
    // Clear ActionGroup and MenuModel from full menu hierarchy
    void                        ClearActionGroupAndMenuModel();
    GtkSalMenu*                 GetTopLevel();
    void                        SetNeedsUpdate();

    GtkWidget*                  GetMenuBarWidget() const { return mpMenuBarWidget; }
    GtkWidget*                  GetMenuBarContainerWidget() const { return mpMenuBarContainerWidget; }

    void CreateMenuBarWidget();
    void DestroyMenuBarWidget();
#if !GTK_CHECK_VERSION(4, 0, 0)
    gboolean SignalKey(GdkEventKey const * pEvent);
#endif
    void ReturnFocus();

    virtual bool ShowNativePopupMenu(FloatingWindow * pWin, const tools::Rectangle& rRect, FloatWinPopupFlags nFlags) override;
    virtual void ShowCloseButton(bool bShow) override;
    virtual bool AddMenuBarButton( const SalMenuButtonItem& rNewItem ) override;
    virtual void RemoveMenuBarButton( sal_uInt16 nId ) override;
    virtual tools::Rectangle GetMenuBarButtonRectPixel( sal_uInt16 i_nItemId, SalFrame* i_pReferenceFrame ) override;
    virtual bool CanGetFocus() const override;
    virtual bool TakeFocus() override;
    virtual int GetMenuBarHeight() const override;
};

class GtkSalMenuItem final : public SalMenuItem
{
public:
    GtkSalMenuItem( const SalItemParams* );
    virtual ~GtkSalMenuItem() override;

    GtkSalMenu*         mpParentMenu;       // The menu into which this menu item is inserted
    GtkSalMenu*         mpSubMenu;          // Submenu of this item (if defined)
    MenuItemType        mnType;             // Item type
    sal_uInt16          mnId;               // Item ID
    bool                mbVisible;          // Item visibility.
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
