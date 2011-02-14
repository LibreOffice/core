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
#include <vos/process.hxx>
#include "tools/solar.h"
#include "tools/string.hxx"
#include "tools/debug.hxx"

#include <comphelper/uieventslogger.hxx>

using namespace rtl;

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
    vos:: OStartupInfo  aStartInfo;
    return aStartInfo.getCommandArgCount();
}

String GetCommandLineParam( sal_uInt32 nParam )
{
    vos:: OStartupInfo  aStartInfo;
    ::rtl::OUString aParam;
    vos:: OStartupInfo ::TStartupError eError = aStartInfo.getCommandArg( nParam, aParam );
    if ( eError == vos:: OStartupInfo ::E_None )
        return String( aParam );
    else
    {
        DBG_ERROR( "Unable to get CommandLineParam" );
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
                DBG_ERROR1( "Unable to get Symbol 'CreateRemoteControl' from library %s while loading testtool support.", SVLIBRARY( "sts" ) );
            }
        }
        else
        {
            DBG_ERROR1( "Unable to access library %s while loading testtool support.", SVLIBRARY( "sts" ) );
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
                DBG_ERROR1( "Unable to get Symbol 'CreateEventLogger' from library %s while loading testtool support.", SVLIBRARY( "sts" ) );
            }
        }
        else
        {
            DBG_ERROR1( "Unable to access library %s while loading testtool support.", SVLIBRARY( "sts" ) );
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
