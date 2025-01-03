/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <functional>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/security/XCertificate.hpp>
#include <com/sun/star/security/CertificateKind.hpp>

#include <sal/types.h>

class SfxViewShell;
namespace svl::crypto
{
class SigningContext;
}

namespace sfx2
{
/// Extension of css::security::XDocumentDigitalSignatures for internal purposes.
class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI SAL_LOPLUGIN_ANNOTATE("crosscast") DigitalSignatures
{
public:
    /// Same as signDocumentWithCertificate(), but passes the xModel as well.
    virtual bool SignModelWithCertificate(const css::uno::Reference<css::frame::XModel>& xModel,
                                          svl::crypto::SigningContext& rSigningContext,
                                          const css::uno::Reference<css::embed::XStorage>& xStorage,
                                          const css::uno::Reference<css::io::XStream>& xStream)
        = 0;

    /// Async replacement for signDocumentContent().
    virtual void SignDocumentContentAsync(const css::uno::Reference<css::embed::XStorage>& xStorage,
                                          const css::uno::Reference<css::io::XStream>& xSignStream,
                                          SfxViewShell* pViewShell,
                                          const std::function<void(bool)>& rCallback)
        = 0;

    /// Async replacement for signScriptingContent().
    virtual void
    SignScriptingContentAsync(const css::uno::Reference<css::embed::XStorage>& rxStorage,
                              const css::uno::Reference<css::io::XStream>& xSignStream,
                              const std::function<void(bool)>& rCallback)
        = 0;

    /// Create a scripting signature before creating a document signature.
    virtual void
    SetSignScriptingContent(const css::uno::Reference<css::io::XStream>& xScriptingSignStream)
        = 0;

    /// View-aware replacement for selectSigningCertificateWithType().
    virtual css::uno::Reference<css::security::XCertificate>
    SelectSigningCertificateWithType(SfxViewShell* pViewShell,
                                     const css::security::CertificateKind certificateKind,
                                     OUString& rDescription)
        = 0;

protected:
    ~DigitalSignatures() noexcept = default;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
