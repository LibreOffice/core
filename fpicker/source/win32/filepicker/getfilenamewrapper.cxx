/*************************************************************************
 *
 *  $RCSfile: getfilenamewrapper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-10-04 11:08:11 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif


#ifndef _GETFILENAMEWRAPPER_HXX_
#include "getfilenamewrapper.hxx"
#endif


#include <process.h>

//-----------------------------------------------
// Simple struct for automatically init/deinit
// of a STA
//-----------------------------------------------

struct STAInit
{
    STAInit( )  { CoInitialize( NULL ); }
    ~STAInit( ) { CoUninitialize( ); }
};

//-----------------------------------------------
// This function returns true if either the
// calling thread belongs to an STA or to no
// apartment at all
//-----------------------------------------------

bool IsThreadContextSTA( )
{
    HRESULT hr = CoInitialize( NULL );
    bool bRet = false;

    if ( SUCCEEDED( hr ) )
    {
        bRet = true;
        CoUninitialize( );
    }

    return bRet;
}

//-----------------------------------------------
// Someone may extend this method on demand and
// even create the appropriate instance depending
// on the OS version etc.
//-----------------------------------------------

CGetFileNameWrapper* CGetFileNameWrapper::create( )
{
    CGetFileNameWrapper* pGetFileNameWrapper = new CSTAGetFileNameWrapper( );

    OSL_POSTCOND( pGetFileNameWrapper, "can't create instance, not enough memory" );

    return pGetFileNameWrapper;
}


//###############################################


//-----------------------------------------------
//
//-----------------------------------------------

CSTAGetFileNameWrapper::CSTAGetFileNameWrapper( )
{
}

//-----------------------------------------------
//
//-----------------------------------------------

CSTAGetFileNameWrapper::~CSTAGetFileNameWrapper( )
{
}

//-----------------------------------------------
//
//-----------------------------------------------

BOOL SAL_CALL CSTAGetFileNameWrapper::getOpenFileName( LPOPENFILENAMEW lpofn )
{
    executeGetFileName( true, lpofn );
    return m_bResult;
}

//-----------------------------------------------
//
//-----------------------------------------------

BOOL SAL_CALL CSTAGetFileNameWrapper::getSaveFileName( LPOPENFILENAMEW lpofn )
{
    executeGetFileName( false, lpofn );
    return m_bResult;
}

//-----------------------------------------------
//
//-----------------------------------------------

DWORD SAL_CALL CSTAGetFileNameWrapper::commDlgExtendedError( )
{
    return m_LastError;
}

//-----------------------------------------------
//
//-----------------------------------------------

void SAL_CALL CSTAGetFileNameWrapper::executeGetFileName( )
{
    if ( m_bFileOpenDialog )
        m_bResult = CGetFileNameWrapper::getOpenFileName( m_lpofn );
    else
        m_bResult = CGetFileNameWrapper::getSaveFileName( m_lpofn );

    if ( !m_bResult )
        m_LastError = CGetFileNameWrapper::commDlgExtendedError( );
}

//-----------------------------------------------
//
//-----------------------------------------------

void SAL_CALL CSTAGetFileNameWrapper::executeGetFileName( BOOL bFileOpenDialog, LPOPENFILENAMEW lpofn )
{
    m_bFileOpenDialog = bFileOpenDialog;
    m_lpofn           = lpofn;
    m_bResult         = false;

    if ( !IsThreadContextSTA( ) && threadExecuteGetFileName( ) )
    {
        return;
    }
    else
    {
        // execute in the context of the calling thread
        // if it lives in an STA or the execution in a
        // separat thread failed
        executeGetFileName( );
    }
}

//-----------------------------------------------
//
//-----------------------------------------------

bool SAL_CALL CSTAGetFileNameWrapper::threadExecuteGetFileName( )
{
    unsigned ThreadId;
    bool bSuccess = false;

    HANDLE hThread = reinterpret_cast< HANDLE >(
        _beginthreadex( 0, 0, CSTAGetFileNameWrapper::threadProc, this, 0, &ThreadId ) );

    OSL_POSTCOND( hThread, "could not create STA thread" );

    if ( hThread )
    {
        // stop the calling thread until the sta thread
        // has ended
        WaitForSingleObject( hThread, INFINITE );
        CloseHandle( hThread );
        bSuccess = true;
    }

    return bSuccess;
}

//-----------------------------------------------
//
//-----------------------------------------------

unsigned __stdcall CSTAGetFileNameWrapper::threadProc( void* pParam )
{
    CSTAGetFileNameWrapper* pImpl =
        reinterpret_cast< CSTAGetFileNameWrapper* >( pParam );

    OSL_ASSERT( pImpl );

    // setup a STA environment
    STAInit staInit;

    pImpl->executeGetFileName( );

    return 0;
}
