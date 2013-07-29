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
        throw uno::RuntimeException("Can not create cipher!",
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
