/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: salplug.cxx,v $
 * $Revision: 1.30 $
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

#include <osl/module.h>
#include <osl/process.h>

#include <rtl/ustrbuf.hxx>

#include <svunx.h>
#include <tools/prex.h>
#include <X11/Xatom.h>
#include <tools/postx.h>

#include <vcl/salinst.hxx>
#include <saldata.hxx>

#include <cstdio>
#include <unistd.h>

using namespace rtl;

extern "C" {
typedef SalInstance*(*salFactoryProc)( oslModule pModule);
}

static oslModule pCloseModule = NULL;

enum {
    DESKTOP_NONE = 0,
    DESKTOP_UNKNOWN,
    DESKTOP_GNOME,
    DESKTOP_KDE,
    DESKTOP_KDE4,
    DESKTOP_CDE
};

static const char * desktop_strings[] = { "none", "unknown", "GNOME", "KDE", "KDE4", "CDE" };

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
        salFactoryProc aProc = (salFactoryProc)osl_getAsciiFunctionSymbol( aMod, "create_SalInstance" );
        if( aProc )
        {
            pInst = aProc( aMod );
#if OSL_DEBUG_LEVEL > 1
            std::fprintf( stderr, "sal plugin %s produced instance %p\n",
                     OUStringToOString( aModule, RTL_TEXTENCODING_ASCII_US ).getStr(),
                     pInst );
#endif
            if( pInst )
            {
                pCloseModule = aMod;

                /*
                 * Recent GTK+ versions load their modules with RTLD_LOCAL, so we can
                 * not access the 'gnome_accessibility_module_shutdown' anymore.
                 * So make sure libgtk+ & co are still mapped into memory when
                 * atk-bridge's atexit handler gets called.
                 */
                if( rModuleBase.equalsAscii("gtk") )
                {
                    pCloseModule = NULL;
                }

                GetSalData()->m_pPlugin = aMod;
            }
            else
                osl_unloadModule( aMod );
        }
        else
        {
#if OSL_DEBUG_LEVEL > 1
            std::fprintf( stderr, "could not load symbol %s from shared object %s\n",
                     "create_SalInstance",
                     OUStringToOString( aModule, RTL_TEXTENCODING_ASCII_US ).getStr() );
#endif
            osl_unloadModule( aMod );
        }
    }
#if OSL_DEBUG_LEVEL > 1
    else
        std::fprintf( stderr, "could not load shared object %s\n",
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

static bool is_kde_desktop( Display* pDisplay )
{
    if ( NULL != getenv( "KDE_FULL_SESSION" ) )
    {
        const char *pVer = getenv( "KDE_SESSION_VERSION" );
        if ( !pVer || pVer[0] == '0' )
        {
            return true; // does not exist => KDE3
        }

        rtl::OUString aVer( RTL_CONSTASCII_USTRINGPARAM( "3" ) );
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
        rtl::OUString aVer( RTL_CONSTASCII_USTRINGPARAM( "4" ) );

        const char *pVer = getenv( "KDE_SESSION_VERSION" );
        if ( pVer && aVer.equalsIgnoreAsciiCaseAscii( pVer ) )
            return true;
    }

    if ( KDEVersion( pDisplay ) == 4 )
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
        if ( aOver.equalsIgnoreAsciiCase( "kde4" ) )
            pRet = desktop_strings[DESKTOP_KDE4];
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

                if ( is_kde4_desktop( pDisplay ) )
                    pRet = desktop_strings[DESKTOP_KDE4];
                else if ( is_gnome_desktop( pDisplay ) )
                    pRet = desktop_strings[DESKTOP_GNOME];
                else if ( is_kde_desktop( pDisplay ) )
                    pRet = desktop_strings[DESKTOP_KDE];
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
    const char * pRet = "gen";

    // no server at all: dummy plugin
    if ( desktop == desktop_strings[DESKTOP_NONE] )
        pRet = "svp";
    else if ( desktop == desktop_strings[DESKTOP_GNOME] )
        pRet = "gtk";
    else if( desktop == desktop_strings[DESKTOP_KDE] )
        pRet = "kde";
    else if( desktop == desktop_strings[DESKTOP_KDE4] )
        pRet = "kde4";
    else
    {
        // #i95296# use the much nicer looking gtk plugin
        // on desktops that set gtk variables (e.g. XFCE)
        static const char* pEnv = getenv( "GTK2_RC_FILES" );
        if( pEnv && *pEnv ) // check for existance and non emptiness
            pRet = "gtk";
    }

#if OSL_DEBUG_LEVEL > 1
    std::fprintf( stderr, "plugin autodetection: %s\n", pRet );
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

    if( ! pInst )
    {
        /* #i92121# workaround deadlocks in the X11 implementation
        */
        static const char* pNoXInitThreads = getenv( "SAL_NO_XINITTHREADS" );
        /* #i90094#
           from now on we know that an X connection will be
           established, so protect X against itself
        */
        if( ! ( pNoXInitThreads && *pNoXInitThreads ) )
            XInitThreads();
    }

    if( ! pInst && !(pUsePlugin && *pUsePlugin) )
        pUsePlugin = autodetect_plugin();

    if( ! pInst && pUsePlugin && *pUsePlugin )
        pInst = tryInstance( OUString::createFromAscii( pUsePlugin ) );

    // fallback to gen
    if( ! pInst )
        pInst = tryInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "gen" ) ) );

    if( ! pInst )
    {
        std::fprintf( stderr, "no suitable windowing system found, exiting.\n" );
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
        std::fprintf( stderr, "Application Error" );
    else
        std::fprintf( stderr, ByteString( rErrorText, gsl_getSystemTextEncoding() ).GetBuffer() );
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
