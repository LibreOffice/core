
#include "unx/gtk/gtksalmenu.hxx"

//#include <gtk/gtk.h>
#include <unx/gtk/glomenu.h>
#include <unx/gtk/gloactiongroup.h>
#include <vcl/menu.hxx>
#include <unx/gtk/gtkinst.hxx>

#include <iostream>

using namespace std;

//Some menus are special, this is the list of them
gboolean
isSpecialSubmenu (OUString command)
{
    const gchar * specialSubmenus[11] = {".uno:CharFontName",
                                         ".uno:FontHeight",
                                         ".uno:ObjectMenue",
                                         ".uno:InsertPageHeader",
                                         ".uno:InsertPageFooter",
                                         ".uno:ChangeControlType",
                                         ".uno:AvailableToolbars",
                                         ".uno:ScriptOrganizer",
                                         ".uno:RecentFileList",
                                         ".uno:AddDirect",
                                         ".uno:AutoPilotMenu"};

    for (gint i = 0; i < 11; i++)
    {
        if (command.equals (OUString::createFromAscii (specialSubmenus[i])))
            return TRUE;
    }
    return FALSE;
}

void updateNativeMenu( GtkSalMenu* pMenu ) {
    if ( pMenu ) {
        for ( sal_uInt16 i = 0; i < pMenu->GetItemCount(); i++ ) {
            GtkSalMenuItem* pSalMenuItem = pMenu->GetItemAtPos( i );

            if ( !pSalMenuItem->mpVCLMenu )
                continue;

            String aText = pSalMenuItem->mpVCLMenu->GetItemText( pSalMenuItem->mnId );

            // Force updating of native menu labels.
            pMenu->SetItemText( i, pSalMenuItem, aText );

            if ( pSalMenuItem->mpSubMenu && pSalMenuItem->mpSubMenu->GetMenu() ) {
                pSalMenuItem->mpSubMenu->GetMenu()->Activate();
                updateNativeMenu( pSalMenuItem->mpSubMenu );

                // FIXME: Using Deactivate() let the menu to update itself correctly, but generates
                // duplicated popup menus.
//                pSalMenuItem->mpSubMenu->GetMenu()->Deactivate();
            }
        }
    }
}

gboolean GenerateMenu(gpointer user_data) {
    GtkSalMenu* pSalMenu = static_cast< GtkSalMenu* >( user_data );

    updateNativeMenu( pSalMenu );

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

/*
 * GtkSalMenu
 */

GtkSalMenuItem* GtkSalMenu::GetSalMenuItem( sal_uInt16 nId )
{
    for ( sal_uInt16 i = 0; i < maItems.size(); i++ )
    {
        GtkSalMenuItem* pSalMenuItem = maItems[ i ];

        if ( pSalMenuItem->mnId == nId ) {
            return pSalMenuItem;
        }

        if ( pSalMenuItem->mpSubMenu )
        {
            pSalMenuItem = pSalMenuItem->mpSubMenu->GetSalMenuItem( nId );
            if (pSalMenuItem) {
                return pSalMenuItem;
            }
        }
    }

    return NULL;
}

sal_Int16 GtkSalMenu::GetSectionNumber( GMenuModel* pSection )
{
    if ( pSection == NULL )
        return -1;

    for ( int i = 0; maSections.size(); i++ )
    {
        if ( maSections[ i ] == pSection )
            return i;
    }

    return -1;
}

GtkSalMenu::GtkSalMenu( sal_Bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mpVCLMenu( NULL ),
    mpParentSalMenu( NULL ),
    mpFrame( NULL ),
    pSessionBus( NULL ),
    mMenubarId( 0 ),
    mActionGroupId ( 0 ),
    mpMenuModel( NULL ),
    mpActionGroup( NULL )
{
    mpCurrentSection = G_MENU_MODEL( g_lo_menu_new() );
    maSections.push_back( mpCurrentSection );

    if (bMenuBar) {
        mpActionGroup = G_ACTION_GROUP( g_lo_action_group_new() );

        pSessionBus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
        if(!pSessionBus) puts ("Fail bus get");
    } else {
        mpMenuModel = G_MENU_MODEL( g_lo_menu_new() );
        g_lo_menu_append_section( G_LO_MENU( mpMenuModel ), NULL, mpCurrentSection );
    }
}

GtkSalMenu::~GtkSalMenu()
{
    g_source_remove_by_user_data( this );

    g_object_unref( mpCurrentSection );

    if ( mbMenuBar ) {
        g_lo_menu_remove( G_LO_MENU( mpMenuModel ), 0 );
        mpMenuModel = NULL;
    } else {
        g_object_unref( mpMenuModel );
    }

    if ( mpActionGroup ) {
        g_lo_action_group_clear( G_LO_ACTION_GROUP( mpActionGroup ) );
    }

    pSessionBus = NULL;

    maSections.clear();
    maItems.clear();
}

sal_Bool GtkSalMenu::VisibleMenuBar()
{
    return sal_False;
}

void GtkSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    GtkSalMenuItem *pGtkSalMenuItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );

    maItems.push_back( pGtkSalMenuItem );

    if ( pGtkSalMenuItem->mpMenuItem ) {
        if ( !mpCurrentSection ) {
            mpCurrentSection = G_MENU_MODEL( g_lo_menu_new() );
            maSections.push_back( mpCurrentSection );

            if ( mpMenuModel ) {
                g_lo_menu_append_section( G_LO_MENU( mpMenuModel ), NULL, mpCurrentSection );
            }
        }

        pGtkSalMenuItem->mpParentSection = mpCurrentSection;
        pGtkSalMenuItem->mnPos = g_menu_model_get_n_items( mpCurrentSection );

        g_lo_menu_insert_item( G_LO_MENU( mpCurrentSection ), pGtkSalMenuItem->mnPos, pGtkSalMenuItem->mpMenuItem );
    } else {
        // If no mpMenuItem exists, then item is a separator.
        mpCurrentSection = G_MENU_MODEL( g_lo_menu_new() );
        maSections.push_back( mpCurrentSection );

        if ( mpMenuModel ) {
            g_lo_menu_append_section( G_LO_MENU( mpMenuModel ), NULL, mpCurrentSection );
        }
    }

    pGtkSalMenuItem->mpParentMenu = this;
}

void GtkSalMenu::RemoveItem( unsigned nPos )
{
    // FIXME: This method makes the application crash.
//    GtkSalMenuItem* pSalMenuItem = maItems[ nPos ];

//    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( GetActionGroupFromMenubar( this ) );
//    if ( pActionGroup ) {
//        g_lo_action_group_remove( pActionGroup, pSalMenuItem->maCommand );
//    }

//    cout << __FUNCTION__ << " - " << pSalMenuItem->mpParentSection << " - " << pSalMenuItem->mnPos << endl;

//    if ( pSalMenuItem->mpParentSection ) {
//        if ( GetSectionNumber( pSalMenuItem->mpParentSection ) > -1 )
//            g_lo_menu_remove( G_LO_MENU( pSalMenuItem->mpParentSection ), pSalMenuItem->mnPos );

//        if ( g_menu_model_get_n_items( pSalMenuItem->mpParentSection ) == 0 ) {
//            sal_uInt16 nSection = GetSectionNumber( pSalMenuItem->mpParentSection );

//            cout << "Removing section " << nSection << endl;

//            g_lo_menu_remove( G_LO_MENU( mpMenuModel ), nSection );
//            maSections.erase( maSections.begin() + nSection, maSections.begin() + nSection );

//            if ( pSalMenuItem->mpParentSection == mpCurrentSection )
//                mpCurrentSection = maSections.back();
//        }
//    }

//    maItems.erase( maItems.begin() + nPos, maItems.begin() + nPos );
}

void GtkSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
    GtkSalMenuItem *pGtkSalMenuItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );
    GtkSalMenu *pGtkSubMenu = static_cast<GtkSalMenu*>( pSubMenu );

    if ( pGtkSubMenu ) {
        pGtkSalMenuItem->mpSubMenu = pGtkSubMenu;
        g_lo_menu_item_set_submenu( pGtkSalMenuItem->mpMenuItem, pGtkSubMenu->mpMenuModel );

        if ( !pGtkSubMenu->mpParentSalMenu ) {
            pGtkSubMenu->mpParentSalMenu = this;
        }
    }
}

void GtkSalMenu::SetFrame( const SalFrame* pFrame )
{
    mpFrame = static_cast<const GtkSalFrame*>( pFrame );

    GtkWidget *widget = GTK_WIDGET( mpFrame->getWindow() );

    GdkWindow *gdkWindow = gtk_widget_get_window( widget );

    if (gdkWindow) {
        gpointer pMenu = g_object_get_data( G_OBJECT( gdkWindow ), "g-lo-menubar" );
        GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( g_object_get_data( G_OBJECT( gdkWindow ), "g-lo-action-group" ) );

        if ( pMenu && pActionGroup ) {
            mpMenuModel = G_MENU_MODEL( pMenu );

            // Merge current action group with the exported one
            g_lo_action_group_clear( pActionGroup );
            g_lo_action_group_merge( G_LO_ACTION_GROUP( mpActionGroup ), pActionGroup );
            g_lo_action_group_clear( G_LO_ACTION_GROUP( mpActionGroup ) );
            g_object_unref( mpActionGroup );

            mpActionGroup = G_ACTION_GROUP( pActionGroup );
        } else {
            mpMenuModel = G_MENU_MODEL( g_lo_menu_new() );

            g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-menubar", mpMenuModel, ObjectDestroyedNotify );
            g_object_set_data_full( G_OBJECT( gdkWindow ), "g-lo-action-group", mpActionGroup, ObjectDestroyedNotify );

            XLIB_Window windowId = GDK_WINDOW_XID( gdkWindow );

            gchar* aDBusPath = g_strdup_printf("/window/%lu", windowId);
            gchar* aDBusWindowPath = g_strdup_printf( "/window/%lu", windowId );
            gchar* aDBusMenubarPath = g_strdup_printf( "/window/%lu/menus/menubar", windowId );

            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_UNIQUE_BUS_NAME", g_dbus_connection_get_unique_name( pSessionBus ) );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_APPLICATION_OBJECT_PATH", "" );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_WINDOW_OBJECT_PATH", aDBusWindowPath );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_MENUBAR_OBJECT_PATH", aDBusMenubarPath );

            // Publish the menu.
            if ( aDBusMenubarPath ) {
                mMenubarId = g_dbus_connection_export_menu_model (pSessionBus, aDBusMenubarPath, mpMenuModel, NULL);
                if(!mMenubarId) puts("Fail export menubar");
            }

            if ( aDBusPath ) {
                mActionGroupId = g_dbus_connection_export_action_group( pSessionBus, aDBusPath, mpActionGroup, NULL);
            }

            g_free( aDBusPath );
            g_free( aDBusWindowPath );
            g_free( aDBusMenubarPath );
        }

        g_lo_menu_append_section( G_LO_MENU( mpMenuModel ), NULL, mpCurrentSection );

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
//    cout << __FUNCTION__ << " - " << nPos << " - " << bCheck << endl;
//    GActionGroup* pActionGroup = GetActionGroupFromMenubar( this );

//    if ( !pActionGroup )
//        return;

//    GtkSalMenuItem* pSalMenuItem = maItems[ nPos ];

//    if ( !pSalMenuItem || pSalMenuItem->mpSubMenu )
//        return;

//    MenuItemBits itemBits = pSalMenuItem->mnBits;
//    GVariant *pCheckValue = NULL;

//    if ( itemBits & MIB_CHECKABLE ) {
//        gboolean bCheckedValue = ( bCheck == sal_True ) ? TRUE : FALSE;
//        pCheckValue = g_variant_new_boolean( bCheckedValue );
//    }

//    if ( pCheckValue )
//        g_action_group_change_action_state( pActionGroup, pSalMenuItem->maCommand, pCheckValue );
}

void GtkSalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
{
    GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( GetActionGroupFromMenubar( this ) );

    if ( pActionGroup ) {
        GtkSalMenuItem* pSalMenuItem = maItems[ nPos ];

        gboolean bItemEnabled = (bEnable == sal_True) ? TRUE : FALSE;

        if ( g_strcmp0( pSalMenuItem->maCommand, "" ) == 0 )
            return;

        g_lo_action_group_set_action_enabled( pActionGroup, pSalMenuItem->maCommand, bItemEnabled );
    }
}

void GtkSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& rText )
{
    // Replace the "~" character with "_".
    rtl::OUString aText = rText.replace( '~', '_' );
    rtl::OString aConvertedText = OUStringToOString(aText, RTL_TEXTENCODING_UTF8);

    GtkSalMenuItem *pGtkSalMenuItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );

    GLOMenuItem *pMenuItem = G_LO_MENU_ITEM( pGtkSalMenuItem->mpMenuItem );

    GVariant* aCurrentLabel = g_menu_model_get_item_attribute_value( pGtkSalMenuItem->mpParentSection, pGtkSalMenuItem->mnPos, G_MENU_ATTRIBUTE_LABEL, G_VARIANT_TYPE_STRING );

    sal_Bool bSetLabel = sal_True;

    if ( aCurrentLabel ) {
        if ( g_strcmp0( g_variant_get_string( aCurrentLabel, NULL ), aConvertedText.getStr() ) == 0 ) {
            bSetLabel = sal_False;
        }
    }

    if ( bSetLabel == sal_True ) {
        g_lo_menu_item_set_label( pMenuItem, aConvertedText.getStr() );

        if ( pGtkSalMenuItem->mpParentSection ) {
            g_lo_menu_remove( G_LO_MENU( pGtkSalMenuItem->mpParentSection ), pGtkSalMenuItem->mnPos );
            g_lo_menu_insert_item( G_LO_MENU( pGtkSalMenuItem->mpParentSection ), pGtkSalMenuItem->mnPos, pGtkSalMenuItem->mpMenuItem );
        }
    }
}

void GtkSalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage)
{
}

void GtkSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const rtl::OUString& rKeyName )
{
}

void GtkSalMenu::SetItemCommand( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& aCommandStr )
{
    GtkSalMenuItem* pGtkSalMenuItem = static_cast< GtkSalMenuItem* >( pSalMenuItem );

    if ( pGtkSalMenuItem && pGtkSalMenuItem->mpMenuItem ) {
        rtl::OString aOCommandStr = rtl::OUStringToOString( aCommandStr, RTL_TEXTENCODING_UTF8 );

        if ( pGtkSalMenuItem->maCommand )
            g_free( pGtkSalMenuItem->maCommand );

        pGtkSalMenuItem->maCommand = g_strdup( aOCommandStr.getStr() );

        GLOActionGroup* pActionGroup = G_LO_ACTION_GROUP( GetActionGroupFromMenubar( this ) );
        if ( !pGtkSalMenuItem->mpVCLMenu->GetPopupMenu( pGtkSalMenuItem->mnId ) && pActionGroup ) {
            g_lo_action_group_insert( pActionGroup, pGtkSalMenuItem->maCommand, pGtkSalMenuItem );
        }

        gchar* aItemCommand = g_strconcat("win.", pGtkSalMenuItem->maCommand, NULL );

        g_lo_menu_item_set_action_and_target( pGtkSalMenuItem->mpMenuItem, aItemCommand, NULL );

        g_free( aItemCommand );

        if ( pGtkSalMenuItem->mpParentSection ) {
            g_lo_menu_remove( G_LO_MENU( pGtkSalMenuItem->mpParentSection ), pGtkSalMenuItem->mnPos );
            g_lo_menu_insert_item( G_LO_MENU( pGtkSalMenuItem->mpParentSection ), pGtkSalMenuItem->mnPos, pGtkSalMenuItem->mpMenuItem );
        }
    }
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
    mnPos( 0 ),
    maCommand( NULL ),
    mpVCLMenu( pItemData->pMenu ),
    mpParentMenu( NULL ),
    mpSubMenu( NULL ),
    mpMenuItem( NULL ),
    mpStateType( NULL ),
    mpState( NULL )
{
    if ( pItemData->eType != MENUITEM_SEPARATOR ) {
        mpMenuItem = g_lo_menu_item_new( "EMPTY STRING", NULL );

        maCommand = g_strdup( rtl::OUStringToOString( mpVCLMenu->GetItemCommand( mnId ), RTL_TEXTENCODING_UTF8 ).getStr() );
        gchar* aActionCommand = g_strconcat( "win.", maCommand, NULL );
        g_lo_menu_item_set_action_and_target( mpMenuItem, aActionCommand, NULL );

        g_free( aActionCommand );
    }
}

GtkSalMenuItem::~GtkSalMenuItem()
{

    if ( mpMenuItem ) {
        g_object_unref( mpMenuItem );
        g_free( maCommand );
    }
}
