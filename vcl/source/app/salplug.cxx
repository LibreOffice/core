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

// Th current high-level preprocessor structure is:
//
// if !HAVE_FEATURE_UI
//   => STATIC_SAL_INSTANCE
// else
//   ? !STATIC_SAL_INSTANCE
//   ? UNIX_DESKTOP_DETECT
// endif
//
// ENABLE_HEADLESS just signifies the use of the SVP plugin!

#include <config_features.h>
#include <config_vclplug.h>

#include <cstdio>
#include <desktop/crashreport.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/process.h>
#include <salinst.hxx>
#include <sal/log.hxx>
#include <svdata.hxx>
#include <vcl/svapp.hxx>

#if HAVE_FEATURE_UI
#if USING_X11
#define UNIX_DESKTOP_DETECT 1
#include <unx/desktops.hxx>
#else
#define UNIX_DESKTOP_DETECT 0
#endif
#endif

#if defined(DISABLE_DYNLOADING) || !HAVE_FEATURE_UI
#define STATIC_SAL_INSTANCE 1
extern "C" SalInstance* create_SalInstance();
#else
#define STATIC_SAL_INSTANCE 0
#include <osl/module.hxx>
#endif

#if defined(iOS)
#include <premac.h>
#include <UIKit/UIKit.h>
#include <postmac.h>

#elif defined(ANDROID)
#include <android/androidinst.hxx>
#endif

#if defined(_WIN32)
#include <o3tl/char16_t2wchar_t.hxx>
#include <salframe.hxx>
#include <Windows.h>
#else
#include <unistd.h>
#endif

#if ENABLE_HEADLESS
#include <headless/svpdata.hxx>
#include <headless/svpinst.hxx>
#endif

namespace {

#if ENABLE_HEADLESS
SalInstance* svp_create_SalInstance()
{
    SvpSalInstance* pInstance = new SvpSalInstance(std::make_unique<SvpSalYieldMutex>());
    new SvpSalData();
    return pInstance;
}
#endif

#if HAVE_FEATURE_UI

#if !STATIC_SAL_INSTANCE
oslModule pCloseModule = nullptr;

extern "C" typedef SalInstance* (*salFactoryProc)();

SalInstance* tryInstance( const OUString& rModuleBase, bool bForce = false )
{
#if ENABLE_HEADLESS
    if (rModuleBase == "svp")
        return svp_create_SalInstance();
#endif

    SalInstance* pInst = nullptr;
    OUString aUsedModuleBase(rModuleBase);
    if (aUsedModuleBase == "kde5")
        aUsedModuleBase = "kf5";
    OUString aModule(
#ifdef SAL_DLLPREFIX
            SAL_DLLPREFIX
#endif
            "vclplug_" + aUsedModuleBase + "lo" SAL_DLLEXTENSION );

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
                if (aUsedModuleBase == "gtk4" || aUsedModuleBase == "gtk3" ||
                    aUsedModuleBase == "gtk3_kde5" || aUsedModuleBase == "kf5" ||
                    aUsedModuleBase == "qt5" || aUsedModuleBase == "qt6" ||
                    aUsedModuleBase == "win")
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
#endif // !STATIC_SAL_INSTANCE

#if UNIX_DESKTOP_DETECT
#ifndef DISABLE_DYNLOADING
extern "C" typedef DesktopType Fn_get_desktop_environment();
#else
extern "C" DesktopType get_desktop_environment();
#endif

DesktopType lcl_get_desktop_environment()
{
    DesktopType ret = DESKTOP_UNKNOWN;
#ifdef DISABLE_DYNLOADING
    ret = get_desktop_environment();
#else
    OUString aModule(DESKTOP_DETECTOR_DLL_NAME);
    oslModule aMod = osl_loadModuleRelative(
        reinterpret_cast< oslGenericFunction >( &tryInstance ), aModule.pData,
        SAL_LOADMODULE_DEFAULT );
    if( aMod )
    {
        Fn_get_desktop_environment * pSym
            = reinterpret_cast<Fn_get_desktop_environment *>(
                osl_getAsciiFunctionSymbol(aMod, "get_desktop_environment"));
        if( pSym )
            ret = pSym();
    }
    osl_unloadModule( aMod );
#endif
    return ret;
}

#if !STATIC_SAL_INSTANCE
const char* const* autodetect_plugin_list()
{
    static const char* const pKDEFallbackList[] =
    {
#if ENABLE_KF5
        "kf5",
#endif
#if ENABLE_GTK3_KDE5
        "gtk3_kde5",
#endif
#if ENABLE_GTK3
        "gtk3",
#endif
#if ENABLE_GEN
        "gen",
#endif
        nullptr
    };

    static const char* const pStandardFallbackList[] =
    {
#if ENABLE_GTK3
        "gtk3",
#endif
#if ENABLE_GEN
        "gen",
#endif
        nullptr
    };

#if ENABLE_HEADLESS
    static const char* const pHeadlessFallbackList[] =
    {
        "svp",
        nullptr
    };
#endif

    DesktopType desktop = lcl_get_desktop_environment();
    const char * const * pList = pStandardFallbackList;

#if ENABLE_HEADLESS
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
    else if (desktop == DESKTOP_PLASMA5 || desktop == DESKTOP_PLASMA6 || desktop == DESKTOP_LXQT)
        pList = pKDEFallbackList;

    return pList;
}
#endif // !STATIC_SAL_INSTANCE
#endif // UNIX_DESKTOP_DETECT

#endif // HAVE_FEATURE_UI

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

} // anonymous namespace

SalInstance *CreateSalInstance()
{
    OUString aUsePlugin;
    rtl::Bootstrap::get("SAL_USE_VCLPLUGIN", aUsePlugin);
    SAL_INFO_IF(!aUsePlugin.isEmpty(), "vcl.plugadapt", "Requested VCL plugin: " << aUsePlugin);

    if (Application::IsBitmapRendering() || (aUsePlugin.isEmpty() && IsHeadlessModeRequested()))
        aUsePlugin = "svp";

    if (aUsePlugin == "svp")
    {
        Application::EnableBitmapRendering();
#if ENABLE_HEADLESS
        return svp_create_SalInstance();
#else
        aUsePlugin.clear();
#endif
    }

#if STATIC_SAL_INSTANCE
    return create_SalInstance();

#else // !STATIC_SAL_INSTANCE
    SalInstance *pInst = nullptr;

    if( !aUsePlugin.isEmpty() )
        pInst = tryInstance( aUsePlugin, true );

#if UNIX_DESKTOP_DETECT
    const char* const* pPluginList = pInst ? nullptr : autodetect_plugin_list();
    for (int i = 0; !pInst && pPluginList[i]; ++i)
    {
        pInst = tryInstance(OUString::createFromAscii(pPluginList[i]));
        SAL_INFO_IF(pInst, "vcl.plugadapt", "plugin autodetection: " << pPluginList[i]);
    }
#endif

    // fallback, try everything
    static const char* const pPlugin[] = {
#ifdef _WIN32
        "win",
#elif defined(MACOSX)
        "osx",
#else // !_WIN32 && !MACOSX
#if ENABLE_GTK3
        "gtk3",
#endif
#if ENABLE_KF5
        "kf5",
#endif
#if ENABLE_GTK3_KDE5
        "gtk3_kde5",
#endif
#if ENABLE_GEN
        "gen",
#endif
#if ENABLE_QT5
        "qt5",
#endif
#if ENABLE_QT6
        "qt6",
#endif
#endif // !_WIN32 && !MACOSX
        nullptr
    };

    for (int i = 0; !pInst && pPlugin[i]; ++i)
        pInst = tryInstance( OUString::createFromAscii( pPlugin[ i ] ) );

    if( ! pInst )
    {
        std::fprintf( stderr, "no suitable windowing system found, exiting.\n" );
        _exit( 1 );
    }

    return pInst;
#endif // !STATIC_SAL_INSTANCE
}

void DestroySalInstance( SalInstance *pInst )
{
    delete pInst;
#if !STATIC_SAL_INSTANCE
    if( pCloseModule )
        osl_unloadModule( pCloseModule );
#endif
}

void SalAbort( const OUString& rErrorText, bool bDumpCore )
{
    if (GetSalInstance())
        GetSalInstance()->BeforeAbort(rErrorText, bDumpCore);

#if defined _WIN32
    if( rErrorText.isEmpty() )
    {
        // make sure crash reporter is triggered
        RaiseException( 0, EXCEPTION_NONCONTINUABLE, 0, nullptr );
        FatalAppExitW( 0, L"Application Error" );
    }
    else
    {
        CrashReporter::addKeyValue("AbortMessage", rErrorText, CrashReporter::Write);
        // make sure crash reporter is triggered
        RaiseException( 0, EXCEPTION_NONCONTINUABLE, 0, nullptr );
        FatalAppExitW( 0, o3tl::toW(rErrorText.getStr()) );
    }
#else // !_WIN32
#if defined ANDROID
    OUString aError(rErrorText.isEmpty() ? "Unspecified application error" : rErrorText);
    LOGE("SalAbort: '%s'", OUStringToOString(aError, osl_getThreadTextEncoding()).getStr());
#elif defined(iOS)
    NSLog(@"SalAbort: %s", OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr());
#else
    if( rErrorText.isEmpty() )
        std::fprintf( stderr, "Unspecified Application Error\n" );
    else
    {
        CrashReporter::addKeyValue("AbortMessage", rErrorText, CrashReporter::Write);
        std::fprintf( stderr, "%s\n", OUStringToOString(rErrorText, osl_getThreadTextEncoding()).getStr() );
    }
#endif
    if( bDumpCore )
        abort();
    else
        _exit(1);
#endif // !_WIN32
}

const OUString& SalGetDesktopEnvironment()
{
#if !HAVE_FEATURE_UI
    static OUString aDesktopEnvironment("headless");
#elif defined(_WIN32)
    static OUString aDesktopEnvironment( "Windows" );
#elif defined(MACOSX)
    static OUString aDesktopEnvironment( "MacOSX" );
#elif defined(EMSCRIPTEN)
    static OUString aDesktopEnvironment("WASM");
#elif defined(ANDROID)
    static OUString aDesktopEnvironment("android");
#elif defined(iOS)
    static OUString aDesktopEnvironment("iOS");
#elif UNIX_DESKTOP_DETECT
    // Order to match desktops.hxx' DesktopType
    static const char * const desktop_strings[] = {
        "none", "unknown", "GNOME", "UNITY",
        "XFCE", "MATE", "PLASMA5", "PLASMA6", "LXQT" };
    static OUString aDesktopEnvironment;
    if( aDesktopEnvironment.isEmpty())
    {
        aDesktopEnvironment = OUString::createFromAscii(
            desktop_strings[lcl_get_desktop_environment()]);
    }
#else
    static OUString aDesktopEnvironment("unknown");
#endif
    return aDesktopEnvironment;
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
