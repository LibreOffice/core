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


#include <prex.h>
#include <X11/Xatom.h>
#include <postx.h>

#include <unx/desktops.hxx>

#include "rtl/process.h"
#include "rtl/ustrbuf.hxx"
#include "osl/module.h"
#include "osl/thread.h"
#include "vcl/svapp.hxx"

#include "vclpluginapi.h"

#include <unistd.h>
#include <string.h>


static bool is_gnome_desktop( Display* pDisplay )
{
    bool ret = false;

    // warning: these checks are coincidental, GNOME does not
    // explicitly advertise itself
    if ( NULL != getenv( "GNOME_DESKTOP_SESSION_ID" ) )
        ret = true;

    if( ! ret )
    {
        Atom nAtom1 = XInternAtom( pDisplay, "GNOME_SM_PROXY", True );
        Atom nAtom2 = XInternAtom( pDisplay, "NAUTILUS_DESKTOP_WINDOW_ID", True );
        if( nAtom1 || nAtom2 )
        {
            int nProperties = 0;
            Atom* pProperties = XListProperties( pDisplay, DefaultRootWindow( pDisplay ), &nProperties );
            if( pProperties && nProperties )
            {
                for( int i = 0; i < nProperties; i++ )
                    if( pProperties[ i ] == nAtom1 ||
                        pProperties[ i ] == nAtom2 )
                {
                    ret = true;
                }
                XFree( pProperties );
            }
        }
    }

    if( ! ret )
    {
        Atom nUTFAtom       = XInternAtom( pDisplay, "UTF8_STRING", True );
        Atom nNetWMNameAtom = XInternAtom( pDisplay, "_NET_WM_NAME", True );
        if( nUTFAtom && nNetWMNameAtom )
        {
            // another, more expensive check: search for a gnome-panel
            XLIB_Window aRoot, aParent, *pChildren = NULL;
            unsigned int nChildren = 0;
            XQueryTree( pDisplay, DefaultRootWindow( pDisplay ),
                        &aRoot, &aParent, &pChildren, &nChildren );
            if( pChildren && nChildren )
            {
                for( unsigned int i = 0; i < nChildren && ! ret; i++ )
                {
                    Atom nType = None;
                    int nFormat = 0;
                    unsigned long nItems = 0, nBytes = 0;
                    unsigned char* pProp = NULL;
                    XGetWindowProperty( pDisplay,
                                        pChildren[i],
                                        nNetWMNameAtom,
                                        0, 8,
                                        False,
                                        nUTFAtom,
                                        &nType,
                                        &nFormat,
                                        &nItems,
                                        &nBytes,
                                        &pProp );
                    if( pProp && nType == nUTFAtom )
                    {
                        OString aWMName( (sal_Char*)pProp );
                        if (
                            (aWMName.equalsIgnoreAsciiCase("gnome-shell")) ||
                            (aWMName.equalsIgnoreAsciiCase("gnome-panel"))
                           )
                        {
                            ret = true;
                        }
                    }
                    if( pProp )
                        XFree( pProp );
                }
                XFree( pChildren );
            }
        }
    }

    return ret;
}

static bool bWasXError = false;

static inline bool WasXError()
{
    bool bRet = bWasXError;
    bWasXError = false;
    return bRet;
}

extern "C"
{
    static int autodect_error_handler( Display*, XErrorEvent* )
    {
        bWasXError = true;
        return 0;
    }

    typedef int(* XErrorHandler)(Display*,XErrorEvent*);
}

static int TDEVersion( Display* pDisplay )
{
    int nRet = 0;

    Atom nFullSession = XInternAtom( pDisplay, "TDE_FULL_SESSION", True );
    Atom nTDEVersion  = XInternAtom( pDisplay, "TDE_SESSION_VERSION", True );

    if( nFullSession )
    {
        if( !nTDEVersion )
            return 14;

        Atom                aRealType   = None;
        int                 nFormat     = 8;
        unsigned long       nItems      = 0;
        unsigned long       nBytesLeft  = 0;
        unsigned char*  pProperty   = NULL;
        XGetWindowProperty( pDisplay,
                            DefaultRootWindow( pDisplay ),
                            nTDEVersion,
                            0, 1,
                            False,
                            AnyPropertyType,
                            &aRealType,
                            &nFormat,
                            &nItems,
                            &nBytesLeft,
                            &pProperty );
        if( !WasXError() && nItems != 0 && pProperty )
        {
            nRet = *reinterpret_cast< sal_Int32* >( pProperty );
        }
        if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }
    }
    return nRet;
}

static int KDEVersion( Display* pDisplay )
{
    int nRet = 0;

    Atom nFullSession = XInternAtom( pDisplay, "KDE_FULL_SESSION", True );
    Atom nKDEVersion  = XInternAtom( pDisplay, "KDE_SESSION_VERSION", True );

    if( nFullSession )
    {
        if( !nKDEVersion )
            return 3;

        Atom                aRealType   = None;
        int                 nFormat     = 8;
        unsigned long       nItems      = 0;
        unsigned long       nBytesLeft  = 0;
        unsigned char*  pProperty   = NULL;
        XGetWindowProperty( pDisplay,
                            DefaultRootWindow( pDisplay ),
                            nKDEVersion,
                            0, 1,
                            False,
                            AnyPropertyType,
                            &aRealType,
                            &nFormat,
                            &nItems,
                            &nBytesLeft,
                            &pProperty );
        if( !WasXError() && nItems != 0 && pProperty )
        {
            nRet = *reinterpret_cast< sal_Int32* >( pProperty );
        }
        if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }
    }
    return nRet;
}

static bool is_tde_desktop( Display* pDisplay )
{
    if ( NULL != getenv( "TDE_FULL_SESSION" ) )
    {
        return true; // TDE
    }

    if ( TDEVersion( pDisplay ) >= 14 )
        return true;

    return false;
}

static bool is_kde_desktop( Display* pDisplay )
{
    if ( NULL != getenv( "KDE_FULL_SESSION" ) )
    {
        const char *pVer = getenv( "KDE_SESSION_VERSION" );
        if ( !pVer || pVer[0] == '0' )
        {
            return true; // does not exist => KDE3
        }

        OUString aVer( "3" );
        if ( aVer.equalsIgnoreAsciiCaseAscii( pVer ) )
        {
            return true;
        }
    }

    if ( KDEVersion( pDisplay ) == 3 )
        return true;

    return false;
}

static bool is_kde4_desktop( Display* pDisplay )
{
    if ( NULL != getenv( "KDE_FULL_SESSION" ) )
    {
        OUString aVer( "4" );

        const char *pVer = getenv( "KDE_SESSION_VERSION" );
        if ( pVer && aVer.equalsIgnoreAsciiCaseAscii( pVer ) )
            return true;
    }

    if ( KDEVersion( pDisplay ) == 4 )
        return true;

    return false;
}

extern "C"
{

DESKTOP_DETECTOR_PUBLIC DesktopType get_desktop_environment()
{
    static const char *pOverride = getenv( "OOO_FORCE_DESKTOP" );

    if ( pOverride && *pOverride )
    {
        OString aOver( pOverride );

        if ( aOver.equalsIgnoreAsciiCase( "tde" ) )
            return DESKTOP_TDE;
        if ( aOver.equalsIgnoreAsciiCase( "kde4" ) )
            return DESKTOP_KDE4;
        if ( aOver.equalsIgnoreAsciiCase( "gnome" ) )
            return DESKTOP_GNOME;
        if ( aOver.equalsIgnoreAsciiCase( "xfce" ) )
            return DESKTOP_XFCE;
        if ( aOver.equalsIgnoreAsciiCase( "mate" ) )
            return DESKTOP_MATE;
        if ( aOver.equalsIgnoreAsciiCase( "kde" ) )
            return DESKTOP_KDE;
        if ( aOver.equalsIgnoreAsciiCase( "none" ) )
            return DESKTOP_UNKNOWN;
    }

    // get display to connect to
    const char* pDisplayStr = getenv( "DISPLAY" );

    const char* pUsePlugin = getenv( "SAL_USE_VCLPLUGIN" );

    if ((pUsePlugin && (strcmp(pUsePlugin, "svp") == 0))
        || Application::IsHeadlessModeRequested())
        pDisplayStr = NULL;
    else
    {
        int nParams = rtl_getAppCommandArgCount();
        OUString aParam;
        OString aBParm;
        for( int i = 0; i < nParams; i++ )
        {
            rtl_getAppCommandArg( i, &aParam.pData );
            if( i < nParams-1 && (aParam == "-display" || aParam == "--display" ) )
            {
                rtl_getAppCommandArg( i+1, &aParam.pData );
                aBParm = OUStringToOString( aParam, osl_getThreadTextEncoding() );
                pDisplayStr = aBParm.getStr();
                break;
            }
        }
    }

    // no server at all
    if( ! pDisplayStr || !*pDisplayStr )
        return DESKTOP_NONE;

    /* #i92121# workaround deadlocks in the X11 implementation
    */
    static const char* pNoXInitThreads = getenv( "SAL_NO_XINITTHREADS" );
    /* #i90094#
       from now on we know that an X connection will be
       established, so protect X against itself
    */
    if( ! ( pNoXInitThreads && *pNoXInitThreads ) )
        XInitThreads();

    Display* pDisplay = XOpenDisplay( pDisplayStr );
    if( pDisplay == NULL )
        return DESKTOP_NONE;

    DesktopType ret;

    XErrorHandler pOldHdl = XSetErrorHandler( autodect_error_handler );

    const char *pSession;
    OString aDesktopSession;

    if ( ( pSession = getenv( "DESKTOP_SESSION" ) ) )
        aDesktopSession = OString( pSession, strlen( pSession ) );

    // fast environment variable checks
    if ( aDesktopSession.equalsIgnoreAsciiCase( "gnome" ) )
        ret = DESKTOP_GNOME;
    else if ( aDesktopSession.equalsIgnoreAsciiCase( "mate" ) )
        ret = DESKTOP_MATE;
    else if ( aDesktopSession.equalsIgnoreAsciiCase( "xfce" ) )
        ret = DESKTOP_XFCE;

    // these guys can be slower, with X property fetches,
    // round-trips etc. and so are done later.
    else if ( is_kde4_desktop( pDisplay ) )
        ret = DESKTOP_KDE4;
    else if ( is_gnome_desktop( pDisplay ) )
        ret = DESKTOP_GNOME;
    else if ( is_kde_desktop( pDisplay ) )
        ret = DESKTOP_KDE;
    else if ( is_tde_desktop( pDisplay ) )
        ret = DESKTOP_TDE;
    else
        ret = DESKTOP_UNKNOWN;

    // set the default handler again
    XSetErrorHandler( pOldHdl );

    XCloseDisplay( pDisplay );

    return ret;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
