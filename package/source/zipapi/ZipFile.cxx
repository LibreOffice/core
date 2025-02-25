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

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/packages/NoEncryptionException.hpp>
#include <com/sun/star/packages/WrongPasswordException.hpp>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/packages/zip/ZipException.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/xml/crypto/XCipherContext.hpp>
#include <com/sun/star/xml/crypto/XDigestContext.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/NSSInitializer.hpp>

#include <comphelper/bytereader.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/threadpool.hxx>
#include <rtl/digest.h>
#include <rtl/crc.h>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include <argon2.h>

#include "blowfishcontext.hxx"
#include "sha1context.hxx"
#include <ZipFile.hxx>
#include <ZipEnumeration.hxx>
#include "XUnbufferedStream.hxx"
#include "XBufferedThreadedStream.hxx"
#include <PackageConstants.hxx>
#include <EncryptedDataHeader.hxx>
#include <EncryptionData.hxx>
#include "MemoryByteGrabber.hxx"

#include <CRC32.hxx>

using namespace com::sun::star;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::packages;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::zip::ZipConstants;

using ZipUtils::Inflater;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

/** This class is used to read entries from a zip file
 */
ZipFile::ZipFile( rtl::Reference< comphelper::RefCountedMutex > aMutexHolder,
                  uno::Reference < XInputStream > const &xInput,
                  uno::Reference < XComponentContext > xContext,
                  bool bInitialise, bool bForceRecovery,
                  Checks const checks)
: m_aMutexHolder(std::move( aMutexHolder ))
, m_Checks(checks)
, aGrabber( xInput )
, aInflater( true )
, xStream(xInput)
, m_xContext (std::move( xContext ))
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
            m_EntriesInsensitive.clear();
            throw ZipException(u"stream data looks to be broken"_ustr );
        }
    }
}

ZipFile::~ZipFile()
{
    aEntries.clear();
}

void ZipFile::setInputStream ( const uno::Reference < XInputStream >& xNewStream )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    xStream = xNewStream;
    aGrabber.setInputStream ( xStream );
}

uno::Reference< xml::crypto::XDigestContext > ZipFile::StaticGetDigestContextForChecksum( const uno::Reference< uno::XComponentContext >& xArgContext, const ::rtl::Reference< EncryptionData >& xEncryptionData )
{
    assert(xEncryptionData->m_oCheckAlg); // callers checked it already

    uno::Reference< xml::crypto::XDigestContext > xDigestContext;
    if (*xEncryptionData->m_oCheckAlg == xml::crypto::DigestID::SHA256_1K)
    {
        uno::Reference< uno::XComponentContext > xContext = xArgContext;
        if ( !xContext.is() )
            xContext = comphelper::getProcessComponentContext();

        uno::Reference< xml::crypto::XNSSInitializer > xDigestContextSupplier = xml::crypto::NSSInitializer::create( xContext );

        xDigestContext.set(xDigestContextSupplier->getDigestContext(
                *xEncryptionData->m_oCheckAlg, uno::Sequence<beans::NamedValue>()),
            uno::UNO_SET_THROW);
    }
    else if (*xEncryptionData->m_oCheckAlg == xml::crypto::DigestID::SHA1_1K)
    {
        if (xEncryptionData->m_bTryWrongSHA1)
        {
            xDigestContext.set(StarOfficeSHA1DigestContext::Create(), uno::UNO_SET_THROW);
        }
        else
        {
            xDigestContext.set(CorrectSHA1DigestContext::Create(), uno::UNO_SET_THROW);
        }
    }

    return xDigestContext;
}

uno::Reference< xml::crypto::XCipherContext > ZipFile::StaticGetCipher( const uno::Reference< uno::XComponentContext >& xArgContext, const ::rtl::Reference< EncryptionData >& xEncryptionData, bool bEncrypt )
{
    uno::Reference< xml::crypto::XCipherContext > xResult;

    if (xEncryptionData->m_nDerivedKeySize < 0)
    {
        throw ZipIOException(u"Invalid derived key length!"_ustr );
    }

    uno::Sequence< sal_Int8 > aDerivedKey( xEncryptionData->m_nDerivedKeySize );
    if (!xEncryptionData->m_oPBKDFIterationCount && !xEncryptionData->m_oArgon2Args
        && xEncryptionData->m_nDerivedKeySize == xEncryptionData->m_aKey.getLength())
    {
        // gpg4libre: no need to derive key, m_aKey is already
        // usable as symmetric session key
        aDerivedKey = xEncryptionData->m_aKey;
    }
    else if (xEncryptionData->m_oArgon2Args)
    {
        // apparently multiple lanes cannot be processed in parallel (the
        // implementation will clamp), but it doesn't make sense to have more
        // threads than CPUs
        uint32_t const threads(::comphelper::ThreadPool::getPreferredConcurrency());
        // need to use context to set a fixed version
        argon2_context context = {
            .out = reinterpret_cast<uint8_t *>(aDerivedKey.getArray()),
            .outlen = ::sal::static_int_cast<uint32_t>(aDerivedKey.getLength()),
            .pwd = reinterpret_cast<uint8_t *>(xEncryptionData->m_aKey.getArray()),
            .pwdlen = ::sal::static_int_cast<uint32_t>(xEncryptionData->m_aKey.getLength()),
            .salt = reinterpret_cast<uint8_t *>(xEncryptionData->m_aSalt.getArray()),
            .saltlen = ::sal::static_int_cast<uint32_t>(xEncryptionData->m_aSalt.getLength()),
            .secret = nullptr, .secretlen = 0,
            .ad = nullptr, .adlen = 0,
            .t_cost = ::sal::static_int_cast<uint32_t>(::std::get<0>(*xEncryptionData->m_oArgon2Args)),
            .m_cost = ::sal::static_int_cast<uint32_t>(::std::get<1>(*xEncryptionData->m_oArgon2Args)),
            .lanes = ::sal::static_int_cast<uint32_t>(::std::get<2>(*xEncryptionData->m_oArgon2Args)),
            .threads = threads,
            .version = ARGON2_VERSION_13,
            .allocate_cbk = nullptr, .free_cbk = nullptr,
            .flags = ARGON2_DEFAULT_FLAGS
        };
        // libargon2 validates all the arguments so don't need to do it here
        int const rc = argon2id_ctx(&context);
        if (rc != ARGON2_OK)
        {
            SAL_WARN("package", "argon2id_ctx failed to derive key: " << argon2_error_message(rc));
            throw ZipIOException(u"argon2id_ctx failed to derive key"_ustr);
        }
    }
    else if ( rtl_Digest_E_None != rtl_digest_PBKDF2( reinterpret_cast< sal_uInt8* >( aDerivedKey.getArray() ),
                        aDerivedKey.getLength(),
                        reinterpret_cast< const sal_uInt8 * > (xEncryptionData->m_aKey.getConstArray() ),
                        xEncryptionData->m_aKey.getLength(),
                        reinterpret_cast< const sal_uInt8 * > ( xEncryptionData->m_aSalt.getConstArray() ),
                        xEncryptionData->m_aSalt.getLength(),
                        *xEncryptionData->m_oPBKDFIterationCount) )
    {
        throw ZipIOException(u"Can not create derived key!"_ustr );
    }

    if (xEncryptionData->m_nEncAlg == xml::crypto::CipherID::AES_CBC_W3C_PADDING
        || xEncryptionData->m_nEncAlg == xml::crypto::CipherID::AES_GCM_W3C)
    {
        uno::Reference< uno::XComponentContext > xContext = xArgContext;
        if ( !xContext.is() )
            xContext = comphelper::getProcessComponentContext();

        uno::Reference< xml::crypto::XNSSInitializer > xCipherContextSupplier = xml::crypto::NSSInitializer::create( xContext );

        xResult = xCipherContextSupplier->getCipherContext( xEncryptionData->m_nEncAlg, aDerivedKey, xEncryptionData->m_aInitVector, bEncrypt, uno::Sequence< beans::NamedValue >() );
    }
    else if ( xEncryptionData->m_nEncAlg == xml::crypto::CipherID::BLOWFISH_CFB_8 )
    {
        xResult = BlowfishCFB8CipherContext::Create( aDerivedKey, xEncryptionData->m_aInitVector, bEncrypt );
    }
    else
    {
        throw ZipIOException(u"Unknown cipher algorithm is requested!"_ustr );
    }

    return xResult;
}

void ZipFile::StaticFillHeader( const ::rtl::Reference< EncryptionData >& rData,
                                sal_Int64 nSize,
                                const OUString& aMediaType,
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
    sal_Int32 const nIterationCount = rData->m_oPBKDFIterationCount ? *rData->m_oPBKDFIterationCount : 0;
    *(pHeader++) = static_cast< sal_Int8 >(( nIterationCount >> 0 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nIterationCount >> 8 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nIterationCount >> 16 ) & 0xFF);
    *(pHeader++) = static_cast< sal_Int8 >(( nIterationCount >> 24 ) & 0xFF);

    sal_Int32 const nArgon2t = rData->m_oArgon2Args ? ::std::get<0>(*rData->m_oArgon2Args) : 0;
    *(pHeader++) = static_cast<sal_Int8>((nArgon2t >> 0) & 0xFF);
    *(pHeader++) = static_cast<sal_Int8>((nArgon2t >> 8) & 0xFF);
    *(pHeader++) = static_cast<sal_Int8>((nArgon2t >> 16) & 0xFF);
    *(pHeader++) = static_cast<sal_Int8>((nArgon2t >> 24) & 0xFF);

    sal_Int32 const nArgon2m = rData->m_oArgon2Args ? ::std::get<1>(*rData->m_oArgon2Args) : 0;
    *(pHeader++) = static_cast<sal_Int8>((nArgon2m >> 0) & 0xFF);
    *(pHeader++) = static_cast<sal_Int8>((nArgon2m >> 8) & 0xFF);
    *(pHeader++) = static_cast<sal_Int8>((nArgon2m >> 16) & 0xFF);
    *(pHeader++) = static_cast<sal_Int8>((nArgon2m >> 24) & 0xFF);

    sal_Int32 const nArgon2p = rData->m_oArgon2Args ? ::std::get<2>(*rData->m_oArgon2Args) : 0;
    *(pHeader++) = static_cast<sal_Int8>((nArgon2p >> 0) & 0xFF);
    *(pHeader++) = static_cast<sal_Int8>((nArgon2p >> 8) & 0xFF);
    *(pHeader++) = static_cast<sal_Int8>((nArgon2p >> 16) & 0xFF);
    *(pHeader++) = static_cast<sal_Int8>((nArgon2p >> 24) & 0xFF);

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
    sal_Int32 nChecksumAlgID = rData->m_oCheckAlg ? *rData->m_oCheckAlg : 0;
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

bool ZipFile::StaticFillData (  ::rtl::Reference< BaseEncryptionData > const & rData,
                                    sal_Int32 &rEncAlg,
                                    sal_Int32 &rChecksumAlg,
                                    sal_Int32 &rDerivedKeySize,
                                    sal_Int32 &rStartKeyGenID,
                                    sal_Int32 &rSize,
                                    OUString& aMediaType,
                                    const uno::Reference< XInputStream >& rStream )
{
    bool bOk = false;
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
            if (nCount != 0)
            {
                rData->m_oPBKDFIterationCount.emplace(nCount);
            }
            else
            {
                rData->m_oPBKDFIterationCount.reset();
            }

            sal_Int32 nArgon2t = pBuffer[nPos++] & 0xFF;
            nArgon2t |= ( pBuffer[nPos++] & 0xFF ) << 8;
            nArgon2t |= ( pBuffer[nPos++] & 0xFF ) << 16;
            nArgon2t |= ( pBuffer[nPos++] & 0xFF ) << 24;

            sal_Int32 nArgon2m = pBuffer[nPos++] & 0xFF;
            nArgon2m |= ( pBuffer[nPos++] & 0xFF ) << 8;
            nArgon2m |= ( pBuffer[nPos++] & 0xFF ) << 16;
            nArgon2m |= ( pBuffer[nPos++] & 0xFF ) << 24;

            sal_Int32 nArgon2p = pBuffer[nPos++] & 0xFF;
            nArgon2p |= ( pBuffer[nPos++] & 0xFF ) << 8;
            nArgon2p |= ( pBuffer[nPos++] & 0xFF ) << 16;
            nArgon2p |= ( pBuffer[nPos++] & 0xFF ) << 24;

            if (nArgon2t != 0 && nArgon2m != 0 && nArgon2p != 0)
            {
                rData->m_oArgon2Args.emplace(nArgon2t, nArgon2m, nArgon2p);
            }
            else
            {
                rData->m_oArgon2Args.reset();
            }

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
                            aMediaType = OUString( reinterpret_cast<sal_Unicode const *>(aBuffer.getConstArray()),
                                                            nMediaTypeLength / sizeof( sal_Unicode ) );
                            bOk = true;
                        }
                    }
                }
            }
        }
    }
    return bOk;
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

bool ZipFile::StaticHasValidPassword( const uno::Reference< uno::XComponentContext >& rxContext, const Sequence< sal_Int8 > &aReadBuffer, const ::rtl::Reference< EncryptionData > &rData )
{
    assert(rData->m_nEncAlg != xml::crypto::CipherID::AES_GCM_W3C); // should not be called for AEAD

    if ( !rData.is() || !rData->m_aKey.hasElements() )
        return false;

    bool bRet = false;

    uno::Reference< xml::crypto::XCipherContext > xCipher( StaticGetCipher( rxContext, rData, false ), uno::UNO_SET_THROW );

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

    if ( aDecryptBuffer2.hasElements() )
    {
        sal_Int32 nOldLen = aDecryptBuffer.getLength();
        aDecryptBuffer.realloc( nOldLen + aDecryptBuffer2.getLength() );
        memcpy( aDecryptBuffer.getArray() + nOldLen, aDecryptBuffer2.getConstArray(), aDecryptBuffer2.getLength() );
    }

    if ( aDecryptBuffer.getLength() > n_ConstDigestLength )
        aDecryptBuffer.realloc( n_ConstDigestLength );

    uno::Sequence< sal_Int8 > aDigestSeq;
    uno::Reference< xml::crypto::XDigestContext > xDigestContext( StaticGetDigestContextForChecksum( rxContext, rData ), uno::UNO_SET_THROW );

    xDigestContext->updateDigest( aDecryptBuffer );
    aDigestSeq = xDigestContext->finalizeDigestAndDispose();

    // If we don't have a digest, then we have to assume that the password is correct
    if (  rData->m_aDigest.hasElements() &&
          ( aDigestSeq.getLength() != rData->m_aDigest.getLength() ||
            0 != memcmp ( aDigestSeq.getConstArray(),
                                     rData->m_aDigest.getConstArray(),
                                    aDigestSeq.getLength() ) ) )
    {
        // We should probably tell the user that the password they entered was wrong
    }
    else
        bRet = true;

    return bRet;
}

uno::Reference<io::XInputStream> ZipFile::checkValidPassword(
    ZipEntry const& rEntry, ::rtl::Reference<EncryptionData> const& rData,
    sal_Int64 const nDecryptedSize,
    rtl::Reference<comphelper::RefCountedMutex> const& rMutex)
{
    if (rData.is() && rData->m_nEncAlg == xml::crypto::CipherID::AES_GCM_W3C)
    {
        try // the only way to find out: decrypt the whole stream, which will
        {   // check the tag
            uno::Reference<io::XInputStream> const xRet =
                createStreamForZipEntry(rMutex, rEntry, rData, UNBUFF_STREAM_DATA, nDecryptedSize);
            // currently XBufferedStream reads the whole stream in its ctor (to
            // verify the tag) - in case this gets changed, explicitly seek here
            uno::Reference<io::XSeekable> const xSeek(xRet, uno::UNO_QUERY_THROW);
            xSeek->seek(xSeek->getLength());
            xSeek->seek(0);
            return xRet;
        }
        catch (uno::Exception const&)
        {
            return {};
        }
    }
    else if (rData.is() && rData->m_aKey.hasElements())
    {
        ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

        css::uno::Reference < css::io::XSeekable > xSeek(xStream, UNO_QUERY_THROW);
        xSeek->seek( rEntry.nOffset );
        sal_Int64 nSize = rEntry.nMethod == DEFLATED ? rEntry.nCompressedSize : rEntry.nSize;

        // Only want to read enough to verify the digest
        if ( nSize > n_ConstDigestDecrypt )
            nSize = n_ConstDigestDecrypt;

        assert(nSize <= n_ConstDigestDecrypt && nSize >= 0 && "silence bogus coverity overflow_sink");
        Sequence<sal_Int8> aReadBuffer(nSize);

        xStream->readBytes( aReadBuffer, nSize );

        if (StaticHasValidPassword(m_xContext, aReadBuffer, rData))
        {
            return createStreamForZipEntry(
                    rMutex, rEntry, rData, UNBUFF_STREAM_DATA, nDecryptedSize);
        }
    }

    return {};
}

namespace {

class XBufferedStream : public cppu::WeakImplHelper<css::io::XInputStream, css::io::XSeekable>,
                        public comphelper::ByteReader
{
    std::vector<sal_Int8> maBytes;
    size_t mnPos;

    size_t remainingSize() const
    {
        return maBytes.size() - mnPos;
    }

    bool hasBytes() const
    {
        return mnPos < maBytes.size();
    }

public:
    XBufferedStream( const uno::Reference<XInputStream>& xSrcStream ) : mnPos(0)
    {
        sal_Int32 nRemaining = xSrcStream->available();
        maBytes.reserve(nRemaining);

        if (auto pByteReader = dynamic_cast< comphelper::ByteReader* >( xSrcStream.get() ))
        {
            maBytes.resize(nRemaining);

            sal_Int8* pData = maBytes.data();
            while (nRemaining > 0)
            {
                sal_Int32 nRead = pByteReader->readSomeBytes(pData, nRemaining);
                nRemaining -= nRead;
                pData += nRead;
            }
            return;
        }

        const sal_Int32 nBufSize = 8192;
        uno::Sequence<sal_Int8> aBuf(nBufSize);
        while (nRemaining > 0)
        {
            const sal_Int32 nBytes = xSrcStream->readBytes(aBuf, std::min(nBufSize, nRemaining));
            if (!nBytes)
                break;
            maBytes.insert(maBytes.end(), aBuf.begin(), aBuf.begin() + nBytes);
            nRemaining -= nBytes;
        }
    }

    virtual sal_Int32 SAL_CALL readBytes( uno::Sequence<sal_Int8>& rData, sal_Int32 nBytesToRead ) override
    {
        if (!hasBytes())
            return 0;

        sal_Int32 nReadSize = std::min<sal_Int32>(nBytesToRead, remainingSize());
        rData.realloc(nReadSize);
        auto pData = rData.getArray();
        std::vector<sal_Int8>::const_iterator it = maBytes.cbegin();
        std::advance(it, mnPos);
        for (sal_Int32 i = 0; i < nReadSize; ++i, ++it)
            pData[i] = *it;

        mnPos += nReadSize;

        return nReadSize;
    }

    virtual sal_Int32 readSomeBytes(sal_Int8* pData, sal_Int32 nBytesToRead) override
    {
        if (!hasBytes())
            return 0;

        sal_Int32 nReadSize = std::min<sal_Int32>(nBytesToRead, remainingSize());
        std::vector<sal_Int8>::const_iterator it = maBytes.cbegin();
        std::advance(it, mnPos);
        for (sal_Int32 i = 0; i < nReadSize; ++i, ++it)
            pData[i] = *it;

        mnPos += nReadSize;

        return nReadSize;
    }

    virtual sal_Int32 SAL_CALL readSomeBytes( ::css::uno::Sequence<sal_Int8>& rData, sal_Int32 nMaxBytesToRead ) override
    {
        return readBytes(rData, nMaxBytesToRead);
    }

    virtual void SAL_CALL skipBytes( sal_Int32 nBytesToSkip ) override
    {
        if (!hasBytes())
            return;

        mnPos += nBytesToSkip;
    }

    virtual sal_Int32 SAL_CALL available() override
    {
        if (!hasBytes())
            return 0;

        return remainingSize();
    }

    virtual void SAL_CALL closeInput() override
    {
    }
    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) override
    {
        if ( location < 0 || o3tl::make_unsigned(location) > maBytes.size() )
            throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );
        mnPos = location;
    }
    virtual sal_Int64 SAL_CALL getPosition() override
    {
        return mnPos;
    }
    virtual sal_Int64 SAL_CALL getLength() override
    {
        return maBytes.size();
    }
};

}

uno::Reference< XInputStream > ZipFile::createStreamForZipEntry(
            const rtl::Reference< comphelper::RefCountedMutex >& aMutexHolder,
            ZipEntry const & rEntry,
            const ::rtl::Reference< EncryptionData > &rData,
            sal_Int8 nStreamMode,
            ::std::optional<sal_Int64> const oDecryptedSize,
            const bool bUseBufferedStream,
            const OUString& aMediaType )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    rtl::Reference< XUnbufferedStream > xSrcStream = new XUnbufferedStream(
        m_xContext, aMutexHolder, rEntry, xStream, rData, nStreamMode, oDecryptedSize, aMediaType, bRecoveryMode);

    if (!bUseBufferedStream)
        return xSrcStream;

    uno::Reference<io::XInputStream> xBufStream;
#ifndef EMSCRIPTEN
    static const sal_Int32 nThreadingThreshold = 10000;

    // "encrypted-package" is the only data stream, no point in threading it
    if (nThreadingThreshold < xSrcStream->available()
        && rEntry.sPath != "encrypted-package"
        // tdf#160888 no threading for AEAD streams:
        // 1. the whole stream must be read immediately to verify tag
        // 2. XBufferedThreadedStream uses same m_aMutexHolder->GetMutex()
        //    => caller cannot read without deadlock
        && (nStreamMode != UNBUFF_STREAM_DATA
            || !rData.is()
            || rData->m_nEncAlg != xml::crypto::CipherID::AES_GCM_W3C))
    {
        xBufStream = new XBufferedThreadedStream(xSrcStream, xSrcStream->getSize());
    }
    else
#endif
        xBufStream = new XBufferedStream(xSrcStream);

    return xBufStream;
}

uno::Reference< XInputStream > ZipFile::StaticGetDataFromRawStream(
        const rtl::Reference<comphelper::RefCountedMutex>& rMutexHolder,
        const uno::Reference<uno::XComponentContext>& rxContext,
        const uno::Reference<XInputStream>& xStream,
        const ::rtl::Reference<EncryptionData> &rData)
{
    if (!rData.is())
        throw ZipIOException(u"Encrypted stream without encryption data!"_ustr );

    if (!rData->m_aKey.hasElements())
        throw packages::WrongPasswordException(THROW_WHERE);

    uno::Reference<XSeekable> xSeek(xStream, UNO_QUERY);
    if (!xSeek.is())
        throw ZipIOException(u"The stream must be seekable!"_ustr);

    // if we have a digest, then this file is an encrypted one and we should
    // check if we can decrypt it or not
    SAL_WARN_IF(rData->m_nEncAlg != xml::crypto::CipherID::AES_GCM_W3C && !rData->m_aDigest.hasElements(),
            "package", "Can't detect password correctness without digest!");
    if (rData->m_nEncAlg == xml::crypto::CipherID::AES_GCM_W3C)
    {
        // skip header
        xSeek->seek(n_ConstHeaderSize + rData->m_aInitVector.getLength()
                + rData->m_aSalt.getLength() + rData->m_aDigest.getLength());

        try
        {   // XUnbufferedStream does not support XSeekable so wrap it
            ::rtl::Reference<XBufferedStream> const pRet(
                new XBufferedStream(new XUnbufferedStream(rMutexHolder, xStream, rData)));
            // currently XBufferedStream reads the whole stream in its ctor (to
            // verify the tag) - in case this gets changed, explicitly seek here
            pRet->seek(pRet->getLength());
            pRet->seek(0);
            return pRet;
        }
        catch (uno::Exception const&)
        {
            throw packages::WrongPasswordException(THROW_WHERE);
        }
    }
    else if (rData->m_aDigest.hasElements())
    {
        sal_Int32 nSize = sal::static_int_cast<sal_Int32>(xSeek->getLength());
        if (nSize > n_ConstDigestLength + 32)
            nSize = n_ConstDigestLength + 32;

        // skip header
        xSeek->seek(n_ConstHeaderSize + rData->m_aInitVector.getLength() +
                    rData->m_aSalt.getLength() + rData->m_aDigest.getLength());

        // Only want to read enough to verify the digest
        Sequence<sal_Int8> aReadBuffer(nSize);

        xStream->readBytes(aReadBuffer, nSize);

        if (!StaticHasValidPassword(rxContext, aReadBuffer, rData))
            throw packages::WrongPasswordException(THROW_WHERE);
    }

    return new XUnbufferedStream(rMutexHolder, xStream, rData);
}

ZipEnumeration ZipFile::entries()
{
    return aEntries;
}

uno::Reference< XInputStream > ZipFile::getInputStream( ZipEntry& rEntry,
        const ::rtl::Reference< EncryptionData > &rData,
        ::std::optional<sal_Int64> const oDecryptedSize,
        const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( rEntry.nOffset <= 0 )
        readLOC( rEntry );

    // We want to return a rawStream if we either don't have a key or if the
    // key is wrong

    bool bNeedRawStream = rEntry.nMethod == STORED;

    if (oDecryptedSize && rData.is())
    {
        uno::Reference<XInputStream> const xRet(
            checkValidPassword(rEntry, rData, *oDecryptedSize, aMutexHolder));
        if (xRet.is())
        {
            return xRet;
        }
        bNeedRawStream = true;
    }

    return createStreamForZipEntry ( aMutexHolder,
                                    rEntry,
                                    rData,
                                    bNeedRawStream ? UNBUFF_STREAM_RAW : UNBUFF_STREAM_DATA,
                                    oDecryptedSize);
}

uno::Reference< XInputStream > ZipFile::getDataStream( ZipEntry& rEntry,
        const ::rtl::Reference< EncryptionData > &rData,
        ::std::optional<sal_Int64> const oDecryptedSize,
        const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( rEntry.nOffset <= 0 )
        readLOC( rEntry );

    // An exception must be thrown in case stream is encrypted and
    // there is no key or the key is wrong
    bool bNeedRawStream = false;
    if (oDecryptedSize)
    {
        // in case no digest is provided there is no way
        // to detect password correctness
        if ( !rData.is() )
            throw ZipException(u"Encrypted stream without encryption data!"_ustr );

        // if we have a digest, then this file is an encrypted one and we should
        // check if we can decrypt it or not
        SAL_WARN_IF(rData->m_nEncAlg != xml::crypto::CipherID::AES_GCM_W3C && !rData->m_aDigest.hasElements(),
            "package", "Can't detect password correctness without digest!");
        uno::Reference<XInputStream> const xRet(checkValidPassword(rEntry, rData, *oDecryptedSize, aMutexHolder));
        if (!xRet.is())
        {
            throw packages::WrongPasswordException(THROW_WHERE);
        }
        return xRet;
    }
    else
        bNeedRawStream = ( rEntry.nMethod == STORED );

    return createStreamForZipEntry ( aMutexHolder,
                                    rEntry,
                                    rData,
                                    bNeedRawStream ? UNBUFF_STREAM_RAW : UNBUFF_STREAM_DATA,
                                    oDecryptedSize);
}

uno::Reference< XInputStream > ZipFile::getRawData( ZipEntry& rEntry,
        const ::rtl::Reference< EncryptionData >& rData,
        ::std::optional<sal_Int64> const oDecryptedSize,
        const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder,
        const bool bUseBufferedStream )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( rEntry.nOffset <= 0 )
        readLOC( rEntry );

    return createStreamForZipEntry(aMutexHolder, rEntry, rData,
            UNBUFF_STREAM_RAW, oDecryptedSize, bUseBufferedStream);
}

uno::Reference< XInputStream > ZipFile::getWrappedRawStream(
        ZipEntry& rEntry,
        const ::rtl::Reference< EncryptionData >& rData,
        sal_Int64 const nDecryptedSize,
        const OUString& aMediaType,
        const rtl::Reference<comphelper::RefCountedMutex>& aMutexHolder )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    if ( !rData.is() )
        throw packages::NoEncryptionException(THROW_WHERE );

    if ( rEntry.nOffset <= 0 )
        readLOC( rEntry );

    return createStreamForZipEntry(aMutexHolder, rEntry, rData,
            UNBUFF_STREAM_WRAPPEDRAW, nDecryptedSize, true, aMediaType);
}

sal_uInt64 ZipFile::readLOC(ZipEntry &rEntry)
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );
    std::vector<sal_Int8> aNameBuffer;
    std::vector<sal_Int8> aExtraBuffer;
    return readLOC_Impl(rEntry, aNameBuffer, aExtraBuffer);
}

// Pass in a shared name buffer to reduce the number of allocations
// we do when reading the CEN.
sal_uInt64 ZipFile::readLOC_Impl(ZipEntry &rEntry, std::vector<sal_Int8>& rNameBuffer, std::vector<sal_Int8>& rExtraBuffer)
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    sal_Int64 nPos = -rEntry.nOffset;

    aGrabber.seek(nPos);
    std::array<sal_Int8, 30> aHeader;
    if (aGrabber.readBytes(aHeader.data(), 30) != 30)
        throw uno::RuntimeException();
    MemoryByteGrabber headerMemGrabber(aHeader.data(), 30);

    sal_Int32 nTestSig = headerMemGrabber.ReadInt32();
    if (nTestSig != LOCSIG)
        throw ZipIOException(u"Invalid LOC header (bad signature)"_ustr );

    // Ignore all (duplicated) information from the local file header.
    // various programs produced "broken" zip files; even LO at some point.
    // Just verify the path and calculate the data offset and otherwise
    // rely on the central directory info.

    // version - ignore any mismatch (Maven created JARs)
    sal_uInt16 const nVersion = headerMemGrabber.ReadUInt16();
    sal_uInt16 const nLocFlag = headerMemGrabber.ReadUInt16(); // general purpose bit flag
    sal_uInt16 const nLocMethod = headerMemGrabber.ReadUInt16(); // compression method
    // Do *not* compare timestamps, since MSO 2010 can produce documents
    // with timestamp difference in the central directory entry and local
    // file header.
    headerMemGrabber.ReadInt32(); //time
    sal_uInt32 nLocCrc = headerMemGrabber.ReadUInt32(); //crc
    sal_uInt64 nLocCompressedSize = headerMemGrabber.ReadUInt32(); //compressed size
    sal_uInt64 nLocSize = headerMemGrabber.ReadUInt32(); //size
    sal_Int16 nPathLen = headerMemGrabber.ReadInt16();
    sal_Int16 nExtraLen = headerMemGrabber.ReadInt16();

    if (nPathLen < 0)
    {
        SAL_WARN("package", "bogus path len of: " << nPathLen);
        nPathLen = 0;
    }

    rEntry.nOffset = aGrabber.getPosition() + nPathLen + nExtraLen;

    sal_Int64 nEnd = {}; // avoid -Werror=maybe-uninitialized
    bool bBroken = false;

    try
    {
        // read always in UTF8, some tools seem not to set UTF8 bit
        // coverity[tainted_data] - we've checked negative lens, and up to max short is ok here
        rNameBuffer.resize(nPathLen);
        sal_Int32 nRead = aGrabber.readBytes(rNameBuffer.data(), nPathLen);
        std::string_view aNameView(reinterpret_cast<const char *>(rNameBuffer.data()), nRead);

        OUString sLOCPath( aNameView.data(), aNameView.size(), RTL_TEXTENCODING_UTF8 );

        if ( rEntry.nPathLen == -1 ) // the file was created
        {
            rEntry.nPathLen = nPathLen;
            rEntry.sPath = sLOCPath;
        }

        if (rEntry.nPathLen != nPathLen || rEntry.sPath != sLOCPath)
        {
            SAL_INFO("package", "LOC inconsistent name: \"" << rEntry.sPath << "\"");
            bBroken = true;
        }

        bool isZip64{false};
        ::std::optional<sal_uInt64> oOffset64;
        if (nExtraLen != 0)
        {
            rExtraBuffer.resize(nExtraLen);
            aGrabber.readBytes(rExtraBuffer.data(), nExtraLen);
            MemoryByteGrabber extraMemGrabber(rExtraBuffer.data(), nExtraLen);

            isZip64 = readExtraFields(extraMemGrabber, nExtraLen,
                    nLocSize, nLocCompressedSize, oOffset64, &aNameView);
        }
        if (!isZip64 && 45 <= nVersion)
        {
            // for Excel compatibility, assume Zip64 - https://rzymek.github.io/post/excel-zip64/
            isZip64 = true;
        }

        // Just plain ignore bits 1 & 2 of the flag field - they are either
        // purely informative, or even fully undefined (depending on method).
        // Also ignore bit 11 ("Language encoding flag"): tdf125300.docx is
        // example with mismatch - and the actual file names are compared in
        // any case and required to be UTF-8.
        if ((rEntry.nFlag & ~0x806U) != (nLocFlag & ~0x806U))
        {
            SAL_INFO("package", "LOC inconsistent flag: \"" << rEntry.sPath << "\"");
            bBroken = true;
        }

        // TODO: "older versions with encrypted streams write mismatching DEFLATE/STORE" ???
        if (rEntry.nMethod != nLocMethod)
        {
            SAL_INFO("package", "LOC inconsistent method: \"" << rEntry.sPath << "\"");
            bBroken = true;
        }

        if (o3tl::checked_add<sal_Int64>(rEntry.nOffset, rEntry.nCompressedSize, nEnd))
        {
            throw ZipException(u"Integer-overflow"_ustr);
        }

        // read "data descriptor" - this can be 12, 16, 20, or 24 bytes in size
        if ((rEntry.nFlag & 0x08) != 0)
        {
#if 0
            // Unfortunately every encrypted ODF package entry hits this,
            // because ODF requires deflated entry with value STORED and OOo/LO
            // has always written compressed streams with data descriptor.
            // So it is checked later in ZipPackage::checkZipEntriesWithDD()
            if (nLocMethod == STORED)
            {
                SAL_INFO("package", "LOC STORED with data descriptor: \"" << rEntry.sPath << "\"");
                bBroken = true;
            }
            else
#endif
            {
                decltype(nLocCrc) nDDCrc;
                decltype(nLocCompressedSize) nDDCompressedSize;
                decltype(nLocSize) nDDSize;
                aGrabber.seek(aGrabber.getPosition() + rEntry.nCompressedSize);
                sal_uInt32 nTemp = aGrabber.ReadUInt32();
                if (nTemp == 0x08074b50) // APPNOTE says PK78 is optional???
                {
                    nDDCrc = aGrabber.ReadUInt32();
                }
                else
                {
                    nDDCrc = nTemp;
                }
                if (isZip64)
                {
                    nDDCompressedSize = aGrabber.ReadUInt64();
                    nDDSize = aGrabber.ReadUInt64();
                }
                else
                {
                    nDDCompressedSize = aGrabber.ReadUInt32();
                    nDDSize = aGrabber.ReadUInt32();
                }
                if (nEnd < aGrabber.getPosition())
                {
                    nEnd = aGrabber.getPosition();
                }
                else
                {
                    SAL_INFO("package", "LOC invalid size: \"" << rEntry.sPath << "\"");
                    bBroken = true;
                }
                // tdf91429.docx has same values in LOC and in (superfluous) DD
                if ((nLocCrc == 0 || nLocCrc == nDDCrc)
                    && (nLocCompressedSize == 0 || nLocCompressedSize == sal_uInt64(-1) || nLocCompressedSize == nDDCompressedSize)
                    && (nLocSize == 0 || nLocSize == sal_uInt64(-1) || nLocSize == nDDSize))

                {
                    nLocCrc = nDDCrc;
                    nLocCompressedSize = nDDCompressedSize;
                    nLocSize = nDDSize;
                }
                else
                {
                    SAL_INFO("package", "LOC non-0 with data descriptor: \"" << rEntry.sPath << "\"");
                    bBroken = true;
                }
            }
        }

        // unit test file export64.zip has nLocCrc/nLocCS/nLocSize = 0 on mimetype
        if (nLocCrc != 0 && static_cast<sal_uInt32>(rEntry.nCrc) != nLocCrc)
        {
            SAL_INFO("package", "LOC inconsistent CRC: \"" << rEntry.sPath << "\"");
            bBroken = true;
        }

        if (nLocCompressedSize != 0 && static_cast<sal_uInt64>(rEntry.nCompressedSize) != nLocCompressedSize)
        {
            SAL_INFO("package", "LOC inconsistent compressed size: \"" << rEntry.sPath << "\"");
            bBroken = true;
        }

        if (nLocSize != 0 && static_cast<sal_uInt64>(rEntry.nSize) != nLocSize)
        {
            SAL_INFO("package", "LOC inconsistent size: \"" << rEntry.sPath << "\"");
            bBroken = true;
        }

        if (oOffset64 && o3tl::make_unsigned(nPos) != *oOffset64)
        {
            SAL_INFO("package", "LOC inconsistent offset: \"" << rEntry.sPath << "\"");
            bBroken = true;
        }
    }
    catch(...)
    {
        bBroken = true;
    }

    if ( bBroken && !bRecoveryMode )
        throw ZipIOException(u"The stream seems to be broken!"_ustr );

    return nEnd;
}

std::tuple<sal_Int64, sal_Int64, sal_Int64> ZipFile::findCentralDirectory()
{
    // this method is called in constructor only, no need for mutex
    try
    {
        sal_Int64 const nLength = aGrabber.getLength();
        if (nLength < ENDHDR)
        {
            throw ZipException(u"Zip too small!"_ustr);
        }
        sal_Int64 nPos = nLength - ENDHDR - ZIP_MAXNAMELEN;
        sal_Int64 nEnd = nPos >= 0 ? nPos : 0;

        aGrabber.seek( nEnd );

        auto nSize = nLength - nEnd;
        std::unique_ptr<sal_Int8[]> aBuffer(new sal_Int8[nSize]);
        if (nSize != aGrabber.readBytes(aBuffer.get(), nSize))
            throw ZipException(u"Zip END signature not found!"_ustr );

        const sal_Int8 *pBuffer = aBuffer.get();

        sal_Int64 nEndPos = {};
        nPos = nSize - ENDHDR;
        while ( nPos >= 0 )
        {
            if (pBuffer[nPos] == 'P' && pBuffer[nPos+1] == 'K' && pBuffer[nPos+2] == 5 && pBuffer[nPos+3] == 6 )
            {
                nEndPos = nPos + nEnd;
                break;
            }
            if (nPos == 0)
            {
                throw ZipException(u"Zip END signature not found!"_ustr);
            }
            nPos--;
        }

        aGrabber.seek(nEndPos + 4);
        sal_uInt16 const nEndDisk = aGrabber.ReadUInt16();
        if (nEndDisk != 0 && nEndDisk != 0xFFFF)
        {   // only single disk is supported!
            throw ZipException(u"invalid end (disk)"_ustr );
        }
        sal_uInt16 const nEndDirDisk = aGrabber.ReadUInt16();
        if (nEndDirDisk != 0 && nEndDisk != 0xFFFF)
        {
            throw ZipException(u"invalid end (directory disk)"_ustr );
        }
        sal_uInt16 const nEndDiskEntries = aGrabber.ReadUInt16();
        sal_uInt16 const nEndEntries = aGrabber.ReadUInt16();
        if (nEndDiskEntries != nEndEntries)
        {
            throw ZipException(u"invalid end (entries)"_ustr );
        }
        sal_Int32 const nEndDirSize = aGrabber.ReadInt32();
        sal_Int32 const nEndDirOffset = aGrabber.ReadInt32();

        // Zip64 end of central directory locator must immediately precede
        // end of central directory record
        if (20 <= nEndPos)
        {
            aGrabber.seek(nEndPos - 20);
            std::array<sal_Int8, 20> aZip64EndLocator;
            if (20 != aGrabber.readBytes(aZip64EndLocator.data(), 20))
                throw uno::RuntimeException();
            MemoryByteGrabber loc64Grabber(aZip64EndLocator.data(), 20);
            if (loc64Grabber.ReadUInt8() == 'P'
                && loc64Grabber.ReadUInt8() == 'K'
                && loc64Grabber.ReadUInt8() == 6
                && loc64Grabber.ReadUInt8() == 7)
            {
                sal_uInt32 const nLoc64Disk = loc64Grabber.ReadUInt32();
                if (nLoc64Disk != 0)
                {
                    throw ZipException(u"invalid Zip64 end locator (disk)"_ustr);
                }
                sal_Int64 const nLoc64End64Offset = loc64Grabber.ReadUInt64();
                if (nEndPos < 20 + 56 || (nEndPos - 20 - 56) < nLoc64End64Offset
                    || nLoc64End64Offset < 0)
                {
                    throw ZipException(u"invalid Zip64 end locator (offset)"_ustr);
                }
                sal_uInt32 const nLoc64Disks = loc64Grabber.ReadUInt32();
                if (nLoc64Disks != 1)
                {
                    throw ZipException(u"invalid Zip64 end locator (number of disks)"_ustr);
                }
                aGrabber.seek(nLoc64End64Offset);
                std::vector<sal_Int8> aZip64EndDirectory(nEndPos - 20 - nLoc64End64Offset);
                aGrabber.readBytes(aZip64EndDirectory.data(), nEndPos - 20 - nLoc64End64Offset);
                MemoryByteGrabber end64Grabber(aZip64EndDirectory.data(), nEndPos - 20 - nLoc64End64Offset);
                if (end64Grabber.ReadUInt8() != 'P'
                    || end64Grabber.ReadUInt8() != 'K'
                    || end64Grabber.ReadUInt8() != 6
                    || end64Grabber.ReadUInt8() != 6)
                {
                    throw ZipException(u"invalid Zip64 end (signature)"_ustr);
                }
                sal_Int64 const nEnd64Size = end64Grabber.ReadUInt64();
                if (nEnd64Size != nEndPos - 20 - nLoc64End64Offset - 12)
                {
                    throw ZipException(u"invalid Zip64 end (size)"_ustr);
                }
                end64Grabber.ReadUInt16(); // ignore version made by
                end64Grabber.ReadUInt16(); // ignore version needed to extract
                sal_uInt32 const nEnd64Disk = end64Grabber.ReadUInt32();
                if (nEnd64Disk != 0)
                {
                    throw ZipException(u"invalid Zip64 end (disk)"_ustr);
                }
                sal_uInt32 const nEnd64EndDisk = end64Grabber.ReadUInt32();
                if (nEnd64EndDisk != 0)
                {
                    throw ZipException(u"invalid Zip64 end (directory disk)"_ustr);
                }
                sal_uInt64 const nEnd64DiskEntries = end64Grabber.ReadUInt64();
                sal_uInt64 const nEnd64Entries = end64Grabber.ReadUInt64();
                if (nEnd64DiskEntries != nEnd64Entries)
                {
                    throw ZipException(u"invalid Zip64 end (entries)"_ustr);
                }
                sal_Int64 const nEnd64DirSize = end64Grabber.ReadUInt64();
                sal_Int64 const nEnd64DirOffset = end64Grabber.ReadUInt64();
                if (nEndEntries != sal_uInt16(-1) && nEnd64Entries != nEndEntries)
                {
                    throw ZipException(u"inconsistent Zip/Zip64 end (entries)"_ustr);
                }
                if (nEndDirSize != -1
                    && nEnd64DirSize != nEndDirSize)
                {
                    throw ZipException(u"inconsistent Zip/Zip64 end (size)"_ustr);
                }
                if (nEndDirOffset != -1
                    && nEnd64DirOffset != nEndDirOffset)
                {
                    throw ZipException(u"inconsistent Zip/Zip64 end (offset)"_ustr);
                }

                sal_Int64 end;
                if (o3tl::checked_add<sal_Int64>(nEnd64DirOffset, nEnd64DirSize, end)
                    || nLoc64End64Offset < end
                    || nEnd64DirOffset < 0
                    || nLoc64End64Offset - nEnd64DirSize != nEnd64DirOffset)
                {
                    throw ZipException(u"Invalid Zip64 end (bad central directory size)"_ustr);
                }

                return { nEnd64Entries, nEnd64DirSize, nEnd64DirOffset };
            }
        }

        sal_Int32 end;
        if (o3tl::checked_add<sal_Int32>(nEndDirOffset, nEndDirSize, end)
            || nEndPos < end
            || nEndDirOffset < 0
            || nEndPos - nEndDirSize != nEndDirOffset)
        {
            throw ZipException(u"Invalid END header (bad central directory size)"_ustr);
        }

        return { nEndEntries, nEndDirSize, nEndDirOffset };
    }
    catch ( IllegalArgumentException& )
    {
        throw ZipException(u"Zip END signature not found!"_ustr );
    }
    catch ( NotConnectedException& )
    {
        throw ZipException(u"Zip END signature not found!"_ustr );
    }
    catch ( BufferSizeExceededException& )
    {
        throw ZipException(u"Zip END signature not found!"_ustr );
    }
}

sal_Int32 ZipFile::readCEN()
{
    // this method is called in constructor only, no need for mutex
    sal_Int32 nCenPos = -1;

    try
    {
        auto [nTotal, nCenLen, nCenOff] = findCentralDirectory();
        nCenPos = nCenOff; // data before start of zip is not supported

        if ( nTotal > ZIP_MAXENTRIES )
            throw ZipException(u"too many entries in ZIP File"_ustr );

        if (nCenLen < nTotal * CENHDR) // prevent overflow with ZIP_MAXENTRIES
            throw ZipException(u"invalid END header (bad entry count)"_ustr );

        if (nCenLen > SAL_MAX_INT32 || nCenLen < 0)
            throw ZipException(u"central directory too big"_ustr);

        aGrabber.seek(nCenPos);
        std::vector<sal_Int8> aCENBuffer(nCenLen);
        sal_Int64 nRead = aGrabber.readBytes ( aCENBuffer.data(), nCenLen );
        if (nCenLen != nRead)
            throw ZipException (u"Error reading CEN into memory buffer!"_ustr );

        MemoryByteGrabber aMemGrabber(aCENBuffer.data(), nCenLen);

        ZipEntry aEntry;
        sal_Int16 nCommentLen;
        ::std::vector<std::pair<sal_uInt64, sal_uInt64>> unallocated = { { 0, nCenPos } };

        aEntries.reserve(nTotal);
        sal_Int64 nCount;
        std::vector<sal_Int8> aTempNameBuffer;
        std::vector<sal_Int8> aTempExtraBuffer;
        for (nCount = 0 ; nCount < nTotal; nCount++)
        {
            sal_Int32 nTestSig = aMemGrabber.ReadInt32();
            if ( nTestSig != CENSIG )
                throw ZipException(u"Invalid CEN header (bad signature)"_ustr );

            sal_uInt16 versionMadeBy = aMemGrabber.ReadUInt16();
            aEntry.nVersion = aMemGrabber.ReadInt16();
            aEntry.nFlag = aMemGrabber.ReadInt16();

            if ( ( aEntry.nFlag & 1 ) == 1 )
                throw ZipException(u"Invalid CEN header (encrypted entry)"_ustr );

            aEntry.nMethod = aMemGrabber.ReadInt16();

            if ( aEntry.nMethod != STORED && aEntry.nMethod != DEFLATED)
                throw ZipException(u"Invalid CEN header (bad compression method)"_ustr );

            aEntry.nTime = aMemGrabber.ReadInt32();
            aEntry.nCrc = aMemGrabber.ReadInt32();

            sal_uInt64 nCompressedSize = aMemGrabber.ReadUInt32();
            sal_uInt64 nSize = aMemGrabber.ReadUInt32();
            aEntry.nPathLen = aMemGrabber.ReadInt16();
            aEntry.nExtraLen = aMemGrabber.ReadInt16();
            nCommentLen = aMemGrabber.ReadInt16();
            aMemGrabber.skipBytes ( 4 );
            sal_uInt32 externalFileAttributes = aMemGrabber.ReadUInt32();
            sal_uInt64 nOffset = aMemGrabber.ReadUInt32();

            if ( aEntry.nPathLen < 0 )
                throw ZipException(u"unexpected name length"_ustr );

            if ( nCommentLen < 0 )
                throw ZipException(u"unexpected comment length"_ustr );

            if ( aEntry.nExtraLen < 0 )
                throw ZipException(u"unexpected extra header info length"_ustr );

            if (aEntry.nPathLen > aMemGrabber.remainingSize())
                throw ZipException(u"name too long"_ustr);

            // read always in UTF8, some tools seem not to set UTF8 bit
            std::string_view aPathView(reinterpret_cast<char const *>(aMemGrabber.getCurrentPos()), aEntry.nPathLen);
            aEntry.sPath = OUString( aPathView.data(), aPathView.size(), RTL_TEXTENCODING_UTF8 );

            if ( !::comphelper::OStorageHelper::IsValidZipEntryFileName( aEntry.sPath, true ) )
                throw ZipException(u"Zip entry has an invalid name."_ustr );

            aMemGrabber.skipBytes(aEntry.nPathLen);

            if (aEntry.nExtraLen>0)
            {
                ::std::optional<sal_uInt64> oOffset64;
                readExtraFields(aMemGrabber, aEntry.nExtraLen, nSize, nCompressedSize, oOffset64, &aPathView);
                if (oOffset64)
                {
                    nOffset = *oOffset64;
                }
            }
            aEntry.nCompressedSize = nCompressedSize;
            aEntry.nSize = nSize;
            aEntry.nOffset = nOffset;

            if (o3tl::checked_multiply<sal_Int64>(aEntry.nOffset, -1, aEntry.nOffset))
                throw ZipException(u"Integer-overflow"_ustr);

            if (aEntry.nMethod == STORED && aEntry.nCompressedSize != aEntry.nSize)
            {
                throw ZipException(u"entry STORED with inconsistent size"_ustr);
            }

            aMemGrabber.skipBytes(nCommentLen);

            // unfortunately readLOC is required now to check the consistency
            assert(aEntry.nOffset <= 0);
            sal_uInt64 const nStart{ o3tl::make_unsigned(-aEntry.nOffset) };
            sal_uInt64 const nEnd = readLOC_Impl(aEntry, aTempNameBuffer, aTempExtraBuffer);
            assert(nStart < nEnd);
            for (auto it = unallocated.begin(); ; ++it)
            {
                if (it == unallocated.end())
                {
                    throw ZipException(u"overlapping entries"_ustr);
                }
                if (nStart < it->first)
                {
                    throw ZipException(u"overlapping entries"_ustr);
                }
                else if (it->first == nStart)
                {
                    if (it->second == nEnd)
                    {
                        unallocated.erase(it);
                        break;
                    }
                    else if (nEnd < it->second)
                    {
                        it->first = nEnd;
                        break;
                    }
                    else
                    {
                        throw ZipException(u"overlapping entries"_ustr);
                    }
                }
                else if (nStart < it->second)
                {
                    if (nEnd < it->second)
                    {
                        auto const temp{it->first};
                        it->first = nEnd;
                        unallocated.insert(it, { temp, nStart });
                        break;
                    }
                    else if (nEnd == it->second)
                    {
                        it->second = nStart;
                        break;
                    }
                    else
                    {
                        throw ZipException(u"overlapping entries"_ustr);
                    }
                }
            }

            // Is this a FAT-compatible empty entry?
            if (aEntry.nSize == 0 && (versionMadeBy & 0xff00) == 0)
            {
                constexpr sal_uInt32 FILE_ATTRIBUTE_DIRECTORY = 16;
                if (externalFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    continue; // This is a directory entry, not a stream - skip it
            }

            if (aEntries.find(aEntry.sPath) != aEntries.end())
            {
                SAL_INFO("package", "Duplicate CEN entry: \"" << aEntry.sPath << "\"");
                throw ZipException(u"Duplicate CEN entry"_ustr);
            }
            if (aEntries.empty() && m_Checks == Checks::TryCheckInsensitive)
            {
                if (aEntry.sPath == "mimetype" && aEntry.nSize == 0)
                {   // tdf#162866 AutoCorrect uses ODF package, directories are
                    m_Checks = Checks::Default; // user-defined => ignore!
                }
                else
                {
                    m_Checks = Checks::CheckInsensitive;
                }
            }
            // this is required for OOXML, but not for ODF
            auto const lowerPath(aEntry.sPath.toAsciiLowerCase());
            if (!m_EntriesInsensitive.insert(lowerPath).second && m_Checks == Checks::CheckInsensitive)
            {
                SAL_INFO("package", "Duplicate CEN entry (case insensitive): \"" << aEntry.sPath << "\"");
                throw ZipException(u"Duplicate CEN entry (case insensitive)"_ustr);
            }
            aEntries[aEntry.sPath] = aEntry;
        }

        if (nCount != nTotal)
            throw ZipException(u"Count != Total"_ustr );
        if (!unallocated.empty())
        {
            throw ZipException(u"Zip file has holes! It will leak!"_ustr);
        }
    }
    catch ( IllegalArgumentException & )
    {
        // seek can throw this...
        nCenPos = -1; // make sure we return -1 to indicate an error
    }
    return nCenPos;
}

bool ZipFile::readExtraFields(MemoryByteGrabber& aMemGrabber, sal_Int16 nExtraLen,
        sal_uInt64& nSize, sal_uInt64& nCompressedSize,
        std::optional<sal_uInt64> & roOffset,
        std::string_view const * pCENFilenameToCheck)
{
    bool isZip64{false};
    while (nExtraLen > 0) // Extensible data fields
    {
        sal_Int16 nheaderID = aMemGrabber.ReadInt16();
        sal_uInt16 dataSize = aMemGrabber.ReadUInt16();
        if (nheaderID == 1) // Load Zip64 Extended Information Extra Field
        {
            // Datasize should be 28byte but some files have less (maybe non standard?)
            nSize = aMemGrabber.ReadUInt64();
            sal_uInt16 nReadSize = 8;
            if (dataSize >= 16)
            {
                nCompressedSize = aMemGrabber.ReadUInt64();
                nReadSize = 16;
                if (dataSize >= 24)
                {
                    roOffset.emplace(aMemGrabber.ReadUInt64());
                    nReadSize = 24;
                    // 4 byte should be "Disk Start Number" but we not need it
                }
            }
            if (dataSize > nReadSize)
                aMemGrabber.skipBytes(dataSize - nReadSize);
            isZip64 = true;
        }
        // Info-ZIP Unicode Path Extra Field - pointless as we expect UTF-8 in CEN already
        else if (nheaderID == 0x7075 && pCENFilenameToCheck) // ignore in recovery mode
        {
            if (aMemGrabber.remainingSize() < dataSize)
            {
                SAL_INFO("package", "Invalid Info-ZIP Unicode Path Extra Field: invalid TSize");
                throw ZipException(u"Invalid Info-ZIP Unicode Path Extra Field"_ustr);
            }
            auto const nVersion = aMemGrabber.ReadUInt8();
            if (nVersion != 1)
            {
                SAL_INFO("package", "Invalid Info-ZIP Unicode Path Extra Field: unexpected Version");
                throw ZipException(u"Invalid Info-ZIP Unicode Path Extra Field"_ustr);
            }
            // this CRC32 is actually of the pCENFilenameToCheck
            // so it's pointless to check it if we require the UnicodeName
            // to be equal to the CEN name anyway (and pCENFilenameToCheck
            // is already converted to UTF-16 here)
            (void) aMemGrabber.ReadUInt32();
            // this is required to be UTF-8
            std::string_view unicodePath(reinterpret_cast<char const *>(aMemGrabber.getCurrentPos()),
                    dataSize - 5);
            aMemGrabber.skipBytes(dataSize - 5);
            if (unicodePath != *pCENFilenameToCheck)
            {
                SAL_INFO("package", "Invalid Info-ZIP Unicode Path Extra Field: unexpected UnicodeName");
                throw ZipException(u"Invalid Info-ZIP Unicode Path Extra Field"_ustr);
            }
        }
        else
        {
            aMemGrabber.skipBytes(dataSize);
        }
        nExtraLen -= dataSize + 4;
    }
    return isZip64;
}

// PK34: Local file header
void ZipFile::HandlePK34(std::span<const sal_Int8> data, sal_Int64 dataOffset, sal_Int64 totalSize)
{
    ZipEntry aEntry;
    Sequence<sal_Int8> aTmpBuffer(data.data() + 4, 26);
    MemoryByteGrabber aMemGrabber(aTmpBuffer);

    aEntry.nVersion = aMemGrabber.ReadInt16();
    aEntry.nFlag = aMemGrabber.ReadInt16();
    if ((aEntry.nFlag & 1) == 1)
        return;

    aEntry.nMethod = aMemGrabber.ReadInt16();
    if (aEntry.nMethod != STORED && aEntry.nMethod != DEFLATED)
        return;

    aEntry.nTime = aMemGrabber.ReadInt32();
    aEntry.nCrc = aMemGrabber.ReadInt32();
    sal_uInt64 nCompressedSize = aMemGrabber.ReadUInt32();
    sal_uInt64 nSize = aMemGrabber.ReadUInt32();
    aEntry.nPathLen = aMemGrabber.ReadInt16();
    aEntry.nExtraLen = aMemGrabber.ReadInt16();

    const sal_Int32 nDescrLength = (aEntry.nMethod == DEFLATED && (aEntry.nFlag & 8)) ? 16 : 0;
    const sal_Int64 nBlockHeaderLength = aEntry.nPathLen + aEntry.nExtraLen + 30 + nDescrLength;
    if (aEntry.nPathLen < 0 || aEntry.nExtraLen < 0 || dataOffset + nBlockHeaderLength > totalSize)
        return;

    // read always in UTF8, some tools seem not to set UTF8 bit
    if (o3tl::make_unsigned(30 + aEntry.nPathLen) <= data.size())
        aEntry.sPath = OUString(reinterpret_cast<char const*>(data.data() + 30), aEntry.nPathLen,
                                RTL_TEXTENCODING_UTF8);
    else
    {
        std::vector<sal_Int8> aFileName(aEntry.nPathLen);
        aGrabber.seek(dataOffset + 30);
        aEntry.nPathLen = aGrabber.readBytes(aFileName.data(), aEntry.nPathLen);
        aEntry.sPath = OUString(reinterpret_cast<const char*>(aFileName.data()),
                                aEntry.nPathLen, RTL_TEXTENCODING_UTF8);
    }
    aEntry.sPath = aEntry.sPath.replace('\\', '/');

    // read 64bit header
    if (aEntry.nExtraLen > 0)
    {
        std::vector<sal_Int8> aExtraBuffer(aEntry.nExtraLen);
        if (o3tl::make_unsigned(30 + aEntry.nPathLen) + aEntry.nExtraLen <= data.size())
        {
            auto it = data.begin() + 30 + aEntry.nPathLen;
            std::copy(it, it + aEntry.nExtraLen, aExtraBuffer.begin());
        }
        else
        {
            aGrabber.seek(dataOffset + 30 + aEntry.nExtraLen);
            aGrabber.readBytes(aExtraBuffer.data(), aEntry.nExtraLen);
        }
        MemoryByteGrabber aMemGrabberExtra(aExtraBuffer.data(), aEntry.nExtraLen);
        if (aEntry.nExtraLen > 0)
        {
            ::std::optional<sal_uInt64> oOffset64;
            readExtraFields(aMemGrabberExtra, aEntry.nExtraLen, nSize, nCompressedSize, oOffset64, nullptr);
        }
    }

    sal_Int64 nDataSize = (aEntry.nMethod == DEFLATED) ? nCompressedSize : nSize;
    sal_Int64 nBlockLength = nDataSize + nBlockHeaderLength;

    if (dataOffset + nBlockLength > totalSize)
        return;

    aEntry.nCompressedSize = nCompressedSize;
    aEntry.nSize = nSize;

    aEntry.nOffset = dataOffset + 30 + aEntry.nPathLen + aEntry.nExtraLen;

    if ((aEntry.nSize || aEntry.nCompressedSize) && !checkSizeAndCRC(aEntry))
    {
        aEntry.nCrc = 0;
        aEntry.nCompressedSize = 0;
        aEntry.nSize = 0;
    }

    // Do not add this entry, if it is empty and is a directory of an already existing entry
    if (aEntry.nSize == 0 && aEntry.nCompressedSize == 0
        && std::find_if(aEntries.begin(), aEntries.end(),
                        [path = OUString(aEntry.sPath + "/")](const auto& r)
                        { return r.first.startsWith(path); })
               != aEntries.end())
        return;

    auto const lowerPath(aEntry.sPath.toAsciiLowerCase());
    if (m_EntriesInsensitive.find(lowerPath) != m_EntriesInsensitive.end())
    {   // this is required for OOXML, but not for ODF
        return;
    }
    m_EntriesInsensitive.insert(lowerPath);
    aEntries.emplace(aEntry.sPath, aEntry);

    // Drop any "directory" entry corresponding to this one's path; since we don't use
    // central directory, we don't see external file attributes, so sanitize here
    sal_Int32 i = 0;
    for (OUString subdir = aEntry.sPath.getToken(0, '/', i); i >= 0;
         subdir += OUString::Concat("/") + o3tl::getToken(aEntry.sPath, 0, '/', i))
    {
        if (auto it = aEntries.find(subdir); it != aEntries.end())
        {
            // if not empty, let it fail later in ZipPackage::getZipFileContents
            if (it->second.nSize == 0 && it->second.nCompressedSize == 0)
                aEntries.erase(it);
        }
    }
}

// PK78: Data descriptor
void ZipFile::HandlePK78(std::span<const sal_Int8> data, sal_Int64 dataOffset)
{
    sal_Int64 nCompressedSize, nSize;
    Sequence<sal_Int8> aTmpBuffer(data.data() + 4, 12 + 8 + 4);
    MemoryByteGrabber aMemGrabber(aTmpBuffer);
    sal_Int32 nCRC32 = aMemGrabber.ReadInt32();

    // FIXME64: find a better way to recognize if Zip64 mode is used
    // Now we check if the memory at +16 byte seems to be a signature
    // if not, then probably Zip64 mode is used here, except
    // if memory at +24 byte seems not to be a signature.
    // Normally Data Descriptor should followed by the next Local File header
    // that should start with PK34, except for the last file, then it may
    // followed by Central directory that start with PK12, or
    // followed by "archive decryption header" that don't have a signature.
    if ((data[16] == 'P' && data[17] == 'K' && data[19] == data[18] + 1
         && (data[18] == 3 || data[18] == 1))
        || !(data[24] == 'P' && data[25] == 'K' && data[27] == data[26] + 1
             && (data[26] == 3 || data[26] == 1)))
    {
        nCompressedSize = aMemGrabber.ReadUInt32();
        nSize = aMemGrabber.ReadUInt32();
    }
    else
    {
        nCompressedSize = aMemGrabber.ReadUInt64();
        nSize = aMemGrabber.ReadUInt64();
    }

    for (auto& rEntry : aEntries)
    {
        // this is a broken package, accept this block not only for DEFLATED streams
        if ((rEntry.second.nFlag & 8) == 0)
            continue;
        sal_Int64 nStreamOffset = dataOffset - nCompressedSize;
        if (nStreamOffset == rEntry.second.nOffset
            && nCompressedSize > rEntry.second.nCompressedSize)
        {
            // only DEFLATED blocks need to be checked
            bool bAcceptBlock = (rEntry.second.nMethod == STORED && nCompressedSize == nSize);

            if (!bAcceptBlock)
            {
                sal_Int64 nRealSize = 0;
                sal_Int32 nRealCRC = 0;
                getSizeAndCRC(nStreamOffset, nCompressedSize, &nRealSize, &nRealCRC);
                bAcceptBlock = (nRealSize == nSize && nRealCRC == nCRC32);
            }

            if (bAcceptBlock)
            {
                rEntry.second.nCrc = nCRC32;
                rEntry.second.nCompressedSize = nCompressedSize;
                rEntry.second.nSize = nSize;
            }
        }
#if 0
// for now ignore clearly broken streams
        else if( !rEntry.second.nCompressedSize )
        {
            rEntry.second.nCrc = nCRC32;
            sal_Int32 nRealStreamSize = dataOffset - rEntry.second.nOffset;
            rEntry.second.nCompressedSize = nRealStreamSize;
            rEntry.second.nSize = nSize;
        }
#endif
    }
}

void ZipFile::recover()
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    const sal_Int64 nToRead = 32000;
    std::vector<sal_Int8> aBuffer(nToRead);

    try
    {
        const sal_Int64 nLength = aGrabber.getLength();
        if (nLength < ENDHDR)
            return;

        aGrabber.seek( 0 );

        sal_Int32 nRead;
        for( sal_Int64 nGenPos = 0; (nRead = aGrabber.readBytes( aBuffer.data(), nToRead )) && nRead > 16; )
        {
            const sal_Int8 *pBuffer = aBuffer.data();
            const sal_Int32 nBufSize = nRead;

            sal_Int64 nPos = 0;
            // the buffer should contain at least one header,
            // or if it is end of the file, at least the postheader with sizes and hash
            while( nPos < nBufSize - 30
                || ( nBufSize < nToRead && nPos < nBufSize - 16 ) )

            {
                if ( nPos < nBufSize - 30 && pBuffer[nPos] == 'P' && pBuffer[nPos+1] == 'K' && pBuffer[nPos+2] == 3 && pBuffer[nPos+3] == 4 )
                {
                    HandlePK34(std::span(pBuffer + nPos, nBufSize - nPos), nGenPos + nPos, nLength);
                    nPos += 4;
                }
                else if (pBuffer[nPos] == 'P' && pBuffer[nPos+1] == 'K' && pBuffer[nPos+2] == 7 && pBuffer[nPos+3] == 8 )
                {
                    HandlePK78(std::span(pBuffer + nPos, nBufSize - nPos), nGenPos + nPos);
                    nPos += 4;
                }
                else
                    nPos++;
            }

            nGenPos += nPos;
            aGrabber.seek( nGenPos );
        }
    }
    catch ( IllegalArgumentException& )
    {
        throw ZipException(u"Zip END signature not found!"_ustr );
    }
    catch ( NotConnectedException& )
    {
        throw ZipException(u"Zip END signature not found!"_ustr );
    }
    catch ( BufferSizeExceededException& )
    {
        throw ZipException(u"Zip END signature not found!"_ustr );
    }
}

bool ZipFile::checkSizeAndCRC( const ZipEntry& aEntry )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    try
    {
        sal_Int32 nCRC = 0;
        sal_Int64 nSize = 0;

        if( aEntry.nMethod == STORED )
            return ( getCRC( aEntry.nOffset, aEntry.nSize ) == aEntry.nCrc );

        if (aEntry.nCompressedSize < 0)
        {
            SAL_WARN("package", "bogus compressed size of: " << aEntry.nCompressedSize);
            return false;
        }

        getSizeAndCRC( aEntry.nOffset, aEntry.nCompressedSize, &nSize, &nCRC );
        return ( aEntry.nSize == nSize && aEntry.nCrc == nCRC );
    }
    catch (uno::Exception const&)
    {
        return false;
    }
}

sal_Int32 ZipFile::getCRC( sal_Int64 nOffset, sal_Int64 nSize )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    CRC32 aCRC;
    sal_Int64 nBlockSize = ::std::min(nSize, static_cast< sal_Int64 >(32000));
    std::vector<sal_Int8> aBuffer(nBlockSize);

    aGrabber.seek( nOffset );
    sal_Int32 nRead;
    for (sal_Int64 ind = 0;
         (nRead = aGrabber.readBytes( aBuffer.data(), nBlockSize )) && ind * nBlockSize < nSize;
         ++ind)
    {
        aCRC.updateSegment(aBuffer.data(), nRead);
    }

    return aCRC.getValue();
}

void ZipFile::getSizeAndCRC( sal_Int64 nOffset, sal_Int64 nCompressedSize, sal_Int64 *nSize, sal_Int32 *nCRC )
{
    ::osl::MutexGuard aGuard( m_aMutexHolder->GetMutex() );

    CRC32 aCRC;
    sal_Int64 nRealSize = 0;
    ZipUtils::InflaterBytes aInflaterLocal;
    sal_Int32 nBlockSize = static_cast< sal_Int32 > (::std::min( nCompressedSize, static_cast< sal_Int64 >( 32000 ) ) );
    std::vector < sal_Int8 > aBuffer(nBlockSize);
    std::vector< sal_Int8 > aData( nBlockSize );

    aGrabber.seek( nOffset );
    sal_Int32 nRead;
    for ( sal_Int64 ind = 0;
          !aInflaterLocal.finished()
          && (nRead = aGrabber.readBytes( aBuffer.data(), nBlockSize ))
          && ind * nBlockSize < nCompressedSize;
          ind++ )
    {
        sal_Int32 nLastInflated = 0;
        sal_Int64 nInBlock = 0;

        aInflaterLocal.setInput( aBuffer.data(), nRead );
        do
        {
            nLastInflated = aInflaterLocal.doInflateSegment( aData.data(), nBlockSize, 0, nBlockSize );
            aCRC.updateSegment( aData.data(), nLastInflated );
            nInBlock += nLastInflated;
        } while( !aInflater.finished() && nLastInflated );

        nRealSize += nInBlock;
    }

    *nSize = nRealSize;
    *nCRC = aCRC.getValue();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
