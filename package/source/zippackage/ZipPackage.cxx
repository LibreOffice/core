/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ZipPackage.cxx,v $
 * $Revision: 1.114 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"
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
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/packages/manifest/XManifestReader.hpp>
#include <com/sun/star/packages/manifest/XManifestWriter.hpp>
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
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/UseBackupException.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <cppuhelper/implbase1.hxx>
#include <ContentInfo.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/uri.hxx>
#include <rtl/random.h>
#include <rtl/logfile.hxx>
#include <osl/time.h>
#include <osl/file.hxx>
#include "com/sun/star/io/XAsyncOutputMonitor.hpp"

#include <memory>
#include <vector>

#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/fileidentifierconverter.hxx>
#include <comphelper/seekableinput.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/ofopxmlhelper.hxx>

using namespace rtl;
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


sal_Bool isLocalFile_Impl( ::rtl::OUString aURL )
{
    ::rtl::OUString aSystemPath;
    ContentBroker* pBroker = ContentBroker::get();
    if ( !pBroker )
    {
        ::rtl::OUString aRet;
        if ( FileBase::getSystemPathFromFileURL( aURL, aRet ) == FileBase::E_None )
            aSystemPath = aRet;
    }
    else
    {
        uno::Reference< XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();
        try
        {
               aSystemPath = getSystemPathFromFileURL( xManager, aURL );
        }
        catch ( Exception& )
        {
        }
    }

    return ( aSystemPath.getLength() != 0 );
}

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
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >&, sal_Int32 )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        { return 0; }

    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >&, sal_Int32 )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        { return 0; }

    virtual void SAL_CALL skipBytes( sal_Int32 )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        {}

    virtual sal_Int32 SAL_CALL available()
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        { return 0; }

    virtual void SAL_CALL closeInput()
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        {}
};

//===========================================================================

ZipPackage::ZipPackage (const uno::Reference < XMultiServiceFactory > &xNewFactory)
: bHasEncryptedEntries ( sal_False )
, bUseManifest ( sal_True )
, bForceRecovery ( sal_False )
, m_bMediaTypeFallbackUsed ( sal_False )
, m_nFormat( PACKAGE_FORMAT ) // package is the default format
, m_bAllowRemoveOnInsert( sal_True )
, eMode ( e_IMode_None )
, xFactory( xNewFactory )
, pRootFolder( NULL )
, pZipFile( NULL )
{
    xRootFolder = pRootFolder = new ZipPackageFolder( xFactory, m_nFormat, m_bAllowRemoveOnInsert );
}

ZipPackage::~ZipPackage( void )
{
    delete pZipFile;

    // All folders and streams contain pointers to their parents, when a parent diappeares
    // it should disconnect all the children from itself during destruction automatically.
    // So there is no need in explicit pRootFolder->releaseUpwardRef() call here any more
    // since pRootFolder has no parent and cleaning of it's children will be done automatically
    // during pRootFolder dieing by refcount.

#if 0
    // As all folders and streams contain references to their parents,
    // we must remove these references so that they will be deleted when
    // the hash_map of the root folder is cleared, releasing all subfolders
    // and substreams which in turn release theirs, etc. When xRootFolder is
    // released when this destructor completes, the folder tree should be
    // deleted fully (and automagically).

    pRootFolder->releaseUpwardRef();
#endif
}

void ZipPackage::parseManifest()
{
    if ( m_nFormat == PACKAGE_FORMAT )
    {
        sal_Bool bManifestParsed = sal_False;
        const OUString sMeta ( RTL_CONSTASCII_USTRINGPARAM ( "META-INF" ) );
        if ( xRootFolder->hasByName( sMeta ) )
        {
            const OUString sManifest (RTL_CONSTASCII_USTRINGPARAM( "manifest.xml") );

            try {
                uno::Reference< XUnoTunnel > xTunnel;
                Any aAny = xRootFolder->getByName( sMeta );
                aAny >>= xTunnel;
                uno::Reference< XNameContainer > xMetaInfFolder( xTunnel, UNO_QUERY );
                if ( xMetaInfFolder.is() && xMetaInfFolder->hasByName( sManifest ) )
                {
                    aAny = xMetaInfFolder->getByName( sManifest );
                    aAny >>= xTunnel;
                    uno::Reference < XActiveDataSink > xSink (xTunnel, UNO_QUERY);
                    if (xSink.is())
                    {
                        OUString sManifestReader ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.ManifestReader" ) );
                        uno::Reference < XManifestReader > xReader (xFactory->createInstance( sManifestReader ), UNO_QUERY );
                        if ( xReader.is() )
                        {
                            const OUString sPropFullPath ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
                            const OUString sPropVersion ( RTL_CONSTASCII_USTRINGPARAM ( "Version" ) );
                            const OUString sPropMediaType ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
                            const OUString sPropInitialisationVector ( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) );
                            const OUString sPropSalt ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
                            const OUString sPropIterationCount ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );
                            const OUString sPropSize ( RTL_CONSTASCII_USTRINGPARAM ( "Size" ) );
                            const OUString sPropDigest ( RTL_CONSTASCII_USTRINGPARAM ( "Digest" ) );

                            Sequence < Sequence < PropertyValue > > aManifestSequence = xReader->readManifestSequence ( xSink->getInputStream() );
                            sal_Int32 nLength = aManifestSequence.getLength();
                            const Sequence < PropertyValue > *pSequence = aManifestSequence.getConstArray();
                            ZipPackageStream *pStream = NULL;
                            ZipPackageFolder *pFolder = NULL;

                            for (sal_Int32 i = 0; i < nLength ; i++, pSequence++)
                            {
                                OUString sPath, sMediaType, sVersion;
                                const PropertyValue *pValue = pSequence->getConstArray();
                                const Any *pSalt = NULL, *pVector = NULL, *pCount = NULL, *pSize = NULL, *pDigest = NULL;
                                for (sal_Int32 j = 0, nNum = pSequence->getLength(); j < nNum; j++ )
                                {
                                    if (pValue[j].Name.equals( sPropFullPath ) )
                                        pValue[j].Value >>= sPath;
                                    else if (pValue[j].Name.equals( sPropVersion ) )
                                        pValue[j].Value >>= sVersion;
                                    else if (pValue[j].Name.equals( sPropMediaType ) )
                                        pValue[j].Value >>= sMediaType;
                                    else if (pValue[j].Name.equals( sPropSalt ) )
                                        pSalt = &(pValue[j].Value);
                                    else if (pValue[j].Name.equals( sPropInitialisationVector ) )
                                        pVector = &(pValue[j].Value);
                                    else if (pValue[j].Name.equals( sPropIterationCount ) )
                                        pCount = &(pValue[j].Value);
                                    else if (pValue[j].Name.equals( sPropSize ) )
                                        pSize = &(pValue[j].Value);
                                    else if (pValue[j].Name.equals( sPropDigest ) )
                                        pDigest = &(pValue[j].Value);
                                }
                                if (sPath.getLength() && hasByHierarchicalName ( sPath ) )
                                {
                                    aAny = getByHierarchicalName( sPath );
                                    uno::Reference < XUnoTunnel > xUnoTunnel;
                                    aAny >>= xUnoTunnel;
                                    sal_Int64 nTest=0;
                                    if ((nTest = xUnoTunnel->getSomething(ZipPackageFolder::static_getImplementationId())) != 0)
                                    {
                                        pFolder = reinterpret_cast < ZipPackageFolder* > ( nTest );
                                        pFolder->SetMediaType ( sMediaType );
                                        pFolder->SetVersion ( sVersion );
                                    }
                                    else
                                    {
                                        pStream = reinterpret_cast < ZipPackageStream* > ( xUnoTunnel->getSomething(ZipPackageStream::static_getImplementationId()));
                                        pStream->SetMediaType ( sMediaType );

                                        if (pSalt && pVector && pCount && pSize)
                                        {
                                            Sequence < sal_uInt8 > aSequence;
                                            sal_Int32 nCount = 0, nSize = 0;
                                                                           pStream->SetToBeEncrypted ( sal_True );

                                            *pSalt >>= aSequence;
                                            pStream->setSalt ( aSequence );

                                            *pVector >>= aSequence;
                                            pStream->setInitialisationVector ( aSequence );

                                            *pCount >>= nCount;
                                            pStream->setIterationCount ( nCount );

                                            *pSize >>= nSize;
                                            pStream->setSize ( nSize );

                                            if ( pDigest )
                                            {
                                                *pDigest >>= aSequence;
                                                pStream->setDigest ( aSequence );
                                            }

                                            pStream->SetToBeCompressed ( sal_True );
                                            pStream->SetToBeEncrypted ( sal_True );
                                            pStream->SetIsEncrypted ( sal_True );
                                            if ( !bHasEncryptedEntries && pStream->getName().compareToAscii ( "content.xml" ) == 0 )
                                                bHasEncryptedEntries = sal_True;
                                        }
                                    }
                                }
                            }

                            bManifestParsed = sal_True;
                        }
                        else
                            VOS_ENSURE ( 0, "Couldn't get a ManifestReader!" ); // throw RuntimeException?
                    }

                    // now hide the manifest.xml file from user
                    xMetaInfFolder->removeByName( sManifest );
                }
            }
            catch( Exception& )
            {
                if ( !bForceRecovery )
                    throw;
            }
        }

        const OUString sMimetype ( RTL_CONSTASCII_USTRINGPARAM ( "mimetype" ) );
        if ( xRootFolder->hasByName( sMimetype ) )
        {
            if ( !bManifestParsed )
            {
                // try to get mediatype from the "mimetype" stream
                uno::Reference< lang::XUnoTunnel > xMimeTypeTunnel;
                xRootFolder->getByName( sMimetype ) >>= xMimeTypeTunnel;
                uno::Reference < io::XActiveDataSink > xMimeSink( xMimeTypeTunnel, UNO_QUERY );
                if ( xMimeSink.is() )
                {
                    uno::Reference< io::XInputStream > xMimeInStream = xMimeSink->getInputStream();
                    if ( xMimeInStream.is() )
                    {
                        // Mediatypes longer than 1024 symbols should not appear here
                        uno::Sequence< sal_Int8 > aData( 1024 );
                        sal_Int32 nRead = xMimeInStream->readBytes( aData, 1024 );
                        OSL_ENSURE( nRead == aData.getLength(), "Difference between reading result and data!\n" );
                        if ( nRead > aData.getLength() )
                            nRead = aData.getLength();
                        if ( nRead )
                        {
                            ::rtl::OUString aFallBack( (sal_Char*)aData.getConstArray(), nRead, RTL_TEXTENCODING_ASCII_US );
                            if ( aFallBack.compareToAscii( RTL_CONSTASCII_STRINGPARAM( "application/vnd." ) ) == 0 )
                            {
                                // accept only types that look similar to own mediatypes
                                pRootFolder->SetMediaType( aFallBack );
                                m_bMediaTypeFallbackUsed = sal_True;
                            }
                        }
                    }
                }

            }
            xRootFolder->removeByName( sMimetype );
        }
    }
}

void ZipPackage::parseContentType()
{
    if ( m_nFormat == OFOPXML_FORMAT )
    {
        const ::rtl::OUString aContentTypes( RTL_CONSTASCII_USTRINGPARAM ( "[Content_Types].xml" ) );
        try {
            // the content type must exist in OFOPXML format!
            if ( !xRootFolder->hasByName( aContentTypes ) )
                throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Wrong format!" ) ),
                                        uno::Reference< uno::XInterface >() );

            uno::Reference< lang::XUnoTunnel > xTunnel;
            uno::Any aAny = xRootFolder->getByName( aContentTypes );
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
                        ::comphelper::OFOPXMLHelper::ReadContentTypeSequence( xInStream, xFactory );

                    if ( aContentTypeInfo.getLength() != 2 )
                        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

                    // set the implicit types fist
                    for ( nInd = 0; nInd < aContentTypeInfo[0].getLength(); nInd++ )
                        pRootFolder->setChildStreamsTypeByExtension( aContentTypeInfo[0][nInd] );

                    // now set the explicit types
                    for ( nInd = 0; nInd < aContentTypeInfo[1].getLength(); nInd++ )
                    {
                        ::rtl::OUString aPath;
                        if ( aContentTypeInfo[1][nInd].First.toChar() == (sal_Unicode)'/' )
                            aPath = aContentTypeInfo[1][nInd].First.copy( 1 );
                        else
                            aPath = aContentTypeInfo[1][nInd].First;

                        if ( aPath.getLength() && hasByHierarchicalName( aPath ) )
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

            xRootFolder->removeByName( aContentTypes );
        }
        catch( uno::Exception& )
        {
            if ( !bForceRecovery )
                throw;
        }
    }
}

void ZipPackage::getZipFileContents()
{
    auto_ptr < ZipEnumeration > pEnum ( pZipFile->entries() );
    ZipPackageStream *pPkgStream;
    ZipPackageFolder *pPkgFolder, *pCurrent;
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nIndex, nStreamIndex;
    FolderHash::iterator aIter;

    while (pEnum->hasMoreElements())
    {
        nIndex = nOldIndex = 0;
        pCurrent = pRootFolder;
        const ZipEntry & rEntry = *pEnum->nextElement();
        const OUString & rName = rEntry.sName;

        nStreamIndex = rName.lastIndexOf ( '/' );
        if ( nStreamIndex != -1 )
        {
            sDirName = rName.copy ( 0, nStreamIndex);
            aIter = aRecent.find ( sDirName );
            if ( aIter != aRecent.end() )
                pCurrent = (*aIter).second;
        }

        if ( pCurrent == pRootFolder )
        {
            while ( (nIndex = rName.indexOf('/', nOldIndex) ) != -1 )
            {
                sTemp = rName.copy ( nOldIndex, nIndex - nOldIndex );
                if (nIndex == nOldIndex)
                    break;
                if ( !pCurrent->hasByName( sTemp ) )
                {
                    pPkgFolder = new ZipPackageFolder( xFactory, m_nFormat, m_bAllowRemoveOnInsert );
                    pPkgFolder->setName( sTemp );
                    pPkgFolder->doSetParent( pCurrent, sal_True );
                    pCurrent = pPkgFolder;
                }
                else
                    pCurrent = pCurrent->doGetByName(sTemp).pFolder;
                nOldIndex = nIndex+1;
            }
            if ( nStreamIndex != -1 && sDirName.getLength() )
                aRecent [ sDirName ] = pCurrent;
        }
        if ( rName.getLength() -1 != nStreamIndex )
        {
            nStreamIndex++;
            sTemp = rName.copy( nStreamIndex, rName.getLength() - nStreamIndex);
            pPkgStream = new ZipPackageStream( *this, xFactory, m_bAllowRemoveOnInsert );
            pPkgStream->SetPackageMember( sal_True );
            pPkgStream->setZipEntryOnLoading( rEntry );
            pPkgStream->setName( sTemp );
            pPkgStream->doSetParent( pCurrent, sal_True );
        }
    }

    if ( m_nFormat == PACKAGE_FORMAT )
        parseManifest();
    else if ( m_nFormat == OFOPXML_FORMAT )
        parseContentType();
}

// XInitialization
void SAL_CALL ZipPackage::initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException)
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
            if ( (aArguments[ind] >>= aParamUrl))
            {
                eMode = e_IMode_URL;
                try
                {
                    sal_Int32 nParam = aParamUrl.indexOf( '?' );
                    if ( nParam >= 0 )
                    {
                        sURL = aParamUrl.copy( 0, nParam );
                        OUString aParam = aParamUrl.copy( nParam + 1 );

                          sal_Int32 nIndex = 0;
                        do
                        {
                            ::rtl::OUString aCommand = aParam.getToken( 0, '&', nIndex );
                            if ( aCommand.equals( OUString::createFromAscii( "repairpackage" ) ) )
                            {
                                bForceRecovery = sal_True;
                                break;
                            }
                            else if ( aCommand.equals( OUString::createFromAscii( "purezip" ) ) )
                            {
                                m_nFormat = ZIP_FORMAT;
                                pRootFolder->setPackageFormat_Impl( m_nFormat );
                                break;
                            }
                            else if ( aCommand.equals( OUString::createFromAscii( "ofopxml" ) ) )
                            {
                                m_nFormat = OFOPXML_FORMAT;
                                pRootFolder->setPackageFormat_Impl( m_nFormat );
                                break;
                            }
                        }
                        while ( nIndex >= 0 );
                    }
                    else
                        sURL = aParamUrl;

                    Content aContent ( sURL, uno::Reference < XCommandEnvironment >() );
                    Any aAny = aContent.getPropertyValue( OUString::createFromAscii( "Size" ) );
                    sal_uInt64 aSize = 0;
                    // kind of optimisation: treat empty files as nonexistent files
                    // and write to such files directly. Note that "Size" property is optional.
                    bool bHasSizeProperty = aAny >>= aSize;
                    if( !bHasSizeProperty || ( bHasSizeProperty && aSize ) )
                    {
                        uno::Reference < XActiveDataSink > xSink = new ZipPackageSink;
                        if (aContent.openStream ( xSink ) )
                            xContentStream = xSink->getInputStream();
                    }
                    else
                        bHaveZipFile = sal_False;
                }
                catch (com::sun::star::uno::Exception&)
                {
                    // Exception derived from uno::Exception thrown. This probably
                    // means the file doesn't exist...we'll create it at
                    // commitChanges time
                    bHaveZipFile = sal_False;
                }
            }
            else if ( (aArguments[ind] >>= xStream ) )
            {
                // a writable stream can implement both XStream & XInputStream
                eMode = e_IMode_XStream;
                xContentStream = xStream->getInputStream();
            }
            else if ( (aArguments[ind] >>= xContentStream) )
            {
                eMode = e_IMode_XInputStream;
            }
            else if ( ( aArguments[ind] >>= aNamedValue ) )
            {
                if ( aNamedValue.Name.equalsAscii( "RepairPackage" ) )
                    aNamedValue.Value >>= bForceRecovery;
                else if ( aNamedValue.Name.equalsAscii( "PackageFormat" ) )
                {
                    // setting this argument to true means Package format
                    // setting it to false means plain Zip format

                    sal_Bool bPackFormat = sal_True;
                    aNamedValue.Value >>= bPackFormat;
                    if ( !bPackFormat )
                        m_nFormat = ZIP_FORMAT;

                    pRootFolder->setPackageFormat_Impl( m_nFormat );
                }
                else if ( aNamedValue.Name.equalsAscii( "StorageFormat" ) )
                {
                    ::rtl::OUString aFormatName;
                    aNamedValue.Value >>= aFormatName;
                    if ( aFormatName.equalsAscii( "PackageFormat" ) )
                        m_nFormat = PACKAGE_FORMAT;
                    else if ( aFormatName.equalsAscii( "ZipFormat" ) )
                        m_nFormat = ZIP_FORMAT;
                    else if ( aFormatName.equalsAscii( "OFOPXMLFormat" ) )
                        m_nFormat = OFOPXML_FORMAT;
                    else
                        throw lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >(), 1 );

                    pRootFolder->setPackageFormat_Impl( m_nFormat );
                }
                else if ( aNamedValue.Name.equalsAscii( "AllowRemoveOnInsert" ) )
                {
                    aNamedValue.Value >>= m_bAllowRemoveOnInsert;
                    pRootFolder->setRemoveOnInsertMode_Impl( m_bAllowRemoveOnInsert );
                }

                // for now the progress handler is not used, probably it will never be
                // if ( aNamedValue.Name.equalsAscii( "ProgressHandler" )
            }
            else
            {
                // The URL is not acceptable
                throw com::sun::star::uno::Exception ( OUString( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "Bad arguments." ) ),
                    static_cast < ::cppu::OWeakObject * > ( this ) );
            }
        }

        try
        {
            if (xContentStream.is())
            {
                // the stream must be seekable, if it is not it will be wrapped
                xContentStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( xContentStream, xFactory );
                xContentSeek = uno::Reference < XSeekable > ( xContentStream, UNO_QUERY );
                if ( ! xContentSeek.is() )
                    throw com::sun::star::uno::Exception ( OUString( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "The package component _requires_ an XSeekable interface!" ) ),
                            static_cast < ::cppu::OWeakObject * > ( this ) );

                if ( !xContentSeek->getLength() )
                    bHaveZipFile = sal_False;
            }
            else
                bHaveZipFile = sal_False;
        }
        catch (com::sun::star::uno::Exception&)
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
                pZipFile = new ZipFile ( xContentStream, xFactory, sal_True, bForceRecovery, xProgressHandler );
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
                if( pZipFile ) { delete pZipFile; pZipFile = NULL; }
                throw;
            }

            if ( bBadZipFile )
            {
                // clean up the memory, and tell the UCB about the error
                if( pZipFile ) { delete pZipFile; pZipFile = NULL; }

                throw com::sun::star::packages::zip::ZipIOException (
                    OUString( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "Bad Zip File." ) ),
                    static_cast < ::cppu::OWeakObject * > ( this ) );
            }
        }
    }

    RTL_LOGFILE_TRACE_AUTHOR ( "package", LOGFILE_AUTHOR, "} ZipPackage::initialize" );
}

Any SAL_CALL ZipPackage::getByHierarchicalName( const OUString& aName )
        throw(NoSuchElementException, RuntimeException)
{
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nIndex, nStreamIndex;
    FolderHash::iterator aIter;

    if ( (nIndex = aName.getLength() ) == 1 && *aName.getStr() == '/' )
        return makeAny ( uno::Reference < XUnoTunnel > (pRootFolder) );
    else
    {
        nStreamIndex = aName.lastIndexOf ( '/' );
        bool bFolder = nStreamIndex == nIndex-1;
        if ( nStreamIndex != -1 )
        {
            sDirName = aName.copy ( 0, nStreamIndex);
            aIter = aRecent.find ( sDirName );
            if ( aIter != aRecent.end() )
            {
                if ( bFolder )
                {
                    sal_Int32 nDirIndex = aName.lastIndexOf ( '/', nStreamIndex );
                    sTemp = aName.copy ( nDirIndex == -1 ? 0 : nDirIndex+1, nStreamIndex-nDirIndex-1 );
                    if ( sTemp == (*aIter).second->getName() )
                        return makeAny ( uno::Reference < XUnoTunnel > ( (*aIter).second ) );
                    else
                        aRecent.erase ( aIter );
                }
                else
                {
                    sTemp = aName.copy ( nStreamIndex + 1 );
                    if ( (*aIter).second->hasByName( sTemp ) )
                        return (*aIter).second->getByName( sTemp );
                    else
                        aRecent.erase( aIter );
                }
            }
        }
        else
        {
            if ( pRootFolder->hasByName ( aName ) )
                return pRootFolder->getByName ( aName );
        }
        nOldIndex = 0;
        ZipPackageFolder * pCurrent = pRootFolder;
        ZipPackageFolder * pPrevious = NULL;
        while ( ( nIndex = aName.indexOf('/', nOldIndex)) != -1)
        {
            sTemp = aName.copy (nOldIndex, nIndex - nOldIndex);
            if ( nIndex == nOldIndex )
                break;
            if ( pCurrent->hasByName( sTemp ) )
            {
                pPrevious = pCurrent;
                pCurrent = pCurrent->doGetByName(sTemp).pFolder;
            }
            else
                throw NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
            nOldIndex = nIndex+1;
        }
        if ( bFolder )
        {
            if (nStreamIndex != -1 )
                aRecent[sDirName] = pPrevious;
            return makeAny ( uno::Reference < XUnoTunnel > ( pCurrent ) );
        }
        else
        {
            sTemp = aName.copy( nOldIndex, aName.getLength() - nOldIndex);
            if ( pCurrent->hasByName ( sTemp ) )
            {
                if (nStreamIndex != -1 )
                    aRecent[sDirName] = pCurrent;
                return pCurrent->getByName( sTemp );
            }
            else
                throw NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
        }
    }
}

sal_Bool SAL_CALL ZipPackage::hasByHierarchicalName( const OUString& aName )
        throw(RuntimeException)
{
    OUString sTemp, sDirName;
    sal_Int32 nOldIndex, nIndex, nStreamIndex;
    FolderHash::iterator aIter;

    if ( (nIndex = aName.getLength() ) == 1 && *aName.getStr() == '/' )
        return sal_True;
    else
    {
        nStreamIndex = aName.lastIndexOf ( '/' );
        bool bFolder = nStreamIndex == nIndex-1;
        if ( nStreamIndex != -1 )
        {
            sDirName = aName.copy ( 0, nStreamIndex);
            aIter = aRecent.find ( sDirName );
            if ( aIter != aRecent.end() )
            {
                if ( bFolder )
                {
                    sal_Int32 nDirIndex = aName.lastIndexOf ( '/', nStreamIndex );
                    sTemp = aName.copy ( nDirIndex == -1 ? 0 : nDirIndex+1, nStreamIndex-nDirIndex-1 );
                    if ( sTemp == (*aIter).second->getName() )
                        return sal_True;
                    else
                        aRecent.erase ( aIter );
                }
                else
                {
                    sTemp = aName.copy ( nStreamIndex + 1 );
                    if ( (*aIter).second->hasByName( sTemp ) )
                        return sal_True;
                    else
                        aRecent.erase( aIter );
                }
            }
        }
        else
        {
            if ( pRootFolder->hasByName ( aName ) )
                return sal_True;
        }
        ZipPackageFolder * pCurrent = pRootFolder;
        ZipPackageFolder * pPrevious = NULL;
        nOldIndex = 0;
        while ( ( nIndex = aName.indexOf('/', nOldIndex)) != -1)
        {
            sTemp = aName.copy (nOldIndex, nIndex - nOldIndex);
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
            aRecent[sDirName] = pPrevious;
            return sal_True;
        }
        else
        {
            sTemp = aName.copy( nOldIndex, aName.getLength() - nOldIndex);

            if ( pCurrent->hasByName( sTemp ) )
            {
                aRecent[sDirName] = pCurrent;
                return sal_True;
            }
        }
        return sal_False;
    }
}

// XSingleServiceFactory
uno::Reference< XInterface > SAL_CALL ZipPackage::createInstance(  )
        throw(Exception, RuntimeException)
{
    uno::Reference < XInterface > xRef = *(new ZipPackageStream ( *this, xFactory, m_bAllowRemoveOnInsert ));
    return xRef;
}
uno::Reference< XInterface > SAL_CALL ZipPackage::createInstanceWithArguments( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException)
{
    sal_Bool bArg = sal_False;
    uno::Reference < XInterface > xRef;
    if ( aArguments.getLength() )
        aArguments[0] >>= bArg;
    if (bArg)
        xRef = *new ZipPackageFolder ( xFactory, m_nFormat, m_bAllowRemoveOnInsert );
    else
        xRef = *new ZipPackageStream ( *this, xFactory, m_bAllowRemoveOnInsert );

    return xRef;
}

void ZipPackage::WriteMimetypeMagicFile( ZipOutputStream& aZipOut )
{
    const OUString sMime ( RTL_CONSTASCII_USTRINGPARAM ( "mimetype" ) );
    if (xRootFolder->hasByName( sMime ) )
        xRootFolder->removeByName( sMime );

    ZipEntry * pEntry = new ZipEntry;
    sal_Int32 nBufferLength = pRootFolder->GetMediaType( ).getLength();
    OString sMediaType = OUStringToOString( pRootFolder->GetMediaType(), RTL_TEXTENCODING_ASCII_US );
    Sequence< sal_Int8 > aType( (sal_Int8*)sMediaType.getStr(),
                                nBufferLength );


    pEntry->sName = sMime;
    pEntry->nMethod = STORED;
    pEntry->nSize = pEntry->nCompressedSize = nBufferLength;
    pEntry->nTime = ZipOutputStream::getCurrentDosTime();

    CRC32 aCRC32;
    aCRC32.update( aType );
    pEntry->nCrc = aCRC32.getValue();

    try
    {
        vos::ORef < EncryptionData > xEmpty;
        aZipOut.putNextEntry( *pEntry, xEmpty );
        aZipOut.write( aType, 0, nBufferLength );
        aZipOut.closeEntry();
    }
    catch ( ::com::sun::star::io::IOException & r )
    {
        VOS_ENSURE( 0, "Error adding mimetype to the ZipOutputStream" );
        throw WrappedTargetException(
                OUString( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "Error adding mimetype to the ZipOutputStream!" ) ),
                static_cast < OWeakObject * > ( this ),
                makeAny( r ) );
    }
}

sal_Bool ZipPackage::writeFileIsTemp()
{
    // In case the target local file does not exist or empty
    // write directly to it otherwize create a temporary file to write to

    sal_Bool bUseTemp = sal_True;
    uno::Reference < XOutputStream > xTempOut;
    uno::Reference< XActiveDataStreamer > xSink;

    if ( eMode == e_IMode_URL && !pZipFile && isLocalFile_Impl( sURL ) )
    {
        xSink = openOriginalForOutput();
        if( xSink.is() )
        {
            uno::Reference< XStream > xStr = xSink->getStream();
            if( xStr.is() )
            {
                xTempOut = xStr->getOutputStream();
                if( xTempOut.is() )
                    bUseTemp = sal_False;
            }
        }
    }
    else if ( eMode == e_IMode_XStream && !pZipFile )
    {
        // write directly to an empty stream
        xTempOut = xStream->getOutputStream();
        if( xTempOut.is() )
            bUseTemp = sal_False;
    }

    if( bUseTemp )
    {
        // create temporary file
        const OUString sServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.io.TempFile" ) );
        xTempOut = uno::Reference < XOutputStream > ( xFactory->createInstance ( sServiceName ), UNO_QUERY );
    }

    // Hand it to the ZipOutputStream:
    ZipOutputStream aZipOut ( xTempOut );
    aZipOut.setMethod(DEFLATED);
    aZipOut.setLevel(DEFAULT_COMPRESSION);

    try
    {
        if ( m_nFormat == PACKAGE_FORMAT )
        {
            // Remove the old manifest.xml file as the
            // manifest will be re-generated and the
            // META-INF directory implicitly created if does not exist
            const OUString sMeta ( RTL_CONSTASCII_USTRINGPARAM ( "META-INF" ) );

            if ( xRootFolder->hasByName( sMeta ) )
            {
                const OUString sManifest (RTL_CONSTASCII_USTRINGPARAM( "manifest.xml") );

                uno::Reference< XUnoTunnel > xTunnel;
                Any aAny = xRootFolder->getByName( sMeta );
                aAny >>= xTunnel;
                uno::Reference< XNameContainer > xMetaInfFolder( xTunnel, UNO_QUERY );
                if ( xMetaInfFolder.is() && xMetaInfFolder->hasByName( sManifest ) )
                    xMetaInfFolder->removeByName( sManifest );
            }

            // Write a magic file with mimetype
            WriteMimetypeMagicFile( aZipOut );
        }
        if ( m_nFormat == OFOPXML_FORMAT )
        {
            // Remove the old [Content_Types].xml file as the
            // file will be re-generated

            const ::rtl::OUString aContentTypes( RTL_CONSTASCII_USTRINGPARAM ( "[Content_Types].xml" ) );

            if ( xRootFolder->hasByName( aContentTypes ) )
                xRootFolder->removeByName( aContentTypes );
        }

        // Create a vector to store data for the manifest.xml file
        vector < Sequence < PropertyValue > > aManList;

        // Make a reference to the manifest output stream so it persists
        // until the call to ZipOutputStream->finish()
        uno::Reference < XOutputStream > xManOutStream;
        const OUString sMediaType ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
        const OUString sVersion ( RTL_CONSTASCII_USTRINGPARAM ( "Version" ) );
        const OUString sFullPath ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );

        if ( m_nFormat == PACKAGE_FORMAT )
        {
            Sequence < PropertyValue > aPropSeq ( PKG_SIZE_NOENCR_MNFST );
            aPropSeq [PKG_MNFST_MEDIATYPE].Name = sMediaType;
            aPropSeq [PKG_MNFST_MEDIATYPE].Value <<= pRootFolder->GetMediaType( );
            aPropSeq [PKG_MNFST_VERSION].Name = sVersion;
            aPropSeq [PKG_MNFST_VERSION].Value <<= pRootFolder->GetVersion( );
            aPropSeq [PKG_MNFST_FULLPATH].Name = sFullPath;
            aPropSeq [PKG_MNFST_FULLPATH].Value <<= OUString ( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );

            aManList.push_back( aPropSeq );
        }

        // Get a random number generator and seed it with current timestamp
        // This will be used to generate random salt and initialisation vectors
        // for encrypted streams
        TimeValue aTime;
        osl_getSystemTime( &aTime );
        rtlRandomPool aRandomPool = rtl_random_createPool ();
        rtl_random_addBytes ( aRandomPool, &aTime, 8 );


        // call saveContents (it will recursively save sub-directories
        OUString aEmptyString;
        pRootFolder->saveContents( aEmptyString, aManList, aZipOut, aEncryptionKey, aRandomPool );

        // Clean up random pool memory
        rtl_random_destroyPool ( aRandomPool );

        if( bUseManifest && m_nFormat == PACKAGE_FORMAT )
        {
            // Write the manifest
            OUString sManifestWriter( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.ManifestWriter" ) );
            uno::Reference < XManifestWriter > xWriter ( xFactory->createInstance( sManifestWriter ), UNO_QUERY );
            if ( xWriter.is() )
            {
                ZipEntry * pEntry = new ZipEntry;
                ZipPackageBuffer *pBuffer = new ZipPackageBuffer( n_ConstBufferSize );
                xManOutStream = uno::Reference < XOutputStream > (*pBuffer, UNO_QUERY);

                pEntry->sName = OUString( RTL_CONSTASCII_USTRINGPARAM ( "META-INF/manifest.xml") );
                pEntry->nMethod = DEFLATED;
                pEntry->nCrc = pEntry->nSize = pEntry->nCompressedSize = -1;
                pEntry->nTime = ZipOutputStream::getCurrentDosTime();

                // Convert vector into a Sequence
                Sequence < Sequence < PropertyValue > > aManifestSequence ( aManList.size() );
                Sequence < PropertyValue > * pSequence = aManifestSequence.getArray();
                for (vector < Sequence < PropertyValue > >::const_iterator aIter = aManList.begin(), aEnd = aManList.end();
                     aIter != aEnd;
                     aIter++, pSequence++)
                    *pSequence= (*aIter);
                xWriter->writeManifestSequence ( xManOutStream,  aManifestSequence );

                sal_Int32 nBufferLength = static_cast < sal_Int32 > ( pBuffer->getPosition() );
                pBuffer->realloc( nBufferLength );

                // the manifest.xml is never encrypted - so pass an empty reference
                vos::ORef < EncryptionData > xEmpty;
                aZipOut.putNextEntry( *pEntry, xEmpty );
                aZipOut.write( pBuffer->getSequence(), 0, nBufferLength );
                aZipOut.closeEntry();
            }
            else
            {
                VOS_ENSURE ( 0, "Couldn't get a ManifestWriter!" );
                IOException aException;
                throw WrappedTargetException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "Couldn't get a ManifestWriter!" ) ),
                        static_cast < OWeakObject * > ( this ),
                        makeAny( aException ) );
            }
        }
        else if( m_nFormat == OFOPXML_FORMAT )
        {
            ZipEntry* pEntry = new ZipEntry;
            ZipPackageBuffer *pBuffer = new ZipPackageBuffer( n_ConstBufferSize );
            uno::Reference< io::XOutputStream > xConTypeOutStream( *pBuffer, UNO_QUERY );

            pEntry->sName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "[Content_Types].xml") );
            pEntry->nMethod = DEFLATED;
            pEntry->nCrc = pEntry->nSize = pEntry->nCompressedSize = -1;
            pEntry->nTime = ZipOutputStream::getCurrentDosTime();

            // Convert vector into a Sequence
            // TODO/LATER: use Defaulst entries in future
            uno::Sequence< beans::StringPair > aDefaultsSequence;
            uno::Sequence< beans::StringPair > aOverridesSequence( aManList.size() );
            sal_Int32 nSeqLength = 0;
            for ( vector< uno::Sequence< beans::PropertyValue > >::const_iterator aIter = aManList.begin(),
                    aEnd = aManList.end();
                 aIter != aEnd;
                 aIter++)
            {
                ::rtl::OUString aPath;
                ::rtl::OUString aType;
                OSL_ENSURE( (*aIter)[PKG_MNFST_MEDIATYPE].Name.equals( sMediaType ) && (*aIter)[PKG_MNFST_FULLPATH].Name.equals( sFullPath ),
                            "The mediatype sequence format is wrong!\n" );
                (*aIter)[PKG_MNFST_MEDIATYPE].Value >>= aType;
                if ( aType.getLength() )
                {
                    // only nonempty type makes sence here
                    nSeqLength++;
                    (*aIter)[PKG_MNFST_FULLPATH].Value >>= aPath;
                    aOverridesSequence[nSeqLength-1].First = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) + aPath;
                    aOverridesSequence[nSeqLength-1].Second = aType;
                }
            }
            aOverridesSequence.realloc( nSeqLength );

            ::comphelper::OFOPXMLHelper::WriteContentSequence(
                    xConTypeOutStream, aDefaultsSequence, aOverridesSequence, xFactory );

            sal_Int32 nBufferLength = static_cast < sal_Int32 > ( pBuffer->getPosition() );
            pBuffer->realloc( nBufferLength );

            // there is no encryption in this format currently
            vos::ORef < EncryptionData > xEmpty;
            aZipOut.putNextEntry( *pEntry, xEmpty );
            aZipOut.write( pBuffer->getSequence(), 0, nBufferLength );
            aZipOut.closeEntry();
        }

        aZipOut.finish();

        // Update our References to point to the new temp file
        if( bUseTemp )
        {
            xContentStream = uno::Reference < XInputStream > ( xTempOut, UNO_QUERY_THROW );
            xContentSeek = uno::Reference < XSeekable > ( xTempOut, UNO_QUERY_THROW );
        }
        else
        {
            // the case when the original contents were written directly
            xTempOut->flush();

            // in case the stream is based on a file it will implement the following interface
            // the call should be used to be sure that the contents are written to the file system
            uno::Reference< io::XAsyncOutputMonitor > asyncOutputMonitor( xTempOut, uno::UNO_QUERY );
            if ( asyncOutputMonitor.is() )
                asyncOutputMonitor->waitForCompletion();

            if ( eMode == e_IMode_URL )
                xContentStream = xSink->getStream()->getInputStream();
            else if ( eMode == e_IMode_XStream )
                xContentStream = xStream->getInputStream();

            xContentSeek = uno::Reference < XSeekable > ( xContentStream, UNO_QUERY_THROW );

            OSL_ENSURE( xContentStream.is() && xContentSeek.is(), "XSeekable interface is required!" );
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
                    OUString( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "Problem writing the original content!" ) ),
                    static_cast < OWeakObject * > ( this ),
                    aCaught );
        }
        else
        {
            // the document is written directly, although it was empty it is important to notify that the writing has failed
            // TODO/LATER: let the package be able to recover in this situation
            ::rtl::OUString aErrTxt( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "This package is unusable!" ) );
            embed::UseBackupException aException( aErrTxt, uno::Reference< uno::XInterface >(), ::rtl::OUString() );
            throw WrappedTargetException( aErrTxt,
                                            static_cast < OWeakObject * > ( this ),
                                            makeAny ( aException ) );
        }
    }

    // seek back to the beginning of the temp file so we can read segments from it
    xContentSeek->seek ( 0 );
    if ( pZipFile )
        pZipFile->setInputStream ( xContentStream );
    else
        pZipFile = new ZipFile ( xContentStream, xFactory, sal_False );

    return bUseTemp;
}

uno::Reference< XActiveDataStreamer > ZipPackage::openOriginalForOutput()
{
    // open and truncate the original file
    Content aOriginalContent (sURL, uno::Reference < XCommandEnvironment >() );
    uno::Reference< XActiveDataStreamer > xSink = new ActiveDataStreamer;

    if ( eMode == e_IMode_URL )
    {
        try
        {
            sal_Bool bTruncSuccess = sal_False;

            try
            {
                Exception aDetect;
                sal_Int64 aSize = 0;
                Any aAny = aOriginalContent.setPropertyValue( OUString::createFromAscii( "Size" ), makeAny( aSize ) );
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
               aArg.Properties = Sequence< Property >( 0 ); // unused

            aOriginalContent.executeCommand( OUString::createFromAscii( "open" ), makeAny( aArg ) );
        }
        catch( Exception& )
        {
            // seems to be nonlocal file
            // temporary file mechanics should be used
        }
    }

    return xSink;
}

// XChangesBatch
void SAL_CALL ZipPackage::commitChanges(  )
        throw(WrappedTargetException, RuntimeException)
{
    if ( eMode == e_IMode_XInputStream )
    {
        IOException aException;
        throw WrappedTargetException( OUString( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "This package is read only!" ) ),
                static_cast < OWeakObject * > ( this ), makeAny ( aException ) );
    }

    RTL_LOGFILE_TRACE_AUTHOR ( "package", LOGFILE_AUTHOR, "{ ZipPackage::commitChanges" );
    // First we write the entire package to a temporary file. After writeTempFile,
    // xContentSeek and xContentStream will reference the new temporary file.
    // Exception - empty or nonexistent local file that is written directly

    if ( writeFileIsTemp() )
    {
        if ( eMode == e_IMode_XStream )
        {
            // First truncate our output stream
            uno::Reference < XOutputStream > xOutputStream;

            // preparation for copy step
            try
            {
                xContentSeek->seek( 0 );

                xOutputStream = xStream->getOutputStream();
                uno::Reference < XTruncate > xTruncate ( xOutputStream, UNO_QUERY );
                if ( !xTruncate.is() )
                    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

                // after successful truncation the original file contents are already lost
                xTruncate->truncate();
            }
            catch( uno::Exception& r )
            {
                throw WrappedTargetException( OUString( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "This package is read only!" ) ),
                        static_cast < OWeakObject * > ( this ), makeAny ( r ) );
            }

            try
            {
                // then copy the contents of the tempfile to our output stream
                ::comphelper::OStorageHelper::CopyInputToOutput( xContentStream, xOutputStream );
                xOutputStream->flush();
                uno::Reference< io::XAsyncOutputMonitor > asyncOutputMonitor(
                    xOutputStream, uno::UNO_QUERY);
                if (asyncOutputMonitor.is()) {
                    asyncOutputMonitor->waitForCompletion();
                }
            }
            catch( uno::Exception& )
            {
                // if anything goes wrong in this block the target file becomes corrupted
                // so an exception should be thrown as a notification about it
                // and the package must disconnect from the stream
                DisconnectFromTargetAndThrowException_Impl( xContentStream );
            }
        }
        else if ( eMode == e_IMode_URL )
        {
            uno::Reference< XOutputStream > aOrigFileStream;
            sal_Bool bCanBeCorrupted = sal_False;

            if( isLocalFile_Impl( sURL ) )
            {
                // write directly in case of local file
                uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSimpleAccess(
                    xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                    uno::UNO_QUERY );
                OSL_ENSURE( xSimpleAccess.is(), "Can't instatiate SimpleFileAccess service!\n" );
                uno::Reference< io::XTruncate > xOrigTruncate;
                if ( xSimpleAccess.is() )
                {
                    try
                    {
                        aOrigFileStream = xSimpleAccess->openFileWrite( sURL );
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
                        ::comphelper::OStorageHelper::CopyInputToOutput( xContentStream, aOrigFileStream );
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
                    uno::Reference < XPropertySet > xPropSet ( xContentStream, UNO_QUERY );
                    OSL_ENSURE( xPropSet.is(), "This is a temporary file that must implement XPropertySet!\n" );
                    if ( !xPropSet.is() )
                        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

                    OUString sTargetFolder = sURL.copy ( 0, sURL.lastIndexOf ( static_cast < sal_Unicode > ( '/' ) ) );
                    Content aContent ( sTargetFolder, uno::Reference < XCommandEnvironment > () );

                    OUString sTempURL;
                    Any aAny = xPropSet->getPropertyValue ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Uri" ) ) );
                    aAny >>= sTempURL;

                    TransferInfo aInfo;
                    aInfo.NameClash = NameClash::OVERWRITE;
                    aInfo.MoveData = sal_False;
                    aInfo.SourceURL = sTempURL;
                    aInfo.NewTitle = rtl::Uri::decode ( sURL.copy ( 1 + sURL.lastIndexOf ( static_cast < sal_Unicode > ( '/' ) ) ),
                                                        rtl_UriDecodeWithCharset,
                                                        RTL_TEXTENCODING_UTF8 );
                    aAny <<= aInfo;

                    // if the file is still not corrupted, it can become after the next step
                    aContent.executeCommand ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "transfer" ) ), aAny );
                }
                catch (::com::sun::star::uno::Exception& r)
                {
                    if ( bCanBeCorrupted )
                        DisconnectFromTargetAndThrowException_Impl( xContentStream );

                    throw WrappedTargetException(
                                                OUString( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "This package may be read only!" ) ),
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

void ZipPackage::DisconnectFromTargetAndThrowException_Impl( const uno::Reference< io::XInputStream >& xTempStream )
{
    xStream = uno::Reference< io::XStream >( xTempStream, uno::UNO_QUERY );
    if ( xStream.is() )
        eMode = e_IMode_XStream;
    else
        eMode = e_IMode_XInputStream;

    ::rtl::OUString aTempURL;
    try {
        uno::Reference< beans::XPropertySet > xTempFile( xContentStream, uno::UNO_QUERY_THROW );
        uno::Any aUrl = xTempFile->getPropertyValue( ::rtl::OUString::createFromAscii( "Uri" ) );
        aUrl >>= aTempURL;
        xTempFile->setPropertyValue( ::rtl::OUString::createFromAscii( "RemoveFile" ),
                                     uno::makeAny( sal_False ) );
    }
    catch ( uno::Exception& )
    {
        OSL_ENSURE( sal_False, "These calls are pretty simple, they should not fail!\n" );
    }

    ::rtl::OUString aErrTxt( RTL_CONSTASCII_USTRINGPARAM ( OSL_LOG_PREFIX "This package is read only!" ) );
    embed::UseBackupException aException( aErrTxt, uno::Reference< uno::XInterface >(), aTempURL );
    throw WrappedTargetException( aErrTxt,
                                    static_cast < OWeakObject * > ( this ),
                                    makeAny ( aException ) );
}

sal_Bool SAL_CALL ZipPackage::hasPendingChanges(  )
        throw(RuntimeException)
{
    return sal_False;
}
Sequence< ElementChange > SAL_CALL ZipPackage::getPendingChanges(  )
        throw(RuntimeException)
{
    return Sequence < ElementChange > ();
}

/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
uno::Reference < XInterface >SAL_CALL ZipPackage_createInstance(
    const uno::Reference< XMultiServiceFactory > & xMgr )
{
    return uno::Reference< XInterface >( *new ZipPackage(xMgr) );
}

OUString ZipPackage::static_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.comp.ZipPackage" ) );
}

Sequence< OUString > ZipPackage::static_getSupportedServiceNames()
{
    Sequence< OUString > aNames(1);
    aNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.Package" ) );
    return aNames;
}
sal_Bool SAL_CALL ZipPackage::static_supportsService( OUString const & rServiceName )
{
    return rServiceName == getSupportedServiceNames()[0];
}

OUString ZipPackage::getImplementationName()
    throw (RuntimeException)
{
    return static_getImplementationName();
}

Sequence< OUString > ZipPackage::getSupportedServiceNames()
    throw (RuntimeException)
{
    return static_getSupportedServiceNames();
}
sal_Bool SAL_CALL ZipPackage::supportsService( OUString const & rServiceName )
    throw (RuntimeException)
{
    return static_supportsService ( rServiceName );
}
uno::Reference < XSingleServiceFactory > ZipPackage::createServiceFactory( uno::Reference < XMultiServiceFactory > const & rServiceFactory )
{
    return cppu::createSingleFactory (rServiceFactory,
                                           static_getImplementationName(),
                                           ZipPackage_createInstance,
                                           static_getSupportedServiceNames());
}

// XUnoTunnel
Sequence< sal_Int8 > ZipPackage::getUnoTunnelImplementationId( void )
    throw (RuntimeException)
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

sal_Int64 SAL_CALL ZipPackage::getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw(RuntimeException)
{
    if (aIdentifier.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        return reinterpret_cast < sal_Int64 > ( this );
    return 0;
}

uno::Reference< XPropertySetInfo > SAL_CALL ZipPackage::getPropertySetInfo(  )
        throw(RuntimeException)
{
    return uno::Reference < XPropertySetInfo > ();
}
void SAL_CALL ZipPackage::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    if ( m_nFormat != PACKAGE_FORMAT )
        throw UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("HasEncryptedEntries") )
      ||aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MediaTypeFallbackUsed") ) )
        throw PropertyVetoException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("EncryptionKey") ) )
    {
        if (!( aValue >>= aEncryptionKey ) )
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >(), 2 );
    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("UseManifest") ) )
    {
        if (!( aValue >>= bUseManifest ) )
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >(), 2 );
    }
    else
        throw UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}
Any SAL_CALL ZipPackage::getPropertyValue( const OUString& PropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    // TODO/LATER: Activate the check when zip-ucp is ready
    // if ( m_nFormat != PACKAGE_FORMAT )
    //  throw UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

    Any aAny;
    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "EncryptionKey" ) ) )
    {
        aAny <<= aEncryptionKey;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "HasEncryptedEntries" ) ) )
    {
        aAny <<= bHasEncryptedEntries;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "UseManifest" ) ) )
    {
        aAny <<= bUseManifest;
        return aAny;
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "MediaTypeFallbackUsed" ) ) )
    {
        aAny <<= m_bMediaTypeFallbackUsed;
        return aAny;
    }
    throw UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}
void SAL_CALL ZipPackage::addPropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackage::removePropertyChangeListener( const OUString& /*aPropertyName*/, const uno::Reference< XPropertyChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackage::addVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackage::removeVetoableChangeListener( const OUString& /*PropertyName*/, const uno::Reference< XVetoableChangeListener >& /*aListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
