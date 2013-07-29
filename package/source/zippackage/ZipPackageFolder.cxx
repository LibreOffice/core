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

#include <string.h>

#include <ZipPackageFolder.hxx>
#include <ZipFile.hxx>
#include <ZipOutputStream.hxx>
#include <ZipPackageStream.hxx>
#include <PackageConstants.hxx>
#include <ZipPackageFolderEnumeration.hxx>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <osl/diagnose.h>
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
using namespace com::sun::star::packages;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::io;
using namespace cppu;
using namespace std;
using namespace ::com::sun::star;

namespace { struct lcl_CachedImplId : public rtl::Static< uno::Sequence < sal_Int8 >, lcl_CachedImplId > {}; }

ZipPackageFolder::ZipPackageFolder ( sal_Int32 nFormat,
                                     sal_Bool bAllowRemoveOnInsert )
: m_nFormat( nFormat )
{
    this->mbAllowRemoveOnInsert = bAllowRemoveOnInsert;

    SetFolder ( sal_True );
    aEntry.nVersion     = -1;
    aEntry.nFlag        = 0;
    aEntry.nMethod      = STORED;
    aEntry.nTime        = -1;
    aEntry.nCrc         = 0;
    aEntry.nCompressedSize = 0;
    aEntry.nSize        = 0;
    aEntry.nOffset      = -1;
    uno::Sequence < sal_Int8 > &rCachedImplId = lcl_CachedImplId::get();
    if ( !rCachedImplId.getLength() )
        rCachedImplId = getImplementationId();
}

ZipPackageFolder::~ZipPackageFolder()
{
}

sal_Bool ZipPackageFolder::LookForUnexpectedODF12Streams( const OUString& aPath )
{
    sal_Bool bHasUnexpected = sal_False;

    for ( ContentHash::const_iterator aCI = maContents.begin(), aEnd = maContents.end();
          !bHasUnexpected && aCI != aEnd;
          ++aCI)
    {
        const OUString &rShortName = (*aCI).first;
        const ContentInfo &rInfo = *(*aCI).second;

        if ( rInfo.bFolder )
        {
            if ( aPath == "META-INF/" )
            {
                // META-INF is not allowed to contain subfolders
                bHasUnexpected = sal_True;
            }
            else
            {
                OUString sOwnPath = aPath + rShortName + "/";
                bHasUnexpected = rInfo.pFolder->LookForUnexpectedODF12Streams( sOwnPath );
            }
        }
        else
        {
            if ( aPath == "META-INF/" )
            {
                if ( rShortName != "manifest.xml"
                  && rShortName.indexOf( "signatures" ) == -1 )
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
                if ( !aPath.isEmpty() || rShortName != "mimetype" )
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
    OUString aExt;
    if ( aPair.First.toChar() == (sal_Unicode)'.' )
        aExt = aPair.First;
    else
        aExt = "." + aPair.First;

    for ( ContentHash::const_iterator aCI = maContents.begin(), aEnd = maContents.end();
          aCI != aEnd;
          ++aCI)
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
void SAL_CALL ZipPackageFolder::insertByName( const OUString& aName, const uno::Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, uno::RuntimeException)
{
    if (hasByName(aName))
        throw ElementExistException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
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
                throw IllegalArgumentException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 );

            if (pEntry->getName() != aName )
                pEntry->setName (aName);
            doInsertByName ( pEntry, sal_True );
        }
        else
            throw IllegalArgumentException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 0 );
    }
}
void SAL_CALL ZipPackageFolder::removeByName( const OUString& Name )
        throw(NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    ContentHash::iterator aIter = maContents.find ( Name );
    if ( aIter == maContents.end() )
        throw NoSuchElementException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
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
ContentInfo& ZipPackageFolder::doGetByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    ContentHash::iterator aIter = maContents.find ( aName );
    if ( aIter == maContents.end())
        throw NoSuchElementException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    return *(*aIter).second;
}
uno::Any SAL_CALL ZipPackageFolder::getByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    return uno::makeAny ( doGetByName ( aName ).xTunnel );
}
uno::Sequence< OUString > SAL_CALL ZipPackageFolder::getElementNames(  )
        throw(uno::RuntimeException)
{
    sal_uInt32 i=0, nSize = maContents.size();
    uno::Sequence < OUString > aSequence ( nSize );
    for ( ContentHash::const_iterator aIterator = maContents.begin(), aEnd = maContents.end();
          aIterator != aEnd;
          ++i, ++aIterator)
        aSequence[i] = (*aIterator).first;
    return aSequence;
}
sal_Bool SAL_CALL ZipPackageFolder::hasByName( const OUString& aName )
    throw(uno::RuntimeException)
{
    return maContents.find ( aName ) != maContents.end ();
}
    // XNameReplace
void SAL_CALL ZipPackageFolder::replaceByName( const OUString& aName, const uno::Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    if ( hasByName( aName ) )
        removeByName( aName );
    else
        throw NoSuchElementException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
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

bool ZipPackageFolder::saveChild( const OUString &rShortName, const ContentInfo &rInfo, OUString &rPath, std::vector < uno::Sequence < PropertyValue > > &rManList, ZipOutputStream & rZipOut, const uno::Sequence < sal_Int8 >& rEncryptionKey, rtlRandomPool &rRandomPool)
{
    bool bSuccess = true;

    const OUString sMediaTypeProperty ("MediaType");
    const OUString sVersionProperty ("Version");
    const OUString sFullPathProperty ("FullPath");
    const OUString sInitialisationVectorProperty ("InitialisationVector");
    const OUString sSaltProperty ("Salt");
    const OUString sIterationCountProperty ("IterationCount");
    const OUString sSizeProperty ("Size");
    const OUString sDigestProperty ("Digest");
    const OUString sEncryptionAlgProperty    ("EncryptionAlgorithm");
    const OUString sStartKeyAlgProperty  ("StartKeyAlgorithm");
    const OUString sDigestAlgProperty    ("DigestAlgorithm");
    const OUString  sDerivedKeySizeProperty  ("DerivedKeySize");

    uno::Sequence < PropertyValue > aPropSet (PKG_SIZE_NOENCR_MNFST);

    OSL_ENSURE( ( rInfo.bFolder && rInfo.pFolder ) || ( !rInfo.bFolder && rInfo.pStream ), "A valid child object is expected!" );
    if ( rInfo.bFolder )
    {
        OUString sTempName = rPath + rShortName + "/";

        if ( !rInfo.pFolder->GetMediaType().isEmpty() )
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
        pTempEntry->nPathLen = (sal_Int16)( OUStringToOString( pTempEntry->sPath, RTL_TEXTENCODING_UTF8 ).getLength() );

        sal_Bool bToBeEncrypted = rInfo.pStream->IsToBeEncrypted() && (rEncryptionKey.getLength() || rInfo.pStream->HasOwnKey());
        sal_Bool bToBeCompressed = bToBeEncrypted ? sal_True : rInfo.pStream->IsToBeCompressed();

        aPropSet[PKG_MNFST_MEDIATYPE].Name = sMediaTypeProperty;
        aPropSet[PKG_MNFST_MEDIATYPE].Value <<= rInfo.pStream->GetMediaType( );
        aPropSet[PKG_MNFST_VERSION].Name = sVersionProperty;
        aPropSet[PKG_MNFST_VERSION].Value <<= OUString(); // no version is stored for streams currently
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
        sal_Int64 nOwnStreamOrigSize = bRawStream ? rInfo.pStream->GetMagicalHackSize() : rInfo.pStream->getSize();

        sal_Bool bUseNonSeekableAccess = sal_False;
        uno::Reference < XInputStream > xStream;
        if ( !rInfo.pStream->IsPackageMember() && !bRawStream && !bToBeEncrypted && bToBeCompressed )
        {
            // the stream is not a package member, not a raw stream,
            // it should not be encrypted and it should be compressed,
            // in this case nonseekable access can be used

            xStream = rInfo.pStream->GetOwnStreamNoWrap();
            uno::Reference < XSeekable > xSeek ( xStream, uno::UNO_QUERY );

            bUseNonSeekableAccess = ( xStream.is() && !xSeek.is() );
        }

        if ( !bUseNonSeekableAccess )
        {
            xStream = rInfo.pStream->getRawData();

            if ( !xStream.is() )
            {
                OSL_FAIL( "ZipPackageStream didn't have a stream associated with it, skipping!" );
                bSuccess = false;
                return bSuccess;
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
                        OSL_ENSURE( !bRawStream || !(bToBeCompressed || bToBeEncrypted), "The stream is already encrypted!\n" );
                        xSeek->seek ( bRawStream ? rInfo.pStream->GetMagicalHackPos() : 0 );
                        ImplSetStoredData ( *pTempEntry, xStream );

                        // TODO/LATER: Get rid of hacks related to switching of Flag Method and Size properties!
                    }
                    else if ( bToBeEncrypted )
                    {
                        // this is the correct original size
                        pTempEntry->nSize = xSeek->getLength();
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
                        bSuccess = false;
                        return bSuccess;
                    }
                }
            }
            catch ( uno::Exception& )
            {
                bSuccess = false;
                return bSuccess;
            }

            if ( bToBeEncrypted || bRawStream || bTransportOwnEncrStreamAsRaw )
            {
                if ( bToBeEncrypted && !bTransportOwnEncrStreamAsRaw )
                {
                    uno::Sequence < sal_Int8 > aSalt( 16 ), aVector( rInfo.pStream->GetBlockSize() );
                    rtl_random_getBytes ( rRandomPool, aSalt.getArray(), 16 );
                    rtl_random_getBytes ( rRandomPool, aVector.getArray(), aVector.getLength() );
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
                    ::rtl::Reference< EncryptionData > xEncData = rInfo.pStream->GetEncryptionData();
                    if ( !xEncData.is() )
                        throw uno::RuntimeException();

                    aPropSet[PKG_MNFST_DIGEST].Name = sDigestProperty;
                    aPropSet[PKG_MNFST_DIGEST].Value <<= rInfo.pStream->getDigest();
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
                    bSuccess = false;
                    return bSuccess;
                }
            }

            try
            {
                if ( bRawStream )
                    xStream->skipBytes( rInfo.pStream->GetMagicalHackPos() );

                rZipOut.putNextEntry ( *pTempEntry, rInfo.pStream, sal_False );
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
                bSuccess = false;
            }
            catch ( IOException& )
            {
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
                    bSuccess = false;
                    return bSuccess;
                }
            }

            if ( bToBeCompressed )
            {
                pTempEntry->nMethod = DEFLATED;
                pTempEntry->nCrc = -1;
                pTempEntry->nCompressedSize = pTempEntry->nSize = -1;
            }

            try
            {
                rZipOut.putNextEntry ( *pTempEntry, rInfo.pStream, bToBeEncrypted);
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
                bSuccess = false;
            }
            catch ( IOException& )
            {
                bSuccess = false;
            }

            if ( bToBeEncrypted )
            {
                ::rtl::Reference< EncryptionData > xEncData = rInfo.pStream->GetEncryptionData();
                if ( !xEncData.is() )
                    throw uno::RuntimeException();

                aPropSet[PKG_MNFST_DIGEST].Name = sDigestProperty;
                aPropSet[PKG_MNFST_DIGEST].Value <<= rInfo.pStream->getDigest();
                aPropSet[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
                aPropSet[PKG_MNFST_ENCALG].Value <<= xEncData->m_nEncAlg;
                aPropSet[PKG_MNFST_STARTALG].Name = sStartKeyAlgProperty;
                aPropSet[PKG_MNFST_STARTALG].Value <<= xEncData->m_nStartKeyGenID;
                aPropSet[PKG_MNFST_DIGESTALG].Name = sDigestAlgProperty;
                aPropSet[PKG_MNFST_DIGESTALG].Value <<= xEncData->m_nCheckAlg;
                aPropSet[PKG_MNFST_DERKEYSIZE].Name = sDerivedKeySizeProperty;
                aPropSet[PKG_MNFST_DERKEYSIZE].Value <<= xEncData->m_nDerivedKeySize;

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

            // Then copy it back afterwards...
            ZipPackageFolder::copyZipEntry ( rInfo.pStream->aEntry, *pTempEntry );

            // Remove hacky bit from entry flags
            if ( rInfo.pStream->aEntry.nFlag & ( 1 << 4 ) )
            {
                rInfo.pStream->aEntry.nFlag &= ~( 1 << 4 );
                rInfo.pStream->aEntry.nMethod = STORED;
            }

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

void ZipPackageFolder::saveContents( OUString &rPath, std::vector < uno::Sequence < PropertyValue > > &rManList, ZipOutputStream & rZipOut, const uno::Sequence < sal_Int8 >& rEncryptionKey, rtlRandomPool &rRandomPool )
    throw( uno::RuntimeException )
{
    bool bWritingFailed = false;

    if ( maContents.begin() == maContents.end() && !rPath.isEmpty() && m_nFormat != embed::StorageFormats::OFOPXML )
    {
        // it is an empty subfolder, use workaround to store it
        ZipEntry* pTempEntry = new ZipEntry();
        ZipPackageFolder::copyZipEntry ( *pTempEntry, aEntry );
        pTempEntry->nPathLen = (sal_Int16)( OUStringToOString( rPath, RTL_TEXTENCODING_UTF8 ).getLength() );
        pTempEntry->nExtraLen = -1;
        pTempEntry->sPath = rPath;

        try
        {
            rZipOut.putNextEntry( *pTempEntry, NULL, sal_False );
            rZipOut.rawCloseEntry();
        }
        catch ( ZipException& )
        {
            bWritingFailed = true;
        }
        catch ( IOException& )
        {
            bWritingFailed = true;
        }
    }

    bool bMimeTypeStreamStored = false;
    OUString aMimeTypeStreamName("mimetype");
    if ( m_nFormat == embed::StorageFormats::ZIP && rPath.isEmpty() )
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
          ++aCI)
    {
        const OUString &rShortName = (*aCI).first;
        const ContentInfo &rInfo = *(*aCI).second;

        if ( !bMimeTypeStreamStored || !rShortName.equals( aMimeTypeStreamName ) )
            bWritingFailed = !saveChild( rShortName, rInfo, rPath, rManList, rZipOut, rEncryptionKey, rRandomPool );
    }

    if( bWritingFailed )
        throw uno::RuntimeException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
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

    OSL_ENSURE ( m_refCount == 1, "Ref-count is not 1!" );
#endif
}

sal_Int64 SAL_CALL ZipPackageFolder::getSomething( const uno::Sequence< sal_Int8 >& aIdentifier )
    throw(uno::RuntimeException)
{
    sal_Int64 nMe = 0;
    if ( aIdentifier.getLength() == 16 &&
         0 == memcmp(static_getImplementationId().getConstArray(),  aIdentifier.getConstArray(), 16 ) )
        nMe = reinterpret_cast < sal_Int64 > ( this );
    return nMe;
}
void SAL_CALL ZipPackageFolder::setPropertyValue( const OUString& aPropertyName, const uno::Any& aValue )
        throw(UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, uno::RuntimeException)
{
    if ( aPropertyName == "MediaType" )
    {
        // TODO/LATER: activate when zip ucp is ready
        // if ( m_nFormat != embed::StorageFormats::PACKAGE )
        //  throw UnknownPropertyException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        aValue >>= sMediaType;
    }
    else if ( aPropertyName == "Version" )
        aValue >>= m_sVersion;
    else if ( aPropertyName == "Size" )
        aValue >>= aEntry.nSize;
    else
        throw UnknownPropertyException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
}
uno::Any SAL_CALL ZipPackageFolder::getPropertyValue( const OUString& PropertyName )
        throw(UnknownPropertyException, WrappedTargetException, uno::RuntimeException)
{
    if ( PropertyName == "MediaType" )
    {
        // TODO/LATER: activate when zip ucp is ready
        // if ( m_nFormat != embed::StorageFormats::PACKAGE )
        //  throw UnknownPropertyException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

        return uno::makeAny ( sMediaType );
    }
    else if ( PropertyName == "Version" )
        return uno::makeAny( m_sVersion );
    else if ( PropertyName == "Size" )
        return uno::makeAny ( aEntry.nSize );
    else
        throw UnknownPropertyException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
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
OUString ZipPackageFolder::getImplementationName()
    throw (uno::RuntimeException)
{
    return OUString("ZipPackageFolder");
}

uno::Sequence< OUString > ZipPackageFolder::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    uno::Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.packages.PackageFolder";
    return aNames;
}
sal_Bool SAL_CALL ZipPackageFolder::supportsService( OUString const & rServiceName )
    throw (uno::RuntimeException)
{
    return rServiceName == getSupportedServiceNames()[0];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
