/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SFX2_DOCUMENTSIGNER_HXX
#define INCLUDED_SFX2_DOCUMENTSIGNER_HXX

#include <sal/config.h>
#include <sfx2/dllapi.h>

#include <memory>

#include <com/sun/star/security/XCertificate.hpp>

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

#endif // INCLUDED_SFX2_DOCUMENTSIGNER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
