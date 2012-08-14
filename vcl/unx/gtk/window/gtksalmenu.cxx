
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

static void
dispatchAction (GSimpleAction   *action,
                GVariant        *parameter,
                gpointer        user_data)
{
    GTK_YIELD_GRAB();

    if ( user_data ) {
        GtkSalMenuItem *pSalMenuItem = static_cast< GtkSalMenuItem* >( user_data );

        if ( !pSalMenuItem->mpSubMenu ) {
            const GtkSalFrame *pFrame = pSalMenuItem->mpParentMenu ? pSalMenuItem->mpParentMenu->GetFrame() : NULL;

            if ( pFrame && !pFrame->GetParent() ) {
                ((PopupMenu*) pSalMenuItem->mpVCLMenu)->SetSelectedEntry( pSalMenuItem->mnId );
                SalMenuEvent aMenuEvt( pSalMenuItem->mnId, pSalMenuItem->mpVCLMenu );
                pFrame->CallCallback( SALEVENT_MENUCOMMAND, &aMenuEvt );
            }
            else if ( pSalMenuItem->mpVCLMenu )
            {
                // if an item from submenu was selected. the corresponding Window does not exist because
                // we use native popup menus, so we have to set the selected menuitem directly
                // incidentally this of course works for top level popup menus, too
                PopupMenu * pPopupMenu = dynamic_cast<PopupMenu *>(pSalMenuItem->mpVCLMenu);
                if( pPopupMenu )
                {
                    // FIXME: revise this ugly code

                    // select handlers in vcl are dispatch on the original menu
                    // if not consumed by the select handler of the current menu
                    // however since only the starting menu ever came into Execute
                    // the hierarchy is not build up. Workaround this by getting
                    // the menu it should have been

                    // get started from hierarchy in vcl menus
                    GtkSalMenu* pParentMenu = pSalMenuItem->mpParentMenu;
                    Menu* pCurMenu = pSalMenuItem->mpVCLMenu;
                    while( pParentMenu && pParentMenu->GetMenu() )
                    {
                        pCurMenu = pParentMenu->GetMenu();
                        pParentMenu = pParentMenu->GetParentSalMenu();
                    }

                    pPopupMenu->SetSelectedEntry( pSalMenuItem->mnId );
                    pPopupMenu->ImplSelectWithStart( pCurMenu );
                }
                else
                    OSL_FAIL( "menubar item without frame !" );
            }
        }
    }
}

void generateActions( GtkSalMenu* pMenu, GLOActionGroup* pActionGroup )
{
    if ( !pMenu || !pMenu->GetMenuModel() )
        return;

    for (sal_uInt16 i = 0; i < pMenu->GetItemCount(); i++) {
        GtkSalMenuItem *pSalMenuItem = pMenu->GetItemAtPos( i );

        if ( pSalMenuItem->mpAction ) {
            g_lo_action_group_insert( pActionGroup, pSalMenuItem->mpAction );
        }

        generateActions( pSalMenuItem->mpSubMenu, pActionGroup );
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

void GtkSalMenu::publishMenu( GMenuModel *pMenu, GActionGroup *pActionGroup )
{
    if ( mMenubarId ) {
        g_dbus_connection_unexport_menu_model( pSessionBus, mMenubarId );
        mMenubarId = 0;
    }

    if ( mActionGroupId ) {
        g_dbus_connection_unexport_action_group( pSessionBus, mActionGroupId );
        mActionGroupId = 0;
    }

    if ( aDBusMenubarPath ) {
        mMenubarId = g_dbus_connection_export_menu_model (pSessionBus, aDBusMenubarPath, pMenu, NULL);
        if(!mMenubarId) puts("Fail export menubar");
    }

    if ( aDBusPath ) {
        mActionGroupId = g_dbus_connection_export_action_group( pSessionBus, aDBusPath, pActionGroup, NULL);
    }
}


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

GtkSalMenu::GtkSalMenu( sal_Bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mpVCLMenu( NULL ),
    mpParentSalMenu( NULL ),
    mpFrame( NULL ),
    aDBusPath( NULL ),
    aDBusMenubarPath( NULL ),
    pSessionBus( NULL ),
    mMenubarId( 0 ),
    mActionGroupId ( 0 )
{
    mpCurrentSection = G_MENU_MODEL( g_lo_menu_new() );
    maSections.push_back( mpCurrentSection );

    mpMenuModel = G_MENU_MODEL( g_lo_menu_new() );
    g_lo_menu_append_section( G_LO_MENU( mpMenuModel ), NULL, mpCurrentSection );

    if (bMenuBar) {
        pSessionBus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
        if(!pSessionBus) puts ("Fail bus get");
    }
}

GtkSalMenu::~GtkSalMenu()
{
    if ( mpFrame ) {
        GtkWidget *widget = GTK_WIDGET( mpFrame->getWindow() );
        GdkWindow *gdkWindow = gtk_widget_get_window( widget );
        if (gdkWindow) {
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_UNIQUE_BUS_NAME", NULL );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_APPLICATION_OBJECT_PATH", NULL );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_WINDOW_OBJECT_PATH", NULL );
            gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_MENUBAR_OBJECT_PATH", NULL );
        }
    }

    if ( mMenubarId ) {
        g_dbus_connection_unexport_menu_model( pSessionBus, mMenubarId );
        mMenubarId = 0;
    }

    if ( mActionGroupId ) {
        g_dbus_connection_unexport_action_group( pSessionBus, mActionGroupId );
        mActionGroupId = 0;
    }

    if ( pSessionBus ) {
        g_dbus_connection_flush_sync( pSessionBus, NULL, NULL );
    }

    pSessionBus = NULL;

    g_free( aDBusPath );
    g_free( aDBusMenubarPath );

    maSections.clear();
    maItems.clear();

    g_object_unref( mpMenuModel );
    g_object_unref( mpCurrentSection );
}

sal_Bool GtkSalMenu::VisibleMenuBar()
{
    return sal_False;
}

void GtkSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    GtkSalMenuItem *pGtkSalMenuItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );

    if ( pGtkSalMenuItem->mpMenuItem ) {
        pGtkSalMenuItem->mpParentSection = mpCurrentSection;
        pGtkSalMenuItem->mnPos = g_menu_model_get_n_items( mpCurrentSection );

        maItems.push_back( pGtkSalMenuItem );

        g_lo_menu_insert_item( G_LO_MENU( mpCurrentSection ), pGtkSalMenuItem->mnPos, pGtkSalMenuItem->mpMenuItem );
    } else {
        // If no mpMenuItem exists, then item is a separator.
        mpCurrentSection = G_MENU_MODEL( g_lo_menu_new() );
        maSections.push_back( mpCurrentSection );
        g_lo_menu_append_section( G_LO_MENU( mpMenuModel ), NULL, mpCurrentSection );
    }

    pGtkSalMenuItem->mpParentMenu = this;
}

void GtkSalMenu::RemoveItem( unsigned nPos )
{
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
        XLIB_Window windowId = GDK_WINDOW_XID( gdkWindow );

        aDBusPath = g_strdup_printf("/window/%lu", windowId);
        gchar* aDBusWindowPath = g_strdup_printf( "/window/%lu", windowId );
        aDBusMenubarPath = g_strdup_printf( "/window/%lu/menus/menubar", windowId );

        gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_UNIQUE_BUS_NAME", g_dbus_connection_get_unique_name( pSessionBus ) );
        gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_APPLICATION_OBJECT_PATH", "" );
        gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_WINDOW_OBJECT_PATH", aDBusWindowPath );
        gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_MENUBAR_OBJECT_PATH", aDBusMenubarPath );

        // Try to publish the menu with the right bus data.
        Freeze();
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

void GtkSalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
{
    sal_uInt16 itemId = mpVCLMenu->GetItemId( nPos );

    GtkSalMenuItem *pSalMenuItem = GetSalMenuItem( itemId );

    if ( pSalMenuItem && pSalMenuItem->mpAction ) {
        gboolean bItemEnabled = (bEnable == sal_True) ? TRUE : FALSE;
        g_simple_action_set_enabled( G_SIMPLE_ACTION( pSalMenuItem->mpAction ), bItemEnabled );
    }
}

void GtkSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& rText )
{
    // Replace the "~" character with "_".
    rtl::OUString aText = rText.replace( '~', '_' );
    rtl::OString aConvertedText = OUStringToOString(aText, RTL_TEXTENCODING_UTF8);

    GtkSalMenuItem *pGtkSalMenuItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );

    GLOMenuItem *pMenuItem = G_LO_MENU_ITEM( pGtkSalMenuItem->mpMenuItem );

    g_lo_menu_item_set_label( pMenuItem, aConvertedText.getStr() );

    if ( pGtkSalMenuItem->mpParentSection ) {
        g_lo_menu_remove( G_LO_MENU( pGtkSalMenuItem->mpParentSection ), pGtkSalMenuItem->mnPos );
        g_lo_menu_insert_item( G_LO_MENU( pGtkSalMenuItem->mpParentSection ), pGtkSalMenuItem->mnPos, pGtkSalMenuItem->mpMenuItem );
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

    if ( pGtkSalMenuItem->mpAction ) {
        g_object_unref( pGtkSalMenuItem->mpAction );
    }

    rtl::OString aOCommandStr = rtl::OUStringToOString( aCommandStr, RTL_TEXTENCODING_UTF8 );

    GSimpleAction *pAction = g_simple_action_new( aOCommandStr.getStr(), NULL );

//    if ( !pGtkSalMenuItem->mpVCLMenu->GetPopupMenu( pGtkSalMenuItem->mnId ) ) {
        g_signal_connect(pAction, "activate", G_CALLBACK( dispatchAction ), pGtkSalMenuItem);
//    }

    pGtkSalMenuItem->mpAction = G_ACTION( pAction );


    rtl::OString aItemCommand = "win." + aOCommandStr;
    g_lo_menu_item_set_action_and_target( pGtkSalMenuItem->mpMenuItem, aItemCommand.getStr(), NULL );

    if ( pGtkSalMenuItem->mpParentSection ) {
        g_lo_menu_remove( G_LO_MENU( pGtkSalMenuItem->mpParentSection ), pGtkSalMenuItem->mnPos );
        g_lo_menu_insert_item( G_LO_MENU( pGtkSalMenuItem->mpParentSection ), pGtkSalMenuItem->mnPos, pGtkSalMenuItem->mpMenuItem );
    }
}

void GtkSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
}

bool GtkSalMenu::ShowNativePopupMenu(FloatingWindow * pWin, const Rectangle& rRect, sal_uLong nFlags)
{
    return TRUE;
}

void updateNativeMenu( GtkSalMenu* pMenu ) {
    if ( pMenu ) {
        for ( sal_uInt16 i = 0; i < pMenu->GetItemCount(); i++ ) {
            GtkSalMenuItem* pSalMenuItem = pMenu->GetItemAtPos( i );
            String aText = pSalMenuItem->mpVCLMenu->GetItemText( pSalMenuItem->mnId );

            // Force updating of native menu labels.
            pMenu->SetItemText( i, pSalMenuItem, aText );

            if ( pSalMenuItem->mpSubMenu && pSalMenuItem->mpSubMenu->GetMenu() ) {
                pSalMenuItem->mpSubMenu->GetMenu()->Activate();
                updateNativeMenu( pSalMenuItem->mpSubMenu );
            }
        }
    }
}
void GtkSalMenu::Freeze()
{
    updateNativeMenu( this );

    GLOActionGroup *mpActionGroup = g_lo_action_group_new();

    generateActions( this, mpActionGroup );

    // Menubar would have one section only.
    this->publishMenu( mpMenuModel, G_ACTION_GROUP( mpActionGroup ) );
}

// =======================================================================

/*
 * GtkSalMenuItem
 */

GtkSalMenuItem::GtkSalMenuItem( const SalItemParams* pItemData ) :
    mnId( pItemData->nId ),
    mnPos( 0 ),
    mpVCLMenu( pItemData->pMenu ),
    mpParentMenu( NULL ),
    mpSubMenu( NULL ),
    mpMenuItem( NULL ),
    mpAction( NULL )
{
    if ( pItemData->eType != MENUITEM_SEPARATOR ) {
        mpMenuItem = g_lo_menu_item_new( "EMPTY STRING", NULL );
    }
}

GtkSalMenuItem::~GtkSalMenuItem()
{
    if ( mpMenuItem ) {
        g_object_unref( mpMenuItem );
    }
}
