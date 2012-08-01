
#include "unx/gtk/gtksalmenu.hxx"

#include <gtk/gtk.h>
#include <iostream>

using namespace std;


//const GtkSalMenu * GtkSalMenu::pCurrentMenubar = NULL;

#define BUS_NAME "org.gtk.LibreOffice"
#define OBJ_PATH "/org/gtk/LibreOffice"

static void
quit (GSimpleAction *action,
      GVariant      *parameter,
      gpointer       user_data)
{
    exit(1);
}

GMenuModel* generateMenuModel2( Menu *pVCLMenu )
{
    if (!pVCLMenu)
        return NULL;

    GMenu *pMenuModel = g_menu_new();
    GMenu *pSectionMenuModel = g_menu_new();

    for (int i = 0; i < pVCLMenu->GetItemCount(); i++) {
        MenuItemType itemType = pVCLMenu->GetItemType( i );

        if ( itemType == MENUITEM_SEPARATOR ) {
            g_menu_append_section( pMenuModel, NULL, G_MENU_MODEL( pSectionMenuModel ) );
            pSectionMenuModel = g_menu_new();
        } else {
            sal_Int16 nId = pVCLMenu->GetItemId( i );

            // Menu item label
            rtl::OUString aTextLabel = pVCLMenu->GetItemText( nId );
            rtl::OUString aText = aTextLabel.replace( '~', '_' );
            rtl::OString aConvertedText = OUStringToOString(aText, RTL_TEXTENCODING_UTF8);

            // Menu item accelerator key
//            KeyCode accelKey = pVCLMenu->GetAccelKey( nId );

            GMenuItem *menuItem = g_menu_item_new( (char*) aConvertedText.getStr(), NULL);

            GMenuModel *pSubmenu = generateMenuModel2( pVCLMenu->GetPopupMenu( nId ) );

            g_menu_item_set_submenu( menuItem, pSubmenu );

            g_menu_append_item( pSectionMenuModel, menuItem );
        }
    }

    g_menu_append_section( pMenuModel, NULL, G_MENU_MODEL( pSectionMenuModel ) );

    return G_MENU_MODEL( pMenuModel );
}

GMenuModel *generateMockMenuModel()
{
    GMenu *menu = g_menu_new ();
    //            g_menu_append (menu, "Add", "app.add");
    //            g_menu_append (menu, "Del", "app.del");
    GMenu *fileMenu = g_menu_new();
    GMenu *submenu = g_menu_new ();
    g_menu_append( submenu, "Option1", NULL );
    g_menu_append( submenu, "Option2", NULL );

    g_menu_append_section( fileMenu, NULL, G_MENU_MODEL(submenu));

    g_menu_append (fileMenu, "Quit", "app.quit");

    g_menu_append_submenu( menu, "Test", G_MENU_MODEL( fileMenu ));

    return G_MENU_MODEL( menu );
}

GMenuModel *generateMenuModel( GtkSalMenu* );

GMenuModel *generateSectionMenuModel( GtkSalMenuSection *pSection )
{
    if ( !pSection )
        return NULL;

    GMenu *pSectionMenuModel = g_menu_new();

    for (int i=0; i < pSection->maItems.size(); i++) {
        GtkSalMenuItem *pSalMenuItem = pSection->maItems[ i ];
        GMenuItem *pMenuItem = pSalMenuItem->mpMenuItem;

        if (pSalMenuItem->mpSubMenu) {
            GMenuModel *pSubmenu = generateMenuModel( pSalMenuItem->mpSubMenu );
            g_menu_item_set_submenu( pMenuItem, pSubmenu );
        }

        g_menu_append_item( pSectionMenuModel, pMenuItem );
    }

    return G_MENU_MODEL( pSectionMenuModel );
}

GMenuModel *generateMenuModel( GtkSalMenu *pMenu )
{
    if ( !pMenu )
        return NULL;

    GMenu *pMenuModel = g_menu_new();

    for (int i=0; i < pMenu->maItems.size(); i++) {
        GtkSalMenuItem *pSalMenuItem = pMenu->maItems[ i ];
        GMenuItem *pMenuItem = pSalMenuItem->mpMenuItem;

        if (pSalMenuItem->mpSubMenu) {
            GMenuModel *pSubmenu = generateMenuModel( pSalMenuItem->mpSubMenu );
            g_menu_item_set_submenu( pMenuItem, pSubmenu );
        }

        g_menu_append_item( pMenuModel, pMenuItem );
    }

    for (int i=0; i < pMenu->maSections.size(); i++) {
        GtkSalMenuSection *pSection = pMenu->maSections[ i ];

        GMenuModel *pSectionMenuModel = generateSectionMenuModel( pSection );

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

void GtkSalMenu::publishMenu( GMenuModel *pMenu )
{
    pSessionBus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
    if(!pSessionBus) puts ("Fail bus get");
    mBusId = g_bus_own_name_on_connection (pSessionBus, "org.libreoffice", G_BUS_NAME_OWNER_FLAGS_NONE, NULL, NULL, NULL, NULL);
    if(!mBusId) puts ("Fail own name");

//        guint appmenuID = g_dbus_connection_export_menu_model (bus, "/org/libreoffice/menus/appmenu", mpMenuModel, NULL);
//        if(!appmenuID) puts("Fail export appmenu");
    mMenubarId = g_dbus_connection_export_menu_model (pSessionBus, "/org/libreoffice/menus/menubar", pMenu, NULL);
    if(!mMenubarId) puts("Fail export menubar");

//        g_object_unref (menu);
}

GtkSalMenu::GtkSalMenu( sal_Bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mpVCLMenu( NULL ),
    aDBusMenubarPath( NULL ),
    pSessionBus( NULL ),
    mBusId( 0 ),
    mMenubarId( 0 )
{
    if (!bMenuBar) {
        mpCurrentSection = new GtkSalMenuSection();
        maSections.push_back( mpCurrentSection );
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

    maItems.clear();
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
        if ( mbMenuBar ) {
//            if ( maItems.size() == 0 ) {
                maItems.push_back( pGtkSalMenuItem );
//            } else {
//                maItems.insert( maItems.begin() + nPos, pGtkSalMenuItem );
//            }
        } else {
//            if ( mpCurrentSection->maItems.size() == 0) {
                mpCurrentSection->maItems.push_back( pGtkSalMenuItem );
//            } else {
//                mpCurrentSection->maItems.insert( mpCurrentSection->maItems.begin() + nPos, pGtkSalMenuItem );
//            }
        }
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

    pGtkSalMenuItem->mpSubMenu = pGtkSubMenu;
}

void GtkSalMenu::SetFrame( const SalFrame* pFrame )
{
    cout << __FUNCTION__ << endl;

    mpFrame = static_cast<const GtkSalFrame*>( pFrame );

    GtkWidget *widget = GTK_WIDGET( mpFrame->getWindow() );

    GdkWindow *gdkWindow = gtk_widget_get_window( widget );

    if (gdkWindow) {
        gdk_x11_window_set_utf8_property (gdkWindow, "_GTK_APPLICATION_ID", "org.libreoffice");
        gdk_x11_window_set_utf8_property (gdkWindow, "_GTK_UNIQUE_BUS_NAME", "org.libreoffice");
        gdk_x11_window_set_utf8_property (gdkWindow, "_GTK_APPLICATION_OBJECT_PATH", "/org/libreoffice");
        gdk_x11_window_set_utf8_property (gdkWindow, "_GTK_WINDOW_OBJECT_PATH", "/org/libreoffice/windows");
//        gdk_x11_window_set_utf8_property (gdkWindow, "_GTK_APP_MENU_OBJECT_PATH", "/org/libreoffice/menus/appmenu");
        gdk_x11_window_set_utf8_property (gdkWindow, "_GTK_MENUBAR_OBJECT_PATH", "/org/libreoffice/menus/menubar");
    }
}

void GtkSalMenu::CheckItem( unsigned nPos, sal_Bool bCheck )
{
    cout << __FUNCTION__ << endl;
}

void GtkSalMenu::EnableItem( unsigned nPos, sal_Bool bEnable )
{
    cout << __FUNCTION__ << endl;
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

//    GtkSalMenuItem *pMenuItem = static_cast< GtkSalMenuItem* >( pSalMenuItem );

//    rtl::OString aConvertedKeyName = OUStringToOString( rKeyName, RTL_TEXTENCODING_UTF8 );

//    GVariant *gaKeyCode = g_variant_new_string( aConvertedKeyName.getStr() );
//    g_menu_item_set_attribute_value( pMenuItem->mpMenuItem, "accel", gaKeyCode );
}

void GtkSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
    cout << __FUNCTION__ << endl;
}

void printMenu( GtkSalMenu * );

void printSection ( GtkSalMenuSection *pSection )
{
    if (pSection) {
        for (int i = 0; i < pSection->maItems.size(); i++) {
            GtkSalMenuItem *pSalMenuItem = static_cast< GtkSalMenuItem* >(pSection->maItems[ i ]);
            cout << pSalMenuItem->mpVCLMenu->GetItemText( pSalMenuItem->mnId ) << endl;

            if (pSalMenuItem->mpSubMenu) {
                cout << "--- Submenu ---" << endl;
                printMenu( pSalMenuItem->mpSubMenu);
                cout << "---------------" << endl;
            }
        }
    }
}

void printMenu( GtkSalMenu *pMenu )
{
    if ( pMenu ) {
        for (int i = 0; i < pMenu->maItems.size(); i++) {
            GtkSalMenuItem *pSalMenuItem = static_cast< GtkSalMenuItem* >(pMenu->maItems[ i ]);
            cout << pSalMenuItem->mpVCLMenu->GetItemText( pSalMenuItem->mnId ) << endl;

            if (pSalMenuItem->mpSubMenu) {
                cout << "--- Submenu ---" << endl;
                printMenu( pSalMenuItem->mpSubMenu);
                cout << "---------------" << endl;
            }
        }

        for (int i = 0; i < pMenu->maSections.size(); i++) {
            GtkSalMenuSection *pSalMenuSection = static_cast< GtkSalMenuSection* >(pMenu->maSections[ i ]);

            cout << "--- Submenu ---" << endl;
            printSection( pSalMenuSection );
            cout << "---------------" << endl;
        }
    }
}

void GtkSalMenu::Freeze()
{
    cout << __FUNCTION__ << endl;
    GMenuModel *mpMenuModel = generateMenuModel( this );
    this->publishMenu( mpMenuModel );
    g_object_unref( mpMenuModel );

    cout << "==================== MENUBAR ===================" << endl;
    printMenu( this );
    cout << "================================================" << endl;
}

// =======================================================================

/*
 * GtlSalMenuSection
 */

GtkSalMenuSection::~GtkSalMenuSection()
{
    maItems.clear();
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
    mpMenuItem( NULL )
{
    cout << __FUNCTION__ << "Type: " << pItemData->eType << endl;

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
