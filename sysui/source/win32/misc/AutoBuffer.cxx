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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include "AutoBuffer.hxx"
#include <osl/diagnose.h>

#include <windows.h>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using rtl::OUString;

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CAutoUnicodeBuffer::CAutoUnicodeBuffer( size_t size, sal_Bool bLazyCreation ) :
    m_buffSize( size ),
    m_pBuff( NULL )
{
    if ( !bLazyCreation )
        init( );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CAutoUnicodeBuffer::~CAutoUnicodeBuffer( )
{
    delete [] m_pBuff;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CAutoUnicodeBuffer::resize( size_t new_size )
{
    OSL_ASSERT( new_size >= 0 );

    if ( new_size != m_buffSize )
    {
        if ( new_size > m_buffSize )
        {
            delete [] m_pBuff;
            m_pBuff = NULL;
        }

        m_buffSize = new_size;
    }

    return sal_True;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CAutoUnicodeBuffer::empty( )
{
    if ( m_pBuff )
        ZeroMemory( m_pBuff, m_buffSize * sizeof( sal_Unicode ) );
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Bool SAL_CALL CAutoUnicodeBuffer::fill( const sal_Unicode* pContent, size_t nLen )
{
    OSL_ASSERT( pContent && m_buffSize && (m_buffSize >= nLen) );

    init( );

    sal_Bool bRet = sal_False;

    if ( m_pBuff && pContent && nLen )
    {
        CopyMemory( m_pBuff, pContent, nLen * sizeof( sal_Unicode ) );
        bRet = sal_True;
    }

    return bRet;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

size_t SAL_CALL CAutoUnicodeBuffer::size( ) const
{
    return m_buffSize;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

CAutoUnicodeBuffer::operator sal_Unicode*( )
{
    return m_pBuff;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

sal_Unicode* CAutoUnicodeBuffer::operator&( )
{
    return m_pBuff;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

const sal_Unicode* CAutoUnicodeBuffer::operator&( ) const
{
    return m_pBuff;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void SAL_CALL CAutoUnicodeBuffer::init( )
{
    if ( !m_pBuff && (m_buffSize > 0) )
        m_pBuff = new sal_Unicode[ m_buffSize ];

    empty( );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
