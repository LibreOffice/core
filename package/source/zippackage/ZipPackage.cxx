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

#include <ZipPackage.hxx>
#include <ZipPackageSink.hxx>
#include <ZipEnumeration.hxx>
#include <ZipPackageStream.hxx>
#include <ZipPackageFolder.hxx>
#include <ZipOutputEntry.hxx>
#include <ZipOutputStream.hxx>
#include <ZipPackageBuffer.hxx>
#include <ZipFile.hxx>
#include <PackageConstants.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/packages/manifest/ManifestReader.hpp>
#include <com/sun/star/packages/manifest/ManifestWriter.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <comphelper/fileurl.hxx>
#include <ucbhelper/content.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/UseBackupException.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <cppuhelper/implbase.hxx>
#include <ContentInfo.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uri.hxx>
#include <rtl/random.h>
#include <rtl/instance.hxx>
#include <osl/time.h>
#include <osl/diagnose.h>
#include "com/sun/star/io/XAsyncOutputMonitor.hpp"

#include <cstring>
#include <memory>
#include <vector>

#include <comphelper/processfactory.hxx>
#include <comphelper/seekableinput.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/sequence.hxx>

using namespace std;
using namespace osl;
using namespace cppu;
using namespace ucbhelper;
using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::task;
using namespace com::sun::star::beans;
using namespace com::sun::star::packages;
using namespace com::sun::star::container;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::manifest;
using namespace com::sun::star::packages::zip::ZipConstants;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

class ActiveDataStreamer : public ::cppu::WeakImplHelper< XActiveDataStreamer >
{
    uno::Reference< XStream > mStream;
public:

    virtual uno::Reference< XStream > SAL_CALL getStream()
            throw( RuntimeException, std::exception ) override
            { return mStream; }

    virtual void SAL_CALL setStream( const uno::Reference< XStream >& stream )
            throw( RuntimeException, std::exception ) override
            { mStream = stream; }
};

class DummyInputStream : public ::cppu::WeakImplHelper< XInputStream >
{
    virtual sal_Int32 SAL_CALL readBytes( uno::Sequence< sal_Int8 >&, sal_Int32 )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception ) override
        { return 0; }

    virtual sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< sal_Int8 >&, sal_Int32 )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception ) override
        { return 0; }

    virtual void SAL_CALL skipBytes( sal_Int32 )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception ) override
        {}

    virtual sal_Int32 SAL_CALL available()
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception ) override
        { return 0; }

    virtual void SAL_CALL closeInput()
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException, std::exception ) override
        {}
};

ZipPackage::ZipPackage ( const uno::Reference < XComponentContext > &xContext )
: m_aMutexHolder( new SotMutexHolder )
, m_nStartKeyGenerationID( xml::crypto::DigestID::SHA1 )
, m_nChecksumDigestID( xml::crypto::DigestID::SHA1_1K )
, m_nCommonEncryptionID( xml::crypto::CipherID::BLOWFISH_CFB_8 )
, m_bHasEncryptedEntries ( false )
, m_bHasNonEncryptedEntries ( false )
, m_bInconsistent ( false )
, m_bForceRecovery ( false )
, m_bMediaTypeFallbackUsed ( false )
, m_nFormat( embed::StorageFormats::PACKAGE ) // package is the default format
, m_bAllowRemoveOnInsert( true )
, m_eMode ( e_IMode_None )
, m_xContext( xContext )
, m_pRootFolder( nullptr )
, m_pZipFile( nullptr )
{
    m_xRootFolder = m_pRootFolder = new ZipPackageFolder( m_xContext, m_nFormat, m_bAllowRemoveOnInsert );
}

ZipPackage::~ZipPackage()
{
    delete m_pZipFile;

    // All folders and streams contain pointers to their parents, when a parent diappeares
    // it should disconnect all the children from itself during destruction automatically.
    // So there is no need in explicit m_pRootFolder->releaseUpwardRef() call here any more
    // since m_pRootFolder has no parent and cleaning of its children will be done automatically
    // during m_pRootFolder dying by refcount.
}

bool ZipPackage::isLocalFile() const
{
    return comphelper::isFileUrl(m_aURL);
}

void ZipPackage::parseManifest()
{
    if ( m_nFormat == embed::StorageFormats::PACKAGE )
    {
        bool bManifestParsed = false;
        const OUString sMeta ("META-INF");
        if ( m_xRootFolder->hasByName( sMeta ) )
        {
            const OUString sManifest ("manifest.xml");

            try {
                uno::Reference< XUnoTunnel > xTunnel;
                Any aAny = m_xRootFolder->getByName( sMeta );
                aAny >>= xTunnel;
                uno::Reference< XNameContainer > xMetaInfFolder( xTunnel, UNO_QUERY );
                if ( xMetaInfFolder.is() && xMetaInfFolder->hasByName( sManifest ) )
                {
                    aAny = xMetaInfFolder->getByName( sManifest );
                    aAny >>= xTunnel;
                    uno::Reference < XActiveDataSink > xSink ( xTunnel, UNO_QUERY );
                    if ( xSink.is() )
                    {
                        uno::Reference < XManifestReader > xReader = ManifestReader::create( m_xContext );

                        const OUString sPropFullPath ("FullPath");
                        const OUString sPropVersion ("Version");
                        const OUString sPropMediaType ("MediaType");
                        const OUString sPropInitialisationVector ("InitialisationVector");
                        const OUString sPropSalt ("Salt");
                        const OUString sPropIterationCount ("IterationCount");
                        const OUString sPropSize ("Size");
                        const OUString sPropDigest ("Digest");
                        const OUString sPropDerivedKeySize ("DerivedKeySize");
                        const OUString sPropDigestAlgorithm ("DigestAlgorithm");
                        const OUString sPropEncryptionAlgorithm ("EncryptionAlgorithm");
                        const OUString sPropStartKeyAlgorithm ("StartKeyAlgorithm");

                        uno::Sequence < uno::Sequence < PropertyValue > > aManifestSequence = xReader->readManifestSequence ( xSink->getInputStream() );
                        sal_Int32 nLength = aManifestSequence.getLength();
                        const uno::Sequence < PropertyValue > *pSequence = aManifestSequence.getConstArray();
                        ZipPackageStream *pStream = nullptr;
                        ZipPackageFolder *pFolder = nullptr;

                        for ( sal_Int32 i = 0; i < nLength ; i++, pSequence++ )
                        {
                            OUString sPath, sMediaType, sVersion;
                            const PropertyValue *pValue = pSequence->getConstArray();
                            const Any *pSalt = nullptr, *pVector = nullptr, *pCount = nullptr, *pSize = nullptr, *pDigest = nullptr, *pDigestAlg = nullptr, *pEncryptionAlg = nullptr, *pStartKeyAlg = nullptr, *pDerivedKeySize = nullptr;
                            for ( sal_Int32 j = 0, nNum = pSequence->getLength(); j < nNum; j++ )
                            {
                                if ( pValue[j].Name.equals( sPropFullPath ) )
                                    pValue[j].Value >>= sPath;
                                else if ( pValue[j].Name.equals( sPropVersion ) )
                                    pValue[j].Value >>= sVersion;
                                else if ( pValue[j].Name.equals( sPropMediaType ) )
                                    pValue[j].Value >>= sMediaType;
                                else if ( pValue[j].Name.equals( sPropSalt ) )
                                    pSalt = &( pValue[j].Value );
                                else if ( pValue[j].Name.equals( sPropInitialisationVector ) )
                                    pVector = &( pValue[j].Value );
                                else if ( pValue[j].Name.equals( sPropIterationCount ) )
                                    pCount = &( pValue[j].Value );
                                else if ( pValue[j].Name.equals( sPropSize ) )
                                    pSize = &( pValue[j].Value );
                                else if ( pValue[j].Name.equals( sPropDigest ) )
                                    pDigest = &( pValue[j].Value );
                                else if ( pValue[j].Name.equals( sPropDigestAlgorithm ) )
                                    pDigestAlg = &( pValue[j].Value );
                                else if ( pValue[j].Name.equals( sPropEncryptionAlgorithm ) )
                                    pEncryptionAlg = &( pValue[j].Value );
                                else if ( pValue[j].Name.equals( sPropStartKeyAlgorithm ) )
                                    pStartKeyAlg = &( pValue[j].Value );
                                else if ( pValue[j].Name.equals( sPropDerivedKeySize ) )
                                    pDerivedKeySize = &( pValue[j].Value );
                            }

                            if ( !sPath.isEmpty() && hasByHierarchicalName ( sPath ) )
                            {
                                aAny = getByHierarchicalName( sPath );
                                uno::Reference < XUnoTunnel > xUnoTunnel;
                                aAny >>= xUnoTunnel;
                                sal_Int64 nTest=0;
                                if ( (nTest = xUnoTunnel->getSomething( ZipPackageFolder::static_getImplementationId() )) != 0 )
                                {
                                    pFolder = reinterpret_cast < ZipPackageFolder* > ( nTest );
                                    pFolder->SetMediaType ( sMediaType );
                                    pFolder->SetVersion ( sVersion );
                                }
                                else
                                {
                                    pStream = reinterpret_cast < ZipPackageStream* > ( xUnoTunnel->getSomething( ZipPackageStream::static_getImplementationId() ));
                                    pStream->SetMediaType ( sMediaType );
                                    pStream->SetFromManifest( true );

                                    if ( pSalt && pVector && pCount && pSize && pDigest && pDigestAlg && pEncryptionAlg )
                                    {
                                        uno::Sequence < sal_Int8 > aSequence;
                                        sal_Int64 nSize = 0;
                                        sal_Int32 nCount = 0, nDigestAlg = 0, nEncryptionAlg = 0;
                                        sal_Int32 nDerivedKeySize = 16, nStartKeyAlg = xml::crypto::DigestID::SHA1;

                                        pStream->SetToBeEncrypted ( true );

                                        *pSalt >>= aSequence;
                                        pStream->setSalt ( aSequence );

                                        *pVector >>= aSequence;
                                        pStream->setInitialisationVector ( aSequence );

                                        *pCount >>= nCount;
                                        pStream->setIterationCount ( nCount );

                                        *pSize >>= nSize;
                                        pStream->setSize ( nSize );

                                        *pDigest >>= aSequence;
                                        pStream->setDigest ( aSequence );

                                        *pDigestAlg >>= nDigestAlg;
                                        pStream->SetImportedChecksumAlgorithm( nDigestAlg );

                                        *pEncryptionAlg >>= nEncryptionAlg;
                                        pStream->SetImportedEncryptionAlgorithm( nEncryptionAlg );

                                        if ( pDerivedKeySize )
                                            *pDerivedKeySize >>= nDerivedKeySize;
                                        pStream->SetImportedDerivedKeySize( nDerivedKeySize );

                                        if ( pStartKeyAlg )
                                            *pStartKeyAlg >>= nStartKeyAlg;
                                        pStream->SetImportedStartKeyAlgorithm( nStartKeyAlg );

                                        pStream->SetToBeCompressed ( true );
                                        pStream->SetToBeEncrypted ( true );
                                        pStream->SetIsEncrypted ( true );
                                        if ( !m_bHasEncryptedEntries && pStream->getName() == "content.xml" )
                                        {
                                            m_bHasEncryptedEntries = true;
                                            m_nStartKeyGenerationID = nStartKeyAlg;
                                            m_nChecksumDigestID = nDigestAlg;
                                            m_nCommonEncryptionID = nEncryptionAlg;
                                        }
                                    }
                                    else
                                        m_bHasNonEncryptedEntries = true;
                                }
                            }
                        }

                        bManifestParsed = true;
                    }

                    // now hide the manifest.xml file from user
                    xMetaInfFolder->removeByName( sManifest );
                }
            }
            catch( Exception& )
            {
                if ( !m_bForceRecovery )
                    throw;
            }
        }

        if ( !bManifestParsed && !m_bForceRecovery )
            throw ZipIOException(
                THROW_WHERE "Could not parse manifest.xml\n" );

        const OUString sMimetype ("mimetype");
        if ( m_xRootFolder->hasByName( sMimetype ) )
        {
            // get mediatype from the "mimetype" stream
            OUString aPackageMediatype;
            uno::Reference< lang::XUnoTunnel > xMimeTypeTunnel;
            m_xRootFolder->getByName( sMimetype ) >>= xMimeTypeTunnel;
            uno::Reference < io::XActiveDataSink > xMimeSink( xMimeTypeTunnel, UNO_QUERY );
            if ( xMimeSink.is() )
            {
                uno::Reference< io::XInputStream > xMimeInStream = xMimeSink->getInputStream();
                if ( xMimeInStream.is() )
                {
                    // Mediatypes longer than 1024 symbols should not appear here
                    uno::Sequence< sal_Int8 > aData( 1024 );
                    sal_Int32 nRead = xMimeInStream->readBytes( aData, 1024 );
                    if ( nRead > aData.getLength() )
                        nRead = aData.getLength();

                    if ( nRead )
                        aPackageMediatype = OUString( reinterpret_cast<char const *>(aData.getConstArray()), nRead, RTL_TEXTENCODING_ASCII_US );
                }
            }

            if ( !bManifestParsed )
            {
                // the manifest.xml could not be successfully parsed, this is an inconsistent package
                if ( aPackageMediatype.startsWith("application/vnd.") )
                {
                    // accept only types that look similar to own mediatypes
                    m_pRootFolder->SetMediaType( aPackageMediatype );
                    m_bMediaTypeFallbackUsed = true;
                }
            }
            else if ( !m_bForceRecovery )
            {
                // the mimetype stream should contain the information from manifest.xml
                if ( !m_pRootFolder->GetMediaType().equals( aPackageMediatype ) )
                    throw ZipIOException(
                        THROW_WHERE
                        "mimetype conflicts with manifest.xml, \""
                        + m_pRootFolder->GetMediaType() + "\" vs. \""
                        + aPackageMediatype + "\"" );
            }

            m_xRootFolder->removeByName( sMimetype );
        }

        m_bInconsistent = m_pRootFolder->LookForUnexpectedODF12Streams( OUString() );

        bool bODF12AndNewer = ( m_pRootFolder->GetVersion().compareTo( ODFVER_012_TEXT ) >= 0 );
        if ( !m_bForceRecovery && bODF12AndNewer )
        {
            bool bDifferentStartKeyAlgorithm = false;

            if ( m_bInconsistent )
            {
                // this is an ODF1.2 document that contains streams not referred in the manifest.xml;
                // in case of ODF1.2 documents without version in manifest.xml the property IsInconsistent
                // should be checked later
                throw ZipIOException(
                    THROW_WHERE "there are streams not referred in manifest.xml" );
            }
            else if ( bDifferentStartKeyAlgorithm )
            {
                // all the streams should be encrypted with the same StartKey in ODF1.2
                // TODO/LATER: in future the exception should be thrown
                OSL_ENSURE( false, "ODF1.2 contains different StartKey Algorithms" );
                // throw ZipIOException( THROW_WHERE "More than one Start Key Generation algorithm is specified!" );
            }
        }

        // in case it is a correct ODF1.2 document, the version must be set
        // and the META-INF folder is reserved for package format
        if ( bODF12AndNewer )
            m_xRootFolder->removeByName( sMeta );
    }
}

void ZipPackage::parseContentType()
{
    if ( m_nFormat == embed::StorageFormats::OFOPXML )
    {
        const OUString aContentTypes("[Content_Types].xml");
        try {
            // the content type must exist in OFOPXML format!
            if ( !m_xRootFolder->hasByName( aContentTypes ) )
                throw io::IOException(THROW_WHERE "Wrong format!" );

            uno::Reference< lang::XUnoTunnel > xTunnel;
            uno::Any aAny = m_xRootFolder->getByName( aContentTypes );
            aAny >>= xTunnel;
            uno::Reference < io::XActiveDataSink > xSink( xTunnel, UNO_QUERY );
            if ( xSink.is() )
            {
                uno::Reference< io::XInputStream > xInStream = xSink->getInputStream();
                if ( xInStream.is() )
                {
                    sal_Int32 nInd = 0;
                    // here aContentTypeInfo[0] - Defaults, and aContentTypeInfo[1] - Overrides
                    uno::Sequence< uno::Sequence< beans::StringPair > > aContentTypeInfo =
                        ::comphelper::OFOPXMLHelper::ReadContentTypeSequence( xInStream, m_xContext );

                    if ( aContentTypeInfo.getLength() != 2 )
                        throw io::IOException(THROW_WHERE );

                    // set the implicit types fist
                    for ( nInd = 0; nInd < aContentTypeInfo[0].getLength(); nInd++ )
                        m_pRootFolder->setChildStreamsTypeByExtension( aContentTypeInfo[0][nInd] );

                    // now set the explicit types
                    for ( nInd = 0; nInd < aContentTypeInfo[1].getLength(); nInd++ )
                    {
                        OUString aPath;
                        if ( aContentTypeInfo[1][nInd].First.toChar() == ( sal_Unicode )'/' )
                            aPath = aContentTypeInfo[1][nInd].First.copy( 1 );
                        else
                            aPath = aContentTypeInfo[1][nInd].First;

                        if ( !aPath.isEmpty() && hasByHierarchicalName( aPath ) )
                        {
                            uno::Any aIterAny = getByHierarchicalName( aPath );
                            uno::Reference < lang::XUnoTunnel > xIterTunnel;
                            aIterAny >>= xIterTunnel;
                            sal_Int64 nTest = xIterTunnel->getSomething( ZipPackageStream::static_getImplementationId() );
                            if ( nTest != 0 )
                            {
                                // this is a package stream, in OFOPXML format only streams can have mediatype
                                ZipPackageStream *pStream = reinterpret_cast < ZipPackageStream* > ( nTest );
                                pStream->SetMediaType( aContentTypeInfo[1][nInd].Second );
                            }
                        }
                    }
                }
            }

            m_xRootFolder->removeByName( aContentTypes );
        }
        catch( uno::Exception& )
        {
            if ( !m_bForceRecovery )
                throw;
        }
    }
}

void ZipPackage::getZipFileContents()
{
    std::unique_ptr < ZipEnumeration > xEnum(m_pZipFile->entries());
    ZipPackageStream *pPkgStream;
    ZipPackageFolder *pPkgFolder, *pCurrent;
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nStreamIndex;
    FolderHash::iterator aIter;

    while (xEnum->hasMoreElements())
    {
        nOldIndex = 0;
        pCurrent = m_pRootFolder;
        const ZipEntry & rEntry = *xEnum->nextElement();
        OUString rName = rEntry.sPath;

        if ( m_bForceRecovery )
        {
            // the PKZIP Application note version 6.2 does not allows to use '\' as separator
            // unfortunately it is used by some implementations, so we have to support it in recovery mode
            rName = rName.replace( '\\', '/' );
        }

        nStreamIndex = rName.lastIndexOf ( '/' );
        if ( nStreamIndex != -1 )
        {
            sDirName = rName.copy ( 0, nStreamIndex );
            aIter = m_aRecent.find ( sDirName );
            if ( aIter != m_aRecent.end() )
                pCurrent = ( *aIter ).second;
        }

        if ( pCurrent == m_pRootFolder )
        {
            sal_Int32 nIndex;
            while ( ( nIndex = rName.indexOf( '/', nOldIndex ) ) != -1 )
            {
                sTemp = rName.copy ( nOldIndex, nIndex - nOldIndex );
                if ( nIndex == nOldIndex )
                    break;
                if ( !pCurrent->hasByName( sTemp ) )
                {
                    pPkgFolder = new ZipPackageFolder( m_xContext, m_nFormat, m_bAllowRemoveOnInsert );
                    pPkgFolder->setName( sTemp );
                    pPkgFolder->doSetParent( pCurrent, true );
                    pCurrent = pPkgFolder;
                }
                else
                    pCurrent = pCurrent->doGetByName( sTemp ).pFolder;
                nOldIndex = nIndex+1;
            }
            if ( nStreamIndex != -1 && !sDirName.isEmpty() )
                m_aRecent [ sDirName ] = pCurrent;
        }
        if ( rName.getLength() -1 != nStreamIndex )
        {
            nStreamIndex++;
            sTemp = rName.copy( nStreamIndex, rName.getLength() - nStreamIndex );
            pPkgStream = new ZipPackageStream( *this, m_xContext, m_nFormat, m_bAllowRemoveOnInsert );
            pPkgStream->SetPackageMember( true );
            pPkgStream->setZipEntryOnLoading( rEntry );
            pPkgStream->setName( sTemp );
            pPkgStream->doSetParent( pCurrent, true );
        }
    }

    if ( m_nFormat == embed::StorageFormats::PACKAGE )
        parseManifest();
    else if ( m_nFormat == embed::StorageFormats::OFOPXML )
        parseContentType();
}

void SAL_CALL ZipPackage::initialize( const uno::Sequence< Any >& aArguments )
        throw( Exception, RuntimeException, std::exception )
{
    uno::Reference< XProgressHandler > xProgressHandler;
    beans::NamedValue aNamedValue;

    if ( aArguments.getLength() )
    {
        bool bHaveZipFile = true;

        for( int ind = 0; ind < aArguments.getLength(); ind++ )
        {
            OUString aParamUrl;
            if ( ( aArguments[ind] >>= aParamUrl ))
            {
                m_eMode = e_IMode_URL;
                try
                {
                    sal_Int32 nParam = aParamUrl.indexOf( '?' );
                    if ( nParam >= 0 )
                    {
                        m_aURL = aParamUrl.copy( 0, nParam );
                        OUString aParam = aParamUrl.copy( nParam + 1 );

                          sal_Int32 nIndex = 0;
                        do
                        {
                            OUString aCommand = aParam.getToken( 0, '&', nIndex );
                            if ( aCommand == "repairpackage" )
                            {
                                m_bForceRecovery = true;
                                break;
                            }
                            else if ( aCommand == "purezip" )
                            {
                                m_nFormat = embed::StorageFormats::ZIP;
                                m_pRootFolder->setPackageFormat_Impl( m_nFormat );
                                break;
                            }
                            else if ( aCommand == "ofopxml" )
                            {
                                m_nFormat = embed::StorageFormats::OFOPXML;
                                m_pRootFolder->setPackageFormat_Impl( m_nFormat );
                                break;
                            }
                        }
                        while ( nIndex >= 0 );
                    }
                    else
                        m_aURL = aParamUrl;

                    Content aContent(
                        m_aURL, uno::Reference< XCommandEnvironment >(),
                        m_xContext );
                    Any aAny = aContent.getPropertyValue("Size");
                    sal_uInt64 aSize = 0;
                    // kind of optimization: treat empty files as nonexistent files
                    // and write to such files directly. Note that "Size" property is optional.
                    bool bHasSizeProperty = aAny >>= aSize;
                    if( !bHasSizeProperty || ( bHasSizeProperty && aSize ) )
                    {
                        uno::Reference < XActiveDataSink > xSink = new ZipPackageSink;
                        if ( aContent.openStream ( xSink ) )
                            m_xContentStream = xSink->getInputStream();
                    }
                    else
                        bHaveZipFile = false;
                }
                catch ( css::uno::Exception& )
                {
                    // Exception derived from uno::Exception thrown. This probably
                    // means the file doesn't exist...we'll create it at
                    // commitChanges time
                    bHaveZipFile = false;
                }
            }
            else if ( ( aArguments[ind] >>= m_xStream ) )
            {
                // a writable stream can implement both XStream & XInputStream
                m_eMode = e_IMode_XStream;
                m_xContentStream = m_xStream->getInputStream();
            }
            else if ( ( aArguments[ind] >>= m_xContentStream ) )
            {
                m_eMode = e_IMode_XInputStream;
            }
            else if ( ( aArguments[ind] >>= aNamedValue ) )
            {
                if ( aNamedValue.Name == "RepairPackage" )
                    aNamedValue.Value >>= m_bForceRecovery;
                else if ( aNamedValue.Name == "PackageFormat" )
                {
                    // setting this argument to true means Package format
                    // setting it to false means plain Zip format

                    bool bPackFormat = true;
                    aNamedValue.Value >>= bPackFormat;
                    if ( !bPackFormat )
                        m_nFormat = embed::StorageFormats::ZIP;

                    m_pRootFolder->setPackageFormat_Impl( m_nFormat );
                }
                else if ( aNamedValue.Name == "StorageFormat" )
                {
                    OUString aFormatName;
                    sal_Int32 nFormatID = 0;
                    if ( aNamedValue.Value >>= aFormatName )
                    {
                        if ( aFormatName == PACKAGE_STORAGE_FORMAT_STRING )
                            m_nFormat = embed::StorageFormats::PACKAGE;
                        else if ( aFormatName == ZIP_STORAGE_FORMAT_STRING )
                            m_nFormat = embed::StorageFormats::ZIP;
                        else if ( aFormatName == OFOPXML_STORAGE_FORMAT_STRING )
                            m_nFormat = embed::StorageFormats::OFOPXML;
                        else
                            throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );
                    }
                    else if ( aNamedValue.Value >>= nFormatID )
                    {
                        if ( nFormatID != embed::StorageFormats::PACKAGE
                          && nFormatID != embed::StorageFormats::ZIP
                          && nFormatID != embed::StorageFormats::OFOPXML )
                            throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );

                        m_nFormat = nFormatID;
                    }
                    else
                        throw lang::IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );

                    m_pRootFolder->setPackageFormat_Impl( m_nFormat );
                }
                else if ( aNamedValue.Name == "AllowRemoveOnInsert" )
                {
                    aNamedValue.Value >>= m_bAllowRemoveOnInsert;
                    m_pRootFolder->setRemoveOnInsertMode_Impl( m_bAllowRemoveOnInsert );
                }

                // for now the progress handler is not used, probably it will never be
                // if ( aNamedValue.Name == "ProgressHandler" )
            }
            else
            {
                // The URL is not acceptable
                throw css::uno::Exception (THROW_WHERE "Bad arguments.",
                    static_cast < ::cppu::OWeakObject * > ( this ) );
            }
        }

        try
        {
            if ( m_xContentStream.is() )
            {
                // the stream must be seekable, if it is not it will be wrapped
                m_xContentStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( m_xContentStream, m_xContext );
                m_xContentSeek.set( m_xContentStream, UNO_QUERY );
                if ( ! m_xContentSeek.is() )
                    throw css::uno::Exception (THROW_WHERE "The package component _requires_ an XSeekable interface!",
                            static_cast < ::cppu::OWeakObject * > ( this ) );

                if ( !m_xContentSeek->getLength() )
                    bHaveZipFile = false;
            }
            else
                bHaveZipFile = false;
        }
        catch ( css::uno::Exception& )
        {
            // Exception derived from uno::Exception thrown. This probably
            // means the file doesn't exist...we'll create it at
            // commitChanges time
            bHaveZipFile = false;
        }
        if ( bHaveZipFile )
        {
            bool bBadZipFile = false;
            OUString message;
            try
            {
                m_pZipFile = new ZipFile ( m_xContentStream, m_xContext, true, m_bForceRecovery, xProgressHandler );
                getZipFileContents();
            }
            catch ( IOException & e )
            {
                bBadZipFile = true;
                message = "IOException: " + e.Message;
            }
            catch ( ZipException & e )
            {
                bBadZipFile = true;
                message = "ZipException: " + e.Message;
            }
            catch ( Exception & )
            {
                if( m_pZipFile ) { delete m_pZipFile; m_pZipFile = nullptr; }
                throw;
            }

            if ( bBadZipFile )
            {
                // clean up the memory, and tell the UCB about the error
                if( m_pZipFile ) { delete m_pZipFile; m_pZipFile = nullptr; }

                throw css::packages::zip::ZipIOException (
                    THROW_WHERE "Bad Zip File, " + message,
                    static_cast < ::cppu::OWeakObject * > ( this ) );
            }
        }
    }
}

Any SAL_CALL ZipPackage::getByHierarchicalName( const OUString& aName )
        throw( NoSuchElementException, RuntimeException, std::exception )
{
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nIndex, nStreamIndex;
    FolderHash::iterator aIter;

    if ( ( nIndex = aName.getLength() ) == 1 && *aName.getStr() == '/' )
        return makeAny ( uno::Reference < XUnoTunnel > ( m_pRootFolder ) );
    else
    {
        nStreamIndex = aName.lastIndexOf ( '/' );
        bool bFolder = nStreamIndex == nIndex-1;
        if ( nStreamIndex != -1 )
        {
            sDirName = aName.copy ( 0, nStreamIndex );
            aIter = m_aRecent.find ( sDirName );
            if ( aIter != m_aRecent.end() )
            {
                if ( bFolder )
                {
                    sal_Int32 nDirIndex = aName.lastIndexOf ( '/', nStreamIndex );
                    sTemp = aName.copy ( nDirIndex == -1 ? 0 : nDirIndex+1, nStreamIndex-nDirIndex-1 );
                    if ( sTemp == ( *aIter ).second->getName() )
                        return makeAny ( uno::Reference < XUnoTunnel > ( ( *aIter ).second ) );
                    else
                        m_aRecent.erase ( aIter );
                }
                else
                {
                    sTemp = aName.copy ( nStreamIndex + 1 );
                    if ( ( *aIter ).second->hasByName( sTemp ) )
                        return ( *aIter ).second->getByName( sTemp );
                    else
                        m_aRecent.erase( aIter );
                }
            }
        }
        else
        {
            if ( m_pRootFolder->hasByName ( aName ) )
                return m_pRootFolder->getByName ( aName );
        }
        nOldIndex = 0;
        ZipPackageFolder * pCurrent = m_pRootFolder;
        ZipPackageFolder * pPrevious = nullptr;
        while ( ( nIndex = aName.indexOf( '/', nOldIndex )) != -1 )
        {
            sTemp = aName.copy ( nOldIndex, nIndex - nOldIndex );
            if ( nIndex == nOldIndex )
                break;
            if ( pCurrent->hasByName( sTemp ) )
            {
                pPrevious = pCurrent;
                pCurrent = pCurrent->doGetByName( sTemp ).pFolder;
            }
            else
                throw NoSuchElementException(THROW_WHERE );
            nOldIndex = nIndex+1;
        }
        if ( bFolder )
        {
            if ( nStreamIndex != -1 )
                m_aRecent[sDirName] = pPrevious;
            return makeAny ( uno::Reference < XUnoTunnel > ( pCurrent ) );
        }
        else
        {
            sTemp = aName.copy( nOldIndex, aName.getLength() - nOldIndex );
            if ( pCurrent->hasByName ( sTemp ) )
            {
                if ( nStreamIndex != -1 )
                    m_aRecent[sDirName] = pCurrent;
                return pCurrent->getByName( sTemp );
            }
            else
                throw NoSuchElementException(THROW_WHERE );
        }
    }
}

sal_Bool SAL_CALL ZipPackage::hasByHierarchicalName( const OUString& aName )
        throw( RuntimeException, std::exception )
{
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nIndex, nStreamIndex;
    FolderHash::iterator aIter;

    if ( ( nIndex = aName.getLength() ) == 1 && *aName.getStr() == '/' )
        return sal_True;

    try
    {
        nStreamIndex = aName.lastIndexOf ( '/' );
        bool bFolder = nStreamIndex == nIndex-1;
        if ( nStreamIndex != -1 )
        {
            sDirName = aName.copy ( 0, nStreamIndex );
            aIter = m_aRecent.find ( sDirName );
            if ( aIter != m_aRecent.end() )
            {
                if ( bFolder )
                {
                    sal_Int32 nDirIndex = aName.lastIndexOf ( '/', nStreamIndex );
                    sTemp = aName.copy ( nDirIndex == -1 ? 0 : nDirIndex+1, nStreamIndex-nDirIndex-1 );
                    if ( sTemp == ( *aIter ).second->getName() )
                        return sal_True;
                    else
                        m_aRecent.erase ( aIter );
                }
                else
                {
                    sTemp = aName.copy ( nStreamIndex + 1 );
                    if ( ( *aIter ).second->hasByName( sTemp ) )
                        return sal_True;
                    else
                        m_aRecent.erase( aIter );
                }
            }
        }
        else
        {
            if ( m_pRootFolder->hasByName ( aName ) )
                return sal_True;
        }
        ZipPackageFolder * pCurrent = m_pRootFolder;
        ZipPackageFolder * pPrevious = nullptr;
        nOldIndex = 0;
        while ( ( nIndex = aName.indexOf( '/', nOldIndex )) != -1 )
        {
            sTemp = aName.copy ( nOldIndex, nIndex - nOldIndex );
            if ( nIndex == nOldIndex )
                break;
            if ( pCurrent->hasByName( sTemp ) )
            {
                pPrevious = pCurrent;
                pCurrent = pCurrent->doGetByName( sTemp ).pFolder;
            }
            else
                return sal_False;
            nOldIndex = nIndex+1;
        }
        if ( bFolder )
        {
            m_aRecent[sDirName] = pPrevious;
            return sal_True;
        }
        else
        {
            sTemp = aName.copy( nOldIndex, aName.getLength() - nOldIndex );

            if ( pCurrent->hasByName( sTemp ) )
            {
                m_aRecent[sDirName] = pCurrent;
                return sal_True;
            }
        }
    }
    catch (const uno::RuntimeException &)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
        uno::Any e(::cppu::getCaughtException());
        throw lang::WrappedTargetRuntimeException(
            OUString("ZipPackage::hasByHierarchicalName"),
            nullptr, e);
    }
    return sal_False;
}

uno::Reference< XInterface > SAL_CALL ZipPackage::createInstance()
        throw( Exception, RuntimeException, std::exception )
{
    uno::Reference < XInterface > xRef = *( new ZipPackageStream( *this, m_xContext, m_nFormat, m_bAllowRemoveOnInsert ) );
    return xRef;
}

uno::Reference< XInterface > SAL_CALL ZipPackage::createInstanceWithArguments( const uno::Sequence< Any >& aArguments )
        throw( Exception, RuntimeException, std::exception )
{
    bool bArg = false;
    uno::Reference < XInterface > xRef;
    if ( aArguments.getLength() )
        aArguments[0] >>= bArg;
    if ( bArg )
        xRef = *new ZipPackageFolder( m_xContext, m_nFormat, m_bAllowRemoveOnInsert );
    else
        xRef = *new ZipPackageStream( *this, m_xContext, m_nFormat, m_bAllowRemoveOnInsert );

    return xRef;
}

void ZipPackage::WriteMimetypeMagicFile( ZipOutputStream& aZipOut )
{
    const OUString sMime ("mimetype");
    if ( m_xRootFolder->hasByName( sMime ) )
        m_xRootFolder->removeByName( sMime );

    ZipEntry * pEntry = new ZipEntry;
    sal_Int32 nBufferLength = m_pRootFolder->GetMediaType().getLength();
    OString sMediaType = OUStringToOString( m_pRootFolder->GetMediaType(), RTL_TEXTENCODING_ASCII_US );
    const uno::Sequence< sal_Int8 > aType( reinterpret_cast<sal_Int8 const *>(sMediaType.getStr()),
                                     nBufferLength );

    pEntry->sPath = sMime;
    pEntry->nMethod = STORED;
    pEntry->nSize = pEntry->nCompressedSize = nBufferLength;
    pEntry->nTime = ZipOutputStream::getCurrentDosTime();

    CRC32 aCRC32;
    aCRC32.update( aType );
    pEntry->nCrc = aCRC32.getValue();

    try
    {
        ZipOutputStream::setEntry(pEntry);
        aZipOut.writeLOC(pEntry);
        aZipOut.rawWrite(aType);
        aZipOut.rawCloseEntry();
    }
    catch ( const css::io::IOException & r )
    {
        throw WrappedTargetException(
                THROW_WHERE "Error adding mimetype to the ZipOutputStream!",
                static_cast < OWeakObject * > ( this ),
                makeAny( r ) );
    }
}

void ZipPackage::WriteManifest( ZipOutputStream& aZipOut, const vector< uno::Sequence < PropertyValue > >& aManList )
{
    // Write the manifest
    uno::Reference < XManifestWriter > xWriter = ManifestWriter::create( m_xContext );
    ZipEntry * pEntry = new ZipEntry;
    ZipPackageBuffer *pBuffer = new ZipPackageBuffer( n_ConstBufferSize );
    uno::Reference < XOutputStream > xManOutStream( *pBuffer, UNO_QUERY );

    pEntry->sPath = "META-INF/manifest.xml";
    pEntry->nMethod = DEFLATED;
    pEntry->nCrc = -1;
    pEntry->nSize = pEntry->nCompressedSize = -1;
    pEntry->nTime = ZipOutputStream::getCurrentDosTime();

    xWriter->writeManifestSequence ( xManOutStream,  comphelper::containerToSequence(aManList) );

    sal_Int32 nBufferLength = static_cast < sal_Int32 > ( pBuffer->getPosition() );
    pBuffer->realloc( nBufferLength );

    // the manifest.xml is never encrypted - so pass an empty reference
    ZipOutputStream::setEntry(pEntry);
    aZipOut.writeLOC(pEntry);
    ZipOutputEntry aZipEntry(aZipOut.getStream(), m_xContext, *pEntry, nullptr);
    aZipEntry.write(pBuffer->getSequence());
    aZipEntry.closeEntry();
    aZipOut.rawCloseEntry();
}

void ZipPackage::WriteContentTypes( ZipOutputStream& aZipOut, const vector< uno::Sequence < PropertyValue > >& aManList )
{
    ZipEntry* pEntry = new ZipEntry;
    ZipPackageBuffer *pBuffer = new ZipPackageBuffer( n_ConstBufferSize );
    uno::Reference< io::XOutputStream > xConTypeOutStream( *pBuffer, UNO_QUERY );

    pEntry->sPath = "[Content_Types].xml";
    pEntry->nMethod = DEFLATED;
    pEntry->nCrc = -1;
    pEntry->nSize = pEntry->nCompressedSize = -1;
    pEntry->nTime = ZipOutputStream::getCurrentDosTime();

    // Convert vector into a uno::Sequence
    // TODO/LATER: use Default entries in future
    uno::Sequence< beans::StringPair > aDefaultsSequence;
    uno::Sequence< beans::StringPair > aOverridesSequence( aManList.size() );
    sal_Int32 nSeqLength = 0;
    for ( vector< uno::Sequence< beans::PropertyValue > >::const_iterator aIter = aManList.begin(),
            aEnd = aManList.end();
         aIter != aEnd;
         ++aIter)
    {
        OUString aPath;
        OUString aType;
        OSL_ENSURE( ( *aIter )[PKG_MNFST_MEDIATYPE].Name == "MediaType" && ( *aIter )[PKG_MNFST_FULLPATH].Name == "FullPath",
                    "The mediatype sequence format is wrong!\n" );
        ( *aIter )[PKG_MNFST_MEDIATYPE].Value >>= aType;
        if ( !aType.isEmpty() )
        {
            // only nonempty type makes sense here
            nSeqLength++;
            ( *aIter )[PKG_MNFST_FULLPATH].Value >>= aPath;
            aOverridesSequence[nSeqLength-1].First = "/" + aPath;
            aOverridesSequence[nSeqLength-1].Second = aType;
        }
    }
    aOverridesSequence.realloc( nSeqLength );

    ::comphelper::OFOPXMLHelper::WriteContentSequence(
            xConTypeOutStream, aDefaultsSequence, aOverridesSequence, m_xContext );

    sal_Int32 nBufferLength = static_cast < sal_Int32 > ( pBuffer->getPosition() );
    pBuffer->realloc( nBufferLength );

    // there is no encryption in this format currently
    ZipOutputStream::setEntry(pEntry);
    aZipOut.writeLOC(pEntry);
    ZipOutputEntry aZipEntry(aZipOut.getStream(), m_xContext, *pEntry, nullptr);
    aZipEntry.write(pBuffer->getSequence());
    aZipEntry.closeEntry();
    aZipOut.rawCloseEntry();
}

void ZipPackage::ConnectTo( const uno::Reference< io::XInputStream >& xInStream )
{
    m_xContentSeek.set( xInStream, uno::UNO_QUERY_THROW );
    m_xContentStream = xInStream;

    // seek back to the beginning of the temp file so we can read segments from it
    m_xContentSeek->seek( 0 );
    if ( m_pZipFile )
        m_pZipFile->setInputStream( m_xContentStream );
    else
        m_pZipFile = new ZipFile ( m_xContentStream, m_xContext, false );
}

namespace
{
    class RandomPool
    {
    private:
        rtlRandomPool m_aRandomPool;
    public:
        RandomPool()
        {
            // Get a random number generator and seed it with current timestamp
            TimeValue aTime;
            osl_getSystemTime( &aTime );
            m_aRandomPool = rtl_random_createPool ();
            rtl_random_addBytes (m_aRandomPool, &aTime, 8);
        }
        rtlRandomPool get()
        {
            return m_aRandomPool;
        }
        ~RandomPool()
        {
            // Clean up random pool memory
            rtl_random_destroyPool(m_aRandomPool);
        }
    };
}

uno::Reference< io::XInputStream > ZipPackage::writeTempFile()
{
    // In case the target local file does not exist or empty
    // write directly to it otherwize create a temporary file to write to.
    // If a temporary file is created it is returned back by the method.
    // If the data written directly, xComponentStream will be switched here

    bool bUseTemp = true;
    uno::Reference < io::XInputStream > xResult;
    uno::Reference < io::XInputStream > xTempIn;

    uno::Reference < io::XOutputStream > xTempOut;
    uno::Reference< io::XActiveDataStreamer > xSink;

    if ( m_eMode == e_IMode_URL && !m_pZipFile && isLocalFile() )
    {
        xSink = openOriginalForOutput();
        if( xSink.is() )
        {
            uno::Reference< io::XStream > xStr = xSink->getStream();
            if( xStr.is() )
            {
                xTempOut = xStr->getOutputStream();
                if( xTempOut.is() )
                    bUseTemp = false;
            }
        }
    }
    else if ( m_eMode == e_IMode_XStream && !m_pZipFile )
    {
        // write directly to an empty stream
        xTempOut = m_xStream->getOutputStream();
        if( xTempOut.is() )
            bUseTemp = false;
    }

    if( bUseTemp )
    {
        // create temporary file
        uno::Reference < io::XTempFile > xTempFile( io::TempFile::create(m_xContext) );
        xTempOut.set( xTempFile->getOutputStream(), UNO_SET_THROW );
        xTempIn.set( xTempFile->getInputStream(), UNO_SET_THROW );
    }

    // Hand it to the ZipOutputStream:
    ZipOutputStream aZipOut( xTempOut );
    try
    {
        if ( m_nFormat == embed::StorageFormats::PACKAGE )
        {
            // Remove the old manifest.xml file as the
            // manifest will be re-generated and the
            // META-INF directory implicitly created if does not exist
            const OUString sMeta ("META-INF");

            if ( m_xRootFolder->hasByName( sMeta ) )
            {
                const OUString sManifest ("manifest.xml");

                uno::Reference< XUnoTunnel > xTunnel;
                Any aAny = m_xRootFolder->getByName( sMeta );
                aAny >>= xTunnel;
                uno::Reference< XNameContainer > xMetaInfFolder( xTunnel, UNO_QUERY );
                if ( xMetaInfFolder.is() && xMetaInfFolder->hasByName( sManifest ) )
                    xMetaInfFolder->removeByName( sManifest );
            }

            // Write a magic file with mimetype
            WriteMimetypeMagicFile( aZipOut );
        }
        else if ( m_nFormat == embed::StorageFormats::OFOPXML )
        {
            // Remove the old [Content_Types].xml file as the
            // file will be re-generated

            const OUString aContentTypes("[Content_Types].xml");

            if ( m_xRootFolder->hasByName( aContentTypes ) )
                m_xRootFolder->removeByName( aContentTypes );
        }

        // Create a vector to store data for the manifest.xml file
        vector < uno::Sequence < PropertyValue > > aManList;

        const OUString sMediaType ("MediaType");
        const OUString sVersion ("Version");
        const OUString sFullPath ("FullPath");

        if ( m_nFormat == embed::StorageFormats::PACKAGE )
        {
            uno::Sequence < PropertyValue > aPropSeq( PKG_SIZE_NOENCR_MNFST );
            aPropSeq [PKG_MNFST_MEDIATYPE].Name = sMediaType;
            aPropSeq [PKG_MNFST_MEDIATYPE].Value <<= m_pRootFolder->GetMediaType();
            aPropSeq [PKG_MNFST_VERSION].Name = sVersion;
            aPropSeq [PKG_MNFST_VERSION].Value <<= m_pRootFolder->GetVersion();
            aPropSeq [PKG_MNFST_FULLPATH].Name = sFullPath;
            aPropSeq [PKG_MNFST_FULLPATH].Value <<= OUString("/");

            aManList.push_back( aPropSeq );
        }

        {
            // This will be used to generate random salt and initialisation vectors
            // for encrypted streams
            RandomPool aRandomPool;

            // call saveContents ( it will recursively save sub-directories
            OUString aEmptyString;
            m_pRootFolder->saveContents(aEmptyString, aManList, aZipOut, GetEncryptionKey(), aRandomPool.get());
        }

        if( m_nFormat == embed::StorageFormats::PACKAGE )
        {
            WriteManifest( aZipOut, aManList );
        }
        else if( m_nFormat == embed::StorageFormats::OFOPXML )
        {
            WriteContentTypes( aZipOut, aManList );
        }

        aZipOut.finish();

        if( bUseTemp )
            xResult = xTempIn;

        // Update our References to point to the new temp file
        if( !bUseTemp )
        {
            // the case when the original contents were written directly
            xTempOut->flush();

            // in case the stream is based on a file it will implement the following interface
            // the call should be used to be sure that the contents are written to the file system
            uno::Reference< io::XAsyncOutputMonitor > asyncOutputMonitor( xTempOut, uno::UNO_QUERY );
            if ( asyncOutputMonitor.is() )
                asyncOutputMonitor->waitForCompletion();

            // no need to postpone switching to the new stream since the target was written directly
            uno::Reference< io::XInputStream > xNewStream;
            if ( m_eMode == e_IMode_URL )
                xNewStream = xSink->getStream()->getInputStream();
            else if ( m_eMode == e_IMode_XStream && m_xStream.is() )
                xNewStream = m_xStream->getInputStream();

            if ( xNewStream.is() )
                ConnectTo( xNewStream );
        }
    }
    catch ( uno::Exception& )
    {
        if( bUseTemp )
        {
            // no information loss appears, thus no special handling is required
               uno::Any aCaught( ::cppu::getCaughtException() );

            // it is allowed to throw WrappedTargetException
            WrappedTargetException aException;
            if ( aCaught >>= aException )
                throw aException;

            throw WrappedTargetException(
                    THROW_WHERE "Problem writing the original content!",
                    static_cast < OWeakObject * > ( this ),
                    aCaught );
        }
        else
        {
            // the document is written directly, although it was empty it is important to notify that the writing has failed
            // TODO/LATER: let the package be able to recover in this situation
            OUString aErrTxt(THROW_WHERE "This package is unusable!");
            embed::UseBackupException aException( aErrTxt, uno::Reference< uno::XInterface >(), OUString() );
            throw WrappedTargetException( aErrTxt,
                                            static_cast < OWeakObject * > ( this ),
                                            makeAny ( aException ) );
        }
    }

    return xResult;
}

uno::Reference< XActiveDataStreamer > ZipPackage::openOriginalForOutput()
{
    // open and truncate the original file
    Content aOriginalContent(
        m_aURL, uno::Reference< XCommandEnvironment >(),
        m_xContext );
    uno::Reference< XActiveDataStreamer > xSink = new ActiveDataStreamer;

    if ( m_eMode == e_IMode_URL )
    {
        try
        {
            bool bTruncSuccess = false;

            try
            {
                Exception aDetect;
                sal_Int64 aSize = 0;
                Any aAny = aOriginalContent.setPropertyValue("Size", makeAny( aSize ) );
                if( !( aAny >>= aDetect ) )
                    bTruncSuccess = true;
            }
            catch( Exception& )
            {
            }

            if( !bTruncSuccess )
            {
                // the file is not accessible
                // just try to write an empty stream to it

                uno::Reference< XInputStream > xTempIn = new DummyInputStream; //uno::Reference< XInputStream >( xTempOut, UNO_QUERY );
                aOriginalContent.writeStream( xTempIn , true );
            }

            OpenCommandArgument2 aArg;
               aArg.Mode        = OpenMode::DOCUMENT;
               aArg.Priority    = 0; // unused
               aArg.Sink       = xSink;
               aArg.Properties = uno::Sequence< Property >( 0 ); // unused

            aOriginalContent.executeCommand("open", makeAny( aArg ) );
        }
        catch( Exception& )
        {
            // seems to be nonlocal file
            // temporary file mechanics should be used
        }
    }

    return xSink;
}

void SAL_CALL ZipPackage::commitChanges()
        throw( WrappedTargetException, RuntimeException, std::exception )
{
    // lock the component for the time of committing
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_eMode == e_IMode_XInputStream )
    {
        IOException aException;
        throw WrappedTargetException(THROW_WHERE "This package is read only!",
                static_cast < OWeakObject * > ( this ), makeAny ( aException ) );
    }
    // first the writeTempFile is called, if it returns a stream the stream should be written to the target
    // if no stream was returned, the file was written directly, nothing should be done
    uno::Reference< io::XInputStream > xTempInStream;
    try
    {
        xTempInStream = writeTempFile();
    }
    catch (const ucb::ContentCreationException& r)
    {
        throw WrappedTargetException(THROW_WHERE "Temporary file should be creatable!",
                    static_cast < OWeakObject * > ( this ), makeAny ( r ) );
    }
    if ( xTempInStream.is() )
    {
        uno::Reference< io::XSeekable > xTempSeek( xTempInStream, uno::UNO_QUERY_THROW );

        try
        {
            xTempSeek->seek( 0 );
        }
        catch( const uno::Exception& r )
        {
            throw WrappedTargetException(THROW_WHERE "Temporary file should be seekable!",
                    static_cast < OWeakObject * > ( this ), makeAny ( r ) );
        }

        try
        {
            // connect to the temporary stream
            ConnectTo( xTempInStream );
        }
        catch( const io::IOException& r )
        {
            throw WrappedTargetException(THROW_WHERE "Temporary file should be connectable!",
                    static_cast < OWeakObject * > ( this ), makeAny ( r ) );
        }

        if ( m_eMode == e_IMode_XStream )
        {
            // First truncate our output stream
            uno::Reference < XOutputStream > xOutputStream;

            // preparation for copy step
            try
            {
                xOutputStream = m_xStream->getOutputStream();
                uno::Reference < XTruncate > xTruncate ( xOutputStream, UNO_QUERY );
                if ( !xTruncate.is() )
                    throw uno::RuntimeException(THROW_WHERE );

                // after successful truncation the original file contents are already lost
                xTruncate->truncate();
            }
            catch( const uno::Exception& r )
            {
                throw WrappedTargetException(THROW_WHERE "This package is read only!",
                        static_cast < OWeakObject * > ( this ), makeAny ( r ) );
            }

            try
            {
                // then copy the contents of the tempfile to our output stream
                ::comphelper::OStorageHelper::CopyInputToOutput( xTempInStream, xOutputStream );
                xOutputStream->flush();
                uno::Reference< io::XAsyncOutputMonitor > asyncOutputMonitor(
                    xOutputStream, uno::UNO_QUERY );
                if ( asyncOutputMonitor.is() ) {
                    asyncOutputMonitor->waitForCompletion();
                }
            }
            catch( uno::Exception& )
            {
                // if anything goes wrong in this block the target file becomes corrupted
                // so an exception should be thrown as a notification about it
                // and the package must disconnect from the stream
                DisconnectFromTargetAndThrowException_Impl( xTempInStream );
            }
        }
        else if ( m_eMode == e_IMode_URL )
        {
            uno::Reference< XOutputStream > aOrigFileStream;
            bool bCanBeCorrupted = false;

            if( isLocalFile() )
            {
                // write directly in case of local file
                uno::Reference< css::ucb::XSimpleFileAccess3 > xSimpleAccess(
                    SimpleFileAccess::create( m_xContext ) );
                OSL_ENSURE( xSimpleAccess.is(), "Can't instantiate SimpleFileAccess service!\n" );
                uno::Reference< io::XTruncate > xOrigTruncate;
                if ( xSimpleAccess.is() )
                {
                    try
                    {
                        aOrigFileStream = xSimpleAccess->openFileWrite( m_aURL );
                        xOrigTruncate.set( aOrigFileStream, uno::UNO_QUERY_THROW );
                        // after successful truncation the file is already corrupted
                        xOrigTruncate->truncate();
                    }
                    catch( uno::Exception& )
                    {}
                }

                if( xOrigTruncate.is() )
                {
                    try
                    {
                        ::comphelper::OStorageHelper::CopyInputToOutput( xTempInStream, aOrigFileStream );
                        aOrigFileStream->closeOutput();
                    }
                    catch( uno::Exception& )
                    {
                        try {
                            aOrigFileStream->closeOutput();
                        } catch ( uno::Exception& ) {}

                        aOrigFileStream.clear();
                        // the original file can already be corrupted
                        bCanBeCorrupted = true;
                    }
                }
            }

            if( !aOrigFileStream.is() )
            {
                try
                {
                    uno::Reference < XPropertySet > xPropSet ( xTempInStream, UNO_QUERY );
                    OSL_ENSURE( xPropSet.is(), "This is a temporary file that must implement XPropertySet!\n" );
                    if ( !xPropSet.is() )
                        throw uno::RuntimeException(THROW_WHERE );

                    OUString sTargetFolder = m_aURL.copy ( 0, m_aURL.lastIndexOf ( static_cast < sal_Unicode > ( '/' ) ) );
                    Content aContent(
                        sTargetFolder, uno::Reference< XCommandEnvironment >(),
                        m_xContext );

                    OUString sTempURL;
                    Any aAny = xPropSet->getPropertyValue ("Uri");
                    aAny >>= sTempURL;

                    TransferInfo aInfo;
                    aInfo.NameClash = NameClash::OVERWRITE;
                    aInfo.MoveData = sal_False;
                    aInfo.SourceURL = sTempURL;
                    aInfo.NewTitle = rtl::Uri::decode ( m_aURL.copy ( 1 + m_aURL.lastIndexOf ( static_cast < sal_Unicode > ( '/' ) ) ),
                                                        rtl_UriDecodeWithCharset,
                                                        RTL_TEXTENCODING_UTF8 );
                    aAny <<= aInfo;

                    // if the file is still not corrupted, it can become after the next step
                    aContent.executeCommand ("transfer", aAny );
                }
                catch ( const css::uno::Exception& r )
                {
                    if ( bCanBeCorrupted )
                        DisconnectFromTargetAndThrowException_Impl( xTempInStream );

                    throw WrappedTargetException(
                                                THROW_WHERE "This package may be read only!",
                                                static_cast < OWeakObject * > ( this ),
                                                makeAny ( r ) );
                }
            }
        }
    }

    // after successful storing it can be set to false
    m_bMediaTypeFallbackUsed = false;
}

void ZipPackage::DisconnectFromTargetAndThrowException_Impl( const uno::Reference< io::XInputStream >& xTempStream )
{
    m_xStream.set( xTempStream, uno::UNO_QUERY );
    if ( m_xStream.is() )
        m_eMode = e_IMode_XStream;
    else
        m_eMode = e_IMode_XInputStream;

    OUString aTempURL;
    try {
        uno::Reference< beans::XPropertySet > xTempFile( xTempStream, uno::UNO_QUERY_THROW );
        uno::Any aUrl = xTempFile->getPropertyValue("Uri");
        aUrl >>= aTempURL;
        xTempFile->setPropertyValue("RemoveFile",
                                     uno::makeAny( sal_False ) );
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL( "These calls are pretty simple, they should not fail!\n" );
    }

    OUString aErrTxt(THROW_WHERE "This package is read only!");
    embed::UseBackupException aException( aErrTxt, uno::Reference< uno::XInterface >(), aTempURL );
    throw WrappedTargetException( aErrTxt,
                                    static_cast < OWeakObject * > ( this ),
                                    makeAny ( aException ) );
}

const uno::Sequence< sal_Int8 > ZipPackage::GetEncryptionKey()
{
    uno::Sequence< sal_Int8 > aResult;

    if ( m_aStorageEncryptionKeys.getLength() )
    {
        OUString aNameToFind;
        if ( m_nStartKeyGenerationID == xml::crypto::DigestID::SHA256 )
            aNameToFind = PACKAGE_ENCRYPTIONDATA_SHA256UTF8;
        else if ( m_nStartKeyGenerationID == xml::crypto::DigestID::SHA1 )
            aNameToFind = PACKAGE_ENCRYPTIONDATA_SHA1UTF8;
        else
            throw uno::RuntimeException(THROW_WHERE "No expected key is provided!" );

        for ( sal_Int32 nInd = 0; nInd < m_aStorageEncryptionKeys.getLength(); nInd++ )
            if ( m_aStorageEncryptionKeys[nInd].Name.equals( aNameToFind ) )
                m_aStorageEncryptionKeys[nInd].Value >>= aResult;

        // empty keys are not allowed here
        // so it is not important whether there is no key, or the key is empty, it is an error
        if ( !aResult.getLength() )
            throw uno::RuntimeException(THROW_WHERE "No expected key is provided!" );
    }
    else
        aResult = m_aEncryptionKey;

    return aResult;
}

sal_Bool SAL_CALL ZipPackage::hasPendingChanges()
        throw( RuntimeException, std::exception )
{
    return sal_False;
}
Sequence< ElementChange > SAL_CALL ZipPackage::getPendingChanges()
        throw( RuntimeException, std::exception )
{
    return uno::Sequence < ElementChange > ();
}

/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
uno::Reference < XInterface >SAL_CALL ZipPackage_createInstance(
    const uno::Reference< XMultiServiceFactory > & xMgr )
{
    return uno::Reference< XInterface >( *new ZipPackage( comphelper::getComponentContext(xMgr) ) );
}

OUString ZipPackage::static_getImplementationName()
{
    return OUString("com.sun.star.packages.comp.ZipPackage");
}

Sequence< OUString > ZipPackage::static_getSupportedServiceNames()
{
    uno::Sequence<OUString> aNames { "com.sun.star.packages.Package" };
    return aNames;
}

OUString ZipPackage::getImplementationName()
    throw ( RuntimeException, std::exception )
{
    return static_getImplementationName();
}

Sequence< OUString > ZipPackage::getSupportedServiceNames()
    throw ( RuntimeException, std::exception )
{
    return static_getSupportedServiceNames();
}

sal_Bool SAL_CALL ZipPackage::supportsService( OUString const & rServiceName )
    throw ( RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Reference < XSingleServiceFactory > ZipPackage::createServiceFactory( uno::Reference < XMultiServiceFactory > const & rServiceFactory )
{
    return cppu::createSingleFactory ( rServiceFactory,
                                           static_getImplementationName(),
                                           ZipPackage_createInstance,
                                           static_getSupportedServiceNames() );
}

namespace { struct lcl_ImplId : public rtl::Static< ::cppu::OImplementationId, lcl_ImplId > {}; }

Sequence< sal_Int8 > ZipPackage::getUnoTunnelImplementationId()
    throw ( RuntimeException )
{
    ::cppu::OImplementationId &rId = lcl_ImplId::get();
    return rId.getImplementationId();
}

sal_Int64 SAL_CALL ZipPackage::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw( RuntimeException, std::exception )
{
    if ( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast < sal_Int64 > ( this );
    return 0;
}

uno::Reference< XPropertySetInfo > SAL_CALL ZipPackage::getPropertySetInfo()
        throw( RuntimeException, std::exception )
{
    return uno::Reference < XPropertySetInfo > ();
}

void SAL_CALL ZipPackage::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception )
{
    if ( m_nFormat != embed::StorageFormats::PACKAGE )
        throw UnknownPropertyException(THROW_WHERE );

    if (aPropertyName == HAS_ENCRYPTED_ENTRIES_PROPERTY
      ||aPropertyName == HAS_NONENCRYPTED_ENTRIES_PROPERTY
      ||aPropertyName == IS_INCONSISTENT_PROPERTY
      ||aPropertyName == MEDIATYPE_FALLBACK_USED_PROPERTY)
        throw PropertyVetoException(THROW_WHERE );
    else if ( aPropertyName == ENCRYPTION_KEY_PROPERTY )
    {
        if ( !( aValue >>= m_aEncryptionKey ) )
            throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 2 );

        m_aStorageEncryptionKeys.realloc( 0 );
    }
    else if ( aPropertyName == STORAGE_ENCRYPTION_KEYS_PROPERTY )
    {
        // this property is only necessary to support raw passwords in storage API;
        // because of this support the storage has to operate with more than one key dependent on storage generation algorithm;
        // when this support is removed, the storage will get only one key from outside
        // TODO/LATER: Get rid of this property as well as of support of raw passwords in storages
        uno::Sequence< beans::NamedValue > aKeys;
        if ( !( aValue >>= aKeys ) || ( aKeys.getLength() && aKeys.getLength() < 2 ) )
            throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 2 );

        if ( aKeys.getLength() )
        {
            bool bHasSHA256 = false;
            bool bHasSHA1 = false;
            for ( sal_Int32 nInd = 0; nInd < aKeys.getLength(); nInd++ )
            {
                if ( aKeys[nInd].Name == PACKAGE_ENCRYPTIONDATA_SHA256UTF8 )
                    bHasSHA256 = true;
                if ( aKeys[nInd].Name == PACKAGE_ENCRYPTIONDATA_SHA1UTF8 )
                    bHasSHA1 = true;
            }

            if ( !bHasSHA256 || !bHasSHA1 )
                throw IllegalArgumentException(THROW_WHERE "Expected keys are not provided!", uno::Reference< uno::XInterface >(), 2 );
        }

        m_aStorageEncryptionKeys = aKeys;
        m_aEncryptionKey.realloc( 0 );
    }
    else if ( aPropertyName == ENCRYPTION_ALGORITHMS_PROPERTY )
    {
        uno::Sequence< beans::NamedValue > aAlgorithms;
        if ( m_pZipFile || !( aValue >>= aAlgorithms ) || aAlgorithms.getLength() == 0 )
        {
            // the algorithms can not be changed if the file has a persistence based on the algorithms ( m_pZipFile )
            throw IllegalArgumentException(THROW_WHERE "unexpected algorithms list is provided.", uno::Reference< uno::XInterface >(), 2 );
        }

        for ( sal_Int32 nInd = 0; nInd < aAlgorithms.getLength(); nInd++ )
        {
            if ( aAlgorithms[nInd].Name == "StartKeyGenerationAlgorithm" )
            {
                sal_Int32 nID = 0;
                if ( !( aAlgorithms[nInd].Value >>= nID )
                  || ( nID != xml::crypto::DigestID::SHA256 && nID != xml::crypto::DigestID::SHA1 ) )
                    throw IllegalArgumentException(THROW_WHERE "Unexpected start key generation algorithm is provided!", uno::Reference< uno::XInterface >(), 2 );

                m_nStartKeyGenerationID = nID;
            }
            else if ( aAlgorithms[nInd].Name == "EncryptionAlgorithm" )
            {
                sal_Int32 nID = 0;
                if ( !( aAlgorithms[nInd].Value >>= nID )
                  || ( nID != xml::crypto::CipherID::AES_CBC_W3C_PADDING && nID != xml::crypto::CipherID::BLOWFISH_CFB_8 ) )
                    throw IllegalArgumentException(THROW_WHERE "Unexpected start key generation algorithm is provided!", uno::Reference< uno::XInterface >(), 2 );

                m_nCommonEncryptionID = nID;
            }
            else if ( aAlgorithms[nInd].Name == "ChecksumAlgorithm" )
            {
                sal_Int32 nID = 0;
                if ( !( aAlgorithms[nInd].Value >>= nID )
                  || ( nID != xml::crypto::DigestID::SHA1_1K && nID != xml::crypto::DigestID::SHA256_1K ) )
                    throw IllegalArgumentException(THROW_WHERE "Unexpected start key generation algorithm is provided!", uno::Reference< uno::XInterface >(), 2 );

                m_nChecksumDigestID = nID;
            }
            else
            {
                OSL_ENSURE( false, "Unexpected encryption algorithm is provided!" );
                throw IllegalArgumentException(THROW_WHERE "unexpected algorithms list is provided.", uno::Reference< uno::XInterface >(), 2 );
            }
        }
    }
    else
        throw UnknownPropertyException(THROW_WHERE );
}

Any SAL_CALL ZipPackage::getPropertyValue( const OUString& PropertyName )
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
    // TODO/LATER: Activate the check when zip-ucp is ready
    // if ( m_nFormat != embed::StorageFormats::PACKAGE )
    //  throw UnknownPropertyException(THROW_WHERE );

    Any aAny;
    if ( PropertyName == ENCRYPTION_KEY_PROPERTY )
    {
        aAny <<= m_aEncryptionKey;
        return aAny;
    }
    else if ( PropertyName == ENCRYPTION_ALGORITHMS_PROPERTY )
    {
        ::comphelper::SequenceAsHashMap aAlgorithms;
        aAlgorithms["StartKeyGenerationAlgorithm"] <<= m_nStartKeyGenerationID;
        aAlgorithms["EncryptionAlgorithm"] <<= m_nCommonEncryptionID;
        aAlgorithms["ChecksumAlgorithm"] <<= m_nChecksumDigestID;
        aAny <<= aAlgorithms.getAsConstNamedValueList();
        return aAny;
    }
    if ( PropertyName == STORAGE_ENCRYPTION_KEYS_PROPERTY )
    {
        aAny <<= m_aStorageEncryptionKeys;
        return aAny;
    }
    else if ( PropertyName == HAS_ENCRYPTED_ENTRIES_PROPERTY )
    {
        aAny <<= m_bHasEncryptedEntries;
        return aAny;
    }
    else if ( PropertyName == HAS_NONENCRYPTED_ENTRIES_PROPERTY )
    {
        aAny <<= m_bHasNonEncryptedEntries;
        return aAny;
    }
    else if ( PropertyName == IS_INCONSISTENT_PROPERTY )
    {
        aAny <<= m_bInconsistent;
        return aAny;
    }
    else if ( PropertyName == MEDIATYPE_FALLBACK_USED_PROPERTY )
    {
        aAny <<= m_bMediaTypeFallbackUsed;
        return aAny;
    }
    throw UnknownPropertyException(THROW_WHERE );
}
void SAL_CALL ZipPackage::addPropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
}
void SAL_CALL ZipPackage::removePropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*aListener*/ )
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
}
void SAL_CALL ZipPackage::addVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
}
void SAL_CALL ZipPackage::removeVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
