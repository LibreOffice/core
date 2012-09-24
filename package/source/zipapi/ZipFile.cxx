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


#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>
#include <com/sun/star/xml/crypto/XCipherContextSupplier.hpp>
#include <com/sun/star/xml/crypto/XDigestContextSupplier.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>

#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <rtl/digest.h>

#include <vector>

#include "blowfishcontext.hxx"
#include "sha1context.hxx"
#include <ZipFile.hxx>
#include <ZipEnumeration.hxx>
#include <XUnbufferedStream.hxx>
#include <PackageConstants.hxx>
#include <EncryptedDataHeader.hxx>
#include <EncryptionData.hxx>
#include <MemoryByteGrabber.hxx>

#include <CRC32.hxx>

#define AES_CBC_BLOCK_SIZE 16

using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::lang;
using namespace com::sun::star::packages;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::zip::ZipConstants;

using rtl::OUString;
using ZipUtils::Inflater;

/** This class is used to read entries from a zip file
 */
ZipFile::ZipFile( uno::Reference < XInputStream > &xInput, const uno::Reference < XMultiServiceFactory > &xNewFactory, sal_Bool bInitialise )
    throw(IOException, ZipException, RuntimeException)
: aGrabber(xInput)
, aInflater (sal_True)
, xStream(xInput)
, xSeek(xInput, UNO_QUERY)
, m_xFactory ( xNewFactory )
, bRecoveryMode( sal_False )
{
    if (bInitialise)
    {
        if ( readCEN() == -1 )
        {
            aEntries.clear();
            throw ZipException( "stream data looks to be broken", uno::Reference < XInterface > () );
        }
    }
}



ZipFile::ZipFile( uno::Reference < XInputStream > &xInput, const uno::Reference < XMultiServiceFactory > &xNewFactory, sal_Bool bInitialise, sal_Bool bForceRecovery, uno::Reference < XProgressHandler > xProgress )
    throw(IOException, ZipException, RuntimeException)
: aGrabber(xInput)
, aInflater (sal_True)
, xStream(xInput)
, xSeek(xInput, UNO_QUERY)
, m_xFactory ( xNewFactory )
, xProgressHandler( xProgress )
, bRecoveryMode( bForceRecovery )
{
    if (bInitialise)
    {
        if ( bForceRecovery )
        {
            recover();
        }
        else if ( readCEN() == -1 )
        {
            aEntries.clear();
            throw ZipException("stream data looks to be broken", uno::Reference < XInterface > () );
        }
    }
}

ZipFile::~ZipFile()
{
    aEntries.clear();
}

void ZipFile::setInputStream ( uno::Reference < XInputStream > xNewStream )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    xStream = xNewStream;
    xSeek = uno::Reference < XSeekable > ( xStream, UNO_QUERY );
    aGrabber.setInputStream ( xStream );
}

uno::Reference< xml::crypto::XDigestContext > ZipFile::StaticGetDigestContextForChecksum( const uno::Reference< lang::XMultiServiceFactory >& xArgFactory, const ::rtl::Reference< EncryptionData >& xEncryptionData )
{
    uno::Reference< xml::crypto::XDigestContext > xDigestContext;
    if ( xEncryptionData->m_nCheckAlg == xml::crypto::DigestID::SHA256_1K )
    {
        uno::Reference< lang::XMultiServiceFactory > xFactory = xArgFactory;
        if ( !xFactory.is() )
            xFactory.set( comphelper::getProcessServiceFactory(), uno::UNO_SET_THROW );

        uno::Reference< xml::crypto::XDigestContextSupplier > xDigestContextSupplier(
            xFactory->createInstance("com.sun.star.xml.crypto.NSSInitializer"),
            uno::UNO_QUERY_THROW );

        xDigestContext.set( xDigestContextSupplier->getDigestContext( xEncryptionData->m_nCheckAlg, uno::Sequence< beans::NamedValue >() ), uno::UNO_SET_THROW );
    }
    else if ( xEncryptionData->m_nCheckAlg == xml::crypto::DigestID::SHA1_1K )
        xDigestContext.set( SHA1DigestContext::Create(), uno::UNO_SET_THROW );

    return xDigestContext;
}

uno::Reference< xml::crypto::XCipherContext > ZipFile::StaticGetCipher( const uno::Reference< lang::XMultiServiceFactory >& xArgFactory, const ::rtl::Reference< EncryptionData >& xEncryptionData, bool bEncrypt )
{
    uno::Reference< xml::crypto::XCipherContext > xResult;

    try
    {
        if (xEncryptionData->m_nDerivedKeySize < 0)
        {
            throw ZipIOException("Invalid derived key length!",
                                  uno::Reference< XInterface >() );
        }

        uno::Sequence< sal_Int8 > aDerivedKey( xEncryptionData->m_nDerivedKeySize );
        if ( rtl_Digest_E_None != rtl_digest_PBKDF2( reinterpret_cast< sal_uInt8* >( aDerivedKey.getArray() ),
                            aDerivedKey.getLength(),
                            reinterpret_cast< const sal_uInt8 * > (xEncryptionData->m_aKey.getConstArray() ),
                            xEncryptionData->m_aKey.getLength(),
                            reinterpret_cast< const sal_uInt8 * > ( xEncryptionData->m_aSalt.getConstArray() ),
                            xEncryptionData->m_aSalt.getLength(),
                            xEncryptionData->m_nIterationCount ) )
        {
            throw ZipIOException("Can not create derived key!",
                                  uno::Reference< XInterface >() );
        }

        if ( xEncryptionData->m_nEncAlg == xml::crypto::CipherID::AES_CBC_W3C_PADDING )
        {
            uno::Reference< lang::XMultiServiceFactory > xFactory = xArgFactory;
            if ( !xFactory.is() )
                xFactory.set( comphelper::getProcessServiceFactory(), uno::UNO_SET_THROW );

            uno::Reference< xml::crypto::XCipherContextSupplier > xCipherContextSupplier(
                xFactory->createInstance("com.sun.star.xml.crypto.NSSInitializer"),
                uno::UNO_QUERY_THROW );

            xResult = xCipherContextSupplier->getCipherContext( xEncryptionData->m_nEncAlg, aDerivedKey, xEncryptionData->m_aInitVector, bEncrypt, uno::Sequence< beans::NamedValue >() );
        }
        else if ( xEncryptionData->m_nEncAlg == xml::crypto::CipherID::BLOWFISH_CFB_8 )
        {
            xResult = BlowfishCFB8CipherContext::Create( aDerivedKey, xEncryptionData->m_aInitVector, bEncrypt );
        }
        else
        {
            throw ZipIOException("Unknown cipher algorithm is requested!",
                                  uno::Reference< XInterface >() );
        }
    }
    catch( ... )
    {
        OSL_ENSURE( sal_False, "Can not create cipher context!" );
    }

    return xResult;
}

void ZipFile::StaticFillHeader( const ::rtl::Reference< EncryptionData >& rData,
                                sal_Int64 nSize,
                                const ::rtl::OUString& aMediaType,
                                sal_Int8 * & pHeader )
{
    // I think it's safe to restrict vector and salt length to 2 bytes !
    sal_Int16 nIVLength = static_cast < sal_Int16 > ( rData->m_aInitVector.getLength() );
    sal_Int16 nSaltLength = static_cast < sal_Int16 > ( rData->m_aSalt.getLength() );
    sal_Int16 nDigestLength = static_cast < sal_Int16 > ( rData->m_aDigest.getLength() );
    sal_Int16 nMediaTypeLength = static_cast < sal_Int16 > ( aMediaType.getLength() * sizeof( sal_Unicode ) );

    // First the header
    *(pHeader++) = ( n_ConstHeader >> 0 ) & 0xFF;
    *(pHeader++) = ( n_ConstHeader >> 8 ) & 0xFF;
    *(pHeader++) = ( n_ConstHeader >> 16 ) & 0xFF;
    *(pHeader++) = ( n_ConstHeader >> 24 ) & 0xFF;

    // Then the version
    *(pHeader++) = ( n_ConstCurrentVersion >> 0 ) & 0xFF;
    *(pHeader++) = ( n_ConstCurrentVersion >> 8 ) & 0xFF;

    // Then the iteration Count
    sal_Int32 nIterationCount = rData->m_nIterationCount;
    *(pHeader++) = static_cast< sal_Int8 >(( nIterationCount >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nIterationCount >> 8 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nIterationCount >> 16 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nIterationCount >> 24 ) & 0xFF);

    // FIXME64: need to handle larger sizes
    // Then the size:
    *(pHeader++) = static_cast< sal_Int8 >(( nSize >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nSize >> 8 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nSize >> 16 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nSize >> 24 ) & 0xFF);

    // Then the encryption algorithm
    sal_Int32 nEncAlgID = rData->m_nEncAlg;
    *(pHeader++) = static_cast< sal_Int8 >(( nEncAlgID >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nEncAlgID >> 8 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nEncAlgID >> 16 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nEncAlgID >> 24 ) & 0xFF);

    // Then the checksum algorithm
    sal_Int32 nChecksumAlgID = rData->m_nCheckAlg;
    *(pHeader++) = static_cast< sal_Int8 >(( nChecksumAlgID >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nChecksumAlgID >> 8 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nChecksumAlgID >> 16 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nChecksumAlgID >> 24 ) & 0xFF);

    // Then the derived key size
    sal_Int32 nDerivedKeySize = rData->m_nDerivedKeySize;
    *(pHeader++) = static_cast< sal_Int8 >(( nDerivedKeySize >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nDerivedKeySize >> 8 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nDerivedKeySize >> 16 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nDerivedKeySize >> 24 ) & 0xFF);

    // Then the start key generation algorithm
    sal_Int32 nKeyAlgID = rData->m_nStartKeyGenID;
    *(pHeader++) = static_cast< sal_Int8 >(( nKeyAlgID >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nKeyAlgID >> 8 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nKeyAlgID >> 16 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nKeyAlgID >> 24 ) & 0xFF);

    // Then the salt length
    *(pHeader++) = static_cast< sal_Int8 >(( nSaltLength >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nSaltLength >> 8 ) & 0xFF);

    // Then the IV length
    *(pHeader++) = static_cast< sal_Int8 >(( nIVLength >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nIVLength >> 8 ) & 0xFF);

    // Then the digest length
    *(pHeader++) = static_cast< sal_Int8 >(( nDigestLength >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nDigestLength >> 8 ) & 0xFF);

    // Then the mediatype length
    *(pHeader++) = static_cast< sal_Int8 >(( nMediaTypeLength >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nMediaTypeLength >> 8 ) & 0xFF);

    // Then the salt content
    memcpy ( pHeader, rData->m_aSalt.getConstArray(), nSaltLength );
    pHeader += nSaltLength;

    // Then the IV content
    memcpy ( pHeader, rData->m_aInitVector.getConstArray(), nIVLength );
    pHeader += nIVLength;

    // Then the digest content
    memcpy ( pHeader, rData->m_aDigest.getConstArray(), nDigestLength );
    pHeader += nDigestLength;

    // Then the mediatype itself
    memcpy ( pHeader, aMediaType.getStr(), nMediaTypeLength );
    pHeader += nMediaTypeLength;
}

sal_Bool ZipFile::StaticFillData (  ::rtl::Reference< BaseEncryptionData > & rData,
                                    sal_Int32 &rEncAlg,
                                    sal_Int32 &rChecksumAlg,
                                    sal_Int32 &rDerivedKeySize,
                                    sal_Int32 &rStartKeyGenID,
                                    sal_Int32 &rSize,
                                    ::rtl::OUString& aMediaType,
                                    const uno::Reference< XInputStream >& rStream )
{
    sal_Bool bOk = sal_False;
    const sal_Int32 nHeaderSize = n_ConstHeaderSize - 4;
    Sequence < sal_Int8 > aBuffer ( nHeaderSize );
    if ( nHeaderSize == rStream->readBytes ( aBuffer, nHeaderSize ) )
    {
        sal_Int16 nPos = 0;
        sal_Int8 *pBuffer = aBuffer.getArray();
        sal_Int16 nVersion = pBuffer[nPos++] & 0xFF;
        nVersion |= ( pBuffer[nPos++] & 0xFF ) << 8;
        if ( nVersion == n_ConstCurrentVersion )
        {
            sal_Int32 nCount = pBuffer[nPos++] & 0xFF;
            nCount |= ( pBuffer[nPos++] & 0xFF ) << 8;
            nCount |= ( pBuffer[nPos++] & 0xFF ) << 16;
            nCount |= ( pBuffer[nPos++] & 0xFF ) << 24;
            rData->m_nIterationCount = nCount;

            rSize  =   pBuffer[nPos++] & 0xFF;
            rSize |= ( pBuffer[nPos++] & 0xFF ) << 8;
            rSize |= ( pBuffer[nPos++] & 0xFF ) << 16;
            rSize |= ( pBuffer[nPos++] & 0xFF ) << 24;

            rEncAlg   =   pBuffer[nPos++] & 0xFF;
            rEncAlg  |= ( pBuffer[nPos++] & 0xFF ) << 8;
            rEncAlg  |= ( pBuffer[nPos++] & 0xFF ) << 16;
            rEncAlg  |= ( pBuffer[nPos++] & 0xFF ) << 24;

            rChecksumAlg   =   pBuffer[nPos++] & 0xFF;
            rChecksumAlg  |= ( pBuffer[nPos++] & 0xFF ) << 8;
            rChecksumAlg  |= ( pBuffer[nPos++] & 0xFF ) << 16;
            rChecksumAlg  |= ( pBuffer[nPos++] & 0xFF ) << 24;

            rDerivedKeySize   =   pBuffer[nPos++] & 0xFF;
            rDerivedKeySize  |= ( pBuffer[nPos++] & 0xFF ) << 8;
            rDerivedKeySize  |= ( pBuffer[nPos++] & 0xFF ) << 16;
            rDerivedKeySize  |= ( pBuffer[nPos++] & 0xFF ) << 24;

            rStartKeyGenID   =   pBuffer[nPos++] & 0xFF;
            rStartKeyGenID  |= ( pBuffer[nPos++] & 0xFF ) << 8;
            rStartKeyGenID  |= ( pBuffer[nPos++] & 0xFF ) << 16;
            rStartKeyGenID  |= ( pBuffer[nPos++] & 0xFF ) << 24;

            sal_Int16 nSaltLength =   pBuffer[nPos++] & 0xFF;
            nSaltLength          |= ( pBuffer[nPos++] & 0xFF ) << 8;
            sal_Int16 nIVLength   = ( pBuffer[nPos++] & 0xFF );
            nIVLength            |= ( pBuffer[nPos++] & 0xFF ) << 8;
            sal_Int16 nDigestLength = pBuffer[nPos++] & 0xFF;
            nDigestLength        |= ( pBuffer[nPos++] & 0xFF ) << 8;

            sal_Int16 nMediaTypeLength = pBuffer[nPos++] & 0xFF;
            nMediaTypeLength |= ( pBuffer[nPos++] & 0xFF ) << 8;

            if ( nSaltLength == rStream->readBytes ( aBuffer, nSaltLength ) )
            {
                rData->m_aSalt.realloc ( nSaltLength );
                memcpy ( rData->m_aSalt.getArray(), aBuffer.getConstArray(), nSaltLength );
                if ( nIVLength == rStream->readBytes ( aBuffer, nIVLength ) )
                {
                    rData->m_aInitVector.realloc ( nIVLength );
                    memcpy ( rData->m_aInitVector.getArray(), aBuffer.getConstArray(), nIVLength );
                    if ( nDigestLength == rStream->readBytes ( aBuffer, nDigestLength ) )
                    {
                        rData->m_aDigest.realloc ( nDigestLength );
                        memcpy ( rData->m_aDigest.getArray(), aBuffer.getConstArray(), nDigestLength );

                        if ( nMediaTypeLength == rStream->readBytes ( aBuffer, nMediaTypeLength ) )
                        {
                            aMediaType = ::rtl::OUString( (sal_Unicode*)aBuffer.getConstArray(),
                                                            nMediaTypeLength / sizeof( sal_Unicode ) );
                            bOk = sal_True;
                        }
                    }
                }
            }
        }
    }
    return bOk;
}

uno::Reference< XInputStream > ZipFile::StaticGetDataFromRawStream( const uno::Reference< lang::XMultiServiceFactory >& xFactory,
                                                                const uno::Reference< XInputStream >& xStream,
                                                                const ::rtl::Reference< EncryptionData > &rData )
        throw ( packages::WrongPasswordException, ZipIOException, RuntimeException )
{
    if ( !rData.is() )
        throw ZipIOException("Encrypted stream without encryption data!\n",
                            uno::Reference< XInterface >() );

    if ( !rData->m_aKey.getLength() )
        throw packages::WrongPasswordException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    uno::Reference< XSeekable > xSeek( xStream, UNO_QUERY );
    if ( !xSeek.is() )
        throw ZipIOException("The stream must be seekable!\n",
                            uno::Reference< XInterface >() );


    // if we have a digest, then this file is an encrypted one and we should
    // check if we can decrypt it or not
    OSL_ENSURE( rData->m_aDigest.getLength(), "Can't detect password correctness without digest!\n" );
    if ( rData->m_aDigest.getLength() )
    {
        sal_Int32 nSize = sal::static_int_cast< sal_Int32 >( xSeek->getLength() );
        if ( nSize > n_ConstDigestLength + 32 )
            nSize = n_ConstDigestLength + 32;

        // skip header
        xSeek->seek( n_ConstHeaderSize + rData->m_aInitVector.getLength() +
                                rData->m_aSalt.getLength() + rData->m_aDigest.getLength() );

        // Only want to read enough to verify the digest
        Sequence < sal_Int8 > aReadBuffer ( nSize );

        xStream->readBytes( aReadBuffer, nSize );

        if ( !StaticHasValidPassword( xFactory, aReadBuffer, rData ) )
            throw packages::WrongPasswordException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }

    return new XUnbufferedStream( xFactory, xStream, rData );
}

#if 0
// for debugging purposes
void CheckSequence( const uno::Sequence< sal_Int8 >& aSequence )
{
    if ( aSequence.getLength() )
    {
        sal_Int32* pPointer = *( (sal_Int32**)&aSequence );
        sal_Int32 nSize = *( pPointer + 1 );
        sal_Int32 nMemSize = *( pPointer - 2 );
        sal_Int32 nUsedMemSize = ( nSize + 4 * sizeof( sal_Int32 ) );
        OSL_ENSURE( nSize == aSequence.getLength() && nUsedMemSize + 7 - ( nUsedMemSize - 1 ) % 8 == nMemSize, "Broken Sequence!" );
    }
}
#endif

sal_Bool ZipFile::StaticHasValidPassword( const uno::Reference< lang::XMultiServiceFactory >& xFactory, const Sequence< sal_Int8 > &aReadBuffer, const ::rtl::Reference< EncryptionData > &rData )
{
    if ( !rData.is() || !rData->m_aKey.getLength() )
        return sal_False;

    sal_Bool bRet = sal_False;

    uno::Reference< xml::crypto::XCipherContext > xCipher( StaticGetCipher( xFactory, rData, false ), uno::UNO_SET_THROW );

    uno::Sequence< sal_Int8 > aDecryptBuffer;
    uno::Sequence< sal_Int8 > aDecryptBuffer2;
    try
    {
        aDecryptBuffer = xCipher->convertWithCipherContext( aReadBuffer );
        aDecryptBuffer2 = xCipher->finalizeCipherContextAndDispose();
    }
    catch( uno::Exception& )
    {
        // decryption with padding will throw the exception in finalizing if the buffer represent only part of the stream
        // it is no problem, actually this is why we read 32 additional bytes ( two of maximal possible encryption blocks )
    }

    if ( aDecryptBuffer2.getLength() )
    {
        sal_Int32 nOldLen = aDecryptBuffer.getLength();
        aDecryptBuffer.realloc( nOldLen + aDecryptBuffer2.getLength() );
        memcpy( aDecryptBuffer.getArray() + nOldLen, aDecryptBuffer2.getArray(), aDecryptBuffer2.getLength() );
    }

    if ( aDecryptBuffer.getLength() > n_ConstDigestLength )
        aDecryptBuffer.realloc( n_ConstDigestLength );

    uno::Sequence< sal_Int8 > aDigestSeq;
    uno::Reference< xml::crypto::XDigestContext > xDigestContext( StaticGetDigestContextForChecksum( xFactory, rData ), uno::UNO_SET_THROW );

    xDigestContext->updateDigest( aDecryptBuffer );
    aDigestSeq = xDigestContext->finalizeDigestAndDispose();

    // If we don't have a digest, then we have to assume that the password is correct
    if (  rData->m_aDigest.getLength() != 0  &&
          ( aDigestSeq.getLength() != rData->m_aDigest.getLength() ||
            0 != memcmp ( aDigestSeq.getConstArray(),
                                     rData->m_aDigest.getConstArray(),
                                    aDigestSeq.getLength() ) ) )
    {
        // We should probably tell the user that the password they entered was wrong
    }
    else
        bRet = sal_True;

    return bRet;
}

sal_Bool ZipFile::hasValidPassword ( ZipEntry & rEntry, const ::rtl::Reference< EncryptionData >& rData )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Bool bRet = sal_False;
    if ( rData.is() && rData->m_aKey.getLength() )
    {
        xSeek->seek( rEntry.nOffset );
        sal_Int64 nSize = rEntry.nMethod == DEFLATED ? rEntry.nCompressedSize : rEntry.nSize;

        // Only want to read enough to verify the digest
        if ( nSize > n_ConstDigestDecrypt )
            nSize = n_ConstDigestDecrypt;

        Sequence < sal_Int8 > aReadBuffer ( nSize );

        xStream->readBytes( aReadBuffer, nSize );

        bRet = StaticHasValidPassword( m_xFactory, aReadBuffer, rData );
    }

    return bRet;
}

uno::Reference< XInputStream > ZipFile::createUnbufferedStream(
            SotMutexHolderRef aMutexHolder,
            ZipEntry & rEntry,
            const ::rtl::Reference< EncryptionData > &rData,
            sal_Int8 nStreamMode,
            sal_Bool bIsEncrypted,
            ::rtl::OUString aMediaType )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return new XUnbufferedStream ( m_xFactory, aMutexHolder, rEntry, xStream, rData, nStreamMode, bIsEncrypted, aMediaType, bRecoveryMode );
}


ZipEnumeration * SAL_CALL ZipFile::entries(  )
{
    return new ZipEnumeration ( aEntries );
}

uno::Reference< XInputStream > SAL_CALL ZipFile::getInputStream( ZipEntry& rEntry,
        const ::rtl::Reference< EncryptionData > &rData,
        sal_Bool bIsEncrypted,
        SotMutexHolderRef aMutexHolder )
    throw(IOException, ZipException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( rEntry.nOffset <= 0 )
        readLOC( rEntry );

    // We want to return a rawStream if we either don't have a key or if the
    // key is wrong

    sal_Bool bNeedRawStream = rEntry.nMethod == STORED;

    // if we have a digest, then this file is an encrypted one and we should
    // check if we can decrypt it or not
    if ( bIsEncrypted && rData.is() && rData->m_aDigest.getLength() )
        bNeedRawStream = !hasValidPassword ( rEntry, rData );

    return createUnbufferedStream ( aMutexHolder,
                                    rEntry,
                                    rData,
                                    bNeedRawStream ? UNBUFF_STREAM_RAW : UNBUFF_STREAM_DATA,
                                    bIsEncrypted );
}

uno::Reference< XInputStream > SAL_CALL ZipFile::getDataStream( ZipEntry& rEntry,
        const ::rtl::Reference< EncryptionData > &rData,
        sal_Bool bIsEncrypted,
        SotMutexHolderRef aMutexHolder )
    throw ( packages::WrongPasswordException,
            IOException,
            ZipException,
            RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( rEntry.nOffset <= 0 )
        readLOC( rEntry );

    // An exception must be thrown in case stream is encrypted and
    // there is no key or the key is wrong
    sal_Bool bNeedRawStream = sal_False;
    if ( bIsEncrypted )
    {
        // in case no digest is provided there is no way
        // to detect password correctness
        if ( !rData.is() )
            throw ZipException("Encrypted stream without encryption data!\n",
                                uno::Reference< XInterface >() );

        // if we have a digest, then this file is an encrypted one and we should
        // check if we can decrypt it or not
        OSL_ENSURE( rData->m_aDigest.getLength(), "Can't detect password correctness without digest!\n" );
        if ( rData->m_aDigest.getLength() && !hasValidPassword ( rEntry, rData ) )
                throw packages::WrongPasswordException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
    }
    else
        bNeedRawStream = ( rEntry.nMethod == STORED );

    return createUnbufferedStream ( aMutexHolder,
                                    rEntry,
                                    rData,
                                    bNeedRawStream ? UNBUFF_STREAM_RAW : UNBUFF_STREAM_DATA,
                                    bIsEncrypted );
}

uno::Reference< XInputStream > SAL_CALL ZipFile::getRawData( ZipEntry& rEntry,
        const ::rtl::Reference< EncryptionData >& rData,
        sal_Bool bIsEncrypted,
        SotMutexHolderRef aMutexHolder )
    throw(IOException, ZipException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( rEntry.nOffset <= 0 )
        readLOC( rEntry );

    return createUnbufferedStream ( aMutexHolder, rEntry, rData, UNBUFF_STREAM_RAW, bIsEncrypted );
}

uno::Reference< XInputStream > SAL_CALL ZipFile::getWrappedRawStream(
        ZipEntry& rEntry,
        const ::rtl::Reference< EncryptionData >& rData,
        const ::rtl::OUString& aMediaType,
        SotMutexHolderRef aMutexHolder )
    throw ( packages::NoEncryptionException,
            IOException,
            ZipException,
            RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !rData.is() )
        throw packages::NoEncryptionException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );

    if ( rEntry.nOffset <= 0 )
        readLOC( rEntry );

    return createUnbufferedStream ( aMutexHolder, rEntry, rData, UNBUFF_STREAM_WRAPPEDRAW, sal_True, aMediaType );
}

sal_Bool ZipFile::readLOC( ZipEntry &rEntry )
    throw(IOException, ZipException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nTestSig, nTime, nCRC, nSize, nCompressedSize;
    sal_Int16 nVersion, nFlag, nHow, nPathLen, nExtraLen;
    sal_Int64 nPos = -rEntry.nOffset;

    aGrabber.seek(nPos);
    aGrabber >> nTestSig;

    if (nTestSig != LOCSIG)
        throw ZipIOException("Invalid LOC header (bad signature)", uno::Reference < XInterface > () );
    aGrabber >> nVersion;
    aGrabber >> nFlag;
    aGrabber >> nHow;
    aGrabber >> nTime;
    aGrabber >> nCRC;
    aGrabber >> nCompressedSize;
    aGrabber >> nSize;
    aGrabber >> nPathLen;
    aGrabber >> nExtraLen;
    rEntry.nOffset = aGrabber.getPosition() + nPathLen + nExtraLen;

    // FIXME64: need to read 64bit LOC

    sal_Bool bBroken = sal_False;

    try
    {
        // read always in UTF8, some tools seem not to set UTF8 bit
        uno::Sequence < sal_Int8 > aNameBuffer( nPathLen );
        sal_Int32 nRead = aGrabber.readBytes( aNameBuffer, nPathLen );
        if ( nRead < aNameBuffer.getLength() )
                aNameBuffer.realloc( nRead );

        ::rtl::OUString sLOCPath = rtl::OUString::intern( (sal_Char *) aNameBuffer.getArray(),
                                                          aNameBuffer.getLength(),
                                                          RTL_TEXTENCODING_UTF8 );

        if ( rEntry.nPathLen == -1 ) // the file was created
        {
            rEntry.nPathLen = nPathLen;
            rEntry.sPath = sLOCPath;
        }

        // check basic local file header / entry consistency, just
        // plain ignore bits 1 & 2 of the flag field - they are either
        // purely informative, or even fully undefined (depending on
        // nMethod)
        // Do *not* compare nMethod / nHow, older versions with
        // encrypted streams write mismatching DEFLATE/STORE pairs
        // there.
        // Do *not* compare timestamps, since MSO 2010 can produce documents
        // with timestamp difference in the central directory entry and local
        // file header.
        bBroken = rEntry.nVersion != nVersion
                        || (rEntry.nFlag & ~6L) != (nFlag & ~6L)
                        || rEntry.nPathLen != nPathLen
                        || !rEntry.sPath.equals( sLOCPath );
    }
    catch(...)
    {
        bBroken = sal_True;
    }

    if ( bBroken && !bRecoveryMode )
        throw ZipIOException("The stream seems to be broken!",
                            uno::Reference< XInterface >() );

    return sal_True;
}

sal_Int32 ZipFile::findEND( )
    throw(IOException, ZipException, RuntimeException)
{
    // this method is called in constructor only, no need for mutex
    sal_Int32 nLength, nPos, nEnd;
    Sequence < sal_Int8 > aBuffer;
    try
    {
        nLength = static_cast <sal_Int32 > (aGrabber.getLength());
        if (nLength == 0 || nLength < ENDHDR)
            return -1;
        nPos = nLength - ENDHDR - ZIP_MAXNAMELEN;
        nEnd = nPos >= 0 ? nPos : 0 ;

        aGrabber.seek( nEnd );
        aGrabber.readBytes ( aBuffer, nLength - nEnd );

        const sal_Int8 *pBuffer = aBuffer.getConstArray();

        nPos = nLength - nEnd - ENDHDR;
        while ( nPos >= 0 )
        {
            if (pBuffer[nPos] == 'P' && pBuffer[nPos+1] == 'K' && pBuffer[nPos+2] == 5 && pBuffer[nPos+3] == 6 )
                return nPos + nEnd;
            nPos--;
        }
    }
    catch ( IllegalArgumentException& )
    {
        throw ZipException("Zip END signature not found!", uno::Reference < XInterface > () );
    }
    catch ( NotConnectedException& )
    {
        throw ZipException("Zip END signature not found!", uno::Reference < XInterface > () );
    }
    catch ( BufferSizeExceededException& )
    {
        throw ZipException("Zip END signature not found!", uno::Reference < XInterface > () );
    }
    throw ZipException("Zip END signature not found!", uno::Reference < XInterface > () );
}

sal_Int32 ZipFile::readCEN()
    throw(IOException, ZipException, RuntimeException)
{
    // this method is called in constructor only, no need for mutex
    sal_Int32 nCenLen, nCenPos = -1, nCenOff, nEndPos, nLocPos;
    sal_uInt16 nCount, nTotal;

    try
    {
        nEndPos = findEND();
        if (nEndPos == -1)
            return -1;
        aGrabber.seek(nEndPos + ENDTOT);
        aGrabber >> nTotal;
        aGrabber >> nCenLen;
        aGrabber >> nCenOff;

        if ( nTotal * CENHDR > nCenLen )
            throw ZipException("invalid END header (bad entry count)", uno::Reference < XInterface > () );

        if ( nTotal > ZIP_MAXENTRIES )
            throw ZipException("too many entries in ZIP File", uno::Reference < XInterface > () );

        if ( nCenLen < 0 || nCenLen > nEndPos )
            throw ZipException("Invalid END header (bad central directory size)", uno::Reference < XInterface > () );

        nCenPos = nEndPos - nCenLen;

        if ( nCenOff < 0 || nCenOff > nCenPos )
            throw ZipException("Invalid END header (bad central directory size)", uno::Reference < XInterface > () );

        nLocPos = nCenPos - nCenOff;
        aGrabber.seek( nCenPos );
        Sequence < sal_Int8 > aCENBuffer ( nCenLen );
        sal_Int64 nRead = aGrabber.readBytes ( aCENBuffer, nCenLen );
        if ( static_cast < sal_Int64 > ( nCenLen ) != nRead )
            throw ZipException ("Error reading CEN into memory buffer!", uno::Reference < XInterface > () );

        MemoryByteGrabber aMemGrabber ( aCENBuffer );

        ZipEntry aEntry;
        sal_Int32 nTestSig;
        sal_Int16 nCommentLen;

        for (nCount = 0 ; nCount < nTotal; nCount++)
        {
            aMemGrabber >> nTestSig;
            if ( nTestSig != CENSIG )
                throw ZipException("Invalid CEN header (bad signature)", uno::Reference < XInterface > () );

            aMemGrabber.skipBytes ( 2 );
            aMemGrabber >> aEntry.nVersion;

            if ( ( aEntry.nVersion & 1 ) == 1 )
                throw ZipException("Invalid CEN header (encrypted entry)", uno::Reference < XInterface > () );

            aMemGrabber >> aEntry.nFlag;
            aMemGrabber >> aEntry.nMethod;

            if ( aEntry.nMethod != STORED && aEntry.nMethod != DEFLATED)
                throw ZipException("Invalid CEN header (bad compression method)", uno::Reference < XInterface > () );

            aMemGrabber >> aEntry.nTime;
            aMemGrabber >> aEntry.nCrc;

            sal_uInt32 nCompressedSize, nSize, nOffset;

            aMemGrabber >> nCompressedSize;
            aMemGrabber >> nSize;
            aMemGrabber >> aEntry.nPathLen;
            aMemGrabber >> aEntry.nExtraLen;
            aMemGrabber >> nCommentLen;
            aMemGrabber.skipBytes ( 8 );
            aMemGrabber >> nOffset;

            // FIXME64: need to read the 64bit header instead
            if ( nSize == 0xffffffff ||
                 nOffset == 0xffffffff ||
                 nCompressedSize == 0xffffffff ) {
                throw ZipException("PK64 zip file entry", uno::Reference < XInterface > () );
            } else {
                aEntry.nCompressedSize = nCompressedSize;
                aEntry.nSize = nSize;
                aEntry.nOffset = nOffset;
            }

            aEntry.nOffset += nLocPos;
            aEntry.nOffset *= -1;

            if ( aEntry.nPathLen < 0 )
                throw ZipException("unexpected name length", uno::Reference < XInterface > () );

            if ( nCommentLen < 0 )
                throw ZipException("unexpected comment length", uno::Reference < XInterface > () );

            if ( aEntry.nExtraLen < 0 )
                throw ZipException("unexpected extra header info length", uno::Reference < XInterface > () );

            // read always in UTF8, some tools seem not to set UTF8 bit
            aEntry.sPath = rtl::OUString::intern ( (sal_Char *) aMemGrabber.getCurrentPos(),
                                                   aEntry.nPathLen,
                                                   RTL_TEXTENCODING_UTF8 );

            if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( aEntry.sPath, sal_True ) )
                throw ZipException("Zip entry has an invalid name.", uno::Reference < XInterface > () );

            aMemGrabber.skipBytes( aEntry.nPathLen + aEntry.nExtraLen + nCommentLen );
            aEntries[aEntry.sPath] = aEntry;
        }

        if (nCount != nTotal)
            throw ZipException("Count != Total", uno::Reference < XInterface > () );
    }
    catch ( IllegalArgumentException & )
    {
        // seek can throw this...
        nCenPos = -1; // make sure we return -1 to indicate an error
    }
    return nCenPos;
}

sal_Int32 ZipFile::recover()
    throw(IOException, ZipException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int64 nLength;
    Sequence < sal_Int8 > aBuffer;
    Sequence < sal_Int32 > aHeaderOffsets;

    try
    {
        nLength = aGrabber.getLength();
        if (nLength == 0 || nLength < ENDHDR)
            return -1;

        aGrabber.seek( 0 );

        const sal_Int64 nToRead = 32000;
        for( sal_Int64 nGenPos = 0; aGrabber.readBytes( aBuffer, nToRead ) && aBuffer.getLength() > 16; )
        {
            const sal_Int8 *pBuffer = aBuffer.getConstArray();
            sal_Int32 nBufSize = aBuffer.getLength();

            sal_Int64 nPos = 0;
            // the buffer should contain at least one header,
            // or if it is end of the file, at least the postheader with sizes and hash
            while( nPos < nBufSize - 30
                || ( nBufSize < nToRead && nPos < nBufSize - 16 ) )

            {
                if ( nPos < nBufSize - 30 && pBuffer[nPos] == 'P' && pBuffer[nPos+1] == 'K' && pBuffer[nPos+2] == 3 && pBuffer[nPos+3] == 4 )
                {
                    ZipEntry aEntry;
                    MemoryByteGrabber aMemGrabber ( Sequence< sal_Int8 >( ((sal_Int8*)(&(pBuffer[nPos+4]))), 26 ) );

                    aMemGrabber >> aEntry.nVersion;
                    if ( ( aEntry.nVersion & 1 ) != 1 )
                    {
                        aMemGrabber >> aEntry.nFlag;
                        aMemGrabber >> aEntry.nMethod;

                        if ( aEntry.nMethod == STORED || aEntry.nMethod == DEFLATED )
                        {
                            sal_uInt32 nCompressedSize, nSize;

                            aMemGrabber >> aEntry.nTime;
                            aMemGrabber >> aEntry.nCrc;
                            aMemGrabber >> nCompressedSize;
                            aMemGrabber >> nSize;
                            aMemGrabber >> aEntry.nPathLen;
                            aMemGrabber >> aEntry.nExtraLen;

                            // FIXME64: need to read the 64bit header instead
                            if ( nSize == 0xffffffff ||
                                 nCompressedSize == 0xffffffff ) {
                                throw ZipException("PK64 zip file entry", uno::Reference < XInterface > () );
                            } else {
                                aEntry.nCompressedSize = nCompressedSize;
                                aEntry.nSize = nSize;
                            }

                            sal_Int32 nDescrLength =
                                ( aEntry.nMethod == DEFLATED && ( aEntry.nFlag & 8 ) ) ? 16 : 0;

                            sal_Int64 nDataSize = ( aEntry.nMethod == DEFLATED ) ? aEntry.nCompressedSize : aEntry.nSize;
                            sal_Int64 nBlockLength = nDataSize + aEntry.nPathLen + aEntry.nExtraLen + 30 + nDescrLength;
                            if ( aEntry.nPathLen >= 0 && aEntry.nExtraLen >= 0
                                && ( nGenPos + nPos + nBlockLength ) <= nLength )
                            {
                                // read always in UTF8, some tools seem not to set UTF8 bit
                                if( nPos + 30 + aEntry.nPathLen <= nBufSize )
                                    aEntry.sPath = OUString ( (sal_Char *) &pBuffer[nPos + 30],
                                                              aEntry.nPathLen,
                                                              RTL_TEXTENCODING_UTF8 );
                                else
                                {
                                    Sequence < sal_Int8 > aFileName;
                                    aGrabber.seek( nGenPos + nPos + 30 );
                                    aGrabber.readBytes( aFileName, aEntry.nPathLen );
                                    aEntry.sPath = OUString ( (sal_Char *) aFileName.getArray(),
                                                              aFileName.getLength(),
                                                              RTL_TEXTENCODING_UTF8 );
                                    aEntry.nPathLen = static_cast< sal_Int16 >(aFileName.getLength());
                                }

                                aEntry.nOffset = nGenPos + nPos + 30 + aEntry.nPathLen + aEntry.nExtraLen;

                                if ( ( aEntry.nSize || aEntry.nCompressedSize ) && !checkSizeAndCRC( aEntry ) )
                                {
                                    aEntry.nCrc = 0;
                                    aEntry.nCompressedSize = 0;
                                    aEntry.nSize = 0;
                                }

                                if ( aEntries.find( aEntry.sPath ) == aEntries.end() )
                                    aEntries[aEntry.sPath] = aEntry;
                            }
                        }
                    }

                    nPos += 4;
                }
                else if (pBuffer[nPos] == 'P' && pBuffer[nPos+1] == 'K' && pBuffer[nPos+2] == 7 && pBuffer[nPos+3] == 8 )
                {
                    sal_Int32 nCRC32;
                    sal_uInt32 nCompressedSize32, nSize32;
                    sal_Int64 nCompressedSize, nSize;
                    MemoryByteGrabber aMemGrabber ( Sequence< sal_Int8 >( ((sal_Int8*)(&(pBuffer[nPos+4]))), 12 ) );
                    aMemGrabber >> nCRC32;
                    aMemGrabber >> nCompressedSize32;
                    aMemGrabber >> nSize32;

                    // FIXME64: work to be done here ...
                    nCompressedSize = nCompressedSize32;
                    nSize = nSize32;

                    for( EntryHash::iterator aIter = aEntries.begin(); aIter != aEntries.end(); ++aIter )
                    {
                        ZipEntry aTmp = (*aIter).second;

                        // this is a broken package, accept this block not only for DEFLATED streams
                        if( (*aIter).second.nFlag & 8 )
                        {
                            sal_Int64 nStreamOffset = nGenPos + nPos - nCompressedSize;
                            if ( nStreamOffset == (*aIter).second.nOffset && nCompressedSize > (*aIter).second.nCompressedSize )
                            {
                                // only DEFLATED blocks need to be checked
                                sal_Bool bAcceptBlock = ( (*aIter).second.nMethod == STORED && nCompressedSize == nSize );

                                if ( !bAcceptBlock )
                                {
                                    sal_Int64 nRealSize = 0;
                                    sal_Int32 nRealCRC = 0;
                                    getSizeAndCRC( nStreamOffset, nCompressedSize, &nRealSize, &nRealCRC );
                                    bAcceptBlock = ( nRealSize == nSize && nRealCRC == nCRC32 );
                                }

                                if ( bAcceptBlock )
                                {
                                    (*aIter).second.nCrc = nCRC32;
                                    (*aIter).second.nCompressedSize = nCompressedSize;
                                    (*aIter).second.nSize = nSize;
                                }
                            }
#if 0
// for now ignore clearly broken streams
                            else if( !(*aIter).second.nCompressedSize )
                            {
                                (*aIter).second.nCrc = nCRC32;
                                sal_Int32 nRealStreamSize = nGenPos + nPos - (*aIter).second.nOffset;
                                (*aIter).second.nCompressedSize = nGenPos + nPos - (*aIter).second.nOffset;
                                (*aIter).second.nSize = nSize;
                            }
#endif
                        }
                    }

                    nPos += 4;
                }
                else
                    nPos++;
            }

            nGenPos += nPos;
            aGrabber.seek( nGenPos );
        }

        return 0;
    }
    catch ( IllegalArgumentException& )
    {
        throw ZipException("Zip END signature not found!", uno::Reference < XInterface > () );
    }
    catch ( NotConnectedException& )
    {
        throw ZipException("Zip END signature not found!", uno::Reference < XInterface > () );
    }
    catch ( BufferSizeExceededException& )
    {
        throw ZipException("Zip END signature not found!", uno::Reference < XInterface > () );
    }
}

sal_Bool ZipFile::checkSizeAndCRC( const ZipEntry& aEntry )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nCRC = 0;
    sal_Int64 nSize = 0;

    if( aEntry.nMethod == STORED )
        return ( getCRC( aEntry.nOffset, aEntry.nSize ) == aEntry.nCrc );

    getSizeAndCRC( aEntry.nOffset, aEntry.nCompressedSize, &nSize, &nCRC );
    return ( aEntry.nSize == nSize && aEntry.nCrc == nCRC );
}

sal_Int32 ZipFile::getCRC( sal_Int64 nOffset, sal_Int64 nSize )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Sequence < sal_Int8 > aBuffer;
    CRC32 aCRC;
    sal_Int32 nBlockSize = static_cast< sal_Int32 > (::std::min( nSize, static_cast< sal_Int64 >( 32000 ) ) );

    aGrabber.seek( nOffset );
    for ( int ind = 0;
          aGrabber.readBytes( aBuffer, nBlockSize ) && ind * nBlockSize < nSize;
          ind++ )
    {
        sal_Int64 nLen = ::std::min( static_cast< sal_Int64 >( nBlockSize ),
                                     nSize - ind * nBlockSize );
        aCRC.updateSegment( aBuffer, 0, static_cast< sal_Int32 >( nLen ) );
    }

    return aCRC.getValue();
}

void ZipFile::getSizeAndCRC( sal_Int64 nOffset, sal_Int64 nCompressedSize, sal_Int64 *nSize, sal_Int32 *nCRC )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    Sequence < sal_Int8 > aBuffer;
    CRC32 aCRC;
    sal_Int64 nRealSize = 0;
    Inflater aInflaterLocal( sal_True );
    sal_Int32 nBlockSize = static_cast< sal_Int32 > (::std::min( nCompressedSize, static_cast< sal_Int64 >( 32000 ) ) );

    aGrabber.seek( nOffset );
    for ( int ind = 0;
          !aInflaterLocal.finished() && aGrabber.readBytes( aBuffer, nBlockSize ) && ind * nBlockSize < nCompressedSize;
          ind++ )
    {
        Sequence < sal_Int8 > aData( nBlockSize );
        sal_Int32 nLastInflated = 0;
        sal_Int64 nInBlock = 0;

        aInflaterLocal.setInput( aBuffer );
        do
        {
            nLastInflated = aInflaterLocal.doInflateSegment( aData, 0, nBlockSize );
            aCRC.updateSegment( aData, 0, nLastInflated );
            nInBlock += nLastInflated;
        } while( !aInflater.finished() && nLastInflated );

        nRealSize += nInBlock;
    }

    *nSize = nRealSize;
    *nCRC = aCRC.getValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
