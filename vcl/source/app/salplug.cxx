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

#include <unistd.h>
#else
#include <saldatabasic.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <Windows.h>
#endif

#include <cstdio>

#ifdef ANDROID
#include <android/androidinst.hxx>
#endif

#if USING_X11
#define DESKTOPDETECT
#endif
#if ENABLE_HEADLESS
#define HEADLESS_VCLPLUG
#endif

extern "C" {
typedef SalInstance*(*salFactoryProc)();
}

namespace {

#ifndef DISABLE_DYNLOADING
oslModule pCloseModule = nullptr;
#endif

SalInstance* tryInstance( const OUString& rModuleBase, bool bForce = false )
{
#ifdef DISABLE_DYNLOADING
    (void)rModuleBase;
    (void)bForce;
    return create_SalInstance();
#else // !DISABLE_DYNLOADING
#ifdef HEADLESS_VCLPLUG
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
#endif // !DISABLE_DYNLOADING
}

#ifdef DESKTOPDETECT
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

SalInstance* autodetect_plugin()
{
#ifdef DISABLE_DYNLOADING
    return nullptr;
#else // !DISABLE_DYNLOADING
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

#ifdef HEADLESS_VCLPLUG
    static const char* const pHeadlessFallbackList[] =
    {
        "svp",
        nullptr
    };
#endif

    SalInstance* pInst = nullptr;
    DesktopType desktop = lcl_get_desktop_environment();
    const char * const * pList = pStandardFallbackList;

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
    else if (desktop == DESKTOP_PLASMA5 || desktop == DESKTOP_LXQT)
        pList = pKDEFallbackList;

    for (int i = 0; !pInst && pList[i]; ++i)
    {
        OUString aTry(OUString::createFromAscii(pList[i]));
        pInst = tryInstance( aTry );
        SAL_INFO_IF(pInst, "vcl.plugadapt", "plugin autodetection: " << pList[i]);
    }
    return pInst;
#endif // !DISABLE_DYNLOADING
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
        "win",
#elif defined(MACOSX)
        "osx",
#else
#if ENABLE_GTK3
        "gtk3",
#endif
#if ENABLE_KF5
        "kf5",
#endif
#if ENABLE_GEN
        "gen",
#endif
#endif
        nullptr
     };

    for (int i = 0; !pInst && pPlugin[i]; ++i)
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
#ifndef DISABLE_DYNLOADING
    if( pCloseModule )
        osl_unloadModule( pCloseModule );
#endif
}

void SalAbort( const OUString& rErrorText, bool bDumpCore )
{
    if (GetSalData()->m_pInstance)
        GetSalData()->m_pInstance->BeforeAbort(rErrorText, bDumpCore);

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
#else
#if defined ANDROID
    OUString aError(rErrorText.isEmpty() ? "Unspecified application error" : rErrorText);
    LOGE("SalAbort: '%s'", OUStringToOString(aError, osl_getThreadTextEncoding()).getStr());
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
#endif
}

const OUString& SalGetDesktopEnvironment()
{
#ifdef _WIN32
    static OUString aDesktopEnvironment( "Windows" );
#elif defined(MACOSX)
    static OUString aDesktopEnvironment( "MacOSX" );
#elif defined(EMSCRIPTEN)
    static OUString aDesktopEnvironment("WASM");
#elif defined(ANDROID)
    static OUString aDesktopEnvironment("android");
#elif USING_X11
    // Order to match desktops.hxx' DesktopType
    static const char * const desktop_strings[] = {
        "none", "unknown", "GNOME", "UNITY",
        "XFCE", "MATE", "PLASMA5", "LXQT" };
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
