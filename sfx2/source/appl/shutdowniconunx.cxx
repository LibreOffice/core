/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifdef ENABLE_QUICKSTART_APPLET

#include <unotools/moduleoptions.hxx>

#include <unotools/dynamicmenuoptions.hxx>

#include <gtk/gtk.h>
#include <glib.h>
#include <osl/mutex.hxx>
#include <osl/module.hxx>
#include "tools/rc.hxx"
#include <sfx2/app.hxx>
#include "app.hrc"
#include "shutdownicon.hxx"

#ifdef ENABLE_GIO
#include <gio/gio.h>
#endif

// Cut/paste from vcl/inc/svids.hrc
#define SV_ICON_ID_TEXT                         2
#define SV_ICON_ID_SPREADSHEET                  4
#define SV_ICON_ID_DRAWING                      6
#define SV_ICON_ID_PRESENTATION                 8
#define SV_ICON_ID_TEMPLATE                    11
#define SV_ICON_ID_DATABASE                    12
#define SV_ICON_ID_FORMULA                     13

using namespace ::osl;

extern "C" {

void SAL_DLLPUBLIC_EXPORT plugin_init_sys_tray();
void SAL_DLLPUBLIC_EXPORT plugin_shutdown_sys_tray();

}

static ResMgr *pVCLResMgr;
static GtkStatusIcon* pTrayIcon;
static GtkWidget *pExitMenuItem = nullptr;
static GtkWidget *pOpenMenuItem = nullptr;
static GtkWidget *pDisableMenuItem = nullptr;
#ifdef ENABLE_GIO
GFileMonitor* pMonitor = nullptr;
#endif

static void open_url_cb( GtkWidget *, gpointer data )
{
    ShutdownIcon::OpenURL( *static_cast<OUString *>(data),
                           "_default" );
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
    plugin_shutdown_sys_tray();
    //terminate may cause this .so to be unloaded. So we must be hands off
    //all calls into this .so after this call
    ShutdownIcon::terminateDesktop();
}

static void menu_deactivate_cb( GtkWidget *pMenu )
{
    gtk_menu_popdown( GTK_MENU( pMenu ) );
}

extern "C" {
static void oustring_delete (gpointer  data,
                             GClosure * /* closure */)
{
    OUString *pURL = static_cast<OUString *>(data);
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
        aLabel = OUStringToOString (ShutdownIcon::GetUrlDescription( *pURL ),
                                    RTL_TEXTENCODING_UTF8);
    }

    gchar* appicon;

    if (nResId == SV_ICON_ID_TEXT)
        appicon = g_strdup ("libreoffice-writer");
    else if (nResId == SV_ICON_ID_SPREADSHEET)
        appicon = g_strdup ("libreoffice-calc");
    else if (nResId == SV_ICON_ID_DRAWING)
        appicon = g_strdup ("libreoffice-draw");
    else if (nResId == SV_ICON_ID_PRESENTATION)
        appicon = g_strdup ("libreoffice-impress");
    else if (nResId == SV_ICON_ID_DATABASE)
        appicon = g_strdup ("libreoffice-base");
    else if (nResId == SV_ICON_ID_FORMULA)
        appicon = g_strdup ("libreoffice-math");
    else
        appicon = g_strdup ("libreoffice-main");

    GtkWidget *pImage = gtk_image_new_from_icon_name (appicon, GTK_ICON_SIZE_MENU);
    GtkWidget *pMenuItem = gtk_image_menu_item_new_with_label( aLabel.getStr() );
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
        OUString aURL;
        OUString aDescription;
        Sequence < PropertyValue >& aEntry = aMenu[n];
        for ( sal_Int32 m=0; m<aEntry.getLength(); m++ )
        {
            if ( aEntry[m].Name == "URL" )
                aEntry[m].Value >>= aURL;
            if ( aEntry[m].Name == "Title" )
                aEntry[m].Value >>= aDescription;
        }

        if ( aURL == BASE_URL && !aDescription.isEmpty() )
        {
            add_item (pMenuShell, pAsciiURL, &aDescription, nResId, pFnCallback);
            break;
        }
    }
}

static GtkWidget *
add_image_menu_item( GtkMenuShell *pMenuShell,
                     const gchar *stock_id,
                     const OUString& aLabel,
                     GCallback     activate_cb )
{
    OString aUtfLabel = OUStringToOString (aLabel, RTL_TEXTENCODING_UTF8 );

    GtkWidget *pImage;
    pImage = gtk_image_new_from_stock( stock_id, GTK_ICON_SIZE_MENU );

    GtkWidget *pMenuItem;
    pMenuItem = gtk_image_menu_item_new_with_label( aUtfLabel.getStr() );
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
        add_item (pMenuShell, WRITER_URL, nullptr,
                  SV_ICON_ID_TEXT, G_CALLBACK( open_url_cb ));

    if ( aModuleOptions.IsCalc() )
        add_item (pMenuShell, CALC_URL, nullptr,
                  SV_ICON_ID_SPREADSHEET, G_CALLBACK( open_url_cb ));

    if ( aModuleOptions.IsImpress() )
        add_item (pMenuShell, IMPRESS_URL, nullptr,
                  SV_ICON_ID_PRESENTATION, G_CALLBACK( open_url_cb ));

    if ( aModuleOptions.IsDraw() )
        add_item (pMenuShell, DRAW_URL, nullptr,
                  SV_ICON_ID_DRAWING, G_CALLBACK( open_url_cb ));

    if ( aModuleOptions.IsDataBase() )
        add_ugly_db_item (pMenuShell, BASE_URL,
                          SV_ICON_ID_DATABASE, G_CALLBACK( open_url_cb ));

    if ( aModuleOptions.IsMath() )
        add_item (pMenuShell, MATH_URL, nullptr,
                  SV_ICON_ID_FORMULA, G_CALLBACK( open_url_cb ));

    OUString aULabel = pShutdownIcon->GetResString( STR_QUICKSTART_FROMTEMPLATE );
    add_item (pMenuShell, "dummy", &aULabel,
              SV_ICON_ID_TEMPLATE, G_CALLBACK( open_template_cb ));

    GtkWidget *pMenuItem;

    pMenuItem = gtk_separator_menu_item_new();
    gtk_menu_shell_append( pMenuShell, pMenuItem );

    pOpenMenuItem = add_image_menu_item
        (pMenuShell, GTK_STOCK_OPEN,
         pShutdownIcon->GetResString( STR_QUICKSTART_FILEOPEN ),
         G_CALLBACK( open_file_cb ));


    pMenuItem = gtk_separator_menu_item_new();
    gtk_menu_shell_append( pMenuShell, pMenuItem );

    pDisableMenuItem = add_image_menu_item
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
    gtk_widget_set_sensitive( pDisableMenuItem, !bModal);
}

static gboolean display_menu_cb( GtkWidget *,
                                 GdkEventButton *event, GtkWidget *pMenu )
{
    if (event->button == 2)
        return sal_False;

    refresh_menu( pMenu );

    gtk_menu_popup( GTK_MENU( pMenu ), nullptr, nullptr,
                    gtk_status_icon_position_menu, pTrayIcon,
                    0, event->time );

    return sal_True;
}

#ifdef ENABLE_GIO
/*
 * If the quickstarter is running, then LibreOffice is
 * upgraded, then the old quickstarter is still running, but is now unreliable
 * as the old install has been deleted. A fairly intractable problem but we
 * can avoid much of the pain if we turn off the quickstarter if we detect
 * that it has been physically deleted or overwritten
*/
static void notify_file_changed(GFileMonitor * /*gfilemonitor*/, GFile * /*arg1*/,
    GFile * /*arg2*/, GFileMonitorEvent event_type, gpointer /*user_data*/)
{
    //Shutdown the quick starter if anything has happened to make it unsafe
    //to remain running, e.g. rpm --erased and all libs deleted, or
    //rpm --upgrade and libs being overwritten
    switch (event_type)
    {
        case G_FILE_MONITOR_EVENT_DELETED:
        case G_FILE_MONITOR_EVENT_CREATED:
        case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
        case G_FILE_MONITOR_EVENT_UNMOUNTED:
            exit_quickstarter_cb(GTK_WIDGET(pTrayIcon));
            break;
        default:
            break;
    }
}
#endif

void plugin_init_sys_tray()
{
    ::SolarMutexGuard aGuard;

    /* we need the vcl plugin and mainloop initialized */
    if (!g_type_from_name( "GdkDisplay" ))
        return;

    OString aLabel;
    ShutdownIcon *pShutdownIcon = ShutdownIcon::getInstance();

    aLabel = OUStringToOString (
            pShutdownIcon->GetResString( STR_QUICKSTART_TIP ),
            RTL_TEXTENCODING_UTF8 );

    pVCLResMgr = ResMgr::CreateResMgr("vcl");

    pTrayIcon = gtk_status_icon_new_from_icon_name ("libreoffice-main");

    g_object_set (pTrayIcon, "title", aLabel.getStr(),
                  "tooltip_text", aLabel.getStr(), NULL);

    GtkWidget *pMenu = gtk_menu_new();
    g_signal_connect(pTrayIcon,  "button-press-event",
                     G_CALLBACK(display_menu_cb), pMenu);
    g_signal_connect (pMenu, "deactivate",
                      G_CALLBACK (menu_deactivate_cb), NULL);

    // disable shutdown
    pShutdownIcon->SetVeto( true );
    ShutdownIcon::addTerminateListener();

#ifdef ENABLE_GIO
    GFile* pFile = nullptr;
    OUString sLibraryFileUrl;
    if (osl::Module::getUrlFromAddress(plugin_init_sys_tray, sLibraryFileUrl))
        pFile = g_file_new_for_uri(OUStringToOString(sLibraryFileUrl, RTL_TEXTENCODING_UTF8).getStr());

    if (pFile)
    {
        if ((pMonitor = g_file_monitor_file(pFile, G_FILE_MONITOR_NONE, nullptr, nullptr)))
            g_signal_connect(pMonitor, "changed", reinterpret_cast<GCallback>(notify_file_changed), NULL);
        g_object_unref(pFile);
    }
#endif
}

void plugin_shutdown_sys_tray()
{
    ::SolarMutexGuard aGuard;
    if( !pTrayIcon )
        return;

#ifdef ENABLE_GIO
    if (pMonitor)
    {
        g_signal_handlers_disconnect_by_func(pMonitor,
            reinterpret_cast<gpointer>(&notify_file_changed), pMonitor);
        g_file_monitor_cancel(pMonitor);
        g_object_unref(pMonitor);
        pMonitor = nullptr;
    }
#endif

    g_object_unref(pTrayIcon);
    pTrayIcon = nullptr;

    pExitMenuItem = nullptr;
    pOpenMenuItem = nullptr;
    pDisableMenuItem = nullptr;
}

#endif // ENABLE_QUICKSTART_APPLET

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
