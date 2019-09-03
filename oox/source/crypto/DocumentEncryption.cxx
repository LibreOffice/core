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
#include <oox/crypto/Standard2007Engine.hxx>
#include <oox/crypto/IRMEngine.hxx>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>
#include <oox/ole/olestorage.hxx>

namespace oox {
namespace core {

using namespace css::io;
using namespace css::uno;

DocumentEncryption::DocumentEncryption(Reference<XStream> const & xDocumentStream,
                                       oox::ole::OleStorage& rOleStorage,
                                       Sequence<css::beans::NamedValue>& rMediaEncData)
    : mxDocumentStream(xDocumentStream)
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
            if (sCryptoType == "IRM")
            {
                mEngine.reset(new IRMEngine);
            }
            else
            {
                mEngine.reset(new Standard2007Engine);
            }
        }
    }
}

bool DocumentEncryption::encrypt()
{
    Reference<XInputStream> xInputStream (mxDocumentStream->getInputStream(), UNO_SET_THROW);
    Reference<XSeekable> xSeekable(xInputStream, UNO_QUERY);

    if (!xSeekable.is())
        return false;

    sal_uInt32 aLength = xSeekable->getLength(); // check length of the stream
    xSeekable->seek(0); // seek to begin of the document stream

    if (!mrOleStorage.isStorage())
        return false;

    mEngine->setupEncryption(mMediaEncData);

    Reference<XOutputStream> xOutputStream(mrOleStorage.openOutputStream("EncryptedPackage"), UNO_SET_THROW);
    mEngine->encrypt(xInputStream, xOutputStream, aLength);
    xOutputStream->flush();
    xOutputStream->closeOutput();

    mEngine->writeEncryptionInfo(mrOleStorage);

    return true;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
