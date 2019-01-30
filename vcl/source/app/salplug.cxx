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

#include <osl/module.hxx>
#include <osl/process.h>

#include <rtl/bootstrap.hxx>
#include <rtl/process.h>
#include <sal/log.hxx>

#include <salframe.hxx>
#include <salinst.hxx>
#include <config_vclplug.h>
#include <desktop/crashreport.hxx>

#ifndef _WIN32
#include <headless/svpinst.hxx>
#include <printerinfomanager.hxx>
#include <unx/desktops.hxx>
#include <unx/gensys.h>
#include <unx/gendata.hxx>

#include <unistd.h>
#else
#include <saldatabasic.hxx>
#include <Windows.h>
#endif

#include <cstdio>

#ifdef ANDROID
#error "Android has no plugin infrastructure!"
#endif

#if !(defined _WIN32 || defined MACOSX)
#define DESKTOPDETECT
#define HEADLESS_VCLPLUG
#endif

extern "C" {
typedef SalInstance*(*salFactoryProc)();
}

namespace {

oslModule pCloseModule = nullptr;

SalInstance* tryInstance( const OUString& rModuleBase, bool bForce = false )
{
#ifdef HEADLESS_VCLPLUG
    if (rModuleBase == "svp")
        return svp_create_SalInstance();
#endif

    SalInstance* pInst = nullptr;
    OUString aModule(
#ifdef SAL_DLLPREFIX
            SAL_DLLPREFIX
#endif
            "vclplug_" + rModuleBase + "lo" SAL_DLLEXTENSION );

    osl::Module aMod;
    if (aMod.loadRelative(reinterpret_cast<oslGenericFunction>(&tryInstance), aModule, SAL_LOADMODULE_GLOBAL))
    {
        salFactoryProc aProc = reinterpret_cast<salFactoryProc>(aMod.getFunctionSymbol("create_SalInstance"));
        if (aProc)
        {
            pInst = aProc();
            SAL_INFO(
                "vcl.plugadapt",
                "sal plugin " << aModule << " produced instance " << pInst);
            if (pInst)
            {
                pCloseModule = static_cast<oslModule>(aMod);
                aMod.release();

                /*
                 * Recent GTK+ versions load their modules with RTLD_LOCAL, so we can
                 * not access the 'gnome_accessibility_module_shutdown' anymore.
                 * So make sure libgtk+ & co are still mapped into memory when
                 * atk-bridge's atexit handler gets called.
                 */
                if( rModuleBase == "gtk" || rModuleBase == "gtk3" || rModuleBase == "gtk3_kde5" || rModuleBase == "win" )
                {
                    pCloseModule = nullptr;
                }
            }
        }
        else
        {
            SAL_WARN(
                "vcl.plugadapt",
                "could not load symbol create_SalInstance from shared object "
                    << aModule);
        }
    }
    else if (bForce)
    {
        SAL_WARN("vcl.plugadapt", "could not load shared object " << aModule);
    }
    else
    {
        SAL_INFO("vcl.plugadapt", "could not load shared object " << aModule);
    }

    // coverity[leaked_storage] - this is on purpose
    return pInst;
}

#ifdef DESKTOPDETECT
extern "C" typedef DesktopType Fn_get_desktop_environment();

DesktopType get_desktop_environment()
{
    OUString aModule(DESKTOP_DETECTOR_DLL_NAME);
    oslModule aMod = osl_loadModuleRelative(
        reinterpret_cast< oslGenericFunction >( &tryInstance ), aModule.pData,
        SAL_LOADMODULE_DEFAULT );
    DesktopType ret = DESKTOP_UNKNOWN;
    if( aMod )
    {
        Fn_get_desktop_environment * pSym
            = reinterpret_cast<Fn_get_desktop_environment *>(
                osl_getAsciiFunctionSymbol(aMod, "get_desktop_environment"));
        if( pSym )
            ret = pSym();
    }
    osl_unloadModule( aMod );
    return ret;
}

SalInstance* autodetect_plugin()
{
    static const char* const pKDEFallbackList[] =
    {
#if ENABLE_KDE5
        "kde5",
#endif
#if ENABLE_GTK3_KDE5
        "gtk3_kde5",
#endif
        "gtk3", "gtk", "gen", nullptr
    };

    static const char* const pStandardFallbackList[] =
    {
        "gtk3", "gtk", "gen", nullptr
    };

#ifdef HEADLESS_VCLPLUG
    static const char* const pHeadlessFallbackList[] =
    {
        "svp", nullptr
    };
#endif

    DesktopType desktop = get_desktop_environment();
    const char * const * pList = pStandardFallbackList;
    int nListEntry = 0;

#ifdef HEADLESS_VCLPLUG
    // no server at all: dummy plugin
    if ( desktop == DESKTOP_NONE )
        pList = pHeadlessFallbackList;
    else
#endif
        if ( desktop == DESKTOP_GNOME ||
              desktop == DESKTOP_UNITY ||
              desktop == DESKTOP_XFCE  ||
              desktop == DESKTOP_MATE )
        pList = pStandardFallbackList;
    else if( desktop == DESKTOP_KDE5 ||
              desktop == DESKTOP_LXQT )
        pList = pKDEFallbackList;

    SalInstance* pInst = nullptr;
    while( pList[nListEntry] && pInst == nullptr )
    {
        OUString aTry( OUString::createFromAscii( pList[nListEntry] ) );
        pInst = tryInstance( aTry );
        SAL_INFO_IF(
            pInst, "vcl.plugadapt",
            "plugin autodetection: " << pList[nListEntry]);
        nListEntry++;
    }

    return pInst;
}
#endif // DESKTOPDETECT

#ifdef HEADLESS_VCLPLUG
// HACK to obtain Application::IsHeadlessModeEnabled early on, before
// Application::EnableHeadlessMode has potentially been called:
bool IsHeadlessModeRequested()
{
    if (Application::IsHeadlessModeEnabled()) {
        return true;
    }
    sal_uInt32 n = rtl_getAppCommandArgCount();
    for (sal_uInt32 i = 0; i < n; ++i) {
        OUString arg;
        rtl_getAppCommandArg(i, &arg.pData);
        if ( arg == "--headless" || arg == "-headless" ) {
            return true;
        }
    }
    return false;
}
#endif

} // anonymous namespace

SalInstance *CreateSalInstance()
{
    SalInstance *pInst = nullptr;

    OUString aUsePlugin;
    rtl::Bootstrap::get("SAL_USE_VCLPLUGIN", aUsePlugin);
    SAL_INFO_IF(!aUsePlugin.isEmpty(), "vcl", "Requested VCL plugin: " << aUsePlugin);
#ifdef HEADLESS_VCLPLUG
    if (Application::IsBitmapRendering() || (aUsePlugin.isEmpty() && IsHeadlessModeRequested()))
        aUsePlugin = "svp";
#endif

    if (aUsePlugin == "svp")
    {
        Application::EnableBitmapRendering();
#ifndef HEADLESS_VCLPLUG
        aUsePlugin.clear();
#endif
    }
    if( !aUsePlugin.isEmpty() )
        pInst = tryInstance( aUsePlugin, true );

#ifdef DESKTOPDETECT
    if( ! pInst )
        pInst = autodetect_plugin();
#endif

    // fallback, try everything
    static const char* const pPlugin[] = {
#ifdef _WIN32
        "win"
#else
#ifdef MACOSX
        "osx"
#else
        "gtk3", "gtk", "kde5", "gen"
#endif
#endif
     };

    for ( int i = 0; !pInst && i != SAL_N_ELEMENTS(pPlugin); ++i )
        pInst = tryInstance( OUString::createFromAscii( pPlugin[ i ] ) );

    if( ! pInst )
    {
        std::fprintf( stderr, "no suitable windowing system found, exiting.\n" );
        _exit( 1 );
    }

    // acquire SolarMutex
    pInst->AcquireYieldMutex();

    return pInst;
}

void DestroySalInstance( SalInstance *pInst )
{
    // release SolarMutex
    pInst->ReleaseYieldMutexAll();

    delete pInst;
    if( pCloseModule )
        osl_unloadModule( pCloseModule );
}

void SalAbort( const OUString& rErrorText, bool bDumpCore )
{
    if( rErrorText.isEmpty() )
        std::fprintf( stderr, "Application Error\n" );
    else
    {
        CrashReporter::AddKeyValue("AbortMessage", rErrorText);
        std::fprintf( stderr, "%s\n", OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr() );
    }
    if( bDumpCore )
        abort();
    else
        _exit(1);
}

const OUString& SalGetDesktopEnvironment()
{
#ifdef _WIN32
    static OUString aDesktopEnvironment( "Windows" );

#else
#ifdef MACOSX
    static OUString aDesktopEnvironment( "MacOSX" );
#else
    // Order to match desktops.hxx' DesktopType
    static const char * const desktop_strings[] = {
        "none", "unknown", "GNOME", "UNITY",
        "XFCE", "MATE", "KDE5", "LXQT" };
    static OUString aDesktopEnvironment;
    if( aDesktopEnvironment.isEmpty())
    {
        aDesktopEnvironment = OUString::createFromAscii(
            desktop_strings[get_desktop_environment()]);
    }
#endif
#endif
    return aDesktopEnvironment;
}

SalData::SalData() :
    m_pInstance(nullptr),
    m_pPIManager(nullptr)
{
}

SalData::~SalData() COVERITY_NOEXCEPT_FALSE
{
#if (defined UNX && !defined MACOSX)
    psp::PrinterInfoManager::release();
#endif
}

#ifdef _WIN32
bool HasAtHook()
{
    BOOL bIsRunning = FALSE;
    // pvParam must be BOOL
    return SystemParametersInfoW(SPI_GETSCREENREADER, 0, &bIsRunning, 0)
        && bIsRunning;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
