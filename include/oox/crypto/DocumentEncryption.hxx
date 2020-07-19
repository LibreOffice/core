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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace com::sun::star {
    namespace io { class XStream; }
    namespace packages { class XPackageEncryption; }
    namespace beans { struct NamedValue; }
    namespace uno { class XComponentContext; }
}

namespace oox::ole { class OleStorage; }

namespace oox::crypto {

class DocumentEncryption
{
private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::io::XStream > mxDocumentStream;
    oox::ole::OleStorage& mrOleStorage;

    css::uno::Reference< css::packages::XPackageEncryption > mxPackageEncryption;
    const css::uno::Sequence< css::beans::NamedValue >& mMediaEncData;

public:
    DocumentEncryption(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        css::uno::Reference< css::io::XStream > const & xDocumentStream,
        oox::ole::OleStorage& rOleStorage,
        const css::uno::Sequence< css::beans::NamedValue >& rMediaEncData);

    bool encrypt();

};

} // namespace oox::crypto

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
