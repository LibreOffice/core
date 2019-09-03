/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <oox/crypto/IRMEngine.hxx>

#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/binaryoutputstream.hxx>

#include <sax/tools/converter.hxx>

#include <comphelper/hash.hxx>
#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/random.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/sequenceashashmap.hxx>

#include <filter/msfilter/mscodec.hxx>
#include <tools/stream.hxx>
#include <tools/XmlWriter.hxx>

#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/sax/XFastTokenHandler.hpp>
#include <com/sun/star/xml/sax/FastParser.hpp>
#include <com/sun/star/xml/sax/FastToken.hpp>

#include <msipc.h>

using namespace css;
using namespace css::beans;
using namespace css::io;
using namespace css::lang;
using namespace css::uno;
using namespace css::xml::sax;
using namespace css::xml;

namespace oox
{
namespace core
{
IRMEngine::IRMEngine() {}

bool IRMEngine::checkDataIntegrity() { return true; }

bool IRMEngine::decrypt(BinaryXInputStream& aInputStream, BinaryXOutputStream& aOutputStream)
{
    aInputStream.readInt64(); // Skip stream size

    HRESULT hr = IpcInitialize();
    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED))
    {
        // ERROR_ALREADY_INITIALIZED not an error
        // TODO: some reaction?
    }

    // Get decryption key
    IPC_BUFFER licenseBuffer;
    IPC_KEY_HANDLE key;
    licenseBuffer.pvBuffer = (void*)mInfo.license.getStr();
    licenseBuffer.cbBuffer = mInfo.license.getLength();
    hr = IpcGetKey(&licenseBuffer, 0, NULL, NULL, &key);
    if (FAILED(hr))
    {
        // TODO: some reaction?
    }

    // Get size of decrypt block
    DWORD* blockSize;
    hr = IpcGetKeyProperty(key, IPC_KI_BLOCK_SIZE, nullptr, (LPVOID*)&blockSize);
    if (FAILED(hr))
    {
        // TODO: some reaction?
    }

    char* pEncryptedBuffer = new char[*blockSize];
    char* pDecryptedBuffer = new char[*blockSize];
    int blockNo = 0;
    bool lastBlock = false;

    do
    {
        sal_uInt32 readBytes = aInputStream.readArray(pEncryptedBuffer, *blockSize);
        lastBlock = readBytes != *blockSize;
        DWORD bytes = 0;
        hr = IpcDecrypt(key, blockNo, lastBlock, (PBYTE)pEncryptedBuffer, *blockSize,
                        (PBYTE)pDecryptedBuffer, *blockSize, &bytes);

        if (FAILED(hr))
        {
            // TODO: some reaction?
        }

        aOutputStream.writeArray(pDecryptedBuffer, bytes);

        blockNo++;
    } while (!lastBlock);

    delete[] pEncryptedBuffer;
    delete[] pDecryptedBuffer;

    return true;
}

void IRMEngine::createEncryptionData(comphelper::SequenceAsHashMap& aEncryptionData,
                                     const OUString rPassword)
{
    aEncryptionData["OOXPassword"] <<= rPassword;

    css::uno::Sequence<sal_uInt8> seq;
    seq.realloc(mInfo.license.getLength());
    memcpy(seq.getArray(), mInfo.license.getStr(), mInfo.license.getLength());

    aEncryptionData["license"] <<= seq;
}

bool IRMEngine::readEncryptionInfo(uno::Reference<io::XInputStream>& rxInputStream)
{
    // MS-OFFCRYPTO 2.2.6: XrMLLicense
    BinaryXInputStream aBinaryStream(rxInputStream, true);
    sal_uInt32 aStringLength = aBinaryStream.readuInt32();
    mInfo.license = aBinaryStream.readCharArray(aStringLength);

    if (mInfo.license.getLength()
        && static_cast<sal_uChar>(mInfo.license[0]) != 0x0ef) // BOM is missing?
    {
        mInfo.license = "\x0ef\x0bb\x0bf" + mInfo.license;
    }

    // TODO: CHECK info data

    return true;
}

bool IRMEngine::setupEncryption(css::uno::Sequence<css::beans::NamedValue>& rMediaEncData)
{
    for (int i = 0; i < rMediaEncData.getLength(); i++)
    {
        if (rMediaEncData[i].Name == "license")
        {
            css::uno::Sequence<sal_uInt8> seq;
            rMediaEncData[i].Value >>= seq;
            mInfo.license = OString(reinterpret_cast<sal_Char*>(seq.getArray()), seq.getLength());
        }
    }

    return true;
}

void IRMEngine::writeEncryptionInfo(oox::ole::OleStorage& rOleStorage)
{
    // Write 0x6DataSpaces/DataSpaceMap
    Reference<XOutputStream> xDataSpaceMap(
        rOleStorage.openOutputStream("\006DataSpaces/DataSpaceMap"), UNO_SET_THROW);
    BinaryXOutputStream aDataSpaceMapStream(xDataSpaceMap, false);

    aDataSpaceMapStream.WriteInt32(8); // Header length
    aDataSpaceMapStream.WriteInt32(1); // Entries count

    // DataSpaceMapEntry (MS-OFFCRYPTO 2.1.6.1)
    OUString sDataSpaceName("DRMEncryptedDataSpace");
    OUString sReferenceComponent("EncryptedPackage");

    aDataSpaceMapStream.WriteInt32(0x58); // Length
    aDataSpaceMapStream.WriteInt32(1); // References count
    aDataSpaceMapStream.WriteInt32(0); // References component type

    aDataSpaceMapStream.WriteInt32(sReferenceComponent.getLength() * 2);
    aDataSpaceMapStream.writeUnicodeArray(sReferenceComponent);
    while (aDataSpaceMapStream.tell() % 4) // Padding
    {
        aDataSpaceMapStream.writeValue<sal_Char>(0);
    }

    aDataSpaceMapStream.WriteInt32(sDataSpaceName.getLength() * 2);
    aDataSpaceMapStream.writeUnicodeArray(sDataSpaceName);
    while (aDataSpaceMapStream.tell() % 4) // Padding
    {
        aDataSpaceMapStream.writeValue<sal_Char>(0);
    }

    // Write length
    sal_uInt32 nLength = aDataSpaceMapStream.tell() - 8;
    aDataSpaceMapStream.seek(8);
    aDataSpaceMapStream.WriteInt32(nLength);

    aDataSpaceMapStream.close();
    xDataSpaceMap->flush();
    xDataSpaceMap->closeOutput();

    // Write 0x6DataSpaces/Version
    Reference<XOutputStream> xVersion(rOleStorage.openOutputStream("\006DataSpaces/Version"),
                                      UNO_SET_THROW);
    BinaryXOutputStream aVersionStream(xVersion, false);

    OUString sFeatureIdentifier("Microsoft.Container.DataSpaces");
    aVersionStream.WriteInt32(sFeatureIdentifier.getLength() * 2);
    aVersionStream.writeUnicodeArray(sFeatureIdentifier);
    while (aVersionStream.tell() % 4) // Padding
    {
        aVersionStream.writeValue<sal_Char>(0);
    }

    aVersionStream.WriteInt32(1); // Reader version
    aVersionStream.WriteInt32(1); // Updater version
    aVersionStream.WriteInt32(1); // Writer version

    aVersionStream.close();
    xVersion->flush();
    xVersion->closeOutput();

    // Write 0x6DataSpaces/DataSpaceInfo/[dataspacename]
    OUString sStreamName = "\006DataSpaces/DataSpaceInfo/" + sDataSpaceName;
    Reference<XOutputStream> xDataSpaceInfo(rOleStorage.openOutputStream(sStreamName),
                                            UNO_SET_THROW);
    BinaryXOutputStream aDataSpaceInfoStream(xDataSpaceInfo, false);

    aDataSpaceInfoStream.WriteInt32(8); // Header length
    aDataSpaceInfoStream.WriteInt32(1); // Entries count

    OUString sTransformName("DRMEncryptedTransform");
    aDataSpaceInfoStream.WriteInt32(sTransformName.getLength() * 2);
    aDataSpaceInfoStream.writeUnicodeArray(sTransformName);
    while (aDataSpaceInfoStream.tell() % 4) // Padding
    {
        aDataSpaceInfoStream.writeValue<sal_Char>(0);
    }

    aDataSpaceInfoStream.close();
    xDataSpaceInfo->flush();
    xDataSpaceInfo->closeOutput();

    // Write 0x6DataSpaces/TransformInfo/[transformname]
    sStreamName = "\006DataSpaces/TransformInfo/" + sTransformName + "/\006Primary";
    Reference<XOutputStream> xTransformInfo(rOleStorage.openOutputStream(sStreamName),
                                            UNO_SET_THROW);
    BinaryXOutputStream aTransformInfoStream(xTransformInfo, false);

    // MS-OFFCRYPTO 2.1.8: TransformInfoHeader
    aTransformInfoStream.WriteInt32(0); // TransformLength, will be written later
    aTransformInfoStream.WriteInt32(1); // TransformType

    // TransformId
    OUString sTransformId("{C73DFACD-061F-43B0-8B64-0C620D2A8B50}");
    aTransformInfoStream.WriteInt32(sTransformId.getLength() * 2);
    aTransformInfoStream.writeUnicodeArray(sTransformId);
    while (aTransformInfoStream.tell() % 4) // Padding
    {
        aTransformInfoStream.writeValue<sal_Char>(0);
    }

    // Calculate length and write it into beginning
    nLength = aTransformInfoStream.tell();
    aTransformInfoStream.seek(0);
    aTransformInfoStream.WriteInt32(nLength);
    aTransformInfoStream.seek(nLength);

    // TransformName
    OUString sTransformInfoName("Microsoft.Metadata.DRMTransform");
    aTransformInfoStream.WriteInt32(sTransformInfoName.getLength() * 2);
    aTransformInfoStream.writeUnicodeArray(sTransformInfoName);
    while (aTransformInfoStream.tell() % 4) // Padding
    {
        aTransformInfoStream.writeValue<sal_Char>(0);
    }

    aTransformInfoStream.WriteInt32(1); // ReaderVersion
    aTransformInfoStream.WriteInt32(1); // UpdateVersion
    aTransformInfoStream.WriteInt32(1); // WriterVersion

    aTransformInfoStream.WriteInt32(4); // Extensibility Header

    aTransformInfoStream.WriteInt32(mInfo.license.getLength() - 3); // LicenseLength
    aTransformInfoStream.writeArray<sal_Char>(mInfo.license.getStr() + 3,
                                              mInfo.license.getLength() - 3);
    aTransformInfoStream.writeValue<sal_Char>(0);

    aTransformInfoStream.close();
    xTransformInfo->flush();
    xTransformInfo->closeOutput();
}

void IRMEngine::encrypt(css::uno::Reference<css::io::XInputStream>& rxInputStream,
                        css::uno::Reference<css::io::XOutputStream>& rxOutputStream,
                        sal_uInt32 /*nSize*/)
{
    HRESULT hr = IpcInitialize();

    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED))
    {
        // ERROR_ALREADY_INITIALIZED not an error
        // TODO: some reaction?
    }

    BinaryXInputStream aInputStream(rxInputStream, false);
    BinaryXOutputStream aOutputStream(rxOutputStream, false);
    aOutputStream.WriteInt64(aInputStream.size()); // Stream size

    // Get decryption key
    IPC_BUFFER licenseBuffer;
    IPC_KEY_HANDLE key;
    licenseBuffer.pvBuffer = (void*)mInfo.license.getStr();
    licenseBuffer.cbBuffer = mInfo.license.getLength();
    hr = IpcGetKey(&licenseBuffer, 0, NULL, NULL, &key);
    if (FAILED(hr))
    {
        // TODO: some reaction?
    }

    // Get size of encrypt block
    DWORD* blockSize;
    hr = IpcGetKeyProperty(key, IPC_KI_BLOCK_SIZE, nullptr, (LPVOID*)&blockSize);
    if (FAILED(hr))
    {
        // TODO: some reaction?
    }

    char* pEncryptedBuffer = new char[*blockSize];
    char* pDecryptedBuffer = new char[*blockSize];
    int blockNo = 0;
    bool lastBlock = false;

    do
    {
        sal_uInt32 readBytes = aInputStream.readArray(pDecryptedBuffer, *blockSize);
        lastBlock = readBytes != *blockSize;
        DWORD bytes = 0;
        hr = IpcEncrypt(key, blockNo, lastBlock, (PBYTE)pDecryptedBuffer, *blockSize,
                        (PBYTE)pEncryptedBuffer, *blockSize, &bytes);

        if (FAILED(hr))
        {
            // TODO: some reaction?
        }

        aOutputStream.writeArray(pEncryptedBuffer, bytes);

        blockNo++;
    } while (!lastBlock);

    delete[] pEncryptedBuffer;
    delete[] pDecryptedBuffer;
}

bool IRMEngine::generateEncryptionKey(const OUString& /*password*/) { return true; }

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
