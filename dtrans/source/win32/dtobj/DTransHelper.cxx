/*************************************************************************
 *
 *  $RCSfile: DTransHelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-06 12:24:46 $
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

#ifndef _DTRANSHELPER_HXX_
#include "DTransHelper.hxx"
#endif

//------------------------------------------------------------------------
// implementation
//------------------------------------------------------------------------

CStgTransferHelper::CStgTransferHelper( sal_Bool bAutoInit,
                                        HGLOBAL hGlob,
                                        sal_Bool bDelStgOnRelease,
                                        sal_Bool bReleaseStreamOnDestr ) :
    m_lpStream( NULL ),
    m_bDelStgOnRelease( bDelStgOnRelease ),
    m_bReleaseStreamOnDestr( bReleaseStreamOnDestr )
{
    OSL_ASSERT( !(bDelStgOnRelease && !bReleaseStreamOnDestr) );

    if ( bAutoInit )
        init( hGlob, m_bDelStgOnRelease, m_bReleaseStreamOnDestr );
}

//------------------------------------------------------------------------
// dtor
//------------------------------------------------------------------------


CStgTransferHelper::~CStgTransferHelper( )
{
    if ( m_bReleaseStreamOnDestr && m_lpStream )
        m_lpStream->Release( );
}

//------------------------------------------------------------------------
// TransferData into the
//------------------------------------------------------------------------

void SAL_CALL CStgTransferHelper::write( const void* lpData, ULONG cb, ULONG* cbWritten )
{
    HRESULT hr = E_FAIL;

    if ( m_lpStream )
        hr = m_lpStream->Write( lpData, cb, cbWritten );

    if ( FAILED( hr ) )
        throw CStgTransferException( hr );

#ifdef _DEBUG
    HGLOBAL hGlob;
    hr = GetHGlobalFromStream( m_lpStream, &hGlob );
    OSL_ASSERT( SUCCEEDED( hr ) );

    DWORD dwSize = GlobalSize( hGlob );
    LPVOID lpdbgData = GlobalLock( hGlob );
    GlobalUnlock( hGlob );
#endif
}

//------------------------------------------------------------------------
// read
//------------------------------------------------------------------------

void SAL_CALL CStgTransferHelper::read( LPVOID pv, ULONG cb, ULONG* pcbRead )
{
    HRESULT hr = E_FAIL;

    if ( m_lpStream )
        hr = m_lpStream->Read( pv, cb , pcbRead );

    if ( FAILED( hr ) )
        throw CStgTransferException( hr );
}

//------------------------------------------------------------------------
// GetHGlobal
//------------------------------------------------------------------------

HGLOBAL SAL_CALL CStgTransferHelper::getHGlobal( ) const
{
    OSL_ASSERT( m_lpStream );

    HGLOBAL hGlob = NULL;

    if ( m_lpStream )
    {
        HRESULT hr = GetHGlobalFromStream( m_lpStream, &hGlob );
        if ( FAILED( hr ) )
            hGlob = NULL;
    }

    return hGlob;
}

//------------------------------------------------------------------------
// getIStream
//------------------------------------------------------------------------

void SAL_CALL CStgTransferHelper::getIStream( LPSTREAM* ppStream )
{
    OSL_ASSERT( ppStream );
    *ppStream = m_lpStream;
}

//------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------

void SAL_CALL CStgTransferHelper::init( SIZE_T newSize,
                                        sal_uInt32 uiFlags,
                                        sal_Bool bDelStgOnRelease,
                                        sal_Bool bReleaseStreamOnDestr )
{
    OSL_ASSERT( !(bDelStgOnRelease && !bReleaseStreamOnDestr) );

    cleanup( );

    m_bDelStgOnRelease      = bDelStgOnRelease;
    m_bReleaseStreamOnDestr = bReleaseStreamOnDestr;

    HGLOBAL hGlob = GlobalAlloc( uiFlags, newSize );
    if ( NULL == hGlob )
        throw CStgTransferException( STG_E_MEDIUMFULL );

    HRESULT hr = CreateStreamOnHGlobal( hGlob, m_bDelStgOnRelease, &m_lpStream );
    if ( FAILED( hr ) )
    {
        GlobalFree( hGlob );
        m_lpStream = NULL;
        throw CStgTransferException( hr );
    }

#ifdef _DEBUG
    STATSTG statstg;
    hr = m_lpStream->Stat( &statstg, STATFLAG_DEFAULT );
    OSL_ASSERT( SUCCEEDED( hr ) );
#endif
}

//------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------

void SAL_CALL CStgTransferHelper::init( HGLOBAL hGlob,
                                         sal_Bool bDelStgOnRelease,
                                        sal_Bool bReleaseStreamOnDestr )
{
    OSL_ASSERT( !(bDelStgOnRelease && !bReleaseStreamOnDestr) );

    cleanup( );

    m_bDelStgOnRelease      = bDelStgOnRelease;
    m_bReleaseStreamOnDestr = bReleaseStreamOnDestr;

    HRESULT hr = CreateStreamOnHGlobal( hGlob, m_bDelStgOnRelease, &m_lpStream );
    if ( FAILED( hr ) )
        throw CStgTransferException( hr );
}

//------------------------------------------------------------------------
// free the global memory and invalidate the stream pointer
//------------------------------------------------------------------------

void SAL_CALL CStgTransferHelper::cleanup( )
{
    if ( m_lpStream && !m_bDelStgOnRelease )
    {
        HGLOBAL hGlob;
        GetHGlobalFromStream( m_lpStream, &hGlob );
        GlobalFree( hGlob );
    }

    if ( m_lpStream )
    {
        m_lpStream->Release( );
        m_lpStream = NULL;
    }
}

//------------------------------------------------------------------------
// return the size of memory we point to
//------------------------------------------------------------------------

sal_uInt32 SAL_CALL CStgTransferHelper::memSize( ) const
{
    DWORD dwSize = 0;

    if ( NULL != m_lpStream )
    {
        HGLOBAL hGlob;

        GetHGlobalFromStream( m_lpStream, &hGlob );
        dwSize = GlobalSize( hGlob );
    }

    return dwSize;
}

