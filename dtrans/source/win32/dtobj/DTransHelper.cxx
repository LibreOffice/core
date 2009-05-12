/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DTransHelper.cxx,v $
 * $Revision: 1.14 $
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
#include "precompiled_dtrans.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------
#include <rtl/ustring.h>
#include <osl/diagnose.h>
#include "DTransHelper.hxx"

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
                dwSize = rtl_ustr_getLength( pText ) * sizeof( sal_Unicode );
                GlobalUnlock( hGlob );
            }
        }
        else
            dwSize = GlobalSize( hGlob );
    }

    return dwSize;
}

