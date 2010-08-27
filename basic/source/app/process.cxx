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
#include "precompiled_basic.hxx"


#ifdef WNT
#include    <tools/prewin.h>
#include "winbase.h"
#include    <tools/postwin.h>
#endif
#include <tools/errcode.hxx>
#include <vos/process.hxx>
#include <basic/sbxcore.hxx>
#include <tools/string.hxx>
#include <osl/file.hxx>

#ifndef _BASIC_TTRESHLP_HXX
#include <basic/ttstrhlp.hxx>
#endif

//#ifndef _BYTE_STRING_LIST
//DECLARE_LIST( ByteStringList, ByteString * );
//#define _BYTE_STRING_LIST
//#endif

#include <basic/process.hxx>

Process::Process()
: pArgumentList( NULL )
, pEnvList( NULL )
, pProcess( NULL )
, bWasGPF( FALSE )
, bHasBeenStarted( FALSE )
{
}

Process::~Process()
{
//    delete pArgumentList;
//    delete pEnvList;
    delete pProcess;
}


BOOL Process::ImplIsRunning()
{
    if ( pProcess && bHasBeenStarted )
    {
        vos::OProcess::TProcessInfo aProcessInfo;
        pProcess->getInfo( vos::OProcess::TData_ExitCode, &aProcessInfo );
        if ( !(aProcessInfo.Fields & vos::OProcess::TData_ExitCode) )
            return TRUE;
        else
            return FALSE;
    }
    else
        return FALSE;
}

long Process::ImplGetExitCode()
{
    if ( pProcess )
    {
        vos::OProcess::TProcessInfo aProcessInfo;
        pProcess->getInfo( vos::OProcess::TData_ExitCode, &aProcessInfo );
        if ( !(aProcessInfo.Fields & vos::OProcess::TData_ExitCode) )
            SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
        return aProcessInfo.Code;
    }
    else
        SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
        return 0;
}


////////////////////////////////////////////////////////////////////////////

void Process::SetImage( const String &aAppPath, const String &aAppParams, const Environment *pEnv )
{ // Set image file of executable
    if ( pProcess && ImplIsRunning() )
        SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
    else
    {
        delete pArgumentList; pArgumentList = NULL;
        delete pEnvList; pEnvList = NULL;
        delete pProcess; pProcess = NULL;

        xub_StrLen i, nCount = aAppParams.GetQuotedTokenCount( CUniString("\"\"" ), ' ' );
        ::rtl::OUString *pParamList = new ::rtl::OUString[nCount];

        xub_StrLen nParamCount = 0;
        for ( i = 0 ; i < nCount ; i++ )
        {
            ::rtl::OUString aTemp = ::rtl::OUString(aAppParams.GetQuotedToken( i, CUniString("\"\"''" ), ' ' ));
            if ( aTemp.getLength() )
            {
                pParamList[nParamCount] = aTemp;
                nParamCount++;
            }
        }
        pArgumentList = new vos::OArgumentList( pParamList, nCount );


        ::rtl::OUString *pEnvArray = NULL;
        if ( pEnv )
        {
            pEnvArray = new ::rtl::OUString[pEnv->size()];

            xub_StrLen nEnvCount = 0;
            Environment::const_iterator aIter = pEnv->begin();
            while ( aIter != pEnv->end() )
            {
                ::rtl::OUString aTemp = ::rtl::OUString( (*aIter).first );
                aTemp += ::rtl::OUString::createFromAscii( "=" );
                aTemp += ::rtl::OUString( (*aIter).second );
                pEnvArray[nEnvCount] = aTemp;
                nEnvCount++;
                aIter++;
            }
            pEnvList = new vos::OEnvironment( pEnvArray, nEnvCount );
        }

        ::rtl::OUString aNormalizedAppPath;
        osl::FileBase::getFileURLFromSystemPath( ::rtl::OUString(aAppPath), aNormalizedAppPath );
        pProcess = new vos::OProcess( aNormalizedAppPath );
        bHasBeenStarted = FALSE;

        delete [] pParamList;
        delete [] pEnvArray;
    }
}

BOOL Process::Start()
{ // Start program
    BOOL bSuccess=FALSE;
    if ( pProcess && !ImplIsRunning() )
    {
        bWasGPF = FALSE;
#ifdef WNT
//      sal_uInt32 nErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_NOALIGNMENTFAULTEXCEPT | SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
        sal_uInt32 nErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOGPFAULTERRORBOX);
        try
        {
#endif
            if ( pEnvList )
            {
                bSuccess = pProcess->execute( (vos::OProcess::TProcessOption)
                            ( vos::OProcess::TOption_SearchPath
                            /*| vos::OProcess::TOption_Detached*/
                            /*| vos::OProcess::TOption_Wait*/ ),
                            *pArgumentList,
                            *pEnvList ) == vos::OProcess::E_None;
            }
            else
            {
                bSuccess = pProcess->execute( (vos::OProcess::TProcessOption)
                            ( vos::OProcess::TOption_SearchPath
                            /*| vos::OProcess::TOption_Detached*/
                            /*| vos::OProcess::TOption_Wait*/ ),
                            *pArgumentList ) == vos::OProcess::E_None;
            }
#ifdef WNT
        }
        catch( ... )
        {
            bWasGPF = TRUE;
            // TODO: Output debug message !!
        }
        nErrorMode = SetErrorMode(nErrorMode);
#endif
        bHasBeenStarted = bSuccess;
    }
    else
        SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
    return bSuccess;
}

ULONG Process::GetExitCode()
{ // ExitCode of program after execution
    return ImplGetExitCode();
}

BOOL Process::IsRunning()
{
    return ImplIsRunning();
}

BOOL Process::WasGPF()
{ // Did the process fail?
#ifdef WNT
    return ImplGetExitCode() == 3221225477;
#else
    return bWasGPF;
#endif
}

BOOL Process::Terminate()
{
    if ( ImplIsRunning() )
        return pProcess->terminate() == vos::OProcess::E_None;
    return TRUE;
}

