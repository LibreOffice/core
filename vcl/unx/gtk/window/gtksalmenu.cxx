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

#include "unx/gtk/gtksalmenu.hxx"

//#include <gtk/gtk.h>
#include <unx/gtk/glomenu.h>
#include <unx/gtk/gloactiongroup.h>
#include <vcl/menu.hxx>
#include <unx/gtk/gtkinst.hxx>

#include <iostream>

using namespace std;


static void UpdateNativeMenu( GtkSalMenu* pMenu )
{
    if ( pMenu == NULL )
        return;

    Menu* pVCLMenu = pMenu->GetMenu();
    GLOMenu* pLOMenu = G_LO_MENU( pMenu->GetMenuModel() );
    GActionGroup* pActionGroup = pMenu->GetActionGroup();

    sal_uInt16 nLOMenuSize = g_menu_model_get_n_items( G_MENU_MODEL( pLOMenu ) );

    if ( nLOMenuSize == 0 )
        g_lo_menu_new_section( pLOMenu, 0, NULL );

    sal_uInt16 nSection = 0;
    sal_uInt16 nItemPos = 0;
    sal_uInt16 validItems = 0;
    sal_uInt16 nItem;

    for ( nItem = 0; nItem < pMenu->GetItemCount(); nItem++ ) {
        GtkSalMenuItem *pSalMenuItem = pMenu->GetItemAtPos( nItem );
        sal_uInt16 nId = pSalMenuItem->mnId;

        if ( pSalMenuItem->mnType == MENUITEM_SEPARATOR )
        {
            nSection++;
            nItemPos = 0;

            if ( nLOMenuSize <= nSection )
            {
                g_lo_menu_new_section( pLOMenu, nSection, NULL );
                nLOMenuSize++;
            }

            continue;
        }

        if ( nItemPos >= g_lo_menu_get_n_items_from_section( pLOMenu, nSection ) )
            g_lo_menu_insert_in_section( pLOMenu, nSection, nItemPos, "EMPTY STRING" );

        // Get internal menu item values.
        String aText = pVCLMenu->GetItemText( nId );
        rtl::OUString aCommand( pVCLMenu->GetItemCommand( nId ) );
        sal_Bool itemEnabled = pVCLMenu->IsItemEnabled( nId );
        KeyCode nAccelKey = pVCLMenu->GetAccelKey( nId );
        sal_Bool itemChecked = pVCLMenu->IsItemChecked( nId );
        MenuItemBits itemBits = pVCLMenu->GetItemBits( nId );

        // Convert internal values to native values.
        gboolean bChecked = ( itemChecked == sal_True ) ? TRUE : FALSE;
        gboolean bEnabled = ( itemEnabled == sal_True ) ? TRUE : FALSE;
        gchar* aNativeCommand = g_strdup( rtl::OUStringToOString( aCommand, RTL_TEXTENCODING_UTF8 ).getStr() );

        // Force updating of native menu labels.
        pMenu->NativeSetItemText( nSection, nItemPos, aText );
        pMenu->NativeSetAccelerator( nSection, nItemPos, nAccelKey, nAccelKey.GetName( pMenu->GetFrame()->GetWindow() ) );

        if ( g_strcmp0( aNativeCommand, "" ) != 0 && pSalMenuItem->mpSubMenu == NULL )
        {
            pMenu->NativeSetItemCommand( nSection, nItemPos, pSalMenuItem, aNativeCommand );
            pMenu->NativeSetEnableItem( aNativeCommand, bEnabled );

            if ( ( itemBits & MIB_CHECKABLE ) || ( itemBits & MIB_RADIOCHECK ) )
                pMenu->NativeCheckItem( nSection, nItemPos, itemBits, bChecked );
        }

        g_free( aNativeCommand );

        GtkSalMenu* pSubmenu = pSalMenuItem->mpSubMenu;

        if ( pSubmenu && pSubmenu->GetMenu() )
        {
            GLOMenu* pSubMenuModel = g_lo_menu_get_submenu_from_item_in_section( pLOMenu, nSection, nItemPos );

            if ( pSubMenuModel == NULL )
            {
                pSubMenuModel = g_lo_menu_new();
                g_lo_menu_set_submenu_to_item_in_section( pLOMenu, nSection, nItemPos, G_MENU_MODEL( pSubMenuModel ) );
            }

            pSubmenu->GetMenu()->Activate();

            pSubmenu->SetMenuModel( G_MENU_MODEL( pSubMenuModel ) );
            pSubmenu->SetActionGroup( pActionGroup );
            UpdateNativeMenu( pSubmenu );

            pSubmenu->GetMenu()->Deactivate();
        }

        nItemPos++;
        validItems++;
    }
}

gboolean GenerateMenu(gpointer user_data)
{
    GtkSalMenu* pSalMenu = static_cast< GtkSalMenu* >( user_data );

    UpdateNativeMenu( pSalMenu );

    return TRUE;
}

void ObjectDestroyedNotify( gpointer data )
{
    if ( data ) {
        g_object_unref( data );
    }
}

void
gdk_x11_window_set_utf8_property  (GdkWindow *window,
                                   const gchar *name,
                                   const gchar *value)
{
  GdkDisplay *display;

  //if (!WINDOW_IS_TOPLEVEL (window))
    //return;

  display = gdk_window_get_display (window);

  if (value != NULL)
    {
      XChangeProperty (GDK_DISPLAY_XDISPLAY (display),
                       GDK_WINDOW_XID (window),
                       gdk_x11_get_xatom_by_name_for_display (display, name),
                       gdk_x11_get_xatom_by_name_for_display (display, "UTF8_STRING"), 8,
                       PropModeReplace, (guchar *)value, strlen (value));
    }
  else
    {
      XDeleteProperty (GDK_DISPLAY_XDISPLAY (display),
                       GDK_WINDOW_XID (window),
                       gdk_x11_get_xatom_by_name_for_display (display, name));
    }
}

// FIXME: Check for missing keys. Maybe translating keycodes would be safer...
rtl::OUString GetGtkKeyName( rtl::OUString keyName )
{
    rtl::OUString aGtkKeyName("");

    sal_Int32 nIndex = 0;

    do
    {
        rtl::OUString token = keyName.getToken( 0, '+', nIndex );

        if ( token == "Ctrl" ) {
            aGtkKeyName += "<Control>";
        } else if ( token == "Alt" ) {
            aGtkKeyName += "<Alt>";
        } else if ( token == "Shift" ) {
            aGtkKeyName += "<Shift>";
        } else {
            aGtkKeyName += token;
        }
    } while ( nIndex >= 0 );

    return aGtkKeyName;
}

// AppMenu watch functions.

static sal_Bool bDBusIsAvailable = sal_False;

static void
on_registrar_available (GDBusConnection * /*connection*/,
                        const gchar     * /*name*/,
                        const gchar     * /*name_owner*/,
                        gpointer         user_data)
{
    GtkSalFrame* pSalFrame = static_cast< GtkSalFrame* >( user_data );
    GtkSalMenu* pSalMenu = static_cast< GtkSalMenu* >( pSalFrame->GetMenu() );

    if ( pSalMenu != NULL )
    {
        MenuBar* pMenuBar = static_cast< MenuBar* >( pSalMenu->GetMenu() );

        GtkWidget *pWidget = pSalFrame->getWindow();
        GdkWindow *gdkWindow = gtk_widget_get_window( pWidget );

        if ( gdkWindow != NULL )
        {
            XLIB_Window windowId = GDK_WINDOW_XID( gdkWindow );

            gchar* aDBusPath = g_strdup_printf("/window/%lu", windowId);
            gchar* aDBusWindowPath = g_strdup_printf( "/window/%lu", windowId );
            gchar* aDBusMenubarPath = g_strdup_printf( "/window/%lu/menus/menubar", windowId );

            // Get a DBus session connection.
            GDBusConnection* pSessionBus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);

            if( pSessionBus == NULL )
                return;

            // Publish the menu.
            if ( aDBusMenubarPath != NULL )
                g_dbus_connection_export_menu_model (pSessionBus, aDBusMenubarPath, pSalMenu->GetMenuModel(), NULL);

            if ( aDBusPath != NULL )
                g_dbus_connection_export_action_group( pSessionBus, aDBusPath, pSalMenu->GetActionGroup(), NULL);

            // Set window properties.
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_UNIQUE_BUS_NAME", g_dbus_connection_get_unique_name( pSessionBus ) );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_APPLICATION_OBJECT_PATH", "" );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_WINDOW_OBJECT_PATH", aDBusWindowPath );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_MENUBAR_OBJECT_PATH", aDBusMenubarPath );

            g_free( aDBusPath );
            g_free( aDBusWindowPath );
            g_free( aDBusMenubarPath );

            bDBusIsAvailable = sal_True;
            pSalMenu->SetVisibleMenuBar( sal_True );
            pMenuBar->SetDisplayable( sal_False );
        }
    }

    return;
}

//This is called when the registrar becomes unavailable. It shows the menubar.
static void
on_registrar_unavailable (GDBusConnection * /*connection*/,
                          const gchar     * /*name*/,
                          gpointer         user_data)
{
    GtkSalFrame* pSalFrame = static_cast< GtkSalFrame* >( user_data );
    GtkSalMenu* pSalMenu = static_cast< GtkSalMenu* >( pSalFrame->GetMenu() );

    if ( pSalMenu ) {
        MenuBar* pMenuBar = static_cast< MenuBar* >( pSalMenu->GetMenu() );

        bDBusIsAvailable = sal_False;
        pSalMenu->SetVisibleMenuBar( sal_False );
        pMenuBar->SetDisplayable( sal_True );
    }

    return;
}

/*
 * GtkSalMenu
 */

GtkSalMenu::GtkSalMenu( sal_Bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mbVisible( sal_False ),
    mpVCLMenu( NULL ),
    mpParentSalMenu( NULL ),
    mpFrame( NULL ),
    mWatcherId( 0 ),
    mpMenuModel( NULL ),
    mpActionGroup( NULL )
{
}

GtkSalMenu::~GtkSalMenu()
{
    if ( mbMenuBar == sal_True ) {
        g_source_remove_by_user_data( this );

        ((GtkSalFrame*) mpFrame)->SetMenu( NULL );

        if ( mpActionGroup ) {
            g_lo_action_group_clear( G_LO_ACTION_GROUP( mpActionGroup ) );
        }

        if ( mpMenuModel ) {
            g_lo_menu_remove( G_LO_MENU( mpMenuModel ), 0 );
        }
    }

    maItems.clear();
}

void GtkSalMenu::SetVisibleMenuBar( sal_Bool bVisible )
{
//    mbVisible = bVisible;
}

sal_Bool GtkSalMenu::VisibleMenuBar()
{
//    return mbVisible;
    return bDBusIsAvailable;
}

void GtkSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    GtkSalMenuItem *pItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );

    if ( nPos == MENU_APPEND )
        maItems.push_back( pItem );
    else
        maItems.insert( maItems.begin() + nPos, pItem );

    pItem->mpParentMenu = this;
}

void GtkSalMenu::RemoveItem( unsigned nPos )
{
    maItems.erase( maItems.begin() + nPos );
}

void GtkSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
    GtkSalMenuItem *pItem = static_cast< GtkSalMenuItem* >( pSalMenuItem );
    GtkSalMenu *pGtkSubMenu = static_cast< GtkSalMenu* >( pSubMenu );

    if ( pGtkSubMenu == NULL )
        return;

    pGtkSubMenu->mpParentSalMenu = this;
    pItem->mpSubMenu = pGtkSubMenu;
}

void GtkSalMenu::SetFrame( const SalFrame* pFrame )
{
    mpFrame = static_cast< const GtkSalFrame* >( pFrame );

    ( ( GtkSalFrame* ) mpFrame )->SetMenu( this );

    GtkWidget *widget = GTK_WIDGET( mpFrame->getWindow() );

    GdkWindow *gdkWindow = gtk_widget_get_window( widget );

    if (gdkWindow) {
        mpMenuModel = G_MENU_MODEL( g_object_get_data( G_OBJECT( gdkWindow ), "g-lo-menubar" ) );
        mpActionGroup = G_ACTION_GROUP( g_object_get_data( G_OBJECT( gdkWindow ), "g-lo-action-group" ) );

        if ( mpMenuModel == NULL && mpActionGroup == NULL ) {
            mpMenuModel = G_MENU_MODEL( g_lo_menu_new() );
            mpActionGroup = G_ACTION_GROUP( g_lo_action_group_new( ( gpointer ) mpFrame ) );

            g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-menubar", mpMenuModel, ObjectDestroyedNotify );
            g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-action-group", mpActionGroup, ObjectDestroyedNotify );

            // Publish the menu only if AppMenu registrar is available.
            guint nWatcherId = g_bus_watch_name (G_BUS_TYPE_SESSION,
                                                 "com.canonical.AppMenu.Registrar",
                                                 G_BUS_NAME_WATCHER_FLAGS_NONE,
                                                 on_registrar_available,
                                                 on_registrar_unavailable,
                                                 (gpointer) mpFrame,
                                                 NULL);

            ( ( GtkSalFrame* ) mpFrame )->SetWatcherId( nWatcherId );
        }

        // Generate the main menu structure.
//        GenerateMenu( this );

        // Refresh the menu every second.
        // This code is a workaround until required modifications in Gtk+ are available.
        g_timeout_add_seconds( 1, GenerateMenu, this );
    }
}

const GtkSalFrame* GtkSalMenu::GetFrame() const
{
    const GtkSalMenu* pMenu = this;
    while( pMenu && ! pMenu->mpFrame )
        pMenu = pMenu->mpParentSalMenu;
    return pMenu ? pMenu->mpFrame : NULL;
}

void GtkSalMenu::CheckItem( unsigned nPos, sal_Bool bCheck )
{
}

void GtkSalMenu::NativeCheckItem( unsigned nSection, unsigned nItemPos, MenuItemBits bits, gboolean bCheck )
{
    if ( mpActionGroup == NULL )
        return;

    gchar* aCommand = g_lo_menu_get_command_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos );

    if ( aCommand != NULL || g_strcmp0( aCommand, "" ) != 0 )
    {
        GVariant *pCheckValue = NULL;
        GVariant *pCurrentState = g_action_group_get_action_state( mpActionGroup, aCommand );

        if ( bits & MIB_CHECKABLE )
        {
            pCheckValue = g_variant_new_boolean( bCheck );
        }
        else if ( bits & MIB_RADIOCHECK )
        {
            pCheckValue = ( bCheck == TRUE ) ? g_variant_new_string( aCommand ) : g_variant_new_string( "" );
        }

        if ( pCurrentState == NULL || g_variant_equal( pCurrentState, pCheckValue) == FALSE )
            g_action_group_change_action_state( mpActionGroup, aCommand, pCheckValue );
    }

    if ( aCommand )
        g_free( aCommand );
}

void GtkSalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
{
}

void GtkSalMenu::NativeSetEnableItem( gchar* aCommand, gboolean bEnable )
{
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( mpActionGroup );

    if ( g_action_group_get_action_enabled( G_ACTION_GROUP( pActionGroup ), aCommand ) != bEnable )
        g_lo_action_group_set_action_enabled( pActionGroup, aCommand, bEnable );
}

void GtkSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& rText )
{
}

void GtkSalMenu::NativeSetItemText( unsigned nSection, unsigned nItemPos, const rtl::OUString& rText )
{
    // Replace the "~" character with "_".
    rtl::OUString aText = rText.replace( '~', '_' );
    rtl::OString aConvertedText = OUStringToOString( aText, RTL_TEXTENCODING_UTF8 );

    // Update item text only when necessary.
    gchar* aLabel = g_lo_menu_get_label_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos );

    if ( aLabel == NULL || g_strcmp0( aLabel, aConvertedText.getStr() ) != 0 )
        g_lo_menu_set_label_to_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos, aConvertedText.getStr() );

    if ( aLabel )
        g_free( aLabel );
}

void GtkSalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage)
{
}

void GtkSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const rtl::OUString& rKeyName )
{
}

void GtkSalMenu::NativeSetAccelerator( unsigned nSection, unsigned nItemPos, const KeyCode& rKeyCode, const rtl::OUString& rKeyName )
{
    if ( rKeyName.isEmpty() )
        return;

    rtl::OString aAccelerator = rtl::OUStringToOString( GetGtkKeyName( rKeyName ), RTL_TEXTENCODING_UTF8 );

    gchar* aCurrentAccel = g_lo_menu_get_accelerator_from_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos );

    if ( aCurrentAccel == NULL && g_strcmp0( aCurrentAccel, aAccelerator.getStr() ) != 0 )
        g_lo_menu_set_accelerator_to_item_in_section ( G_LO_MENU( mpMenuModel ), nSection, nItemPos, aAccelerator.getStr() );

    if ( aCurrentAccel )
        g_free( aCurrentAccel );
}

void GtkSalMenu::SetItemCommand( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& aCommandStr )
{
}

void GtkSalMenu::NativeSetItemCommand( unsigned nSection, unsigned nItemPos, GtkSalMenuItem* pItem, const gchar* aCommand )
{
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( mpActionGroup );

    GVariant *pTarget = NULL;

    if ( g_action_group_has_action( mpActionGroup, aCommand ) == FALSE ) {
        gboolean bChecked = ( pItem->mpVCLMenu->IsItemChecked( pItem->mnId ) ) ? TRUE : FALSE;

        // FIXME: Why pItem->mnBits differs from GetItemBits value?
        MenuItemBits bits = pItem->mpVCLMenu->GetItemBits( pItem->mnId );

        if ( bits & MIB_CHECKABLE )
        {
            // Item is a checkmark button.
            GVariantType* pStateType = g_variant_type_new( (gchar*) G_VARIANT_TYPE_BOOLEAN );
            GVariant* pState = g_variant_new_boolean( bChecked );

            g_lo_action_group_insert_stateful( pActionGroup, aCommand, pItem->mnId, NULL, pStateType, NULL, pState );
        }
        else if ( bits & MIB_RADIOCHECK )
        {
            // Item is a radio button.
            GVariantType* pParameterType = g_variant_type_new( (gchar*) G_VARIANT_TYPE_STRING );
            GVariantType* pStateType = g_variant_type_new( (gchar*) G_VARIANT_TYPE_STRING );
            GVariant* pState = g_variant_new_string( "" );
            pTarget = g_variant_new_string( aCommand );

            g_lo_action_group_insert_stateful( pActionGroup, aCommand, pItem->mnId, pParameterType, pStateType, NULL, pState );
        }
        else
        {
            // Item is not special, so insert a stateless action.
            g_lo_action_group_insert( pActionGroup, aCommand, pItem->mnId );
        }
    }

    GLOMenu* pMenu = G_LO_MENU( mpMenuModel );

    // Menu item is not updated unless it's necessary.
    gchar* aCurrentCommand = g_lo_menu_get_command_from_item_in_section( pMenu, nSection, nItemPos );

    if ( aCurrentCommand == NULL || g_strcmp0( aCurrentCommand, aCommand ) != 0 )
    {
        g_lo_menu_set_command_to_item_in_section( pMenu, nSection, nItemPos, aCommand );

        gchar* aItemCommand = g_strconcat("win.", aCommand, NULL );

        g_lo_menu_set_action_and_target_value_to_item_in_section( pMenu, nSection, nItemPos, aItemCommand, pTarget );

        g_free( aItemCommand );
    }

    if ( aCurrentCommand )
        g_free( aCurrentCommand );
}

void GtkSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
}

GtkSalMenu* GtkSalMenu::GetMenuForItemCommand( gchar* aCommand )
{
    GtkSalMenu* pMenu = NULL;

    for ( sal_uInt16 nPos = 0; nPos < maItems.size(); nPos++ )
    {
        GtkSalMenuItem *pSalItem = maItems[ nPos ];

        String aItemCommand = mpVCLMenu->GetItemCommand( pSalItem->mnId );

        gchar* aItemCommandStr = (gchar*) rtl::OUStringToOString( aItemCommand, RTL_TEXTENCODING_UTF8 ).getStr();

        if ( g_strcmp0( aItemCommandStr, aCommand ) == 0 )
        {
            pMenu = this;
            break;
        }
        else
        {
            if ( pSalItem->mpSubMenu != NULL )
                pMenu = pSalItem->mpSubMenu->GetMenuForItemCommand( aCommand );

            if ( pMenu != NULL )
               break;
        }
    }

    return pMenu;
}

void GtkSalMenu::Freeze()
{
}

// =======================================================================

/*
 * GtkSalMenuItem
 */

GtkSalMenuItem::GtkSalMenuItem( const SalItemParams* pItemData ) :
    mnId( pItemData->nId ),
    mnBits( pItemData->nBits ),
    mnType( pItemData->eType ),
    mpVCLMenu( pItemData->pMenu ),
    mpParentMenu( NULL ),
    mpSubMenu( NULL )
{
}

GtkSalMenuItem::~GtkSalMenuItem()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
