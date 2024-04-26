/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <certificate.hxx>

#include <sal/types.h>
#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/security/CertificateKind.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#endif
#include <key.h>
#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic pop
#endif

class CertificateImpl : public cppu::WeakImplHelper< css::security::XCertificate,
                                                     css::lang::XServiceInfo >,
                        public xmlsecurity::Certificate
{
private:
    GpgME::Key m_pKey;
    std::shared_ptr<GpgME::Context> m_pContext;
    css::uno::Sequence< sal_Int8 > m_aBits;

public:
    CertificateImpl();
    virtual ~CertificateImpl() override;

    //Methods from XCertificate
    virtual sal_Int16 SAL_CALL getVersion() override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getSerialNumber() override;

    virtual OUString SAL_CALL getIssuerName() override;
    virtual OUString SAL_CALL getSubjectName() override;

    virtual css::util::DateTime SAL_CALL getNotValidBefore() override;
    virtual css::util::DateTime SAL_CALL getNotValidAfter() override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getIssuerUniqueID() override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getSubjectUniqueID() override;

    virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificateExtension > > SAL_CALL getExtensions() override;

    virtual css::uno::Reference< css::security::XCertificateExtension > SAL_CALL findCertificateExtension(const css::uno::Sequence< sal_Int8 >& oid) override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getEncoded() override;

    virtual OUString SAL_CALL getSubjectPublicKeyAlgorithm() override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getSubjectPublicKeyValue() override;

    virtual OUString SAL_CALL getSignatureAlgorithm() override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getSHA1Thumbprint() override;

    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getMD5Thumbprint() override;

    virtual sal_Int32 SAL_CALL getCertificateUsage() override;

    /// @see xmlsecurity::Certificate::getSHA256Thumbprint().
    virtual css::uno::Sequence<sal_Int8> getSHA256Thumbprint() override;
    /// @see xmlsecurity::Certificate::getSignatureMethodAlgorithm().
    virtual svl::crypto::SignatureMethodAlgorithm getSignatureMethodAlgorithm() override;
    virtual css::security::CertificateKind SAL_CALL getCertificateKind() override;

    // Helper methods
    void setCertificate(std::shared_ptr<GpgME::Context> ctx, const GpgME::Key& key);
    const GpgME::Key* getCertificate() const;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;
} ;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
