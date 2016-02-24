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
#include <unx/gtk/gtkinst.hxx>

#if GTK_CHECK_VERSION(3,0,0)
#  include <gdk/gdkkeysyms-compat.h>
#endif

#include <sal/log.hxx>
#include <window.h>

// FIXME Copied from framework/inc/framework/menuconfiguration.hxx to
// avoid circular dependency between modules. It should be in a common
// header (probably in vcl).
const sal_uInt16 START_ITEMID_WINDOWLIST    = 4600;
const sal_uInt16 END_ITEMID_WINDOWLIST      = 4699;

/*
 * This function generates the proper command name for all actions, including
 * duplicated or special ones.
 */
static gchar* GetCommandForItem( GtkSalMenuItem* pSalMenuItem, gchar* aCurrentCommand, GActionGroup* pActionGroup )
{
    gchar* aCommand = nullptr;

    sal_uInt16 nId = pSalMenuItem->mnId;
    Menu* pMenu = pSalMenuItem->mpVCLMenu;

    // If item belongs to window list, generate a command with "window-(id)" format.
    if ( ( nId >= START_ITEMID_WINDOWLIST ) && ( nId <= END_ITEMID_WINDOWLIST ) )
        aCommand = g_strdup_printf( "window-%d", nId );
    else
    {
        if ( !pMenu )
            return nullptr;

        OUString aMenuCommand = pMenu->GetItemCommand(nId);
        if (aMenuCommand.isEmpty())
            aMenuCommand = "slot:" + OUString::number(nId);
        gchar* aCommandStr = g_strdup( OUStringToOString( aMenuCommand, RTL_TEXTENCODING_UTF8 ).getStr() );
        aCommand = g_strdup( aCommandStr );

        // Some items could have duplicated commands. A new one should be generated.
        for ( sal_uInt16 i = 2; ; i++ )
        {
            if ( !g_action_group_has_action( pActionGroup, aCommand )
                    || ( aCurrentCommand && g_strcmp0( aCurrentCommand, aCommand ) == 0 ) )
                break;

            g_free( aCommand );
            aCommand = g_strdup_printf("%s%d", aCommandStr, i);
        }

        g_free( aCommandStr );
    }

    return aCommand;
}

bool GtkSalMenu::PrepUpdate()
{
    bool bMenuVisibility;

    //get top level visibility
    const GtkSalMenu* pMenu = this;
    do
    {
        bMenuVisibility = pMenu->mbMenuVisibility;
        pMenu = pMenu->mpParentSalMenu;
    } while (pMenu);

    return bMenuVisibility && mpMenuModel && mpActionGroup;
}

/*
 * Menu updating methods
 */

void RemoveSpareItemsFromNativeMenu( GLOMenu* pMenu, GList** pOldCommandList, unsigned nSection, unsigned nValidItems )
{
    sal_Int32 nSectionItems = g_lo_menu_get_n_items_from_section( pMenu, nSection );

    while ( nSectionItems > (sal_Int32) nValidItems )
    {
        gchar* aCommand = g_lo_menu_get_command_from_item_in_section( pMenu, nSection, --nSectionItems );

        if ( aCommand != nullptr && pOldCommandList != nullptr )
            *pOldCommandList = g_list_append( *pOldCommandList, g_strdup( aCommand ) );

        g_free( aCommand );

        g_lo_menu_remove_from_section( pMenu, nSection, nSectionItems );
    }
}

void RemoveDisabledItemsFromNativeMenu(GLOMenu* pMenu, GList** pOldCommandList,
                                       sal_Int32 nSection, GActionGroup* pActionGroup)
{
    while (nSection >= 0)
    {
        sal_Int32 nSectionItems = g_lo_menu_get_n_items_from_section( pMenu, nSection );
        while (nSectionItems--)
        {
            gchar* pCommand = g_lo_menu_get_command_from_item_in_section(pMenu, nSection, nSectionItems);
            // remove disabled entries
            bool bRemove = g_action_group_get_action_enabled(pActionGroup, pCommand) == false;
            if (!bRemove)
            {
                //also remove any empty submenus
                GLOMenu* pSubMenuModel = g_lo_menu_get_submenu_from_item_in_section(pMenu, nSection, nSectionItems);
                if (pSubMenuModel)
                {
                    gint nSubMenuSections = g_menu_model_get_n_items(G_MENU_MODEL(pSubMenuModel));
                    bRemove = (nSubMenuSections == 0 ||
                              (nSubMenuSections == 1 && g_lo_menu_get_n_items_from_section(pSubMenuModel, 0) == 0));
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

void RemoveSpareSectionsFromNativeMenu( GLOMenu* pMenu, GList** pOldCommandList, sal_Int32 nLastSection )
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

gint CompareStr( gpointer str1, gpointer str2 )
{
    return g_strcmp0( static_cast<const gchar*>(str1), static_cast<const gchar*>(str2) );
}

void RemoveUnusedCommands( GLOActionGroup* pActionGroup, GList* pOldCommandList, GList* pNewCommandList )
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

    for ( nItem = 0; nItem < ( sal_Int32 ) GetItemCount(); nItem++ ) {
        if ( !IsItemVisible( nItem ) )
            continue;

        GtkSalMenuItem *pSalMenuItem = GetItemAtPos( nItem );
        sal_uInt16 nId = pSalMenuItem->mnId;

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
        bool bEnabled = pVCLMenu->IsItemEnabled( nId );
        vcl::KeyCode nAccelKey = pVCLMenu->GetAccelKey( nId );
        bool bChecked = pVCLMenu->IsItemChecked( nId );
        MenuItemBits itemBits = pVCLMenu->GetItemBits( nId );

        // Store current item command in command list.
        gchar *aCurrentCommand = g_lo_menu_get_command_from_item_in_section( pLOMenu, nSection, nItemPos );

        if ( aCurrentCommand != nullptr )
            pOldCommandList = g_list_append( pOldCommandList, aCurrentCommand );

        // Get the new command for the item.
        gchar* aNativeCommand = GetCommandForItem( pSalMenuItem, aCurrentCommand, mpActionGroup );

        // Force updating of native menu labels.
        NativeSetItemText( nSection, nItemPos, aText );
        NativeSetAccelerator( nSection, nItemPos, nAccelKey, nAccelKey.GetName( GetFrame()->GetWindow() ) );

        if ( g_strcmp0( aNativeCommand, "" ) != 0 && pSalMenuItem->mpSubMenu == nullptr )
        {
            NativeSetItemCommand( nSection, nItemPos, nId, aNativeCommand, itemBits, bChecked, FALSE );
            NativeCheckItem( nSection, nItemPos, itemBits, bChecked );
            NativeSetEnableItem( aNativeCommand, bEnabled );

            pNewCommandList = g_list_append( pNewCommandList, g_strdup( aNativeCommand ) );
        }

        GtkSalMenu* pSubmenu = pSalMenuItem->mpSubMenu;

        if ( pSubmenu && pSubmenu->GetMenu() )
        {
            NativeSetItemCommand( nSection, nItemPos, nId, aNativeCommand, itemBits, FALSE, TRUE );
            pNewCommandList = g_list_append( pNewCommandList, g_strdup( aNativeCommand ) );

            GLOMenu* pSubMenuModel = g_lo_menu_get_submenu_from_item_in_section( pLOMenu, nSection, nItemPos );

            if ( pSubMenuModel == nullptr )
            {
                g_lo_menu_new_submenu_in_item_in_section( pLOMenu, nSection, nItemPos );
                pSubMenuModel = g_lo_menu_get_submenu_from_item_in_section( pLOMenu, nSection, nItemPos );
            }

            g_object_unref( pSubMenuModel );

            if ( bRecurse )
            {
                SAL_INFO("vcl.unity", "preparing submenu  " << pSubMenuModel << " to menu model " << G_MENU_MODEL(pSubMenuModel) << " and action group " << G_ACTION_GROUP(pActionGroup));
                pSubmenu->SetMenuModel( G_MENU_MODEL( pSubMenuModel ) );
                pSubmenu->SetActionGroup( G_ACTION_GROUP( pActionGroup ) );
                pSubmenu->ImplUpdate(bRecurse, bRemoveDisabledEntries);
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
}

void GtkSalMenu::Update()
{
    //find out if top level is a menubar or not, if not, then its a popup menu
    //hierarchy and in those we hide (most) disabled entries
    const GtkSalMenu* pMenu = this;
    while (pMenu->mpParentSalMenu)
        pMenu = pMenu->mpParentSalMenu;
    ImplUpdate(false, !pMenu->mbMenuBar);
}

void GtkSalMenu::UpdateFull()
{
    //find out if top level is a menubar or not, if not, then its a popup menu
    //hierarchy and in those we hide (most) disabled entries
    const GtkSalMenu* pMenu = this;
    while (pMenu->mpParentSalMenu)
        pMenu = pMenu->mpParentSalMenu;
    ImplUpdate(true, !pMenu->mbMenuBar);
}

bool GtkSalMenu::ShowNativePopupMenu(FloatingWindow* pWin, const Rectangle& /*rRect*/,
                                     FloatWinPopupFlags /*nFlags*/)
{
#if GTK_CHECK_VERSION(3,0,0)
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
        nTime = gtk_get_current_event_time();
    }

    Display(true);

    mpFrame = static_cast<GtkSalFrame*>(pWin->ImplGetWindowImpl()->mpRealParent->ImplGetFrame());

    GLOActionGroup* pActionGroup = g_lo_action_group_new(static_cast<gpointer>(mpFrame));
    g_lo_action_group_set_top_menu(pActionGroup, static_cast<gpointer>(this));

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
    gtk_menu_popup(GTK_MENU(pWidget), nullptr, nullptr, nullptr, nullptr, nButton, nTime);
    if (g_main_loop_is_running(pLoop))
    {
        gdk_threads_leave();
        g_main_loop_run(pLoop);
        gdk_threads_enter();
    }
    g_main_loop_unref(pLoop);

    gtk_widget_insert_action_group(mpFrame->getMouseEventWidget(), "win", nullptr);

    gtk_widget_destroy(pWidget);

    g_object_unref(mpActionGroup);

    return true;
#else
    (void)pWin;
    return false;
#endif
}

/*
 * GtkSalMenu
 */

GtkSalMenu::GtkSalMenu( bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mbMenuVisibility( false ),
    mpVCLMenu( nullptr ),
    mpParentSalMenu( nullptr ),
    mpFrame( nullptr ),
    mpMenuModel( nullptr ),
    mpActionGroup( nullptr )
{
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

    if (mpMenuModel)
        g_object_unref(mpMenuModel);

    maItems.clear();
}

bool GtkSalMenu::VisibleMenuBar()
{
    return mbMenuBar && mbMenuVisibility;
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
}

void GtkSalMenu::RemoveItem( unsigned nPos )
{
    SolarMutexGuard aGuard;
    maItems.erase( maItems.begin() + nPos );
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
        g_lo_action_group_set_top_menu(pActionGroup, static_cast<gpointer>(this));
        mpActionGroup = G_ACTION_GROUP( pActionGroup );
    }

    // Generate the main menu structure.
    if (mbMenuVisibility)
        UpdateFull();

    g_lo_menu_insert_section( pMenuModel, 0, nullptr, mpMenuModel );
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

void GtkSalMenu::NativeSetEnableItem( gchar* aCommand, gboolean bEnable )
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

void GtkSalMenu::NativeSetItemCommand( unsigned nSection,
                                       unsigned nItemPos,
                                       sal_uInt16 nId,
                                       const gchar* aCommand,
                                       MenuItemBits nBits,
                                       gboolean bChecked,
                                       gboolean bIsSubmenu )
{
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
        g_lo_menu_set_command_to_item_in_section( pMenu, nSection, nItemPos, aCommand );

        gchar* aItemCommand = g_strconcat("win.", aCommand, nullptr );

        if ( bIsSubmenu )
            g_lo_menu_set_submenu_action_to_item_in_section( pMenu, nSection, nItemPos, aItemCommand );
        else
            g_lo_menu_set_action_and_target_value_to_item_in_section( pMenu, nSection, nItemPos, aItemCommand, pTarget );

        g_free( aItemCommand );
    }

    if ( aCurrentCommand )
        g_free( aCurrentCommand );
}

GtkSalMenu* GtkSalMenu::GetMenuForItemCommand( gchar* aCommand, gboolean bGetSubmenu )
{
    SolarMutexGuard aGuard;
    GtkSalMenu* pMenu = nullptr;
    for ( size_t nPos = 0; nPos < maItems.size(); nPos++ )
    {
        GtkSalMenuItem *pSalItem = maItems[ nPos ];

        OUString aItemCommand = mpVCLMenu->GetItemCommand( pSalItem->mnId );
        // Do not join the following two lines, or the OString will be destroyed
        // immediately, and the gchar* pointed to by aItemCommandStr will be
        // freed before it can be used - fdo#69090
        OString aItemCommandOStr = OUStringToOString( aItemCommand, RTL_TEXTENCODING_UTF8 );
        gchar* aItemCommandStr = const_cast<gchar*>(aItemCommandOStr.getStr());

        if ( g_strcmp0( aItemCommandStr, aCommand ) == 0 )
        {
            pMenu = bGetSubmenu ? pSalItem->mpSubMenu : this;
            break;
        }
        else
        {
            if ( pSalItem->mpSubMenu != nullptr )
                pMenu = pSalItem->mpSubMenu->GetMenuForItemCommand( aCommand, bGetSubmenu );

            if ( pMenu != nullptr )
               break;
        }
    }

    return pMenu;
}

void GtkSalMenu::DispatchCommand( gint itemId, const gchar *aCommand )
{
    SolarMutexGuard aGuard;
    GtkSalMenu* pSalSubMenu = GetMenuForItemCommand( const_cast<gchar*>(aCommand), FALSE );
    Menu* pSubMenu = ( pSalSubMenu != nullptr ) ? pSalSubMenu->GetMenu() : nullptr;
    mpVCLMenu->HandleMenuCommandEvent(pSubMenu, itemId);
}

void GtkSalMenu::ActivateAllSubmenus(Menu* pMenuBar)
{
    pMenuBar->HandleMenuActivateEvent(mpVCLMenu);
    pMenuBar->HandleMenuDeActivateEvent(mpVCLMenu);
    for ( size_t nPos = 0; nPos < maItems.size(); nPos++ )
    {
        GtkSalMenuItem *pSalItem = maItems[ nPos ];
        if ( pSalItem->mpSubMenu != nullptr )
        {
            pSalItem->mpSubMenu->ActivateAllSubmenus(pMenuBar);
            pSalItem->mpSubMenu->Update();
        }
    }
}

void GtkSalMenu::Activate( const gchar* aMenuCommand )
{
    if ( !aMenuCommand ) {
        ActivateAllSubmenus(mpVCLMenu);
        return;
    }

    GtkSalMenu* pSalSubMenu = GetMenuForItemCommand( const_cast<gchar*>(aMenuCommand), TRUE );

    if ( pSalSubMenu != nullptr ) {
        mpVCLMenu->HandleMenuActivateEvent( pSalSubMenu->mpVCLMenu );
        pSalSubMenu->Update();
    }
}

void GtkSalMenu::Deactivate( const gchar* aMenuCommand )
{
    GtkSalMenu* pSalSubMenu = GetMenuForItemCommand( const_cast<gchar*>(aMenuCommand), TRUE );

    if ( pSalSubMenu != nullptr ) {
        mpVCLMenu->HandleMenuDeActivateEvent( pSalSubMenu->mpVCLMenu );
    }
}

void GtkSalMenu::Display( bool bVisible )
{
    mbMenuVisibility = bVisible;

    if (mbMenuBar)
    {
        bool bVCLMenuVisible = !bVisible;
        MenuBar* pMenuBar = static_cast<MenuBar*>(mpVCLMenu);
        pMenuBar->SetDisplayable(bVCLMenuVisible);
    }
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

void GtkSalMenu::EnableItem( unsigned, bool )
{
}

void GtkSalMenu::ShowItem( unsigned nPos, bool bShow )
{
    SolarMutexGuard aGuard;
    if ( nPos < maItems.size() )
        maItems[ nPos ]->mbVisible = bShow;
}

void GtkSalMenu::SetItemText( unsigned, SalMenuItem*, const OUString& )
{
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

/*
 * GtkSalMenuItem
 */

GtkSalMenuItem::GtkSalMenuItem( const SalItemParams* pItemData ) :
    mnId( pItemData->nId ),
    mnType( pItemData->eType ),
    mbVisible( true ),
    mpVCLMenu( pItemData->pMenu ),
    mpParentMenu( nullptr ),
    mpSubMenu( nullptr )
{
}

GtkSalMenuItem::~GtkSalMenuItem()
{
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
