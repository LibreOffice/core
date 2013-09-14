/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef DOCUMENT_DECRYPTION_HXX
#define DOCUMENT_DECRYPTION_HXX

#include "oox/dllapi.h"

#include "oox/ole/olestorage.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include "CryptTools.hxx"
#include "AgileEngine.hxx"
#include "Standard2007Engine.hxx"

#include <boost/scoped_ptr.hpp>
#include <vector>

namespace oox {
namespace core {

class OOX_DLLPUBLIC DocumentDecryption
{
private:
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > mxContext;

    enum CryptoType
    {
        UNKNOWN,
        STANDARD_2007,
        AGILE
    };

    oox::ole::OleStorage&           mrOleStorage;
    boost::scoped_ptr<CryptoEngine> mEngine;
    CryptoType                      mCryptoType;

    bool readAgileEncryptionInfo( com::sun::star::uno::Reference< com::sun::star::io::XInputStream >& rStream );
    bool readStandard2007EncryptionInfo( BinaryInputStream& rStream );

public:
    DocumentDecryption(
        oox::ole::OleStorage& rOleStorage,
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > xContext);

    bool decrypt(com::sun::star::uno::Reference< com::sun::star::io::XStream > xDocumentStream);
    bool readEncryptionInfo();
    bool generateEncryptionKey(const OUString& rPassword);

    com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > createEncryptionData(const OUString& rPassword);

    static bool checkEncryptionData( const com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue >& rEncryptionData );
};

} // namespace core
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
