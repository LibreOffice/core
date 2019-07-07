/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_OOX_CRYPTO_DOCUMENTENCRYPTION_HXX
#define INCLUDED_OOX_CRYPTO_DOCUMENTENCRYPTION_HXX

#include <oox/dllapi.h>

#include <com/sun/star/uno/Reference.hxx>
#include <oox/crypto/Standard2007Engine.hxx>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace io { class XStream; }
} } }

namespace oox { namespace ole { class OleStorage; } }

namespace oox {
namespace core {

class OOX_DLLPUBLIC DocumentEncryption
{
private:
    css::uno::Reference< css::io::XStream > mxDocumentStream;
    oox::ole::OleStorage& mrOleStorage;
    OUString const maPassword;

    Standard2007Engine mEngine;

public:
    DocumentEncryption(
        css::uno::Reference< css::io::XStream > const & xDocumentStream,
        oox::ole::OleStorage& rOleStorage,
        const OUString& aPassword);

    bool encrypt();

};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
