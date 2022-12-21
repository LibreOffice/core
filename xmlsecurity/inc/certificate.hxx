/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>

#include "xsecxmlsecdllapi.h"

namespace svl
{
namespace crypto
{
enum class SignatureMethodAlgorithm;
}
}

namespace xmlsecurity
{
/// Base class of css::security::XCertificate implementations, for module-internal purposes.
class XSECXMLSEC_DLLPUBLIC Certificate
    : public ::cppu::WeakImplHelper<css::security::XCertificate, css::lang::XUnoTunnel,
                                    css::lang::XServiceInfo>
{
public:
    /// Returns the SHA-256 thumbprint.
    ///
    /// @throws css::uno::RuntimeException
    virtual css::uno::Sequence<sal_Int8> getSHA256Thumbprint() = 0;

    /// Same as getSubjectPublicKeyAlgorithm(), but returns an ID, not a string.
    virtual svl::crypto::SignatureMethodAlgorithm getSignatureMethodAlgorithm() = 0;

    sal_Int64 SAL_CALL getSomething(css::uno::Sequence<sal_Int8> const& aIdentifier) override;
    static css::uno::Sequence<sal_Int8> const& getUnoTunnelId();

protected:
    ~Certificate() noexcept = default;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
