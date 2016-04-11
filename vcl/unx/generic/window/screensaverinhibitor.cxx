/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unx/gensys.h>
#include <unx/screensaverinhibitor.hxx>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <config_dbus.h>

#if ENABLE_DBUS
#include <dbus/dbus-glib.h>

#define FDO_DBUS_SERVICE        "org.freedesktop.ScreenSaver"
#define FDO_DBUS_PATH           "/org/freedesktop/ScreenSaver"
#define FDO_DBUS_INTERFACE      "org.freedesktop.ScreenSaver"

#define FDOPM_DBUS_SERVICE      "org.freedesktop.PowerManagement.Inhibit"
#define FDOPM_DBUS_PATH         "/org/freedesktop/PowerManagement/Inhibit"
#define FDOPM_DBUS_INTERFACE    "org.freedesktop.PowerManagement.Inhibit"

#define GSM_DBUS_SERVICE        "org.gnome.SessionManager"
#define GSM_DBUS_PATH           "/org/gnome/SessionManager"
#define GSM_DBUS_INTERFACE      "org.gnome.SessionManager"

// Mate <= 1.10 uses org.mate.SessionManager, > 1.10 will use org.gnome.SessionManager
#define MSM_DBUS_SERVICE        "org.mate.SessionManager"
#define MSM_DBUS_PATH           "/org/mate/SessionManager"
#define MSM_DBUS_INTERFACE      "org.mate.SessionManager"
#endif

#include <sal/log.hxx>

void ScreenSaverInhibitor::inhibit( bool bInhibit, const OUString& sReason,
                                    bool bIsX11, const boost::optional<unsigned int>& xid, boost::optional<Display*> pDisplay )
{
    const char* appname = SalGenericSystem::getFrameClassName();
    const OString aReason = OUStringToOString( sReason, RTL_TEXTENCODING_UTF8 );

    inhibitFDO( bInhibit, appname, aReason.getStr() );
    inhibitFDOPM( bInhibit, appname, aReason.getStr() );

    if ( bIsX11 )
    {
        if ( pDisplay != boost::none )
        {
            inhibitXScreenSaver( bInhibit, pDisplay.get() );
            inhibitXAutoLock( bInhibit, pDisplay.get() );
            inhibitDPMS( bInhibit, pDisplay.get() );
        }

        if ( xid != boost::none )
        {
            inhibitGSM( bInhibit, appname, aReason.getStr(), xid.get() );
            inhibitMSM( bInhibit, appname, aReason.getStr(), xid.get() );
        }
    }
}

#if ENABLE_DBUS
void dbusInhibit( bool bInhibit,
                  const gchar* service, const gchar* path, const gchar* interface,
                  const std::function<bool( DBusGProxy*, guint&, GError*& )>& fInhibit,
                  const std::function<bool( DBusGProxy*, const guint, GError*& )>& fUnInhibit,
                  boost::optional<guint>& rCookie )
{
    if ( ( !bInhibit && ( rCookie == boost::none ) ) ||
         ( bInhibit && ( rCookie != boost::none ) ) )
    {
        return;
    }

    gboolean         res;
    GError          *error = nullptr;
    DBusGProxy      *proxy = nullptr;

    DBusGConnection *session_connection = dbus_g_bus_get( DBUS_BUS_SESSION, &error );
    if (error != nullptr) {
        SAL_WARN( "vcl.screensaverinhibitor", "failed to connect to dbus session bus: " << error->message );
        g_error_free( error );
        return;
    }

    proxy = dbus_g_proxy_new_for_name( session_connection,
                                       service,
                                       path,
                                       interface );
    if (proxy == nullptr) {
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

    if (error != nullptr)
    {
        SAL_INFO( "vcl.screensaverinhibitor", "Error: " << error->message );
        g_error_free( error );
    }

    g_object_unref( G_OBJECT( proxy ) );

}
#endif // ENABLE_DBUS

void ScreenSaverInhibitor::inhibitFDO( bool bInhibit, const char* appname, const char* reason )
{
#if ENABLE_DBUS
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
#else
    (void) this;
    (void) bInhibit;
    (void) appname;
    (void) reason;
#endif // ENABLE_DBUS
}

void ScreenSaverInhibitor::inhibitFDOPM( bool bInhibit, const char* appname, const char* reason )
{
#if ENABLE_DBUS
    dbusInhibit( bInhibit,
                 FDOPM_DBUS_SERVICE, FDOPM_DBUS_PATH, FDOPM_DBUS_INTERFACE,
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
                 mnFDOPMCookie );
#else
    (void) this;
    (void) bInhibit;
    (void) appname;
    (void) reason;
#endif // ENABLE_DBUS
}

void ScreenSaverInhibitor::inhibitGSM( bool bInhibit, const char* appname, const char* reason, const unsigned int xid )
{
#if ENABLE_DBUS
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
#else
    (void) this;
    (void) bInhibit;
    (void) appname;
    (void) reason;
    (void) xid;
#endif // ENABLE_DBUS
}

void ScreenSaverInhibitor::inhibitMSM( bool bInhibit, const char* appname, const char* reason, const unsigned int xid )
{
#if ENABLE_DBUS
    dbusInhibit( bInhibit,
                 MSM_DBUS_SERVICE, MSM_DBUS_PATH, MSM_DBUS_INTERFACE,
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
                 mnMSMCookie );
#else
    (void) this;
    (void) bInhibit;
    (void) appname;
    (void) reason;
    (void) xid;
#endif // ENABLE_DBUS
}

/**
 * Disable screensavers using the XSetScreenSaver/XGetScreenSaver API.
 *
 * Worth noting: xscreensaver explicitly ignores this and does it's own
 * timeout handling.
 */
void ScreenSaverInhibitor::inhibitXScreenSaver( bool bInhibit, Display* pDisplay )
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
    else if ( !bInhibit && ( mnXScreenSaverTimeout != boost::none ) )
    {
        XSetScreenSaver( pDisplay, mnXScreenSaverTimeout.get(),
                         nInterval, bPreferBlanking,
                         bAllowExposures );
        mnXScreenSaverTimeout = boost::none;
    }
}


/* definitions from xautolock.c (pl15) */
#define XAUTOLOCK_DISABLE 1
#define XAUTOLOCK_ENABLE  2

void ScreenSaverInhibitor::inhibitXAutoLock( bool bInhibit, Display* pDisplay )
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

void ScreenSaverInhibitor::inhibitDPMS( bool bInhibit, Display* pDisplay )
{
#if !defined(SOLARIS) && !defined(AIX)
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
#endif // !defined(SOLARIS) && !defined(AIX)
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
