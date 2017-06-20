/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_XMLSECURITY_SOURCE_GPG_X509CERTIFICATE_HXX
#define INCLUDED_XMLSECURITY_SOURCE_GPG_X509CERTIFICATE_HXX

#include <certificate.hxx>

#include <sal/types.h>
#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/SecurityException.hpp>
#include <com/sun/star/security/CertificateKind.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#include <key.h>

class CertificateImpl : public cppu::WeakImplHelper< css::security::XCertificate,
                                                     css::lang::XUnoTunnel >,
                        public xmlsecurity::Certificate
{
private:
    GpgME::Key m_pKey;

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

    //Methods from XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething(const css::uno::Sequence< sal_Int8 >& aIdentifier) override;

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    /// @see xmlsecurity::Certificate::getSHA256Thumbprint().
    virtual css::uno::Sequence<sal_Int8> getSHA256Thumbprint() override;
    virtual css::security::CertificateKind getCertificateKind() override;

    // Helper methods
    void setCertificate(const GpgME::Key& key);
    const GpgME::Key* getCertificate() const;
} ;

#endif // INCLUDED_XMLSECURITY_SOURCE_GPG_X509CERTIFICATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
