/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_gpgme.h>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XEncryptionProtectedStorage.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/embed/FileSystemStorageFactory.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/xml/crypto/NSSInitializer.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#include <vector>

#include <rtl/digest.h>
#include <rtl/random.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <ucbhelper/content.hxx>

#include <comphelper/bytereader.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/hash.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <o3tl/string_view.hxx>

#if HAVE_FEATURE_GPGME
# include <context.h>
# include <encryptionresult.h>
# include <key.h>
# include <data.h>
#endif

using namespace ::com::sun::star;

namespace comphelper {


uno::Reference< lang::XSingleServiceFactory > OStorageHelper::GetStorageFactory(
                            const uno::Reference< uno::XComponentContext >& rxContext )
{
    uno::Reference< uno::XComponentContext> xContext = rxContext.is() ? rxContext : ::comphelper::getProcessComponentContext();

    return embed::StorageFactory::create( xContext );
}


uno::Reference< lang::XSingleServiceFactory > OStorageHelper::GetFileSystemStorageFactory(
                            const uno::Reference< uno::XComponentContext >& rxContext )
{
    return embed::FileSystemStorageFactory::create(rxContext);
}


uno::Reference< embed::XStorage > OStorageHelper::GetTemporaryStorage(
            const uno::Reference< uno::XComponentContext >& rxContext )
{
    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstance(),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Reference< embed::XStorage > OStorageHelper::GetStorageFromURL(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const uno::Reference< uno::XComponentContext >& rxContext )
{
    uno::Sequence< uno::Any > aArgs{ uno::Any(aURL), uno::Any(nStorageMode) };
    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Reference< embed::XStorage > OStorageHelper::GetStorageFromURL2(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const uno::Reference< uno::XComponentContext >& rxContext )
{
    uno::Sequence< uno::Any > aArgs{ uno::Any(aURL), uno::Any(nStorageMode) };

    uno::Reference< lang::XSingleServiceFactory > xFact;
    css::uno::Any anyEx;
    try {
        ::ucbhelper::Content aCntnt( aURL,
            uno::Reference< css::ucb::XCommandEnvironment > (),
            getProcessComponentContext() );
        if (aCntnt.isDocument()) {
            xFact = GetStorageFactory( rxContext );
        } else {
            xFact = GetFileSystemStorageFactory( rxContext );
        }
    } catch (uno::Exception &)
    {
        anyEx = cppu::getCaughtException();
    }

    if (!xFact.is())
    {
        if (anyEx.hasValue())
            throw css::lang::WrappedTargetRuntimeException( "", nullptr, anyEx );
        else
            throw uno::RuntimeException();
    }

    uno::Reference< embed::XStorage > xTempStorage(
        xFact->createInstanceWithArguments( aArgs ), uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Reference< embed::XStorage > OStorageHelper::GetStorageFromInputStream(
            const uno::Reference < io::XInputStream >& xStream,
            const uno::Reference< uno::XComponentContext >& rxContext )
{
    uno::Sequence< uno::Any > aArgs{ uno::Any(xStream), uno::Any(embed::ElementModes::READ) };
    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Reference< embed::XStorage > OStorageHelper::GetStorageFromStream(
            const uno::Reference < io::XStream >& xStream,
            sal_Int32 nStorageMode,
            const uno::Reference< uno::XComponentContext >& rxContext )
{
    uno::Sequence< uno::Any > aArgs{ uno::Any(xStream), uno::Any(nStorageMode) };
    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


void OStorageHelper::CopyInputToOutput(
            const uno::Reference< io::XInputStream >& xInput,
            const uno::Reference< io::XOutputStream >& xOutput )
{
    static const sal_Int32 nConstBufferSize = 32000;

    uno::Reference< css::lang::XUnoTunnel > xInputTunnel( xInput, uno::UNO_QUERY );
    comphelper::ByteReader* pByteReader = nullptr;
    comphelper::ByteWriter* pByteWriter = nullptr;
    if (xInputTunnel)
        pByteReader = reinterpret_cast< comphelper::ByteReader* >( xInputTunnel->getSomething( comphelper::ByteReader::getUnoTunnelId() ) );
    if (pByteReader)
    {
        uno::Reference< css::lang::XUnoTunnel > xOutputTunnel( xOutput, uno::UNO_QUERY );
        if (xOutputTunnel)
            pByteWriter = reinterpret_cast< comphelper::ByteWriter* >( xOutputTunnel->getSomething( comphelper::ByteWriter::getUnoTunnelId() ) );
    }

    if (pByteWriter)
    {
        sal_Int32 nRead;
        sal_Int8 aTempBuf[ nConstBufferSize ];
        do
        {
            nRead = pByteReader->readSomeBytes ( aTempBuf, nConstBufferSize );
            pByteWriter->writeBytes ( aTempBuf, nRead );
        }
        while ( nRead == nConstBufferSize );
    }
    else
    {
        sal_Int32 nRead;
        uno::Sequence < sal_Int8 > aSequence ( nConstBufferSize );

        do
        {
            nRead = xInput->readBytes ( aSequence, nConstBufferSize );
            if ( nRead < nConstBufferSize )
            {
                uno::Sequence < sal_Int8 > aTempBuf ( aSequence.getConstArray(), nRead );
                xOutput->writeBytes ( aTempBuf );
            }
            else
                xOutput->writeBytes ( aSequence );
        }
        while ( nRead == nConstBufferSize );
    }
}


uno::Reference< io::XInputStream > OStorageHelper::GetInputStreamFromURL(
            const OUString& aURL,
            const uno::Reference< uno::XComponentContext >& context )
{
    uno::Reference< io::XInputStream > xInputStream = ucb::SimpleFileAccess::create(context)->openFileRead( aURL );
    if ( !xInputStream.is() )
        throw uno::RuntimeException();

    return xInputStream;
}


void OStorageHelper::SetCommonStorageEncryptionData(
            const uno::Reference< embed::XStorage >& xStorage,
            const uno::Sequence< beans::NamedValue >& aEncryptionData )
{
    uno::Reference< embed::XEncryptionProtectedStorage > xEncrSet( xStorage, uno::UNO_QUERY );
    if ( !xEncrSet.is() )
        throw io::IOException("no XEncryptionProtectedStorage"); // TODO

    if ( aEncryptionData.getLength() == 2 &&
         aEncryptionData[0].Name == "GpgInfos" &&
         aEncryptionData[1].Name == "EncryptionKey" )
    {
        xEncrSet->setGpgProperties(
            aEncryptionData[0].Value.get< uno::Sequence< uno::Sequence< beans::NamedValue > > >() );
        xEncrSet->setEncryptionData(
            aEncryptionData[1].Value.get< uno::Sequence< beans::NamedValue > >() );
    }
    else
        xEncrSet->setEncryptionData( aEncryptionData );
}


sal_Int32 OStorageHelper::GetXStorageFormat(
            const uno::Reference< embed::XStorage >& xStorage )
{
    uno::Reference< beans::XPropertySet > xStorProps( xStorage, uno::UNO_QUERY_THROW );

    OUString aMediaType;
    xStorProps->getPropertyValue("MediaType") >>= aMediaType;

    sal_Int32 nResult = 0;

    // TODO/LATER: the filter configuration could be used to detect it later, or better a special service
    if (
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_VND_SUN_XML_WRITER_ASCII       ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII   ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_VND_SUN_XML_DRAW_ASCII         ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_VND_SUN_XML_IMPRESS_ASCII      ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_VND_SUN_XML_CALC_ASCII         ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_VND_SUN_XML_CHART_ASCII        ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_VND_SUN_XML_MATH_ASCII         )
       )
    {
        nResult = SOFFICE_FILEFORMAT_60;
    }
    else if (
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII        ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII    ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII     ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII       ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII     ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII    ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII    ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_REPORT_CHART_ASCII    ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII        ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_TEMPLATE_ASCII ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII     ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII       ) ||
        aMediaType.equalsIgnoreAsciiCase(MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII     )
       )
    {
        nResult = SOFFICE_FILEFORMAT_8;
    }
    else
    {
        // the mediatype is not known
        OUString aMsg = __func__
                      + OUString::Concat(u":")
                      + OUString::number(__LINE__)
                      + ": unknown media type '"
                      + aMediaType
                      + "'";
        throw beans::IllegalTypeException(aMsg);
    }

    return nResult;
}


uno::Reference< embed::XStorage > OStorageHelper::GetStorageOfFormatFromURL(
            const OUString& aFormat,
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const uno::Reference< uno::XComponentContext >& rxContext )
{
    uno::Sequence< beans::PropertyValue > aProps{ comphelper::makePropertyValue("StorageFormat",
                                                                                aFormat) };

    uno::Sequence< uno::Any > aArgs{ uno::Any(aURL), uno::Any(nStorageMode), uno::Any(aProps) };
    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Reference< embed::XStorage > OStorageHelper::GetStorageOfFormatFromInputStream(
            const OUString& aFormat,
            const uno::Reference < io::XInputStream >& xStream,
            const uno::Reference< uno::XComponentContext >& rxContext,
            bool bRepairStorage )
{
    uno::Sequence< beans::PropertyValue > aProps( bRepairStorage ? 2 : 1 );
    auto pProps = aProps.getArray();
    pProps[0].Name = "StorageFormat";
    pProps[0].Value <<= aFormat;
    if ( bRepairStorage )
    {
        pProps[1].Name = "RepairPackage";
        pProps[1].Value <<= bRepairStorage;
    }

    uno::Sequence< uno::Any > aArgs{ uno::Any(xStream), uno::Any(embed::ElementModes::READ), uno::Any(aProps) };
    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Reference< embed::XStorage > OStorageHelper::GetStorageOfFormatFromStream(
            const OUString& aFormat,
            const uno::Reference < io::XStream >& xStream,
            sal_Int32 nStorageMode,
            const uno::Reference< uno::XComponentContext >& rxContext,
            bool bRepairStorage )
{
    uno::Sequence< beans::PropertyValue > aProps( bRepairStorage ? 2 : 1 );
    auto pProps = aProps.getArray();
    pProps[0].Name = "StorageFormat";
    pProps[0].Value <<= aFormat;
    if ( bRepairStorage )
    {
        pProps[1].Name = "RepairPackage";
        pProps[1].Value <<= bRepairStorage;
    }

    uno::Sequence< uno::Any > aArgs{ uno::Any(xStream), uno::Any(nStorageMode), uno::Any(aProps) };
    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Sequence< beans::NamedValue > OStorageHelper::CreatePackageEncryptionData( std::u16string_view aPassword )
{
    // TODO/LATER: Should not the method be part of DocPasswordHelper?
    uno::Sequence< beans::NamedValue > aEncryptionData;
    if ( !aPassword.empty() )
    {
        sal_Int32 nSha1Ind = 0;
        // generate SHA256 start key
        try
        {
            uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();

            uno::Reference< css::xml::crypto::XNSSInitializer > xDigestContextSupplier = css::xml::crypto::NSSInitializer::create(xContext);
            uno::Reference< css::xml::crypto::XDigestContext > xDigestContext( xDigestContextSupplier->getDigestContext( css::xml::crypto::DigestID::SHA256, uno::Sequence< beans::NamedValue >() ), uno::UNO_SET_THROW );

            OString aUTF8Password( OUStringToOString( aPassword, RTL_TEXTENCODING_UTF8 ) );
            xDigestContext->updateDigest( uno::Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aUTF8Password.getStr() ), aUTF8Password.getLength() ) );
            uno::Sequence< sal_Int8 > aDigest = xDigestContext->finalizeDigestAndDispose();

            ++nSha1Ind;
            aEncryptionData = { { PACKAGE_ENCRYPTIONDATA_SHA256UTF8, uno::Any(aDigest) } };
        }
        catch ( uno::Exception& )
        {
            OSL_ENSURE( false, "Can not create SHA256 digest!" );
        }

        // MS_1252 encoding was used for SO60 document format password encoding,
        // this encoding supports only a minor subset of nonascii characters,
        // but for compatibility reasons it has to be used for old document formats
        aEncryptionData.realloc( nSha1Ind + 3 );
        auto pEncryptionData = aEncryptionData.getArray();
        // these are StarOffice not-quite-SHA1
        pEncryptionData[nSha1Ind].Name = PACKAGE_ENCRYPTIONDATA_SHA1UTF8;
        pEncryptionData[nSha1Ind + 1].Name = PACKAGE_ENCRYPTIONDATA_SHA1MS1252;

        rtl_TextEncoding const pEncoding[2] = { RTL_TEXTENCODING_UTF8, RTL_TEXTENCODING_MS_1252 };

        for ( sal_Int32 nInd = 0; nInd < 2; nInd++ )
        {
            OString aByteStrPass = OUStringToOString( aPassword, pEncoding[nInd] );

            sal_uInt8 pBuffer[RTL_DIGEST_LENGTH_SHA1];
            rtlDigestError nError = rtl_digest_SHA1( aByteStrPass.getStr(),
                                                    aByteStrPass.getLength(),
                                                    pBuffer,
                                                    RTL_DIGEST_LENGTH_SHA1 );

            if ( nError != rtl_Digest_E_None )
            {
                aEncryptionData.realloc( nSha1Ind );
                return aEncryptionData;
            }

            // coverity[overrun-buffer-arg : FALSE] - coverity has difficulty with css::uno::Sequence
            pEncryptionData[nSha1Ind+nInd].Value <<= uno::Sequence< sal_Int8 >( reinterpret_cast<sal_Int8*>(pBuffer), RTL_DIGEST_LENGTH_SHA1 );
        }

        // actual SHA1
        pEncryptionData[nSha1Ind + 2].Name = PACKAGE_ENCRYPTIONDATA_SHA1CORRECT;
        OString aByteStrPass = OUStringToOString(aPassword, RTL_TEXTENCODING_UTF8);
        std::vector<unsigned char> const sha1(::comphelper::Hash::calculateHash(
                reinterpret_cast<unsigned char const*>(aByteStrPass.getStr()), aByteStrPass.getLength(),
                ::comphelper::HashType::SHA1));
        pEncryptionData[nSha1Ind + 2].Value <<= uno::Sequence<sal_Int8>(
                reinterpret_cast<sal_Int8 const*>(sha1.data()), sha1.size());
    }

    return aEncryptionData;
}

uno::Sequence< beans::NamedValue > OStorageHelper::CreateGpgPackageEncryptionData()
{
#if HAVE_FEATURE_GPGME
    // generate session key
    // --------------------

    rtlRandomPool aRandomPool = rtl_random_createPool();

    // get 32 random chars out of it
    uno::Sequence < sal_Int8 > aVector(32);
    rtl_random_getBytes( aRandomPool, aVector.getArray(), aVector.getLength() );

    rtl_random_destroyPool(aRandomPool);

    std::vector< uno::Sequence< beans::NamedValue > > aGpgEncryptions;

    uno::Reference< security::XDocumentDigitalSignatures > xSigner(
        // here none of the version-dependent methods are called
        security::DocumentDigitalSignatures::createDefault(
            comphelper::getProcessComponentContext()));

    // fire up certificate chooser dialog - user can multi-select!
    const uno::Sequence< uno::Reference< security::XCertificate > > xSignCertificates=
        xSigner->chooseEncryptionCertificate();

    if (!xSignCertificates.hasElements())
        return uno::Sequence< beans::NamedValue >(); // user cancelled

    // generate one encrypted key entry for each recipient
    // ---------------------------------------------------

    std::unique_ptr<GpgME::Context> ctx;
    GpgME::Error err = GpgME::checkEngine(GpgME::OpenPGP);
    if (err)
        throw uno::RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    ctx.reset( GpgME::Context::createForProtocol(GpgME::OpenPGP) );
    if (ctx == nullptr)
        throw uno::RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");
    ctx->setArmor(false);

    for (const auto & cert : xSignCertificates)
    {
        uno::Sequence < sal_Int8 > aKeyID;
        if (cert.is())
            aKeyID = cert->getSHA1Thumbprint();

        std::vector<GpgME::Key> keys
        {
            ctx->key(
                reinterpret_cast<const char*>(aKeyID.getConstArray()),
                err, false)
        };

        // ctx is setup now, let's encrypt the lot!
        GpgME::Data plain(
            reinterpret_cast<const char*>(aVector.getConstArray()),
            size_t(aVector.getLength()), false);
        GpgME::Data cipher;

        GpgME::EncryptionResult crypt_res = ctx->encrypt(
            keys, plain,
            cipher, GpgME::Context::NoCompress);

        off_t result = cipher.seek(0,SEEK_SET);
        (void) result;
        assert(result == 0);
        int len=0, curr=0; char buf;
        while( (curr=cipher.read(&buf, 1)) )
            len += curr;

        if(crypt_res.error() || !len)
            throw lang::IllegalArgumentException(
                "Not a suitable key, or failed to encrypt.",
                css::uno::Reference<css::uno::XInterface>(), -1);

        uno::Sequence < sal_Int8 > aCipherValue(len);
        result = cipher.seek(0,SEEK_SET);
        assert(result == 0);
        if( cipher.read(aCipherValue.getArray(), len) != len )
            throw uno::RuntimeException("The GpgME library failed to read the encrypted value.");

        SAL_INFO("comphelper.crypto", "Generated gpg crypto of length: " << len);

        uno::Sequence< beans::NamedValue > aGpgEncryptionEntry{
            { "KeyId", uno::Any(aKeyID) },
            { "KeyPacket", uno::Any(aKeyID) },
            { "CipherValue", uno::Any(aCipherValue) }
        };

        aGpgEncryptions.push_back(aGpgEncryptionEntry);
    }

    uno::Sequence<beans::NamedValue> aEncryptionData
        = { { PACKAGE_ENCRYPTIONDATA_SHA256UTF8, uno::Any(aVector) } };

    uno::Sequence<beans::NamedValue> aContainer
        = { { "GpgInfos", uno::Any(comphelper::containerToSequence(aGpgEncryptions)) },
            { "EncryptionKey", uno::Any(aEncryptionData) } };

    return aContainer;
#else
    return uno::Sequence< beans::NamedValue >();
#endif
}

bool OStorageHelper::IsValidZipEntryFileName( std::u16string_view aName, bool bSlashAllowed )
{
    return IsValidZipEntryFileName( aName.data(), aName.size(), bSlashAllowed );
}


bool OStorageHelper::IsValidZipEntryFileName(
    const sal_Unicode *pChar, sal_Int32 nLength, bool bSlashAllowed )
{
    for ( sal_Int32 i = 0; i < nLength; i++ )
    {
        switch ( pChar[i] )
        {
            case '\\':
            case '?':
            case '<':
            case '>':
            case '\"':
            case '|':
            case ':':
                return false;
            case '/':
                if ( !bSlashAllowed )
                    return false;
                break;
            default:
                if ( pChar[i] < 32  || (pChar[i] >= 0xD800 && pChar[i] <= 0xDFFF) )
                    return false;
        }
    }
    return true;
}


bool OStorageHelper::PathHasSegment( std::u16string_view aPath, std::u16string_view aSegment )
{
    bool bResult = false;
    const size_t nPathLen = aPath.size();
    const size_t nSegLen = aSegment.size();

    if ( !aSegment.empty() && nPathLen >= nSegLen )
    {
        OUString aEndSegment = OUString::Concat("/") + aSegment;
        OUString aInternalSegment = aEndSegment + "/";

        if ( aPath.find( aInternalSegment ) != std::u16string_view::npos )
            bResult = true;

        if ( !bResult && o3tl::starts_with(aPath, aSegment ) )
        {
            if ( nPathLen == nSegLen || aPath[nSegLen] == '/' )
                bResult = true;
        }

        if ( !bResult && nPathLen > nSegLen && aPath.substr( nPathLen - nSegLen - 1, nSegLen + 1 ) == aEndSegment )
            bResult = true;
    }

    return bResult;
}

class LifecycleProxy::Impl
    : public std::vector< uno::Reference< embed::XStorage > > {};
LifecycleProxy::LifecycleProxy()
    : m_xBadness( new Impl ) { }
LifecycleProxy::~LifecycleProxy() { }

void LifecycleProxy::commitStorages()
{
    std::for_each(m_xBadness->rbegin(), m_xBadness->rend(), // reverse order (outwards)
        [](Impl::reference rxItem) {
            uno::Reference<embed::XTransactedObject> const xTransaction(rxItem, uno::UNO_QUERY);
            if (xTransaction.is())
            {
                xTransaction->commit();
            }
        });
}

static void splitPath( std::vector<OUString> &rElems, std::u16string_view rPath )
{
    for (sal_Int32 i = 0; i >= 0;)
        rElems.push_back( OUString(o3tl::getToken(rPath, 0, '/', i )) );
}

static uno::Reference< embed::XStorage > LookupStorageAtPath(
        const uno::Reference< embed::XStorage > &xParentStorage,
        std::vector<OUString> &rElems, sal_uInt32 nOpenMode,
        LifecycleProxy const &rNastiness )
{
    uno::Reference< embed::XStorage > xStorage( xParentStorage );
    rNastiness.m_xBadness->push_back( xStorage );
    for( size_t i = 0; i < rElems.size() && xStorage.is(); i++ )
    {
        xStorage = xStorage->openStorageElement( rElems[i], nOpenMode );
        rNastiness.m_xBadness->push_back( xStorage );
    }
    return xStorage;
}

uno::Reference< embed::XStorage > OStorageHelper::GetStorageAtPath(
        const uno::Reference< embed::XStorage > &xStorage,
        std::u16string_view rPath, sal_uInt32 nOpenMode,
        LifecycleProxy const &rNastiness )
{
    std::vector<OUString> aElems;
    splitPath( aElems, rPath );
    return LookupStorageAtPath( xStorage, aElems, nOpenMode, rNastiness );
}

uno::Reference< io::XStream > OStorageHelper::GetStreamAtPath(
        const uno::Reference< embed::XStorage > &xParentStorage,
        std::u16string_view rPath, sal_uInt32 nOpenMode,
        LifecycleProxy const &rNastiness )
{
    std::vector<OUString> aElems;
    splitPath( aElems, rPath );
    OUString aName( aElems.back() );
    aElems.pop_back();
    sal_uInt32 nStorageMode = nOpenMode & ~embed::ElementModes::TRUNCATE;
    uno::Reference< embed::XStorage > xStorage(
        LookupStorageAtPath( xParentStorage, aElems, nStorageMode, rNastiness ),
        uno::UNO_SET_THROW );
    return xStorage->openStreamElement( aName, nOpenMode );
}

uno::Reference< io::XStream > OStorageHelper::GetStreamAtPackageURL(
        uno::Reference< embed::XStorage > const& xParentStorage,
        const OUString& rURL, sal_uInt32 const nOpenMode,
        LifecycleProxy const & rNastiness)
{
    OUString path;
    if (rURL.startsWithIgnoreAsciiCase("vnd.sun.star.Package:", &path))
    {
        return GetStreamAtPath(xParentStorage, path, nOpenMode, rNastiness);
    }
    return nullptr;
}

OUString OStorageHelper::GetODFVersionFromStorage(const uno::Reference<embed::XStorage>& xStorage)
{
    OUString aODFVersion;
    try
    {
        uno::Reference<beans::XPropertySet> xPropSet(xStorage, uno::UNO_QUERY_THROW);
        xPropSet->getPropertyValue("Version") >>= aODFVersion;
    }
    catch (uno::Exception&)
    {
    }
    return aODFVersion;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
