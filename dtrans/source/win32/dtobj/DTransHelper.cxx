/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DTransHelper.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:04:07 $
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
                                        sal_Bool bDelStgOnRelease ) :
    m_lpStream( NULL ),
    m_bDelStgOnRelease( bDelStgOnRelease )
{
    if ( bAutoInit )
        init( hGlob, m_bDelStgOnRelease );
}

//------------------------------------------------------------------------
// dtor
//------------------------------------------------------------------------


CStgTransferHelper::~CStgTransferHelper( )
{
    if ( m_lpStream )
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

#if OSL_DEBUG_LEVEL > 0
    HGLOBAL hGlob;
    hr = GetHGlobalFromStream( m_lpStream, &hGlob );
    OSL_ASSERT( SUCCEEDED( hr ) );

    /*DWORD dwSize =*/ GlobalSize( hGlob );
    /*LPVOID lpdbgData =*/ GlobalLock( hGlob );
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
    if ( *ppStream )
        static_cast< LPUNKNOWN >( *ppStream )->AddRef( );
}

//------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------

void SAL_CALL CStgTransferHelper::init( SIZE_T newSize,
                                        sal_uInt32 uiFlags,
                                        sal_Bool bDelStgOnRelease )
{
    cleanup( );

    m_bDelStgOnRelease      = bDelStgOnRelease;

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

#if OSL_DEBUG_LEVEL > 0
    STATSTG statstg;
    hr = m_lpStream->Stat( &statstg, STATFLAG_DEFAULT );
    OSL_ASSERT( SUCCEEDED( hr ) );
#endif
}

//------------------------------------------------------------------------
// Init
//------------------------------------------------------------------------

void SAL_CALL CStgTransferHelper::init( HGLOBAL hGlob,
                                         sal_Bool bDelStgOnRelease )
{
    cleanup( );

    m_bDelStgOnRelease      = bDelStgOnRelease;

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

sal_uInt32 SAL_CALL CStgTransferHelper::memSize( CLIPFORMAT cf ) const
{
    DWORD dwSize = 0;

    if ( NULL != m_lpStream )
    {
        HGLOBAL hGlob;
        GetHGlobalFromStream( m_lpStream, &hGlob );

        if ( CF_TEXT == cf || RegisterClipboardFormat( "HTML Format" ) == cf )
        {
            sal_Char* pText = static_cast< sal_Char* >( GlobalLock( hGlob ) );
            if ( pText )
            {
                dwSize = strlen(pText) + 1; // strlen + trailing '\0'
                GlobalUnlock( hGlob );
            }
        }
        else if ( CF_UNICODETEXT == cf )
        {
            sal_Unicode* pText = static_cast< sal_Unicode* >( GlobalLock( hGlob ) );
            if ( pText )
            {
                dwSize = wcslen( pText ) * sizeof( sal_Unicode );
                GlobalUnlock( hGlob );
            }
        }
        else
            dwSize = GlobalSize( hGlob );
    }

    return dwSize;
}

