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

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XEncryptionProtectedSource2.hpp>
#include <com/sun/star/embed/XEncryptionProtectedStorage.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/embed/FileSystemStorageFactory.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/xml/crypto/NSSInitializer.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContextSupplier.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/security/DocumentDigitalSignatures.hpp>
#include <com/sun/star/security/XCertificate.hpp>

#include <vector>

#include <rtl/digest.h>
#include <rtl/random.h>
#include <osl/time.h>
#include <osl/diagnose.h>
#include <sax/tools/converter.hxx>

#include <ucbhelper/content.hxx>

#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/storagehelper.hxx>

#include <gpgme.h>
#include <context.h>
#include <key.h>
#include <data.h>

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
    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= aURL;
    aArgs[1] <<= nStorageMode;

    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Reference< embed::XStorage > OStorageHelper::GetStorageFromURL2(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const uno::Reference< uno::XComponentContext >& rxContext )
{
    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= aURL;
    aArgs[1] <<= nStorageMode;

    uno::Reference< lang::XSingleServiceFactory > xFact;
    try {
        ::ucbhelper::Content aCntnt( aURL,
            uno::Reference< css::ucb::XCommandEnvironment > (),
            getProcessComponentContext() );
        if (aCntnt.isDocument()) {
            xFact = GetStorageFactory( rxContext );
        } else {
            xFact = GetFileSystemStorageFactory( rxContext );
        }
    } catch (uno::Exception &) { }

    if (!xFact.is()) throw uno::RuntimeException();

    uno::Reference< embed::XStorage > xTempStorage(
        xFact->createInstanceWithArguments( aArgs ), uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Reference< embed::XStorage > OStorageHelper::GetStorageFromInputStream(
            const uno::Reference < io::XInputStream >& xStream,
            const uno::Reference< uno::XComponentContext >& rxContext )
{
    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= embed::ElementModes::READ;

    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Reference< embed::XStorage > OStorageHelper::GetStorageFromStream(
            const uno::Reference < io::XStream >& xStream,
            sal_Int32 nStorageMode,
            const uno::Reference< uno::XComponentContext >& rxContext )
{
    uno::Sequence< uno::Any > aArgs( 2 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= nStorageMode;

    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


void OStorageHelper::CopyInputToOutput(
            const uno::Reference< io::XInputStream >& xInput,
            const uno::Reference< io::XOutputStream >& xOutput )
{
    static const sal_Int32 nConstBufferSize = 32000;

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
        throw io::IOException(); // TODO

    if ( aEncryptionData.getLength() == 2 &&
         aEncryptionData[0].Name == "GpgInfos" &&
         aEncryptionData[1].Name == "EncryptionKey" )
    {
        xEncrSet->setGpgProperties(
            aEncryptionData[0].Value.get< uno::Sequence< beans::NamedValue > >() );
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

    // TODO/LATER: the filter configuration could be used to detect it later, or batter a special service
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
        OUString aMsg = OUString(OSL_THIS_FUNC)
                      + ":"
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
    uno::Sequence< beans::PropertyValue > aProps( 1 );
    aProps[0].Name = "StorageFormat";
    aProps[0].Value <<= aFormat;

    uno::Sequence< uno::Any > aArgs( 3 );
    aArgs[0] <<= aURL;
    aArgs[1] <<= nStorageMode;
    aArgs[2] <<= aProps;

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
    uno::Sequence< beans::PropertyValue > aProps( 1 );
    sal_Int32 nPos = 0;
    aProps[nPos].Name = "StorageFormat";
    aProps[nPos].Value <<= aFormat;
    ++nPos;
    if ( bRepairStorage )
    {
        aProps.realloc(nPos+1);
        aProps[nPos].Name = "RepairPackage";
        aProps[nPos].Value <<= bRepairStorage;
        ++nPos;
    }

    uno::Sequence< uno::Any > aArgs( 3 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= embed::ElementModes::READ;
    aArgs[2] <<= aProps;

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
    uno::Sequence< beans::PropertyValue > aProps( 1 );
    sal_Int32 nPos = 0;
    aProps[nPos].Name = "StorageFormat";
    aProps[nPos].Value <<= aFormat;
    ++nPos;
    if ( bRepairStorage )
    {
        aProps.realloc(nPos+1);
        aProps[nPos].Name = "RepairPackage";
        aProps[nPos].Value <<= bRepairStorage;
        ++nPos;
    }

    uno::Sequence< uno::Any > aArgs( 3 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= nStorageMode;
    aArgs[2] <<= aProps;

    uno::Reference< embed::XStorage > xTempStorage( GetStorageFactory( rxContext )->createInstanceWithArguments( aArgs ),
                                                    uno::UNO_QUERY_THROW );
    return xTempStorage;
}


uno::Sequence< beans::NamedValue > OStorageHelper::CreatePackageEncryptionData( const OUString& aPassword )
{
    // TODO/LATER: Should not the method be part of DocPasswordHelper?
    uno::Sequence< beans::NamedValue > aEncryptionData;
    if ( !aPassword.isEmpty() )
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

            aEncryptionData.realloc( ++nSha1Ind );
            aEncryptionData[0].Name = PACKAGE_ENCRYPTIONDATA_SHA256UTF8;
            aEncryptionData[0].Value <<= aDigest;
        }
        catch ( uno::Exception& )
        {
            OSL_ENSURE( false, "Can not create SHA256 digest!" );
        }

        // MS_1252 encoding was used for SO60 document format password encoding,
        // this encoding supports only a minor subset of nonascii characters,
        // but for compatibility reasons it has to be used for old document formats
        aEncryptionData.realloc( nSha1Ind + 2 );
        aEncryptionData[nSha1Ind].Name = PACKAGE_ENCRYPTIONDATA_SHA1UTF8;
        aEncryptionData[nSha1Ind + 1].Name = PACKAGE_ENCRYPTIONDATA_SHA1MS1252;

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
                break;
            }

            aEncryptionData[nSha1Ind+nInd].Value <<= uno::Sequence< sal_Int8 >( reinterpret_cast<sal_Int8*>(pBuffer), RTL_DIGEST_LENGTH_SHA1 );
        }
    }

    return aEncryptionData;
}

uno::Sequence< beans::NamedValue > OStorageHelper::CreateGpgPackageEncryptionData()
{
    // generate session key
    // --------------------

    // Get a random number generator and seed it with current timestamp
    TimeValue aTime;
    osl_getSystemTime( &aTime );
    rtlRandomPool aRandomPool = rtl_random_createPool();
    rtl_random_addBytes(aRandomPool, &aTime, 8);

    // get 16 random chars out of it
    uno::Sequence < sal_Int8 > aVector(16);
    rtl_random_getBytes( aRandomPool, aVector.getArray(), aVector.getLength() );

    rtl_random_destroyPool(aRandomPool);

    uno::Sequence< beans::NamedValue > aContainer(2);
    uno::Sequence< beans::NamedValue > aGpgEncryptionData(3);
    uno::Sequence< beans::NamedValue > aEncryptionData(1);

    // TODO fire certificate chooser dialog
    uno::Reference< security::XDocumentDigitalSignatures > xSigner(
        security::DocumentDigitalSignatures::createWithVersion(
            comphelper::getProcessComponentContext(), "1.2" ) );

    // The user may provide a description while choosing a certificate.
    OUString aDescription;
    uno::Reference< security::XCertificate > xSignCertificate=
        xSigner->chooseEncryptionCertificate(aDescription);

    uno::Sequence < sal_Int8 > aKeyID;
    if (xSignCertificate.is())
    {
        aKeyID = xSignCertificate->getSHA1Thumbprint();
    }

    std::unique_ptr<GpgME::Context> ctx;
    GpgME::Error err = GpgME::checkEngine(GpgME::OpenPGP);
    if (err)
        throw uno::RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    ctx.reset( GpgME::Context::createForProtocol(GpgME::OpenPGP) );
    if (ctx == nullptr)
        throw uno::RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");
    ctx->setArmor(false);
    ctx->setKeyListMode(GPGME_KEYLIST_MODE_LOCAL);
    std::vector<GpgME::Key> keys;
    keys.push_back(
        ctx->key(
            reinterpret_cast<const char*>(aKeyID.getConstArray()),
            err, true));

#if 0
    // good, ctx is setup now, let's sign the lot
    GpgME::Data data_in(
        reinterpret_cast<char*>(xmlSecBufferGetData(pDsigCtx->transformCtx.result)),
        xmlSecBufferGetSize(pDsigCtx->transformCtx.result), false);
    GpgME::Data data_out;

    ctx->encrypt(keys, plain, cipher, GpgME::Context::NoCompress);

    SAL_INFO("xmlsecurity.xmlsec.gpg", "Generating signature for: " << xmlSecBufferGetData(pDsigCtx->transformCtx.result));
#endif

    // TODO perhaps rename that one - bit misleading name ...
    aGpgEncryptionData[0].Name = "KeyId";
    aGpgEncryptionData[0].Value <<= aKeyID;
    aGpgEncryptionData[1].Name = "KeyPacket";
    OUString packet("LS0tLS1CRUdJTiBQR1AgUFJJVkFURSBLRVkgQkxPQ0stLS0tLQpWZXJzaW9uOiBHbnVQRyB2MQoKbFFJR0JGY3J4V3NCQkFDM1VBdko4Sk9PZWZZcVVYQVNzVHkrUHBjNHp3cU9YZlRZT2VTSk45V3RZWDVBdU9RNgpjdzZUTmhhbExwT1hLNlhLcUpoMklqSVh6cE1jUzEvQzg1QlNSK0V6dm51VXlhUCtZTXI4VzkyalZwNGo2OWJFCkR1Mnd2Nm5wTnFvRDhqY3NBMHJLeUFoVEg0c3lNL2RMcm1FOThEVXJibGRscE11R0VDUFg0L2tVSHdBUkFRQUIKL2djREFqbjhxQXJpdllBVVlLVGtxd1U5dTRseUdhUEFzOFZNc0ltWTRYUksxd1hOWHNmVC9vaG44QWh4OHJWWQp3eCtQUnFVemxrS2xiNFhkcjBzL0VKSWp0bmx4c3ZPYWlsY1VFaFpEL0VkaHM5WEI2MUl0UFdSWm1OMW1lbGt0Ck1lZDBLL1hCdzVxejJUemZGcFBaaGIxdjZMY0IwRmZnSjY2K1JBWW9EZXVmRGdGNllNcmhrc3kvVnNwM1MrYTYKNE4rclBuNWlkcTVPOU5lb09MVCt5RGRBVTlFeFAxVnd1SFY5V01UT3JvcUtKZUYyT1lvSW05V2dzTGd6ZEU1OQpuSXFVNC9FcWQxYUpCbTgrRTBtbnhFd0VuS2JJNEtOa0tzdDAwRXFmTmIzVXA4RlFJV3hVRnlaclplL2JmeTNCCnVrM2IzZ0xRUlRqUHJvR0JNS3B2UWQvd2xVVTVESS92OUdHMWtpT2duV1dnZGFWWjBFazdzTWhaY1R3aWpHNFYKaUVaOEE1bVZRWXBMTlFwY29jRGdoM0lyRzBkSFVkNVorRUVaQkRSbElaRklKVnVyak9XempzM2lsd3ZhYWpmUwppSFdiZUw4Nmh2bFVWV2hCTCtqMUhsTDRZWjg2SmJNc0kzQ0ZmaEJXT1pqbEFMMTRvc1NHNUVLMEhGUmxjM1FnClZYTmxjaUE4ZEdWemRFQmxlR0Z0Y0d4bExtTnZiVDZJdmdRVEFRSUFLQVVDVnl2RmF3SWJBd1VKQUFGUmdBWUwKQ1FnSEF3SUdGUWdDQ1FvTEJCWUNBd0VDSGdFQ0Y0QUFDZ2tRanlwQVVkSDBFOWVwTGdQOUcxLytPU0pkVWVzKwp3ZDMvQmdwbUxqdWRsWEVXQVZjZnh2UlpHbmVjY2VwOXl5ay90WHRSc3lnNjMyTnV5REFlUk5EWmRDVEFHNGNUCmQxY1crWnJsQzM5T1MrOFUrQUFVUi82QlVic3JXT0RrMTVzN2VOOUs3NmE2SU9Lb2RKRHd1QkVkZDJQcUdCd3YKQ1Qwam5pT0pXUFVZdjJzOHBPMGFndzdVV0dNUzdmU2RBZ1lFVnl2RmF3RUVBTU04aVZnMERNbEpPSlQwbmhFQgp2dFR3ZUpIeFB2akpnTHNmUlFsdkNlQWs1U2taV1pNTllxRnRxbTl6NmJqMkJobnlYU0tFMENBem4xTXhsc1o2CjlJWkJPQ3ZURlRRY1pZQ3V2N1UwY3prU0xzMlBvN1VMeXJKMFNpSS9OS2NSdVhIR3ZDSzdWUHdpK2RROTNITUEKWUxOZ2FyblJ1cVUzbHd1NU1yaHROZVpOQUJFQkFBSCtCd01DT2Z5b0N1SzlnQlJnZm1XUmlRc1VzU1BkRTlDRwptaGpRcDdKOWRNQTIrWDhsK0NnaU8yU2J3SG5idmwrWEtLaElzaGNRSWp2UitMdHZnRFFoZkN6TlJwNTBnRlRUCkZNZER6RlVJakVhYVRleDh2cUFkV1lQcjl5SitVSGdUVWhTdVB5eG41UHYrVmR1MmhTai9pVzJpeEVUQ2J5ZkUKOWt6R2FsMGJaQWRiWFZEcHFoam1rbHAzVXlkQm1xRzVoMmJ4cS9ZeFRKYmZ4S01RQm03Ukw4MDBHbjYwUWhsUwpDTkNmWVFRa290cmlnMzNzWHl4Q1RSZDQ3Ymk1Zmlta2JoOVllcnU5Q29sUDVqQUQwSm43NjBxYk5MZjcveFNCCnJRRENzL0k4R1lYMmtkQllrOUxpYm03Y1FhNjRrRDVaMzZtdGdNNERGQjEwbUMxaDVRZVpuRUJtczdKei9PUTMKdWtUWU1JYTBUT1VnY25jTG40K0pKckZQTkxYak9rNVdid0FzN0hYKzEyVitXb05oSmhVdlhMYXFHTTcrUWtUUwplMERJazVCZFZxUlp4VURJRDM0OEhQR0Ntc21VUlRGRDcxbEZKMy91Zkg3a2FHTmVzUnBnZFplSmFGUlFybS93CkpLQ1c2SXJJbEt1cWNpNDRMdkpBYTRpbEJCZ0JBZ0FQQlFKWEs4VnJBaHNNQlFrQUFWR0FBQW9KRUk4cVFGSFIKOUJQWHdVVUQvaTh5eStTOVpjdWhWcUxuTmNXNkxzSHhUaHE2MXVMRysxcTg3aFBYVGxLMmt3M0M5QTI2OUlqOApBUkhRaGpBSUFSSkM3MHNCaWVKK0xMMlZWa1ZYakVnYnpqdlNHTUE3dkRXRlBJOHovdHVxSnBKeW1zR0tEbFJ4CkptSVBkRFFOVlJtZGV6cnd1WlNlaVJabE43SjNNNnQvenJCNzFHVU9CakhLS2Jua2pKdUQKPXpyZG4KLS0tLS1FTkQgUEdQIFBSSVZBVEUgS0VZIEJMT0NLLS0tLS0K");
    ::sax::Converter::decodeBase64(aKeyID, packet);
    aGpgEncryptionData[1].Value <<= aKeyID;
    aGpgEncryptionData[2].Name = "CipherValue";
    OUString cipher("FAm4BDOfQRJ66/ecfIByCck3JxaKYYEWwms7z+Vsb+iqPWyPGdbgJNkRBAWH4V92JvMoc/QcD/1+z+iRvR6PMGdDHAyprIh5uGHs7mo+dqabJU0qOhHb16InW2XO1GqhmjzMDUw+q4ot28jpfIVSMKPlf6b8vnNUICMJjXn+aB8=");
    ::sax::Converter::decodeBase64(aKeyID, cipher);
    aGpgEncryptionData[2].Value <<= aKeyID;

    aEncryptionData[0].Name = PACKAGE_ENCRYPTIONDATA_SHA256UTF8;
    aEncryptionData[0].Value <<= aVector;

    aContainer[0].Name = "GpgInfos";
    aContainer[0].Value = makeAny(aGpgEncryptionData);
    aContainer[1].Name = "EncryptionKey";
    aContainer[1].Value = makeAny(aEncryptionData);

    return aContainer;
}

bool OStorageHelper::IsValidZipEntryFileName( const OUString& aName, bool bSlashAllowed )
{
    return IsValidZipEntryFileName( aName.getStr(), aName.getLength(), bSlashAllowed );
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


bool OStorageHelper::PathHasSegment( const OUString& aPath, const OUString& aSegment )
{
    bool bResult = false;
    const sal_Int32 nPathLen = aPath.getLength();
    const sal_Int32 nSegLen = aSegment.getLength();

    if ( !aSegment.isEmpty() && nPathLen >= nSegLen )
    {
        OUString aEndSegment = "/" + aSegment;
        OUString aInternalSegment = aEndSegment + "/";

        if ( aPath.indexOf( aInternalSegment ) >= 0 )
            bResult = true;

        if ( !bResult && aPath.startsWith( aSegment ) )
        {
            if ( nPathLen == nSegLen || aPath[nSegLen] == '/' )
                bResult = true;
        }

        if ( !bResult && nPathLen > nSegLen && aPath.copy( nPathLen - nSegLen - 1, nSegLen + 1 ) == aEndSegment )
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
    for (Impl::reverse_iterator iter = m_xBadness->rbegin();
            iter != m_xBadness->rend(); ++iter) // reverse order (outwards)
    {
        uno::Reference<embed::XTransactedObject> const xTransaction(*iter,
                uno::UNO_QUERY);
        if (xTransaction.is())
        {
            xTransaction->commit();
        }
    }
}

static void splitPath( std::vector<OUString> &rElems,
                       const OUString& rPath )
{
    for (sal_Int32 i = 0; i >= 0;)
        rElems.push_back( rPath.getToken( 0, '/', i ) );
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
        const OUString& rPath, sal_uInt32 nOpenMode,
        LifecycleProxy const &rNastiness )
{
    std::vector<OUString> aElems;
    splitPath( aElems, rPath );
    return LookupStorageAtPath( xStorage, aElems, nOpenMode, rNastiness );
}

uno::Reference< io::XStream > OStorageHelper::GetStreamAtPath(
        const uno::Reference< embed::XStorage > &xParentStorage,
        const OUString& rPath, sal_uInt32 nOpenMode,
        LifecycleProxy const &rNastiness )
{
    std::vector<OUString> aElems;
    splitPath( aElems, rPath );
    OUString aName( aElems.back() );
    aElems.pop_back();
    sal_uInt32 nStorageMode = nOpenMode & ~embed::ElementModes::TRUNCATE;
    uno::Reference< embed::XStorage > xStorage(
        LookupStorageAtPath( xParentStorage, aElems, nStorageMode, rNastiness ),
        uno::UNO_QUERY_THROW );
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
