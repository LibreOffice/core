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
#include <oox/crypto/CryptoEngine.hxx>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace beans { struct NamedValue; }
    namespace io { class XInputStream; }
    namespace io { class XStream; }
    namespace uno { class XComponentContext; }
} } }

namespace oox { namespace ole { class OleStorage; } }

namespace oox {
namespace core {

class OOX_DLLPUBLIC DocumentDecryption
{
private:
    oox::ole::OleStorage&                      mrOleStorage;
    css::uno::Sequence<css::beans::NamedValue> maStreamsSequence;
    std::unique_ptr<CryptoEngine>              mEngine;
    OUString                                   msEngineName;
    css::uno::Reference< css::uno::XComponentContext > mxContext;

    void readStrongEncryptionInfo();

public:
    DocumentDecryption(const css::uno::Reference< css::uno::XComponentContext >& rxContext, oox::ole::OleStorage& rOleStorage);

    bool decrypt(const css::uno::Reference< css::io::XStream >& xDocumentStream);
    bool readEncryptionInfo();
    bool generateEncryptionKey(const OUString& rPassword);

    css::uno::Sequence< css::beans::NamedValue > createEncryptionData(const OUString& rPassword);

};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
