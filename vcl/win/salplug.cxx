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

#include <salinst.hxx>
#include <saldatabasic.hxx>
#include <config_vclplug.h>
#include <desktop/crashreport.hxx>

#include <cstdio>
#include <Windows.h>

extern "C" {
typedef SalInstance*(*salFactoryProc)();
}

namespace {

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

}

static oslModule pCloseModule = nullptr;

static SalInstance* tryInstance( const OUString& rModuleBase, bool bForce = false )
{
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

SalInstance *CreateSalInstance()
{
    SalInstance *pInst = nullptr;

    OUString aUsePlugin;
    rtl::Bootstrap::get( "SAL_USE_VCLPLUGIN", aUsePlugin );

    if( !aUsePlugin.isEmpty() )
        pInst = tryInstance( aUsePlugin, true );

    // fallback, try everything
    static const char* const pPlugin[] = { "win" };

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

void InitSalData()
{
}

void DeInitSalData()
{
}

void InitSalMain()
{
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
    static OUString aDesktopEnvironment( "Windows" );
    return aDesktopEnvironment;
}

SalData::SalData() :
    m_pInstance(nullptr),
    m_pPIManager(nullptr)
{
}

SalData::~SalData() COVERITY_NOEXCEPT_FALSE
{
}

bool HasAtHook()
{
    BOOL bIsRunning = FALSE;
    // pvParam must be BOOL
    return SystemParametersInfoW(SPI_GETSCREENREADER, 0, &bIsRunning, 0)
        && bIsRunning;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
