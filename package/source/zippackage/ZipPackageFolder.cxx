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

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace cppu;
using namespace rtl;
using namespace std;
using namespace ::com::sun::star;
using vos::ORef;

namespace { struct lcl_CachedImplId : public rtl::Static< Sequence < sal_Int8 >, lcl_CachedImplId > {}; }

ZipPackageFolder::ZipPackageFolder ( const Reference< XMultiServiceFactory >& xFactory,
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
    Sequence < sal_Int8 > &rCachedImplId = lcl_CachedImplId::get();
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
        const OUString &rShortName = (*aCI).first;
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
                OUString sOwnPath = aPath + rShortName + OUString( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );
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
        const OUString &rShortName = (*aCI).first;
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
void SAL_CALL ZipPackageFolder::insertByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    if (hasByName(aName))
        throw ElementExistException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    else
    {
        Reference < XUnoTunnel > xRef;
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
void SAL_CALL ZipPackageFolder::removeByName( const OUString& Name )
        throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ContentHash::iterator aIter = maContents.find ( Name );
    if ( aIter == maContents.end() )
        throw NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    maContents.erase( aIter );
}
    // XEnumerationAccess
Reference< XEnumeration > SAL_CALL ZipPackageFolder::createEnumeration(  )
        throw(RuntimeException)
{
    return Reference < XEnumeration> (new ZipPackageFolderEnumeration(maContents));
}
    // XElementAccess
Type SAL_CALL ZipPackageFolder::getElementType(  )
        throw(RuntimeException)
{
    return ::getCppuType ((const Reference< XUnoTunnel > *) 0);
}
sal_Bool SAL_CALL ZipPackageFolder::hasElements(  )
        throw(RuntimeException)
{
    return maContents.size() > 0;
}
    // XNameAccess
ContentInfo& ZipPackageFolder::doGetByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    ContentHash::iterator aIter = maContents.find ( aName );
    if ( aIter == maContents.end())
        throw NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    return *(*aIter).second;
}
Any SAL_CALL ZipPackageFolder::getByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    return makeAny ( doGetByName ( aName ).xTunnel );
}
Sequence< OUString > SAL_CALL ZipPackageFolder::getElementNames(  )
        throw(RuntimeException)
{
    sal_uInt32 i=0, nSize = maContents.size();
    Sequence < OUString > aSequence ( nSize );
    for ( ContentHash::const_iterator aIterator = maContents.begin(), aEnd = maContents.end();
          aIterator != aEnd;
          ++i, ++aIterator)
        aSequence[i] = (*aIterator).first;
    return aSequence;
}
sal_Bool SAL_CALL ZipPackageFolder::hasByName( const OUString& aName )
    throw(RuntimeException)
{
    return maContents.find ( aName ) != maContents.end ();
}
    // XNameReplace
void SAL_CALL ZipPackageFolder::replaceByName( const OUString& aName, const Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    if ( hasByName( aName ) )
        removeByName( aName );
    else
        throw NoSuchElementException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
    insertByName(aName, aElement);
}

static void ImplSetStoredData( ZipEntry & rEntry, Reference < XInputStream> & rStream )
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

bool ZipPackageFolder::saveChild( const OUString &rShortName, const ContentInfo &rInfo, OUString &rPath, std::vector < Sequence < PropertyValue > > &rManList, ZipOutputStream & rZipOut, Sequence < sal_Int8 > &rEncryptionKey, rtlRandomPool &rRandomPool)
{
    bool bSuccess = true;

    const OUString sMediaTypeProperty ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    const OUString sVersionProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Version" ) );
    const OUString sFullPathProperty ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
    const OUString sInitialisationVectorProperty ( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) );
    const OUString sSaltProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
    const OUString sIterationCountProperty ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );
    const OUString sSizeProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Size" ) );
    const OUString sDigestProperty ( RTL_CONSTASCII_USTRINGPARAM ( "Digest" ) );

    Sequence < PropertyValue > aPropSet (PKG_SIZE_NOENCR_MNFST);
    ZipPackageStream *pStream = NULL;

    OSL_ENSURE( ( rInfo.bFolder && rInfo.pFolder ) || ( !rInfo.bFolder && rInfo.pStream ), "A valid child object is expected!" );
    if ( rInfo.bFolder )
    {
        OUString sTempName = rPath + rShortName + OUString( RTL_CONSTASCII_USTRINGPARAM ( "/" ) );

        if ( rInfo.pFolder->GetMediaType().getLength() )
        {
            aPropSet[PKG_MNFST_MEDIATYPE].Name = sMediaTypeProperty;
            aPropSet[PKG_MNFST_MEDIATYPE].Value <<= rInfo.pFolder->GetMediaType();
            aPropSet[PKG_MNFST_VERSION].Name = sVersionProperty;
            aPropSet[PKG_MNFST_VERSION].Value <<= rInfo.pFolder->GetVersion();
            aPropSet[PKG_MNFST_FULLPATH].Name = sFullPathProperty;
            aPropSet[PKG_MNFST_FULLPATH].Value <<= sTempName;
        }
        else
            aPropSet.realloc( 0 );

        rInfo.pFolder->saveContents( sTempName, rManList, rZipOut, rEncryptionKey, rRandomPool);
    }
    else
    {
        // if pTempEntry is necessary, it will be released and passed to the ZipOutputStream
        // and be deleted in the ZipOutputStream destructor
        auto_ptr < ZipEntry > pAutoTempEntry ( new ZipEntry );
        ZipEntry* pTempEntry = pAutoTempEntry.get();

        // In case the entry we are reading is also the entry we are writing, we will
        // store the ZipEntry data in pTempEntry

        ZipPackageFolder::copyZipEntry ( *pTempEntry, rInfo.pStream->aEntry );
        pTempEntry->sPath = rPath + rShortName;
        pTempEntry->nPathLen = (sal_Int16)( ::rtl::OUStringToOString( pTempEntry->sPath, RTL_TEXTENCODING_UTF8 ).getLength() );

        sal_Bool bToBeEncrypted = rInfo.pStream->IsToBeEncrypted() && (rEncryptionKey.getLength() || rInfo.pStream->HasOwnKey());
        sal_Bool bToBeCompressed = bToBeEncrypted ? sal_True : rInfo.pStream->IsToBeCompressed();

        aPropSet[PKG_MNFST_MEDIATYPE].Name = sMediaTypeProperty;
        aPropSet[PKG_MNFST_MEDIATYPE].Value <<= rInfo.pStream->GetMediaType( );
        aPropSet[PKG_MNFST_VERSION].Name = sVersionProperty;
        aPropSet[PKG_MNFST_VERSION].Value <<= ::rtl::OUString(); // no version is stored for streams currently
        aPropSet[PKG_MNFST_FULLPATH].Name = sFullPathProperty;
        aPropSet[PKG_MNFST_FULLPATH].Value <<= pTempEntry->sPath;


        OSL_ENSURE( rInfo.pStream->GetStreamMode() != PACKAGE_STREAM_NOTSET, "Unacceptable ZipPackageStream mode!" );

        sal_Bool bRawStream = sal_False;
        if ( rInfo.pStream->GetStreamMode() == PACKAGE_STREAM_DETECT )
            bRawStream = rInfo.pStream->ParsePackageRawStream();
        else if ( rInfo.pStream->GetStreamMode() == PACKAGE_STREAM_RAW )
            bRawStream = sal_True;

        sal_Bool bTransportOwnEncrStreamAsRaw = sal_False;
        // During the storing the original size of the stream can be changed
        // TODO/LATER: get rid of this hack
        sal_Int32 nOwnStreamOrigSize = bRawStream ? rInfo.pStream->GetMagicalHackSize() : rInfo.pStream->getSize();

        sal_Bool bUseNonSeekableAccess = sal_False;
        Reference < XInputStream > xStream;
        if ( !rInfo.pStream->IsPackageMember() && !bRawStream && !bToBeEncrypted && bToBeCompressed )
        {
            // the stream is not a package member, not a raw stream,
            // it should not be encrypted and it should be compressed,
            // in this case nonseekable access can be used

            xStream = rInfo.pStream->GetOwnStreamNoWrap();
            Reference < XSeekable > xSeek ( xStream, UNO_QUERY );

            bUseNonSeekableAccess = ( xStream.is() && !xSeek.is() );
        }

        if ( !bUseNonSeekableAccess )
        {
            xStream = rInfo.pStream->getRawData();

            if ( !xStream.is() )
            {
                VOS_ENSURE( 0, "ZipPackageStream didn't have a stream associated with it, skipping!" );
                bSuccess = false;
                return bSuccess;
            }

            Reference < XSeekable > xSeek ( xStream, UNO_QUERY );
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
                        xSeek->seek ( bRawStream ? rInfo.pStream->GetMagicalHackPos() : 0 );
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
                    if ( rInfo.pStream->IsPackageMember() )
                    {
                        // if the password has been changed than the stream should not be package member any more
                        if ( rInfo.pStream->IsEncrypted() && rInfo.pStream->IsToBeEncrypted() )
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
                        bSuccess = false;
                        return bSuccess;
                    }
                }
            }
            catch ( Exception& )
            {
                VOS_ENSURE( 0, "The stream provided to the package component has problems!" );
                bSuccess = false;
                return bSuccess;
            }

            if ( bToBeEncrypted || bRawStream || bTransportOwnEncrStreamAsRaw )
            {
                if ( bToBeEncrypted && !bTransportOwnEncrStreamAsRaw )
                {
                    Sequence < sal_uInt8 > aSalt ( 16 ), aVector ( 8 );
                    rtl_random_getBytes ( rRandomPool, aSalt.getArray(), 16 );
                    rtl_random_getBytes ( rRandomPool, aVector.getArray(), 8 );
                    sal_Int32 nIterationCount = 1024;

                    if ( !rInfo.pStream->HasOwnKey() )
                        rInfo.pStream->setKey ( rEncryptionKey );

                    rInfo.pStream->setInitialisationVector ( aVector );
                    rInfo.pStream->setSalt ( aSalt );
                    rInfo.pStream->setIterationCount ( nIterationCount );
                }

                // last property is digest, which is inserted later if we didn't have
                // a magic header
                aPropSet.realloc(PKG_SIZE_ENCR_MNFST);

                aPropSet[PKG_MNFST_INIVECTOR].Name = sInitialisationVectorProperty;
                aPropSet[PKG_MNFST_INIVECTOR].Value <<= rInfo.pStream->getInitialisationVector();
                aPropSet[PKG_MNFST_SALT].Name = sSaltProperty;
                aPropSet[PKG_MNFST_SALT].Value <<= rInfo.pStream->getSalt();
                aPropSet[PKG_MNFST_ITERATION].Name = sIterationCountProperty;
                aPropSet[PKG_MNFST_ITERATION].Value <<= rInfo.pStream->getIterationCount ();

                // Need to store the uncompressed size in the manifest
                OSL_ENSURE( nOwnStreamOrigSize >= 0, "The stream size was not correctly initialized!\n" );
                aPropSet[PKG_MNFST_UCOMPSIZE].Name = sSizeProperty;
                aPropSet[PKG_MNFST_UCOMPSIZE].Value <<= nOwnStreamOrigSize;

                if ( bRawStream || bTransportOwnEncrStreamAsRaw )
                {
                    aPropSet[PKG_MNFST_DIGEST].Name = sDigestProperty;
                    aPropSet[PKG_MNFST_DIGEST].Value <<= rInfo.pStream->getDigest();
                }
            }
        }

        // If the entry is already stored in the zip file in the format we
        // want for this write...copy it raw
        if ( !bUseNonSeekableAccess
          && ( bRawStream || bTransportOwnEncrStreamAsRaw
            || ( rInfo.pStream->IsPackageMember() && !bToBeEncrypted
              && ( ( rInfo.pStream->aEntry.nMethod == DEFLATED && bToBeCompressed )
                || ( rInfo.pStream->aEntry.nMethod == STORED && !bToBeCompressed ) ) ) ) )
        {
            // If it's a PackageMember, then it's an unbuffered stream and we need
            // to get a new version of it as we can't seek backwards.
            if ( rInfo.pStream->IsPackageMember() )
            {
                xStream = rInfo.pStream->getRawData();
                if ( !xStream.is() )
                {
                    // Make sure that we actually _got_ a new one !
                    VOS_ENSURE( 0, "ZipPackageStream didn't have a stream associated with it, skipping!" );
                    bSuccess = false;
                    return bSuccess;
                }
            }

            try
            {
                if ( bRawStream )
                    xStream->skipBytes( rInfo.pStream->GetMagicalHackPos() );

                rZipOut.putNextEntry ( *pTempEntry, rInfo.pStream->getEncryptionData(), sal_False );
                // the entry is provided to the ZipOutputStream that will delete it
                pAutoTempEntry.release();

                Sequence < sal_Int8 > aSeq ( n_ConstBufferSize );
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
                bSuccess = false;
            }
            catch ( IOException& )
            {
                VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                bSuccess = false;
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
            if ( rInfo.pStream->IsPackageMember() )
            {
                xStream = rInfo.pStream->getInputStream();
                if ( !xStream.is() )
                {
                    // Make sure that we actually _got_ a new one !
                    VOS_ENSURE( 0, "ZipPackageStream didn't have a stream associated with it, skipping!" );
                    bSuccess = false;
                    return bSuccess;
                }
            }

            if ( bToBeCompressed )
            {
                pTempEntry->nMethod = DEFLATED;
                pTempEntry->nCrc = pTempEntry->nCompressedSize = pTempEntry->nSize = -1;
            }

            try
            {
                rZipOut.putNextEntry ( *pTempEntry, rInfo.pStream->getEncryptionData(), bToBeEncrypted);
                // the entry is provided to the ZipOutputStream that will delete it
                pAutoTempEntry.release();

                sal_Int32 nLength;
                Sequence < sal_Int8 > aSeq (n_ConstBufferSize);
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
                VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                bSuccess = false;
            }
            catch ( IOException& )
            {
                VOS_ENSURE( 0, "Error writing ZipOutputStream" );
                bSuccess = false;
            }

            if ( bToBeEncrypted )
            {
                aPropSet[PKG_MNFST_DIGEST].Name = sDigestProperty;
                aPropSet[PKG_MNFST_DIGEST].Value <<= rInfo.pStream->getDigest();
                rInfo.pStream->SetIsEncrypted ( sal_True );
            }
        }

        if( bSuccess )
        {
            if ( !rInfo.pStream->IsPackageMember() )
            {
                rInfo.pStream->CloseOwnStreamIfAny();
                rInfo.pStream->SetPackageMember ( sal_True );
            }

            if ( bRawStream )
            {
                // the raw stream was integrated and now behaves
                // as usual encrypted stream
                rInfo.pStream->SetToBeEncrypted( sal_True );
            }

            // Remove hacky bit from entry flags
            if ( pTempEntry->nFlag & ( 1 << 4 ) )
            {
                pTempEntry->nFlag &= ~( 1 << 4 );
                pTempEntry->nMethod = STORED;
            }

            // Then copy it back afterwards...
            ZipPackageFolder::copyZipEntry ( rInfo.pStream->aEntry, *pTempEntry );

            // TODO/LATER: get rid of this hack ( the encrypted stream size property is changed during saving )
            if ( rInfo.pStream->IsEncrypted() )
                rInfo.pStream->setSize( nOwnStreamOrigSize );

            rInfo.pStream->aEntry.nOffset *= -1;
        }
    }

    // folder can have a mediatype only in package format
    if ( aPropSet.getLength()
      && ( m_nFormat == embed::StorageFormats::PACKAGE || ( m_nFormat == embed::StorageFormats::OFOPXML && !rInfo.bFolder ) ) )
        rManList.push_back( aPropSet );

    return bSuccess;
}

void ZipPackageFolder::saveContents(OUString &rPath, std::vector < Sequence < PropertyValue > > &rManList, ZipOutputStream & rZipOut, Sequence < sal_Int8 > &rEncryptionKey, rtlRandomPool &rRandomPool)
    throw(RuntimeException)
{
    bool bWritingFailed = false;

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
            vos::ORef < EncryptionData > aEmptyEncr;
            rZipOut.putNextEntry ( *pTempEntry, aEmptyEncr, sal_False );
            rZipOut.rawCloseEntry();
        }
        catch ( ZipException& )
        {
            VOS_ENSURE( 0, "Error writing ZipOutputStream" );
            bWritingFailed = true;
        }
        catch ( IOException& )
        {
            VOS_ENSURE( 0, "Error writing ZipOutputStream" );
            bWritingFailed = true;
        }
    }

    bool bMimeTypeStreamStored = false;
    ::rtl::OUString aMimeTypeStreamName( RTL_CONSTASCII_USTRINGPARAM( "mimetype" ) );
    if ( m_nFormat == embed::StorageFormats::ZIP && !rPath.getLength() )
    {
        // let the "mimtype" stream in root folder be stored as the first stream if it is zip format
        ContentHash::iterator aIter = maContents.find ( aMimeTypeStreamName );
        if ( aIter != maContents.end() && !(*aIter).second->bFolder )
        {
            bMimeTypeStreamStored = true;
            bWritingFailed = !saveChild( (*aIter).first, *(*aIter).second, rPath, rManList, rZipOut, rEncryptionKey, rRandomPool );
        }
    }

    for ( ContentHash::const_iterator aCI = maContents.begin(), aEnd = maContents.end();
          aCI != aEnd;
          aCI++)
    {
        const OUString &rShortName = (*aCI).first;
        const ContentInfo &rInfo = *(*aCI).second;

        if ( !bMimeTypeStreamStored || !rShortName.equals( aMimeTypeStreamName ) )
            bWritingFailed = !saveChild( rShortName, rInfo, rPath, rManList, rZipOut, rEncryptionKey, rRandomPool );
    }

    if( bWritingFailed )
        throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
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

sal_Int64 SAL_CALL ZipPackageFolder::getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw(RuntimeException)
{
    sal_Int64 nMe = 0;
    if ( aIdentifier.getLength() == 16 &&
         0 == rtl_compareMemory(static_getImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        nMe = reinterpret_cast < sal_Int64 > ( this );
    return nMe;
}
void SAL_CALL ZipPackageFolder::setPropertyValue( const OUString& aPropertyName, const Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
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
Any SAL_CALL ZipPackageFolder::getPropertyValue( const OUString& PropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "MediaType" ) ) )
    {
        // TODO/LATER: activate when zip ucp is ready
        // if ( m_nFormat != embed::StorageFormats::PACKAGE )
        //  throw UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );

        return makeAny ( sMediaType );
    }
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Version" ) ) )
        return makeAny( m_sVersion );
    else if (PropertyName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Size" ) ) )
        return makeAny ( aEntry.nSize );
    else
        throw UnknownPropertyException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

void ZipPackageFolder::doInsertByName ( ZipPackageEntry *pEntry, sal_Bool bSetParent )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
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
OUString ZipPackageFolder::getImplementationName()
    throw (RuntimeException)
{
    return OUString ( RTL_CONSTASCII_USTRINGPARAM ( "ZipPackageFolder" ) );
}

Sequence< OUString > ZipPackageFolder::getSupportedServiceNames()
    throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = OUString( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.packages.PackageFolder" ) );
    return aNames;
}
sal_Bool SAL_CALL ZipPackageFolder::supportsService( OUString const & rServiceName )
    throw (RuntimeException)
{
    return rServiceName == getSupportedServiceNames()[0];
}
