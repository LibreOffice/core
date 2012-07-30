
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

void publishMenu( GtkSalMenu* pMenu )
{
    GActionGroup *group;
    GDBusConnection *bus;
    GError *error = NULL;
    gchar *path;
    guint id;

    if (pMenu->pSessionBus && pMenu->aDBusMenubarPath) {
        GSimpleActionGroup *simpleGroup = g_simple_action_group_new();
//        GSimpleAction *action = g_simple_action_new("quit" );
//        g_simple_action_group_insert( simpleGroup, G_ACTION( action ) );

        group = G_ACTION_GROUP( simpleGroup );


        g_print ("Exporting menus on the bus...\n");
//        path = g_strconcat (OBJ_PATH, "/menus/", "menubar", NULL);
        if (!g_dbus_connection_export_menu_model (pMenu->pSessionBus, pMenu->aDBusMenubarPath, pMenu->mpMenuModel, &error))
        {
            g_warning ("Menu export failed: %s", error->message);
            //        exit (1);
        }
        g_print ("Exporting actions on the bus...\n");
        if (!g_dbus_connection_export_action_group (pMenu->pSessionBus, OBJ_PATH, group, &error))
        {
            g_warning ("Action export failed: %s", error->message);
            //        exit (1);
        }

        g_bus_own_name_on_connection (pMenu->pSessionBus, BUS_NAME, G_BUS_NAME_OWNER_FLAGS_NONE, NULL, NULL, NULL, NULL);
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

GtkSalMenu::GtkSalMenu( sal_Bool bMenuBar ) :
    mbMenuBar( bMenuBar ),
    mpVCLMenu( NULL ),
    mpParentMenuModel( NULL ),
    aDBusMenubarPath( NULL ),
    pSessionBus( NULL )
{
    mpMenuModel = G_MENU_MODEL( g_menu_new() );

    if (bMenuBar) {
//        pSessionBus = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);

//        aDBusMenubarPath = g_strconcat (OBJ_PATH, "/menus/", "menubar", NULL);

//        GMenu *menu = G_MENU( mpMenuModel );

//        g_menu_append( menu, "Menu1", "app.quit");

//        publishMenu( this );
    }
}

GtkSalMenu::~GtkSalMenu()
{
    g_object_unref( mpMenuModel );
    g_object_unref( mpParentMenuModel );
}

sal_Bool GtkSalMenu::VisibleMenuBar()
{
    return sal_False;
}

void GtkSalMenu::InsertItem( SalMenuItem* pSalMenuItem, unsigned nPos )
{
    cout << __FUNCTION__ << endl;
}

void GtkSalMenu::RemoveItem( unsigned nPos )
{
    cout << __FUNCTION__ << endl;
}

void GtkSalMenu::SetSubMenu( SalMenuItem* pSalMenuItem, SalMenu* pSubMenu, unsigned nPos )
{
    cout << __FUNCTION__ << endl;
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

        GMenu *menu = g_menu_new ();
        //    g_menu_append (menu, "Add", "app.add");
        //    g_menu_append (menu, "Del", "app.del");
        g_menu_append (menu, "Quit", "app.quit");

        GDBusConnection *bus = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
        if(!bus) puts ("Fail bus get");
        guint bid = g_bus_own_name_on_connection (bus, "org.libreoffice", G_BUS_NAME_OWNER_FLAGS_NONE, NULL, NULL, NULL, NULL);
        if(!bid) puts ("Fail own name");

        guint appmenuID = g_dbus_connection_export_menu_model (bus, "/org/libreoffice/menus/appmenu", G_MENU_MODEL (menu), NULL);
        if(!appmenuID) puts("Fail export appmenu");
        guint menubarID = g_dbus_connection_export_menu_model (bus, "/org/libreoffice/menus/menubar", G_MENU_MODEL (menu), NULL);
        if(!menubarID) puts("Fail export menubar");

        g_object_unref (menu);
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
}

void GtkSalMenu::SetItemImage( unsigned nPos, SalMenuItem* pSalMenuItem, const Image& rImage)
{
    cout << __FUNCTION__ << endl;
}

void GtkSalMenu::SetAccelerator( unsigned nPos, SalMenuItem* pSalMenuItem, const KeyCode& rKeyCode, const rtl::OUString& rKeyName )
{
    cout << __FUNCTION__ << endl;
}

void GtkSalMenu::GetSystemMenuData( SystemMenuData* pData )
{
    cout << __FUNCTION__ << endl;
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
    mpMenuItem = g_menu_item_new("", NULL);
}

GtkSalMenuItem::~GtkSalMenuItem()
{
    g_object_unref( mpMenuItem );
}
