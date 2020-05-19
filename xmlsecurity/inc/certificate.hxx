/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_XMLSECURITY_INC_CERTIFICATE_H
#define INCLUDED_XMLSECURITY_INC_CERTIFICATE_H

#include <sal/types.h>

#include <com/sun/star/uno/Sequence.hxx>

namespace svl
{
namespace crypto
{
enum class SignatureMethodAlgorithm;
}
}

namespace xmlsecurity
{
/// Extension of css::security::XCertificate for module-internal purposes.
class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI Certificate
{
public:
    /// Returns the SHA-256 thumbprint.
    ///
    /// @throws css::uno::RuntimeException
    virtual css::uno::Sequence<sal_Int8> getSHA256Thumbprint() = 0;

    /// Same as getSubjectPublicKeyAlgorithm(), but returns an ID, not a string.
    virtual svl::crypto::SignatureMethodAlgorithm getSignatureMethodAlgorithm() = 0;

protected:
    ~Certificate() noexcept = default;
};
}

#endif // INCLUDED_XMLSECURITY_INC_CERTIFICATE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
