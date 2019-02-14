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
#include "ZipPackageSink.hxx"
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
#include <com/sun/star/ucb/ContentCreationException.hpp>
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
#include "ContentInfo.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uri.hxx>
#include <rtl/random.h>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <com/sun/star/io/XAsyncOutputMonitor.hpp>

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

    virtual uno::Reference< XStream > SAL_CALL getStream() override
            { return mStream; }

    virtual void SAL_CALL setStream( const uno::Reference< XStream >& stream ) override
            { mStream = stream; }
};

class DummyInputStream : public ::cppu::WeakImplHelper< XInputStream >
{
    virtual sal_Int32 SAL_CALL readBytes( uno::Sequence< sal_Int8 >&, sal_Int32 ) override
        { return 0; }

    virtual sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< sal_Int8 >&, sal_Int32 ) override
        { return 0; }

    virtual void SAL_CALL skipBytes( sal_Int32 ) override
        {}

    virtual sal_Int32 SAL_CALL available() override
        { return 0; }

    virtual void SAL_CALL closeInput() override
        {}
};

ZipPackage::ZipPackage ( const uno::Reference < XComponentContext > &xContext )
: m_aMutexHolder( new comphelper::RefCountedMutex )
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
{
    m_xRootFolder = new ZipPackageFolder( m_xContext, m_nFormat, m_bAllowRemoveOnInsert );
}

ZipPackage::~ZipPackage()
{
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
                        const OUString sKeyInfo ("KeyInfo");

                        uno::Sequence < uno::Sequence < PropertyValue > > aManifestSequence = xReader->readManifestSequence ( xSink->getInputStream() );
                        sal_Int32 nLength = aManifestSequence.getLength();
                        const uno::Sequence < PropertyValue > *pSequence = aManifestSequence.getConstArray();
                        ZipPackageStream *pStream = nullptr;
                        ZipPackageFolder *pFolder = nullptr;
                        const Any *pKeyInfo = nullptr;

                        for ( sal_Int32 i = 0; i < nLength ; i++, pSequence++ )
                        {
                            OUString sPath, sMediaType, sVersion;
                            const PropertyValue *pValue = pSequence->getConstArray();
                            const Any *pSalt = nullptr, *pVector = nullptr, *pCount = nullptr, *pSize = nullptr, *pDigest = nullptr, *pDigestAlg = nullptr, *pEncryptionAlg = nullptr, *pStartKeyAlg = nullptr, *pDerivedKeySize = nullptr;
                            for ( sal_Int32 j = 0, nNum = pSequence->getLength(); j < nNum; j++ )
                            {
                                if ( pValue[j].Name == sPropFullPath )
                                    pValue[j].Value >>= sPath;
                                else if ( pValue[j].Name == sPropVersion )
                                    pValue[j].Value >>= sVersion;
                                else if ( pValue[j].Name == sPropMediaType )
                                    pValue[j].Value >>= sMediaType;
                                else if ( pValue[j].Name == sPropSalt )
                                    pSalt = &( pValue[j].Value );
                                else if ( pValue[j].Name == sPropInitialisationVector )
                                    pVector = &( pValue[j].Value );
                                else if ( pValue[j].Name == sPropIterationCount )
                                    pCount = &( pValue[j].Value );
                                else if ( pValue[j].Name == sPropSize )
                                    pSize = &( pValue[j].Value );
                                else if ( pValue[j].Name == sPropDigest )
                                    pDigest = &( pValue[j].Value );
                                else if ( pValue[j].Name == sPropDigestAlgorithm )
                                    pDigestAlg = &( pValue[j].Value );
                                else if ( pValue[j].Name == sPropEncryptionAlgorithm )
                                    pEncryptionAlg = &( pValue[j].Value );
                                else if ( pValue[j].Name == sPropStartKeyAlgorithm )
                                    pStartKeyAlg = &( pValue[j].Value );
                                else if ( pValue[j].Name == sPropDerivedKeySize )
                                    pDerivedKeySize = &( pValue[j].Value );
                                else if ( pValue[j].Name == sKeyInfo )
                                    pKeyInfo = &( pValue[j].Value );
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

                                    if ( pKeyInfo && pVector && pSize && pDigest && pDigestAlg && pEncryptionAlg )
                                    {
                                        uno::Sequence < sal_Int8 > aSequence;
                                        sal_Int64 nSize = 0;
                                        sal_Int32 nDigestAlg = 0, nEncryptionAlg = 0;

                                        pStream->SetToBeEncrypted ( true );

                                        *pVector >>= aSequence;
                                        pStream->setInitialisationVector ( aSequence );

                                        *pSize >>= nSize;
                                        pStream->setSize ( nSize );

                                        *pDigest >>= aSequence;
                                        pStream->setDigest ( aSequence );

                                        *pDigestAlg >>= nDigestAlg;
                                        pStream->SetImportedChecksumAlgorithm( nDigestAlg );

                                        *pEncryptionAlg >>= nEncryptionAlg;
                                        pStream->SetImportedEncryptionAlgorithm( nEncryptionAlg );

                                        *pKeyInfo >>= m_aGpgProps;

                                        pStream->SetToBeCompressed ( true );
                                        pStream->SetToBeEncrypted ( true );
                                        pStream->SetIsEncrypted ( true );
                                        pStream->setIterationCount(0);

                                        // clamp to default SHA256 start key magic value,
                                        // c.f. ZipPackageStream::GetEncryptionKey()
                                        // trying to get key value from properties
                                        const sal_Int32 nStartKeyAlg = xml::crypto::DigestID::SHA256;
                                        pStream->SetImportedStartKeyAlgorithm( nStartKeyAlg );

                                        if ( !m_bHasEncryptedEntries && pStream->getName() == "content.xml" )
                                        {
                                            m_bHasEncryptedEntries = true;
                                            m_nChecksumDigestID = nDigestAlg;
                                            m_nCommonEncryptionID = nEncryptionAlg;
                                            m_nStartKeyGenerationID = nStartKeyAlg;
                                        }
                                    }
                                    else if ( pSalt && pVector && pCount && pSize && pDigest && pDigestAlg && pEncryptionAlg )
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
                THROW_WHERE "Could not parse manifest.xml" );

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
                    m_xRootFolder->SetMediaType( aPackageMediatype );
                    m_bMediaTypeFallbackUsed = true;
                }
            }
            else if ( !m_bForceRecovery )
            {
                // the mimetype stream should contain the information from manifest.xml
                if ( m_xRootFolder->GetMediaType() != aPackageMediatype )
                    throw ZipIOException(
                        THROW_WHERE
                        "mimetype conflicts with manifest.xml, \""
                        + m_xRootFolder->GetMediaType() + "\" vs. \""
                        + aPackageMediatype + "\"" );
            }

            m_xRootFolder->removeByName( sMimetype );
        }

        m_bInconsistent = m_xRootFolder->LookForUnexpectedODF12Streams( OUString() );

        bool bODF12AndNewer = ( m_xRootFolder->GetVersion().compareTo( ODFVER_012_TEXT ) >= 0 );
        if ( !m_bForceRecovery && bODF12AndNewer )
        {
            if ( m_bInconsistent )
            {
                // this is an ODF1.2 document that contains streams not referred in the manifest.xml;
                // in case of ODF1.2 documents without version in manifest.xml the property IsInconsistent
                // should be checked later
                throw ZipIOException(
                    THROW_WHERE "there are streams not referred in manifest.xml" );
            }
            // all the streams should be encrypted with the same StartKey in ODF1.2
            // TODO/LATER: in future the exception should be thrown
            // throw ZipIOException( THROW_WHERE "More than one Start Key Generation algorithm is specified!" );
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

                    // set the implicit types first
                    for ( nInd = 0; nInd < aContentTypeInfo[0].getLength(); nInd++ )
                        m_xRootFolder->setChildStreamsTypeByExtension( aContentTypeInfo[0][nInd] );

                    // now set the explicit types
                    for ( nInd = 0; nInd < aContentTypeInfo[1].getLength(); nInd++ )
                    {
                        OUString aPath;
                        if ( aContentTypeInfo[1][nInd].First.toChar() == '/' )
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
    std::unique_ptr<ZipEnumeration> xEnum = m_pZipFile->entries();
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nStreamIndex;
    FolderHash::iterator aIter;

    while (xEnum->hasMoreElements())
    {
        nOldIndex = 0;
        ZipPackageFolder* pCurrent = m_xRootFolder.get();
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

        if ( pCurrent == m_xRootFolder.get() )
        {
            sal_Int32 nIndex;
            while ( ( nIndex = rName.indexOf( '/', nOldIndex ) ) != -1 )
            {
                sTemp = rName.copy ( nOldIndex, nIndex - nOldIndex );
                if ( nIndex == nOldIndex )
                    break;
                if ( !pCurrent->hasByName( sTemp ) )
                {
                    ZipPackageFolder* pPkgFolder = new ZipPackageFolder(m_xContext, m_nFormat, m_bAllowRemoveOnInsert);
                    pPkgFolder->setName( sTemp );
                    pPkgFolder->doSetParent( pCurrent );
                    pCurrent = pPkgFolder;
                }
                else
                {
                    ZipContentInfo& rInfo = pCurrent->doGetByName(sTemp);
                    if (!rInfo.bFolder)
                        throw css::packages::zip::ZipIOException("Bad Zip File, stream as folder");
                    pCurrent = rInfo.pFolder;
                }
                nOldIndex = nIndex+1;
            }
            if ( nStreamIndex != -1 && !sDirName.isEmpty() )
                m_aRecent [ sDirName ] = pCurrent;
        }
        if ( rName.getLength() -1 != nStreamIndex )
        {
            nStreamIndex++;
            sTemp = rName.copy( nStreamIndex );

            if (!pCurrent->hasByName(sTemp))
            {
                ZipPackageStream *pPkgStream = new ZipPackageStream(*this, m_xContext, m_nFormat, m_bAllowRemoveOnInsert);
                pPkgStream->SetPackageMember(true);
                pPkgStream->setZipEntryOnLoading(rEntry);
                pPkgStream->setName(sTemp);
                pPkgStream->doSetParent(pCurrent);
            }
        }
    }

    if ( m_nFormat == embed::StorageFormats::PACKAGE )
        parseManifest();
    else if ( m_nFormat == embed::StorageFormats::OFOPXML )
        parseContentType();
}

void SAL_CALL ZipPackage::initialize( const uno::Sequence< Any >& aArguments )
{
    beans::NamedValue aNamedValue;

    if ( aArguments.getLength() )
    {
        bool bHaveZipFile = true;

        for( int ind = 0; ind < aArguments.getLength(); ind++ )
        {
            OUString aParamUrl;
            if ( aArguments[ind] >>= aParamUrl )
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
                                m_xRootFolder->setPackageFormat_Impl( m_nFormat );
                                break;
                            }
                            else if ( aCommand == "ofopxml" )
                            {
                                m_nFormat = embed::StorageFormats::OFOPXML;
                                m_xRootFolder->setPackageFormat_Impl( m_nFormat );
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
                    if( !bHasSizeProperty || aSize )
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
            else if ( aArguments[ind] >>= m_xStream )
            {
                // a writable stream can implement both XStream & XInputStream
                m_eMode = e_IMode_XStream;
                m_xContentStream = m_xStream->getInputStream();
            }
            else if ( aArguments[ind] >>= m_xContentStream )
            {
                m_eMode = e_IMode_XInputStream;
            }
            else if ( aArguments[ind] >>= aNamedValue )
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

                    m_xRootFolder->setPackageFormat_Impl( m_nFormat );
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

                    m_xRootFolder->setPackageFormat_Impl( m_nFormat );
                }
                else if ( aNamedValue.Name == "AllowRemoveOnInsert" )
                {
                    aNamedValue.Value >>= m_bAllowRemoveOnInsert;
                    m_xRootFolder->setRemoveOnInsertMode_Impl( m_bAllowRemoveOnInsert );
                }
                else if (aNamedValue.Name == "NoFileSync")
                    aNamedValue.Value >>= m_bDisableFileSync;

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
                m_xContentSeek.set( m_xContentStream, UNO_QUERY_THROW );
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
                m_pZipFile = std::make_unique<ZipFile>(m_aMutexHolder, m_xContentStream, m_xContext, true, m_bForceRecovery);
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
                m_pZipFile.reset();
                throw;
            }

            if ( bBadZipFile )
            {
                // clean up the memory, and tell the UCB about the error
                m_pZipFile.reset();

                throw css::packages::zip::ZipIOException (
                    THROW_WHERE "Bad Zip File, " + message,
                    static_cast < ::cppu::OWeakObject * > ( this ) );
            }
        }
    }
}

Any SAL_CALL ZipPackage::getByHierarchicalName( const OUString& aName )
{
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nStreamIndex;
    FolderHash::iterator aIter;

    sal_Int32 nIndex = aName.getLength();

    if (aName == "/")
        // root directory.
        return makeAny ( uno::Reference < XUnoTunnel > ( m_xRootFolder.get() ) );

    nStreamIndex = aName.lastIndexOf ( '/' );
    bool bFolder = nStreamIndex == nIndex-1; // last character is '/'.

    if ( nStreamIndex != -1 )
    {
        // The name contains '/'.
        sDirName = aName.copy ( 0, nStreamIndex );
        aIter = m_aRecent.find ( sDirName );
        if ( aIter != m_aRecent.end() )
        {
            // There is a cached entry for this name.

            ZipPackageFolder* pFolder = aIter->second;

            if ( bFolder )
            {
                // Determine the directory name.
                sal_Int32 nDirIndex = aName.lastIndexOf ( '/', nStreamIndex );
                sTemp = aName.copy ( nDirIndex == -1 ? 0 : nDirIndex+1, nStreamIndex-nDirIndex-1 );

                if (pFolder && sTemp == pFolder->getName())
                    return makeAny(uno::Reference<XUnoTunnel>(pFolder));
            }
            else
            {
                // Determine the file name.
                sTemp = aName.copy ( nStreamIndex + 1 );

                if (pFolder && pFolder->hasByName(sTemp))
                    return pFolder->getByName(sTemp);
            }

            m_aRecent.erase( aIter );
        }
    }
    else if ( m_xRootFolder->hasByName ( aName ) )
        // top-level element.
        return m_xRootFolder->getByName ( aName );

    // Not in the cache. Search normally.

    nOldIndex = 0;
    ZipPackageFolder * pCurrent = m_xRootFolder.get();
    ZipPackageFolder * pPrevious = nullptr;

    // Find the right directory for the given path.

    while ( ( nIndex = aName.indexOf( '/', nOldIndex )) != -1 )
    {
        sTemp = aName.copy ( nOldIndex, nIndex - nOldIndex );
        if ( nIndex == nOldIndex )
            break;
        if ( !pCurrent->hasByName( sTemp ) )
            throw NoSuchElementException(THROW_WHERE );

        pPrevious = pCurrent;
        ZipContentInfo& rInfo = pCurrent->doGetByName(sTemp);
        if (!rInfo.bFolder)
            throw css::packages::zip::ZipIOException("Bad Zip File, stream as folder");
        pCurrent = rInfo.pFolder;
        nOldIndex = nIndex+1;
    }

    if ( bFolder )
    {
        if ( nStreamIndex != -1 )
            m_aRecent[sDirName] = pPrevious; // cache it.
        return makeAny ( uno::Reference < XUnoTunnel > ( pCurrent ) );
    }

    sTemp = aName.copy( nOldIndex );

    if ( pCurrent->hasByName ( sTemp ) )
    {
        if ( nStreamIndex != -1 )
            m_aRecent[sDirName] = pCurrent; // cache it.
        return pCurrent->getByName( sTemp );
    }

    throw NoSuchElementException(THROW_WHERE);
}

sal_Bool SAL_CALL ZipPackage::hasByHierarchicalName( const OUString& aName )
{
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nStreamIndex;
    FolderHash::iterator aIter;

    sal_Int32 nIndex = aName.getLength();

    if (aName == "/")
        // root directory
        return true;

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
                        return true;
                    else
                        m_aRecent.erase ( aIter );
                }
                else
                {
                    sTemp = aName.copy ( nStreamIndex + 1 );
                    if ( ( *aIter ).second->hasByName( sTemp ) )
                        return true;
                    else
                        m_aRecent.erase( aIter );
                }
            }
        }
        else
        {
            if ( m_xRootFolder->hasByName ( aName ) )
                return true;
        }
        ZipPackageFolder * pCurrent = m_xRootFolder.get();
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
                ZipContentInfo& rInfo = pCurrent->doGetByName(sTemp);
                if (!rInfo.bFolder)
                    throw css::packages::zip::ZipIOException("Bad Zip File, stream as folder");
                pCurrent = rInfo.pFolder;
            }
            else
                return false;
            nOldIndex = nIndex+1;
        }
        if ( bFolder )
        {
            m_aRecent[sDirName] = pPrevious;
            return true;
        }
        else
        {
            sTemp = aName.copy( nOldIndex );

            if ( pCurrent->hasByName( sTemp ) )
            {
                m_aRecent[sDirName] = pCurrent;
                return true;
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
        throw lang::WrappedTargetRuntimeException("ZipPackage::hasByHierarchicalName", nullptr, e);
    }
    return false;
}

uno::Reference< XInterface > SAL_CALL ZipPackage::createInstance()
{
    uno::Reference < XInterface > xRef = *( new ZipPackageStream( *this, m_xContext, m_nFormat, m_bAllowRemoveOnInsert ) );
    return xRef;
}

uno::Reference< XInterface > SAL_CALL ZipPackage::createInstanceWithArguments( const uno::Sequence< Any >& aArguments )
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
    sal_Int32 nBufferLength = m_xRootFolder->GetMediaType().getLength();
    OString sMediaType = OUStringToOString( m_xRootFolder->GetMediaType(), RTL_TEXTENCODING_ASCII_US );
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
    catch ( const css::io::IOException & )
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw WrappedTargetException(
                THROW_WHERE "Error adding mimetype to the ZipOutputStream!",
                static_cast < OWeakObject * > ( this ),
                anyEx );
    }
}

void ZipPackage::WriteManifest( ZipOutputStream& aZipOut, const vector< uno::Sequence < PropertyValue > >& aManList )
{
    // Write the manifest
    uno::Reference < XManifestWriter > xWriter = ManifestWriter::create( m_xContext );
    ZipEntry * pEntry = new ZipEntry;
    ZipPackageBuffer *pBuffer = new ZipPackageBuffer;
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
    ZipOutputEntry aZipEntry(aZipOut.getStream(), m_xContext, *pEntry, nullptr, /*bEncrypt*/false);
    aZipEntry.write(pBuffer->getSequence());
    aZipEntry.closeEntry();
    aZipOut.rawCloseEntry();
}

void ZipPackage::WriteContentTypes( ZipOutputStream& aZipOut, const vector< uno::Sequence < PropertyValue > >& aManList )
{
    ZipEntry* pEntry = new ZipEntry;
    ZipPackageBuffer *pBuffer = new ZipPackageBuffer;
    uno::Reference< io::XOutputStream > xConTypeOutStream( *pBuffer, UNO_QUERY );

    pEntry->sPath = "[Content_Types].xml";
    pEntry->nMethod = DEFLATED;
    pEntry->nCrc = -1;
    pEntry->nSize = pEntry->nCompressedSize = -1;
    pEntry->nTime = ZipOutputStream::getCurrentDosTime();

    // Convert vector into a uno::Sequence
    // TODO/LATER: use Default entries in future
    uno::Sequence< beans::StringPair > aDefaultsSequence(4);
    // Add at least the standard default entries.
    aDefaultsSequence[0].First = "xml";
    aDefaultsSequence[0].Second= "application/xml";
    aDefaultsSequence[1].First = "rels";
    aDefaultsSequence[1].Second= "application/vnd.openxmlformats-package.relationships+xml";
    aDefaultsSequence[2].First = "png";
    aDefaultsSequence[2].Second= "image/png";
    aDefaultsSequence[3].First = "jpeg";
    aDefaultsSequence[3].Second= "image/jpeg";

    uno::Sequence< beans::StringPair > aOverridesSequence(aManList.size());
    sal_Int32 nOverSeqLength = 0;
    for (const auto& rMan : aManList)
    {
        OUString aPath;
        OUString aType;
        OSL_ENSURE( rMan[PKG_MNFST_MEDIATYPE].Name == "MediaType" && rMan[PKG_MNFST_FULLPATH].Name == "FullPath",
                    "The mediatype sequence format is wrong!" );
        rMan[PKG_MNFST_MEDIATYPE].Value >>= aType;
        if ( !aType.isEmpty() )
        {
            // only nonempty type makes sense here
            rMan[PKG_MNFST_FULLPATH].Value >>= aPath;
            //FIXME: For now we have no way of differentiating defaults from others.
            aOverridesSequence[nOverSeqLength].First = "/" + aPath;
            aOverridesSequence[nOverSeqLength].Second = aType;
            ++nOverSeqLength;
        }
    }
    aOverridesSequence.realloc(nOverSeqLength);

    ::comphelper::OFOPXMLHelper::WriteContentSequence(
            xConTypeOutStream, aDefaultsSequence, aOverridesSequence, m_xContext );

    sal_Int32 nBufferLength = static_cast < sal_Int32 > ( pBuffer->getPosition() );
    pBuffer->realloc( nBufferLength );

    // there is no encryption in this format currently
    ZipOutputStream::setEntry(pEntry);
    aZipOut.writeLOC(pEntry);
    ZipOutputEntry aZipEntry(aZipOut.getStream(), m_xContext, *pEntry, nullptr, /*bEncrypt*/false);
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
        m_pZipFile = std::make_unique<ZipFile>(m_aMutexHolder, m_xContentStream, m_xContext, false);
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
            m_aRandomPool = rtl_random_createPool ();
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
    // write directly to it otherwise create a temporary file to write to.
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
            static const OUString sMeta ("META-INF");

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

            static const OUString aContentTypes("[Content_Types].xml");

            if ( m_xRootFolder->hasByName( aContentTypes ) )
                m_xRootFolder->removeByName( aContentTypes );
        }

        // Create a vector to store data for the manifest.xml file
        vector < uno::Sequence < PropertyValue > > aManList;

        static const OUString sMediaType("MediaType");
        static const OUString sVersion("Version");
        static const OUString sFullPath("FullPath");
        const bool bIsGpgEncrypt = m_aGpgProps.hasElements();

        if ( m_nFormat == embed::StorageFormats::PACKAGE )
        {
            uno::Sequence < PropertyValue > aPropSeq(
                bIsGpgEncrypt ? PKG_SIZE_NOENCR_MNFST+1 : PKG_SIZE_NOENCR_MNFST );
            aPropSeq [PKG_MNFST_MEDIATYPE].Name = sMediaType;
            aPropSeq [PKG_MNFST_MEDIATYPE].Value <<= m_xRootFolder->GetMediaType();
            aPropSeq [PKG_MNFST_VERSION].Name = sVersion;
            aPropSeq [PKG_MNFST_VERSION].Value <<= m_xRootFolder->GetVersion();
            aPropSeq [PKG_MNFST_FULLPATH].Name = sFullPath;
            aPropSeq [PKG_MNFST_FULLPATH].Value <<= OUString("/");

            if( bIsGpgEncrypt )
            {
                aPropSeq[PKG_SIZE_NOENCR_MNFST].Name = "KeyInfo";
                aPropSeq[PKG_SIZE_NOENCR_MNFST].Value <<= m_aGpgProps;
            }
            aManList.push_back( aPropSeq );
        }

        {
            // This will be used to generate random salt and initialisation vectors
            // for encrypted streams
            RandomPool aRandomPool;

            sal_Int32 const nPBKDF2IterationCount = 100000;

            // call saveContents ( it will recursively save sub-directories
            m_xRootFolder->saveContents("", aManList, aZipOut, GetEncryptionKey(), bIsGpgEncrypt ? 0 : nPBKDF2IterationCount, aRandomPool.get());
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
            if (asyncOutputMonitor.is() && !m_bDisableFileSync)
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
                Any aAny = aOriginalContent.setPropertyValue("Size", makeAny( sal_Int64(0) ) );
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
    catch (const ucb::ContentCreationException&)
    {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw WrappedTargetException(THROW_WHERE "Temporary file should be creatable!",
                    static_cast < OWeakObject * > ( this ), anyEx );
    }
    if ( xTempInStream.is() )
    {
        uno::Reference< io::XSeekable > xTempSeek( xTempInStream, uno::UNO_QUERY_THROW );

        try
        {
            xTempSeek->seek( 0 );
        }
        catch( const uno::Exception& )
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw WrappedTargetException(THROW_WHERE "Temporary file should be seekable!",
                    static_cast < OWeakObject * > ( this ), anyEx );
        }

        try
        {
            // connect to the temporary stream
            ConnectTo( xTempInStream );
        }
        catch( const io::IOException& )
        {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw WrappedTargetException(THROW_WHERE "Temporary file should be connectable!",
                    static_cast < OWeakObject * > ( this ), anyEx );
        }

        if ( m_eMode == e_IMode_XStream )
        {
            // First truncate our output stream
            uno::Reference < XOutputStream > xOutputStream;

            // preparation for copy step
            try
            {
                xOutputStream = m_xStream->getOutputStream();

                // Make sure we avoid a situation where the current position is
                // not zero, but the underlying file is truncated in the
                // meantime.
                uno::Reference<io::XSeekable> xSeekable(xOutputStream, uno::UNO_QUERY);
                if (xSeekable.is())
                    xSeekable->seek(0);

                uno::Reference < XTruncate > xTruncate ( xOutputStream, UNO_QUERY_THROW );

                // after successful truncation the original file contents are already lost
                xTruncate->truncate();
            }
            catch( const uno::Exception& )
            {
                css::uno::Any anyEx = cppu::getCaughtException();
                throw WrappedTargetException(THROW_WHERE "This package is read only!",
                        static_cast < OWeakObject * > ( this ), anyEx );
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
                OSL_ENSURE( xSimpleAccess.is(), "Can't instantiate SimpleFileAccess service!" );
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
                    uno::Reference < XPropertySet > xPropSet ( xTempInStream, UNO_QUERY_THROW );

                    OUString sTargetFolder = m_aURL.copy ( 0, m_aURL.lastIndexOf ( u'/' ) );
                    Content aContent(
                        sTargetFolder, uno::Reference< XCommandEnvironment >(),
                        m_xContext );

                    OUString sTempURL;
                    Any aAny = xPropSet->getPropertyValue ("Uri");
                    aAny >>= sTempURL;

                    TransferInfo aInfo;
                    aInfo.NameClash = NameClash::OVERWRITE;
                    aInfo.MoveData = false;
                    aInfo.SourceURL = sTempURL;
                    aInfo.NewTitle = rtl::Uri::decode ( m_aURL.copy ( 1 + m_aURL.lastIndexOf ( u'/' ) ),
                                                        rtl_UriDecodeWithCharset,
                                                        RTL_TEXTENCODING_UTF8 );
                    // if the file is still not corrupted, it can become after the next step
                    aContent.executeCommand ("transfer", Any(aInfo) );
                }
                catch ( const css::uno::Exception& )
                {
                    if ( bCanBeCorrupted )
                        DisconnectFromTargetAndThrowException_Impl( xTempInStream );

                    css::uno::Any anyEx = cppu::getCaughtException();
                    throw WrappedTargetException(
                                                THROW_WHERE "This package may be read only!",
                                                static_cast < OWeakObject * > ( this ),
                                                anyEx );
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
                                     uno::makeAny( false ) );
    }
    catch ( uno::Exception& )
    {
        OSL_FAIL( "These calls are pretty simple, they should not fail!" );
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
            aNameToFind = PACKAGE_ENCRYPTIONDATA_SHA1CORRECT;
        else
            throw uno::RuntimeException(THROW_WHERE "No expected key is provided!" );

        for ( sal_Int32 nInd = 0; nInd < m_aStorageEncryptionKeys.getLength(); nInd++ )
            if ( m_aStorageEncryptionKeys[nInd].Name == aNameToFind )
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
{
    return false;
}
Sequence< ElementChange > SAL_CALL ZipPackage::getPendingChanges()
{
    return uno::Sequence < ElementChange > ();
}

/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
static uno::Reference < XInterface > ZipPackage_createInstance(
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
{
    return static_getImplementationName();
}

Sequence< OUString > ZipPackage::getSupportedServiceNames()
{
    return static_getSupportedServiceNames();
}

sal_Bool SAL_CALL ZipPackage::supportsService( OUString const & rServiceName )
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

Sequence< sal_Int8 > ZipPackage::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId implId;

    return implId.getImplementationId();
}

sal_Int64 SAL_CALL ZipPackage::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
{
    if ( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast < sal_Int64 > ( this );
    return 0;
}

uno::Reference< XPropertySetInfo > SAL_CALL ZipPackage::getPropertySetInfo()
{
    return uno::Reference < XPropertySetInfo > ();
}

void SAL_CALL ZipPackage::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
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
        uno::Sequence< beans::NamedValue > aKeys;
        if ( !( aValue >>= aKeys ) || ( aKeys.getLength() && aKeys.getLength() < 1 ) )
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

            if ( !bHasSHA256 && !bHasSHA1 )
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
    else if ( aPropertyName == ENCRYPTION_GPG_PROPERTIES )
    {
        uno::Sequence< uno::Sequence< beans::NamedValue > > aGpgProps;
        if ( !( aValue >>= aGpgProps ) || aGpgProps.getLength() == 0 )
        {
            throw IllegalArgumentException(THROW_WHERE "unexpected Gpg properties are provided.", uno::Reference< uno::XInterface >(), 2 );
        }

        m_aGpgProps = aGpgProps;

        // override algorithm defaults (which are some legacy ODF
        // defaults) with reasonable values
        m_nStartKeyGenerationID = 0; // this is unused for PGP
        m_nCommonEncryptionID = xml::crypto::CipherID::AES_CBC_W3C_PADDING;
        m_nChecksumDigestID = xml::crypto::DigestID::SHA512_1K;
    }
    else
        throw UnknownPropertyException(THROW_WHERE );
}

Any SAL_CALL ZipPackage::getPropertyValue( const OUString& PropertyName )
{
    // TODO/LATER: Activate the check when zip-ucp is ready
    // if ( m_nFormat != embed::StorageFormats::PACKAGE )
    //  throw UnknownPropertyException(THROW_WHERE );

    if ( PropertyName == ENCRYPTION_KEY_PROPERTY )
    {
        return Any(m_aEncryptionKey);
    }
    else if ( PropertyName == ENCRYPTION_ALGORITHMS_PROPERTY )
    {
        ::comphelper::SequenceAsHashMap aAlgorithms;
        aAlgorithms["StartKeyGenerationAlgorithm"] <<= m_nStartKeyGenerationID;
        aAlgorithms["EncryptionAlgorithm"] <<= m_nCommonEncryptionID;
        aAlgorithms["ChecksumAlgorithm"] <<= m_nChecksumDigestID;
        return Any(aAlgorithms.getAsConstNamedValueList());
    }
    if ( PropertyName == STORAGE_ENCRYPTION_KEYS_PROPERTY )
    {
        return Any(m_aStorageEncryptionKeys);
    }
    else if ( PropertyName == HAS_ENCRYPTED_ENTRIES_PROPERTY )
    {
        return Any(m_bHasEncryptedEntries);
    }
    else if ( PropertyName == ENCRYPTION_GPG_PROPERTIES )
    {
        return Any(m_aGpgProps);
    }
    else if ( PropertyName == HAS_NONENCRYPTED_ENTRIES_PROPERTY )
    {
        return Any(m_bHasNonEncryptedEntries);
    }
    else if ( PropertyName == IS_INCONSISTENT_PROPERTY )
    {
        return Any(m_bInconsistent);
    }
    else if ( PropertyName == MEDIATYPE_FALLBACK_USED_PROPERTY )
    {
        return Any(m_bMediaTypeFallbackUsed);
    }
    throw UnknownPropertyException(THROW_WHERE );
}
void SAL_CALL ZipPackage::addPropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
{
}
void SAL_CALL ZipPackage::removePropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*aListener*/ )
{
}
void SAL_CALL ZipPackage::addVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
{
}
void SAL_CALL ZipPackage::removeVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
