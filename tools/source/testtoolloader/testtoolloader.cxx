/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
#include "precompiled_tools.hxx"

#include "tools/testtoolloader.hxx"
#include <osl/module.h>
#include <rtl/logfile.hxx>
#include <osl/process.h>
#include "tools/solar.h"
#include "tools/string.hxx"
#include "tools/debug.hxx"

#include <comphelper/uieventslogger.hxx>

using ::rtl::OUString;
namespace tools
{
    typedef void ( *pfunc_CreateRemoteControl)();
    typedef void ( *pfunc_DestroyRemoteControl)();

    typedef void ( *pfunc_CreateEventLogger)();
    typedef void ( *pfunc_DestroyEventLogger)();

static oslModule    aTestToolModule = 0;
// are we to be automated at all?
static bool bAutomate = false;
static bool bLoggerStarted = false;


sal_uInt32 GetCommandLineParamCount()
{
    return osl_getCommandArgCount();
}

String GetCommandLineParam( sal_uInt32 nParam )
{
    ::rtl::OUString aParam;
    oslProcessError eError = osl_getCommandArg( nParam, &aParam.pData );
    if ( eError == osl_Process_E_None )
        return String( aParam );
    else
    {
        OSL_FAIL( "Unable to get CommandLineParam" );
        return String();
    }
}

extern "C" { static void SAL_CALL thisModule() {} }

void LoadLib()
{
    if ( !aTestToolModule )
    {
        aTestToolModule = osl_loadModuleRelative(
            &thisModule,
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SVLIBRARY("sts"))).pData,
            SAL_LOADMODULE_GLOBAL );
    }
}

void InitTestToolLib()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::InitTestToolLib" );

    sal_uInt32 i;

    for ( i = 0 ; i < GetCommandLineParamCount() ; i++ )
    {
        if ( GetCommandLineParam( i ).EqualsIgnoreCaseAscii("/enableautomation")
            || GetCommandLineParam( i ).EqualsIgnoreCaseAscii("-enableautomation"))
        {
            bAutomate = true;
            break;
        }
    }

    if ( bAutomate )
    {
        OUString    aFuncName( RTL_CONSTASCII_USTRINGPARAM( "CreateRemoteControl" ));

        LoadLib();
        if ( aTestToolModule )
        {
            oslGenericFunction pInitFunc = osl_getFunctionSymbol(
                aTestToolModule, aFuncName.pData );
            if ( pInitFunc )
                (reinterpret_cast< pfunc_CreateRemoteControl >(pInitFunc))();
            else
            {
                OSL_TRACE( "Unable to get Symbol 'CreateRemoteControl' from library %s while loading testtool support.", SVLIBRARY( "sts" ) );
            }
        }
        else
        {
            OSL_TRACE( "Unable to access library %s while loading testtool support.", SVLIBRARY( "sts" ) );
        }
    }

    if ( ::comphelper::UiEventsLogger::isEnabled() )
    {
        OUString    aFuncName( RTL_CONSTASCII_USTRINGPARAM( "CreateEventLogger" ));

        LoadLib();
        if ( aTestToolModule )
        {
            oslGenericFunction pInitFunc = osl_getFunctionSymbol(
                aTestToolModule, aFuncName.pData );
            if ( pInitFunc )
            {
                (reinterpret_cast< pfunc_CreateEventLogger >(pInitFunc))();
                bLoggerStarted = sal_True;
            }
            else
            {
                OSL_TRACE( "Unable to get Symbol 'CreateEventLogger' from library %s while loading testtool support.", SVLIBRARY( "sts" ) );
            }
        }
        else
        {
            OSL_TRACE( "Unable to access library %s while loading testtool support.", SVLIBRARY( "sts" ) );
        }
    }
}

void DeInitTestToolLib()
{
    if ( aTestToolModule )
    {
        if ( bAutomate )
        {
            OUString    aFuncName( RTL_CONSTASCII_USTRINGPARAM( "DestroyRemoteControl" ));

            oslGenericFunction pDeInitFunc = osl_getFunctionSymbol(
                aTestToolModule, aFuncName.pData );
            if ( pDeInitFunc )
                (reinterpret_cast< pfunc_DestroyRemoteControl >(pDeInitFunc))();
        }

        if ( bLoggerStarted /*::comphelper::UiEventsLogger::isEnabled()*/ )
        {
            OUString    aFuncName( RTL_CONSTASCII_USTRINGPARAM( "DestroyEventLogger" ));

            oslGenericFunction pDeInitFunc = osl_getFunctionSymbol(
                aTestToolModule, aFuncName.pData );
            if ( pDeInitFunc )
            {
                (reinterpret_cast< pfunc_DestroyEventLogger >(pDeInitFunc))();
                bLoggerStarted = sal_False;
            }
        }

        osl_unloadModule( aTestToolModule );
    }
}

} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
