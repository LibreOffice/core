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
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContextSupplier.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>

#include <vector>
#include <rtl/digest.h>

#include <ucbhelper/content.hxx>

#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/documentconstants.hxx>

#include <comphelper/storagehelper.hxx>

using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::embed;
using namespace com::sun::star::beans;
using namespace com::sun::star::ucb;
using namespace com::sun::star::xml::crypto;

using rtl::OUString;
using rtl::OString;

namespace comphelper {

Reference< XSingleServiceFactory >
    OStorageHelper::GetStorageFactory(const Reference< XMultiServiceFactory >& xSF )
    throw ( Exception )
{
    Reference< XMultiServiceFactory > xFactory = xSF.is() ? xSF : ::comphelper::getProcessServiceFactory();
    if ( !xFactory.is() )
        throw RuntimeException();

    OUString sService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.StorageFactory"));

    Reference < XSingleServiceFactory > xStorageFactory(
                    xFactory->createInstance(sService), UNO_QUERY);

    if ( !xStorageFactory.is() )
    {
        throw RuntimeException(OUString(
            RTL_CONSTASCII_USTRINGPARAM("Could not load: ")) + sService,
            Reference< XInterface >());
    }

    return xStorageFactory;
}

Reference< XSingleServiceFactory > OStorageHelper::GetFileSystemStorageFactory(
                            const Reference< XMultiServiceFactory >& xSF )
        throw ( Exception )
{
    Reference< XMultiServiceFactory > xFactory = xSF.is() ? xSF : ::comphelper::getProcessServiceFactory();
    if ( !xFactory.is() )
        throw RuntimeException();

    OUString sService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.FileSystemStorageFactory"));

    Reference < XSingleServiceFactory > xStorageFactory(
                    xFactory->createInstance(sService), UNO_QUERY);

    if ( !xStorageFactory.is() )
    {
        throw RuntimeException(OUString(
            RTL_CONSTASCII_USTRINGPARAM("Could not load: ")) + sService,
            Reference< XInterface >());
    }

    return xStorageFactory;
}

Reference< XStorage > OStorageHelper::GetTemporaryStorage(
            const Reference< XMultiServiceFactory >& xFactory )
    throw ( Exception )
{
    Reference< XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstance(),
                                                    UNO_QUERY );
    if ( !xTempStorage.is() )
        throw RuntimeException();

    return xTempStorage;
}

Reference< XStorage > OStorageHelper::GetStorageFromURL(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const Reference< XMultiServiceFactory >& xFactory )
    throw ( Exception )
{
    Sequence< Any > aArgs( 2 );
    aArgs[0] <<= aURL;
    aArgs[1] <<= nStorageMode;

    Reference< XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ),
                                                    UNO_QUERY );
    if ( !xTempStorage.is() )
        throw RuntimeException();

    return xTempStorage;
}

Reference< XStorage > OStorageHelper::GetStorageFromURL2(
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const Reference< XMultiServiceFactory >& xFactory )
    throw ( Exception )
{
    Sequence< Any > aArgs( 2 );
    aArgs[0] <<= aURL;
    aArgs[1] <<= nStorageMode;

    Reference< XSingleServiceFactory > xFact;
    try {
        ::ucbhelper::Content aCntnt( aURL, Reference< XCommandEnvironment > () );
        if (aCntnt.isDocument())
        {
            xFact = GetStorageFactory( xFactory );
        }
        else
        {
            xFact = GetFileSystemStorageFactory( xFactory );
        }
    } catch (Exception &)
    { }

    if (!xFact.is()) throw RuntimeException();

    Reference< XStorage > xTempStorage( xFact->createInstanceWithArguments( aArgs ), UNO_QUERY );
    if ( !xTempStorage.is() )
        throw RuntimeException();

    return xTempStorage;
}

Reference< XStorage > OStorageHelper::GetStorageFromInputStream(
            const Reference < XInputStream >& xStream,
            const Reference< XMultiServiceFactory >& xFactory )
        throw ( Exception )
{
    Sequence< Any > aArgs( 2 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= ElementModes::READ;

    Reference< XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ), UNO_QUERY );
    if ( !xTempStorage.is() )
        throw RuntimeException();

    return xTempStorage;
}

Reference< XStorage > OStorageHelper::GetStorageFromStream(
            const Reference < XStream >& xStream,
            sal_Int32 nStorageMode,
            const Reference< XMultiServiceFactory >& xFactory )
        throw ( Exception )
{
    Sequence< Any > aArgs( 2 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= nStorageMode;

    Reference< XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ), UNO_QUERY );
    if ( !xTempStorage.is() )
        throw RuntimeException();

    return xTempStorage;
}

void OStorageHelper::CopyInputToOutput(
            const Reference< XInputStream >& xInput,
            const Reference< XOutputStream >& xOutput )
    throw ( Exception )
{
    static const sal_Int32 nConstBufferSize = 32000;

    sal_Int32 nRead;
    Sequence < sal_Int8 > aSequence ( nConstBufferSize );

    do
    {
        nRead = xInput->readBytes ( aSequence, nConstBufferSize );
        if ( nRead < nConstBufferSize )
        {
            Sequence < sal_Int8 > aTempBuf ( aSequence.getConstArray(), nRead );
            xOutput->writeBytes ( aTempBuf );
        }
        else
            xOutput->writeBytes ( aSequence );
    }
    while ( nRead == nConstBufferSize );
}

Reference< XInputStream > OStorageHelper::GetInputStreamFromURL(
            const OUString& aURL,
            const Reference< XMultiServiceFactory >& xSF )
    throw ( Exception )
{
    Reference< XMultiServiceFactory > xFactory = xSF.is() ? xSF : ::comphelper::getProcessServiceFactory();
    if ( !xFactory.is() )
        throw RuntimeException();

    Reference < XSimpleFileAccess > xTempAccess(
            xFactory->createInstance ( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ucb.SimpleFileAccess" )) ),
            UNO_QUERY );

    if ( !xTempAccess.is() )
        throw RuntimeException();

    Reference< XInputStream > xInputStream = xTempAccess->openFileRead( aURL );
    if ( !xInputStream.is() )
        throw RuntimeException();

    return xInputStream;
}

void OStorageHelper::SetCommonStorageEncryptionData(
            const Reference< XStorage >& xStorage,
            const Sequence< NamedValue >& aEncryptionData )
    throw ( Exception )
{
    Reference< XEncryptionProtectedSource2 > xEncrSet( xStorage, UNO_QUERY );
    if ( !xEncrSet.is() )
        throw IOException(); // TODO

    xEncrSet->setEncryptionData( aEncryptionData );
}

sal_Int32 OStorageHelper::GetXStorageFormat(
            const Reference< XStorage >& xStorage )
        throw ( Exception )
{
    Reference< XPropertySet > xStorProps( xStorage, UNO_QUERY_THROW );

    OUString aMediaType;
    xStorProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaType" )) ) >>= aMediaType;

    sal_Int32 nResult = 0;

    // TODO/LATER: the filter configuration could be used to detect it later, or batter a special service
    if (   aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_WRITER_ASCII        )
        || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII    )
        || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII )
        || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_DRAW_ASCII          )
        || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_IMPRESS_ASCII       )
        || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_CALC_ASCII          )
        || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_CHART_ASCII         )
        || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_VND_SUN_XML_MATH_ASCII          ) )
    {
        nResult = SOFFICE_FILEFORMAT_60;
    }
    else if (  aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII                  )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII              )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII           )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII               )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII          )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII           )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII                 )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII               )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII              )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_REPORT_ASCII                )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_REPORT_CHART_ASCII          )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII         )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII      )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII  )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII        )
            || aMediaType.equalsIgnoreAsciiCaseAscii(MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII      ) )
    {
        nResult = SOFFICE_FILEFORMAT_8;
    }
    else
    {
        // the mediatype is not known
        throw IllegalTypeException();
    }

    return nResult;
}

Reference< XStorage > OStorageHelper::GetStorageOfFormatFromURL(
            const OUString& aFormat,
            const OUString& aURL,
            sal_Int32 nStorageMode,
            const Reference< XMultiServiceFactory >& xFactory,
            sal_Bool bRepairStorage )
    throw ( Exception )
{
    Sequence< PropertyValue > aProps( 1 );
    aProps[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "StorageFormat" ) );
    aProps[0].Value <<= aFormat;

    if ( bRepairStorage )
    {
        aProps.realloc( 2 );
        aProps[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "RepairPackage" ) );
        aProps[1].Value <<= bRepairStorage;
    }

    Sequence< Any > aArgs( 3 );
    aArgs[0] <<= aURL;
    aArgs[1] <<= nStorageMode;
    aArgs[2] <<= aProps;

    Reference< XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ), UNO_QUERY );
    if ( !xTempStorage.is() )
        throw RuntimeException();

    return xTempStorage;
}

Reference< XStorage > OStorageHelper::GetStorageOfFormatFromInputStream(
            const OUString& aFormat,
            const Reference < XInputStream >& xStream,
            const Reference< XMultiServiceFactory >& xFactory,
            sal_Bool bRepairStorage )
        throw ( Exception )
{
    Sequence< PropertyValue > aProps( 1 );
    aProps[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "StorageFormat" ) );
    aProps[0].Value <<= aFormat;

    if ( bRepairStorage )
    {
        aProps.realloc( 2 );
        aProps[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "RepairPackage" ) );
        aProps[1].Value <<= bRepairStorage;
    }

    Sequence< Any > aArgs( 3 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= ElementModes::READ;
    aArgs[2] <<= aProps;

    Reference< XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ), UNO_QUERY );
    if ( !xTempStorage.is() )
        throw RuntimeException();

    return xTempStorage;
}

Reference< XStorage > OStorageHelper::GetStorageOfFormatFromStream(
            const OUString& aFormat,
            const Reference < XStream >& xStream,
            sal_Int32 nStorageMode,
            const Reference< XMultiServiceFactory >& xFactory,
            sal_Bool bRepairStorage )
        throw ( Exception )
{
    Sequence< PropertyValue > aProps( 1 );
    aProps[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "StorageFormat" ) );
    aProps[0].Value <<= aFormat;

    if ( bRepairStorage )
    {
        aProps.realloc( 2 );
        aProps[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "RepairPackage" ) );
        aProps[1].Value <<= bRepairStorage;
    }

    Sequence< Any > aArgs( 3 );
    aArgs[0] <<= xStream;
    aArgs[1] <<= nStorageMode;
    aArgs[2] <<= aProps;

    Reference< XStorage > xTempStorage( GetStorageFactory( xFactory )->createInstanceWithArguments( aArgs ), UNO_QUERY );
    if ( !xTempStorage.is() )
        throw RuntimeException();

    return xTempStorage;
}

Sequence< NamedValue > OStorageHelper::CreatePackageEncryptionData( const OUString& aPassword, const Reference< XMultiServiceFactory >& xSF )
{
    // TODO/LATER: Should not the method be part of DocPasswordHelper?
    Sequence< NamedValue > aEncryptionData;
    if ( !aPassword.isEmpty() )
    {
        sal_Int32 nSha1Ind = 0;
        // generate SHA256 start key
        try
        {
            Reference< XMultiServiceFactory > xFactory = xSF.is() ? xSF : ::comphelper::getProcessServiceFactory();
            if ( !xFactory.is() )
                throw RuntimeException();

            Reference< XDigestContextSupplier > xDigestContextSupplier( xFactory->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xml.crypto.NSSInitializer" ) ) ), UNO_QUERY_THROW );
            Reference< XDigestContext > xDigestContext( xDigestContextSupplier->getDigestContext( DigestID::SHA256, Sequence< NamedValue >() ), UNO_SET_THROW );

            OString aUTF8Password( OUStringToOString( aPassword, RTL_TEXTENCODING_UTF8 ) );
            xDigestContext->updateDigest( Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( aUTF8Password.getStr() ), aUTF8Password.getLength() ) );
            Sequence< sal_Int8 > aDigest = xDigestContext->finalizeDigestAndDispose();

            aEncryptionData.realloc( ++nSha1Ind );
            aEncryptionData[0].Name = PACKAGE_ENCRYPTIONDATA_SHA256UTF8;
            aEncryptionData[0].Value <<= aDigest;
        }
        catch ( Exception& )
        {
            OSL_ENSURE( false, "Can not create SHA256 digest!" );
        }

        // MS_1252 encoding was used for SO60 document format password encoding,
        // this encoding supports only a minor subset of nonascii characters,
        // but for compatibility reasons it has to be used for old document formats
        aEncryptionData.realloc( nSha1Ind + 2 );
        aEncryptionData[nSha1Ind].Name = PACKAGE_ENCRYPTIONDATA_SHA1UTF8;
        aEncryptionData[nSha1Ind + 1].Name = PACKAGE_ENCRYPTIONDATA_SHA1MS1252;

        rtl_TextEncoding pEncoding[2] = { RTL_TEXTENCODING_UTF8, RTL_TEXTENCODING_MS_1252 };

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

            aEncryptionData[nSha1Ind+nInd].Value <<= Sequence< sal_Int8 >( (sal_Int8*)pBuffer, RTL_DIGEST_LENGTH_SHA1 );
        }
    }

    return aEncryptionData;
}

sal_Bool OStorageHelper::IsValidZipEntryFileName( const OUString& aName, sal_Bool bSlashAllowed )
{
    return IsValidZipEntryFileName( aName.getStr(), aName.getLength(), bSlashAllowed );
}

sal_Bool OStorageHelper::IsValidZipEntryFileName(
    const sal_Unicode *pChar, sal_Int32 nLength, sal_Bool bSlashAllowed )
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
                return sal_False;
            case '/':
                if ( !bSlashAllowed )
                    return sal_False;
                break;
            default:
                if ( pChar[i] < 32  || (pChar[i] >= 0xD800 && pChar[i] <= 0xDFFF) )
                    return sal_False;
        }
    }
    return sal_True;
}

sal_Bool OStorageHelper::PathHasSegment( const OUString& aPath, const OUString& aSegment )
{
    sal_Bool bResult = sal_False;
    const sal_Int32 nPathLen = aPath.getLength();
    const sal_Int32 nSegLen = aSegment.getLength();

    if ( !aSegment.isEmpty() && nPathLen >= nSegLen )
    {
        OUString aEndSegment( RTL_CONSTASCII_USTRINGPARAM( "/" ) );
        aEndSegment += aSegment;

        OUString aInternalSegment( aEndSegment );
        aInternalSegment += OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) );

        if ( aPath.indexOf( aInternalSegment ) >= 0 )
            bResult = sal_True;

        if ( !bResult && !aPath.compareTo( aSegment, nSegLen ) )
        {
            if ( nPathLen == nSegLen || aPath.getStr()[nSegLen] == (sal_Unicode)'/' )
                bResult = sal_True;
        }

        if ( !bResult && nPathLen > nSegLen && aPath.copy( nPathLen - nSegLen - 1, nSegLen + 1 ).equals( aEndSegment ) )
            bResult = sal_True;
    }

    return bResult;
}

class LifecycleProxy::Impl : public std::vector< Reference< XStorage > > {};

    LifecycleProxy::LifecycleProxy() : m_pBadness( new Impl() ) { }
    LifecycleProxy::~LifecycleProxy() { }

    void LifecycleProxy::commitStorages()
    {
        for (Impl::reverse_iterator iter = m_pBadness->rbegin();
                iter != m_pBadness->rend(); ++iter) // reverse order (outwards)
        {
            Reference<XTransactedObject> const xTransaction(*iter, UNO_QUERY);
            if (xTransaction.is())
                xTransaction->commit();
        }
    }

    static void splitPath( std::vector<OUString> &rElems,
                           const OUString& rPath )
    {
        for (sal_Int32 i = 0; i >= 0;)
            rElems.push_back( rPath.getToken( 0, '/', i ) );
    }

    static Reference< XStorage > LookupStorageAtPath(
            const Reference< XStorage > &xParentStorage,
            std::vector<OUString> &rElems, sal_uInt32 nOpenMode,
            LifecycleProxy &rNastiness )
    {
        Reference< XStorage > xStorage( xParentStorage );
        rNastiness.m_pBadness->push_back( xStorage );
        for( size_t i = 0; i < rElems.size() && xStorage.is(); i++ )
        {
            xStorage = xStorage->openStorageElement( rElems[i], nOpenMode );
            rNastiness.m_pBadness->push_back( xStorage );
        }
        return xStorage;
    }

    Reference< XStorage > OStorageHelper::GetStorageAtPath(
            const Reference< XStorage > &xStorage,
            const OUString& rPath, sal_uInt32 nOpenMode,
            LifecycleProxy &rNastiness )
    {
        std::vector<OUString> aElems;
        splitPath( aElems, rPath );
        return LookupStorageAtPath( xStorage, aElems, nOpenMode, rNastiness );
    }

    Reference< XStream > OStorageHelper::GetStreamAtPath(
            const Reference< XStorage > &xParentStorage,
            const OUString& rPath, sal_uInt32 nOpenMode,
            LifecycleProxy &rNastiness )
    {
        std::vector<OUString> aElems;
        splitPath( aElems, rPath );
        OUString aName( aElems.back() );
        aElems.pop_back();
        sal_uInt32 nStorageMode = nOpenMode & ~ElementModes::TRUNCATE;
        Reference< XStorage > xStorage(
            LookupStorageAtPath( xParentStorage, aElems, nStorageMode, rNastiness ),
            UNO_QUERY_THROW );
        return xStorage->openStreamElement( aName, nOpenMode );
    }

    Reference< XStream > OStorageHelper::GetStreamAtPackageURL(
            Reference< XStorage > const& xParentStorage,
            const OUString& rURL, sal_uInt32 const nOpenMode,
            LifecycleProxy & rNastiness)
    {
        static char const s_PkgScheme[] = "vnd.sun.star.Package:";
        if (0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength(
                    rURL.getStr(), rURL.getLength(),
                    s_PkgScheme, SAL_N_ELEMENTS(s_PkgScheme) - 1))
        {
            OUString const path(rURL.copy(SAL_N_ELEMENTS(s_PkgScheme)-1));
            return GetStreamAtPath(xParentStorage, path, nOpenMode, rNastiness);
        }
        return 0;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
