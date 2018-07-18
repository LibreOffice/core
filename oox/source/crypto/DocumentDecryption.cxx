/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <oox/crypto/DocumentDecryption.hxx>

#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <oox/crypto/AgileEngine.hxx>
#include <oox/crypto/Standard2007Engine.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>
#include <oox/ole/olestorage.hxx>

namespace oox {
namespace core {

using namespace css;

DocumentDecryption::DocumentDecryption(oox::ole::OleStorage& rOleStorage) :
    mrOleStorage(rOleStorage),
    mCryptoType(UNKNOWN)
{}

bool DocumentDecryption::generateEncryptionKey(const OUString& rPassword)
{
    if (mEngine)
        return mEngine->generateEncryptionKey(rPassword);
    return false;
}

bool DocumentDecryption::readEncryptionInfo()
{
    if (!mrOleStorage.isStorage())
        return false;

    uno::Reference<io::XInputStream> xEncryptionInfo(mrOleStorage.openInputStream("EncryptionInfo"), uno::UNO_QUERY);

    BinaryXInputStream aBinaryInputStream(xEncryptionInfo, true);
    sal_uInt32 aVersion = aBinaryInputStream.readuInt32();

    switch (aVersion)
    {
        case msfilter::VERSION_INFO_2007_FORMAT:
        case msfilter::VERSION_INFO_2007_FORMAT_SP2:
            mCryptoType = STANDARD_2007; // Set encryption info format
            mEngine.reset(new Standard2007Engine);
            break;
        case msfilter::VERSION_INFO_AGILE:
            mCryptoType = AGILE; // Set encryption info format
            mEngine.reset(new AgileEngine);
            break;
        default:
            break;
    }
    if (mEngine)
        return mEngine->readEncryptionInfo(xEncryptionInfo);
    return false;
}

uno::Sequence<beans::NamedValue> DocumentDecryption::createEncryptionData(const OUString& rPassword)
{
    comphelper::SequenceAsHashMap aEncryptionData;

    if (mCryptoType == AGILE)
    {
        aEncryptionData["CryptoType"] <<= OUString("Agile");
    }
    else if (mCryptoType == STANDARD_2007)
    {
        aEncryptionData["CryptoType"] <<= OUString("Standard");
    }

    aEncryptionData["OOXPassword"] <<= rPassword;
    return aEncryptionData.getAsConstNamedValueList();
}

bool DocumentDecryption::decrypt(const uno::Reference<io::XStream>& xDocumentStream)
{
    bool bResult = false;

    if (!mrOleStorage.isStorage())
        return false;

    // open the required input streams in the encrypted package
    uno::Reference<io::XInputStream> xEncryptedPackage(mrOleStorage.openInputStream("EncryptedPackage"), uno::UNO_QUERY);

    // create temporary file for unencrypted package
    uno::Reference<io::XOutputStream> xDecryptedPackage(xDocumentStream->getOutputStream(), uno::UNO_QUERY);
    BinaryXOutputStream aDecryptedPackage(xDecryptedPackage, true);
    BinaryXInputStream aEncryptedPackage(xEncryptedPackage, true);

    bResult = mEngine->decrypt(aEncryptedPackage, aDecryptedPackage);

    xDecryptedPackage->flush();
    aDecryptedPackage.seekToStart();

    if (bResult)
        return mEngine->checkDataIntegrity();

    return bResult;
}

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
