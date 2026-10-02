/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/types.h>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/security/XCertificate.hpp>

namespace xmlsecurity
{
/// Extension of css::xml::crypto::XXMLSignatureTemplate to transport data to lower-level library
class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI SAL_LOPLUGIN_ANNOTATE("crosscast") ISignatureTemplateInternal
{
public:
    virtual css::uno::Reference<css::security::XCertificate> GetSigningCertificate() = 0;

    virtual void
    SetVerifiedCertificate(css::uno::Reference<css::security::XCertificate> const& rCertificate)
        = 0;

protected:
    ~ISignatureTemplateInternal() noexcept = default;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
