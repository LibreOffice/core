/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <functional>

#include <unx/gensys.h>
#include <unx/sessioninhibitor.hxx>

#if USING_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#if !defined(__sun)
#include <X11/extensions/dpms.h>
#endif
#endif

#include <config_gio.h>

#if ENABLE_GIO
#include <gio/gio.h>

#define FDOSS_DBUS_SERVICE        "org.freedesktop.ScreenSaver"
#define FDOSS_DBUS_PATH           "/org/freedesktop/ScreenSaver"
#define FDOSS_DBUS_INTERFACE      "org.freedesktop.ScreenSaver"

#define FDOPM_DBUS_SERVICE      "org.freedesktop.PowerManagement.Inhibit"
#define FDOPM_DBUS_PATH         "/org/freedesktop/PowerManagement/Inhibit"
#define FDOPM_DBUS_INTERFACE    "org.freedesktop.PowerManagement.Inhibit"

#define GSM_DBUS_SERVICE        "org.gnome.SessionManager"
#define GSM_DBUS_PATH           "/org/gnome/SessionManager"
#define GSM_DBUS_INTERFACE      "org.gnome.SessionManager"

#endif

#include <sal/log.hxx>

void SessionManagerInhibitor::inhibit(bool bInhibit, std::u16string_view sReason, ApplicationInhibitFlags eType,
                                      unsigned int window_system_id, const char* application_id)
{
    const char* appname = application_id ? application_id : SalGenericSystem::getFrameClassName();
    const OString aReason = OUStringToOString( sReason, RTL_TEXTENCODING_UTF8 );

    if (eType == APPLICATION_INHIBIT_IDLE)
    {
        inhibitFDOSS( bInhibit, appname, aReason.getStr() );
        inhibitFDOPM( bInhibit, appname, aReason.getStr() );
    }

    inhibitGSM(bInhibit, appname, aReason.getStr(), eType, window_system_id);

}


#if USING_X11
void SessionManagerInhibitor::inhibit(bool bInhibit, std::u16string_view sReason, ApplicationInhibitFlags eType,
                                      unsigned int window_system_id, std::optional<Display*> pDisplay,
                                      const char* application_id)
{
    inhibit(bInhibit, sReason, eType, window_system_id, application_id);

    if (eType == APPLICATION_INHIBIT_IDLE && pDisplay)
    {
        inhibitXScreenSaver( bInhibit, *pDisplay );
        inhibitXAutoLock( bInhibit, *pDisplay );
        inhibitDPMS( bInhibit, *pDisplay );
    }
}
#endif

#if ENABLE_GIO
static void dbusInhibit( bool bInhibit,
                  const gchar* service, const gchar* path, const gchar* interface,
                  const std::function<GVariant*( GDBusProxy*, GError*& )>& fInhibit,
                  const std::function<GVariant*( GDBusProxy*, const guint, GError*& )>& fUnInhibit,
                  std::optional<guint>& rCookie )
{
    if ( ( !bInhibit && !rCookie ) ||
         (  bInhibit &&  rCookie ) )
    {
        return;
    }

    GError          *error = nullptr;
    GDBusConnection *session_connection = g_bus_get_sync( G_BUS_TYPE_SESSION, nullptr, &error );
    if (session_connection == nullptr) {
        SAL_WARN( "vcl.sessioninhibitor", "failed to connect to dbus session bus" );

        if (error != nullptr) {
            SAL_WARN( "vcl.sessioninhibitor", "Error: " << error->message );
            g_error_free( error );
        }

        return;
    }

    GDBusProxy *proxy = g_dbus_proxy_new_sync( session_connection,
                                               G_DBUS_PROXY_FLAGS_NONE,
                                               nullptr,
                                               service,
                                               path,
                                               interface,
                                               nullptr,
                                               nullptr );

    g_object_unref( G_OBJECT( session_connection ) );

    if (proxy == nullptr) {
        SAL_INFO( "vcl.sessioninhibitor", "could not get dbus proxy: " << service );
        return;
    }

    GVariant *res = nullptr;

    if ( bInhibit )
    {
        res = fInhibit( proxy, error );

        if (res != nullptr)
        {
            guint nCookie;

            g_variant_get(res, "(u)", &nCookie);
            g_variant_unref(res);

            rCookie = nCookie;
        }
        else
        {
            SAL_INFO( "vcl.sessioninhibitor", service << ".Inhibit failed");
        }
    }
    else
    {
        res = fUnInhibit( proxy, *rCookie, error );
        rCookie.reset();

        if (res != nullptr)
        {
            g_variant_unref(res);
        }
        else
        {
            SAL_INFO( "vcl.sessioninhibitor", service << ".UnInhibit failed" );
        }
    }

    if (error != nullptr)
    {
        SAL_INFO( "vcl.sessioninhibitor", "Error: " << error->message );
        g_error_free( error );
    }

    g_object_unref( G_OBJECT( proxy ) );
}
#endif // ENABLE_GIO

void SessionManagerInhibitor::inhibitFDOSS( bool bInhibit, const char* appname, const char* reason )
{
#if ENABLE_GIO
    dbusInhibit( bInhibit,
                 FDOSS_DBUS_SERVICE, FDOSS_DBUS_PATH, FDOSS_DBUS_INTERFACE,
                 [appname, reason] ( GDBusProxy *proxy, GError*& error ) -> GVariant* {
                     return g_dbus_proxy_call_sync( proxy, "Inhibit",
                                                    g_variant_new("(ss)", appname, reason),
                                                    G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error );
                 },
                 [] ( GDBusProxy *proxy, const guint nCookie, GError*& error ) -> GVariant* {
                     return g_dbus_proxy_call_sync( proxy, "UnInhibit",
                                                    g_variant_new("(u)", nCookie),
                                                    G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error );
                 },
                 mnFDOSSCookie );
#else
    (void) this;
    (void) bInhibit;
    (void) appname;
    (void) reason;
#endif // ENABLE_GIO
}

void SessionManagerInhibitor::inhibitFDOPM( bool bInhibit, const char* appname, const char* reason )
{
#if ENABLE_GIO
    dbusInhibit( bInhibit,
                 FDOPM_DBUS_SERVICE, FDOPM_DBUS_PATH, FDOPM_DBUS_INTERFACE,
                 [appname, reason] ( GDBusProxy *proxy, GError*& error ) -> GVariant* {
                     return g_dbus_proxy_call_sync( proxy, "Inhibit",
                                                    g_variant_new("(ss)", appname, reason),
                                                    G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error );
                 },
                 [] ( GDBusProxy *proxy, const guint nCookie, GError*& error ) -> GVariant* {
                     return g_dbus_proxy_call_sync( proxy, "UnInhibit",
                                                    g_variant_new("(u)", nCookie),
                                                    G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error );
                 },
                 mnFDOPMCookie );
#else
    (void) this;
    (void) bInhibit;
    (void) appname;
    (void) reason;
#endif // ENABLE_GIO
}

void SessionManagerInhibitor::inhibitGSM( bool bInhibit, const char* appname, const char* reason, ApplicationInhibitFlags eType, unsigned int window_system_id )
{
#if ENABLE_GIO
    dbusInhibit( bInhibit,
                 GSM_DBUS_SERVICE, GSM_DBUS_PATH, GSM_DBUS_INTERFACE,
                 [appname, reason, eType, window_system_id] ( GDBusProxy *proxy, GError*& error ) -> GVariant* {
                     return g_dbus_proxy_call_sync( proxy, "Inhibit",
                                                    g_variant_new("(susu)",
                                                                  appname,
                                                                  window_system_id,
                                                                  reason,
                                                                  eType
                                                                 ),
                                                    G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error );
                 },
                 [] ( GDBusProxy *proxy, const guint nCookie, GError*& error ) -> GVariant* {
                     return g_dbus_proxy_call_sync( proxy, "Uninhibit",
                                                    g_variant_new("(u)", nCookie),
                                                    G_DBUS_CALL_FLAGS_NONE, -1, nullptr, &error );
                 },
                 mnGSMCookie );
#else
    (void) this;
    (void) bInhibit;
    (void) appname;
    (void) reason;
    (void) eType;
    (void) window_system_id;
#endif // ENABLE_GIO
}

#if USING_X11
/**
 * Disable screensavers using the XSetScreenSaver/XGetScreenSaver API.
 *
 * Worth noting: xscreensaver explicitly ignores this and does its own
 * timeout handling.
 */
void SessionManagerInhibitor::inhibitXScreenSaver( bool bInhibit, Display* pDisplay )
{
    int nTimeout, nInterval, bPreferBlanking, bAllowExposures;
    XGetScreenSaver( pDisplay, &nTimeout, &nInterval,
                     &bPreferBlanking, &bAllowExposures );

    // To disable/reenable we simply fiddle the timeout, whilst
    // retaining all other properties.
    if ( bInhibit && nTimeout)
    {
        mnXScreenSaverTimeout = nTimeout;
        XResetScreenSaver( pDisplay );
        XSetScreenSaver( pDisplay, 0, nInterval,
                         bPreferBlanking, bAllowExposures );
    }
    else if ( !bInhibit && mnXScreenSaverTimeout )
    {
        XSetScreenSaver( pDisplay, *mnXScreenSaverTimeout,
                         nInterval, bPreferBlanking,
                         bAllowExposures );
        mnXScreenSaverTimeout.reset();
    }
}


/* definitions from xautolock.c (pl15) */
#define XAUTOLOCK_DISABLE 1
#define XAUTOLOCK_ENABLE  2

void SessionManagerInhibitor::inhibitXAutoLock( bool bInhibit, Display* pDisplay )
{
    ::Window aRootWindow = RootWindowOfScreen( ScreenOfDisplay( pDisplay, 0 ) );

    Atom nAtom = XInternAtom( pDisplay,
                              "XAUTOLOCK_MESSAGE",
                              False );

    if ( nAtom == None )
    {
        return;
    }

    int nMessage = bInhibit ? XAUTOLOCK_DISABLE : XAUTOLOCK_ENABLE;

    XChangeProperty( pDisplay,
                     aRootWindow,
                     nAtom,
                     XA_INTEGER,
                     8, // format -- 8 bit quantity
                     PropModeReplace,
                     reinterpret_cast<unsigned char*>( &nMessage ),
                     sizeof( nMessage ) );
}

void SessionManagerInhibitor::inhibitDPMS( bool bInhibit, Display* pDisplay )
{
#if !defined(__sun)
    int dummy;
    // This won't change while X11 is running, hence
    // we can evaluate only once and store as static
    static bool bDPMSExtensionAvailable = ( DPMSQueryExtension( pDisplay, &dummy, &dummy) != 0 );

    if ( !bDPMSExtensionAvailable )
    {
        return;
    }

    if ( bInhibit )
    {
        CARD16 state; // unused by us
        DPMSInfo( pDisplay, &state, &mbDPMSWasEnabled );

        if ( mbDPMSWasEnabled )
        {
            DPMSGetTimeouts( pDisplay,
                             &mnDPMSStandbyTimeout,
                             &mnDPMSSuspendTimeout,
                             &mnDPMSOffTimeout );
            DPMSSetTimeouts( pDisplay,
                             0,
                             0,
                             0 );
        }
    }
    else if ( !bInhibit && mbDPMSWasEnabled )
    {
        DPMSSetTimeouts( pDisplay,
                         mnDPMSStandbyTimeout,
                         mnDPMSSuspendTimeout,
                         mnDPMSOffTimeout );
    }
#endif // !defined(__sun)
}
#endif // USING_X11

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
