/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <oox/crypto/StrongEncryptionDataSpace.hxx>
#include <oox/crypto/AgileEngine.hxx>
#include <oox/crypto/Standard2007Engine.hxx>
#include <oox/helper/binaryoutputstream.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <com/sun/star/io/SequenceInputStream.hpp>
#include <com/sun/star/io/XSequenceOutputStream.hpp>

#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace css;
using namespace css::beans;
using namespace css::io;
using namespace css::lang;
using namespace css::uno;

namespace oox::crypto
{
StrongEncryptionDataSpace::StrongEncryptionDataSpace(const Reference<XComponentContext>& rxContext)
    : mxContext(rxContext)
    , mCryptoEngine(new Standard2007Engine)
{
}

sal_Bool StrongEncryptionDataSpace::generateEncryptionKey(const OUString& rPassword)
{
    if (!mCryptoEngine)
        return false;

    return mCryptoEngine->generateEncryptionKey(rPassword);
}

sal_Bool StrongEncryptionDataSpace::checkDataIntegrity()
{
    if (!mCryptoEngine)
        return false;

    return mCryptoEngine->checkDataIntegrity();
}

sal_Bool StrongEncryptionDataSpace::decrypt(const Reference<XInputStream>& rxInputStream,
                                            Reference<XOutputStream>& rxOutputStream)
{
    if (!mCryptoEngine)
        return false;

    BinaryXInputStream aInputStream(rxInputStream, true);
    BinaryXOutputStream aOutputStream(rxOutputStream, true);

    mCryptoEngine->decrypt(aInputStream, aOutputStream);

    rxOutputStream->flush();
    return true;
}

Reference<XInputStream> StrongEncryptionDataSpace::getStream(const Sequence<NamedValue>& rStreams,
                                                             std::u16string_view sStreamName)
{
    for (const auto& aStream : rStreams)
    {
        if (aStream.Name == sStreamName)
        {
            Sequence<sal_Int8> aSeq;
            aStream.Value >>= aSeq;
            Reference<XInputStream> aStream2(
                io::SequenceInputStream::createStreamFromSequence(mxContext, aSeq),
                UNO_QUERY_THROW);
            return aStream2;
        }
    }
    return nullptr;
}

sal_Bool StrongEncryptionDataSpace::readEncryptionInfo(const Sequence<NamedValue>& aStreams)
{
    Reference<XInputStream> xEncryptionInfo = getStream(aStreams, u"EncryptionInfo");
    if (!xEncryptionInfo.is())
        return false;

    BinaryXInputStream aBinaryInputStream(xEncryptionInfo, true);
    sal_uInt32 aVersion = aBinaryInputStream.readuInt32();

    switch (aVersion)
    {
        case msfilter::VERSION_INFO_2007_FORMAT:
        case msfilter::VERSION_INFO_2007_FORMAT_SP2:
            mCryptoEngine.reset(new Standard2007Engine);
            break;
        case msfilter::VERSION_INFO_AGILE:
            mCryptoEngine.reset(new AgileEngine());
            break;
        default:
            break;
    }

    if (!mCryptoEngine)
        return false;

    return mCryptoEngine->readEncryptionInfo(xEncryptionInfo);
}

sal_Bool StrongEncryptionDataSpace::setupEncryption(const Sequence<NamedValue>& rMediaEncData)
{
    if (!mCryptoEngine)
        return false;

    OUString sPassword;
    for (const auto& aParam : rMediaEncData)
    {
        if (aParam.Name == "OOXPassword")
        {
            aParam.Value >>= sPassword;
        }
    }

    return mCryptoEngine->setupEncryption(sPassword);
}

Sequence<NamedValue> StrongEncryptionDataSpace::createEncryptionData(const OUString& rPassword)
{
    comphelper::SequenceAsHashMap aEncryptionData;
    aEncryptionData[u"OOXPassword"_ustr] <<= rPassword;
    aEncryptionData[u"CryptoType"_ustr] <<= u"StrongEncryptionDataSpace"_ustr;

    return aEncryptionData.getAsConstNamedValueList();
}

Sequence<NamedValue>
StrongEncryptionDataSpace::encrypt(const Reference<XInputStream>& rxInputStream)
{
    if (!mCryptoEngine)
        return Sequence<NamedValue>();

    Reference<XSeekable> xSeekable(rxInputStream, UNO_QUERY);
    if (!xSeekable.is())
        return Sequence<NamedValue>();

    sal_uInt32 aLength = xSeekable->getLength(); // check length of the stream

    Reference<XOutputStream> xOutputStream(
        mxContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.io.SequenceOutputStream"_ustr, mxContext),
        UNO_QUERY);

    mCryptoEngine->encrypt(rxInputStream, xOutputStream, aLength);

    comphelper::SequenceAsHashMap aStreams;

    Reference<XSequenceOutputStream> xEncodedFileSequenceStream(xOutputStream, UNO_QUERY);
    aStreams[u"EncryptedPackage"_ustr] <<= xEncodedFileSequenceStream->getWrittenBytes();

    Reference<XOutputStream> aEncryptionInfoStream(
        mxContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.io.SequenceOutputStream"_ustr, mxContext),
        UNO_QUERY);
    BinaryXOutputStream rStream(aEncryptionInfoStream, false);
    mCryptoEngine->writeEncryptionInfo(rStream);
    aEncryptionInfoStream->flush();
    Reference<XSequenceOutputStream> aEncryptionInfoSequenceStream(aEncryptionInfoStream,
                                                                   UNO_QUERY);

    aStreams[u"EncryptionInfo"_ustr] <<= aEncryptionInfoSequenceStream->getWrittenBytes();

    return aStreams.getAsConstNamedValueList();
}

OUString SAL_CALL StrongEncryptionDataSpace::getImplementationName()
{
    return u"com.sun.star.comp.oox.crypto.StrongEncryptionDataSpace"_ustr;
}

sal_Bool SAL_CALL StrongEncryptionDataSpace::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL StrongEncryptionDataSpace::getSupportedServiceNames()
{
    Sequence<OUString> aServices{ u"com.sun.star.packages.PackageEncryption"_ustr };
    return aServices;
}

} // namespace oox::crypto

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_oox_crypto_StrongEncryptionDataSpace_get_implementation(
    uno::XComponentContext* pCtx, uno::Sequence<uno::Any> const& /*rSeq*/)
{
    return cppu::acquire(new oox::crypto::StrongEncryptionDataSpace(pCtx));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
