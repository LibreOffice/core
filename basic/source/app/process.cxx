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
#include "precompiled_basic.hxx"


#ifdef WNT
#include <prewin.h>
#include <postwin.h>
#endif
#include <tools/errcode.hxx>
#include <basic/sbxcore.hxx>
#include <tools/string.hxx>
#include <osl/file.hxx>
#include <osl/process.h>

#include <basic/ttstrhlp.hxx>
#include <basic/process.hxx>

Process::Process()
: m_nArgumentCount( 0 )
, m_pArgumentList( NULL )
, m_nEnvCount( 0 )
, m_pEnvList( NULL )
, m_aProcessName()
, m_pProcess( NULL )
, bWasGPF( sal_False )
, bHasBeenStarted( sal_False )
{
}

#define FREE_USTRING_LIST( count, list ) \
    if ( count && list ) \
    { \
        for ( unsigned int i = 0; i < count; ++i ) \
        { \
            rtl_uString_release( list[i] ); \
            list[i] = NULL; \
        } \
        delete[] list; \
    } \
    count = 0; \
    list = NULL;

Process::~Process()
{
    FREE_USTRING_LIST( m_nArgumentCount, m_pArgumentList );
    FREE_USTRING_LIST( m_nEnvCount, m_pEnvList );
    if ( m_pProcess )
        osl_freeProcessHandle( m_pProcess );
}


sal_Bool Process::ImplIsRunning()
{
    if ( m_pProcess && bHasBeenStarted )
    {
        oslProcessInfo aProcessInfo;
        aProcessInfo.Size = sizeof(oslProcessInfo);
        osl_getProcessInfo(m_pProcess, osl_Process_EXITCODE, &aProcessInfo );
        if ( !(aProcessInfo.Fields & osl_Process_EXITCODE) )
            return sal_True;
        else
            return sal_False;
    }
    else
        return sal_False;
}

long Process::ImplGetExitCode()
{
    if ( m_pProcess )
    {
        oslProcessInfo aProcessInfo;
        aProcessInfo.Size = sizeof(oslProcessInfo);
        osl_getProcessInfo(m_pProcess, osl_Process_EXITCODE, &aProcessInfo );
        if ( !(aProcessInfo.Fields & osl_Process_EXITCODE) )
            SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
        return aProcessInfo.Code;
    }
    else
        SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
        return 0;
}



void Process::SetImage( const String &aAppPath, const String &aAppParams, const Environment *pEnv )
{ // Set image file of executable
    if ( m_pProcess && ImplIsRunning() )
        SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
    else
    {
        FREE_USTRING_LIST( m_nArgumentCount, m_pArgumentList );
        FREE_USTRING_LIST( m_nEnvCount, m_pEnvList );
        osl_freeProcessHandle( m_pProcess );
        m_pProcess = NULL;

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
        m_nArgumentCount = nParamCount;
        m_pArgumentList = new rtl_uString*[m_nArgumentCount];
        for ( i = 0 ; i < m_nArgumentCount ; i++ )
        {
            m_pArgumentList[i] = NULL;
            rtl_uString_assign( &(m_pArgumentList[i]), pParamList[i].pData );
        }
        delete [] pParamList;

        if ( pEnv )
        {
            m_pEnvList = new rtl_uString*[pEnv->size()];

            m_nEnvCount = 0;
            Environment::const_iterator aIter = pEnv->begin();
            while ( aIter != pEnv->end() )
            {
                ::rtl::OUString aTemp = ::rtl::OUString( (*aIter).first );
                aTemp += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "=" ));
                aTemp += ::rtl::OUString( (*aIter).second );
                m_pEnvList[m_nEnvCount] = NULL;
                rtl_uString_assign( &(m_pEnvList[m_nEnvCount]), aTemp.pData );
                ++m_nEnvCount;
                ++aIter;
            }
        }

        ::rtl::OUString aNormalizedAppPath;
        osl::FileBase::getFileURLFromSystemPath( ::rtl::OUString(aAppPath), aNormalizedAppPath );
        m_aProcessName = aNormalizedAppPath;;
        bHasBeenStarted = sal_False;

    }
}

sal_Bool Process::Start()
{ // Start program
    sal_Bool bSuccess=sal_False;
    if ( m_aProcessName.getLength() && !ImplIsRunning() )
    {
        bWasGPF = sal_False;
#ifdef WNT
        sal_uInt32 nErrorMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOGPFAULTERRORBOX);
        try
        {
#endif
            bSuccess = osl_executeProcess(
                    m_aProcessName.pData,
                    m_pArgumentList,
                    m_nArgumentCount,
                    osl_Process_SEARCHPATH
                    /*| osl_Process_DETACHED*/
                    /*| osl_Process_WAIT*/,
                    NULL,
                    NULL,
                    m_pEnvList,
                    m_nEnvCount,
                    &m_pProcess ) == osl_Process_E_None;

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
        return osl_terminateProcess(m_pProcess) == osl_Process_E_None;
    return sal_True;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
