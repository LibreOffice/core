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

// MARKER( update_precomp.py ): autogen include statement, do not remove
#include <ZipPackage.hxx>
#include <ZipPackageSink.hxx>
#include <ZipEnumeration.hxx>
#include <ZipPackageStream.hxx>
#include <ZipPackageFolder.hxx>
#include <ZipOutputStream.hxx>
#include <ZipPackageBuffer.hxx>
#include <ZipFile.hxx>
#include <PackageConstants.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/packages/manifest/XManifestReader.hpp>
#include <com/sun/star/packages/manifest/XManifestWriter.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <ucbhelper/content.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/ucb/TransferInfo.hpp>
#include <com/sun/star/ucb/NameClash.hpp>
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#include <com/sun/star/ucb/OpenMode.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess2.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/UseBackupException.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <cppuhelper/implbase1.hxx>
#include <ContentInfo.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uri.hxx>
#include <rtl/random.h>
#include <rtl/logfile.hxx>
#include <rtl/instance.hxx>
#include <osl/time.h>
#include "com/sun/star/io/XAsyncOutputMonitor.hpp"

#include <cstring>
#include <memory>
#include <vector>

#include <ucbhelper/fileidentifierconverter.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/seekableinput.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/sequenceashashmap.hxx>

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

#define LOGFILE_AUTHOR "mg115289"

//===========================================================================

class ActiveDataStreamer : public ::cppu::WeakImplHelper1< XActiveDataStreamer >
{
    uno::Reference< XStream > mStream;
public:

    virtual uno::Reference< XStream > SAL_CALL getStream()
            throw( RuntimeException )
            { return mStream; }

    virtual void SAL_CALL setStream( const uno::Reference< XStream >& stream )
            throw( RuntimeException )
            { mStream = stream; }
};

class DummyInputStream : public ::cppu::WeakImplHelper1< XInputStream >
{
    virtual sal_Int32 SAL_CALL readBytes( uno::Sequence< sal_Int8 >&, sal_Int32 )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
        { return 0; }

    virtual sal_Int32 SAL_CALL readSomeBytes( uno::Sequence< sal_Int8 >&, sal_Int32 )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
        { return 0; }

    virtual void SAL_CALL skipBytes( sal_Int32 )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
        {}

    virtual sal_Int32 SAL_CALL available()
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
        { return 0; }

    virtual void SAL_CALL closeInput()
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
        {}
};

//===========================================================================

ZipPackage::ZipPackage ( const uno::Reference < XMultiServiceFactory > &xNewFactory )
: m_aMutexHolder( new SotMutexHolder )
, m_nStartKeyGenerationID( xml::crypto::DigestID::SHA1 )
, m_nChecksumDigestID( xml::crypto::DigestID::SHA1_1K )
, m_nCommonEncryptionID( xml::crypto::CipherID::BLOWFISH_CFB_8 )
, m_bHasEncryptedEntries ( sal_False )
, m_bHasNonEncryptedEntries ( sal_False )
, m_bInconsistent ( sal_False )
, m_bForceRecovery ( sal_False )
, m_bMediaTypeFallbackUsed ( sal_False )
, m_nFormat( embed::StorageFormats::PACKAGE ) // package is the default format
, m_bAllowRemoveOnInsert( sal_True )
, m_eMode ( e_IMode_None )
, m_xFactory( xNewFactory )
, m_pRootFolder( NULL )
, m_pZipFile( NULL )
{
    m_xRootFolder = m_pRootFolder = new ZipPackageFolder( m_xFactory, m_nFormat, m_bAllowRemoveOnInsert );
}

ZipPackage::~ZipPackage( void )
{
    delete m_pZipFile;

    // All folders and streams contain pointers to their parents, when a parent diappeares
    // it should disconnect all the children from itself during destruction automatically.
    // So there is no need in explicit m_pRootFolder->releaseUpwardRef() call here any more
    // since m_pRootFolder has no parent and cleaning of it's children will be done automatically
    // during m_pRootFolder dieing by refcount.
}

sal_Bool ZipPackage::isLocalFile() const
{
    OUString aSystemPath;
    uno::Reference< XUniversalContentBroker > xUcb(
        UniversalContentBroker::create(
            comphelper::getComponentContext( m_xFactory ) ) );
    try
    {
        aSystemPath = getSystemPathFromFileURL( xUcb, m_aURL );
    }
    catch ( Exception& )
    {
    }
    return !aSystemPath.isEmpty();
}

//--------------------------------------------------------
void ZipPackage::parseManifest()
{
    if ( m_nFormat == embed::StorageFormats::PACKAGE )
    {
        sal_Bool bManifestParsed = sal_False;
        bool bDifferentStartKeyAlgorithm = false;
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
                        OUString sManifestReader ("com.sun.star.packages.manifest.ManifestReader");
                        uno::Reference < XManifestReader > xReader ( m_xFactory->createInstance( sManifestReader ), UNO_QUERY );
                        if ( xReader.is() )
                        {
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
                            ZipPackageStream *pStream = NULL;
                            ZipPackageFolder *pFolder = NULL;

                            for ( sal_Int32 i = 0; i < nLength ; i++, pSequence++ )
                            {
                                OUString sPath, sMediaType, sVersion;
                                const PropertyValue *pValue = pSequence->getConstArray();
                                const Any *pSalt = NULL, *pVector = NULL, *pCount = NULL, *pSize = NULL, *pDigest = NULL, *pDigestAlg = NULL, *pEncryptionAlg = NULL, *pStartKeyAlg = NULL, *pDerivedKeySize = NULL;
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
                                        pStream->SetFromManifest( sal_True );

                                        if ( pSalt && pVector && pCount && pSize && pDigest && pDigestAlg && pEncryptionAlg )
                                        {
                                            uno::Sequence < sal_Int8 > aSequence;
                                            sal_Int64 nSize = 0;
                                            sal_Int32 nCount = 0, nDigestAlg = 0, nEncryptionAlg = 0;
                                            sal_Int32 nDerivedKeySize = 16, nStartKeyAlg = xml::crypto::DigestID::SHA1;

                                            pStream->SetToBeEncrypted ( sal_True );

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

                                            pStream->SetToBeCompressed ( sal_True );
                                            pStream->SetToBeEncrypted ( sal_True );
                                            pStream->SetIsEncrypted ( sal_True );
                                            if ( !m_bHasEncryptedEntries && pStream->getName() == "content.xml" )
                                            {
                                                m_bHasEncryptedEntries = sal_True;
                                                m_nStartKeyGenerationID = nStartKeyAlg;
                                                m_nChecksumDigestID = nDigestAlg;
                                                m_nCommonEncryptionID = nEncryptionAlg;
                                            }
                                        }
                                        else
                                            m_bHasNonEncryptedEntries = sal_True;
                                    }
                                }
                            }

                            bManifestParsed = sal_True;
                        }
                        else
                            throw uno::RuntimeException(OSL_LOG_PREFIX "No manifes parser!", uno::Reference< uno::XInterface >() );
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
                OSL_LOG_PREFIX "Could not parse manifest.xml\n",
                uno::Reference< uno::XInterface >() );

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
                        aPackageMediatype = OUString( ( sal_Char* )aData.getConstArray(), nRead, RTL_TEXTENCODING_ASCII_US );
                }
            }


            if ( !bManifestParsed )
            {
                // the manifest.xml could not be successfuly parsed, this is an inconsistent package
                if ( aPackageMediatype.compareToAscii("application/vnd.") == 0 )
                {
                    // accept only types that look similar to own mediatypes
                    m_pRootFolder->SetMediaType( aPackageMediatype );
                    m_bMediaTypeFallbackUsed = sal_True;
                }
            }
            else if ( !m_bForceRecovery )
            {
                // the mimetype stream should contain the information from manifest.xml
                if ( !m_pRootFolder->GetMediaType().equals( aPackageMediatype ) )
                    throw ZipIOException(
                        OSL_LOG_PREFIX "mimetype conflicts with manifest.xml\n",
                        uno::Reference< uno::XInterface >() );
            }

            m_xRootFolder->removeByName( sMimetype );
        }

        m_bInconsistent = m_pRootFolder->LookForUnexpectedODF12Streams( OUString() );

        sal_Bool bODF12AndNewer = ( m_pRootFolder->GetVersion().compareTo( ODFVER_012_TEXT ) >= 0 );
        if ( !m_bForceRecovery && bODF12AndNewer )
        {
            if ( m_bInconsistent )
            {
                // this is an ODF1.2 document that contains streams not referred in the manifest.xml;
                // in case of ODF1.2 documents without version in manifest.xml the property IsInconsistent
                // should be checked later
                throw ZipIOException(
                    OSL_LOG_PREFIX "there are streams not referred in manifest.xml\n",
                    uno::Reference< uno::XInterface >() );
            }
            else if ( bDifferentStartKeyAlgorithm )
            {
                // all the streams should be encrypted with the same StartKey in ODF1.2
                // TODO/LATER: in future the exception should be thrown
                OSL_ENSURE( false, "ODF1.2 contains different StartKey Algorithms" );
                // throw ZipIOException( OSL_LOG_PREFIX "More than one Start Key Generation algorithm is specified!", uno::Reference< uno::XInterface >() );
            }
        }

        // in case it is a correct ODF1.2 document, the version must be set
        // and the META-INF folder is reserved for package format
        if ( bODF12AndNewer )
            m_xRootFolder->removeByName( sMeta );
    }
}

//--------------------------------------------------------
void ZipPackage::parseContentType()
{
    if ( m_nFormat == embed::StorageFormats::OFOPXML )
    {
        const OUString aContentTypes("[Content_Types].xml");
        try {
            // the content type must exist in OFOPXML format!
            if ( !m_xRootFolder->hasByName( aContentTypes ) )
                throw io::IOException(OSL_LOG_PREFIX "Wrong format!",
                                        uno::Reference< uno::XInterface >() );

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
                        ::comphelper::OFOPXMLHelper::ReadContentTypeSequence( xInStream, comphelper::getComponentContext(m_xFactory) );

                    if ( aContentTypeInfo.getLength() != 2 )
                        throw io::IOException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

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

//--------------------------------------------------------
void ZipPackage::getZipFileContents()
{
    auto_ptr < ZipEnumeration > pEnum ( m_pZipFile->entries() );
    ZipPackageStream *pPkgStream;
    ZipPackageFolder *pPkgFolder, *pCurrent;
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nIndex, nStreamIndex;
    FolderHash::iterator aIter;

    while ( pEnum->hasMoreElements() )
    {
        nIndex = nOldIndex = 0;
        pCurrent = m_pRootFolder;
        const ZipEntry & rEntry = *pEnum->nextElement();
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
            while ( ( nIndex = rName.indexOf( '/', nOldIndex ) ) != -1 )
            {
                sTemp = rName.copy ( nOldIndex, nIndex - nOldIndex );
                if ( nIndex == nOldIndex )
                    break;
                if ( !pCurrent->hasByName( sTemp ) )
                {
                    pPkgFolder = new ZipPackageFolder( m_xFactory, m_nFormat, m_bAllowRemoveOnInsert );
                    pPkgFolder->setName( sTemp );
                    pPkgFolder->doSetParent( pCurrent, sal_True );
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
            pPkgStream = new ZipPackageStream( *this, m_xFactory, m_bAllowRemoveOnInsert );
            pPkgStream->SetPackageMember( sal_True );
            pPkgStream->setZipEntryOnLoading( rEntry );
            pPkgStream->setName( sTemp );
            pPkgStream->doSetParent( pCurrent, sal_True );
        }
    }

    if ( m_nFormat == embed::StorageFormats::PACKAGE )
        parseManifest();
    else if ( m_nFormat == embed::StorageFormats::OFOPXML )
        parseContentType();
}

//--------------------------------------------------------
void SAL_CALL ZipPackage::initialize( const uno::Sequence< Any >& aArguments )
        throw( Exception, RuntimeException )
{
    RTL_LOGFILE_TRACE_AUTHOR ( "package", LOGFILE_AUTHOR, "{ ZipPackage::initialize" );
    sal_Bool bBadZipFile = sal_False, bHaveZipFile = sal_True;
    uno::Reference< XProgressHandler > xProgressHandler;
    beans::NamedValue aNamedValue;

    if ( aArguments.getLength() )
    {
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
                                m_bForceRecovery = sal_True;
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
                        comphelper::getComponentContext( m_xFactory ) );
                    Any aAny = aContent.getPropertyValue("Size");
                    sal_uInt64 aSize = 0;
                    // kind of optimisation: treat empty files as nonexistent files
                    // and write to such files directly. Note that "Size" property is optional.
                    bool bHasSizeProperty = aAny >>= aSize;
                    if( !bHasSizeProperty || ( bHasSizeProperty && aSize ) )
                    {
                        uno::Reference < XActiveDataSink > xSink = new ZipPackageSink;
                        if ( aContent.openStream ( xSink ) )
                            m_xContentStream = xSink->getInputStream();
                    }
                    else
                        bHaveZipFile = sal_False;
                }
                catch ( com::sun::star::uno::Exception& )
                {
                    // Exception derived from uno::Exception thrown. This probably
                    // means the file doesn't exist...we'll create it at
                    // commitChanges time
                    bHaveZipFile = sal_False;
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

                    sal_Bool bPackFormat = sal_True;
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
                        if ( aFormatName.equals( PACKAGE_STORAGE_FORMAT_STRING ) )
                            m_nFormat = embed::StorageFormats::PACKAGE;
                        else if ( aFormatName.equals( ZIP_STORAGE_FORMAT_STRING ) )
                            m_nFormat = embed::StorageFormats::ZIP;
                        else if ( aFormatName.equals( OFOPXML_STORAGE_FORMAT_STRING ) )
                            m_nFormat = embed::StorageFormats::OFOPXML;
                        else
                            throw lang::IllegalArgumentException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 );
                    }
                    else if ( aNamedValue.Value >>= nFormatID )
                    {
                        if ( nFormatID != embed::StorageFormats::PACKAGE
                          && nFormatID != embed::StorageFormats::ZIP
                          && nFormatID != embed::StorageFormats::OFOPXML )
                            throw lang::IllegalArgumentException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 );

                        m_nFormat = nFormatID;
                    }
                    else
                        throw lang::IllegalArgumentException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 );

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
                throw com::sun::star::uno::Exception (OSL_LOG_PREFIX "Bad arguments.",
                    static_cast < ::cppu::OWeakObject * > ( this ) );
            }
        }

        try
        {
            if ( m_xContentStream.is() )
            {
                // the stream must be seekable, if it is not it will be wrapped
                m_xContentStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( m_xContentStream, m_xFactory );
                m_xContentSeek = uno::Reference < XSeekable > ( m_xContentStream, UNO_QUERY );
                if ( ! m_xContentSeek.is() )
                    throw com::sun::star::uno::Exception (OSL_LOG_PREFIX "The package component _requires_ an XSeekable interface!",
                            static_cast < ::cppu::OWeakObject * > ( this ) );

                if ( !m_xContentSeek->getLength() )
                    bHaveZipFile = sal_False;
            }
            else
                bHaveZipFile = sal_False;
        }
        catch ( com::sun::star::uno::Exception& )
        {
            // Exception derived from uno::Exception thrown. This probably
            // means the file doesn't exist...we'll create it at
            // commitChanges time
            bHaveZipFile = sal_False;
        }
        if ( bHaveZipFile )
        {
            try
            {
                m_pZipFile = new ZipFile ( m_xContentStream, m_xFactory, sal_True, m_bForceRecovery, xProgressHandler );
                getZipFileContents();
            }
            catch ( IOException & )
            {
                bBadZipFile = sal_True;
            }
            catch ( ZipException & )
            {
                bBadZipFile = sal_True;
            }
            catch ( Exception & )
            {
                if( m_pZipFile ) { delete m_pZipFile; m_pZipFile = NULL; }
                throw;
            }

            if ( bBadZipFile )
            {
                // clean up the memory, and tell the UCB about the error
                if( m_pZipFile ) { delete m_pZipFile; m_pZipFile = NULL; }

                throw com::sun::star::packages::zip::ZipIOException (
                    OSL_LOG_PREFIX "Bad Zip File.",
                    static_cast < ::cppu::OWeakObject * > ( this ) );
            }
        }
    }

    RTL_LOGFILE_TRACE_AUTHOR ( "package", LOGFILE_AUTHOR, "} ZipPackage::initialize" );
}

//--------------------------------------------------------
Any SAL_CALL ZipPackage::getByHierarchicalName( const OUString& aName )
        throw( NoSuchElementException, RuntimeException )
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
        ZipPackageFolder * pPrevious = NULL;
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
                throw NoSuchElementException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
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
                throw NoSuchElementException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
        }
    }
}

//--------------------------------------------------------
sal_Bool SAL_CALL ZipPackage::hasByHierarchicalName( const OUString& aName )
        throw( RuntimeException )
{
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nIndex, nStreamIndex;
    FolderHash::iterator aIter;

    if ( ( nIndex = aName.getLength() ) == 1 && *aName.getStr() == '/' )
        return sal_True;
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
        ZipPackageFolder * pPrevious = NULL;
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
        return sal_False;
    }
}

//--------------------------------------------------------
uno::Reference< XInterface > SAL_CALL ZipPackage::createInstance()
        throw( Exception, RuntimeException )
{
    uno::Reference < XInterface > xRef = *( new ZipPackageStream ( *this, m_xFactory, m_bAllowRemoveOnInsert ) );
    return xRef;
}
//--------------------------------------------------------
uno::Reference< XInterface > SAL_CALL ZipPackage::createInstanceWithArguments( const uno::Sequence< Any >& aArguments )
        throw( Exception, RuntimeException )
{
    sal_Bool bArg = sal_False;
    uno::Reference < XInterface > xRef;
    if ( aArguments.getLength() )
        aArguments[0] >>= bArg;
    if ( bArg )
        xRef = *new ZipPackageFolder ( m_xFactory, m_nFormat, m_bAllowRemoveOnInsert );
    else
        xRef = *new ZipPackageStream ( *this, m_xFactory, m_bAllowRemoveOnInsert );

    return xRef;
}

//--------------------------------------------------------
void ZipPackage::WriteMimetypeMagicFile( ZipOutputStream& aZipOut )
{
    const OUString sMime ("mimetype");
    if ( m_xRootFolder->hasByName( sMime ) )
        m_xRootFolder->removeByName( sMime );

    ZipEntry * pEntry = new ZipEntry;
    sal_Int32 nBufferLength = m_pRootFolder->GetMediaType().getLength();
    OString sMediaType = OUStringToOString( m_pRootFolder->GetMediaType(), RTL_TEXTENCODING_ASCII_US );
    uno::Sequence< sal_Int8 > aType( ( sal_Int8* )sMediaType.getStr(),
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
        aZipOut.putNextEntry( *pEntry, NULL );
        aZipOut.write( aType, 0, nBufferLength );
        aZipOut.closeEntry();
    }
    catch ( const ::com::sun::star::io::IOException & r )
    {
        throw WrappedTargetException(
                OSL_LOG_PREFIX "Error adding mimetype to the ZipOutputStream!",
                static_cast < OWeakObject * > ( this ),
                makeAny( r ) );
    }
}

//--------------------------------------------------------
void ZipPackage::WriteManifest( ZipOutputStream& aZipOut, const vector< uno::Sequence < PropertyValue > >& aManList )
{
    // Write the manifest
    uno::Reference < XOutputStream > xManOutStream;
    uno::Reference < XManifestWriter > xWriter ( m_xFactory->createInstance("com.sun.star.packages.manifest.ManifestWriter"), UNO_QUERY );
    if ( xWriter.is() )
    {
        ZipEntry * pEntry = new ZipEntry;
        ZipPackageBuffer *pBuffer = new ZipPackageBuffer( n_ConstBufferSize );
        xManOutStream = uno::Reference < XOutputStream > ( *pBuffer, UNO_QUERY );

        pEntry->sPath = "META-INF/manifest.xml";
        pEntry->nMethod = DEFLATED;
        pEntry->nCrc = -1;
        pEntry->nSize = pEntry->nCompressedSize = -1;
        pEntry->nTime = ZipOutputStream::getCurrentDosTime();

        // Convert vector into a uno::Sequence
        uno::Sequence < uno::Sequence < PropertyValue > > aManifestSequence ( aManList.size() );
        sal_Int32 nInd = 0;
        for ( vector < uno::Sequence < PropertyValue > >::const_iterator aIter = aManList.begin(), aEnd = aManList.end();
             aIter != aEnd;
             ++aIter, ++nInd )
        {
            aManifestSequence[nInd] = ( *aIter );
        }
        xWriter->writeManifestSequence ( xManOutStream,  aManifestSequence );

        sal_Int32 nBufferLength = static_cast < sal_Int32 > ( pBuffer->getPosition() );
        pBuffer->realloc( nBufferLength );

        // the manifest.xml is never encrypted - so pass an empty reference
        aZipOut.putNextEntry( *pEntry, NULL );
        aZipOut.write( pBuffer->getSequence(), 0, nBufferLength );
        aZipOut.closeEntry();
    }
    else
    {
                OSL_FAIL( "Couldn't get a ManifestWriter!" );
        IOException aException;
        throw WrappedTargetException(
                OSL_LOG_PREFIX "Couldn't get a ManifestWriter!",
                static_cast < OWeakObject * > ( this ),
                makeAny( aException ) );
    }
}

//--------------------------------------------------------
void ZipPackage::WriteContentTypes( ZipOutputStream& aZipOut, const vector< uno::Sequence < PropertyValue > >& aManList )
{
    const OUString sFullPath ("FullPath");
    const OUString sMediaType ("MediaType");

    ZipEntry* pEntry = new ZipEntry;
    ZipPackageBuffer *pBuffer = new ZipPackageBuffer( n_ConstBufferSize );
    uno::Reference< io::XOutputStream > xConTypeOutStream( *pBuffer, UNO_QUERY );

    pEntry->sPath = "[Content_Types].xml";
    pEntry->nMethod = DEFLATED;
    pEntry->nCrc = -1;
    pEntry->nSize = pEntry->nCompressedSize = -1;
    pEntry->nTime = ZipOutputStream::getCurrentDosTime();

    // Convert vector into a uno::Sequence
    // TODO/LATER: use Defaulst entries in future
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
        OSL_ENSURE( ( *aIter )[PKG_MNFST_MEDIATYPE].Name.equals( sMediaType ) && ( *aIter )[PKG_MNFST_FULLPATH].Name.equals( sFullPath ),
                    "The mediatype sequence format is wrong!\n" );
        ( *aIter )[PKG_MNFST_MEDIATYPE].Value >>= aType;
        if ( !aType.isEmpty() )
        {
            // only nonempty type makes sence here
            nSeqLength++;
            ( *aIter )[PKG_MNFST_FULLPATH].Value >>= aPath;
            aOverridesSequence[nSeqLength-1].First = "/" + aPath;
            aOverridesSequence[nSeqLength-1].Second = aType;
        }
    }
    aOverridesSequence.realloc( nSeqLength );

    ::comphelper::OFOPXMLHelper::WriteContentSequence(
            xConTypeOutStream, aDefaultsSequence, aOverridesSequence, m_xFactory );

    sal_Int32 nBufferLength = static_cast < sal_Int32 > ( pBuffer->getPosition() );
    pBuffer->realloc( nBufferLength );

    // there is no encryption in this format currently
    aZipOut.putNextEntry( *pEntry, NULL );
    aZipOut.write( pBuffer->getSequence(), 0, nBufferLength );
    aZipOut.closeEntry();
}

//--------------------------------------------------------
void ZipPackage::ConnectTo( const uno::Reference< io::XInputStream >& xInStream )
{
    m_xContentSeek.set( xInStream, uno::UNO_QUERY_THROW );
    m_xContentStream = xInStream;

    // seek back to the beginning of the temp file so we can read segments from it
    m_xContentSeek->seek( 0 );
    if ( m_pZipFile )
        m_pZipFile->setInputStream( m_xContentStream );
    else
        m_pZipFile = new ZipFile ( m_xContentStream, m_xFactory, sal_False );
}

//--------------------------------------------------------
uno::Reference< io::XInputStream > ZipPackage::writeTempFile()
{
    // In case the target local file does not exist or empty
    // write directly to it otherwize create a temporary file to write to.
    // If a temporary file is created it is returned back by the method.
    // If the data written directly, xComponentStream will be switched here

    sal_Bool bUseTemp = sal_True;
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
                    bUseTemp = sal_False;
            }
        }
    }
    else if ( m_eMode == e_IMode_XStream && !m_pZipFile )
    {
        // write directly to an empty stream
        xTempOut = m_xStream->getOutputStream();
        if( xTempOut.is() )
            bUseTemp = sal_False;
    }

    if( bUseTemp )
    {
        // create temporary file
        uno::Reference < io::XStream > xTempFile( io::TempFile::create(comphelper::getComponentContext(m_xFactory)), UNO_QUERY_THROW );
        xTempOut.set( xTempFile->getOutputStream(), UNO_SET_THROW );
        xTempIn.set( xTempFile->getInputStream(), UNO_SET_THROW );
    }

    // Hand it to the ZipOutputStream:
    ZipOutputStream aZipOut( m_xFactory, xTempOut );
    aZipOut.setMethod( DEFLATED );
    aZipOut.setLevel( DEFAULT_COMPRESSION );

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

        // Get a random number generator and seed it with current timestamp
        // This will be used to generate random salt and initialisation vectors
        // for encrypted streams
        TimeValue aTime;
        osl_getSystemTime( &aTime );
        rtlRandomPool aRandomPool = rtl_random_createPool ();
        rtl_random_addBytes ( aRandomPool, &aTime, 8 );

        // call saveContents ( it will recursively save sub-directories
        OUString aEmptyString;
        m_pRootFolder->saveContents( aEmptyString, aManList, aZipOut, GetEncryptionKey(), aRandomPool );

        // Clean up random pool memory
        rtl_random_destroyPool ( aRandomPool );

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
            // no information loss appeares, thus no special handling is required
               uno::Any aCaught( ::cppu::getCaughtException() );

            // it is allowed to throw WrappedTargetException
            WrappedTargetException aException;
            if ( aCaught >>= aException )
                throw aException;

            throw WrappedTargetException(
                    OSL_LOG_PREFIX "Problem writing the original content!",
                    static_cast < OWeakObject * > ( this ),
                    aCaught );
        }
        else
        {
            // the document is written directly, although it was empty it is important to notify that the writing has failed
            // TODO/LATER: let the package be able to recover in this situation
            OUString aErrTxt(OSL_LOG_PREFIX "This package is unusable!");
            embed::UseBackupException aException( aErrTxt, uno::Reference< uno::XInterface >(), OUString() );
            throw WrappedTargetException( aErrTxt,
                                            static_cast < OWeakObject * > ( this ),
                                            makeAny ( aException ) );
        }
    }

    return xResult;
}

//--------------------------------------------------------
uno::Reference< XActiveDataStreamer > ZipPackage::openOriginalForOutput()
{
    // open and truncate the original file
    Content aOriginalContent(
        m_aURL, uno::Reference< XCommandEnvironment >(),
        comphelper::getComponentContext( m_xFactory ) );
    uno::Reference< XActiveDataStreamer > xSink = new ActiveDataStreamer;

    if ( m_eMode == e_IMode_URL )
    {
        try
        {
            sal_Bool bTruncSuccess = sal_False;

            try
            {
                Exception aDetect;
                sal_Int64 aSize = 0;
                Any aAny = aOriginalContent.setPropertyValue("Size", makeAny( aSize ) );
                if( !( aAny >>= aDetect ) )
                    bTruncSuccess = sal_True;
            }
            catch( Exception& )
            {
            }

            if( !bTruncSuccess )
            {
                // the file is not accessible
                // just try to write an empty stream to it

                uno::Reference< XInputStream > xTempIn = new DummyInputStream; //uno::Reference< XInputStream >( xTempOut, UNO_QUERY );
                aOriginalContent.writeStream( xTempIn , sal_True );
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

//--------------------------------------------------------
void SAL_CALL ZipPackage::commitChanges()
        throw( WrappedTargetException, RuntimeException )
{
    // lock the component for the time of commiting
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( m_eMode == e_IMode_XInputStream )
    {
        IOException aException;
        throw WrappedTargetException(OSL_LOG_PREFIX "This package is read only!",
                static_cast < OWeakObject * > ( this ), makeAny ( aException ) );
    }

    RTL_LOGFILE_TRACE_AUTHOR ( "package", LOGFILE_AUTHOR, "{ ZipPackage::commitChanges" );

    // first the writeTempFile is called, if it returns a stream the stream should be written to the target
    // if no stream was returned, the file was written directly, nothing should be done

    uno::Reference< io::XInputStream > xTempInStream = writeTempFile();
    if ( xTempInStream.is() )
    {
        uno::Reference< io::XSeekable > xTempSeek( xTempInStream, uno::UNO_QUERY_THROW );

        try
        {
            xTempSeek->seek( 0 );
        }
        catch( const uno::Exception& r )
        {
            throw WrappedTargetException(OSL_LOG_PREFIX "Temporary file should be seekable!",
                    static_cast < OWeakObject * > ( this ), makeAny ( r ) );
        }

        // connect to the temporary stream
        ConnectTo( xTempInStream );

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
                    throw uno::RuntimeException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

                // after successful truncation the original file contents are already lost
                xTruncate->truncate();
            }
            catch( const uno::Exception& r )
            {
                throw WrappedTargetException(OSL_LOG_PREFIX "This package is read only!",
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
            sal_Bool bCanBeCorrupted = sal_False;

            if( isLocalFile() )
            {
                // write directly in case of local file
                uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess2 > xSimpleAccess(
                    SimpleFileAccess::create( comphelper::getComponentContext(m_xFactory) ) );
                OSL_ENSURE( xSimpleAccess.is(), "Can't instatiate SimpleFileAccess service!\n" );
                uno::Reference< io::XTruncate > xOrigTruncate;
                if ( xSimpleAccess.is() )
                {
                    try
                    {
                        aOrigFileStream = xSimpleAccess->openFileWrite( m_aURL );
                        xOrigTruncate = uno::Reference< io::XTruncate >( aOrigFileStream, uno::UNO_QUERY_THROW );
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

                        aOrigFileStream = uno::Reference< XOutputStream >();
                        // the original file can already be corrupted
                        bCanBeCorrupted = sal_True;
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
                        throw uno::RuntimeException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

                    OUString sTargetFolder = m_aURL.copy ( 0, m_aURL.lastIndexOf ( static_cast < sal_Unicode > ( '/' ) ) );
                    Content aContent(
                        sTargetFolder, uno::Reference< XCommandEnvironment >(),
                        comphelper::getComponentContext( m_xFactory ) );

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
                catch ( const ::com::sun::star::uno::Exception& r )
                {
                    if ( bCanBeCorrupted )
                        DisconnectFromTargetAndThrowException_Impl( xTempInStream );

                    throw WrappedTargetException(
                                                OSL_LOG_PREFIX "This package may be read only!",
                                                static_cast < OWeakObject * > ( this ),
                                                makeAny ( r ) );
                }
            }
        }
    }

    // after successful storing it can be set to false
    m_bMediaTypeFallbackUsed = sal_False;

    RTL_LOGFILE_TRACE_AUTHOR ( "package", LOGFILE_AUTHOR, "} ZipPackage::commitChanges" );
}

//--------------------------------------------------------
void ZipPackage::DisconnectFromTargetAndThrowException_Impl( const uno::Reference< io::XInputStream >& xTempStream )
{
    m_xStream = uno::Reference< io::XStream >( xTempStream, uno::UNO_QUERY );
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

    OUString aErrTxt(OSL_LOG_PREFIX "This package is read only!");
    embed::UseBackupException aException( aErrTxt, uno::Reference< uno::XInterface >(), aTempURL );
    throw WrappedTargetException( aErrTxt,
                                    static_cast < OWeakObject * > ( this ),
                                    makeAny ( aException ) );
}

//--------------------------------------------------------
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
            throw uno::RuntimeException(OSL_LOG_PREFIX "No expected key is provided!", uno::Reference< uno::XInterface >() );

        for ( sal_Int32 nInd = 0; nInd < m_aStorageEncryptionKeys.getLength(); nInd++ )
            if ( m_aStorageEncryptionKeys[nInd].Name.equals( aNameToFind ) )
                m_aStorageEncryptionKeys[nInd].Value >>= aResult;

        // empty keys are not allowed here
        // so it is not important whether there is no key, or the key is empty, it is an error
        if ( !aResult.getLength() )
            throw uno::RuntimeException(OSL_LOG_PREFIX "No expected key is provided!", uno::Reference< uno::XInterface >() );
    }
    else
        aResult = m_aEncryptionKey;

    return aResult;
}

//--------------------------------------------------------
sal_Bool SAL_CALL ZipPackage::hasPendingChanges()
        throw( RuntimeException )
{
    return sal_False;
}
//--------------------------------------------------------
Sequence< ElementChange > SAL_CALL ZipPackage::getPendingChanges()
        throw( RuntimeException )
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
    return uno::Reference< XInterface >( *new ZipPackage( xMgr ) );
}

//--------------------------------------------------------
OUString ZipPackage::static_getImplementationName()
{
    return OUString("com.sun.star.packages.comp.ZipPackage");
}

//--------------------------------------------------------
Sequence< OUString > ZipPackage::static_getSupportedServiceNames()
{
    uno::Sequence< OUString > aNames( 1 );
    aNames[0] = OUString("com.sun.star.packages.Package");
    return aNames;
}
//--------------------------------------------------------
sal_Bool SAL_CALL ZipPackage::static_supportsService( OUString const & rServiceName )
{
    return rServiceName == getSupportedServiceNames()[0];
}

//--------------------------------------------------------
OUString ZipPackage::getImplementationName()
    throw ( RuntimeException )
{
    return static_getImplementationName();
}

//--------------------------------------------------------
Sequence< OUString > ZipPackage::getSupportedServiceNames()
    throw ( RuntimeException )
{
    return static_getSupportedServiceNames();
}
//--------------------------------------------------------
sal_Bool SAL_CALL ZipPackage::supportsService( OUString const & rServiceName )
    throw ( RuntimeException )
{
    return static_supportsService ( rServiceName );
}
//--------------------------------------------------------
uno::Reference < XSingleServiceFactory > ZipPackage::createServiceFactory( uno::Reference < XMultiServiceFactory > const & rServiceFactory )
{
    return cppu::createSingleFactory ( rServiceFactory,
                                           static_getImplementationName(),
                                           ZipPackage_createInstance,
                                           static_getSupportedServiceNames() );
}

namespace { struct lcl_ImplId : public rtl::Static< ::cppu::OImplementationId, lcl_ImplId > {}; }

//--------------------------------------------------------
Sequence< sal_Int8 > ZipPackage::getUnoTunnelImplementationId( void )
    throw ( RuntimeException )
{
    ::cppu::OImplementationId &rId = lcl_ImplId::get();
    return rId.getImplementationId();
}

//--------------------------------------------------------
sal_Int64 SAL_CALL ZipPackage::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw( RuntimeException )
{
    if ( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast < sal_Int64 > ( this );
    return 0;
}

//--------------------------------------------------------
uno::Reference< XPropertySetInfo > SAL_CALL ZipPackage::getPropertySetInfo()
        throw( RuntimeException )
{
    return uno::Reference < XPropertySetInfo > ();
}

//--------------------------------------------------------
void SAL_CALL ZipPackage::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    if ( m_nFormat != embed::StorageFormats::PACKAGE )
        throw UnknownPropertyException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if (aPropertyName == HAS_ENCRYPTED_ENTRIES_PROPERTY
      ||aPropertyName == HAS_NONENCRYPTED_ENTRIES_PROPERTY
      ||aPropertyName == IS_INCONSISTENT_PROPERTY
      ||aPropertyName == MEDIATYPE_FALLBACK_USED_PROPERTY)
        throw PropertyVetoException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    else if ( aPropertyName == ENCRYPTION_KEY_PROPERTY )
    {
        if ( !( aValue >>= m_aEncryptionKey ) )
            throw IllegalArgumentException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 2 );

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
            throw IllegalArgumentException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 2 );

        if ( aKeys.getLength() )
        {
            bool bHasSHA256 = false;
            bool bHasSHA1 = false;
            for ( sal_Int32 nInd = 0; nInd < aKeys.getLength(); nInd++ )
            {
                if ( aKeys[nInd].Name.equals( PACKAGE_ENCRYPTIONDATA_SHA256UTF8 ) )
                    bHasSHA256 = true;
                if ( aKeys[nInd].Name.equals( PACKAGE_ENCRYPTIONDATA_SHA1UTF8 ) )
                    bHasSHA1 = true;
            }

            if ( !bHasSHA256 || !bHasSHA1 )
                throw IllegalArgumentException(OSL_LOG_PREFIX "Expected keys are not provided!", uno::Reference< uno::XInterface >(), 2 );
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
            throw IllegalArgumentException(OSL_LOG_PREFIX "unexpected algorithms list is provided.", uno::Reference< uno::XInterface >(), 2 );
        }

        for ( sal_Int32 nInd = 0; nInd < aAlgorithms.getLength(); nInd++ )
        {
            if ( aAlgorithms[nInd].Name == "StartKeyGenerationAlgorithm" )
            {
                sal_Int32 nID = 0;
                if ( !( aAlgorithms[nInd].Value >>= nID )
                  || ( nID != xml::crypto::DigestID::SHA256 && nID != xml::crypto::DigestID::SHA1 ) )
                    throw IllegalArgumentException(OSL_LOG_PREFIX "Unexpected start key generation algorithm is provided!", uno::Reference< uno::XInterface >(), 2 );

                m_nStartKeyGenerationID = nID;
            }
            else if ( aAlgorithms[nInd].Name == "EncryptionAlgorithm" )
            {
                sal_Int32 nID = 0;
                if ( !( aAlgorithms[nInd].Value >>= nID )
                  || ( nID != xml::crypto::CipherID::AES_CBC_W3C_PADDING && nID != xml::crypto::CipherID::BLOWFISH_CFB_8 ) )
                    throw IllegalArgumentException(OSL_LOG_PREFIX "Unexpected start key generation algorithm is provided!", uno::Reference< uno::XInterface >(), 2 );

                m_nCommonEncryptionID = nID;
            }
            else if ( aAlgorithms[nInd].Name == "ChecksumAlgorithm" )
            {
                sal_Int32 nID = 0;
                if ( !( aAlgorithms[nInd].Value >>= nID )
                  || ( nID != xml::crypto::DigestID::SHA1_1K && nID != xml::crypto::DigestID::SHA256_1K ) )
                    throw IllegalArgumentException(OSL_LOG_PREFIX "Unexpected start key generation algorithm is provided!", uno::Reference< uno::XInterface >(), 2 );

                m_nChecksumDigestID = nID;
            }
            else
            {
                OSL_ENSURE( sal_False, "Unexpected encryption algorithm is provided!" );
                throw IllegalArgumentException(OSL_LOG_PREFIX "unexpected algorithms list is provided.", uno::Reference< uno::XInterface >(), 2 );
            }
        }
    }
    else
        throw UnknownPropertyException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
}

//--------------------------------------------------------
Any SAL_CALL ZipPackage::getPropertyValue( const OUString& PropertyName )
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    // TODO/LATER: Activate the check when zip-ucp is ready
    // if ( m_nFormat != embed::StorageFormats::PACKAGE )
    //  throw UnknownPropertyException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

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
    throw UnknownPropertyException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
}
//--------------------------------------------------------
void SAL_CALL ZipPackage::addPropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}
//--------------------------------------------------------
void SAL_CALL ZipPackage::removePropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*aListener*/ )
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}
//--------------------------------------------------------
void SAL_CALL ZipPackage::addVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}
//--------------------------------------------------------
void SAL_CALL ZipPackage::removeVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
