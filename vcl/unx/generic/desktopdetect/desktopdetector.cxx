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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <unx/desktops.hxx>

#include <rtl/bootstrap.hxx>
#include <rtl/process.h>
#include <rtl/ustrbuf.hxx>
#include <osl/module.h>
#include <osl/thread.h>

#include <vclpluginapi.h>

#include <unistd.h>
#include <string.h>
#include <comphelper/string.hxx>

static bool is_gnome_desktop( Display* pDisplay )
{

    // warning: these checks are coincidental, GNOME does not
    // explicitly advertise itself
    if ( getenv( "GNOME_DESKTOP_SESSION_ID" ) )
        return true;

    bool ret = false;

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
                    break;
                }
            XFree( pProperties );
        }
    }
    if (ret)
        return true;

    Atom nUTFAtom       = XInternAtom( pDisplay, "UTF8_STRING", True );
    Atom nNetWMNameAtom = XInternAtom( pDisplay, "_NET_WM_NAME", True );
    if( nUTFAtom && nNetWMNameAtom )
    {
        // another, more expensive check: search for a gnome-panel
        ::Window aRoot, aParent, *pChildren = nullptr;
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
                unsigned char* pProp = nullptr;
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
                    OString aWMName( reinterpret_cast<char*>(pProp) );
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

    return ret;
}


static bool is_kde5_desktop()
{
    static const char * pFullVersion = getenv( "KDE_FULL_SESSION" );
    static const char * pSessionVersion = getenv( "KDE_SESSION_VERSION" );
    if ( pFullVersion && pSessionVersion && strcmp(pSessionVersion, "5") == 0)
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

        if ( aOver.equalsIgnoreAsciiCase( "lxqt" ) )
            return DESKTOP_LXQT;
        if ( aOver.equalsIgnoreAsciiCase( "kde5" ) )
            return DESKTOP_KDE5;
        if ( aOver.equalsIgnoreAsciiCase( "gnome" ) )
            return DESKTOP_GNOME;
        if ( aOver.equalsIgnoreAsciiCase( "gnome-wayland" ) )
            return DESKTOP_GNOME;
        if ( aOver.equalsIgnoreAsciiCase( "unity" ) )
            return DESKTOP_UNITY;
        if ( aOver.equalsIgnoreAsciiCase( "xfce" ) )
            return DESKTOP_XFCE;
        if ( aOver.equalsIgnoreAsciiCase( "mate" ) )
            return DESKTOP_MATE;
        if ( aOver.equalsIgnoreAsciiCase( "none" ) )
            return DESKTOP_UNKNOWN;
    }

    OUString plugin;
    rtl::Bootstrap::get("SAL_USE_VCLPLUGIN", plugin);

    if (plugin == "svp")
        return DESKTOP_NONE;

    const char *pDesktop = getenv( "XDG_CURRENT_DESKTOP" );
    if ( pDesktop )
    {
        OString aCurrentDesktop = OString( pDesktop, strlen( pDesktop ) );

        //it may be separated by colon ( e.g. unity:unity7:ubuntu )
        std::vector<OUString> aSplitCurrentDesktop = comphelper::string::split(
                OStringToOUString( aCurrentDesktop, RTL_TEXTENCODING_UTF8), ':');
        for (auto& rCurrentDesktopStr : aSplitCurrentDesktop)
        {
            if ( rCurrentDesktopStr.equalsIgnoreAsciiCase( "unity" ) )
                return DESKTOP_UNITY;
            else if ( rCurrentDesktopStr.equalsIgnoreAsciiCase( "gnome" ) )
                return DESKTOP_GNOME;
            else if ( rCurrentDesktopStr.equalsIgnoreAsciiCase( "lxqt" ) )
                return DESKTOP_LXQT;
        }
    }

    const char *pSession = getenv( "DESKTOP_SESSION" );
    OString aDesktopSession;
    if ( pSession )
        aDesktopSession = OString( pSession, strlen( pSession ) );

    // fast environment variable checks
    if ( aDesktopSession.equalsIgnoreAsciiCase( "gnome" ) )
        return DESKTOP_GNOME;
    else if ( aDesktopSession.equalsIgnoreAsciiCase( "gnome-wayland" ) )
        return DESKTOP_GNOME;
    else if ( aDesktopSession.equalsIgnoreAsciiCase( "mate" ) )
        return DESKTOP_MATE;
    else if ( aDesktopSession.equalsIgnoreAsciiCase( "xfce" ) )
        return DESKTOP_XFCE;
    else if ( aDesktopSession.equalsIgnoreAsciiCase( "lxqt" ) )
        return DESKTOP_LXQT;


    if ( is_kde5_desktop() )
        return DESKTOP_KDE5;

    // tdf#121275 if we still can't tell, and WAYLAND_DISPLAY
    // is set, default to gtk3
    const char* pWaylandStr = getenv("WAYLAND_DISPLAY");
    if (pWaylandStr && *pWaylandStr)
        return DESKTOP_GNOME;

    // these guys can be slower, with X property fetches,
    // round-trips etc. and so are done later.

    // get display to connect to
    const char* pDisplayStr = getenv( "DISPLAY" );

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
    if( pDisplay == nullptr )
        return DESKTOP_NONE;

    DesktopType ret;
    if ( is_gnome_desktop( pDisplay ) )
        ret = DESKTOP_GNOME;
    else
        ret = DESKTOP_UNKNOWN;

    XCloseDisplay( pDisplay );

    return ret;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
