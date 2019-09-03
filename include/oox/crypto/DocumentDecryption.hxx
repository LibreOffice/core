/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_OOX_CRYPTO_DOCUMENTDECRYPTION_HXX
#define INCLUDED_OOX_CRYPTO_DOCUMENTDECRYPTION_HXX

#include <oox/dllapi.h>

#include <memory>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

namespace com::sun::star {
    namespace beans { struct NamedValue; }
    namespace io { class XInputStream; }
    namespace io { class XStream; }
    namespace uno { class XComponentContext; }
    namespace packages { class XPackageEncryption; }
}

namespace oox::ole { class OleStorage; }

namespace oox {
namespace crypto {

class DocumentDecryption
{
private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    oox::ole::OleStorage&                      mrOleStorage;
    css::uno::Sequence<css::beans::NamedValue> maStreamsSequence;
    css::uno::Reference< css::packages::XPackageEncryption > mxPackageEncryption;

public:
    DocumentDecryption(const css::uno::Reference< css::uno::XComponentContext >& rxContext, oox::ole::OleStorage& rOleStorage);

    bool decrypt(const css::uno::Reference< css::io::XStream >& xDocumentStream);
    bool readEncryptionInfo();
    bool generateEncryptionKey(const OUString& rPassword);

    css::uno::Sequence< css::beans::NamedValue > createEncryptionData(const OUString& rPassword);

};

} // namespace crypto
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
