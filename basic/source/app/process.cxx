/*************************************************************************
 *
 *  $RCSfile: process.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:09 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifdef WNT
#include    <tools/prewin.h>
#include "winbase.h"
#include    <tools/postwin.h>
#endif

#ifndef _ERRCODE_HXX //autogen
#include <tools/errcode.hxx>
#endif
#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif
#ifndef _SBXCORE_HXX
#include <svtools/sbxcore.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#ifndef _BASIC_TTRESHLP_HXX
#include "ttstrhlp.hxx"
#endif

//#ifndef _BYTE_STRING_LIST
//DECLARE_LIST( ByteStringList, ByteString * );
//#define _BYTE_STRING_LIST
//#endif

#include "process.hxx"

// Konstruktor für den Process
Process::Process()
: bWasGPF( FALSE )
, pArgumentList( NULL )
, pProcess( NULL )
, bHasBeenStarted( FALSE )
{
}

// Destruktor
Process::~Process()
{
    delete pArgumentList;
    delete pProcess;
}


BOOL Process::ImplIsRunning()
{
    if ( pProcess && bHasBeenStarted )
    {
        NAMESPACE_VOS(OProcess::TProcessInfo) aProcessInfo;
        pProcess->getInfo( NAMESPACE_VOS(OProcess::TData_ExitCode), &aProcessInfo );
        if ( !(aProcessInfo.Fields & NAMESPACE_VOS(OProcess::TData_ExitCode)) )
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
        NAMESPACE_VOS(OProcess::TProcessInfo) aProcessInfo;
        pProcess->getInfo( NAMESPACE_VOS(OProcess::TData_ExitCode), &aProcessInfo );
        if ( !(aProcessInfo.Fields & NAMESPACE_VOS(OProcess::TData_ExitCode)) )
            SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
        return aProcessInfo.Code;
    }
    else
        SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
        return 0;
}


////////////////////////////////////////////////////////////////////////////

// Die Methoden:
void Process::SetImage( const String &aAppPath, const String &aAppParams )
{ // Imagedatei des Executables
    if ( pProcess && ImplIsRunning() )
        SbxBase::SetError( SbxERR_NO_ACTIVE_OBJECT );
    else
    {
        delete pArgumentList;
        delete pProcess;

        xub_StrLen i, nCount = aAppParams.GetQuotedTokenCount( CUniString("\"\"" ), ' ' );
        NAMESPACE_RTL(OUString) *pParamList = new NAMESPACE_RTL(OUString)[nCount];
        for ( i = 0 ; i < nCount ; i++ )
        {
            NAMESPACE_RTL(OUString) aTemp = NAMESPACE_RTL(OUString)(aAppParams.GetQuotedToken( i, CUniString("\"\"" ), ' ' ));
            if ( aTemp.getLength() )
                pParamList[i] = aTemp;
        }
        pArgumentList = new NAMESPACE_VOS(OArgumentList)( pParamList, nCount );
        NAMESPACE_RTL(OUString) aNormalizedAppPath;
        osl::FileBase::normalizePath( NAMESPACE_RTL(OUString)(aAppPath), aNormalizedAppPath );
        pProcess = new NAMESPACE_VOS(OProcess)( aNormalizedAppPath );
        bHasBeenStarted = FALSE;
    }
}

BOOL Process::Start()
{ // Programm wird gestartet
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
            bSuccess = pProcess->execute( (NAMESPACE_VOS(OProcess)::TProcessOption)
                        ( NAMESPACE_VOS(OProcess)::TOption_SearchPath
                        /*| NAMESPACE_VOS(OProcess)::TOption_Detached*/
                        /*| NAMESPACE_VOS(OProcess)::TOption_Wait*/ ),
                        *pArgumentList ) == NAMESPACE_VOS(OProcess)::E_None;
#ifdef WNT
        }
        catch( ... )
        {
            bWasGPF = TRUE;
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
{ // ExitCode des Programms(nachdem es beendet ist)
    return ImplGetExitCode();
}

BOOL Process::IsRunning()
{ // Programm läuft noch
    return ImplIsRunning();
}

BOOL Process::WasGPF()
{ // Programm mit GPF o.ä. abgebrochen
#ifdef WNT
    return ImplGetExitCode() == 3221225477;
#else
    return bWasGPF;
#endif
}

