/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include <ZipPackageFolder.hxx>
#include <ZipFile.hxx>
#include <ZipOutputStream.hxx>
#include <ZipPackageStream.hxx>
#include <PackageConstants.hxx>
#include <ZipPackageFolderEnumeration.hxx>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <vos/diagnose.hxx>
#include <osl/time.h>
#include <rtl/digest.h>
#include <ContentInfo.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <EncryptedDataHeader.hxx>
#include <rtl/random.h>
#include <rtl/instance.hxx>
#include <memory>

using namespace com::sun::star;
using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::container;
using namespace com::sun::star::packages;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace cppu;
using namespace std;
using namespace ::com::sun::star;
using vos::ORef;

namespace { struct lcl_CachedImplId : public rtl::Static< uno::Sequence < sal_Int8 >, lcl_CachedImplId > {}; }

ZipPackageFolder::ZipPackageFolder ( const uno::Reference< XMultiServiceFactory >& xFactory,
                                     sal_Int32 nFormat,
                                     sal_Bool bAllowRemoveOnInsert )
: m_xFactory( xFactory )
, m_nFormat( nFormat )
{
    OSL_ENSURE( m_xFactory.is(), "No factory is provided to the package folder!" );

    this->mbAllowRemoveOnInsert = bAllowRemoveOnInsert;

    SetFolder ( sal_True );
    aEntry.nVersion     = -1;
    aEntry.nFlag        = 0;
    aEntry.nMethod      = STORED;
    aEntry.nTime        = -1;
    aEntry.nCrc         = 0;
    aEntry.nCompressedSize  = 0;
    aEntry.nSize        = 0;
    aEntry.nOffset      = -1;
    uno::Sequence < sal_Int8 > &rCachedImplId = lcl_CachedImplId::get();
    if ( !rCachedImplId.getLength() )
        rCachedImplId = getImplementationId();
}


ZipPackageFolder::~ZipPackageFolder()
{
}

sal_Bool ZipPackageFolder::LookForUnexpectedODF12Streams( const ::rtl::OUString& aPath )
{
    sal_Bool bHasUnexpected = sal_False;

    for ( ContentHash::const_iterator aCI = maContents.begin(), aEnd = maContents.end();
          !bHasUnexpected && aCI != aEnd;
          aCI++)
    {
        const ::rtl::OUString &rShortName = (*aCI).first;
        const ContentInfo &rInfo = *(*aCI).second;

        if ( rInfo.bFolder )
        {
            if ( aPath.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "META-INF/" ) ) ) )
            {
                // META-INF is not allowed to contain subfolders
                bHasUnexpected = sal_True;
            }
            else
            {
                ::rtl::OUString sOwnPath = aPath + rShortName + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );
                bHasUnexpected = rInfo.pFolder->LookForUnexpectedODF12Streams( sOwnPath );
            }
        }
        else
        {
            if ( aPath.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "META-INF/" ) ) ) )
            {
                if ( !rShortName.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "manifest.xml" ) ) )
                  && rShortName.indexOf( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "signatures" ) ) ) == -1 )
                {
                    // a stream from META-INF with unexpected name
                    bHasUnexpected = sal_True;
                }

                // streams from META-INF with expected names are allowed not to be registered in manifest.xml
            }
            else if ( !rInfo.pStream->IsFromManifest() )
            {
                // the stream is not in META-INF and ist notregistered in manifest.xml,
                // check whether it is an internal part of the package format
                if ( aPath.getLength()
                  || !rShortName.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "mimetype" ) ) ) )
                {
                    // if it is not "mimetype" from the root it is not a part of the package
                    bHasUnexpected = sal_True;
                }
            }
        }
    }

    return bHasUnexpected;
}

void ZipPackageFolder::setChildStreamsTypeByExtension( const beans::StringPair& aPair )
{
    ::rtl::OUString aExt;
    if ( aPair.First.toChar() == (sal_Unicode)'.' )
        aExt = aPair.First;
    else
        aExt = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "." ) ) + aPair.First;

    for ( ContentHash::const_iterator aCI = maContents.begin(), aEnd = maContents.end();
          aCI != aEnd;
          aCI++)
    {
        const ::rtl::OUString &rShortName = (*aCI).first;
        const ContentInfo &rInfo = *(*aCI).second;

        if ( rInfo.bFolder )
            rInfo.pFolder->setChildStreamsTypeByExtension( aPair );
        else
        {
            sal_Int32 nPathLength = rShortName.getLength();
            sal_Int32 nExtLength = aExt.getLength();
            if ( nPathLength >= nExtLength && rShortName.match( aExt, nPathLength - nExtLength ) )
                rInfo.pStream->SetMediaType( aPair.Second );
        }
    }
}

void ZipPackageFolder::copyZipEntry( ZipEntry &rDest, const ZipEntry &rSource)
{
      rDest.nVersion            = rSource.nVersion;
    rDest.nFlag             = rSource.nFlag;
    rDest.nMethod           = rSource.nMethod;
    rDest.nTime             = rSource.nTime;
    rDest.nCrc              = rSource.nCrc;
    rDest.nCompressedSize   = rSource.nCompressedSize;
    rDest.nSize             = rSource.nSize;
    rDest.nOffset           = rSource.nOffset;
    rDest.sPath             = rSource.sPath;
    rDest.nPathLen          = rSource.nPathLen;
    rDest.nExtraLen         = rSource.nExtraLen;
}

const ::com::sun::star::uno::Sequence < sal_Int8 >& ZipPackageFolder::static_getImplementationId()
{
    return lcl_CachedImplId::get();
}

    // XNameContainer
void SAL_CALL ZipPackageFolder::insertByName( const ::rtl::OUString& aName, const uno::Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, uno::RuntimeException)
{
    if (hasByName(aName))
        throw ElementExistException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    else
    {
        uno::Reference < XUnoTunnel > xRef;
        aElement >>= xRef;
        if ( (  aElement >>= xRef ) )
        {
            sal_Int64 nTest;
            ZipPackageEntry *pEntry;
            if ( ( nTest = xRef->getSomething ( ZipPackageFolder::static_getImplementationId() ) ) != 0 )
            {
                ZipPackageFolder *pFolder = reinterpret_cast < ZipPackageFolder * > ( nTest );
                pEntry = static_cast < ZipPackageEntry * > ( pFolder );
            }
            else if ( ( nTest = xRef->getSomething ( ZipPackageStream::static_getImplementationId() ) ) != 0 )
            {
                ZipPackageStream *pStream = reinterpret_cast < ZipPackageStream * > ( nTest );
                pEntry = static_cast < ZipPackageEntry * > ( pStream );
            }
            else
                throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >(), 0 );

            if (pEntry->getName() != aName )
                pEntry->setName (aName);
            doInsertByName ( pEntry, sal_True );
        }
        else
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >(), 0 );
    }
}
void SAL_CALL ZipPackageFolder::removeByName( const ::rtl::OUString& Name )
        throw(NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    ContentHash::iterator aIter = maContents.find ( Name );
    if ( aIter == maContents.end() )
        throw NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    maContents.erase( aIter );
}
    // XEnumerationAccess
uno::Reference< XEnumeration > SAL_CALL ZipPackageFolder::createEnumeration(  )
        throw(uno::RuntimeException)
{
    return uno::Reference < XEnumeration> (new ZipPackageFolderEnumeration(maContents));
}
    // XElementAccess
uno::Type SAL_CALL ZipPackageFolder::getElementType(  )
        throw(uno::RuntimeException)
{
    return ::getCppuType ((const uno::Reference< XUnoTunnel > *) 0);
}
sal_Bool SAL_CALL ZipPackageFolder::hasElements(  )
        throw(uno::RuntimeException)
{
    return maContents.size() > 0;
}
    // XNameAccess
ContentInfo& ZipPackageFolder::doGetByName( const ::rtl::OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    ContentHash::iterator aIter = maContents.find ( aName );
    if ( aIter == maContents.end())
        throw NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    return *(*aIter).second;
}
uno::Any SAL_CALL ZipPackageFolder::getByName( const ::rtl::OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny ( doGetByName ( aName ).xTunnel );
}
uno::Sequence< ::rtl::OUString > SAL_CALL ZipPackageFolder::getElementNames(  )
        throw(uno::RuntimeException)
{
    sal_uInt32 i=0, nSize = maContents.size();
    uno::Sequence < ::rtl::OUString > aSequence ( nSize );
    for ( ContentHash::const_iterator aIterator = maContents.begin(), aEnd = maContents.end();
          aIterator != aEnd;
          ++i, ++aIterator)
        aSequence[i] = (*aIterator).first;
    return aSequence;
}
sal_Bool SAL_CALL ZipPackageFolder::hasByName( const ::rtl::OUString& aName )
    throw(uno::RuntimeException)
{
    return maContents.find ( aName ) != maContents.end ();
}
    // XNameReplace
void SAL_CALL ZipPackageFolder::replaceByName( const ::rtl::OUString& aName, const uno::Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    if ( hasByName( aName ) )
        removeByName( aName );
    else
        throw NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    insertByName(aName, aElement);
}

static void ImplSetStoredData( ZipEntry & rEntry, uno::Reference< XInputStream> & rStream )
{
    // It's very annoying that we have to do this, but lots of zip packages
    // don't allow data descriptors for STORED streams, meaning we have to
    // know the size and CRC32 of uncompressed streams before we actually
    // write them !
    CRC32 aCRC32;
    rEntry.nMethod = STORED;
    rEntry.nCompressedSize = rEntry.nSize = aCRC32.updateStream ( rStream );
    rEntry.nCrc = aCRC32.getValue();
}

void ZipPackageFolder::saveContents( ::rtl::OUString &rPath, std::vector < uno::Sequence < PropertyValue > > &rManList, ZipOutputStream & rZipOut, const uno::Sequence < sal_Int8 >&rEncryptionKey, rtlRandomPool &rRandomPool)
    throw(uno::RuntimeException)
{
    sal_Bool bWritingFailed = sal_False;
    ZipPackageFolder *pFolder = NULL;
    ZipPackageStream *pStream = NULL;
    const ::rtl::OUString sMediaTypeProperty ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    const ::rtl::OUString sVersionProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Version" ) );
    const ::rtl::OUString sFullPathProperty ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
    const ::rtl::OUString sInitialisationVectorProperty ( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) );
    const ::rtl::OUString sSaltProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
    const ::rtl::OUString sIterationCountProperty ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );
    const ::rtl::OUString sSizeProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Size" ) );
    const ::rtl::OUString sDigestProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Digest" ) );
    const ::rtl::OUString sEncryptionAlgProperty    ( RTL_CONSTASCII_USTRINGPARAM ( "EncryptionAlgorithm" ) );
    const ::rtl::OUString sStartKeyAlgProperty  ( RTL_CONSTASCII_USTRINGPARAM ( "StartKeyAlgorithm" ) );
    const ::rtl::OUString sDigestAlgProperty    ( RTL_CONSTASCII_USTRINGPARAM ( "DigestAlgorithm" ) );
    const ::rtl::OUString  sDerivedKeySizeProperty  ( RTL_CONSTASCII_USTRINGPARAM ( "DerivedKeySize" ) );

    sal_Bool bHaveEncryptionKey = rEncryptionKey.getLength() ? sal_True : sal_False;

    if ( maContents.begin() == maContents.end() && rPath.getLength() && m_nFormat != embed::StorageFormats::OFOPXML )
    {
        // it is an empty subfolder, use workaround to store it
        ZipEntry* pTempEntry = new ZipEntry();
        ZipPackageFolder::copyZipEntry ( *pTempEntry, aEntry );
        pTempEntry->nPathLen = (sal_Int16)( ::rtl::OUStringToOString( rPath, RTL_TEXTENCODING_UTF8 ).getLength() );
        pTempEntry->nExtraLen = -1;
        pTempEntry->sPath = rPath;

        try
        {
            rZipOut.putNextEntry( *pTempEntry, NULL, sal_False );
            rZipOut.rawCloseEntry();
        }
        catch ( ZipException& )
        {
            VOS_ENSURE( 0, "Error writing ZipOutputStream" );
            bWritingFailed = sal_True;
        }
        catch ( IOException& )
        {
            VOS_ENSURE( 0, "Error writing ZipOutputStream" );
            bWritingFailed = sal_True;
        }
    }

    for ( ContentHash::const_iterator aCI = maContents.begin(), aEnd = maContents.end();
          aCI != aEnd;
          aCI++)
    {
        const ::rtl::OUString &rShortName = (*aCI).first;
        const ContentInfo &rInfo = *(*aCI).second;

        uno::Sequence < PropertyValue > aPropSet( PKG_SIZE_NOENCR_MNFST );

        if ( rInfo.bFolder )
            pFolder = rInfo.pFolder;
        else
            pStream = rInfo.pStream;

        if ( rInfo.bFolder )
        {
            ::rtl::OUString sTempName = rPath + rShortName + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );

            if ( pFolder->GetMediaType().getLength() )
            {
                aPropSet[PKG_MNFST_MEDIATYPE].Name = sMediaTypeProperty;
                aPropSet[PKG_MNFST_MEDIATYPE].Value <<= pFolder->GetMediaType();
                aPropSet[PKG_MNFST_VERSION].Name = sVersionProperty;
                aPropSet[PKG_MNFST_VERSION].Value <<= pFolder->GetVersion();
                aPropSet[PKG_MNFST_FULLPATH].Name = sFullPathProperty;
                aPropSet[PKG_MNFST_FULLPATH].Value <<= sTempName;
            }
            else
                aPropSet.realloc( 0 );

            pFolder->saveContents( sTempName, rManList, rZipOut, rEncryptionKey, rRandomPool);
        }
        else
        {
            // if pTempEntry is necessary, it will be released and passed to the ZipOutputStream
            // and be deleted in the ZipOutputStream destructor
            auto_ptr < ZipEntry > pAutoTempEntry ( new ZipEntry );
            ZipEntry* pTempEntry = pAutoTempEntry.get();

            // In case the entry we are reading is also the entry we are writing, we will
            // store the ZipEntry data in pTempEntry

            ZipPackageFolder::copyZipEntry ( *pTempEntry, pStream->aEntry );
            pTempEntry->sPath = rPath + rShortName;
            pTempEntry->nPathLen = (sal_Int16)( ::rtl::OUStringToOString( pTempEntry->sPath, RTL_TEXTENCODING_UTF8 ).getLength() );

            sal_Bool bToBeEncrypted = pStream->IsToBeEncrypted() && (bHaveEncryptionKey || pStream->HasOwnKey());
            sal_Bool bToBeCompressed = bToBeEncrypted ? sal_True : pStream->IsToBeCompressed();

            aPropSet[PKG_MNFST_MEDIATYPE].Name = sMediaTypeProperty;
            aPropSet[PKG_MNFST_MEDIATYPE].Value <<= pStream->GetMediaType( );
            aPropSet[PKG_MNFST_VERSION].Name = sVersionProperty;
            aPropSet[PKG_MNFST_VERSION].Value <<= ::rtl::OUString(); // no version is stored for streams currently
            aPropSet[PKG_MNFST_FULLPATH].Name = sFullPathProperty;
            aPropSet[PKG_MNFST_FULLPATH].Value <<= pTempEntry->sPath;


            OSL_ENSURE( pStream->GetStreamMode() != PACKAGE_STREAM_NOTSET, "Unacceptable ZipPackageStream mode!" );

            sal_Bool bRawStream = sal_False;
            if ( pStream->GetStreamMode() == PACKAGE_STREAM_DETECT )
                bRawStream = pStream->ParsePackageRawStream();
            else if ( pStream->GetStreamMode() == PACKAGE_STREAM_RAW )
                bRawStream = sal_True;

            sal_Bool bTransportOwnEncrStreamAsRaw = sal_False;
            // During the storing the original size of the stream can be changed
            // TODO/LATER: get rid of this hack
            sal_Int32 nOwnStreamOrigSize = bRawStream ? pStream->GetMagicalHackSize() : pStream->getSize();

            sal_Bool bUseNonSeekableAccess = sal_False;
            uno::Reference < XInputStream > xStream;
            if ( !pStream->IsPackageMember() && !bRawStream && !bToBeEncrypted && bToBeCompressed )
            {
                // the stream is not a package member, not a raw stream,
                // it should not be encrypted and it should be compressed,
                // in this case nonseekable access can be used

                xStream = pStream->GetOwnStreamNoWrap();
                uno::Reference < XSeekable > xSeek ( xStream, uno::UNO_QUERY );

                bUseNonSeekableAccess = ( xStream.is() && !xSeek.is() );
            }

            if ( !bUseNonSeekableAccess )
            {
                xStream = pStream->getRawData();

                if ( !xStream.is() )
                {
                    VOS_ENSURE( 0, "ZipPackageStream didn't have a stream associated with it, skipping!" );
                    bWritingFailed = sal_True;
                    continue;
                }

                uno::Reference < XSeekable > xSeek ( xStream, uno::UNO_QUERY );
                try
                {
                    if ( xSeek.is() )
                    {
                        // If the stream is a raw one, then we should be positioned
                        // at the beginning of the actual data
                        if ( !bToBeCompressed || bRawStream )
                        {
                            // The raw stream can neither be encrypted nor connected
                            OSL_ENSURE( !bRawStream || !bToBeCompressed && !bToBeEncrypted, "The stream is already encrypted!\n" );
                            xSeek->seek ( bRawStream ? pStream->GetMagicalHackPos() : 0 );
                            ImplSetStoredData ( *pTempEntry, xStream );

                            // TODO/LATER: Get rid of hacks related to switching of Flag Method and Size properties!
                        }
                        else if ( bToBeEncrypted )
                        {
                            // this is the correct original size
                            pTempEntry->nSize = static_cast < sal_Int32 > ( xSeek->getLength() );
                            nOwnStreamOrigSize = pTempEntry->nSize;
                        }

                        xSeek->seek ( 0 );
                    }
                    else
                    {
                        // Okay, we don't have an xSeekable stream. This is possibly bad.
                        // check if it's one of our own streams, if it is then we know that
                        // each time we ask for it we'll get a new stream that will be
                        // at position zero...otherwise, assert and skip this stream...
                        if ( pStream->IsPackageMember() )
                        {
                            // if the password has been changed than the stream should not be package member any more
                            if ( pStream->IsEncrypted() && pStream->IsToBeEncrypted() )
                            {
                                // Should be handled close to the raw stream handling
                                bTransportOwnEncrStreamAsRaw = sal_True;
                                pTempEntry->nMethod = STORED;

                                // TODO/LATER: get rid of this situation
                                // this size should be different from the one that will be stored in manifest.xml
                                // it is used in storing algorithms and after storing the correct size will be set
                                pTempEntry->nSize = pTempEntry->nCompressedSize;
                            }
                        }
                        else
                        {
                            VOS_ENSURE( 0, "The package component requires that every stream either be FROM a package or it must support XSeekable!" );
                            continue;
                        }
                    }
                }
                catch ( uno::Exception& )
                {
                    VOS_ENSURE( 0, "The stream provided to the package component has problems!" );
                    bWritingFailed = sal_True;
                    continue;
                }

                if ( bToBeEncrypted || bRawStream || bTransportOwnEncrStreamAsRaw )
                {
                    if ( bToBeEncrypted && !bTransportOwnEncrStreamAsRaw )
                    {
                        sal_Int32 nVectorLen = pStream->GetBlockSize();
                        uno::Sequence < sal_Int8 > aSalt ( 16 ), aVector ( nVectorLen );
                        rtl_random_getBytes ( rRandomPool, aSalt.getArray(), 16 );
                        rtl_random_getBytes ( rRandomPool, aVector.getArray(), nVectorLen );
                        sal_Int32 nIterationCount = 1024;

                        if ( !pStream->HasOwnKey() )
                            pStream->setKey ( rEncryptionKey );

                        pStream->setInitialisationVector ( aVector );
                        pStream->setSalt ( aSalt );
                        pStream->setIterationCount ( nIterationCount );
                    }

                    // last property is digest, which is inserted later if we didn't have
                    // a magic header
                    aPropSet.realloc( PKG_SIZE_ENCR_MNFST );

                    aPropSet[PKG_MNFST_INIVECTOR].Name = sInitialisationVectorProperty;
                    aPropSet[PKG_MNFST_INIVECTOR].Value <<= pStream->getInitialisationVector();
                    aPropSet[PKG_MNFST_SALT].Name = sSaltProperty;
                    aPropSet[PKG_MNFST_SALT].Value <<= pStream->getSalt();
                    aPropSet[PKG_MNFST_ITERATION].Name = sIterationCountProperty;
                    aPropSet[PKG_MNFST_ITERATION].Value <<= pStream->getIterationCount ();

                    // Need to store the uncompressed size in the manifest
                    OSL_ENSURE( nOwnStreamOrigSize >= 0, "The stream size was not correctly initialized!\n" );
                    aPropSet[PKG_MNFST_UCOMPSIZE].Name = sSizeProperty;
                    aPropSet[PKG_MNFST_UCOMPSIZE].Value <<= nOwnStreamOrigSize;

                    if ( bRawStream || bTransportOwnEncrStreamAsRaw )
                    {
                        ::rtl::Reference< EncryptionData > xEncData = pStream->GetEncryptionData();
                        if ( xEncData.is() )
                        {
                            aPropSet[PKG_MNFST_DIGEST].Name = sDigestProperty;
                            aPropSet[PKG_MNFST_DIGEST].Value <<= xEncData->m_aDigest;
                            aPropSet[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
                            aPropSet[PKG_MNFST_ENCALG].Value <<= xEncData->m_nEncAlg;
                            aPropSet[PKG_MNFST_STARTALG].Name = sStartKeyAlgProperty;
                            aPropSet[PKG_MNFST_STARTALG].Value <<= xEncData->m_nStartKeyGenID;
                            aPropSet[PKG_MNFST_DIGESTALG].Name = sDigestAlgProperty;
                            aPropSet[PKG_MNFST_DIGESTALG].Value <<= xEncData->m_nCheckAlg;
                            aPropSet[PKG_MNFST_DERKEYSIZE].Name = sDerivedKeySizeProperty;
                            aPropSet[PKG_MNFST_DERKEYSIZE].Value <<= xEncData->m_nDerivedKeySize;
                        }
                    }
                }
            }

            // If the entry is already stored in the zip file in the format we
            // want for this write...copy it raw
            if ( !bUseNonSeekableAccess
              && ( bRawStream || bTransportOwnEncrStreamAsRaw
                || ( pStream->IsPackageMember() && !bToBeEncrypted
                  && ( ( pStream->aEntry.nMethod == DEFLATED && bToBeCompressed )
                    || ( pStream->aEntry.nMethod == STORED && !bToBeCompressed ) ) ) ) )
            {
                // If it's a PackageMember, then it's an unbuffered stream and we need
                // to get a new version of it as we can't seek backwards.
                if ( pStream->IsPackageMember() )
                {
                    xStream = pStream->getRawData();
                    if ( !xStream.is() )
                    {
                        // Make sure that we actually _got_ a new one !
                        VOS_ENSURE( 0, "ZipPackageStream didn't have a stream associated with it, skipping!" );
                        continue;
                    }
                }

                try
                {
                    if ( bRawStream )
                        xStream->skipBytes( pStream->GetMagicalHackPos() );

                    rZipOut.putNextEntry( *pTempEntry, pStream, sal_False );
                    // the entry is provided to the ZipOutputStream that will delete it
                    pAutoTempEntry.release();

                    uno::Sequence < sal_Int8 > aSeq ( n_ConstBufferSize );
                    sal_Int32 nLength;

                    do
                    {
                        nLength = xStream->readBytes( aSeq, n_ConstBufferSize );
                        rZipOut.rawWrite(aSeq, 0, nLength);
                    }
                    while ( nLength == n_ConstBufferSize );

                    rZipOut.rawCloseEntry();
                }
                catch ( ZipException& )
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                    bWritingFailed = sal_True;
                }
                catch ( IOException& )
                {
                    VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                    bWritingFailed = sal_True;
                }
            }
            else
            {
                // This stream is defenitly not a raw stream

                // If nonseekable access is used the stream should be at the beginning and
                // is useless after the storing. Thus if the storing fails the package should
                // be thrown away ( as actually it is done currently )!
                // To allow to reuse the package after the error, the optimization must be removed!

                // If it's a PackageMember, then our previous reference held a 'raw' stream
                // so we need to re-get it, unencrypted, uncompressed and positioned at the
                // beginning of the stream
                if ( pStream->IsPackageMember() )
                {
                    xStream = pStream->getInputStream();
                    if ( !xStream.is() )
                    {
                        // Make sure that we actually _got_ a new one !
                        VOS_ENSURE( 0, "ZipPackageStream didn't have a stream associated with it, skipping!" );
                        continue;
                    }
                }

                if ( bToBeCompressed )
                {
                    pTempEntry->nMethod = DEFLATED;
                    pTempEntry->nCrc = pTempEntry->nCompressedSize = pTempEntry->nSize = -1;
                }

                try
                {
                    rZipOut.putNextEntry ( *pTempEntry, pStream, bToBeEncrypted);
                    // the entry is provided to the ZipOutputStream that will delete it
                    pAutoTempEntry.release();

                    sal_Int32 nLength;
                    uno::Sequence < sal_Int8 > aSeq (n_ConstBufferSize);
                    do
                    {
                        nLength = xStream->readBytes(aSeq, n_ConstBufferSize);
                        rZipOut.write(aSeq, 0, nLength);
                    }
                    while ( nLength == n_ConstBufferSize );

                    rZipOut.closeEntry();
                }
                catch ( ZipException& )
                {
                    bWritingFailed = sal_True;
                }
                catch ( IOException& )
                {
                    bWritingFailed = sal_True;
                }

                if ( bToBeEncrypted )
                {
                    ::rtl::Reference< EncryptionData > xEncData = pStream->GetEncryptionData();
                    if ( xEncData.is() )
                    {
                        aPropSet[PKG_MNFST_DIGEST].Name = sDigestProperty;
                        aPropSet[PKG_MNFST_DIGEST].Value <<= xEncData->m_aDigest;
                        aPropSet[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
                        aPropSet[PKG_MNFST_ENCALG].Value <<= xEncData->m_nEncAlg;
                        aPropSet[PKG_MNFST_STARTALG].Name = sStartKeyAlgProperty;
                        aPropSet[PKG_MNFST_STARTALG].Value <<= xEncData->m_nStartKeyGenID;
                        aPropSet[PKG_MNFST_DIGESTALG].Name = sDigestAlgProperty;
                        aPropSet[PKG_MNFST_DIGESTALG].Value <<= xEncData->m_nCheckAlg;
                        aPropSet[PKG_MNFST_DERKEYSIZE].Name = sDerivedKeySizeProperty;
                        aPropSet[PKG_MNFST_DERKEYSIZE].Value <<= xEncData->m_nDerivedKeySize;

                        pStream->SetIsEncrypted ( sal_True );
                    }
                }
            }

            if( !bWritingFailed )
            {
                if ( !pStream->IsPackageMember() )
                {
                    pStream->CloseOwnStreamIfAny();
                    pStream->SetPackageMember ( sal_True );
                }

                if ( bRawStream )
                {
                    // the raw stream was integrated and now behaves
                    // as usual encrypted stream
                    pStream->SetToBeEncrypted( sal_True );
                }

                // Remove hacky bit from entry flags
                if ( pTempEntry->nFlag & ( 1 << 4 ) )
                {
                    pTempEntry->nFlag &= ~( 1 << 4 );
                    pTempEntry->nMethod = STORED;
                }

                // Then copy it back afterwards...
                ZipPackageFolder::copyZipEntry ( pStream->aEntry, *pTempEntry );

                // TODO/LATER: get rid of this hack ( the encrypted stream size property is changed during saving )
                if ( pStream->IsEncrypted() )
                    pStream->setSize( nOwnStreamOrigSize );

                pStream->aEntry.nOffset *= -1;
            }
        }

        // folder can have a mediatype only in package format
        if ( aPropSet.getLength()
          && ( m_nFormat == embed::StorageFormats::PACKAGE || ( m_nFormat == embed::StorageFormats::OFOPXML && !rInfo.bFolder ) ) )
            rManList.push_back( aPropSet );
    }

    if( bWritingFailed )
        throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

void ZipPackageFolder::releaseUpwardRef( void )
{
    // Now it is possible that a package folder is disconnected from the package before removing of the folder.
    // Such a scenario is used in storage implementation. When a new version of a folder is provided the old
    // one is retrieved, removed from the package but preserved for the error handling.
    // In this scenario the referencing to the parent is not really useful, since it requires disposing.

    // Actually there is no need in having a reference to the parent, it even make things more complicated and
    // requires disposing mechanics. Using of a simple pointer seems to be easier solution and also a safe enough.

    clearParent();

#if 0
    for ( ContentHash::const_iterator aCI = maContents.begin();
          aCI!=maContents.end();
          aCI++)
    {
        ContentInfo &rInfo = * (*aCI).second;
        if ( rInfo.bFolder )// && ! rInfo.pFolder->HasReleased () )
            rInfo.pFolder->releaseUpwardRef();
        else //if ( !rInfo.bFolder && !rInfo.pStream->HasReleased() )
            rInfo.pStream->clearParent();
    }
    clearParent();

    VOS_ENSURE ( m_refCount == 1, "Ref-count is not 1!" );
#endif
}

sal_Int64 SAL_CALL ZipPackageFolder::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw(uno::RuntimeException)
{
    sal_Int64 nMe = 0;
    if ( aIdentifier.getLength() == 16 &&
         0 == rtl_compareMemory(static_getImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        nMe = reinterpret_cast < sal_Int64 > ( this );
    return nMe;
}
void SAL_CALL ZipPackageFolder::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, uno::RuntimeException)
{
    if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("MediaType")))
    {
        // TODO/LATER: activate when zip ucp is ready
        // if ( m_nFormat != embed::StorageFormats::PACKAGE )
        //  throw UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        aValue >>= sMediaType;
    }
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Version")))
        aValue >>= m_sVersion;
    else if (aPropertyName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Size") ) )
        aValue >>= aEntry.nSize;
    else
        throw UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}
uno::Any SAL_CALL ZipPackageFolder::getPropertyValue( const ::rtl::OUString& PropertyName )
        throw(UnknownPropertyException, WrappedTargetException, uno::RuntimeException)
{
    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
    {
        // TODO/LATER: activate when zip ucp is ready
        // if ( m_nFormat != embed::StorageFormats::PACKAGE )
        //  throw UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        return uno::makeAny ( sMediaType );
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Version" ) ) )
        return uno::makeAny( m_sVersion );
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Size" ) ) )
        return uno::makeAny ( aEntry.nSize );
    else
        throw UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

void ZipPackageFolder::doInsertByName ( ZipPackageEntry *pEntry, sal_Bool bSetParent )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, uno::RuntimeException)
{
    try
    {
        if ( pEntry->IsFolder() )
            maContents[pEntry->getName()] = new ContentInfo ( static_cast < ZipPackageFolder *> ( pEntry ) );
        else
            maContents[pEntry->getName()] = new ContentInfo ( static_cast < ZipPackageStream *> ( pEntry ) );
    }
    catch(const uno::Exception& rEx)
    {
        (void)rEx;
        throw;
    }
    if ( bSetParent )
        pEntry->setParent ( *this );
}
::rtl::OUString ZipPackageFolder::getImplementationName()
    throw (uno::RuntimeException)
{
    return ::rtl::OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ZipPackageFolder" ) );
}

uno::Sequence< ::rtl::OUString > ZipPackageFolder::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.PackageFolder" ) );
    return aNames;
}
sal_Bool SAL_CALL ZipPackageFolder::supportsService( ::rtl::OUString const & rServiceName )
    throw (uno::RuntimeException)
{
    return rServiceName == getSupportedServiceNames()[0];
}
