/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "osl/module.h"
#include "osl/process.h"

#include "rtl/ustrbuf.hxx"


#include "vcl/salinst.hxx"
#include "saldata.hxx"

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

static const rtl::OUString& get_desktop_environment()
{
    static rtl::OUString aRet;
    if( ! aRet.getLength() )
    {
        OUStringBuffer aModName( 128 );
        aModName.appendAscii( SAL_DLLPREFIX"desktop_detector" );
        aModName.appendAscii( SAL_DLLPOSTFIX );
        aModName.appendAscii( SAL_DLLEXTENSION );
        OUString aModule = aModName.makeStringAndClear();

        oslModule aMod = osl_loadModuleRelative(
            reinterpret_cast< oslGenericFunction >( &tryInstance ), aModule.pData,
            SAL_LOADMODULE_DEFAULT );
        if( aMod )
        {
            rtl::OUString (*pSym)() = (rtl::OUString(*)())
                osl_getAsciiFunctionSymbol( aMod, "get_desktop_environment" );
            if( pSym )
                aRet = pSym();
        }
        osl_unloadModule( aMod );
    }
    return aRet;
}

static const char* autodetect_plugin()
{
    const rtl::OUString& desktop( get_desktop_environment() );
    const char * pRet = "gen";

    // no server at all: dummy plugin
    if ( desktop.equalsAscii( desktop_strings[DESKTOP_NONE] ) )
        pRet = "svp";
    else if ( desktop.equalsAscii( desktop_strings[DESKTOP_GNOME] ) )
        pRet = "gtk";
    else if( desktop.equalsAscii( desktop_strings[DESKTOP_KDE] ) )
        pRet = "kde";
    else if( desktop.equalsAscii( desktop_strings[DESKTOP_KDE4] ) )
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
    return get_desktop_environment();
}

SalData::SalData() :
    m_pInstance(NULL),
    m_pPlugin(NULL)
{
}

SalData::~SalData()
{
}
