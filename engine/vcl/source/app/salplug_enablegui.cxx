/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cstdio>
#include <rtl/bootstrap.hxx>
#include <rtl/process.h>
#include <salinst.hxx>
#include <sal/backtrace.hxx>
#include <sal/log.hxx>
#include <svdata.hxx>
#include <vcl/svapp.hxx>
#include <tools/svlibrary.h>

#if USING_X11
#define UNIX_DESKTOP_DETECT 1
#include <unx/desktops.hxx>
#elif !(defined ANDROID || defined MACOSX || defined _WIN32 || defined __EMSCRIPTEN__ \
    || defined iOS)
#define UNIX_DESKTOP_DETECT 0
#endif

#if defined(DISABLE_DYNLOADING)
#define STATIC_SAL_INSTANCE 1
#include <staticsalinstance.hxx>
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

#if !STATIC_SAL_INSTANCE
oslModule pCloseModule = nullptr;

extern "C" typedef SalInstance* (*salFactoryProc)();

#if !ENABLE_UNITEDLIBS
SalInstance* tryInstance( const OUString& rModuleBase, bool bForce = false )
{
#if ENABLE_HEADLESS
    if (rModuleBase == "svp")
        return svp_create_SalInstance();
#endif

    SalInstance* pInst = nullptr;
    OUString aUsedModuleBase(rModuleBase);
    OUString aModule( SAL_DLLPREFIX "vclplug_" + aUsedModuleBase + "lo" SAL_DLLEXTENSION );

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
                if (aUsedModuleBase == "gtk3" ||
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
#endif // !ENABLE_UNITEDLIBS
#endif // !STATIC_SAL_INSTANCE

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
    rtl::Bootstrap::get(u"SAL_USE_VCLPLUGIN"_ustr, aUsePlugin);
    SAL_INFO_IF(!aUsePlugin.isEmpty(), "vcl.plugadapt", "Requested VCL plugin: " << aUsePlugin);

    if (Application::IsBitmapRendering() || (aUsePlugin.isEmpty() && IsHeadlessModeRequested()))
        aUsePlugin = u"svp"_ustr;

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

#if ENABLE_UNITEDLIBS
    if (osl_isUnitedLibActive())
    {
#if ENABLE_GTK3
        if (aUsePlugin.isEmpty() || aUsePlugin == "gtk3")
        {
            auto fpCreateInst = reinterpret_cast<salFactoryProc>(osl::Module(SVLIBRARY("united")).getFunctionSymbol("create_SalInstanceGtk3"));
            pInst = fpCreateInst();
        }
#endif
#if ENABLE_GEN
        if (!pInst && (aUsePlugin.isEmpty() || aUsePlugin == "gen"))
        {
            auto fpCreateInst = reinterpret_cast<salFactoryProc>(osl::Module(SVLIBRARY("united")).getFunctionSymbol("create_SalInstanceGen"));
            pInst = fpCreateInst();
        }
#endif
#if defined(MACOSX) && !USE_HEADLESS_CODE
        if (!pInst && (aUsePlugin.isEmpty() || aUsePlugin == "osx"))
        {
            auto fpCreateInst = reinterpret_cast<salFactoryProc>(osl::Module(SVLIBRARY("united")).getFunctionSymbol("create_SalInstanceMac"));
            pInst = fpCreateInst();
        }
#endif
        assert(pInst && "no working window system found");
        if (!pInst)
        {
            std::fprintf( stderr, "no working vcl plugin found, exiting.\n" );
            _exit( 1 );
        }
        return pInst;
    }

#else

    if( !aUsePlugin.isEmpty() )
        pInst = tryInstance( aUsePlugin, true );

    static const char* const pPlugin[] = {
#ifdef _WIN32
        "win",
#elif defined(MACOSX)
        "osx",
#else // !_WIN32 && !MACOSX
#if ENABLE_GTK3
        "gtk3",
#endif
#if ENABLE_GEN
        "gen",
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

#endif  // ENABLE_UNITEDLIBS
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

const OUString& SalGetDesktopEnvironment()
{
#if defined(_WIN32)
    static constexpr OUString aDesktopEnvironment(u"Windows"_ustr);
#elif defined(MACOSX)
    static constexpr OUString aDesktopEnvironment(u"MacOSX"_ustr);
#elif defined(__EMSCRIPTEN__)
    static constexpr OUString aDesktopEnvironment(u"WASM"_ustr);
#elif defined(ANDROID)
    static constexpr OUString aDesktopEnvironment(u"android"_ustr);
#elif defined(iOS)
    static constexpr OUString aDesktopEnvironment(u"iOS"_ustr);
#elif UNIX_DESKTOP_DETECT
    // Order to match desktops.hxx' DesktopType
    static constexpr OUString desktop_strings[] = {
        u"none"_ustr, u"unknown"_ustr, u"GNOME"_ustr, u"UNITY"_ustr,
        u"XFCE"_ustr, u"MATE"_ustr, u"PLASMA5"_ustr, u"PLASMA6"_ustr, u"LXQT"_ustr };
    static OUString aDesktopEnvironment;
    if( aDesktopEnvironment.isEmpty())
    {
        aDesktopEnvironment = desktop_strings[get_desktop_environment()];
    }
#else
    static constexpr OUString aDesktopEnvironment(u"unknown"_ustr);
#endif
    return aDesktopEnvironment;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
