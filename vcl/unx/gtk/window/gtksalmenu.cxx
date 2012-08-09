
#include "unx/gtk/gtksalmenu.hxx"

//#include <gtk/gtk.h>
#include <unx/gtk/glomenu.h>
#include <unx/gtk/gloactiongroup.h>
#include <vcl/menu.hxx>
#include <unx/gtk/gtkinst.hxx>

#include <iostream>

using namespace std;

#define GTK_MENU_BUS_NAME   "org.libreoffice"
#define GTK_MENU_OBJ_PATH   "/org/libreoffice"

static void
dispatchAction (GSimpleAction   *action,
                GVariant        *parameter,
                gpointer        user_data)
{
    cout << "ACTION: " << g_action_get_name( G_ACTION( action ) ) << " triggered." << endl;
    GTK_YIELD_GRAB();

    if ( user_data ) {
        GtkSalMenuItem *pSalMenuItem = static_cast< GtkSalMenuItem* >( user_data );

        if ( !pSalMenuItem->mpSubMenu ) {
            const GtkSalFrame *pFrame = pSalMenuItem->mpParentMenu ? pSalMenuItem->mpParentMenu->getFrame() : NULL;

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
                    while( pParentMenu && pParentMenu->mpVCLMenu )
                    {
                        pCurMenu = pParentMenu->mpVCLMenu;
                        pParentMenu = pParentMenu->mpParentSalMenu;
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

//GMenuModel* generateMenuModel2( Menu *pVCLMenu )
//{
//    if (!pVCLMenu)
//        return NULL;

//    GMenu *pMenuModel = g_menu_new();
//    GMenu *pSectionMenuModel = g_menu_new();

//    for (int i = 0; i < pVCLMenu->GetItemCount(); i++) {
//        MenuItemType itemType = pVCLMenu->GetItemType( i );

//        if ( itemType == MENUITEM_SEPARATOR ) {
//            g_menu_append_section( pMenuModel, NULL, G_MENU_MODEL( pSectionMenuModel ) );
//            pSectionMenuModel = g_menu_new();
//        } else {
//            sal_Int16 nId = pVCLMenu->GetItemId( i );

//            // Menu item label
//            rtl::OUString aTextLabel = pVCLMenu->GetItemText( nId );
//            rtl::OUString aText = aTextLabel.replace( '~', '_' );
//            rtl::OString aConvertedText = OUStringToOString(aText, RTL_TEXTENCODING_UTF8);

//            // Menu item accelerator key
////            KeyCode accelKey = pVCLMenu->GetAccelKey( nId );

//            GMenuItem *menuItem = g_menu_item_new( (char*) aConvertedText.getStr(), NULL);

//            GMenuModel *pSubmenu = generateMenuModel2( pVCLMenu->GetPopupMenu( nId ) );

//            g_menu_item_set_submenu( menuItem, pSubmenu );

//            g_menu_append_item( pSectionMenuModel, menuItem );
//        }
//    }

//    g_menu_append_section( pMenuModel, NULL, G_MENU_MODEL( pSectionMenuModel ) );

//    return G_MENU_MODEL( pMenuModel );
//}


GMenuModel *generateMenuModelAndActions( GtkSalMenu*, GLOActionGroup* );

GMenuModel *generateSectionMenuModel( GtkSalMenuSection* pSection, GLOActionGroup* pActionGroup )
{
    if ( !pSection )
        return NULL;

    GMenu *pSectionMenuModel = g_menu_new();

    for (sal_uInt16 i = 0; i < pSection->size(); i++) {
        GtkSalMenuItem *pSalMenuItem = pSection->at( i );
        GMenuItem *pMenuItem = pSalMenuItem->mpMenuItem;

        if (pSalMenuItem->mpSubMenu) {
            GMenuModel *pSubmenu = generateMenuModelAndActions( pSalMenuItem->mpSubMenu, pActionGroup );
            g_menu_item_set_submenu( pMenuItem, pSubmenu );
        }

        g_menu_append_item( pSectionMenuModel, pMenuItem );

        if (pSalMenuItem->mpAction) {
            g_lo_action_group_insert( pActionGroup, pSalMenuItem->mpAction );
        }
    }

    return G_MENU_MODEL( pSectionMenuModel );
}

GMenuModel *generateMenuModelAndActions( GtkSalMenu *pMenu, GLOActionGroup *pActionGroup )
{
    if ( !pMenu )
        return NULL;

    GMenu *pMenuModel = g_menu_new();

//    for (sal_uInt16 i = 0; i < pMenu->maItems.size(); i++) {
//        GtkSalMenuItem *pSalMenuItem = pMenu->maItems[ i ];
//        GMenuItem *pMenuItem = pSalMenuItem->mpMenuItem;

//        if (pSalMenuItem->mpSubMenu) {
//            GMenuModel *pSubmenu = generateMenuModelAndActions( pSalMenuItem->mpSubMenu, pActionGroup );
//            g_menu_item_set_submenu( pMenuItem, pSubmenu );
//        }

//        g_menu_append_item( pMenuModel, pMenuItem );
//        g_lo_action_group_insert( pActionGroup, pSalMenuItem->mpAction );
//    }

    for (sal_uInt16 i = 0; i < pMenu->maSections.size(); i++) {
        GtkSalMenuSection *pSection = pMenu->maSections[ i ];

        GMenuModel *pSectionMenuModel = generateSectionMenuModel( pSection, pActionGroup );

        g_menu_append_section( pMenuModel, NULL, pSectionMenuModel );
    }

    return G_MENU_MODEL( pMenuModel );
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
//        guint appmenuID = g_dbus_connection_export_menu_model (bus, "/org/libreoffice/menus/appmenu", mpMenuModel, NULL);
//        if(!appmenuID) puts("Fail export appmenu");

    if ( mMenubarId ) {
        g_dbus_connection_unexport_menu_model( pSessionBus, mMenubarId );
        mbMenuBar = 0;
    }

    mMenubarId = g_dbus_connection_export_menu_model (pSessionBus, "/org/libreoffice/menus/menubar", pMenu, NULL);
    if(!mMenubarId) puts("Fail export menubar");

    g_dbus_connection_export_action_group( pSessionBus, GTK_MENU_OBJ_PATH, pActionGroup, NULL);
}


// FIXME: HIGHLY IMPROVABLE CODE
GtkSalMenuItem* GtkSalMenu::GetSalMenuItem( sal_uInt16 nId )
{
    for ( sal_uInt16 i = 0; i < maSections.size(); i++ )
    {
        GtkSalMenuSection* pSection = maSections[ i ];

        for ( sal_uInt16 j = 0; j < pSection->size(); j++ )
        {
            GtkSalMenuItem* pSalMenuItem = pSection->at( j );

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
    }

    return NULL;
}

GtkSalMenu::GtkSalMenu( sal_Bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mpVCLMenu( NULL ),
    mpParentSalMenu( NULL ),
    mpFrame( NULL ),
    aDBusMenubarPath( NULL ),
    pSessionBus( NULL ),
    mBusId( 0 ),
    mMenubarId( 0 ),
    mActionGroupId ( 0 )
{
    mpCurrentSection = new GtkSalMenuSection();
    maSections.push_back( mpCurrentSection );

    if (bMenuBar) {
        pSessionBus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
        if(!pSessionBus) puts ("Fail bus get");

        mBusId = g_bus_own_name_on_connection (pSessionBus, GTK_MENU_BUS_NAME, G_BUS_NAME_OWNER_FLAGS_NONE, NULL, NULL, NULL, NULL);
        if(!mBusId) puts ("Fail own name");
    }
}

GtkSalMenu::~GtkSalMenu()
{
    if ( mMenubarId ) {
        g_dbus_connection_unexport_menu_model( pSessionBus, mMenubarId );
    }

    if ( mBusId ) {
        g_bus_unown_name( mBusId );
    }

    if ( pSessionBus ) {
        g_dbus_connection_close_sync( pSessionBus, NULL, NULL );
    }

    maSections.clear();
}

sal_Bool GtkSalMenu::VisibleMenuBar()
{
    return sal_False;
}

void GtkSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    cout << __FUNCTION__ << "  pos: " << nPos << endl;
    GtkSalMenuItem *pGtkSalMenuItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );

    if ( pGtkSalMenuItem->mpMenuItem ) {
        mpCurrentSection->push_back( pGtkSalMenuItem );
    } else {
        // If no mpMenuItem exists, then item is a separator.
        mpCurrentSection = new GtkSalMenuSection();
        maSections.push_back( mpCurrentSection );
    }

    pGtkSalMenuItem->mpParentMenu = this;
}

void GtkSalMenu::RemoveItem( unsigned nPos )
{
    cout << __FUNCTION__ << " Item: " << nPos << endl;

//    if (nPos < maItems.size()) {
//        std::vector< GtkSalMenuItem* >::iterator iterator;
//        iterator = maItems.begin() + nPos;

//        maItems.erase( iterator, iterator );
//    }
}

void GtkSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
    cout << __FUNCTION__ << "  Pos: " << nPos << endl;

    GtkSalMenuItem *pGtkSalMenuItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );
    GtkSalMenu *pGtkSubMenu = static_cast<GtkSalMenu*>( pSubMenu );

    if ( pGtkSubMenu ) {
        pGtkSalMenuItem->mpSubMenu = pGtkSubMenu;

        if ( !pGtkSubMenu->mpParentSalMenu ) {
            pGtkSubMenu->mpParentSalMenu = this;
        }
    }
}

void GtkSalMenu::SetFrame( const SalFrame* pFrame )
{
    cout << __FUNCTION__ << endl;

    mpFrame = static_cast<const GtkSalFrame*>( pFrame );

    GtkWidget *widget = GTK_WIDGET( mpFrame->getWindow() );

    GdkWindow *gdkWindow = gtk_widget_get_window( widget );

    if (gdkWindow) {
        XLIB_Window windowId = GDK_WINDOW_XID( gdkWindow );

        gchar *aWindowObjectPath = g_strdup_printf( "%s/window/%lu", GTK_MENU_OBJ_PATH, windowId );
        gchar *aMenubarObjectPath = g_strconcat( GTK_MENU_OBJ_PATH, "/menus/menubar", NULL );

//        gdk_x11_window_set_utf8_property (gdkWindow, "_GTK_APPLICATION_ID", "org.libreoffice");
        gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_UNIQUE_BUS_NAME", g_dbus_connection_get_unique_name( pSessionBus ) );
        gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_APPLICATION_OBJECT_PATH", GTK_MENU_OBJ_PATH );
        gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_WINDOW_OBJECT_PATH", aWindowObjectPath );
//        gdk_x11_window_set_utf8_property (gdkWindow, "_GTK_APP_MENU_OBJECT_PATH", "/org/libreoffice/menus/appmenu");
        gdk_x11_window_set_utf8_property ( gdkWindow, "_GTK_MENUBAR_OBJECT_PATH", aMenubarObjectPath );

        g_free( aWindowObjectPath );
        g_free( aMenubarObjectPath );
    }
}

const GtkSalFrame* GtkSalMenu::getFrame() const
{
    const GtkSalMenu* pMenu = this;
    while( pMenu && ! pMenu->mpFrame )
        pMenu = pMenu->mpParentSalMenu;
    return pMenu ? pMenu->mpFrame : NULL;
}

void GtkSalMenu::CheckItem( unsigned nPos, sal_Bool bCheck )
{
    cout << __FUNCTION__ << endl;
}

void GtkSalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
{
    cout << __FUNCTION__ << endl;

    sal_uInt16 itemId = mpVCLMenu->GetItemId( nPos );

    GtkSalMenuItem *pSalMenuItem = GetSalMenuItem( itemId );

    if ( pSalMenuItem->mpAction ) {
        gboolean bItemEnabled = (bEnable == sal_True) ? TRUE : FALSE;
        g_simple_action_set_enabled( G_SIMPLE_ACTION( pSalMenuItem->mpAction ), bItemEnabled );
    }
}

void GtkSalMenu::SetItemText( unsigned nPos, SalMenuItem* pSalMenuItem, const rtl::OUString& rText )
{
    cout << __FUNCTION__ << endl;
    // Replace the "~" character with "_".
    rtl::OUString aText = rText.replace( '~', '_' );
    rtl::OString aConvertedText = OUStringToOString(aText, RTL_TEXTENCODING_UTF8);

    cout << "Setting label: " << aConvertedText.getStr() << endl;

    GtkSalMenuItem *pGtkSalMenuItem = static_cast<GtkSalMenuItem*>( pSalMenuItem );

    GMenuItem *pMenuItem = G_MENU_ITEM( pGtkSalMenuItem->mpMenuItem );

    g_menu_item_set_label( pMenuItem, (char*) aConvertedText.getStr() );
}

void GtkSalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage)
{
    cout << __FUNCTION__ << endl;
}

void GtkSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const rtl::OUString& rKeyName )
{
    cout << __FUNCTION__ << " KeyName: " << rKeyName << endl;

    GtkSalMenuItem *pMenuItem = static_cast< GtkSalMenuItem* >( pSalMenuItem );

//    rtl::OString aConvertedKeyName = OUStringToOString( rKeyName, RTL_TEXTENCODING_UTF8 );

//    GVariant *gaKeyCode = g_variant_new_string( aConvertedKeyName.getStr() );
//    g_menu_item_set_attribute_value( pMenuItem->mpMenuItem, "accel", gaKeyCode );
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


    rtl::OString aItemCommand = "app." + aOCommandStr;
    g_menu_item_set_action_and_target( pGtkSalMenuItem->mpMenuItem, aItemCommand.getStr(), NULL );
//    g_object_unref( aGCommand );
}

void GtkSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
    cout << __FUNCTION__ << endl;
}

bool GtkSalMenu::ShowNativePopupMenu(FloatingWindow * pWin, const Rectangle& rRect, sal_uLong nFlags)
{
    cout << __FUNCTION__ << endl;
    return TRUE;
}

void GtkSalMenu::Freeze()
{
    cout << __FUNCTION__ << endl;
    GLOActionGroup *mpActionGroup = g_lo_action_group_new();

    GMenuModel *mpMenuModel = generateMenuModelAndActions( this, mpActionGroup );
//    GMenuModel *mpMenuModel = generateMockMenuModel();

//    this->publishMenu( mpMenuModel, G_ACTION_GROUP( mpActionGroup ) );
    this->publishMenu( mpMenuModel, G_ACTION_GROUP( mpActionGroup ) );
    g_object_unref( mpMenuModel );
}

// =======================================================================

/*
 * GtkSalMenuItem
 */

GtkSalMenuItem::GtkSalMenuItem( const SalItemParams* pItemData ) :
    mnId( pItemData->nId ),
    mpVCLMenu( pItemData->pMenu ),
    mpParentMenu( NULL ),
    mpSubMenu( NULL ),
    mpMenuItem( NULL ),
    mpAction( NULL )
{
    cout << __FUNCTION__ << "Type: " << (sal_uInt16) pItemData->eType << endl;

    if ( pItemData->eType != MENUITEM_SEPARATOR ) {
        mpMenuItem = g_menu_item_new( "b", NULL );
    }
}

GtkSalMenuItem::~GtkSalMenuItem()
{
    if ( mpMenuItem ) {
        g_object_unref( mpMenuItem );
    }
}
