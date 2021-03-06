/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>

namespace com::sun::star::security { class XCertificate; }
namespace GpgME { class Context; }

class SecurityEnvironmentGpg : public cppu::WeakImplHelper< css::xml::crypto::XSecurityEnvironment,
                                                            css::lang::XUnoTunnel >
{
    std::unique_ptr<GpgME::Context> m_ctx;

public:
    SecurityEnvironmentGpg();
    virtual ~SecurityEnvironmentGpg() override;

    //Methods from XSecurityEnvironment
    virtual ::sal_Int32 SAL_CALL verifyCertificate(
        const css::uno::Reference<
        css::security::XCertificate >& xCert,
        const css::uno::Sequence<
        css::uno::Reference< css::security::XCertificate > > &
        intermediateCerts) override;

    virtual ::sal_Int32 SAL_CALL getCertificateCharacters( const css::uno::Reference< css::security::XCertificate >& xCert ) override;

    virtual OUString SAL_CALL getSecurityEnvironmentInformation() override;

    //Methods from XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    static const css::uno::Sequence< sal_Int8 >& getUnoTunnelId() ;

    virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL getPersonalCertificates() override;

    /** We reinterpret the first parameter (originally issuerName) as keyId. We have no other way to identify a gpg key. */
    virtual css::uno::Reference< css::security::XCertificate > SAL_CALL getCertificate( const OUString& keyId, const css::uno::Sequence< sal_Int8 >& serialNumber ) override;

    virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL buildCertificatePath(
        const css::uno::Reference< css::security::XCertificate >& beginCert ) override;

    virtual css::uno::Reference< css::security::XCertificate > SAL_CALL createCertificateFromRaw(
        const css::uno::Sequence< sal_Int8 >& rawCertificate ) override;
    virtual css::uno::Reference< css::security::XCertificate > SAL_CALL createCertificateFromAscii(
        const OUString& asciiCertificate ) override;

    GpgME::Context& getGpgContext() { return *m_ctx; }
    virtual css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > SAL_CALL getAllCertificates() override;

private:
    css::uno::Sequence< css::uno::Reference< css::security::XCertificate > > getCertificatesImpl( bool bPrivateOnly );
} ;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
