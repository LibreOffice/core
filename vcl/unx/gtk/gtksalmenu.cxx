/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unx/gtk/gtksalmenu.hxx>

#ifdef ENABLE_GMENU_INTEGRATION

#include <unx/gendata.hxx>
#include <unx/saldisp.hxx>
#include <unx/gtk/gtkdata.hxx>
#include <unx/gtk/glomenu.h>
#include <unx/gtk/gloactiongroup.h>
#include <vcl/floatwin.hxx>
#include <vcl/menu.hxx>
#include <vcl/pngwrite.hxx>
#include <unx/gtk/gtkinst.hxx>

#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <window.h>
#include <strings.hrc>

static bool bUnityMode = false;

/*
 * This function generates a unique command name for each menu item
 */
static gchar* GetCommandForItem(GtkSalMenu* pParentMenu, sal_uInt16 nItemId)
{
    OString aCommand("window-");
    aCommand = aCommand + OString::number(reinterpret_cast<unsigned long>(pParentMenu));
    aCommand = aCommand + "-" + OString::number(nItemId);
    return g_strdup(aCommand.getStr());
}

static gchar* GetCommandForItem(GtkSalMenuItem* pSalMenuItem)
{
    return GetCommandForItem(pSalMenuItem->mpParentMenu,
                             pSalMenuItem->mnId);
}

bool GtkSalMenu::PrepUpdate()
{
#if GTK_CHECK_VERSION(3,0,0)
    return mpMenuModel && mpActionGroup;
#else
    return bUnityMode && mpMenuModel && mpActionGroup;
#endif
}

/*
 * Menu updating methods
 */

static void RemoveSpareItemsFromNativeMenu( GLOMenu* pMenu, GList** pOldCommandList, unsigned nSection, unsigned nValidItems )
{
    sal_Int32 nSectionItems = g_lo_menu_get_n_items_from_section( pMenu, nSection );

    while ( nSectionItems > static_cast<sal_Int32>(nValidItems) )
    {
        gchar* aCommand = g_lo_menu_get_command_from_item_in_section( pMenu, nSection, --nSectionItems );

        if ( aCommand != nullptr && pOldCommandList != nullptr )
            *pOldCommandList = g_list_append( *pOldCommandList, g_strdup( aCommand ) );

        g_free( aCommand );

        g_lo_menu_remove_from_section( pMenu, nSection, nSectionItems );
    }
}

typedef std::pair<GtkSalMenu*, sal_uInt16> MenuAndId;

namespace
{
    MenuAndId decode_command(const gchar *action_name)
    {
        OString sCommand(action_name);

        sal_Int32 nIndex = 0;
        OString sWindow = sCommand.getToken(0, '-', nIndex);
        OString sGtkSalMenu = sCommand.getToken(0, '-', nIndex);
        OString sItemId = sCommand.getToken(0, '-', nIndex);

        GtkSalMenu* pSalSubMenu = reinterpret_cast<GtkSalMenu*>(sGtkSalMenu.toInt64());

        assert(sWindow == "window" && pSalSubMenu);
        (void) sWindow;

        return MenuAndId(pSalSubMenu, sItemId.toInt32());
    }
}

static void RemoveDisabledItemsFromNativeMenu(GLOMenu* pMenu, GList** pOldCommandList,
                                       sal_Int32 nSection, GActionGroup* pActionGroup)
{
    while (nSection >= 0)
    {
        sal_Int32 nSectionItems = g_lo_menu_get_n_items_from_section( pMenu, nSection );
        while (nSectionItems--)
        {
            gchar* pCommand = g_lo_menu_get_command_from_item_in_section(pMenu, nSection, nSectionItems);
            // remove disabled entries
            bool bRemove = !g_action_group_get_action_enabled(pActionGroup, pCommand);
            if (!bRemove)
            {
                //also remove any empty submenus
                GLOMenu* pSubMenuModel = g_lo_menu_get_submenu_from_item_in_section(pMenu, nSection, nSectionItems);
                if (pSubMenuModel)
                {
                    gint nSubMenuSections = g_menu_model_get_n_items(G_MENU_MODEL(pSubMenuModel));
                    if (nSubMenuSections == 0)
                        bRemove = true;
                    else if (nSubMenuSections == 1)
                    {
                        gint nItems = g_lo_menu_get_n_items_from_section(pSubMenuModel, 0);
                        if (nItems == 0)
                            bRemove = true;
                        else if (nItems == 1)
                        {
                            //If the only entry is the "No Selection Possible" entry, then we are allowed
                            //to removed it
                            gchar* pSubCommand = g_lo_menu_get_command_from_item_in_section(pSubMenuModel, 0, 0);
                            MenuAndId aMenuAndId(decode_command(pSubCommand));
                            bRemove = aMenuAndId.second == 0xFFFF;
                            g_free(pSubCommand);
                        }
                    }
                }
            }

            if (bRemove)
            {
                //but tdf#86850 Always display clipboard functions
                bRemove = g_strcmp0(pCommand, ".uno:Cut") &&
                          g_strcmp0(pCommand, ".uno:Copy") &&
                          g_strcmp0(pCommand, ".uno:Paste");
            }

            if (bRemove)
            {
                if (pCommand != nullptr && pOldCommandList != nullptr)
                    *pOldCommandList = g_list_append(*pOldCommandList, g_strdup(pCommand));
                g_lo_menu_remove_from_section(pMenu, nSection, nSectionItems);
            }

            g_free(pCommand);
        }
        --nSection;
    }
}

static void RemoveSpareSectionsFromNativeMenu( GLOMenu* pMenu, GList** pOldCommandList, sal_Int32 nLastSection )
{
    if ( pMenu == nullptr || pOldCommandList == nullptr )
        return;

    sal_Int32 n = g_menu_model_get_n_items( G_MENU_MODEL( pMenu ) ) - 1;

    for ( ; n > nLastSection; n--)
    {
        RemoveSpareItemsFromNativeMenu( pMenu, pOldCommandList, n, 0 );
        g_lo_menu_remove( pMenu, n );
    }
}

static gint CompareStr( gpointer str1, gpointer str2 )
{
    return g_strcmp0( static_cast<const gchar*>(str1), static_cast<const gchar*>(str2) );
}

static void RemoveUnusedCommands( GLOActionGroup* pActionGroup, GList* pOldCommandList, GList* pNewCommandList )
{
    if ( pActionGroup == nullptr || pOldCommandList == nullptr )
    {
        g_list_free_full( pOldCommandList, g_free );
        g_list_free_full( pNewCommandList, g_free );
        return;
    }

    while ( pNewCommandList != nullptr )
    {
        GList* pNewCommand = g_list_first( pNewCommandList );
        pNewCommandList = g_list_remove_link( pNewCommandList, pNewCommand );

        gpointer aCommand = g_list_nth_data( pNewCommand, 0 );

        GList* pOldCommand = g_list_find_custom( pOldCommandList, aCommand, reinterpret_cast<GCompareFunc>(CompareStr) );

        if ( pOldCommand != nullptr )
        {
            pOldCommandList = g_list_remove_link( pOldCommandList, pOldCommand );
            g_list_free_full( pOldCommand, g_free );
        }

        g_list_free_full( pNewCommand, g_free );
    }

    while ( pOldCommandList != nullptr )
    {
        GList* pCommand = g_list_first( pOldCommandList );
        pOldCommandList = g_list_remove_link( pOldCommandList, pCommand );

        gchar* aCommand = static_cast<gchar*>(g_list_nth_data( pCommand, 0 ));

        g_lo_action_group_remove( pActionGroup, aCommand );

        g_list_free_full( pCommand, g_free );
    }
}

void GtkSalMenu::ImplUpdate(bool bRecurse, bool bRemoveDisabledEntries)
{
    SolarMutexGuard aGuard;

    SAL_INFO("vcl.unity", "ImplUpdate pre PrepUpdate");
    if( !PrepUpdate() )
        return;

    if (mbNeedsUpdate)
    {
        mbNeedsUpdate = false;
        if (mbMenuBar && maUpdateMenuBarIdle.IsActive())
        {
            maUpdateMenuBarIdle.Stop();
            maUpdateMenuBarIdle.Invoke();
            return;
        }
    }

    Menu* pVCLMenu = mpVCLMenu;
    GLOMenu* pLOMenu = G_LO_MENU( mpMenuModel );
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( mpActionGroup );
    SAL_INFO("vcl.unity", "Syncing vcl menu " << pVCLMenu << " to menu model " << pLOMenu << " and action group " << pActionGroup);
    GList *pOldCommandList = nullptr;
    GList *pNewCommandList = nullptr;

    sal_uInt16 nLOMenuSize = g_menu_model_get_n_items( G_MENU_MODEL( pLOMenu ) );

    if ( nLOMenuSize == 0 )
        g_lo_menu_new_section( pLOMenu, 0, nullptr );

    sal_Int32 nSection = 0;
    sal_Int32 nItemPos = 0;
    sal_Int32 validItems = 0;
    sal_Int32 nItem;

    for ( nItem = 0; nItem < static_cast<sal_Int32>(GetItemCount()); nItem++ ) {
        if ( !IsItemVisible( nItem ) )
            continue;

        GtkSalMenuItem *pSalMenuItem = GetItemAtPos( nItem );
        sal_uInt16 nId = pSalMenuItem->mnId;

        // PopupMenu::ImplExecute might add <No Selection Possible> entry to top-level
        // popup menu, but we have our own implementation below, so skip that one.
        if ( nId == 0xFFFF )
            continue;

        if ( pSalMenuItem->mnType == MenuItemType::SEPARATOR )
        {
            // Delete extra items from current section.
            RemoveSpareItemsFromNativeMenu( pLOMenu, &pOldCommandList, nSection, validItems );

            nSection++;
            nItemPos = 0;
            validItems = 0;

            if ( nLOMenuSize <= nSection )
            {
                g_lo_menu_new_section( pLOMenu, nSection, nullptr );
                nLOMenuSize++;
            }

            continue;
        }

        if ( nItemPos >= g_lo_menu_get_n_items_from_section( pLOMenu, nSection ) )
            g_lo_menu_insert_in_section( pLOMenu, nSection, nItemPos, "EMPTY STRING" );

        // Get internal menu item values.
        OUString aText = pVCLMenu->GetItemText( nId );
        Image aImage = pVCLMenu->GetItemImage( nId );
        bool bEnabled = pVCLMenu->IsItemEnabled( nId );
        vcl::KeyCode nAccelKey = pVCLMenu->GetAccelKey( nId );
        bool bChecked = pVCLMenu->IsItemChecked( nId );
        MenuItemBits itemBits = pVCLMenu->GetItemBits( nId );

        // Store current item command in command list.
        gchar *aCurrentCommand = g_lo_menu_get_command_from_item_in_section( pLOMenu, nSection, nItemPos );

        if ( aCurrentCommand != nullptr )
            pOldCommandList = g_list_append( pOldCommandList, aCurrentCommand );

        // Get the new command for the item.
        gchar* aNativeCommand = GetCommandForItem(pSalMenuItem);

        // Force updating of native menu labels.
        NativeSetItemText( nSection, nItemPos, aText );
        NativeSetItemIcon( nSection, nItemPos, aImage );
        NativeSetAccelerator( nSection, nItemPos, nAccelKey, nAccelKey.GetName( GetFrame()->GetWindow() ) );

        if ( g_strcmp0( aNativeCommand, "" ) != 0 && pSalMenuItem->mpSubMenu == nullptr )
        {
            NativeSetItemCommand( nSection, nItemPos, nId, aNativeCommand, itemBits, bChecked, false );
            NativeCheckItem( nSection, nItemPos, itemBits, bChecked );
            NativeSetEnableItem( aNativeCommand, bEnabled );

            pNewCommandList = g_list_append( pNewCommandList, g_strdup( aNativeCommand ) );
        }

        GtkSalMenu* pSubmenu = pSalMenuItem->mpSubMenu;

        if ( pSubmenu && pSubmenu->GetMenu() )
        {
            bool bNonMenuChangedToMenu = NativeSetItemCommand( nSection, nItemPos, nId, aNativeCommand, itemBits, false, true );
            pNewCommandList = g_list_append( pNewCommandList, g_strdup( aNativeCommand ) );

            GLOMenu* pSubMenuModel = g_lo_menu_get_submenu_from_item_in_section( pLOMenu, nSection, nItemPos );

            if ( pSubMenuModel == nullptr )
            {
                g_lo_menu_new_submenu_in_item_in_section( pLOMenu, nSection, nItemPos );
                pSubMenuModel = g_lo_menu_get_submenu_from_item_in_section( pLOMenu, nSection, nItemPos );
            }

            g_object_unref( pSubMenuModel );

            if (bRecurse || bNonMenuChangedToMenu)
            {
                SAL_INFO("vcl.unity", "preparing submenu  " << pSubMenuModel << " to menu model " << G_MENU_MODEL(pSubMenuModel) << " and action group " << G_ACTION_GROUP(pActionGroup));
                pSubmenu->SetMenuModel( G_MENU_MODEL( pSubMenuModel ) );
                pSubmenu->SetActionGroup( G_ACTION_GROUP( pActionGroup ) );
                pSubmenu->ImplUpdate(true, bRemoveDisabledEntries);
            }
        }

        g_free( aNativeCommand );

        ++nItemPos;
        ++validItems;
    }

    if (bRemoveDisabledEntries)
    {
        // Delete disabled items in last section.
        RemoveDisabledItemsFromNativeMenu(pLOMenu, &pOldCommandList, nSection, G_ACTION_GROUP(pActionGroup));
    }

    // Delete extra items in last section.
    RemoveSpareItemsFromNativeMenu( pLOMenu, &pOldCommandList, nSection, validItems );

    // Delete extra sections.
    RemoveSpareSectionsFromNativeMenu( pLOMenu, &pOldCommandList, nSection );

    // Delete unused commands.
    RemoveUnusedCommands( pActionGroup, pOldCommandList, pNewCommandList );

    // Resolves: tdf#103166 if the menu is empty, add a disabled
    // <No Selection Possible> placeholder.
    sal_Int32 nSectionsCount = g_menu_model_get_n_items(G_MENU_MODEL(pLOMenu));
    gint nItemsCount = 0;
    for (nSection = 0; nSection < nSectionsCount; ++nSection)
    {
        nItemsCount += g_lo_menu_get_n_items_from_section(pLOMenu, nSection);
        if (nItemsCount)
            break;
    }
    if (!nItemsCount)
    {
        gchar* aNativeCommand = GetCommandForItem(this, 0xFFFF);
        OUString aPlaceholderText(VclResId(SV_RESID_STRING_NOSELECTIONPOSSIBLE));
        g_lo_menu_insert_in_section(pLOMenu, nSection-1, 0,
                                    OUStringToOString(aPlaceholderText, RTL_TEXTENCODING_UTF8).getStr());
        NativeSetItemCommand(nSection-1, 0, 0xFFFF, aNativeCommand, MenuItemBits::NONE, false, false);
        NativeSetEnableItem(aNativeCommand, false);
        g_free(aNativeCommand);
    }
}

void GtkSalMenu::Update()
{
    //find out if top level is a menubar or not, if not, then it's a popup menu
    //hierarchy and in those we hide (most) disabled entries
    const GtkSalMenu* pMenu = this;
    while (pMenu->mpParentSalMenu)
        pMenu = pMenu->mpParentSalMenu;
    ImplUpdate(false, !pMenu->mbMenuBar);
}

#if GTK_CHECK_VERSION(3,0,0)
static void MenuPositionFunc(GtkMenu* menu, gint* x, gint* y, gboolean* push_in, gpointer user_data)
{
    Point *pPos = static_cast<Point*>(user_data);
    *x = pPos->X();
    if (gtk_widget_get_default_direction() == GTK_TEXT_DIR_RTL)
    {
        GtkRequisition natural_size;
        gtk_widget_get_preferred_size(GTK_WIDGET(menu), nullptr, &natural_size);
        *x -= natural_size.width;
    }
    *y = pPos->Y();
    *push_in = false;
}
#endif

bool GtkSalMenu::ShowNativePopupMenu(FloatingWindow* pWin, const tools::Rectangle& rRect,
                                     FloatWinPopupFlags nFlags)
{
#if GTK_CHECK_VERSION(3,0,0)
    VclPtr<vcl::Window> xParent = pWin->ImplGetWindowImpl()->mpRealParent;
    mpFrame = static_cast<GtkSalFrame*>(xParent->ImplGetFrame());

    GLOActionGroup* pActionGroup = g_lo_action_group_new();
    mpActionGroup = G_ACTION_GROUP(pActionGroup);
    mpMenuModel = G_MENU_MODEL(g_lo_menu_new());
    // Generate the main menu structure, populates mpMenuModel
    UpdateFull();

    GtkWidget *pWidget = gtk_menu_new_from_model(mpMenuModel);
    gtk_menu_attach_to_widget(GTK_MENU(pWidget), mpFrame->getMouseEventWidget(), nullptr);
    gtk_widget_insert_action_group(mpFrame->getMouseEventWidget(), "win", mpActionGroup);

    //run in a sub main loop because we need to keep vcl PopupMenu alive to use
    //it during DispatchCommand, returning now to the outer loop causes the
    //launching PopupMenu to be destroyed, instead run the subloop here
    //until the gtk menu is destroyed
    GMainLoop* pLoop = g_main_loop_new(nullptr, true);
    g_signal_connect_swapped(G_OBJECT(pWidget), "deactivate", G_CALLBACK(g_main_loop_quit), pLoop);

#if GTK_CHECK_VERSION(3,22,0)
    if (gtk_check_version(3, 22, 0) == nullptr)
    {
        GdkGravity rect_anchor = GDK_GRAVITY_SOUTH_WEST, menu_anchor = GDK_GRAVITY_NORTH_WEST;

        if (nFlags & FloatWinPopupFlags::Left)
        {
            rect_anchor = GDK_GRAVITY_NORTH_WEST;
            menu_anchor = GDK_GRAVITY_NORTH_EAST;
        }
        else if (nFlags & FloatWinPopupFlags::Up)
        {
            rect_anchor = GDK_GRAVITY_NORTH_WEST;
            menu_anchor = GDK_GRAVITY_SOUTH_WEST;
        }
        else if (nFlags & FloatWinPopupFlags::Right)
        {
            rect_anchor = GDK_GRAVITY_NORTH_EAST;
        }

        tools::Rectangle aFloatRect = FloatingWindow::ImplConvertToAbsPos(xParent, rRect);
        aFloatRect.Move(-mpFrame->maGeometry.nX, -mpFrame->maGeometry.nY);
        GdkRectangle rect {static_cast<int>(aFloatRect.Left()), static_cast<int>(aFloatRect.Top()),
                           static_cast<int>(aFloatRect.GetWidth()), static_cast<int>(aFloatRect.GetHeight())};

        GdkWindow* gdkWindow = widget_get_window(mpFrame->getMouseEventWidget());
        gtk_menu_popup_at_rect(GTK_MENU(pWidget), gdkWindow, &rect, rect_anchor, menu_anchor, nullptr);
    }
    else
#endif
    {
        guint nButton;
        guint32 nTime;

        //typically there is an event, and we can then distinguish if this was
        //launched from the keyboard (gets auto-mnemoniced) or the mouse (which
        //doesn't)
        GdkEvent *pEvent = gtk_get_current_event();
        if (pEvent)
        {
            gdk_event_get_button(pEvent, &nButton);
            nTime = gdk_event_get_time(pEvent);
        }
        else
        {
            nButton = 0;
            nTime = GtkSalFrame::GetLastInputEventTime();
        }

        // do the same strange semantics as vcl popup windows to arrive at a frame geometry
        // in mirrored UI case; best done by actually executing the same code
        sal_uInt16 nArrangeIndex;
        Point aPos = FloatingWindow::ImplCalcPos(pWin, rRect, nFlags, nArrangeIndex);
        aPos = FloatingWindow::ImplConvertToAbsPos(xParent, aPos);

        gtk_menu_popup(GTK_MENU(pWidget), nullptr, nullptr, MenuPositionFunc,
                       &aPos, nButton, nTime);
    }

    if (g_main_loop_is_running(pLoop))
    {
        gdk_threads_leave();
        g_main_loop_run(pLoop);
        gdk_threads_enter();
    }
    g_main_loop_unref(pLoop);

    mpVCLMenu->Deactivate();

    gtk_widget_insert_action_group(mpFrame->getMouseEventWidget(), "win", nullptr);

    gtk_widget_destroy(pWidget);

    g_object_unref(mpActionGroup);
    ClearActionGroupAndMenuModel();

    mpFrame = nullptr;

    return true;
#else
    (void)pWin;
    (void)rRect;
    (void)nFlags;
    return false;
#endif
}

/*
 * GtkSalMenu
 */

GtkSalMenu::GtkSalMenu( bool bMenuBar ) :
    mbInActivateCallback( false ),
    mbMenuBar( bMenuBar ),
    mbNeedsUpdate( false ),
    mbReturnFocusToDocument( false ),
    mbAddedGrab( false ),
    mpMenuBarContainerWidget( nullptr ),
    mpMenuAllowShrinkWidget( nullptr ),
    mpMenuBarWidget( nullptr ),
    mpMenuBarContainerProvider( nullptr ),
    mpMenuBarProvider( nullptr ),
    mpCloseButton( nullptr ),
    mpVCLMenu( nullptr ),
    mpParentSalMenu( nullptr ),
    mpFrame( nullptr ),
    mpMenuModel( nullptr ),
    mpActionGroup( nullptr )
{
    //typically this only gets called after the menu has been customized on the
    //next idle slot, in the normal case of a new menubar SetFrame is called
    //directly long before this idle would get called.
    maUpdateMenuBarIdle.SetPriority(TaskPriority::HIGHEST);
    maUpdateMenuBarIdle.SetInvokeHandler(LINK(this, GtkSalMenu, MenuBarHierarchyChangeHandler));
    maUpdateMenuBarIdle.SetDebugName("Native Gtk Menu Update Idle");
}

IMPL_LINK_NOARG(GtkSalMenu, MenuBarHierarchyChangeHandler, Timer *, void)
{
    SAL_WARN_IF(!mpFrame, "vcl.gtk", "MenuBar layout changed, but no frame for some reason!");
    if (!mpFrame)
        return;
    SetFrame(mpFrame);
}

void GtkSalMenu::SetNeedsUpdate()
{
    GtkSalMenu* pMenu = this;
    // start that the menu and its parents are in need of an update
    // on the next activation
    while (pMenu && !pMenu->mbNeedsUpdate)
    {
        pMenu->mbNeedsUpdate = true;
        pMenu = pMenu->mpParentSalMenu;
    }
    // only if a menubar is directly updated do we force in a full
    // structure update
    if (mbMenuBar && !maUpdateMenuBarIdle.IsActive())
        maUpdateMenuBarIdle.Start();
}

void GtkSalMenu::SetMenuModel(GMenuModel* pMenuModel)
{
    if (mpMenuModel)
        g_object_unref(mpMenuModel);
    mpMenuModel = pMenuModel;
    if (mpMenuModel)
        g_object_ref(mpMenuModel);
}

GtkSalMenu::~GtkSalMenu()
{
    SolarMutexGuard aGuard;

    DestroyMenuBarWidget();

    if (mpMenuModel)
        g_object_unref(mpMenuModel);

    maItems.clear();

    if (mpFrame)
        mpFrame->SetMenu(nullptr);
}

bool GtkSalMenu::VisibleMenuBar()
{
    return mbMenuBar && (bUnityMode || mpMenuBarContainerWidget);
}

void GtkSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    SolarMutexGuard aGuard;
    GtkSalMenuItem *pItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );

    if ( nPos == MENU_APPEND )
        maItems.push_back( pItem );
    else
        maItems.insert( maItems.begin() + nPos, pItem );

    pItem->mpParentMenu = this;

    SetNeedsUpdate();
}

void GtkSalMenu::RemoveItem( unsigned nPos )
{
    SolarMutexGuard aGuard;
    maItems.erase( maItems.begin() + nPos );
    SetNeedsUpdate();
}

void GtkSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned )
{
    SolarMutexGuard aGuard;
    GtkSalMenuItem *pItem = static_cast< GtkSalMenuItem* >( pSalMenuItem );
    GtkSalMenu *pGtkSubMenu = static_cast< GtkSalMenu* >( pSubMenu );

    if ( pGtkSubMenu == nullptr )
        return;

    pGtkSubMenu->mpParentSalMenu = this;
    pItem->mpSubMenu = pGtkSubMenu;

    SetNeedsUpdate();
}

#if GTK_CHECK_VERSION(3,0,0)
static void CloseMenuBar(GtkWidget *, gpointer pMenu)
{
    Application::PostUserEvent(static_cast<MenuBar*>(pMenu)->GetCloseButtonClickHdl());
}
#endif

void GtkSalMenu::ShowCloseButton(bool bShow)
{
#if GTK_CHECK_VERSION(3,0,0)
    assert(mbMenuBar);
    if (!mpMenuBarContainerWidget)
        return;

    if (!bShow)
    {
        if (mpCloseButton)
            gtk_widget_destroy(mpCloseButton);
        return;
    }

    MenuBar *pVclMenuBar = static_cast<MenuBar*>(mpVCLMenu.get());
    mpCloseButton = gtk_button_new();
    g_signal_connect(mpCloseButton, "clicked", G_CALLBACK(CloseMenuBar), pVclMenuBar);

    gtk_button_set_relief(GTK_BUTTON(mpCloseButton), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click(GTK_BUTTON(mpCloseButton), false);
    gtk_widget_set_can_focus(mpCloseButton, false);

    GtkStyleContext *pButtonContext = gtk_widget_get_style_context(GTK_WIDGET(mpCloseButton));

    GtkCssProvider *pProvider = gtk_css_provider_new();
    static const gchar data[] = "* { "
      "padding: 0;"
      "margin-left: 8px;"
      "margin-right: 8px;"
      "min-width: 18px;"
      "min-height: 18px;"
      "}";
    const gchar olddata[] = "* { "
      "padding: 0;"
      "margin-left: 8px;"
      "margin-right: 8px;"
      "}";
    gtk_css_provider_load_from_data(pProvider, gtk_check_version(3, 20, 0) == nullptr ? data : olddata, -1, nullptr);
    gtk_style_context_add_provider(pButtonContext,
                                   GTK_STYLE_PROVIDER(pProvider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_style_context_add_class(pButtonContext, "flat");
    gtk_style_context_add_class(pButtonContext, "small-button");

    GIcon* icon = g_themed_icon_new_with_default_fallbacks("window-close-symbolic");
    GtkWidget* image = gtk_image_new_from_gicon(icon, GTK_ICON_SIZE_MENU);
    gtk_widget_show(image);
    g_object_unref(icon);

    OUString sToolTip(VclResId(SV_HELPTEXT_CLOSEDOCUMENT));
    gtk_widget_set_tooltip_text(mpCloseButton,
        OUStringToOString(sToolTip, RTL_TEXTENCODING_UTF8).getStr());

    gtk_widget_set_valign(mpCloseButton, GTK_ALIGN_CENTER);

    gtk_container_add(GTK_CONTAINER(mpCloseButton), image);
    gtk_grid_attach(GTK_GRID(mpMenuBarContainerWidget), GTK_WIDGET(mpCloseButton), 1, 0, 1, 1);
    gtk_widget_show_all(mpCloseButton);
#else
    (void)bShow;
    (void)mpMenuBarContainerWidget;
    (void)mpCloseButton;
#endif
}

//Typically when the menubar is deactivated we want the focus to return
//to where it came from. If the menubar was activated because of F6
//moving focus into the associated VCL menubar then on pressing ESC
//or any other normal reason for deactivation we want focus to return
//to the document, definitely not still stuck in the associated
//VCL menubar. But if F6 is pressed while the menubar is activated
//we want to pass that F6 back to the VCL menubar which will move
//focus to the next pane by itself.
void GtkSalMenu::ReturnFocus()
{
    if (mbAddedGrab)
    {
        gtk_grab_remove(mpMenuBarWidget);
        mbAddedGrab = false;
    }
    if (!mbReturnFocusToDocument)
        gtk_widget_grab_focus(GTK_WIDGET(mpFrame->getEventBox()));
    else
        mpFrame->GetWindow()->GrabFocusToDocument();
    mbReturnFocusToDocument = false;
}

gboolean GtkSalMenu::SignalKey(GdkEventKey const * pEvent)
{
    if (pEvent->keyval == GDK_KEY_F6)
    {
        mbReturnFocusToDocument = false;
        gtk_menu_shell_cancel(GTK_MENU_SHELL(mpMenuBarWidget));
        //because we return false here, the keypress will continue
        //to propagate and in the case that vcl focus is in
        //the vcl menubar then that will also process F6 and move
        //to the next pane
    }
    return false;
}

//The GtkSalMenu is owner by a Vcl Menu/MenuBar. In the menubar
//case the vcl menubar is present and "visible", but with a 0 height
//so it not apparent. Normally it acts as though it is not there when
//a Native menubar is active. If we return true here, then for keyboard
//activation and traversal with F6 through panes then the vcl menubar
//acts as though it *is* present and we translate its take focus and F6
//traversal key events into the gtk menubar equivalents.
bool GtkSalMenu::CanGetFocus() const
{
    return mpMenuBarWidget != nullptr;
}

bool GtkSalMenu::TakeFocus()
{
    if (!mpMenuBarWidget)
        return false;

    //Send a keyboard event to the gtk menubar to let it know it has been
    //activated via the keyboard. Doesn't do anything except cause the gtk
    //menubar "keyboard_mode" member to get set to true, so typically mnemonics
    //are shown which will serve as indication that the menubar has focus
    //(given that we wnt to show it with no menus popped down)
    GdkEvent *event = gdk_event_new(GDK_KEY_PRESS);
    event->key.window = GDK_WINDOW(g_object_ref(gtk_widget_get_window(mpMenuBarWidget)));
    event->key.send_event = 1 /* TRUE */;
    event->key.time = gtk_get_current_event_time();
    event->key.state = 0;
    event->key.keyval = 0;
    event->key.length = 0;
    event->key.string = nullptr;
    event->key.hardware_keycode = 0;
    event->key.group = 0;
    event->key.is_modifier = false;
    gtk_widget_event(mpMenuBarWidget, event);
    gdk_event_free(event);

    //this pairing results in a menubar with keyboard focus with no menus
    //auto-popped down
    gtk_grab_add(mpMenuBarWidget);
    mbAddedGrab = true;
    gtk_menu_shell_select_first(GTK_MENU_SHELL(mpMenuBarWidget), false);
    gtk_menu_shell_deselect(GTK_MENU_SHELL(mpMenuBarWidget));
    mbReturnFocusToDocument = true;
    return true;
}

#if GTK_CHECK_VERSION(3,0,0)

static void MenuBarReturnFocus(GtkMenuShell*, gpointer menu)
{
    GtkSalFrame::UpdateLastInputEventTime(gtk_get_current_event_time());
    GtkSalMenu* pMenu = static_cast<GtkSalMenu*>(menu);
    pMenu->ReturnFocus();
}

static gboolean MenuBarSignalKey(GtkWidget*, GdkEventKey* pEvent, gpointer menu)
{
    GtkSalMenu* pMenu = static_cast<GtkSalMenu*>(menu);
    return pMenu->SignalKey(pEvent);
}

#endif

void GtkSalMenu::CreateMenuBarWidget()
{
#if GTK_CHECK_VERSION(3,0,0)
    if (mpMenuBarContainerWidget)
        return;

    GtkGrid* pGrid = mpFrame->getTopLevelGridWidget();
    mpMenuBarContainerWidget = gtk_grid_new();

    gtk_widget_set_hexpand(GTK_WIDGET(mpMenuBarContainerWidget), true);
    gtk_grid_insert_row(pGrid, 0);
    gtk_grid_attach(pGrid, mpMenuBarContainerWidget, 0, 0, 1, 1);

    mpMenuAllowShrinkWidget = gtk_scrolled_window_new(nullptr, nullptr);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(mpMenuAllowShrinkWidget), GTK_SHADOW_NONE);
    // tdf#116290 external policy on scrolledwindow will not show a scrollbar,
    // but still allow scrolled window to not be sized to the child content.
    // So the menubar can be shrunk past its nominal smallest width.
    // Unlike a hack using GtkFixed/GtkLayout the correct placement of the menubar occurs under RTL
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(mpMenuAllowShrinkWidget), GTK_POLICY_EXTERNAL, GTK_POLICY_NEVER);
    gtk_grid_attach(GTK_GRID(mpMenuBarContainerWidget), mpMenuAllowShrinkWidget, 0, 0, 1, 1);

    mpMenuBarWidget = gtk_menu_bar_new_from_model(mpMenuModel);

    gtk_widget_insert_action_group(mpMenuBarWidget, "win", mpActionGroup);
    gtk_widget_set_hexpand(GTK_WIDGET(mpMenuBarWidget), true);
    gtk_widget_set_hexpand(mpMenuAllowShrinkWidget, true);
    gtk_container_add(GTK_CONTAINER(mpMenuAllowShrinkWidget), mpMenuBarWidget);

    g_signal_connect(G_OBJECT(mpMenuBarWidget), "deactivate", G_CALLBACK(MenuBarReturnFocus), this);
    g_signal_connect(G_OBJECT(mpMenuBarWidget), "key-press-event", G_CALLBACK(MenuBarSignalKey), this);

    gtk_widget_show_all(mpMenuBarContainerWidget);

    ShowCloseButton( static_cast<MenuBar*>(mpVCLMenu.get())->HasCloseButton() );

    ApplyPersona();
#else
    (void)mpMenuAllowShrinkWidget;
    (void)mpMenuBarContainerWidget;
#endif
}

void GtkSalMenu::ApplyPersona()
{
#if GTK_CHECK_VERSION(3,0,0)
    if (!mpMenuBarContainerWidget)
        return;
    assert(mbMenuBar);
    // I'm dubious about the persona theming feature, but as it exists, lets try and support
    // it, apply the image to the mpMenuBarContainerWidget
    const BitmapEx& rPersonaBitmap = Application::GetSettings().GetStyleSettings().GetPersonaHeader();

    GtkStyleContext *pMenuBarContainerContext = gtk_widget_get_style_context(GTK_WIDGET(mpMenuBarContainerWidget));
    if (mpMenuBarContainerProvider)
    {
        gtk_style_context_remove_provider(pMenuBarContainerContext, GTK_STYLE_PROVIDER(mpMenuBarContainerProvider));
        mpMenuBarContainerProvider = nullptr;
    }
    GtkStyleContext *pMenuBarContext = gtk_widget_get_style_context(GTK_WIDGET(mpMenuBarWidget));
    if (mpMenuBarProvider)
    {
        gtk_style_context_remove_provider(pMenuBarContext, GTK_STYLE_PROVIDER(mpMenuBarProvider));
        mpMenuBarProvider = nullptr;
    }

    if (!rPersonaBitmap.IsEmpty())
    {
        if (maPersonaBitmap != rPersonaBitmap)
        {
            vcl::PNGWriter aPNGWriter(rPersonaBitmap);
            mxPersonaImage.reset(new utl::TempFile);
            mxPersonaImage->EnableKillingFile(true);
            SvStream* pStream = mxPersonaImage->GetStream(StreamMode::WRITE);
            aPNGWriter.Write(*pStream);
            mxPersonaImage->CloseStream();
        }

        mpMenuBarContainerProvider = gtk_css_provider_new();
        OUString aBuffer = "* { background-image: url(\"" + mxPersonaImage->GetURL() + "\"); background-position: top right; }";
        OString aResult = OUStringToOString(aBuffer, RTL_TEXTENCODING_UTF8);
        gtk_css_provider_load_from_data(mpMenuBarContainerProvider, aResult.getStr(), aResult.getLength(), nullptr);
        gtk_style_context_add_provider(pMenuBarContainerContext, GTK_STYLE_PROVIDER(mpMenuBarContainerProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);


        // force the menubar to be transparent when persona is active otherwise for
        // me the menubar becomes gray when its in the backdrop
        mpMenuBarProvider = gtk_css_provider_new();
        static const gchar data[] = "* { "
          "background-image: none;"
          "background-color: transparent;"
          "}";
        gtk_css_provider_load_from_data(mpMenuBarProvider, data, -1, nullptr);
        gtk_style_context_add_provider(pMenuBarContext,
                                       GTK_STYLE_PROVIDER(mpMenuBarProvider),
                                       GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    }
    maPersonaBitmap = rPersonaBitmap;
#else
    (void)maPersonaBitmap;
    (void)mpMenuBarContainerProvider;
    (void)mpMenuBarProvider;
#endif
}

void GtkSalMenu::DestroyMenuBarWidget()
{
#if GTK_CHECK_VERSION(3,0,0)
    if (mpMenuBarContainerWidget)
    {
        gtk_widget_destroy(mpMenuBarContainerWidget);
        mpMenuBarContainerWidget = nullptr;
        mpCloseButton = nullptr;
    }
#else
    (void)mpMenuBarContainerWidget;
#endif
}

void GtkSalMenu::SetFrame(const SalFrame* pFrame)
{
    SolarMutexGuard aGuard;
    assert(mbMenuBar);
    SAL_INFO("vcl.unity", "GtkSalMenu set to frame");
    mpFrame = const_cast<GtkSalFrame*>(static_cast<const GtkSalFrame*>(pFrame));

    // if we had a menu on the GtkSalMenu we have to free it as we generate a
    // full menu anyway and we might need to reuse an existing model and
    // actiongroup
    mpFrame->SetMenu( this );
    mpFrame->EnsureAppMenuWatch();

    // Clean menu model and action group if needed.
    GtkWidget* pWidget = mpFrame->getWindow();
    GdkWindow* gdkWindow = gtk_widget_get_window( pWidget );

    GLOMenu* pMenuModel = G_LO_MENU( g_object_get_data( G_OBJECT( gdkWindow ), "g-lo-menubar" ) );
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( g_object_get_data( G_OBJECT( gdkWindow ), "g-lo-action-group" ) );
    SAL_INFO("vcl.unity", "Found menu model: " << pMenuModel << " and action group: " << pActionGroup);

    if ( pMenuModel )
    {
        if ( g_menu_model_get_n_items( G_MENU_MODEL( pMenuModel ) ) > 0 )
            g_lo_menu_remove( pMenuModel, 0 );

        mpMenuModel = G_MENU_MODEL( g_lo_menu_new() );
    }

    if ( pActionGroup )
    {
        g_lo_action_group_clear( pActionGroup );
        mpActionGroup = G_ACTION_GROUP( pActionGroup );
    }

    // Generate the main menu structure.
    if ( PrepUpdate() )
        UpdateFull();

    g_lo_menu_insert_section( pMenuModel, 0, nullptr, mpMenuModel );

    if (!bUnityMode && static_cast<MenuBar*>(mpVCLMenu.get())->IsDisplayable())
    {
        DestroyMenuBarWidget();
        CreateMenuBarWidget();
    }
}

const GtkSalFrame* GtkSalMenu::GetFrame() const
{
    SolarMutexGuard aGuard;
    const GtkSalMenu* pMenu = this;
    while( pMenu && ! pMenu->mpFrame )
        pMenu = pMenu->mpParentSalMenu;
    return pMenu ? pMenu->mpFrame : nullptr;
}

void GtkSalMenu::NativeCheckItem( unsigned nSection, unsigned nItemPos, MenuItemBits bits, gboolean bCheck )
{
    SolarMutexGuard aGuard;

    if ( mpActionGroup == nullptr )
        return;

    gchar* aCommand = g_lo_menu_get_command_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos );

    if ( aCommand != nullptr || g_strcmp0( aCommand, "" ) != 0 )
    {
        GVariant *pCheckValue = nullptr;
        GVariant *pCurrentState = g_action_group_get_action_state( mpActionGroup, aCommand );

        if ( bits & MenuItemBits::RADIOCHECK )
            pCheckValue = bCheck ? g_variant_new_string( aCommand ) : g_variant_new_string( "" );
        else
        {
            // By default, all checked items are checkmark buttons.
            if (bCheck || pCurrentState != nullptr)
                pCheckValue = g_variant_new_boolean( bCheck );
        }

        if ( pCheckValue != nullptr )
        {
            if ( pCurrentState == nullptr || g_variant_equal( pCurrentState, pCheckValue ) == FALSE )
            {
                g_action_group_change_action_state( mpActionGroup, aCommand, pCheckValue );
            }
            else
            {
                g_variant_unref (pCheckValue);
            }
        }

        if ( pCurrentState != nullptr )
            g_variant_unref( pCurrentState );
    }

    if ( aCommand )
        g_free( aCommand );
}

void GtkSalMenu::NativeSetEnableItem( gchar const * aCommand, gboolean bEnable )
{
    SolarMutexGuard aGuard;
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( mpActionGroup );

    if ( g_action_group_get_action_enabled( G_ACTION_GROUP( pActionGroup ), aCommand ) != bEnable )
        g_lo_action_group_set_action_enabled( pActionGroup, aCommand, bEnable );
}

void GtkSalMenu::NativeSetItemText( unsigned nSection, unsigned nItemPos, const OUString& rText )
{
    SolarMutexGuard aGuard;
    // Escape all underscores so that they don't get interpreted as hotkeys
    OUString aText = rText.replaceAll( "_", "__" );
    // Replace the LibreOffice hotkey identifier with an underscore
    aText = aText.replace( '~', '_' );
    OString aConvertedText = OUStringToOString( aText, RTL_TEXTENCODING_UTF8 );

    // Update item text only when necessary.
    gchar* aLabel = g_lo_menu_get_label_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos );

    if ( aLabel == nullptr || g_strcmp0( aLabel, aConvertedText.getStr() ) != 0 )
        g_lo_menu_set_label_to_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos, aConvertedText.getStr() );

    if ( aLabel )
        g_free( aLabel );
}

namespace
{
    void DestroyMemoryStream(gpointer data)
    {
        SvMemoryStream* pMemStm = static_cast<SvMemoryStream*>(data);
        delete pMemStm;
    }
}

void GtkSalMenu::NativeSetItemIcon( unsigned nSection, unsigned nItemPos, const Image& rImage )
{
#if GLIB_CHECK_VERSION(2,38,0)
    SolarMutexGuard aGuard;

    if (!!rImage)
    {
        SvMemoryStream* pMemStm = new SvMemoryStream;
        vcl::PNGWriter aWriter(rImage.GetBitmapEx());
        aWriter.Write(*pMemStm);

        GBytes *pBytes = g_bytes_new_with_free_func(pMemStm->GetData(),
                                                    pMemStm->TellEnd(),
                                                    DestroyMemoryStream,
                                                    pMemStm);

        GIcon *pIcon = g_bytes_icon_new(pBytes);

        g_lo_menu_set_icon_to_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos, pIcon );
        g_object_unref(pIcon);
        g_bytes_unref(pBytes);
    }
    else
        g_lo_menu_set_icon_to_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos, nullptr );
#else
    (void)nSection;
    (void)nItemPos;
    (void)rImage;
#endif
}

void GtkSalMenu::NativeSetAccelerator( unsigned nSection, unsigned nItemPos, const vcl::KeyCode& rKeyCode, const OUString& rKeyName )
{
    SolarMutexGuard aGuard;

    if ( rKeyName.isEmpty() )
        return;

    guint nKeyCode;
    GdkModifierType nModifiers;
    GtkSalFrame::KeyCodeToGdkKey(rKeyCode, &nKeyCode, &nModifiers);

    gchar* aAccelerator = gtk_accelerator_name( nKeyCode, nModifiers );

    gchar* aCurrentAccel = g_lo_menu_get_accelerator_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos );

    if ( aCurrentAccel == nullptr && g_strcmp0( aCurrentAccel, aAccelerator ) != 0 )
        g_lo_menu_set_accelerator_to_item_in_section ( G_LO_MENU( mpMenuModel ), nSection, nItemPos, aAccelerator );

    g_free( aAccelerator );
    g_free( aCurrentAccel );
}

bool GtkSalMenu::NativeSetItemCommand( unsigned nSection,
                                       unsigned nItemPos,
                                       sal_uInt16 nId,
                                       const gchar* aCommand,
                                       MenuItemBits nBits,
                                       bool bChecked,
                                       bool bIsSubmenu )
{
    bool bSubMenuAddedOrRemoved = false;

    SolarMutexGuard aGuard;
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( mpActionGroup );

    GVariant *pTarget = nullptr;

    if (g_action_group_has_action(mpActionGroup, aCommand))
        g_lo_action_group_remove(pActionGroup, aCommand);

    if ( ( nBits & MenuItemBits::CHECKABLE ) || bIsSubmenu )
    {
        // Item is a checkmark button.
        GVariantType* pStateType = g_variant_type_new( reinterpret_cast<gchar const *>(G_VARIANT_TYPE_BOOLEAN) );
        GVariant* pState = g_variant_new_boolean( bChecked );

        g_lo_action_group_insert_stateful( pActionGroup, aCommand, nId, bIsSubmenu, nullptr, pStateType, nullptr, pState );
    }
    else if ( nBits & MenuItemBits::RADIOCHECK )
    {
        // Item is a radio button.
        GVariantType* pParameterType = g_variant_type_new( reinterpret_cast<gchar const *>(G_VARIANT_TYPE_STRING) );
        GVariantType* pStateType = g_variant_type_new( reinterpret_cast<gchar const *>(G_VARIANT_TYPE_STRING) );
        GVariant* pState = g_variant_new_string( "" );
        pTarget = g_variant_new_string( aCommand );

        g_lo_action_group_insert_stateful( pActionGroup, aCommand, nId, FALSE, pParameterType, pStateType, nullptr, pState );
    }
    else
    {
        // Item is not special, so insert a stateless action.
        g_lo_action_group_insert( pActionGroup, aCommand, nId, FALSE );
    }

    GLOMenu* pMenu = G_LO_MENU( mpMenuModel );

    // Menu item is not updated unless it's necessary.
    gchar* aCurrentCommand = g_lo_menu_get_command_from_item_in_section( pMenu, nSection, nItemPos );

    if ( aCurrentCommand == nullptr || g_strcmp0( aCurrentCommand, aCommand ) != 0 )
    {
        bool bOldHasSubmenu = g_lo_menu_get_submenu_from_item_in_section(pMenu, nSection, nItemPos) != nullptr;
        bSubMenuAddedOrRemoved = bOldHasSubmenu != bIsSubmenu;
        if (bSubMenuAddedOrRemoved)
        {
            //tdf#98636 it's not good enough to unset the "submenu-action" attribute to change something
            //from a submenu to a non-submenu item, so remove the old one entirely and re-add it to
            //support achieving that
            gchar* pLabel = g_lo_menu_get_label_from_item_in_section(pMenu, nSection, nItemPos);
            g_lo_menu_remove_from_section(pMenu, nSection, nItemPos);
            g_lo_menu_insert_in_section(pMenu, nSection, nItemPos, pLabel);
            g_free(pLabel);
        }

        g_lo_menu_set_command_to_item_in_section( pMenu, nSection, nItemPos, aCommand );

        gchar* aItemCommand = g_strconcat("win.", aCommand, nullptr );

        if ( bIsSubmenu )
            g_lo_menu_set_submenu_action_to_item_in_section( pMenu, nSection, nItemPos, aItemCommand );
        else
        {
            g_lo_menu_set_action_and_target_value_to_item_in_section( pMenu, nSection, nItemPos, aItemCommand, pTarget );
            pTarget = nullptr;
        }

        g_free( aItemCommand );
    }

    if ( aCurrentCommand )
        g_free( aCurrentCommand );

    if (pTarget)
        g_variant_unref(pTarget);

    return bSubMenuAddedOrRemoved;
}

GtkSalMenu* GtkSalMenu::GetTopLevel()
{
    GtkSalMenu *pMenu = this;
    while (pMenu->mpParentSalMenu)
        pMenu = pMenu->mpParentSalMenu;
    return pMenu;
}

void GtkSalMenu::DispatchCommand(const gchar *pCommand)
{
    SolarMutexGuard aGuard;
    MenuAndId aMenuAndId = decode_command(pCommand);
    GtkSalMenu* pSalSubMenu = aMenuAndId.first;
    GtkSalMenu* pTopLevel = pSalSubMenu->GetTopLevel();
    pTopLevel->GetMenu()->HandleMenuCommandEvent(pSalSubMenu->GetMenu(), aMenuAndId.second);
}

void GtkSalMenu::ActivateAllSubmenus(Menu* pMenuBar)
{
    for (GtkSalMenuItem* pSalItem : maItems)
    {
        if ( pSalItem->mpSubMenu != nullptr )
        {
            pSalItem->mpSubMenu->mbInActivateCallback = true;
            pMenuBar->HandleMenuActivateEvent(pSalItem->mpSubMenu->GetMenu());
            pSalItem->mpSubMenu->mbInActivateCallback = false;
            pSalItem->mpSubMenu->ActivateAllSubmenus(pMenuBar);
            pSalItem->mpSubMenu->Update();
            pMenuBar->HandleMenuDeActivateEvent(pSalItem->mpSubMenu->GetMenu());
        }
    }
}

void GtkSalMenu::ClearActionGroupAndMenuModel()
{
    SetMenuModel(nullptr);
    mpActionGroup = nullptr;
    for (GtkSalMenuItem* pSalItem : maItems)
    {
        if ( pSalItem->mpSubMenu != nullptr )
        {
            pSalItem->mpSubMenu->ClearActionGroupAndMenuModel();
        }
    }
}

void GtkSalMenu::Activate(const gchar* pCommand)
{
    MenuAndId aMenuAndId = decode_command(pCommand);
    GtkSalMenu* pSalMenu = aMenuAndId.first;
    GtkSalMenu* pTopLevel = pSalMenu->GetTopLevel();
    Menu* pVclMenu = pSalMenu->GetMenu();
    Menu* pVclSubMenu = pVclMenu->GetPopupMenu(aMenuAndId.second);
    GtkSalMenu* pSubMenu = pSalMenu->GetItemAtPos(pVclMenu->GetItemPos(aMenuAndId.second))->mpSubMenu;

    pSubMenu->mbInActivateCallback = true;
    pTopLevel->GetMenu()->HandleMenuActivateEvent(pVclSubMenu);
    pSubMenu->mbInActivateCallback = false;
    pVclSubMenu->UpdateNativeMenu();
}

void GtkSalMenu::Deactivate(const gchar* pCommand)
{
    MenuAndId aMenuAndId = decode_command(pCommand);
    GtkSalMenu* pSalMenu = aMenuAndId.first;
    GtkSalMenu* pTopLevel = pSalMenu->GetTopLevel();
    Menu* pVclMenu = pSalMenu->GetMenu();
    Menu* pVclSubMenu = pVclMenu->GetPopupMenu(aMenuAndId.second);
    pTopLevel->GetMenu()->HandleMenuDeActivateEvent(pVclSubMenu);
}

void GtkSalMenu::EnableUnity(bool bEnable)
{
    bUnityMode = bEnable;

    MenuBar* pMenuBar(static_cast<MenuBar*>(mpVCLMenu.get()));
    bool bDisplayable(pMenuBar->IsDisplayable());

    if (bEnable)
    {
        DestroyMenuBarWidget();
        UpdateFull();
        if (!bDisplayable)
            ShowMenuBar(false);
    }
    else
    {
        Update();
        ShowMenuBar(bDisplayable);
    }

    pMenuBar->LayoutChanged();
}

void GtkSalMenu::ShowMenuBar( bool bVisible )
{
    // Unity tdf#106271: Can't hide global menu, so empty it instead when user wants to hide menubar,
    if (bUnityMode)
    {
        if (bVisible)
            Update();
        else if (mpMenuModel && g_menu_model_get_n_items(G_MENU_MODEL(mpMenuModel)) > 0)
            g_lo_menu_remove(G_LO_MENU(mpMenuModel), 0);
    }
    else if (bVisible)
        CreateMenuBarWidget();
    else
        DestroyMenuBarWidget();
}

bool GtkSalMenu::IsItemVisible( unsigned nPos )
{
    SolarMutexGuard aGuard;
    bool bVisible = false;

    if ( nPos < maItems.size() )
        bVisible = maItems[ nPos ]->mbVisible;

    return bVisible;
}

void GtkSalMenu::CheckItem( unsigned, bool )
{
}

void GtkSalMenu::EnableItem( unsigned nPos, bool bEnable )
{
    SolarMutexGuard aGuard;
    if ( bUnityMode && !mbInActivateCallback && !mbNeedsUpdate && GetTopLevel()->mbMenuBar && ( nPos < maItems.size() ) )
    {
        gchar* pCommand = GetCommandForItem( GetItemAtPos( nPos ) );
        NativeSetEnableItem( pCommand, bEnable );
        g_free( pCommand );
    }
}

void GtkSalMenu::ShowItem( unsigned nPos, bool bShow )
{
    SolarMutexGuard aGuard;
    if ( nPos < maItems.size() )
    {
        maItems[ nPos ]->mbVisible = bShow;
        if ( bUnityMode && !mbInActivateCallback && !mbNeedsUpdate && GetTopLevel()->mbMenuBar )
            Update();
    }
}

void GtkSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const OUString& rText )
{
    SolarMutexGuard aGuard;
    if ( bUnityMode && !mbInActivateCallback && !mbNeedsUpdate && GetTopLevel()->mbMenuBar && ( nPos < maItems.size() ) )
    {
        gchar* pCommand = GetCommandForItem( static_cast< GtkSalMenuItem* >( pSalMenuItem ) );

        gint nSectionsCount = g_menu_model_get_n_items( mpMenuModel );
        for ( gint nSection = 0; nSection < nSectionsCount; ++nSection )
        {
            gint nItemsCount = g_lo_menu_get_n_items_from_section( G_LO_MENU( mpMenuModel ), nSection );
            for ( gint nItem = 0; nItem < nItemsCount; ++nItem )
            {
                gchar* pCommandFromModel = g_lo_menu_get_command_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItem );

                if ( !g_strcmp0( pCommandFromModel, pCommand ) )
                {
                    NativeSetItemText( nSection, nItem, rText );
                    g_free( pCommandFromModel );
                    g_free( pCommand );
                    return;
                }

                g_free( pCommandFromModel );
            }
        }

        g_free( pCommand );
    }
}

void GtkSalMenu::SetItemImage( unsigned, SalMenuItem*, const Image& )
{
}

void GtkSalMenu::SetAccelerator( unsigned, SalMenuItem*, const vcl::KeyCode&, const OUString& )
{
}

void GtkSalMenu::GetSystemMenuData( SystemMenuData* )
{
}

int GtkSalMenu::GetMenuBarHeight() const
{
#if GTK_CHECK_VERSION(3,0,0)
    return mpMenuBarWidget ? gtk_widget_get_allocated_height(mpMenuBarWidget) : 0;
#else
    return 0;
#endif
}

/*
 * GtkSalMenuItem
 */

GtkSalMenuItem::GtkSalMenuItem( const SalItemParams* pItemData ) :
    mnId( pItemData->nId ),
    mnType( pItemData->eType ),
    mbVisible( true ),
    mpParentMenu( nullptr ),
    mpSubMenu( nullptr )
{
}

GtkSalMenuItem::~GtkSalMenuItem()
{
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
