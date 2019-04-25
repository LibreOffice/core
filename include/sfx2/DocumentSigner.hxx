/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <rtl/ustring.hxx>

namespace com::sun::star::security
{
class XCertificate;
}
namespace com::sun::star::uno
{
template <typename> class Reference;
}

namespace sfx2
{
class SFX2_DLLPUBLIC DocumentSigner
{
private:
    OUString m_aUrl;

public:
    DocumentSigner(OUString const& rUrl)
        : m_aUrl(rUrl)
    {
    }

    bool signDocument(css::uno::Reference<css::security::XCertificate> const& rxCertificate);
};

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
