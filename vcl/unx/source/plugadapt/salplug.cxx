/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salplug.cxx,v $
 * $Revision: 1.26 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef OSL_MODULE_H
#include <osl/module.h>
#endif
#ifndef OSL_PROCESS_H
#include <osl/process.h>
#endif
#ifndef RTL_STRING_HXX
#include <rtl/ustrbuf.hxx>
#endif

#include <svunx.h>
#include <prex.h>
#include <X11/Xatom.h>
#include <postx.h>

#include <vcl/salinst.hxx>
#include <saldata.hxx>

#include <cstdio>
#include <unistd.h>

using namespace rtl;

extern "C" {
typedef SalInstance*(*salFactoryProc)( oslModule pModule);
}

static oslModule pCloseModule = NULL;

#define DESKTOP_NONE 0
#define DESKTOP_UNKNOWN 1
#define DESKTOP_GNOME 2
#define DESKTOP_KDE 3
#define DESKTOP_CDE 4

static const char * desktop_strings[5] = { "none", "unknown", "GNOME", "KDE", "CDE" };

static SalInstance* tryInstance( const OUString& rModuleBase )
{
    SalInstance* pInst = NULL;

    OUStringBuffer aModName( 128 );
    aModName.appendAscii( SAL_DLLPREFIX"vclplug_" );
    aModName.append( rModuleBase );
    aModName.appendAscii( SAL_DLLPOSTFIX );
    aModName.appendAscii( SAL_DLLEXTENSION );
    OUString aModule = aModName.makeStringAndClear();

    oslModule aMod = osl_loadModuleRelative(
        reinterpret_cast< oslGenericFunction >( &tryInstance ), aModule.pData,
        SAL_LOADMODULE_DEFAULT );
    if( aMod )
    {
        OUString aSym( RTL_CONSTASCII_USTRINGPARAM( "create_SalInstance" ) );
        salFactoryProc aProc = (salFactoryProc)osl_getFunctionSymbol( aMod, aSym.pData );
        if( aProc )
        {
            pInst = aProc( aMod );
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "sal plugin %s produced instance %p\n",
                     OUStringToOString( aModule, RTL_TEXTENCODING_ASCII_US ).getStr(),
                     pInst );
#endif
            if( pInst )
            {
                pCloseModule = aMod;
                GetSalData()->m_pPlugin = aMod;
            }
            else
                osl_unloadModule( aMod );
        }
        else
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf( stderr, "could not load symbol %s from shared object %s\n",
                     OUStringToOString( aSym, RTL_TEXTENCODING_ASCII_US ).getStr(),
                     OUStringToOString( aModule, RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
            osl_unloadModule( aMod );
        }
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "could not load shared object %s\n",
                 OUStringToOString( aModule, RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif

    return pInst;
}

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
                        if( aWMName.equalsIgnoreAsciiCase( "gnome-panel" ) )
                            ret = true;
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

static OUString getNetWMName( Display* pDisplay )
{
    OUString aRet;

    Atom nWmCheck   = XInternAtom( pDisplay, "_NET_SUPPORTING_WM_CHECK", True );
    Atom nWmName    = XInternAtom( pDisplay, "_NET_WM_NAME", True );
    if( nWmName && nWmCheck )
    {
        XLIB_Window         aCheckWin   = None;
        Atom                aRealType   = None;
        int                 nFormat     = 8;
        unsigned long       nItems      = 0;
        unsigned long       nBytesLeft  = 0;
        unsigned char*  pProperty   = NULL;
        XGetWindowProperty( pDisplay,
                            DefaultRootWindow( pDisplay ),
                            nWmCheck,
                            0, 1,
                            False,
                            XA_WINDOW,
                            &aRealType,
                            &nFormat,
                            &nItems,
                            &nBytesLeft,
                            &pProperty );
        if( aRealType == XA_WINDOW && nFormat == 32 && nItems != 0 )
            aCheckWin = *(XLIB_Window*)pProperty;
        if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }

        // see if that window really exists and has the check property set
        if( aCheckWin != None )
        {
            // clear error flag
            WasXError();
            // get the property
            XGetWindowProperty( pDisplay,
                                aCheckWin,
                                nWmCheck,
                                0, 1,
                                False,
                                XA_WINDOW,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty );
            if( ! WasXError() && aRealType == XA_WINDOW && nFormat == 32 && nItems != 0 && pProperty )
            {
                if( aCheckWin == *(XLIB_Window*)pProperty )
                {
                    XFree( pProperty );
                    pProperty = NULL;
                    XGetWindowProperty( pDisplay,
                                        aCheckWin,
                                        nWmName,
                                        0, 256,
                                        False,
                                        AnyPropertyType,
                                        &aRealType,
                                        &nFormat,
                                        &nItems,
                                        &nBytesLeft,
                                        &pProperty );
                    if( !WasXError() && nItems != 0 && pProperty && *pProperty )
                    {
                        if( aRealType == XA_STRING ) // some WM's use this although the should use UTF8_STRING
                        {
                            aRet = rtl::OStringToOUString( rtl::OString( (sal_Char*)pProperty ), RTL_TEXTENCODING_ISO_8859_1 );
                        }
                        else
                            aRet = rtl::OStringToOUString( rtl::OString( (sal_Char*)pProperty ), RTL_TEXTENCODING_UTF8 );
                    }
                }
            }
            if( pProperty )
            {
                XFree( pProperty );
                pProperty = NULL;
            }
        }
    }
    return aRet;
}

static bool is_kde_desktop( Display* pDisplay )
{
    if ( NULL != getenv( "KDE_FULL_SESSION" ) )
        return true;

    // check for kwin
    rtl::OUString aWM = getNetWMName( pDisplay );
    if( aWM.equalsIgnoreAsciiCaseAscii( "KWin" ) )
        return true;

    return false;
}

static bool is_cde_desktop( Display* pDisplay )
{
    void* pLibrary = NULL;

    Atom nDtAtom = XInternAtom( pDisplay, "_DT_WM_READY", True );
    OUString aPathName( RTL_CONSTASCII_USTRINGPARAM( "file:///usr/dt/lib/libDtSvc.so" ) );
    if( nDtAtom && ( pLibrary = osl_loadModule( aPathName.pData, SAL_LOADMODULE_DEFAULT ) ) )
    {
        osl_unloadModule( (oslModule)pLibrary );
        return true;
    }

    return false;
}


static const char * get_desktop_environment()
{
    static const char *pRet = NULL;
    static const char *pOverride = getenv( "OOO_FORCE_DESKTOP" );

    if ( pOverride && *pOverride )
    {
        OString aOver( pOverride );

        if ( aOver.equalsIgnoreAsciiCase( "cde" ) )
            pRet = desktop_strings[DESKTOP_CDE];
        if ( aOver.equalsIgnoreAsciiCase( "kde" ) )
            pRet = desktop_strings[DESKTOP_KDE];
        if ( aOver.equalsIgnoreAsciiCase( "gnome" ) )
            pRet = desktop_strings[DESKTOP_GNOME];
        if ( aOver.equalsIgnoreAsciiCase( "none" ) )
            pRet = desktop_strings[DESKTOP_UNKNOWN];
    }

    if ( NULL == pRet )
    {
        // get display to connect to
        const char* pDisplayStr = getenv( "DISPLAY" );
        int nParams = osl_getCommandArgCount();
        OUString aParam;
        OString aBParm;
        for( int i = 0; i < nParams; i++ )
        {
            osl_getCommandArg( i, &aParam.pData );
            if( aParam.equalsAscii( "-headless" ) )
            {
                pDisplayStr = NULL;
                break;
            }
            if( i < nParams-1 && (aParam.equalsAscii( "-display" ) || aParam.equalsAscii( "--display" )) )
            {
                osl_getCommandArg( i+1, &aParam.pData );
                aBParm = OUStringToOString( aParam, osl_getThreadTextEncoding() );
                pDisplayStr = aBParm.getStr();
                break;
            }
        }

        // no server at all
        if( ! pDisplayStr || !*pDisplayStr )
            pRet = desktop_strings[DESKTOP_NONE];
        else
        {
            Display* pDisplay = XOpenDisplay( pDisplayStr );
            if( pDisplay )
            {
                XErrorHandler pOldHdl = XSetErrorHandler( autodect_error_handler );

                if ( is_kde_desktop( pDisplay ) )
                    pRet = desktop_strings[DESKTOP_KDE];
                else if ( is_gnome_desktop( pDisplay ) )
                    pRet = desktop_strings[DESKTOP_GNOME];
                else if ( is_cde_desktop( pDisplay ) )
                    pRet = desktop_strings[DESKTOP_CDE];
                else
                    pRet = desktop_strings[DESKTOP_UNKNOWN];

                // set the default handler again
                XSetErrorHandler( pOldHdl );

                XCloseDisplay( pDisplay );
            }
        }
    }

    return pRet;
}


static const char* autodetect_plugin()
{
    const char * desktop = get_desktop_environment();
    const char * pRet = NULL;

    // no server at all: dummy plugin
    if ( desktop == desktop_strings[DESKTOP_NONE] )
        pRet = "svp";
    else if ( desktop == desktop_strings[DESKTOP_GNOME] )
        pRet = "gtk";
    else if( desktop == desktop_strings[DESKTOP_KDE] )
        pRet = "kde";
    else
        pRet = "gen";

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "plugin autodetection: %s\n", pRet );
#endif

    return pRet;
}

static SalInstance* check_headless_plugin()
{
    int nParams = osl_getCommandArgCount();
    OUString aParam;
    for( int i = 0; i < nParams; i++ )
    {
        osl_getCommandArg( i, &aParam.pData );
        if( aParam.equalsAscii( "-headless" ) )
            return tryInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "svp" ) ) );
    }
    return NULL;
}

SalInstance *CreateSalInstance()
{
    SalInstance*    pInst = NULL;

    static const char* pUsePlugin = getenv( "SAL_USE_VCLPLUGIN" );

    if( !(pUsePlugin && *pUsePlugin) )
        pInst = check_headless_plugin();

    if( ! pInst && !(pUsePlugin && *pUsePlugin) )
        pUsePlugin = autodetect_plugin();

    if( ! pInst && pUsePlugin && *pUsePlugin )
        pInst = tryInstance( OUString::createFromAscii( pUsePlugin ) );

    // fallback to gen
    if( ! pInst )
        pInst = tryInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "gen" ) ) );

    if( ! pInst )
    {
        fprintf( stderr, "no suitable windowing system found, exiting.\n" );
        _exit( 1 );
    }

    // acquire SolarMutex
    pInst->AcquireYieldMutex( 1 );

    return pInst;
}

void DestroySalInstance( SalInstance *pInst )
{
    // release SolarMutex
    pInst->ReleaseYieldMutex();

    delete pInst;
    if( pCloseModule )
        osl_unloadModule( pCloseModule );
}

void InitSalData()
{
}

void DeInitSalData()
{
}

void InitSalMain()
{
}

void DeInitSalMain()
{
}

void SalAbort( const XubString& rErrorText )
{
    if( !rErrorText.Len() )
        fprintf( stderr, "Application Error" );
    else
        fprintf( stderr, ByteString( rErrorText, gsl_getSystemTextEncoding() ).GetBuffer() );
    abort();
}

const OUString& SalGetDesktopEnvironment()
{
    static OUString aRet = OStringToOUString(OString(get_desktop_environment()), RTL_TEXTENCODING_ASCII_US);
    return aRet;
}

SalData::SalData() :
    m_pInstance(NULL),
    m_pPlugin(NULL)
{
}

SalData::~SalData()
{
}
