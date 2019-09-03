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

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <oox/crypto/AgileEngine.hxx>
#include <oox/crypto/Standard2007Engine.hxx>
#include <oox/crypto/IRMEngine.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>
#include <oox/ole/olestorage.hxx>

namespace oox {
namespace core {

using namespace css;

DocumentDecryption::DocumentDecryption(oox::ole::OleStorage& rOleStorage) :
    mrOleStorage(rOleStorage)
{}

bool DocumentDecryption::generateEncryptionKey(const OUString& rPassword)
{
    if (mEngine)
        return mEngine->generateEncryptionKey(rPassword);
    return false;
}

bool DocumentDecryption::readStrongEncryptionInfo()
{
    uno::Reference<io::XInputStream> xEncryptionInfo = mrOleStorage.openInputStream("EncryptionInfo");

    BinaryXInputStream aBinaryInputStream(xEncryptionInfo, true);
    sal_uInt32 aVersion = aBinaryInputStream.readuInt32();

    switch (aVersion)
    {
    case msfilter::VERSION_INFO_2007_FORMAT:
    case msfilter::VERSION_INFO_2007_FORMAT_SP2:
        msEngineName = "Standard"; // Set encryption info format
        mEngine.reset(new Standard2007Engine);
        break;
    case msfilter::VERSION_INFO_AGILE:
        msEngineName = "Agile"; // Set encryption info format
        mEngine.reset(new AgileEngine);
        break;
    default:
        break;
    }
    if (mEngine)
        return mEngine->readEncryptionInfo(xEncryptionInfo);
    return false;
}

bool DocumentDecryption::readIRMEncryptionInfo()
{
    // Read TransformInfo storage for IRM ECMA documents (MS-OFFCRYPTO 2.2.4)
    uno::Reference<io::XInputStream> xTransformInfoStream = mrOleStorage.openInputStream("\006DataSpaces/TransformInfo/DRMEncryptedTransform/\006Primary");
    SAL_WARN_IF(!xTransformInfoStream.is(), "oox", "TransormInfo stream is missing!");
    BinaryXInputStream aBinaryStream(xTransformInfoStream, true);

    // MS-OFFCRYPTO 2.1.8: TransformInfoHeader
    aBinaryStream.readuInt32();  // TransformLength
    aBinaryStream.readuInt32();  // TransformType
    // TransformId
    sal_uInt32 aStringLength = aBinaryStream.readuInt32();
    OUString sTransformId = aBinaryStream.readUnicodeArray(aStringLength / 2);
    aBinaryStream.skip((4 - (aStringLength & 3)) & 3);  // Skip padding

    // TransformName
    aStringLength = aBinaryStream.readuInt32();
    OUString sTransformName = aBinaryStream.readUnicodeArray(aStringLength / 2);
    aBinaryStream.skip((4 - (aStringLength & 3)) & 3);  // Skip padding

    aBinaryStream.readuInt32();  // ReaderVersion
    aBinaryStream.readuInt32();  // UpdaterVersion
    aBinaryStream.readuInt32();  // WriterVersion

    // MS-OFFCRYPTO 2.2.5: ExtensibilityHeader
    aBinaryStream.readuInt32();  // ExtensibilityHeader

    msEngineName = "IRM"; // Set encryption info format
    mEngine.reset(new IRMEngine);
    return mEngine->readEncryptionInfo(xTransformInfoStream);
}

bool DocumentDecryption::readEncryptionInfo()
{
    if (!mrOleStorage.isStorage())
        return false;

    // Read 0x6DataSpaces/DataSpaceMap
    uno::Reference<io::XInputStream> xDataSpaceMap = mrOleStorage.openInputStream("\006DataSpaces/DataSpaceMap");
    if (xDataSpaceMap.is())
    {
        BinaryXInputStream aDataSpaceStream(xDataSpaceMap, true);
        sal_uInt32 aHeaderLength = aDataSpaceStream.readuInt32();
        SAL_WARN_IF(aHeaderLength != 8, "oox", "DataSpaceMap length != 8 is not supported. Some content may be skipped");
        sal_uInt32 aEntryCount = aDataSpaceStream.readuInt32();
        SAL_WARN_IF(aEntryCount != 1, "oox", "DataSpaceMap contains more than one entry. Some content may be skipped");

        OUString sDataSpaceName;
        // Read each DataSpaceMapEntry (MS-OFFCRYPTO 2.1.6.1)
        for (sal_uInt32 i = 0; i < aEntryCount; i++)
        {
            aDataSpaceStream.readuInt32();  // Entry length

            // Read each DataSpaceReferenceComponent (MS-OFFCRYPTO 2.1.6.2)
            sal_uInt32 aReferenceComponentCount = aDataSpaceStream.readuInt32();
            for (sal_uInt32 j = 0; j < aReferenceComponentCount; j++)
            {
                // Read next reference component
                aDataSpaceStream.readuInt32(); // ReferenceComponentType
                sal_uInt32 aReferenceComponentNameLength = aDataSpaceStream.readuInt32();
                OUString sReferenceComponentName = aDataSpaceStream.readUnicodeArray(aReferenceComponentNameLength / 2);
                aDataSpaceStream.skip((4 - (aReferenceComponentNameLength & 3)) & 3);  // Skip padding
            }

            sal_uInt32 aDataSpaceNameLength = aDataSpaceStream.readuInt32();
            sDataSpaceName = aDataSpaceStream.readUnicodeArray(aDataSpaceNameLength / 2);
            aDataSpaceStream.skip((4 - (aDataSpaceNameLength & 3)) & 3);  // Skip padding
        }

        if (sDataSpaceName == "DRMEncryptedDataSpace")
        {
            return readIRMEncryptionInfo();
        }
        else if (sDataSpaceName == "\011DRMDataSpace") // 0x09DRMDataSpace
        {
            // TODO: IRM binary file
        }
        else if (sDataSpaceName == "StrongEncryptionDataSpace")
        {
            return readStrongEncryptionInfo();
        }
        else
        {
            SAL_WARN("oox", "Unknown dataspace - document will be not decrypted!");
        }
    }
    else
    {
        // Fallback for documents generated by LO: they sometimes do not have all
        // required by MS-OFFCRYPTO specification streams (0x6DataSpaces/DataSpaceMap and others)
        SAL_WARN("oox", "Encrypted package does not contain DataSpaceMap");
        return readStrongEncryptionInfo();
    }
    return false;
}

uno::Sequence<beans::NamedValue> DocumentDecryption::createEncryptionData(const OUString& rPassword)
{
    comphelper::SequenceAsHashMap aEncryptionData;

    aEncryptionData["CryptoType"] <<= msEngineName;
    mEngine->createEncryptionData(aEncryptionData, rPassword);

    return aEncryptionData.getAsConstNamedValueList();
}

bool DocumentDecryption::decrypt(const uno::Reference<io::XStream>& xDocumentStream)
{
    bool bResult = false;

    if (!mrOleStorage.isStorage())
        return false;

    // open the required input streams in the encrypted package
    uno::Reference<io::XInputStream> xEncryptedPackage = mrOleStorage.openInputStream("EncryptedPackage");

    // create temporary file for unencrypted package
    uno::Reference<io::XOutputStream> xDecryptedPackage = xDocumentStream->getOutputStream();
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
