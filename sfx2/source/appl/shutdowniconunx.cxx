
#ifdef ENABLE_QUICKSTART_APPLET

#include <unotools/moduleoptions.hxx>

#include <unotools/dynamicmenuoptions.hxx>

#include <gtk/gtk.h>
#include <glib.h>
#include <eggtray/eggtrayicon.h>
#include <vos/mutex.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>
#include <sfx2/app.hxx>
#ifndef _SFX_APP_HRC
#include "app.hrc"
#endif
#ifndef __SHUTDOWNICON_HXX__
#define USE_APP_SHORTCUTS
#include "shutdownicon.hxx"
#endif

// Cut/paste from vcl/inc/svids.hrc
#define SV_ICON_SMALL_START                 25000

#define SV_ICON_ID_OFFICE                       1
#define SV_ICON_ID_TEXT                         2
#define SV_ICON_ID_SPREADSHEET                  4
#define SV_ICON_ID_DRAWING                      6
#define SV_ICON_ID_PRESENTATION                 8
#define SV_ICON_ID_DATABASE                    14
#define SV_ICON_ID_FORMULA                     15
#define SV_ICON_ID_TEMPLATE                    16

using namespace ::rtl;
using namespace ::osl;

static ResMgr *pVCLResMgr;
static EggTrayIcon *pTrayIcon;
static GtkWidget *pExitMenuItem = NULL;
static GtkWidget *pOpenMenuItem = NULL;

static void open_url_cb( GtkWidget *, gpointer data )
{
    ShutdownIcon::OpenURL( *(OUString *)data,
                           OUString( RTL_CONSTASCII_USTRINGPARAM( "_default" ) ) );
}

static void open_file_cb( GtkWidget * )
{
    if ( !ShutdownIcon::bModalMode )
        ShutdownIcon::FileOpen();
}

static void open_template_cb( GtkWidget * )
{
    if ( !ShutdownIcon::bModalMode )
        ShutdownIcon::FromTemplate();
}

static void systray_disable_cb()
{
    ShutdownIcon::SetAutostart( false );
    ShutdownIcon::terminateDesktop();
}

static void exit_quickstarter_cb( GtkWidget * )
{
    egg_tray_icon_cancel_message (pTrayIcon, 1 );
    ShutdownIcon::getInstance()->terminateDesktop();
    plugin_shutdown_sys_tray();
}

static void menu_deactivate_cb( GtkWidget *pMenu )
{
    gtk_menu_popdown( GTK_MENU( pMenu ) );
}

static GdkPixbuf * ResIdToPixbuf( sal_uInt16 nResId )
{
    ResId aResId( SV_ICON_SMALL_START + nResId, *pVCLResMgr );
    BitmapEx aIcon( aResId );
    Bitmap pInSalBitmap = aIcon.GetBitmap();
    AlphaMask pInSalAlpha = aIcon.GetAlpha();

    BitmapReadAccess* pSalBitmap = pInSalBitmap.AcquireReadAccess();
    BitmapReadAccess* pSalAlpha = pInSalAlpha.AcquireReadAccess();

    g_return_val_if_fail( pSalBitmap != NULL, NULL );

    Size aSize( pSalBitmap->Width(), pSalBitmap->Height() );
    g_return_val_if_fail( Size( pSalAlpha->Width(), pSalAlpha->Height() ) == aSize, NULL );

    int nX, nY;
    guchar *pPixbufData = ( guchar * )g_malloc( 4 * aSize.Width() * aSize.Height() );
    guchar *pDestData = pPixbufData;

    for( nY = 0; nY < pSalBitmap->Height(); nY++ )
    {
        for( nX = 0; nX < pSalBitmap->Width(); nX++ )
        {
            BitmapColor aPix;
            aPix = pSalBitmap->GetPixel( nY, nX );
            pDestData[0] = aPix.GetRed();
            pDestData[1] = aPix.GetGreen();
            pDestData[2] = aPix.GetBlue();
            if (pSalAlpha)
            {
                aPix = pSalAlpha->GetPixel( nY, nX );
                pDestData[3] = 255 - aPix.GetIndex();
            }
            else
                pDestData[3] = 255;
            pDestData += 4;
        }
    }

    pInSalBitmap.ReleaseAccess( pSalBitmap );
    if( pSalAlpha )
        pInSalAlpha.ReleaseAccess( pSalAlpha );

    return gdk_pixbuf_new_from_data( pPixbufData,
        GDK_COLORSPACE_RGB, sal_True, 8,
        aSize.Width(), aSize.Height(),
        aSize.Width() * 4,
        (GdkPixbufDestroyNotify) g_free,
        NULL );
}

extern "C" {
static void oustring_delete (gpointer  data,
                             GClosure * /* closure */)
{
    OUString *pURL = (OUString *) data;
    delete pURL;
}
}

static void add_item( GtkMenuShell *pMenuShell, const char *pAsciiURL,
                      OUString *pOverrideLabel,
                      sal_uInt16 nResId, GCallback pFnCallback )
{
    OUString *pURL = new OUString (OStringToOUString( pAsciiURL,
                                                      RTL_TEXTENCODING_UTF8 ));
    OString aLabel;
    if (pOverrideLabel)
        aLabel = OUStringToOString (*pOverrideLabel, RTL_TEXTENCODING_UTF8);
    else
    {
        ShutdownIcon *pShutdownIcon = ShutdownIcon::getInstance();
        aLabel = OUStringToOString (pShutdownIcon->GetUrlDescription( *pURL ),
                                    RTL_TEXTENCODING_UTF8);
    }

    GdkPixbuf *pPixbuf= ResIdToPixbuf( nResId );
    GtkWidget *pImage = gtk_image_new_from_pixbuf( pPixbuf );
    g_object_unref( G_OBJECT( pPixbuf ) );

    GtkWidget *pMenuItem = gtk_image_menu_item_new_with_label( aLabel );
    gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM( pMenuItem ), pImage );
    g_signal_connect_data( pMenuItem, "activate", pFnCallback, pURL,
                           oustring_delete, GConnectFlags(0));

    gtk_menu_shell_append( pMenuShell, pMenuItem );
}

// Unbelievably nasty
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

static void add_ugly_db_item( GtkMenuShell *pMenuShell, const char *pAsciiURL,
                              sal_uInt16 nResId, GCallback pFnCallback )
{
    SvtDynamicMenuOptions aOpt;
    Sequence < Sequence < PropertyValue > > aMenu = aOpt.GetMenu( E_NEWMENU );
    for ( sal_Int32 n=0; n<aMenu.getLength(); n++ )
    {
        ::rtl::OUString aURL;
        ::rtl::OUString aDescription;
        Sequence < PropertyValue >& aEntry = aMenu[n];
        for ( sal_Int32 m=0; m<aEntry.getLength(); m++ )
        {
            if ( aEntry[m].Name.equalsAsciiL( "URL", 3 ) )
                aEntry[m].Value >>= aURL;
            if ( aEntry[m].Name.equalsAsciiL( "Title", 5 ) )
                aEntry[m].Value >>= aDescription;
        }

        if ( aURL.equalsAscii( BASE_URL ) && aDescription.getLength() )
        {
            add_item (pMenuShell, pAsciiURL, &aDescription, nResId, pFnCallback);
            break;
        }
    }
}

static GtkWidget *
add_image_menu_item( GtkMenuShell *pMenuShell,
                     const gchar *stock_id,
                     rtl::OUString aLabel,
                     GCallback     activate_cb )
{
    OString aUtfLabel = rtl::OUStringToOString (aLabel, RTL_TEXTENCODING_UTF8 );

    GtkWidget *pImage;
    pImage = gtk_image_new_from_stock( stock_id, GTK_ICON_SIZE_MENU );

    GtkWidget *pMenuItem;
    pMenuItem = gtk_image_menu_item_new_with_label( aUtfLabel );
    gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM( pMenuItem ), pImage );

    gtk_menu_shell_append( pMenuShell, pMenuItem );
    g_signal_connect( pMenuItem, "activate", activate_cb, NULL);

    return pMenuItem;
}

static void populate_menu( GtkWidget *pMenu )
{
    ShutdownIcon *pShutdownIcon = ShutdownIcon::getInstance();
    GtkMenuShell *pMenuShell = GTK_MENU_SHELL( pMenu );
    SvtModuleOptions aModuleOptions;

    if ( aModuleOptions.IsWriter() )
        add_item (pMenuShell, WRITER_URL, NULL,
                  SV_ICON_ID_TEXT, G_CALLBACK( open_url_cb ));

    if ( aModuleOptions.IsCalc() )
        add_item (pMenuShell, CALC_URL, NULL,
                  SV_ICON_ID_SPREADSHEET, G_CALLBACK( open_url_cb ));

    if ( aModuleOptions.IsImpress() )
        add_item (pMenuShell, IMPRESS_URL, NULL,
                  SV_ICON_ID_PRESENTATION, G_CALLBACK( open_url_cb ));

    if ( aModuleOptions.IsDraw() )
        add_item (pMenuShell, DRAW_URL, NULL,
                  SV_ICON_ID_DRAWING, G_CALLBACK( open_url_cb ));

    if ( aModuleOptions.IsDataBase() )
        add_ugly_db_item (pMenuShell, BASE_URL,
                          SV_ICON_ID_DATABASE, G_CALLBACK( open_url_cb ));

    if ( aModuleOptions.IsMath() )
        add_item (pMenuShell, MATH_URL, NULL,
                  SV_ICON_ID_FORMULA, G_CALLBACK( open_url_cb ));

    OUString aULabel = pShutdownIcon->GetResString( STR_QUICKSTART_FROMTEMPLATE );
    add_item (pMenuShell, "dummy", &aULabel,
              SV_ICON_ID_TEMPLATE, G_CALLBACK( open_template_cb ));

    OString aLabel;
    GtkWidget *pMenuItem;

    pMenuItem = gtk_separator_menu_item_new();
    gtk_menu_shell_append( pMenuShell, pMenuItem );

    pOpenMenuItem = add_image_menu_item
        (pMenuShell, GTK_STOCK_OPEN,
         pShutdownIcon->GetResString( STR_QUICKSTART_FILEOPEN ),
         G_CALLBACK( open_file_cb ));

    pMenuItem = gtk_separator_menu_item_new();
    gtk_menu_shell_append( pMenuShell, pMenuItem );

    (void) add_image_menu_item
        ( pMenuShell, GTK_STOCK_CLOSE,
          pShutdownIcon->GetResString( STR_QUICKSTART_PRELAUNCH_UNX ),
          G_CALLBACK( systray_disable_cb ) );

    pMenuItem = gtk_separator_menu_item_new();
    gtk_menu_shell_append( pMenuShell, pMenuItem );

    pExitMenuItem = add_image_menu_item
        ( pMenuShell, GTK_STOCK_QUIT,
          pShutdownIcon->GetResString( STR_QUICKSTART_EXIT ),
          G_CALLBACK( exit_quickstarter_cb ) );

    gtk_widget_show_all( pMenu );
}

static void refresh_menu( GtkWidget *pMenu )
{
    if (!pExitMenuItem)
        populate_menu( pMenu );

    bool bModal = ShutdownIcon::bModalMode;
    gtk_widget_set_sensitive( pExitMenuItem, !bModal);
    gtk_widget_set_sensitive( pOpenMenuItem, !bModal);
}

extern "C" {
static void
layout_menu( GtkMenu *menu,
             gint *x, gint *y, gboolean *push_in,
             gpointer )
{
    GtkRequisition req;
    GtkWidget *ebox = GTK_BIN( pTrayIcon )->child;

    gtk_widget_size_request( GTK_WIDGET( menu ), &req );
    gdk_window_get_origin( ebox->window, x, y );

    (*x) += ebox->allocation.x;
    (*y) += ebox->allocation.y;

    if (*y >= gdk_screen_get_height (gtk_widget_get_screen (ebox)) / 2)
        (*y) -= req.height;
    else
        (*y) += ebox->allocation.height;

    *push_in = sal_True;
}
}

static gboolean display_menu_cb( GtkWidget *,
                                 GdkEventButton *event, GtkWidget *pMenu )
{
    if (event->button == 2)
        return sal_False;

#ifdef TEMPLATE_DIALOG_MORE_POLISHED
    if (event->button == 1 &&
        event->type == GDK_2BUTTON_PRESS)
    {
        open_template_cb( NULL );
        return sal_True;
    }
    if (event->button == 3)
    {
        ... as below ...
#endif

    refresh_menu( pMenu );

    gtk_menu_popup( GTK_MENU( pMenu ), NULL, NULL,
                    layout_menu, NULL, 0, event->time );

    return sal_True;
}

extern "C" {
    static gboolean
    show_at_idle( gpointer )
    {
        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        gtk_widget_show_all( GTK_WIDGET( pTrayIcon ) );
        return sal_False;
    }
}

void SAL_DLLPUBLIC_EXPORT plugin_init_sys_tray()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if( !g_type_from_name( "GdkDisplay" ) )
        return;

    OString aLabel;
    ShutdownIcon *pShutdownIcon = ShutdownIcon::getInstance();

    aLabel = rtl::OUStringToOString (
            pShutdownIcon->GetResString( STR_QUICKSTART_TIP ),
            RTL_TEXTENCODING_UTF8 );

    pTrayIcon = egg_tray_icon_new( aLabel );

    GtkWidget *pParent = gtk_event_box_new();
    GtkTooltips *pTooltips = gtk_tooltips_new();
    gtk_tooltips_set_tip( GTK_TOOLTIPS( pTooltips ), pParent, aLabel, NULL );

    GtkWidget *pIconImage = gtk_image_new();
    gtk_container_add( GTK_CONTAINER( pParent ), pIconImage );

    pVCLResMgr = CREATEVERSIONRESMGR( vcl );

    GdkPixbuf *pPixbuf = ResIdToPixbuf( SV_ICON_ID_OFFICE );
    gtk_image_set_from_pixbuf( GTK_IMAGE( pIconImage ), pPixbuf );
    g_object_unref( pPixbuf );

    GtkWidget *pMenu = gtk_menu_new();
    g_signal_connect (pMenu, "deactivate",
                      G_CALLBACK (menu_deactivate_cb), NULL);
    g_signal_connect( pParent, "button_press_event",
                      G_CALLBACK( display_menu_cb ), pMenu );
    gtk_container_add( GTK_CONTAINER( pTrayIcon ), pParent );

    // Show at idle to avoid artefacts at startup
    g_idle_add (show_at_idle, (gpointer) pTrayIcon);

    // disable shutdown
    pShutdownIcon->SetVeto( true );
    pShutdownIcon->addTerminateListener();
}

void SAL_DLLPUBLIC_EXPORT plugin_shutdown_sys_tray()
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if( !pTrayIcon )
        return;
    gtk_widget_destroy( GTK_WIDGET( pTrayIcon ) );
    pTrayIcon = NULL;
    pExitMenuItem = NULL;
    pOpenMenuItem = NULL;
}

#endif // ENABLE_QUICKSTART_APPLET
