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

#include <sal/config.h>

#include <comphelper/hash.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <rtl/digest.h>
#include <rtl/ref.hxx>

#include "sha1context.hxx"

using namespace ::com::sun::star;

// static
uno::Reference<xml::crypto::XDigestContext> StarOfficeSHA1DigestContext::Create()
{
    ::rtl::Reference<StarOfficeSHA1DigestContext> xResult = new StarOfficeSHA1DigestContext();
    xResult->m_pDigest = rtl_digest_createSHA1();
    if ( !xResult->m_pDigest )
        throw uno::RuntimeException("Can not create cipher!" );

    return xResult;
}

StarOfficeSHA1DigestContext::~StarOfficeSHA1DigestContext()
{
    if ( m_pDigest )
    {
        rtl_digest_destroySHA1( m_pDigest );
        m_pDigest = nullptr;
    }
}

void SAL_CALL StarOfficeSHA1DigestContext::updateDigest(const uno::Sequence<::sal_Int8>& aData)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pDigest )
        throw lang::DisposedException();

    if ( rtl_Digest_E_None != rtl_digest_updateSHA1( m_pDigest, aData.getConstArray(), aData.getLength() ) )
    {
        rtl_digest_destroySHA1( m_pDigest );
        m_pDigest = nullptr;

        throw uno::RuntimeException();
    }
}

uno::Sequence<::sal_Int8> SAL_CALL StarOfficeSHA1DigestContext::finalizeDigestAndDispose()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pDigest )
        throw lang::DisposedException();

    uno::Sequence< sal_Int8 > aResult( RTL_DIGEST_LENGTH_SHA1 );
    if ( rtl_Digest_E_None != rtl_digest_getSHA1( m_pDigest, reinterpret_cast< sal_uInt8* >( aResult.getArray() ), aResult.getLength() ) )
    {
        rtl_digest_destroySHA1( m_pDigest );
        m_pDigest = nullptr;

        throw uno::RuntimeException();
    }

    rtl_digest_destroySHA1( m_pDigest );
    m_pDigest = nullptr;

    return aResult;
}

uno::Reference<xml::crypto::XDigestContext> CorrectSHA1DigestContext::Create()
{
    return new CorrectSHA1DigestContext();
}

struct CorrectSHA1DigestContext::Impl
{
    ::osl::Mutex m_Mutex;
    ::comphelper::Hash m_Hash{::comphelper::HashType::SHA1};
    bool m_bDisposed{false};
};

CorrectSHA1DigestContext::CorrectSHA1DigestContext()
    : m_pImpl(new Impl)
{
}

CorrectSHA1DigestContext::~CorrectSHA1DigestContext()
{
}

void SAL_CALL CorrectSHA1DigestContext::updateDigest(const uno::Sequence<::sal_Int8>& rData)
{
    ::osl::MutexGuard aGuard(m_pImpl->m_Mutex);
    if (m_pImpl->m_bDisposed)
        throw lang::DisposedException();

    m_pImpl->m_Hash.update(reinterpret_cast<unsigned char const*>(rData.getConstArray()), rData.getLength());
}

uno::Sequence<::sal_Int8> SAL_CALL CorrectSHA1DigestContext::finalizeDigestAndDispose()
{
    ::osl::MutexGuard aGuard(m_pImpl->m_Mutex);
    if (m_pImpl->m_bDisposed)
        throw lang::DisposedException();

    m_pImpl->m_bDisposed = true;
    std::vector<unsigned char> const sha1(m_pImpl->m_Hash.finalize());
    return uno::Sequence<sal_Int8>(reinterpret_cast<sal_Int8 const*>(sha1.data()), sha1.size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
