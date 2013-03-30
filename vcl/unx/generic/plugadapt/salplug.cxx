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

#include "osl/module.h"
#include "osl/process.h"

#include "rtl/ustrbuf.hxx"
#include "rtl/bootstrap.hxx"

#include "salinst.hxx"
#include "generic/gensys.h"
#include "generic/gendata.hxx"
#include "unx/desktops.hxx"
#include "vcl/printerinfomanager.hxx"

#include <cstdio>
#include <unistd.h>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
extern "C" {
typedef SalInstance*(*salFactoryProc)( oslModule pModule);
}

static oslModule pCloseModule = NULL;

static SalInstance* tryInstance( const OUString& rModuleBase, bool bForce = false )
{
    SalInstance* pInst = NULL;
    // Disable gtk3 plugin for now unless explicitly requested via
    // SAL_USE_VCLPLUGIN=gtk3 (would ideally depend on experimental mode, but
    // reading the experimental mode setting requires the UNO service manager
    // which has not yet been instantiated):
    if (!bForce && rModuleBase == "gtk3")
    {
        return NULL;
    }
    OUStringBuffer aModName( 128 );
    aModName.appendAscii( SAL_DLLPREFIX"vclplug_" );
    aModName.append( rModuleBase );
    aModName.appendAscii( SAL_DLLPOSTFIX );
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

#ifndef ANDROID
                /*
                 * Recent GTK+ versions load their modules with RTLD_LOCAL, so we can
                 * not access the 'gnome_accessibility_module_shutdown' anymore.
                 * So make sure libgtk+ & co are still mapped into memory when
                 * atk-bridge's atexit handler gets called.
                 * #i109007# KDE3 seems to have the same problem.
		 * And same applies for KDE4.
                 */
                if( rModuleBase == "gtk" || rModuleBase == "gtk3" || rModuleBase == "tde" || rModuleBase == "kde" || rModuleBase == "kde4" )
                {
                    pCloseModule = NULL;
                }
#endif
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

#if !defined(ANDROID)

static DesktopType get_desktop_environment()
{
    OUStringBuffer aModName( 128 );
    #ifdef LIBO_MERGELIBS
        aModName.appendAscii( SAL_DLLPREFIX"merged" );
    #else
        aModName.appendAscii( SAL_DLLPREFIX"desktop_detector" );
    #endif
    aModName.appendAscii( SAL_DLLPOSTFIX );
    OUString aModule = aModName.makeStringAndClear();

    oslModule aMod = osl_loadModuleRelative(
        reinterpret_cast< oslGenericFunction >( &tryInstance ), aModule.pData,
        SAL_LOADMODULE_DEFAULT );
    DesktopType ret = DESKTOP_UNKNOWN;
    if( aMod )
    {
        DesktopType (*pSym)() = (DesktopType(*)())
            osl_getAsciiFunctionSymbol( aMod, "get_desktop_environment" );
        if( pSym )
            ret = pSym();
    }
    osl_unloadModule( aMod );
    return ret;
}

#else

#define get_desktop_environment() DESKTOP_NONE // For now...

#endif

static SalInstance* autodetect_plugin()
{
    static const char* pTDEFallbackList[] =
    {
        "tde", "kde4", "kde", "gtk3", "gtk", "gen", 0
    };

    static const char* pKDEFallbackList[] =
    {
        "kde4", "kde", "gtk3", "gtk", "gen", 0
    };

    static const char* pStandardFallbackList[] =
    {
        "gtk3", "gtk", "gen", 0
    };

    static const char* pHeadlessFallbackList[] =
    {
        "svp", 0
    };

    DesktopType desktop = get_desktop_environment();
    const char ** pList = pStandardFallbackList;
    int nListEntry = 0;

    // no server at all: dummy plugin
    if ( desktop == DESKTOP_NONE )
        pList = pHeadlessFallbackList;
    else if ( desktop == DESKTOP_GNOME )
        pList = pStandardFallbackList;
    else if( desktop == DESKTOP_TDE )
        pList = pTDEFallbackList;
    else if( desktop == DESKTOP_KDE )
    {
        pList = pKDEFallbackList;
        nListEntry = 1;
    }
    else if( desktop == DESKTOP_KDE4 )
        pList = pKDEFallbackList;

    SalInstance* pInst = NULL;
    while( pList[nListEntry] && pInst == NULL )
    {
        rtl::OUString aTry( rtl::OUString::createFromAscii( pList[nListEntry] ) );
        pInst = tryInstance( aTry );
        #if OSL_DEBUG_LEVEL > 1
        if( pInst )
            std::fprintf( stderr, "plugin autodetection: %s\n", pList[nListEntry] );
        #endif
        nListEntry++;
    }

    return pInst;
}

SalInstance *CreateSalInstance()
{
    SalInstance *pInst = NULL;

    OUString aUsePlugin;
    static const char* pUsePlugin = getenv( "SAL_USE_VCLPLUGIN" );
    if( pUsePlugin )
        aUsePlugin = OUString::createFromAscii( pUsePlugin );
    else
        rtl::Bootstrap::get( "SAL_USE_VCLPLUGIN", aUsePlugin );

    if( Application::IsHeadlessModeRequested() )
        aUsePlugin = "svp";

    if( !aUsePlugin.isEmpty() )
        pInst = tryInstance( aUsePlugin, true );

    if( ! pInst )
        pInst = autodetect_plugin();

    // fallback, try everything
    const char* pPlugin[] = { "gtk3", "gtk", "kde4", "kde", "tde", "gen", 0 };

    for ( int i = 0; !pInst && pPlugin[ i ]; ++i )
        pInst = tryInstance( OUString::createFromAscii( pPlugin[ i ] ) );

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

void SalAbort( const rtl::OUString& rErrorText, bool bDumpCore )
{
    if( rErrorText.isEmpty() )
        std::fprintf( stderr, "Application Error\n" );
    else
        std::fprintf( stderr, "%s\n", rtl::OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr() );
    if( bDumpCore )
        abort();
    else
        _exit(1);
}

static const char * desktop_strings[] = { "none", "unknown", "GNOME", "TDE", "KDE", "KDE4" };

const OUString& SalGetDesktopEnvironment()
{
    static rtl::OUString aRet;
    if( aRet.isEmpty())
    {
        rtl::OUStringBuffer buf( 8 );
        buf.appendAscii( desktop_strings[ get_desktop_environment() ] );
        aRet = buf.makeStringAndClear();
    }
    return aRet;
}

SalData::SalData() :
    m_pInstance(NULL),
    m_pPlugin(NULL),
    m_pPIManager(NULL)
{
}

SalData::~SalData()
{
    psp::PrinterInfoManager::release();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
