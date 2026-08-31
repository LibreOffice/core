/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <rtl/ref.hxx>

#include <nss/securityenvironment_nssimpl.hxx>

class X509Certificate_AppleImpl;

/** Security environment backed by both the macOS Keychain and NSS.

    NSS stays the XML-DSig engine (canonicalization, digests, verification), while the
    certificate list is extended with Keychain identities and certificate trust falls back to
    the system trust store. Signing with a Keychain identity is done natively via the Security
    framework, so non-exportable keys (smartcards, Secure Enclave, ...) work.
*/
class SecurityEnvironment_AppleImpl : public SecurityEnvironment_NssImpl
{
public:
    SecurityEnvironment_AppleImpl();
    virtual ~SecurityEnvironment_AppleImpl() override;

    virtual cpo::uno::Sequence<css::uno::Reference<css::security::XCertificate>>
        SAL_CALL getPersonalCertificates() override;

    virtual sal_Int32 SAL_CALL
    verifyCertificate(const css::uno::Reference<css::security::XCertificate>& xCert,
                      const cpo::uno::Sequence<css::uno::Reference<css::security::XCertificate>>&
                          intermediateCerts) override;

    virtual sal_Int32 SAL_CALL getCertificateCharacters(
        const css::uno::Reference<css::security::XCertificate>& xCert) override;

    virtual cpo::uno::Sequence<css::uno::Reference<css::security::XCertificate>>
        SAL_CALL buildCertificatePath(
            const css::uno::Reference<css::security::XCertificate>& beginCert) override;

    /// The certificate remembered by buildCertificatePath(), when it was a Keychain one.
    const rtl::Reference<X509Certificate_AppleImpl>& getAppleSigningCertificate() const
    {
        return m_xAppleSigningCertificate;
    }

private:
    /// The last used signing certificate, when its key lives in the Keychain.
    rtl::Reference<X509Certificate_AppleImpl> m_xAppleSigningCertificate;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
