/*************************************************************************
 *
 *  $RCSfile: AutoBuffer.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-06-28 11:11:06 $
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

#ifndef _AUTO_BUFFER_HXX_
#include "AutoBuffer.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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
