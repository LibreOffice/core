/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AutoBuffer.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 10:55:43 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _AUTO_BUFFER_HXX_
#include "AutoBuffer.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

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
