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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"

#include <rtl/digest.h>
#include <rtl/ref.hxx>

#include "sha1context.hxx"

using namespace ::com::sun::star;

// static
uno::Reference< xml::crypto::XDigestContext > SHA1DigestContext::Create()
{
    ::rtl::Reference< SHA1DigestContext > xResult = new SHA1DigestContext();
    xResult->m_pDigest = rtl_digest_createSHA1();
    if ( !xResult->m_pDigest )
        throw uno::RuntimeException( ::rtl::OUString::createFromAscii( "Can not create cipher!\n" ),
                                     uno::Reference< XInterface >() );

    return uno::Reference< xml::crypto::XDigestContext >( xResult.get() );
}

SHA1DigestContext::~SHA1DigestContext()
{
    if ( m_pDigest )
    {
        rtl_digest_destroySHA1( m_pDigest );
        m_pDigest = NULL;
    }
}

void SAL_CALL SHA1DigestContext::updateDigest( const uno::Sequence< ::sal_Int8 >& aData )
    throw( lang::DisposedException, uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pDigest )
        throw lang::DisposedException();

    if ( rtl_Digest_E_None != rtl_digest_updateSHA1( m_pDigest, aData.getConstArray(), aData.getLength() ) )
    {
        rtl_digest_destroySHA1( m_pDigest );
        m_pDigest = NULL;

        throw uno::RuntimeException();
    }
}

uno::Sequence< ::sal_Int8 > SAL_CALL SHA1DigestContext::finalizeDigestAndDispose()
    throw( lang::DisposedException, uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pDigest )
        throw lang::DisposedException();

    uno::Sequence< sal_Int8 > aResult( RTL_DIGEST_LENGTH_SHA1 );
    if ( rtl_Digest_E_None != rtl_digest_getSHA1( m_pDigest, reinterpret_cast< sal_uInt8* >( aResult.getArray() ), aResult.getLength() ) )
    {
        rtl_digest_destroySHA1( m_pDigest );
        m_pDigest = NULL;

        throw uno::RuntimeException();
    }

    rtl_digest_destroySHA1( m_pDigest );
    m_pDigest = NULL;

    return aResult;
}


