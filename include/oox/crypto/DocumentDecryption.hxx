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

#include <oox/ole/olestorage.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <oox/crypto/CryptTools.hxx>
#include <oox/crypto/AgileEngine.hxx>
#include <oox/crypto/Standard2007Engine.hxx>

#include <memory>
#include <vector>

namespace oox {
namespace core {

class OOX_DLLPUBLIC DocumentDecryption
{
private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;

    enum CryptoType
    {
        UNKNOWN,
        STANDARD_2007,
        AGILE
    };

    oox::ole::OleStorage&           mrOleStorage;
    std::unique_ptr<CryptoEngine>   mEngine;
    CryptoType                      mCryptoType;

    bool readAgileEncryptionInfo( css::uno::Reference< css::io::XInputStream >& rStream );
    bool readStandard2007EncryptionInfo( BinaryInputStream& rStream );

public:
    DocumentDecryption(
        oox::ole::OleStorage& rOleStorage,
        css::uno::Reference< css::uno::XComponentContext > xContext);

    bool decrypt(css::uno::Reference< css::io::XStream > xDocumentStream);
    bool readEncryptionInfo();
    bool generateEncryptionKey(const OUString& rPassword);

    css::uno::Sequence< css::beans::NamedValue > createEncryptionData(const OUString& rPassword);

};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
