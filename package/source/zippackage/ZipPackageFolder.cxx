/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <osl/time.h>
#include <rtl/digest.h>
#include <ContentInfo.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <EncryptedDataHeader.hxx>
#include <rtl/random.h>
#include <rtl/instance.hxx>
#include <boost/scoped_ptr.hpp>

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

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

namespace { struct lcl_CachedImplId : public rtl::Static< uno::Sequence < sal_Int8 >, lcl_CachedImplId > {}; }

ZipPackageFolder::ZipPackageFolder ( sal_Int32 nFormat,
                                     sal_Bool bAllowRemoveOnInsert )
: m_nFormat( nFormat )
{
    this->mbAllowRemoveOnInsert = bAllowRemoveOnInsert;

    SetFolder ( true );
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
                    
                    bHasUnexpected = sal_True;
                }

                
            }
            else if ( !rInfo.pStream->IsFromManifest() )
            {
                
                
                if ( !aPath.isEmpty() || rShortName != "mimetype" )
                {
                    
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

    
void SAL_CALL ZipPackageFolder::insertByName( const OUString& aName, const uno::Any& aElement )
        throw(IllegalArgumentException, ElementExistException, WrappedTargetException, uno::RuntimeException)
{
    if (hasByName(aName))
        throw ElementExistException(THROW_WHERE, uno::Reference< uno::XInterface >() );
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
                throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );

            if (pEntry->getName() != aName )
                pEntry->setName (aName);
            doInsertByName ( pEntry, sal_True );
        }
        else
            throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 0 );
    }
}
void SAL_CALL ZipPackageFolder::removeByName( const OUString& Name )
        throw(NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    ContentHash::iterator aIter = maContents.find ( Name );
    if ( aIter == maContents.end() )
        throw NoSuchElementException(THROW_WHERE, uno::Reference< uno::XInterface >() );
    maContents.erase( aIter );
}
    
uno::Reference< XEnumeration > SAL_CALL ZipPackageFolder::createEnumeration(  )
        throw(uno::RuntimeException)
{
    return uno::Reference < XEnumeration> (new ZipPackageFolderEnumeration(maContents));
}
    
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
    
ContentInfo& ZipPackageFolder::doGetByName( const OUString& aName )
    throw(NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    ContentHash::iterator aIter = maContents.find ( aName );
    if ( aIter == maContents.end())
        throw NoSuchElementException(THROW_WHERE, uno::Reference< uno::XInterface >() );
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
    
void SAL_CALL ZipPackageFolder::replaceByName( const OUString& aName, const uno::Any& aElement )
        throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, uno::RuntimeException)
{
    if ( hasByName( aName ) )
        removeByName( aName );
    else
        throw NoSuchElementException(THROW_WHERE, uno::Reference< uno::XInterface >() );
    insertByName(aName, aElement);
}

static void ImplSetStoredData( ZipEntry & rEntry, uno::Reference< XInputStream> & rStream )
{
    
    
    
    
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
        
        
        auto_ptr < ZipEntry > pAutoTempEntry ( new ZipEntry );
        ZipEntry* pTempEntry = pAutoTempEntry.get();

        
        

        ZipPackageFolder::copyZipEntry ( *pTempEntry, rInfo.pStream->aEntry );
        pTempEntry->sPath = rPath + rShortName;
        pTempEntry->nPathLen = (sal_Int16)( OUStringToOString( pTempEntry->sPath, RTL_TEXTENCODING_UTF8 ).getLength() );

        sal_Bool bToBeEncrypted = rInfo.pStream->IsToBeEncrypted() && (rEncryptionKey.getLength() || rInfo.pStream->HasOwnKey());
        sal_Bool bToBeCompressed = bToBeEncrypted ? sal_True : rInfo.pStream->IsToBeCompressed();

        aPropSet[PKG_MNFST_MEDIATYPE].Name = sMediaTypeProperty;
        aPropSet[PKG_MNFST_MEDIATYPE].Value <<= rInfo.pStream->GetMediaType( );
        aPropSet[PKG_MNFST_VERSION].Name = sVersionProperty;
        aPropSet[PKG_MNFST_VERSION].Value <<= OUString(); 
        aPropSet[PKG_MNFST_FULLPATH].Name = sFullPathProperty;
        aPropSet[PKG_MNFST_FULLPATH].Value <<= pTempEntry->sPath;

        OSL_ENSURE( rInfo.pStream->GetStreamMode() != PACKAGE_STREAM_NOTSET, "Unacceptable ZipPackageStream mode!" );

        sal_Bool bRawStream = sal_False;
        if ( rInfo.pStream->GetStreamMode() == PACKAGE_STREAM_DETECT )
            bRawStream = rInfo.pStream->ParsePackageRawStream();
        else if ( rInfo.pStream->GetStreamMode() == PACKAGE_STREAM_RAW )
            bRawStream = sal_True;

        sal_Bool bTransportOwnEncrStreamAsRaw = sal_False;
        
        
        sal_Int64 nOwnStreamOrigSize = bRawStream ? rInfo.pStream->GetMagicalHackSize() : rInfo.pStream->getSize();

        sal_Bool bUseNonSeekableAccess = sal_False;
        uno::Reference < XInputStream > xStream;
        if ( !rInfo.pStream->IsPackageMember() && !bRawStream && !bToBeEncrypted && bToBeCompressed )
        {
            
            
            

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
                    
                    
                    if ( !bToBeCompressed || bRawStream )
                    {
                        
                        OSL_ENSURE( !bRawStream || !(bToBeCompressed || bToBeEncrypted), "The stream is already encrypted!\n" );
                        xSeek->seek ( bRawStream ? rInfo.pStream->GetMagicalHackPos() : 0 );
                        ImplSetStoredData ( *pTempEntry, xStream );

                        
                    }
                    else if ( bToBeEncrypted )
                    {
                        
                        pTempEntry->nSize = xSeek->getLength();
                        nOwnStreamOrigSize = pTempEntry->nSize;
                    }

                    xSeek->seek ( 0 );
                }
                else
                {
                    
                    
                    
                    
                    if ( rInfo.pStream->IsPackageMember() )
                    {
                        
                        if ( rInfo.pStream->IsEncrypted() && rInfo.pStream->IsToBeEncrypted() )
                        {
                            
                            bTransportOwnEncrStreamAsRaw = sal_True;
                            pTempEntry->nMethod = STORED;

                            
                            
                            
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

                
                
                aPropSet.realloc(PKG_SIZE_ENCR_MNFST);

                aPropSet[PKG_MNFST_INIVECTOR].Name = sInitialisationVectorProperty;
                aPropSet[PKG_MNFST_INIVECTOR].Value <<= rInfo.pStream->getInitialisationVector();
                aPropSet[PKG_MNFST_SALT].Name = sSaltProperty;
                aPropSet[PKG_MNFST_SALT].Value <<= rInfo.pStream->getSalt();
                aPropSet[PKG_MNFST_ITERATION].Name = sIterationCountProperty;
                aPropSet[PKG_MNFST_ITERATION].Value <<= rInfo.pStream->getIterationCount ();

                
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

        
        
        if ( !bUseNonSeekableAccess
          && ( bRawStream || bTransportOwnEncrStreamAsRaw
            || ( rInfo.pStream->IsPackageMember() && !bToBeEncrypted
              && ( ( rInfo.pStream->aEntry.nMethod == DEFLATED && bToBeCompressed )
                || ( rInfo.pStream->aEntry.nMethod == STORED && !bToBeCompressed ) ) ) ) )
        {
            
            
            if ( rInfo.pStream->IsPackageMember() )
            {
                xStream = rInfo.pStream->getRawData();
                if ( !xStream.is() )
                {
                    
                    bSuccess = false;
                    return bSuccess;
                }
            }

            try
            {
                if ( bRawStream )
                    xStream->skipBytes( rInfo.pStream->GetMagicalHackPos() );

                rZipOut.putNextEntry ( *pTempEntry, rInfo.pStream, sal_False );
                
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
            

            
            
            
            

            
            
            
            if ( rInfo.pStream->IsPackageMember() )
            {
                xStream = rInfo.pStream->getInputStream();
                if ( !xStream.is() )
                {
                    
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
                
                
                rInfo.pStream->SetToBeEncrypted( sal_True );
            }

            
            ZipPackageFolder::copyZipEntry ( rInfo.pStream->aEntry, *pTempEntry );

            
            if ( rInfo.pStream->aEntry.nFlag & ( 1 << 4 ) )
            {
                rInfo.pStream->aEntry.nFlag &= ~( 1 << 4 );
                rInfo.pStream->aEntry.nMethod = STORED;
            }

            
            if ( rInfo.pStream->IsEncrypted() )
                rInfo.pStream->setSize( nOwnStreamOrigSize );

            rInfo.pStream->aEntry.nOffset *= -1;
        }
    }

    
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
        throw uno::RuntimeException(THROW_WHERE, uno::Reference< uno::XInterface >() );
}

void ZipPackageFolder::releaseUpwardRef( void )
{
    
    
    
    

    
    

    clearParent();

#if 0
    for ( ContentHash::const_iterator aCI = maContents.begin();
          aCI!=maContents.end();
          aCI++)
    {
        ContentInfo &rInfo = * (*aCI).second;
        if ( rInfo.bFolder )
            rInfo.pFolder->releaseUpwardRef();
        else 
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
        
        
        

        aValue >>= sMediaType;
    }
    else if ( aPropertyName == "Version" )
        aValue >>= m_sVersion;
    else if ( aPropertyName == "Size" )
        aValue >>= aEntry.nSize;
    else
        throw UnknownPropertyException(THROW_WHERE, uno::Reference< uno::XInterface >() );
}
uno::Any SAL_CALL ZipPackageFolder::getPropertyValue( const OUString& PropertyName )
        throw(UnknownPropertyException, WrappedTargetException, uno::RuntimeException)
{
    if ( PropertyName == "MediaType" )
    {
        
        
        

        return uno::makeAny ( sMediaType );
    }
    else if ( PropertyName == "Version" )
        return uno::makeAny( m_sVersion );
    else if ( PropertyName == "Size" )
        return uno::makeAny ( aEntry.nSize );
    else
        throw UnknownPropertyException(THROW_WHERE, uno::Reference< uno::XInterface >() );
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
    return cppu::supportsService(this, rServiceName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
