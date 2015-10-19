/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <generic/gensys.h>
#include <unx/screensaverinhibitor.hxx>

#ifdef ENABLE_DBUS
#include <dbus/dbus-glib.h>

#define FDO_DBUS_SERVICE        "org.freedesktop.ScreenSaver"
#define FDO_DBUS_PATH           "/org/freedesktop/ScreenSaver"
#define FDO_DBUS_INTERFACE      "org.freedesktop.ScreenSaver"

#define GSM_DBUS_SERVICE        "org.gnome.SessionManager"
#define GSM_DBUS_PATH           "/org/gnome/SessionManager"
#define GSM_DBUS_INTERFACE      "org.gnome.SessionManager"
#endif

#include <sal/log.hxx>

void ScreenSaverInhibitor::inhibit( bool bInhibit, const OUString& sReason, bool bIsX11, const boost::optional<guint> xid )
{
    const gchar* appname = SalGenericSystem::getFrameClassName();
    const OString aReason = OUStringToOString( sReason, RTL_TEXTENCODING_UTF8 );

    inhibitFDO( bInhibit, appname, aReason.getStr() );

    if ( bIsX11 && ( xid != boost::none ) )
    {
        inhibitGSM( bInhibit, appname, aReason.getStr(), xid.get() );
    }
}

void ScreenSaverInhibitor::inhibitFDO( bool bInhibit, const gchar* appname, const gchar* reason )
{
#ifdef ENABLE_DBUS
    if ( ( !bInhibit && ( mnFDOCookie == boost::none ) ) ||
         ( bInhibit && ( mnFDOCookie != boost::none ) ) )
    {
        return;
    }

    gboolean         res;
    GError          *error = NULL;
    DBusGProxy      *proxy = NULL;

    DBusGConnection *session_connection = dbus_g_bus_get( DBUS_BUS_SESSION, &error );
    if (error != NULL) {
        SAL_WARN( "vcl.screensaverinhibitor", "failed to connect to dbus session bus: " << error->message );
        g_error_free( error );
        return;
    }

    proxy = dbus_g_proxy_new_for_name( session_connection,
                                       FDO_DBUS_SERVICE,
                                       FDO_DBUS_PATH,
                                       FDO_DBUS_INTERFACE );
    if (proxy == NULL) {
        SAL_INFO( "vcl.screensaverinhibitor", "could not get dbus proxy: " FDO_DBUS_SERVICE );
        return;
    }

    if ( bInhibit )
    {
        guint nCookie;
        res = dbus_g_proxy_call( proxy,
                                 "Inhibit", &error,
                                 G_TYPE_STRING, appname,
                                 G_TYPE_STRING, reason,
                                 G_TYPE_INVALID,
                                 G_TYPE_UINT, &nCookie,
                                 G_TYPE_INVALID);
        if (res)
        {
            mnFDOCookie = nCookie;
        }
        else
        {
            SAL_INFO( "vcl.screensaverinhibitor", FDO_DBUS_SERVICE ".Inhibit failed");
        }
    }
    else
    {
        res = dbus_g_proxy_call (proxy,
                                 "UnInhibit",
                                 &error,
                                 G_TYPE_UINT, mnFDOCookie.get(),
                                 G_TYPE_INVALID,
                                 G_TYPE_INVALID);
        mnFDOCookie = boost::none;

        if (!res)
        {
            SAL_INFO( "vcl.screensaverinhibitor", FDO_DBUS_SERVICE ".UnInhibit failed" );
        }
    }

    if (error != NULL)
    {
        SAL_INFO( "vcl.screensaverinhibitor", "Error: " << error->message );
        g_error_free( error );
    }

    g_object_unref( G_OBJECT( proxy ) );

#endif // ENABLE_DBUS
}

void ScreenSaverInhibitor::inhibitGSM( bool bInhibit, const gchar* appname, const gchar* reason, const guint xid )
{
#ifdef ENABLE_DBUS
    if ( ( !bInhibit && ( mnGSMCookie == boost::none ) ) ||
         ( bInhibit && ( mnGSMCookie != boost::none ) ) )
    {
        return;
    }

    gboolean         res;
    GError          *error = NULL;
    DBusGProxy      *proxy = NULL;

    DBusGConnection *session_connection = dbus_g_bus_get( DBUS_BUS_SESSION, &error );
    if (error != NULL) {
        SAL_WARN( "vcl.screensaverinhibitor", "failed to connect to dbus session bus: " << error->message );
        g_error_free( error );
        return;
    }

    proxy = dbus_g_proxy_new_for_name( session_connection,
                                       GSM_DBUS_SERVICE,
                                       GSM_DBUS_PATH,
                                       GSM_DBUS_INTERFACE );
    if (proxy == NULL) {
        SAL_INFO( "vcl.screensaverinhibitor", "could not get dbus proxy: " GSM_DBUS_SERVICE );
        return;
    }

    if ( bInhibit )
    {
        guint nCookie;
        res = dbus_g_proxy_call (proxy,
                                 "Inhibit", &error,
                                 G_TYPE_STRING, appname,
                                 G_TYPE_UINT, xid,
                                 G_TYPE_STRING, reason,
                                 G_TYPE_UINT, 8, //Inhibit the session being marked as idle
                                 G_TYPE_INVALID,
                                 G_TYPE_UINT, &nCookie,
                                 G_TYPE_INVALID);
        if ( res )
        {
            mnGSMCookie = nCookie;
        }
        else
        {
            SAL_INFO( "vcl.screensaverinhibitor", GSM_DBUS_SERVICE ".Inhibit failed" );
        }
    }
    else
    {
        res = dbus_g_proxy_call (proxy,
                                 "Uninhibit",
                                 &error,
                                 G_TYPE_UINT, mnGSMCookie.get(),
                                 G_TYPE_INVALID,
                                 G_TYPE_INVALID);
        mnGSMCookie = boost::none;

        if ( !res )
        {
            SAL_INFO( "vcl.screensaverinhibitor", GSM_DBUS_SERVICE ".Uninhibit failed" );
        }
    }

    if (error != NULL)
    {
        SAL_INFO( "vcl.screensaverinhibitor", "Error: " << error->message );
        g_error_free( error );
    }

    g_object_unref( G_OBJECT( proxy ) );

#endif // ENABLE_DBUS
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
