/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <oox/crypto/DocumentEncryption.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/packages/XPackageEncryption.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <oox/helper/binaryoutputstream.hxx>
#include <oox/ole/olestorage.hxx>
#include <sal/log.hxx>

namespace oox::crypto {

using namespace css::io;
using namespace css::uno;
using namespace css::beans;

DocumentEncryption::DocumentEncryption(const Reference< XComponentContext >& rxContext,
                                       Reference<XStream> const & xDocumentStream,
                                       oox::ole::OleStorage& rOleStorage,
                                       const Sequence<NamedValue>& rMediaEncData)
    : mxContext(rxContext)
    , mxDocumentStream(xDocumentStream)
    , mrOleStorage(rOleStorage)
    , mMediaEncData(rMediaEncData)
{
    // Select engine
    for (int i = 0; i < rMediaEncData.getLength(); i++)
    {
        if (rMediaEncData[i].Name == "CryptoType")
        {
            OUString sCryptoType;
            rMediaEncData[i].Value >>= sCryptoType;

            if (sCryptoType == "Standard")
                sCryptoType = "StrongEncryptionDataSpace";

            Sequence<Any> aArguments;
            mxPackageEncryption.set(
                mxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    "com.sun.star.comp.oox.crypto." + sCryptoType, aArguments, mxContext), css::uno::UNO_QUERY);

            if (!mxPackageEncryption.is())
            {
                SAL_WARN("oox", "Requested encryption method \"" << sCryptoType << "\" is not supported");
            }

            break;
        }
    }
}

bool DocumentEncryption::encrypt()
{
    if (!mxPackageEncryption.is())
        return false;

    Reference<XInputStream> xInputStream (mxDocumentStream->getInputStream(), UNO_SET_THROW);
    Reference<XSeekable> xSeekable(xInputStream, UNO_QUERY);

    if (!xSeekable.is())
        return false;

    xSeekable->seek(0); // seek to begin of the document stream

    if (!mrOleStorage.isStorage())
        return false;

    mxPackageEncryption->setupEncryption(mMediaEncData);

    Sequence<NamedValue> aStreams = mxPackageEncryption->encrypt(xInputStream);

    for (const NamedValue & aStream : aStreams)
    {
        Reference<XOutputStream> xOutputStream(mrOleStorage.openOutputStream(aStream.Name), UNO_SET_THROW);
        BinaryXOutputStream aBinaryOutputStream(xOutputStream, true);

        css::uno::Sequence<sal_Int8> aStreamSequence;
        aStream.Value >>= aStreamSequence;

        aBinaryOutputStream.writeData(aStreamSequence);

        aBinaryOutputStream.close();
    }

    return true;
}

} // namespace oox::crypto

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
