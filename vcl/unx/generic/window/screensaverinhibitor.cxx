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

void dbusInhibit( bool bInhibit,
                  const gchar* service, const gchar* path, const gchar* interface,
                  std::function<bool( DBusGProxy*, guint&, GError*& )> fInhibit,
                  std::function<bool( DBusGProxy*, const guint, GError*& )> fUnInhibit,
                  boost::optional<guint>& rCookie )
{
#ifdef ENABLE_DBUS
    if ( ( !bInhibit && ( rCookie == boost::none ) ) ||
         ( bInhibit && ( rCookie != boost::none ) ) )
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
                                       service,
                                       path,
                                       interface );
    if (proxy == NULL) {
        SAL_INFO( "vcl.screensaverinhibitor", "could not get dbus proxy: " << service );
        return;
    }

    if ( bInhibit )
    {
        guint nCookie;
        res = fInhibit( proxy, nCookie, error );

        if (res)
        {
            rCookie = nCookie;
        }
        else
        {
            SAL_INFO( "vcl.screensaverinhibitor", service << ".Inhibit failed");
        }
    }
    else
    {
        res = fUnInhibit( proxy, rCookie.get(), error );
        rCookie = boost::none;

        if (!res)
        {
            SAL_INFO( "vcl.screensaverinhibitor", service << ".UnInhibit failed" );
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

void ScreenSaverInhibitor::inhibitFDO( bool bInhibit, const gchar* appname, const gchar* reason )
{
    dbusInhibit( bInhibit,
                 FDO_DBUS_SERVICE, FDO_DBUS_PATH, FDO_DBUS_INTERFACE,
                 [appname, reason] ( DBusGProxy *proxy, guint& nCookie, GError*& error ) -> bool {
                     return dbus_g_proxy_call( proxy,
                                               "Inhibit", &error,
                                               G_TYPE_STRING, appname,
                                               G_TYPE_STRING, reason,
                                               G_TYPE_INVALID,
                                               G_TYPE_UINT, &nCookie,
                                               G_TYPE_INVALID );
                 },
                 [] ( DBusGProxy *proxy, const guint nCookie, GError*& error ) -> bool {
                     return dbus_g_proxy_call( proxy,
                                               "UnInhibit", &error,
                                               G_TYPE_UINT, nCookie,
                                               G_TYPE_INVALID,
                                               G_TYPE_INVALID );
                 },
                 mnFDOCookie );
}

void ScreenSaverInhibitor::inhibitGSM( bool bInhibit, const gchar* appname, const gchar* reason, const guint xid )
{
    dbusInhibit( bInhibit,
                 GSM_DBUS_SERVICE, GSM_DBUS_PATH, GSM_DBUS_INTERFACE,
                 [appname, reason, xid] ( DBusGProxy *proxy, guint& nCookie, GError*& error ) -> bool {
                     return dbus_g_proxy_call( proxy,
                                               "Inhibit", &error,
                                               G_TYPE_STRING, appname,
                                               G_TYPE_UINT, xid,
                                               G_TYPE_STRING, reason,
                                               G_TYPE_UINT, 8, //Inhibit the session being marked as idle
                                               G_TYPE_INVALID,
                                               G_TYPE_UINT, &nCookie,
                                               G_TYPE_INVALID );
                 },
                 [] ( DBusGProxy *proxy, const guint nCookie, GError*& error ) -> bool {
                     return dbus_g_proxy_call( proxy,
                                               "Uninhibit", &error,
                                               G_TYPE_UINT, nCookie,
                                               G_TYPE_INVALID,
                                               G_TYPE_INVALID );
                 },
                 mnGSMCookie );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
