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
#include <com/sun/star/io/SequenceInputStream.hpp>
#include <com/sun/star/io/XSequenceOutputStream.hpp>
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
extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_oox_crypto_DRMEncryptedDataSpace_get_implementation(
    XComponentContext* pCtx, Sequence<Any> const& /*arguments*/)
{
    return cppu::acquire(new IRMEngine(pCtx /*, arguments*/));
}

IRMEngine::IRMEngine(const Reference<XComponentContext>& rxContext)
    : mxContext(rxContext)
{
}

sal_Bool IRMEngine::checkDataIntegrity() { return true; }

sal_Bool IRMEngine::decrypt(const Reference<XInputStream>& rxInputStream,
                            Reference<XOutputStream>& rxOutputStream)
{
    BinaryXInputStream aInputStream(rxInputStream, true);
    BinaryXOutputStream aOutputStream(rxOutputStream, true);

    aInputStream.readInt64(); // Skip stream size

    HRESULT hr = IpcInitialize();
    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED))
    {
        // ERROR_ALREADY_INITIALIZED not an error
        // TODO: some reaction?
        return false;
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
        return false;
    }

    // Read rights
    BOOL value;
    hr = IpcAccessCheck(key, IPC_GENERIC_READ, &value);
    if (FAILED(hr))
    {
        // TODO: some reaction?
        return false;
    }
    mInfo.bCanRead = value;

    // Get size of decrypt block
    DWORD* blockSize;
    hr = IpcGetKeyProperty(key, IPC_KI_BLOCK_SIZE, nullptr, (LPVOID*)&blockSize);
    if (FAILED(hr))
    {
        // TODO: some reaction?
        return false;
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
            return false;
        }

        aOutputStream.writeArray(pDecryptedBuffer, bytes);

        blockNo++;
    } while (!lastBlock);

    delete[] pEncryptedBuffer;
    delete[] pDecryptedBuffer;

    rxOutputStream->flush();

    return true;
}

uno::Sequence<beans::NamedValue> IRMEngine::createEncryptionData(const OUString& /*rPassword*/)
{
    css::uno::Sequence<sal_uInt8> seq;
    seq.realloc(mInfo.license.getLength());
    memcpy(seq.getArray(), mInfo.license.getStr(), mInfo.license.getLength());

    comphelper::SequenceAsHashMap aEncryptionData;
    aEncryptionData["LicenseKey"] <<= seq;
    aEncryptionData["CryptoType"] <<= OUString("DRMEncryptedDataSpace");
    aEncryptionData["OOXPassword"] <<= OUString("1");

    return aEncryptionData.getAsConstNamedValueList();
}

uno::Reference<io::XInputStream> IRMEngine::getStream(const Sequence<NamedValue>& rStreams,
                                                      const OUString sStreamName)
{
    for (const auto& aStream : rStreams)
    {
        if (aStream.Name == sStreamName)
        {
            css::uno::Sequence<sal_Int8> aSeq;
            aStream.Value >>= aSeq;
            Reference<XInputStream> aStream(
                io::SequenceInputStream::createStreamFromSequence(mxContext, aSeq),
                UNO_QUERY_THROW);
            return aStream;
        }
    }
    return nullptr;
}

sal_Bool IRMEngine::readEncryptionInfo(const uno::Sequence<beans::NamedValue>& aStreams)
{
    // Read TransformInfo storage for IRM ECMA documents (MS-OFFCRYPTO 2.2.4)
    uno::Reference<io::XInputStream> xTransformInfoStream
        = getStream(aStreams, "\006DataSpaces/TransformInfo/DRMEncryptedTransform/\006Primary");
    SAL_WARN_IF(!xTransformInfoStream.is(), "oox", "TransormInfo stream is missing!");
    BinaryXInputStream aBinaryStream(xTransformInfoStream, true);

    // MS-OFFCRYPTO 2.1.8: TransformInfoHeader
    aBinaryStream.readuInt32(); // TransformLength
    aBinaryStream.readuInt32(); // TransformType
    // TransformId
    sal_uInt32 aStringLength = aBinaryStream.readuInt32();
    OUString sTransformId = aBinaryStream.readUnicodeArray(aStringLength / 2);
    aBinaryStream.skip((4 - (aStringLength & 3)) & 3); // Skip padding

    // TransformName
    aStringLength = aBinaryStream.readuInt32();
    OUString sTransformName = aBinaryStream.readUnicodeArray(aStringLength / 2);
    aBinaryStream.skip((4 - (aStringLength & 3)) & 3); // Skip padding

    aBinaryStream.readuInt32(); // ReaderVersion
    aBinaryStream.readuInt32(); // UpdaterVersion
    aBinaryStream.readuInt32(); // WriterVersion

    // MS-OFFCRYPTO 2.2.5: ExtensibilityHeader
    aBinaryStream.readuInt32(); // ExtensibilityHeader

    // MS-OFFCRYPTO 2.2.6: XrMLLicense
    aStringLength = aBinaryStream.readuInt32();
    mInfo.license = aBinaryStream.readCharArray(aStringLength);

    if (mInfo.license.getLength()
        && static_cast<sal_uChar>(mInfo.license[0]) != 0x0ef) // BOM is missing?
    {
        mInfo.license = "\x0ef\x0bb\x0bf" + mInfo.license;
    }

    // TODO: CHECK info data

    return true;
}

sal_Bool IRMEngine::setupEncryption(const Sequence<NamedValue>& rMediaEncData)
{
    for (int i = 0; i < rMediaEncData.getLength(); i++)
    {
        if (rMediaEncData[i].Name == "LicenseKey")
        {
            css::uno::Sequence<sal_uInt8> seq;
            rMediaEncData[i].Value >>= seq;
            mInfo.license = OString(reinterpret_cast<sal_Char*>(seq.getArray()), seq.getLength());
        }
    }

    return true;
}

Sequence<NamedValue> IRMEngine::writeEncryptionInfo()
{
    // Write 0x6DataSpaces/DataSpaceMap
    Reference<XOutputStream> xDataSpaceMap(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.io.SequenceOutputStream", mxContext),
        UNO_QUERY);
    BinaryXOutputStream aDataSpaceMapStream(xDataSpaceMap, false);

    aDataSpaceMapStream.WriteInt32(8); // Header length
    aDataSpaceMapStream.WriteInt32(1); // Entries count

    // DataSpaceMapEntry (MS-OFFCRYPTO 2.1.6.1)
    OUString sDataSpaceName("DRMEncryptedDataSpace");
    OUString sReferenceComponent("EncryptedPackage");

    aDataSpaceMapStream.WriteInt32(0x60); // Length
    aDataSpaceMapStream.WriteInt32(1); // References count
    aDataSpaceMapStream.WriteInt32(0); // References component type

    aDataSpaceMapStream.WriteInt32(sReferenceComponent.getLength() * 2);
    aDataSpaceMapStream.writeUnicodeArray(sReferenceComponent);
    for (int i = 0; i < sReferenceComponent.getLength() * 2 % 4; i++) // Padding
    {
        aDataSpaceMapStream.writeValue<sal_Char>(0);
    }

    aDataSpaceMapStream.WriteInt32(sDataSpaceName.getLength() * 2);
    aDataSpaceMapStream.writeUnicodeArray(sDataSpaceName);
    for (int i = 0; i < sDataSpaceName.getLength() * 2 % 4; i++) // Padding
    {
        aDataSpaceMapStream.writeValue<sal_Char>(0);
    }

    aDataSpaceMapStream.close();
    xDataSpaceMap->flush();

    // Write 0x6DataSpaces/Version
    Reference<XOutputStream> xVersion(mxContext->getServiceManager()->createInstanceWithContext(
                                          "com.sun.star.io.SequenceOutputStream", mxContext),
                                      UNO_QUERY);
    BinaryXOutputStream aVersionStream(xVersion, false);

    OUString sFeatureIdentifier("Microsoft.Container.DataSpaces");
    aVersionStream.WriteInt32(sFeatureIdentifier.getLength() * 2);
    aVersionStream.writeUnicodeArray(sFeatureIdentifier);
    for (int i = 0; i < sFeatureIdentifier.getLength() * 2 % 4; i++) // Padding
    {
        aVersionStream.writeValue<sal_Char>(0);
    }

    aVersionStream.WriteInt32(1); // Reader version
    aVersionStream.WriteInt32(1); // Updater version
    aVersionStream.WriteInt32(1); // Writer version

    aVersionStream.close();
    xVersion->flush();

    // Write 0x6DataSpaces/DataSpaceInfo/[dataspacename]
    Reference<XOutputStream> xDataSpaceInfo(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.io.SequenceOutputStream", mxContext),
        UNO_QUERY);
    BinaryXOutputStream aDataSpaceInfoStream(xDataSpaceInfo, false);

    aDataSpaceInfoStream.WriteInt32(0x08); // Header length
    aDataSpaceInfoStream.WriteInt32(1); // Entries count

    OUString sTransformName("DRMEncryptedTransform");
    aDataSpaceInfoStream.WriteInt32(sTransformName.getLength() * 2);
    aDataSpaceInfoStream.writeUnicodeArray(sTransformName);
    for (int i = 0; i < sTransformName.getLength() * 2 % 4; i++) // Padding
    {
        aDataSpaceInfoStream.writeValue<sal_Char>(0);
    }

    aDataSpaceInfoStream.close();
    xDataSpaceInfo->flush();

    // Write 0x6DataSpaces/TransformInfo/[transformname]
    Reference<XOutputStream> xTransformInfo(
        mxContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.io.SequenceOutputStream", mxContext),
        UNO_QUERY);
    BinaryXOutputStream aTransformInfoStream(xTransformInfo, false);
    OUString sTransformId("{C73DFACD-061F-43B0-8B64-0C620D2A8B50}");

    // MS-OFFCRYPTO 2.1.8: TransformInfoHeader
    sal_uInt32 nLength
        = sTransformId.getLength() * 2 + ((4 - (sTransformId.getLength() & 3)) & 3) + 10;
    aTransformInfoStream.WriteInt32(nLength); // TransformLength, will be written later
    aTransformInfoStream.WriteInt32(1); // TransformType

    // TransformId
    aTransformInfoStream.WriteInt32(sTransformId.getLength() * 2);
    aTransformInfoStream.writeUnicodeArray(sTransformId);
    for (int i = 0; i < sTransformId.getLength() * 2 % 4; i++) // Padding
    {
        aTransformInfoStream.writeValue<sal_Char>(0);
    }

    // TransformName
    OUString sTransformInfoName("Microsoft.Metadata.DRMTransform");
    aTransformInfoStream.WriteInt32(sTransformInfoName.getLength() * 2);
    aTransformInfoStream.writeUnicodeArray(sTransformInfoName);
    for (int i = 0; i < sTransformInfoName.getLength() * 2 % 4; i++) // Padding
    {
        aTransformInfoStream.writeValue<sal_Char>(0);
    }

    aTransformInfoStream.WriteInt32(1); // ReaderVersion
    aTransformInfoStream.WriteInt32(1); // UpdateVersion
    aTransformInfoStream.WriteInt32(1); // WriterVersion

    aTransformInfoStream.WriteInt32(4); // Extensibility Header

    aTransformInfoStream.WriteInt32(mInfo.license.getLength() - 3); // LicenseLength - BOM
    aTransformInfoStream.writeArray<sal_Char>(mInfo.license.getStr() + 3,
                                              mInfo.license.getLength() - 3);
    aTransformInfoStream.writeValue<sal_Char>(0);

    aTransformInfoStream.close();
    xTransformInfo->flush();

    // Store all streams into sequence and return back
    comphelper::SequenceAsHashMap aStreams;

    Reference<XSequenceOutputStream> xDataSpaceMapSequence(xDataSpaceMap, UNO_QUERY);
    aStreams["\006DataSpaces/DataSpaceMap"] <<= xDataSpaceMapSequence->getWrittenBytes();

    Reference<XSequenceOutputStream> xVersionSequence(xVersion, UNO_QUERY);
    aStreams["\006DataSpaces/Version"] <<= xVersionSequence->getWrittenBytes();

    OUString sStreamName = "\006DataSpaces/DataSpaceInfo/" + sDataSpaceName;
    Reference<XSequenceOutputStream> xDataSpaceInfoSequence(xDataSpaceInfo, UNO_QUERY);
    aStreams[sStreamName] <<= xDataSpaceInfoSequence->getWrittenBytes();

    sStreamName = "\006DataSpaces/TransformInfo/" + sTransformName + "/\006Primary";
    Reference<XSequenceOutputStream> xTransformInfoSequence(xTransformInfo, UNO_QUERY);
    aStreams[sStreamName] <<= xTransformInfoSequence->getWrittenBytes();

    return aStreams.getAsConstNamedValueList();
}

void IRMEngine::encrypt(const Reference<XInputStream>& rxInputStream,
                        Reference<XOutputStream>& rxOutputStream)
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

sal_Bool IRMEngine::generateEncryptionKey(const OUString& /*password*/) { return true; }

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
