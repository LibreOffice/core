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
#include <com/sun/star/security/XCertificate.hpp>

#include <sal/types.h>

namespace sfx2
{
/// Extension of css::security::XDocumentDigitalSignatures for internal purposes.
class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI DigitalSignatures
{
public:
    /// Same as signDocumentWithCertificate(), but passes the xModel as well.
    virtual bool
    SignModelWithCertificate(const css::uno::Reference<css::frame::XModel>& xModel,
                             const css::uno::Reference<css::security::XCertificate>& xCertificate,
                             const css::uno::Reference<css::embed::XStorage>& xStorage,
                             const css::uno::Reference<css::io::XStream>& xStream)
        = 0;

protected:
    ~DigitalSignatures() noexcept = default;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
