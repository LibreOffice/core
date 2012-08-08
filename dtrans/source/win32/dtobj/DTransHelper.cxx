/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
