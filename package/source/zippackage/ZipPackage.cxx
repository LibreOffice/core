/*************************************************************************
 *
 *  $RCSfile: ZipPackage.cxx,v $
 *
 *  $Revision: 1.92 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 11:54:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _ZIP_PACKAGE_HXX
#include <ZipPackage.hxx>
#endif
#ifndef _ZIP_PACKAGE_SINK_HXX
#include <ZipPackageSink.hxx>
#endif
#ifndef _ZIP_ENUMERATION_HXX
#include <ZipEnumeration.hxx>
#endif
#ifndef _ZIP_PACKAGE_STREAM_HXX
#include <ZipPackageStream.hxx>
#endif
#ifndef _ZIP_PACKAGE_FOLDER_HXX
#include <ZipPackageFolder.hxx>
#endif
#ifndef _ZIP_OUTPUT_STREAM_HXX
#include <ZipOutputStream.hxx>
#endif
#ifndef _ZIP_PACKAGE_BUFFER_HXX
#include <ZipPackageBuffer.hxx>
#endif
#ifndef _ZIP_FILE_HXX
#include <ZipFile.hxx>
#endif
#ifndef _PACKAGE_CONSTANTS_HXX_
#include <PackageConstants.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_MANIFEST_XMANIFESTREADER_HPP_
#include <com/sun/star/packages/manifest/XManifestReader.hpp>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_MANIFEST_XMANIFESTWRITER_HPP_
#include <com/sun/star/packages/manifest/XManifestWriter.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_IOERRORCODE_HPP_
#include <com/sun/star/ucb/IOErrorCode.hpp>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _COM_SUN_STAR_UCB_TRANSFERINFO_HPP_
#include <com/sun/star/ucb/TransferInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_NAMECLASH_HPP_
#include <com/sun/star/ucb/NameClash.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENCOMMANDARGUMENT2_HPP_
#include <com/sun/star/ucb/OpenCommandArgument2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_OPENMODE_HPP_
#include <com/sun/star/ucb/OpenMode.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPROGRESSHANDLER_HPP_
#include <com/sun/star/ucb/XProgressHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASTREAMER_HPP_
#include <com/sun/star/io/XActiveDataStreamer.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _CONTENT_INFO_HXX_
#include <ContentInfo.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _RTL_URI_HXX_
#include <rtl/uri.hxx>
#endif
#ifndef _RTL_RANDOM_H_
#include <rtl/random.h>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include <memory>
#include <vector>

#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/fileidentifierconverter.hxx>
#include <comphelper/seekableinput.hxx>

using namespace rtl;
using namespace ucb;
using namespace std;
using namespace osl;
using namespace cppu;
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
    ::ucb::ContentBroker* pBroker = ::ucb::ContentBroker::get();
    if ( !pBroker )
    {
        ::rtl::OUString aRet;
        if ( FileBase::getSystemPathFromFileURL( aURL, aRet ) == FileBase::E_None )
            aSystemPath = aRet;
    }
    else
    {
        Reference< XContentProviderManager > xManager =
                pBroker->getContentProviderManagerInterface();
        try
        {
               aSystemPath = ::ucb::getSystemPathFromFileURL( xManager, aURL );
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
    Reference< XStream > mStream;
public:

    virtual Reference< XStream > SAL_CALL getStream()
            throw( RuntimeException )
            { return mStream; }

    virtual void SAL_CALL setStream( const Reference< XStream >& stream )
            throw( RuntimeException )
            { mStream = stream; }
};

class DummyInputStream : public ::cppu::WeakImplHelper1< XInputStream >
{
    virtual sal_Int32 SAL_CALL readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        { return 0; }

    virtual sal_Int32 SAL_CALL readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
            throw ( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
        { return 0; }

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
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

void copyInputToOutput_Impl( Reference< XInputStream >& aIn, Reference< XOutputStream >& aOut )
{
    sal_Int32 nRead;
    Sequence < sal_Int8 > aSequence ( n_ConstBufferSize );

    do
    {
        nRead = aIn->readBytes ( aSequence, n_ConstBufferSize );
        if ( nRead < n_ConstBufferSize )
        {
            Sequence < sal_Int8 > aTempBuf ( aSequence.getConstArray(), nRead );
            aOut->writeBytes ( aTempBuf );
        }
        else
            aOut->writeBytes ( aSequence );
    }
    while ( nRead == n_ConstBufferSize );
}

ZipPackage::ZipPackage (const Reference < XMultiServiceFactory > &xNewFactory)
: pZipFile( NULL )
, pRootFolder( NULL )
, xFactory( xNewFactory )
, bHasEncryptedEntries ( sal_False )
, bUseManifest ( sal_True )
, m_bPackageFormat( sal_True )
, bForceRecovery ( sal_False )
, eMode ( e_IMode_None )
{
    xRootFolder = pRootFolder = new ZipPackageFolder( xFactory, m_bPackageFormat );
}

ZipPackage::~ZipPackage( void )
{
    delete pZipFile;
    // As all folders and streams contain references to their parents,
    // we must remove these references so that they will be deleted when
    // the hash_map of the root folder is cleared, releasing all subfolders
    // and substreams which in turn release theirs, etc. When xRootFolder is
    // released when this destructor completes, the folder tree should be
    // deleted fully (and automagically).

    pRootFolder->releaseUpwardRef();
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
                    pPkgFolder = new ZipPackageFolder( xFactory, m_bPackageFormat );
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
            pPkgStream = new ZipPackageStream( *this, xFactory );
            pPkgStream->SetPackageMember( sal_True );
            pPkgStream->setZipEntry( rEntry );
            pPkgStream->setName( sTemp );
            pPkgStream->doSetParent( pCurrent, sal_True );
        }
    }

    if ( m_bPackageFormat )
    {
        const OUString sMeta ( RTL_CONSTASCII_USTRINGPARAM ( "META-INF" ) );
        if ( xRootFolder->hasByName( sMeta ) )
        {
            const OUString sManifest (RTL_CONSTASCII_USTRINGPARAM( "manifest.xml") );

            try {
                Reference< XUnoTunnel > xTunnel;
                Any aAny = xRootFolder->getByName( sMeta );
                aAny >>= xTunnel;
                Reference< XNameContainer > xMetaInfFolder( xTunnel, UNO_QUERY );
                if ( xMetaInfFolder.is() && xMetaInfFolder->hasByName( sManifest ) )
                {
                    aAny = xMetaInfFolder->getByName( sManifest );
                    aAny >>= xTunnel;
                    Reference < XActiveDataSink > xSink (xTunnel, UNO_QUERY);
                    if (xSink.is())
                    {
                        OUString sManifestReader ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.ManifestReader" ) );
                        Reference < XManifestReader > xReader (xFactory->createInstance( sManifestReader ), UNO_QUERY );
                        if ( xReader.is() )
                        {
                            const OUString sPropFullPath ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
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
                                OUString sPath, sMediaType;
                                const PropertyValue *pValue = pSequence->getConstArray();
                                const Any *pSalt = NULL, *pVector = NULL, *pCount = NULL, *pSize = NULL, *pDigest = NULL;
                                for (sal_Int32 j = 0, nNum = pSequence->getLength(); j < nNum; j++ )
                                {
                                    if (pValue[j].Name.equals( sPropFullPath ) )
                                        pValue[j].Value >>= sPath;
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
                                    Any aAny = getByHierarchicalName( sPath );
                                    Reference < XUnoTunnel > xTunnel;
                                    aAny >>= xTunnel;
                                    sal_Int64 nTest=0;
                                    if ((nTest = xTunnel->getSomething(ZipPackageFolder::static_getImplementationId())) != 0)
                                    {
                                        pFolder = reinterpret_cast < ZipPackageFolder* > ( nTest );
                                        pFolder->SetMediaType ( sMediaType );
                                    }
                                    else
                                    {
                                        pStream = reinterpret_cast < ZipPackageStream* > ( xTunnel->getSomething(ZipPackageStream::static_getImplementationId()));
                                        pStream->SetMediaType ( sMediaType );

                                        if (pSalt && pVector && pCount && pSize)
                                        {
                                            Sequence < sal_uInt8 > aSequence;
                                            sal_Int32 nCount, nSize;
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

                                            pStream->SetToBeEncrypted ( sal_True );
                                            pStream->SetIsEncrypted ( sal_True );
                                            if ( !bHasEncryptedEntries && pStream->getName().compareToAscii ( "content.xml" ) == 0 )
                                                bHasEncryptedEntries = sal_True;
                                        }
                                    }
                                }
                            }
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
        xRootFolder->removeByName( sMimetype );
    }
}
// XInitialization
void SAL_CALL ZipPackage::initialize( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException)
{
    RTL_LOGFILE_TRACE_AUTHOR ( "package", LOGFILE_AUTHOR, "{ ZipPackage::initialize" );
    sal_Bool bBadZipFile = sal_False, bHaveZipFile = sal_True;
    Reference< XProgressHandler > xProgressHandler;
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
                                m_bPackageFormat = sal_False;
                                pRootFolder->setPackageFormat_Impl( m_bPackageFormat );
                                break;
                            }

                        }
                        while ( nIndex >= 0 );
                    }
                    else
                        sURL = aParamUrl;

                    Content aContent ( sURL, Reference < XCommandEnvironment >() );
                    Any aAny = aContent.getPropertyValue( OUString::createFromAscii( "Size" ) );
                    sal_uInt64 aSize;
                    // kind of optimisation: treat empty files as nonexistent files
                    // and write to such files directly
                    if( ( aAny >>= aSize ) && aSize )
                    {
                        Reference < XActiveDataSink > xSink = new ZipPackageSink;
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
                    aNamedValue.Value >>= m_bPackageFormat;
                    pRootFolder->setPackageFormat_Impl( m_bPackageFormat );
                }

                // for now the progress handler is not used, probably it will never be
                // if ( aNamedValue.Name.equalsAscii( "ProgressHandler" )
            }
            else
            {
                // The URL is not acceptable
                throw com::sun::star::uno::Exception ( OUString( RTL_CONSTASCII_USTRINGPARAM ( "Bad arguments." ) ),
                    static_cast < ::cppu::OWeakObject * > ( this ) );
            }
        }

        try
        {
            if (xContentStream.is())
            {
                // the stream must be seekable, if it is not it will be wrapped
                xContentStream = ::comphelper::OSeekableInputWrapper::CheckSeekableCanWrap( xContentStream, xFactory );
                xContentSeek = Reference < XSeekable > ( xContentStream, UNO_QUERY );
                if ( ! xContentSeek.is() )
                    throw com::sun::star::uno::Exception (
                            OUString( RTL_CONSTASCII_USTRINGPARAM (
                                    "The package component _requires_ an XSeekable interface!" ) ),
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
                throw com::sun::star::uno::Exception ( OUString( RTL_CONSTASCII_USTRINGPARAM ( "Bad Zip File." ) ),
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
        return makeAny ( Reference < XUnoTunnel > (pRootFolder) );
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
                        return makeAny ( Reference < XUnoTunnel > ( (*aIter).second ) );
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
                throw NoSuchElementException();
            nOldIndex = nIndex+1;
        }
        if ( bFolder )
        {
            if (nStreamIndex != -1 )
                aRecent[sDirName] = pPrevious;
            return makeAny ( Reference < XUnoTunnel > ( pCurrent ) );
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
                throw NoSuchElementException();
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
Reference< XInterface > SAL_CALL ZipPackage::createInstance(  )
        throw(Exception, RuntimeException)
{
    Reference < XInterface > xRef = *(new ZipPackageStream ( *this, xFactory ));
    return xRef;
}
Reference< XInterface > SAL_CALL ZipPackage::createInstanceWithArguments( const Sequence< Any >& aArguments )
        throw(Exception, RuntimeException)
{
    sal_Bool bArg = sal_False;
    Reference < XInterface > xRef;
    if ( aArguments.getLength() )
        aArguments[0] >>= bArg;
    if (bArg)
        xRef = *new ZipPackageFolder ( xFactory, m_bPackageFormat );
    else
        xRef = *new ZipPackageStream ( *this, xFactory );

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
                OUString( RTL_CONSTASCII_USTRINGPARAM ( "Error adding mimetype to the ZipOutputStream!" ) ),
                static_cast < OWeakObject * > ( this ),
                makeAny( r ) );
    }
}

sal_Bool ZipPackage::writeFileIsTemp()
{
    // In case the target local file does not exist or empty
    // write directly to it otherwize create a temporary file to write to

    sal_Bool aUseTemp = sal_True;
    Reference < XOutputStream > xTempOut;
    Reference< XActiveDataStreamer > xSink;

    if ( eMode == e_IMode_URL && !pZipFile
        && isLocalFile_Impl( sURL ) )
    {
        xSink = openOriginalForOutput();
        if( xSink.is() )
        {
            Reference< XStream > xStr = xSink->getStream();
            if( xStr.is() )
            {
                xTempOut = xStr->getOutputStream();
                if( xTempOut.is() )
                    aUseTemp = sal_False;
            }
        }
    }

    if( aUseTemp )
    {
        // create temporary file
        const OUString sServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.io.TempFile" ) );
        xTempOut = Reference < XOutputStream > ( xFactory->createInstance ( sServiceName ), UNO_QUERY );
    }

    // Hand it to the ZipOutputStream:
    ZipOutputStream aZipOut ( xTempOut );
    aZipOut.setMethod(DEFLATED);
    aZipOut.setLevel(DEFAULT_COMPRESSION);

    if ( m_bPackageFormat )
    {
        // Remove the old manifest.xml file as the
        // manifest will be re-generated and the
        // META-INF directory implicitly created if does not exist
        const OUString sMeta ( RTL_CONSTASCII_USTRINGPARAM ( "META-INF" ) );
        try {
            if ( xRootFolder->hasByName( sMeta ) )
            {
                const OUString sManifest (RTL_CONSTASCII_USTRINGPARAM( "manifest.xml") );

                Reference< XUnoTunnel > xTunnel;
                Any aAny = xRootFolder->getByName( sMeta );
                aAny >>= xTunnel;
                Reference< XNameContainer > xMetaInfFolder( xTunnel, UNO_QUERY );
                if ( xMetaInfFolder.is() && xMetaInfFolder->hasByName( sManifest ) )
                    xMetaInfFolder->removeByName( sManifest );
            }
        }
        catch (::com::sun::star::uno::RuntimeException & r )
        {
            VOS_ENSURE( 0, "Error preparing ZIP file for writing to disk" );
            throw WrappedTargetException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM ( "Error preparing ZIP file for writing to disk!" ) ),
                    static_cast < OWeakObject * > ( this ),
                    makeAny( r ) );
        }


        // Write a magic file with mimetype
        WriteMimetypeMagicFile( aZipOut );
    }

    // Create a vector to store data for the manifest.xml file
    vector < Sequence < PropertyValue > > aManList;

    // Make a reference to the manifest output stream so it persists
    // until the call to ZipOutputStream->finish()
    Reference < XOutputStream > xManOutStream;
    const OUString sMediaType ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    const OUString sFullPath ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );

    Sequence < PropertyValue > aPropSeq ( 2 );
    aPropSeq [0].Name = sMediaType;
    aPropSeq [0].Value <<= pRootFolder->GetMediaType( );
    aPropSeq [1].Name = sFullPath;
    aPropSeq [1].Value <<= OUString ( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );

    aManList.push_back( aPropSeq );

    // Get a random number generator and seed it with current timestamp
    // This will be used to generate random salt and initialisation vectors
    // for encrypted streams
    TimeValue aTime;
    osl_getSystemTime( &aTime );
    rtlRandomPool aRandomPool = rtl_random_createPool ();
    rtl_random_addBytes ( aRandomPool, &aTime, 8 );


    // call saveContents (it will recursively save sub-directories
    OUString aEmptyString;
    try {
        pRootFolder->saveContents( aEmptyString, aManList, aZipOut, aEncryptionKey, aRandomPool );
    }
    catch (::com::sun::star::uno::RuntimeException & r )
    {
        VOS_ENSURE( 0, "Error writing ZIP file to disk" );
        throw WrappedTargetException(
                OUString( RTL_CONSTASCII_USTRINGPARAM ( "Error writing ZIP file to disk!" ) ),
                static_cast < OWeakObject * > ( this ),
                makeAny( r ) );
    }

    // Clean up random pool memory
    rtl_random_destroyPool ( aRandomPool );

    if( bUseManifest && m_bPackageFormat )
    {
        // Write the manifest
        OUString sManifestWriter( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.manifest.ManifestWriter" ) );
        Reference < XManifestWriter > xWriter ( xFactory->createInstance( sManifestWriter ), UNO_QUERY );
        if ( xWriter.is() )
        {
            ZipEntry * pEntry = new ZipEntry;
            ZipPackageBuffer *pBuffer = new ZipPackageBuffer( n_ConstBufferSize );
            xManOutStream = Reference < XOutputStream > (*pBuffer, UNO_QUERY);

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

            try
            {
                // the manifest.xml is never encrypted - so pass an empty reference
                vos::ORef < EncryptionData > xEmpty;
                aZipOut.putNextEntry( *pEntry, xEmpty );
                aZipOut.write( pBuffer->getSequence(), 0, nBufferLength );
                aZipOut.closeEntry();
            }
            catch (::com::sun::star::io::IOException & r )
            {
                VOS_ENSURE( 0, "Error adding META-INF/manifest.xml to the ZipOutputStream" );
                throw WrappedTargetException(
                        OUString( RTL_CONSTASCII_USTRINGPARAM ( "Error adding META-INF/manifest.xml to the ZipOutputStream!" ) ),
                        static_cast < OWeakObject * > ( this ),
                        makeAny( r ) );

            }
        }
        else
        {
            VOS_ENSURE ( 0, "Couldn't get a ManifestWriter!" );
            IOException aException;
            throw WrappedTargetException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM ( "Couldn't get a ManifestWriter!" ) ),
                    static_cast < OWeakObject * > ( this ),
                    makeAny( aException ) );
        }
    }

    try
    {
        aZipOut.finish();
    }
    catch (::com::sun::star::io::IOException & r )
    {
        VOS_ENSURE( 0, "Error writing ZIP file to disk" );
        throw WrappedTargetException(
                OUString( RTL_CONSTASCII_USTRINGPARAM ( "Error writing ZIP file to disk!" ) ),
                static_cast < OWeakObject * > ( this ),
                makeAny( r ) );
    }

    if ( eMode == e_IMode_XStream )
    {
        // First truncate our output stream
        Reference < XOutputStream > xOutputStream = xStream->getOutputStream();
        Reference < XTruncate > xTruncate ( xOutputStream, UNO_QUERY );
        if ( !xTruncate.is() )
        {
            IOException aException;
            throw WrappedTargetException( OUString( RTL_CONSTASCII_USTRINGPARAM ( "This package is read only!" ) ),
                    static_cast < OWeakObject * > ( this ), makeAny ( aException ) );
        }
        xTruncate->truncate();

        // Then set up the tempfile to be read from
        Reference < XInputStream > xTempIn ( xTempOut, UNO_QUERY );
        Reference < XSeekable > xTempSeek ( xTempOut, UNO_QUERY );
        xTempSeek->seek ( 0 );

        // then copy the contents of the tempfile to our output stream
        copyInputToOutput_Impl( xTempIn, xOutputStream );
        xOutputStream->flush();
    }

    // Update our References to point to the new temp file
    if( aUseTemp )
    {
        xContentStream = Reference < XInputStream > ( xTempOut, UNO_QUERY );
        xContentSeek = Reference < XSeekable > ( xTempOut, UNO_QUERY );
    }
    else
    {
        // the case when the original file was written directly

        try
        {
            // the output should be closed after request fo input
            // to avoid file closing
            xContentStream = xSink->getStream()->getInputStream();
            xTempOut->closeOutput();
            xContentSeek = Reference < XSeekable > ( xContentStream, UNO_QUERY );
        }
        catch( Exception& )
        {
        }

        OSL_ENSURE( xContentStream.is() && xContentSeek.is(), "XSeekable interface is required!" );
    }

    // seek back to the beginning of the temp file so we can read segments from it
    xContentSeek->seek ( 0 );
    if ( pZipFile )
        pZipFile->setInputStream ( xContentStream );
    else
        pZipFile = new ZipFile ( xContentStream, xFactory, sal_False );

    return aUseTemp;
}

Reference< XActiveDataStreamer > ZipPackage::openOriginalForOutput()
{
    // open and truncate the original file
    Content aOriginalContent (sURL, Reference < XCommandEnvironment >() );
    Reference< XActiveDataStreamer > xSink = new ActiveDataStreamer;

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

                Reference< XInputStream > xTempIn = new DummyInputStream; //Reference< XInputStream >( xTempOut, UNO_QUERY );
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
        throw WrappedTargetException( OUString( RTL_CONSTASCII_USTRINGPARAM ( "This package is read only!" ) ),
                static_cast < OWeakObject * > ( this ), makeAny ( aException ) );
    }

    RTL_LOGFILE_TRACE_AUTHOR ( "package", LOGFILE_AUTHOR, "{ ZipPackage::commitChanges" );
    // First we write the entire package to a temporary file. After writeTempFile,
    // xContentSeek and xContentStream will reference the new temporary file.
    // Exception - empty or nonexistent local file that is written directly

    if ( writeFileIsTemp() && eMode == e_IMode_URL )
    {
        Reference< XOutputStream > aOrigFileStream;

        if( isLocalFile_Impl( sURL ) )
        {
            // write directly in case of local file
            Reference< XActiveDataStreamer > xSink = openOriginalForOutput();

            if( xSink.is() )
            {
                Reference< XStream > xStr = xSink->getStream();
                if( xStr.is() )
                {
                    aOrigFileStream = xStr->getOutputStream();
                    if( aOrigFileStream.is() )
                    {
                        copyInputToOutput_Impl( xContentStream, aOrigFileStream );
                        aOrigFileStream->closeOutput();
                        xContentSeek->seek ( 0 );
                    }
                }
            }
        }

        if( !aOrigFileStream.is() )
        {
            Reference < XPropertySet > xPropSet ( xContentStream, UNO_QUERY );
            if ( xPropSet.is() )
            {
                OUString sTargetFolder = sURL.copy ( 0, sURL.lastIndexOf ( static_cast < sal_Unicode > ( '/' ) ) );
                Content aContent ( sTargetFolder, Reference < XCommandEnvironment > () );

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
                try
                {
                    aContent.executeCommand ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "transfer" ) ), aAny );
                }
                catch (::com::sun::star::uno::Exception& r)
                {
                    throw WrappedTargetException( OUString( RTL_CONSTASCII_USTRINGPARAM ( "Unable to write Zip File to disk!" ) ),
                            static_cast < OWeakObject * > ( this ), makeAny( r ) );
                }
            }
            else
            {
                // not quite sure how it could happen that xContentStream WOULDN'T support
                // XPropertySet, but just in case... :)

                Content aOriginalContent (sURL, Reference < XCommandEnvironment >() );

                try
                {
                    aOriginalContent.writeStream ( xContentStream, sal_True );
                }
                catch (::com::sun::star::uno::Exception& r)
                {
                    throw WrappedTargetException( OUString( RTL_CONSTASCII_USTRINGPARAM ( "Unable to write Zip File to disk!" ) ),
                            static_cast < OWeakObject * > ( this ), makeAny( r ) );
                }
            }
        }
    }

    RTL_LOGFILE_TRACE_AUTHOR ( "package", LOGFILE_AUTHOR, "} ZipPackage::commitChanges" );
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
Reference < XInterface >SAL_CALL ZipPackage_createInstance(
    const Reference< XMultiServiceFactory > & xMgr )
{
    return Reference< XInterface >( *new ZipPackage(xMgr) );
}

OUString ZipPackage::static_getImplementationName()
{
#if SUPD>625
    return OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.comp.ZipPackage" ) );
#else
    return OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.package.Package" ) );
#endif
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
Reference < XSingleServiceFactory > ZipPackage::createServiceFactory( Reference < XMultiServiceFactory > const & rServiceFactory )
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

Reference< XPropertySetInfo > SAL_CALL ZipPackage::getPropertySetInfo(  )
        throw(RuntimeException)
{
    return Reference < XPropertySetInfo > (NULL);
}
void SAL_CALL ZipPackage::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
{
    if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("HasEncryptedEntries") ) )
        throw IllegalArgumentException (); // This property is read-only
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("EncryptionKey") ) )
    {
        if ( !m_bPackageFormat )
            throw PropertyVetoException();

        if (!( aValue >>= aEncryptionKey ) )
            throw IllegalArgumentException();
    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("UseManifest") ) )
    {
        if ( !m_bPackageFormat )
            throw PropertyVetoException();

        if (!( aValue >>= bUseManifest ) )
            throw IllegalArgumentException();
    }
    else
        throw UnknownPropertyException();
}
Any SAL_CALL ZipPackage::getPropertyValue( const OUString& PropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
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
    throw UnknownPropertyException();
}
void SAL_CALL ZipPackage::addPropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& xListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackage::removePropertyChangeListener( const OUString& aPropertyName, const Reference< XPropertyChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackage::addVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
void SAL_CALL ZipPackage::removeVetoableChangeListener( const OUString& PropertyName, const Reference< XVetoableChangeListener >& aListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
}
