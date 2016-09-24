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

namespace xmlsecurity
{

/// Extension of css::security::XCertificate for module-internal purposes.
class SAL_NO_VTABLE SAL_DLLPUBLIC_RTTI Certificate
{
public:

    /// Returns the SHA-256 thumbprint.
    virtual css::uno::Sequence<sal_Int8> getSHA256Thumbprint() throw (css::uno::RuntimeException, std::exception) = 0;

protected:
    ~Certificate() throw () {}
};

}

#endif // INCLUDED_XMLSECURITY_INC_CERTIFICATE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
