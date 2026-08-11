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

#include <svl/applekeychain.hxx>

#include "nss/x509certificate_nssimpl.hxx"

/** A certificate whose private key lives in the macOS Keychain.

    The DER encoding is parsed with NSS (so all the XCertificate introspection behaves exactly
    like for NSS-database certificates), while the private key operations go through the
    Security framework.
*/
class X509Certificate_AppleImpl : public X509Certificate_NssImpl
{
public:
    /// Takes ownership of the (retained) identity; throws cpo::uno::RuntimeException on failure.
    explicit X509Certificate_AppleImpl(svl::crypto::CFRef<SecIdentityRef> aIdentity);
    virtual ~X509Certificate_AppleImpl() override;

    SecIdentityRef getIdentity() const { return m_aIdentity.get(); }

    /// Returns the (retained) private key of the identity, or a null ref.
    svl::crypto::CFRef<SecKeyRef> copyPrivateKey() const;

private:
    svl::crypto::CFRef<SecIdentityRef> m_aIdentity;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
