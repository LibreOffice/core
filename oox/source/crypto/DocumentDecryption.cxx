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

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/packages/XPackageEncryption.hpp>
#include <oox/ole/olestorage.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <filter/msfilter/mscodec.hxx>

#include <com/sun/star/task/PasswordRequestMode.hpp>
#include <comphelper/docpasswordrequest.hxx>
#include <comphelper/stillreadwriteinteraction.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/task/PasswordContainer.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

#include <sal/log.hxx>

namespace {

void lcl_getListOfStreams(oox::StorageBase* pStorage, std::vector<OUString>& rElementNames)
{
    std::vector< OUString > oElementNames;
    pStorage->getElementNames(oElementNames);
    for (const auto & sName : oElementNames)
    {
        oox::StorageRef rSubStorage = pStorage->openSubStorage(sName, false);
        if (rSubStorage && rSubStorage->isStorage())
        {
            lcl_getListOfStreams(rSubStorage.get(), rElementNames);
        }
        else
        {
            if (pStorage->isRootStorage())
                rElementNames.push_back(sName);
            else
                rElementNames.push_back(pStorage->getPath() + "/" + sName);
        }
    }
}

}

namespace oox::crypto {

using namespace css;

DocumentDecryption::DocumentDecryption(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
    oox::ole::OleStorage& rOleStorage) :
    mxContext(rxContext),
    mrOleStorage(rOleStorage)
{
    // Get OLE streams into sequences for later use in CryptoEngine
    std::vector< OUString > aStreamNames;
    lcl_getListOfStreams(&mrOleStorage, aStreamNames);

    comphelper::SequenceAsHashMap aStreamsData;
    for (const auto & sStreamName : aStreamNames)
    {
        uno::Reference<io::XInputStream> xStream = mrOleStorage.openInputStream(sStreamName);
        if (!xStream.is())
            throw io::IOException( "Cannot open OLE input stream for " + sStreamName + "!" );

        BinaryXInputStream aBinaryInputStream(xStream, true);

        css::uno::Sequence< sal_Int8 > oData;
        sal_Int32 nStreamSize = aBinaryInputStream.size();
        sal_Int32 nReadBytes = aBinaryInputStream.readData(oData, nStreamSize);

        if (nStreamSize != nReadBytes)
        {
            SAL_WARN("oox", "OLE stream invalid content");
            throw io::IOException( "OLE stream invalid content for " + sStreamName + "!" );
        }

        aStreamsData[sStreamName] <<= oData;
    }
    maStreamsSequence = aStreamsData.getAsConstNamedValueList();
}

bool DocumentDecryption::generateEncryptionKey(const OUString& rPassword)
{
    if (mxPackageEncryption.is())
        return mxPackageEncryption->generateEncryptionKey(rPassword);
    return false;
}

bool DocumentDecryption::readEncryptionInfo()
{
    if (!mrOleStorage.isStorage())
        return false;

    // Read 0x6DataSpaces/DataSpaceMap
    uno::Reference<io::XInputStream> xDataSpaceMap = mrOleStorage.openInputStream("\006DataSpaces/DataSpaceMap");
    OUString sDataSpaceName;

    if (xDataSpaceMap.is())
    {
        BinaryXInputStream aDataSpaceStream(xDataSpaceMap, true);
        sal_uInt32 aHeaderLength = aDataSpaceStream.readuInt32();
        SAL_WARN_IF(aHeaderLength != 8, "oox", "DataSpaceMap length != 8 is not supported. Some content may be skipped");
        sal_uInt32 aEntryCount = aDataSpaceStream.readuInt32();
        SAL_WARN_IF(aEntryCount != 1, "oox", "DataSpaceMap contains more than one entry. Some content may be skipped");

        // Read each DataSpaceMapEntry (MS-OFFCRYPTO 2.1.6.1)
        for (sal_uInt32 i = 0; i < aEntryCount && !aDataSpaceStream.isEof(); i++)
        {
            // entryLen unused for the moment
            aDataSpaceStream.skip(sizeof(sal_uInt32));

            // Read each DataSpaceReferenceComponent (MS-OFFCRYPTO 2.1.6.2)
            sal_uInt32 aReferenceComponentCount = aDataSpaceStream.readuInt32();
            for (sal_uInt32 j = 0; j < aReferenceComponentCount && !aDataSpaceStream.isEof(); j++)
            {
                // Read next reference component
                // refComponentType unused for the moment
                aDataSpaceStream.skip(sizeof(sal_uInt32));
                sal_uInt32 aReferenceComponentNameLength = aDataSpaceStream.readuInt32();
                // sReferenceComponentName unused for the moment
                aDataSpaceStream.readUnicodeArray(aReferenceComponentNameLength / 2);
                aDataSpaceStream.skip((4 - (aReferenceComponentNameLength & 3)) & 3);  // Skip padding
            }

            sal_uInt32 aDataSpaceNameLength = aDataSpaceStream.readuInt32();
            sDataSpaceName = aDataSpaceStream.readUnicodeArray(aDataSpaceNameLength / 2);
            aDataSpaceStream.skip((4 - (aDataSpaceNameLength & 3)) & 3);  // Skip padding
        }

        if (aDataSpaceStream.isEof())
        {
            SAL_WARN("oox", "EOF on parsing DataSpaceMapEntry table");
            return false;
        }
    }
    else
    {
        // Fallback for documents generated by LO: they sometimes do not have all
        // required by MS-OFFCRYPTO specification streams (0x6DataSpaces/DataSpaceMap and others)
        SAL_WARN("oox", "Encrypted package does not contain DataSpaceMap");
        sDataSpaceName = "StrongEncryptionDataSpace";
    }

    uno::Sequence< uno::Any > aArguments;
    mxPackageEncryption.set(
        mxContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.comp.oox.crypto." + sDataSpaceName, aArguments, mxContext), css::uno::UNO_QUERY);

    if (!mxPackageEncryption.is())
    {
        // we do not know how to decrypt this document
        return false;
    }

    return mxPackageEncryption->readEncryptionInfo(maStreamsSequence);
}

uno::Sequence<beans::NamedValue> DocumentDecryption::createEncryptionData(const OUString& rPassword)
{
    if (!mxPackageEncryption.is())
        return uno::Sequence<beans::NamedValue>();

    return mxPackageEncryption->createEncryptionData(rPassword);
}

bool DocumentDecryption::decrypt(const uno::Reference<io::XStream>& xDocumentStream)
{
    bool bResult = false;

    if (!mrOleStorage.isStorage())
        return false;

    if (!mxPackageEncryption.is())
        return false;

    // open the required input streams in the encrypted package
    uno::Reference<io::XInputStream> xEncryptedPackage = mrOleStorage.openInputStream("EncryptedPackage");

    // create temporary file for unencrypted package
    uno::Reference<io::XOutputStream> xDecryptedPackage = xDocumentStream->getOutputStream();

    bResult = mxPackageEncryption->decrypt(xEncryptedPackage, xDecryptedPackage);

    css::uno::Reference<io::XSeekable> xSeekable(xDecryptedPackage, css::uno::UNO_QUERY);
    xSeekable->seek(0);

    if (bResult)
        return mxPackageEncryption->checkDataIntegrity();

    return bResult;
}

} // namespace oox::crypto

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
