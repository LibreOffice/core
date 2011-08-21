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

#include <precompiled_xmlsecurity.hxx>

#include <pk11pub.h>
#include "digestcontext.hxx"

using namespace ::com::sun::star;

ODigestContext::~ODigestContext()
{
    if ( m_pContext )
    {
        PK11_DestroyContext( m_pContext, PR_TRUE );
        m_pContext = NULL;
    }
}

void SAL_CALL ODigestContext::updateDigest( const uno::Sequence< ::sal_Int8 >& aData )
    throw (lang::DisposedException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bBroken )
        throw uno::RuntimeException();

    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( !m_b1KData || m_nDigested < 1024 )
    {
        uno::Sequence< sal_Int8 > aToDigest = aData;
        if ( m_b1KData && m_nDigested + aData.getLength() > 1024 )
            aToDigest.realloc( 1024 - m_nDigested );

        if ( PK11_DigestOp( m_pContext, reinterpret_cast< const unsigned char* >( aToDigest.getConstArray() ), aToDigest.getLength() ) != SECSuccess )
        {
            PK11_DestroyContext( m_pContext, PR_TRUE );
            m_pContext = NULL;
            m_bBroken = true;
            throw uno::RuntimeException();
        }

        m_nDigested += aToDigest.getLength();
    }
}

uno::Sequence< ::sal_Int8 > SAL_CALL ODigestContext::finalizeDigestAndDispose()
    throw (lang::DisposedException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bBroken )
        throw uno::RuntimeException();

    if ( m_bDisposed )
        throw lang::DisposedException();

    uno::Sequence< sal_Int8 > aResult( m_nDigestLength );
    unsigned int nResultLen = 0;
    if ( PK11_DigestFinal( m_pContext, reinterpret_cast< unsigned char* >( aResult.getArray() ), &nResultLen, aResult.getLength() ) != SECSuccess )
    {
        PK11_DestroyContext( m_pContext, PR_TRUE );
        m_pContext = NULL;
        m_bBroken = true;
        throw uno::RuntimeException();
    }

    PK11_DestroyContext( m_pContext, PR_TRUE );
    m_pContext = NULL;
    m_bDisposed = true;

    aResult.realloc( nResultLen );
    return aResult;
}

