/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
, bWasGPF( sal_False )
, bHasBeenStarted( sal_False )
{
}

Process::~Process()
{
//    delete pArgumentList;
//    delete pEnvList;
    delete pProcess;
}


sal_Bool Process::ImplIsRunning()
{
    if ( pProcess && bHasBeenStarted )
    {
        vos::OProcess::TProcessInfo aProcessInfo;
        pProcess->getInfo( vos::OProcess::TData_ExitCode, &aProcessInfo );
        if ( !(aProcessInfo.Fields & vos::OProcess::TData_ExitCode) )
            return sal_True;
        else
            return sal_False;
    }
    else
        return sal_False;
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
        bHasBeenStarted = sal_False;

        delete [] pParamList;
        delete [] pEnvArray;
    }
}

sal_Bool Process::Start()
{ // Start program
    sal_Bool bSuccess=sal_False;
    if ( pProcess && !ImplIsRunning() )
    {
        bWasGPF = sal_False;
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
            bWasGPF = sal_True;
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

sal_uIntPtr Process::GetExitCode()
{ // ExitCode of program after execution
    return ImplGetExitCode();
}

sal_Bool Process::IsRunning()
{
    return ImplIsRunning();
}

sal_Bool Process::WasGPF()
{ // Did the process fail?
#ifdef WNT
    return ImplGetExitCode() == 3221225477;
#else
    return bWasGPF;
#endif
}

sal_Bool Process::Terminate()
{
    if ( ImplIsRunning() )
        return pProcess->terminate() == vos::OProcess::E_None;
    return sal_True;
}

