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

//Some menus are special, this is the list of them
//gboolean
//isSpecialSubmenu (OUString command)
//{
//    const gchar * specialSubmenus[11] = {".uno:CharFontName",
//                                         ".uno:FontHeight",
//                                         ".uno:ObjectMenue",
//                                         ".uno:InsertPageHeader",
//                                         ".uno:InsertPageFooter",
//                                         ".uno:ChangeControlType",
//                                         ".uno:AvailableToolbars",
//                                         ".uno:ScriptOrganizer",
//                                         ".uno:RecentFileList",
//                                         ".uno:AddDirect",
//                                         ".uno:AutoPilotMenu"};

//    for (gint i = 0; i < 11; i++)
//    {
//        if (command.equals (OUString::createFromAscii (specialSubmenus[i])))
//            return TRUE;
//    }
//    return FALSE;
//}

static void UpdateNativeMenu( GtkSalMenu* pMenu ) {
    if ( pMenu == NULL )
        return;

    Menu* pVCLMenu = pMenu->GetMenu();

    for ( sal_uInt16 i = 0; i < pMenu->GetItemCount(); i++ ) {
        GtkSalMenuItem *pSalMenuItem = pMenu->GetItemAtPos( i );
        sal_uInt16 nId = pSalMenuItem->mnId;

        if ( pSalMenuItem->mnType == MENUITEM_SEPARATOR )
            continue;

        String aText = pVCLMenu->GetItemText( nId );
        String aCommand = pVCLMenu->GetItemCommand( nId );
        sal_Bool itemEnabled = pVCLMenu->IsItemEnabled( nId );
        KeyCode nAccelKey = pVCLMenu->GetAccelKey( nId );
        sal_Bool itemChecked = pVCLMenu->IsItemChecked( nId );

        // Force updating of native menu labels.
        pMenu->SetItemCommand( i, pSalMenuItem, aCommand );
        pMenu->SetItemText( i, pSalMenuItem, aText );
        pMenu->EnableItem( i, itemEnabled );
        pMenu->SetAccelerator( i, pSalMenuItem, nAccelKey, nAccelKey.GetName( pMenu->GetFrame()->GetWindow() ) );
        pMenu->CheckItem( i, itemChecked );

        GtkSalMenu* pSubmenu = pSalMenuItem->mpSubMenu;

        if ( pSubmenu && pSubmenu->GetMenu() ) {
            pSubmenu->GetMenu()->Activate();
            pSubmenu->GetMenu()->Deactivate();
            UpdateNativeMenu( pSubmenu );
        }
    }
}

gboolean GenerateMenu(gpointer user_data) {
    GtkSalMenu* pSalMenu = static_cast< GtkSalMenu* >( user_data );

    UpdateNativeMenu( pSalMenu );

    return TRUE;
}

void ObjectDestroyedNotify( gpointer data ) {
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

GActionGroup* GetActionGroupFromMenubar( GtkSalMenu *pMenu )
{
    GtkSalMenu *pSalMenu = pMenu;

    while ( pSalMenu && pSalMenu->GetParentSalMenu() )
        pSalMenu = pSalMenu->GetParentSalMenu();

    return ( pSalMenu ) ? pSalMenu->GetActionGroup() : NULL;
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

//GVariant* GetRadioButtonHints( GtkSalMenuItem *pSalMenuItem )
//{
//    GVariantBuilder *pBuilder;
//    GVariant *pHints;

//    pBuilder = g_variant_builder_new( G_VARIANT_TYPE_STRING );

//    Menu* pMenu = pSalMenuItem->mpVCLMenu;

//    gboolean bItemIncluded = FALSE;

//    for ( sal_uInt16 i = 0; i < pMenu->GetItemCount(); i++ )
//    {
//        sal_uInt16 nId = pMenu->GetItemId( i );
//        MenuItemBits itemBits = pMenu->GetItemBits( nId );
//        MenuItemType itemType = pMenu->GetItemType( nId );

//        if ( itemBits & MIB_RADIOCHECK )
//        {
//            rtl::OString aValue = rtl::OUStringToOString( pMenu->GetItemText( nId ), RTL_TEXTENCODING_UTF8 );
//            g_variant_builder_add( pBuilder, "s", aValue.getStr() );

//            if ( nId == pSalMenuItem->mnId )
//                bItemIncluded = TRUE;
//        }
//        else if ( itemType == MENUITEM_SEPARATOR )
//        {
//            if ( bItemIncluded == FALSE )
//            {
//                g_variant_builder_clear( pBuilder );
//            }
//            else
//                break;
//        }
//    }

//    // Build an array of G_VARIANT_TYPE_STRING.
//    pHints = g_variant_new ("as", pBuilder);
//    g_variant_builder_unref ( pBuilder );

//    return pHints;
//}

/*
 * GtkSalMenu
 */

// FIXME: Iterating through the whole list everytime is slow, but works. Some fine tuning would be required here...
void GtkSalMenu::GetItemSectionAndPosition( unsigned nPos, unsigned *insertSection, unsigned *insertPos )
{
    if ( mpVCLMenu == NULL || nPos >= mpVCLMenu->GetItemCount() )
    {
        *insertSection = g_menu_model_get_n_items( mpMenuModel ) - 1;
        *insertPos = MENU_APPEND;
    }
    else
    {
        unsigned nItem;
        gint nItemPos;

        for ( nItem = 0, *insertSection = 0, nItemPos = -1; nItem <= nPos; nItem++ )
        {
            if ( mpVCLMenu->GetItemType( nItem ) == MENUITEM_SEPARATOR ) {
                (*insertSection)++;
                nItemPos = -1;
            } else
                nItemPos++;
        }

        *insertPos = nItemPos;
    }
}

GtkSalMenu::GtkSalMenu( sal_Bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mpVCLMenu( NULL ),
    mpParentSalMenu( NULL ),
    mpFrame( NULL ),
    mpMenuModel( NULL ),
    mpActionGroup( NULL )
{
    mpMenuModel = G_MENU_MODEL( g_lo_menu_new() );
    g_lo_menu_new_section( G_LO_MENU( mpMenuModel ), 0, NULL);
}

GtkSalMenu::~GtkSalMenu()
{
    if ( mpActionGroup ) {
        g_lo_action_group_clear( G_LO_ACTION_GROUP( mpActionGroup ) );
    }

    if ( mbMenuBar ) {
        g_source_remove_by_user_data( this );
        g_lo_menu_remove( G_LO_MENU( mpMenuModel ), 0 );
    } else {
        g_object_unref( mpMenuModel );
    }

    maItems.clear();
}

sal_Bool GtkSalMenu::VisibleMenuBar()
{
    return sal_True;
}

void GtkSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    GtkSalMenuItem *pItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );

    if ( nPos == MENU_APPEND )
        maItems.push_back( pItem );
    else
        maItems.insert( maItems.begin() + nPos, pItem );

    pItem->mpParentMenu = this;

    if ( pItem->mnType != MENUITEM_SEPARATOR )
    {
        unsigned nInsertSection, nInsertPos;
        GetItemSectionAndPosition( nPos, &nInsertSection, &nInsertPos );

        g_lo_menu_insert_in_section( G_LO_MENU( mpMenuModel ), nInsertSection, nInsertPos, "EMPTY STRING" );
    }
    else
    {
        g_lo_menu_new_section( G_LO_MENU( mpMenuModel ), MENU_APPEND, NULL );
    }
}

void GtkSalMenu::RemoveItem( unsigned nPos )
{
    GLOMenu* pMenu = G_LO_MENU( mpMenuModel );
    GtkSalMenuItem *pItem = maItems[ nPos ];

    // If item is a separator, the last section of the menu is removed.
    if ( pItem->mnType != MENUITEM_SEPARATOR )
    {
        if ( pItem->maCommand ) {
            GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( GetActionGroupFromMenubar( this ) );

            if ( pActionGroup != NULL )
                g_lo_action_group_remove( pActionGroup, pItem->maCommand );

            g_free ( pItem->maCommand );
            pItem->maCommand = NULL;
        }

        unsigned nSection, nItemPos;
        GetItemSectionAndPosition( nPos, &nSection, &nItemPos );

        g_lo_menu_remove_from_section( pMenu, nSection, nItemPos );
    }
    else
    {
        gint nSection = g_menu_model_get_n_items( mpMenuModel ) - 1;

        if ( nSection > 0 )
            g_lo_menu_remove( pMenu, nSection );
    }

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

    // Update item in GMenuModel.
    unsigned nSection, nItemPos;
    GetItemSectionAndPosition( nPos, &nSection, &nItemPos );

    g_lo_menu_set_submenu_to_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos, pGtkSubMenu->mpMenuModel );
}

void GtkSalMenu::SetFrame( const SalFrame* pFrame )
{
    mpFrame = static_cast<const GtkSalFrame*>( pFrame );

    GtkWidget *widget = GTK_WIDGET( mpFrame->getWindow() );

    GdkWindow *gdkWindow = gtk_widget_get_window( widget );

    if (gdkWindow) {
        GLOMenu* pMainMenu = G_LO_MENU( g_object_get_data( G_OBJECT( gdkWindow ), "g-lo-menubar" ) );
        GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( g_object_get_data( G_OBJECT( gdkWindow ), "g-lo-action-group" ) );

        if ( pMainMenu && pActionGroup ) {
            g_lo_menu_remove( pMainMenu, 0 );
        } else {
            pMainMenu = g_lo_menu_new();
            pActionGroup = g_lo_action_group_new();

            g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-menubar", pMainMenu, ObjectDestroyedNotify );
            g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-action-group", pActionGroup, ObjectDestroyedNotify );

            XLIB_Window windowId = GDK_WINDOW_XID( gdkWindow );

            gchar* aDBusPath = g_strdup_printf("/window/%lu", windowId);
            gchar* aDBusWindowPath = g_strdup_printf( "/window/%lu", windowId );
            gchar* aDBusMenubarPath = g_strdup_printf( "/window/%lu/menus/menubar", windowId );

            // Get a DBus session connection.
            GDBusConnection* pSessionBus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
            if(!pSessionBus) puts ("Failed to get DBus session connection");

            // Publish the menu.
            if ( aDBusMenubarPath ) {
                sal_uInt16 menubarId = g_dbus_connection_export_menu_model (pSessionBus, aDBusMenubarPath, G_MENU_MODEL( pMainMenu ), NULL);
                if(!menubarId) puts("Failed to export menubar");
            }

            if ( aDBusPath ) {
                sal_uInt16 actionGroupId = g_dbus_connection_export_action_group( pSessionBus, aDBusPath, G_ACTION_GROUP( pActionGroup ), NULL);
                if(!actionGroupId) puts("Failed to export action group");
            }

            // Set window properties.
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_UNIQUE_BUS_NAME", g_dbus_connection_get_unique_name( pSessionBus ) );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_APPLICATION_OBJECT_PATH", "" );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_WINDOW_OBJECT_PATH", aDBusWindowPath );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_MENUBAR_OBJECT_PATH", aDBusMenubarPath );

            g_free( aDBusPath );
            g_free( aDBusWindowPath );
            g_free( aDBusMenubarPath );
        }

        // Menubar has only one section, so we put it on the exported menu.
        g_lo_menu_insert_section( pMainMenu, 0, NULL, mpMenuModel );
        mpActionGroup = G_ACTION_GROUP( pActionGroup );

//        UpdateNativeMenu( this );
//        UpdateNativeMenu( this );

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
    GtkSalMenuItem* pItem = maItems[ nPos ];

    if ( pItem->mpSubMenu )
        return;

    if ( pItem->maCommand == NULL || g_strcmp0( pItem->maCommand, "" ) == 0 )
        return;

    GActionGroup* pActionGroup = GetActionGroupFromMenubar( this );

    if ( !pActionGroup )
        return;

    GVariant *pCheckValue = NULL;
    gboolean bCheckedValue = ( bCheck == sal_True ) ? TRUE : FALSE;

    // FIXME: Why pItem->mnBits differs from GetItemBits value?
    MenuItemBits bits = pItem->mpVCLMenu->GetItemBits( pItem->mnId );

    if ( bits & MIB_CHECKABLE ) {
        GVariant* pState = g_action_group_get_action_state( pActionGroup, pItem->maCommand );
        gboolean bCurrentState = g_variant_get_boolean( pState );

        if ( bCurrentState != bCheck )
            pCheckValue = g_variant_new_boolean( bCheckedValue );
    }
    else if ( bits & MIB_RADIOCHECK )
    {
        GVariant* pState = g_action_group_get_action_state( pActionGroup, pItem->maCommand );
        gchar* aCurrentState = (gchar*) g_variant_get_string( pState, NULL );
        gboolean bCurrentState = g_strcmp0( aCurrentState, "" ) != 0;

        if ( bCurrentState != bCheck )
            pCheckValue = (bCheckedValue) ? g_variant_new_string( pItem->maCommand ) : g_variant_new_string( "" );
    }

    if ( pCheckValue )
        g_action_group_change_action_state( pActionGroup, pItem->maCommand, pCheckValue );
}

void GtkSalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
{
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( GetActionGroupFromMenubar( this ) );

    if ( pActionGroup == NULL )
        return;

    sal_uInt16 nId = mpVCLMenu->GetItemId( nPos );
    rtl::OUString aOUCommand = mpVCLMenu->GetItemCommand( nId );

    if ( aOUCommand == NULL || aOUCommand.isEmpty() )
        return;

    gchar* aCommand = (gchar*) rtl::OUStringToOString( aOUCommand, RTL_TEXTENCODING_UTF8 ).getStr();

    gboolean bItemEnabled = (bEnable == sal_True) ? TRUE : FALSE;

    if ( g_action_group_get_action_enabled( G_ACTION_GROUP( pActionGroup ), aCommand ) != bItemEnabled )
        g_lo_action_group_set_action_enabled( pActionGroup, aCommand, bItemEnabled );
}

void GtkSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& rText )
{
    // Replace the "~" character with "_".
    rtl::OUString aText = rText.replace( '~', '_' );
    rtl::OString aConvertedText = OUStringToOString( aText, RTL_TEXTENCODING_UTF8 );

    unsigned nSection, nItemPos;
    GetItemSectionAndPosition( nPos, &nSection, &nItemPos );

    // Update item text only when necessary.
    sal_Bool bSetLabel = sal_True;

    GtkSalMenuItem* pItem = static_cast< GtkSalMenuItem* >( pSalMenuItem );

    // FIXME: It would be better retrieving the label from the menu itself, but it currently crashes the app.
    if ( pItem->maLabel && g_strcmp0( pItem->maLabel, aConvertedText.getStr() ) == 0 )
        bSetLabel = sal_False;

    if ( bSetLabel == sal_True ) {
        if ( pItem->maLabel )
            g_free( pItem->maLabel );

        pItem->maLabel = g_strdup( aConvertedText.getStr() );
        g_lo_menu_set_label_to_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos, pItem->maLabel );
    }
}

void GtkSalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage)
{
}

void GtkSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const rtl::OUString& rKeyName )
{
    GtkSalMenuItem *pItem = static_cast< GtkSalMenuItem* >( pSalMenuItem );

    if ( rKeyName.isEmpty() )
        return;

    rtl::OString aAccelerator = rtl::OUStringToOString( GetGtkKeyName( rKeyName ), RTL_TEXTENCODING_UTF8 );

    unsigned nSection, nItemPos;
    GetItemSectionAndPosition( nPos, &nSection, &nItemPos );

    sal_Bool bSetAccel = sal_True;

    if ( pItem->maAccel && g_strcmp0( pItem->maAccel, aAccelerator.getStr() ) == 0 )
            bSetAccel = sal_False;

    if ( bSetAccel == sal_True ) {
        if (pItem->maAccel)
            g_free( pItem->maAccel );

        pItem->maAccel = g_strdup( aAccelerator.getStr() );
        g_lo_menu_set_accelerator_to_item_in_section ( G_LO_MENU( mpMenuModel ), nSection, nItemPos, pItem->maAccel );
    }
}

void GtkSalMenu::SetItemCommand( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& aCommandStr )
{
    GtkSalMenuItem* pItem = static_cast< GtkSalMenuItem* >( pSalMenuItem );

    if ( pItem->mnType == MENUITEM_SEPARATOR ||
         pItem->mpVCLMenu->GetPopupMenu( pItem->mnId ) != NULL ||
         aCommandStr.isEmpty() )
        return;

    gchar* aCommand = (gchar*) rtl::OUStringToOString( aCommandStr, RTL_TEXTENCODING_UTF8 ).getStr();

    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( GetActionGroupFromMenubar( this ) );
    if ( pActionGroup == NULL )
        return;

    GVariant *pTarget = NULL;

    if ( g_action_group_has_action( G_ACTION_GROUP( pActionGroup ), aCommand ) == FALSE ) {
        gboolean bChecked = ( pItem->mpVCLMenu->IsItemChecked( pItem->mnId ) ) ? TRUE : FALSE;

        // FIXME: Why pItem->mnBits differs from GetItemBits value?
        MenuItemBits bits = pItem->mpVCLMenu->GetItemBits( pItem->mnId );

        if ( bits & MIB_CHECKABLE )
        {
            // Item is a checkmark button.
            GVariantType* pStateType = g_variant_type_new( (gchar*) G_VARIANT_TYPE_BOOLEAN );
            GVariant* pState = g_variant_new_boolean( bChecked );

            g_lo_action_group_insert_stateful( pActionGroup, aCommand, pItem, NULL, pStateType, NULL, pState );
        }
        else if ( bits & MIB_RADIOCHECK )
        {
            // Item is a radio button.
            GVariantType* pParameterType = g_variant_type_new( (gchar*) G_VARIANT_TYPE_STRING );
            GVariantType* pStateType = g_variant_type_new( (gchar*) G_VARIANT_TYPE_STRING );
            //        GVariant* pStateHint = GetRadioButtonHints( pItem );
            GVariant* pState = g_variant_new_string( "" );
            pTarget = g_variant_new_string( aCommand );

            g_lo_action_group_insert_stateful( pActionGroup, aCommand, pItem, pParameterType, pStateType, NULL, pState );
        }
        else
        {
            // Item is not special, so insert a stateless action.
            g_lo_action_group_insert( pActionGroup, aCommand, pItem );
        }
    }

    // Menu item is not updated unless it's necessary.
    if ( ( pItem->maCommand != NULL ) && ( g_strcmp0( pItem->maCommand, aCommand ) == 0 ) )
        return;

    if ( pItem->maCommand != NULL )
        g_free( pItem->maCommand );

    pItem->maCommand = g_strdup( aCommand );

    unsigned nSection, nItemPos;
    GetItemSectionAndPosition( nPos, &nSection, &nItemPos );

    gchar* aItemCommand = g_strconcat("win.", aCommand, NULL );

    if ( pItem->maCommand )
        g_free( pItem->maCommand );

    pItem->maCommand = g_strdup( aCommand );

    g_lo_menu_set_action_and_target_value_to_item_in_section( G_LO_MENU( mpMenuModel ), nSection, nItemPos, aItemCommand, pTarget );

    g_free( aItemCommand );
}

void GtkSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
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
    mpSubMenu( NULL ),
    maCommand( NULL ),
    maLabel( NULL ),
    maAccel( NULL )
{
}

GtkSalMenuItem::~GtkSalMenuItem()
{
    if ( maCommand )
        g_free( maCommand );

    if ( maLabel )
        g_free( maLabel );

    if ( maAccel )
        g_free( maAccel );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
