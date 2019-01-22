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
#include <unotools/tempfile.hxx>
#include <vcl/idle.hxx>

#if GTK_CHECK_VERSION(3,0,0)
#  define ENABLE_GMENU_INTEGRATION
#  include <unx/gtk/glomenu.h>
#  include <unx/gtk/gloactiongroup.h>
#elif ENABLE_DBUS && ENABLE_GIO && \
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

#if !GTK_CHECK_VERSION(3,0,0)
typedef void GtkCssProvider;
#endif

class MenuItemList;
class GtkSalMenuItem;

class GtkSalMenu : public SalMenu
{
private:
    std::vector< GtkSalMenuItem* >  maItems;
    Idle                            maUpdateMenuBarIdle;

    bool                            mbInActivateCallback;
    bool const                      mbMenuBar;
    bool                            mbNeedsUpdate;
    bool                            mbReturnFocusToDocument;
    bool                            mbAddedGrab;
    GtkWidget*                      mpMenuBarContainerWidget;
    std::unique_ptr<utl::TempFile>  mxPersonaImage;
    BitmapEx                        maPersonaBitmap;
    GtkWidget*                      mpMenuAllowShrinkWidget;
    GtkWidget*                      mpMenuBarWidget;
    GtkCssProvider*                 mpMenuBarContainerProvider;
    GtkCssProvider*                 mpMenuBarProvider;
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
    void                        NativeSetEnableItem( gchar const * aCommand, gboolean bEnable );
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

    GtkWidget*                  GetMenuBarContainerWidget() const { return mpMenuBarContainerWidget; }

    void CreateMenuBarWidget();
    void DestroyMenuBarWidget();
    gboolean SignalKey(GdkEventKey const * pEvent);
    void ReturnFocus();

    virtual bool ShowNativePopupMenu(FloatingWindow * pWin, const tools::Rectangle& rRect, FloatWinPopupFlags nFlags) override;
    virtual void ShowCloseButton(bool bShow) override;
    virtual bool CanGetFocus() const override;
    virtual bool TakeFocus() override;
    virtual int GetMenuBarHeight() const override;
    virtual void ApplyPersona() override;
};

class GtkSalMenuItem : public SalMenuItem
{
public:
    GtkSalMenuItem( const SalItemParams* );
    virtual ~GtkSalMenuItem() override;

    sal_uInt16 const    mnId;               // Item ID
    MenuItemType const  mnType;             // Item type
    bool                mbVisible;          // Item visibility.
    GtkSalMenu*         mpParentMenu;       // The menu into which this menu item is inserted
    GtkSalMenu*         mpSubMenu;          // Submenu of this item (if defined)
};

#endif // INCLUDED_VCL_INC_UNX_GTK_GTKSALMENU_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
