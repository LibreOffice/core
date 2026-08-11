/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "x509certificate_appleimpl.hxx"

#include <cpo/uno/RuntimeException.hpp>
#include <sal/log.hxx>

using namespace css;

X509Certificate_AppleImpl::X509Certificate_AppleImpl(svl::crypto::CFRef<SecIdentityRef> aIdentity)
    : m_aIdentity(std::move(aIdentity))
{
    if (!m_aIdentity.is())
        throw cpo::uno::RuntimeException(u"X509Certificate_AppleImpl: no identity"_ustr);

    SecCertificateRef pCertificate = nullptr;
    if (SecIdentityCopyCertificate(m_aIdentity.get(), &pCertificate) != errSecSuccess)
        throw cpo::uno::RuntimeException(u"X509Certificate_AppleImpl: no certificate"_ustr);
    svl::crypto::CFRef<SecCertificateRef> aCertificate(pCertificate);

    svl::crypto::CFRef<CFDataRef> aData(SecCertificateCopyData(aCertificate.get()));
    if (!aData.is())
        throw cpo::uno::RuntimeException(u"X509Certificate_AppleImpl: no DER data"_ustr);

    cpo::uno::Sequence<sal_Int8> aDer(
        reinterpret_cast<const sal_Int8*>(CFDataGetBytePtr(aData.get())),
        CFDataGetLength(aData.get()));
    // Parse with NSS, so introspection behaves identically to NSS-database certificates.
    setRawCert(aDer);
}

X509Certificate_AppleImpl::~X509Certificate_AppleImpl() {}

svl::crypto::CFRef<SecKeyRef> X509Certificate_AppleImpl::copyPrivateKey() const
{
    SecKeyRef pPrivateKey = nullptr;
    OSStatus nStatus = SecIdentityCopyPrivateKey(m_aIdentity.get(), &pPrivateKey);
    if (nStatus != errSecSuccess || !pPrivateKey)
    {
        SAL_WARN("xmlsecurity.xmlsec",
                 "X509Certificate_AppleImpl: SecIdentityCopyPrivateKey failed, status " << nStatus);
        return svl::crypto::CFRef<SecKeyRef>();
    }
    return svl::crypto::CFRef<SecKeyRef>(pPrivateKey);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
