/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_OOX_CRYPTO_DOCUMENTENCRYPTION_HXX
#define INCLUDED_OOX_CRYPTO_DOCUMENTENCRYPTION_HXX

#include <cpo/uno/Reference.hxx>
#include <cpo/uno/Sequence.hxx>

namespace com::sun::star {
    namespace io { class XStream; }
    namespace packages { class XPackageEncryption; }
    namespace beans { struct NamedValue; }
}
namespace cpo::uno { class XComponentContext; }

namespace oox::ole { class OleStorage; }

namespace oox::crypto {

class DocumentEncryption
{
private:
    cpo::uno::Reference< cpo::uno::XComponentContext > mxContext;
    cpo::uno::Reference< css::io::XStream > mxDocumentStream;
    oox::ole::OleStorage& mrOleStorage;

    cpo::uno::Reference< css::packages::XPackageEncryption > mxPackageEncryption;
    const cpo::uno::Sequence< css::beans::NamedValue >& mMediaEncData;

public:
    DocumentEncryption(const cpo::uno::Reference< cpo::uno::XComponentContext >& rxContext,
        cpo::uno::Reference< css::io::XStream > const & xDocumentStream,
        oox::ole::OleStorage& rOleStorage,
        const cpo::uno::Sequence< css::beans::NamedValue >& rMediaEncData);

    bool encrypt();

};

} // namespace oox::crypto

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
