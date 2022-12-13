/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#include <cppuhelper/implbase.hxx>
#include <sal/types.h>
#include <sfx2/dllapi.h>

namespace sfx2
{
/// Extension of css::security::XDocumentDigitalSignatures for internal purposes.
class SFX2_DLLPUBLIC DigitalSignatures : public cppu::WeakImplHelper<css::lang::XUnoTunnel>
{
public:
    /// Same as signDocumentWithCertificate(), but passes the xModel as well.
    virtual bool
    SignModelWithCertificate(const css::uno::Reference<css::frame::XModel>& xModel,
                             const css::uno::Reference<css::security::XCertificate>& xCertificate,
                             const css::uno::Reference<css::embed::XStorage>& xStorage,
                             const css::uno::Reference<css::io::XStream>& xStream)
        = 0;

    sal_Int64 SAL_CALL getSomething(css::uno::Sequence<sal_Int8> const& aIdentifier) override;
    static css::uno::Sequence<sal_Int8> const& getUnoTunnelId();

protected:
    DigitalSignatures();
    ~DigitalSignatures() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
