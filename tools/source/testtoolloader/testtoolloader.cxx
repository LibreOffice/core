/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testtoolloader.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 12:28:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#include "tools/testtoolloader.hxx"

#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _SOLAR_H
#include "tools/solar.h"
#endif
#ifndef _STRING_HXX
#include "tools/string.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include "tools/debug.hxx"
#endif

using namespace rtl;

namespace tools
{
    typedef void ( *pfunc_CreateRemoteControl)();
    typedef void ( *pfunc_DestroyRemoteControl)();

static oslModule    aTestToolModule = 0;


sal_uInt32 GetCommandLineParamCount()
{
    NAMESPACE_VOS( OStartupInfo ) aStartInfo;
    return aStartInfo.getCommandArgCount();
}

String GetCommandLineParam( sal_uInt32 nParam )
{
    NAMESPACE_VOS( OStartupInfo ) aStartInfo;
    ::rtl::OUString aParam;
    NAMESPACE_VOS( OStartupInfo )::TStartupError eError = aStartInfo.getCommandArg( nParam, aParam );
    if ( eError == NAMESPACE_VOS( OStartupInfo )::E_None )
        return String( aParam );
    else
    {
        DBG_ERROR( "Unable to get CommandLineParam" );
        return String();
    }
}

extern "C" { static void SAL_CALL thisModule() {} }

void InitTestToolLib()
{
    RTL_LOGFILE_CONTEXT( aLog, "desktop (cd100003) ::InitTestToolLib" );

    sal_uInt32 i;
    // are we to be automated at all?
    bool bAutomate = false;

    for ( i = 0 ; i < GetCommandLineParamCount() ; i++ )
    {
        if ( GetCommandLineParam( i ).EqualsIgnoreCaseAscii("/enableautomation")
            || GetCommandLineParam( i ).EqualsIgnoreCaseAscii("-enableautomation"))
        {
            bAutomate = true;
            break;
        }
    }

    if ( !bAutomate )
        return;


    OUString    aFuncName( RTL_CONSTASCII_USTRINGPARAM( "CreateRemoteControl" ));

    aTestToolModule = osl_loadModuleRelative(
        &thisModule,
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SVLIBRARY("sts"))).pData,
        SAL_LOADMODULE_DEFAULT );
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

void DeInitTestToolLib()
{
    if ( aTestToolModule )
    {
        OUString    aFuncName( RTL_CONSTASCII_USTRINGPARAM( "DestroyRemoteControl" ));

        oslGenericFunction pDeInitFunc = osl_getFunctionSymbol(
            aTestToolModule, aFuncName.pData );
        if ( pDeInitFunc )
            (reinterpret_cast< pfunc_DestroyRemoteControl >(pDeInitFunc))();

        osl_unloadModule( aTestToolModule );
    }
}

} // namespace tools
