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
