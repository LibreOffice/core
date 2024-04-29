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

#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>

#include "XUnbufferedStream.hxx"
#include <EncryptionData.hxx>
#include <ZipFile.hxx>
#include <EncryptedDataHeader.hxx>
#include <algorithm>
#include <string.h>

#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <utility>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using com::sun::star::packages::zip::ZipIOException;

XUnbufferedStream::XUnbufferedStream(
                      const uno::Reference< uno::XComponentContext >& xContext,
                      rtl::Reference< comphelper::RefCountedMutex > aMutexHolder,
                      ZipEntry const & rEntry,
                      Reference < XInputStream > const & xNewZipStream,
                      const ::rtl::Reference< EncryptionData >& rData,
                      sal_Int8 nStreamMode,
                      bool bIsEncrypted,
                      const OUString& aMediaType,
                      bool bRecoveryMode )
: maMutexHolder(std::move( aMutexHolder ))
, mxZipStream ( xNewZipStream )
, mxZipSeek ( xNewZipStream, UNO_QUERY )
, maEntry ( rEntry )
, mnBlockSize( 1 )
, maInflater ( true )
, mbRawStream ( nStreamMode == UNBUFF_STREAM_RAW || nStreamMode == UNBUFF_STREAM_WRAPPEDRAW )
, mbWrappedRaw ( nStreamMode == UNBUFF_STREAM_WRAPPEDRAW )
, mnHeaderToRead ( 0 )
, mnZipCurrent ( 0 )
, mnZipEnd ( 0 )
, mnZipSize ( 0 )
, mnMyCurrent ( 0 )
, mbCheckCRC(!bRecoveryMode)
{
    mnZipCurrent = maEntry.nOffset;
    sal_Int64 nSize;
    if ( mbRawStream )
    {
        mnZipSize = maEntry.nMethod == DEFLATED ? maEntry.nCompressedSize : maEntry.nSize;
        nSize = mnZipSize;
    }
    else
    {
        mnZipSize = maEntry.nSize;
        nSize = maEntry.nMethod == DEFLATED ? maEntry.nCompressedSize : maEntry.nSize;
    }

    if (mnZipSize < 0)
        throw ZipIOException(u"The stream seems to be broken!"_ustr);

    if (o3tl::checked_add(maEntry.nOffset, nSize, mnZipEnd))
        throw ZipIOException(u"Integer-overflow"_ustr);

    bool bHaveEncryptData = rData.is() && rData->m_aInitVector.hasElements() &&
        ((rData->m_aSalt.hasElements() && (rData->m_oPBKDFIterationCount || rData->m_oArgon2Args))
         ||
         rData->m_aKey.hasElements());
    bool bMustDecrypt = nStreamMode == UNBUFF_STREAM_DATA && bHaveEncryptData && bIsEncrypted;

    if ( bMustDecrypt )
    {
        m_xCipherContext = ZipFile::StaticGetCipher( xContext, rData, false );
        // this is only relevant when padding is used
        mnBlockSize = ( rData->m_nEncAlg == xml::crypto::CipherID::AES_CBC_W3C_PADDING ? 16 : 1 );
    }

    if ( !(bHaveEncryptData && mbWrappedRaw && bIsEncrypted) )
        return;

    // if we have the data needed to decrypt it, but didn't want it decrypted (or
    // we couldn't decrypt it due to wrong password), then we prepend this
    // data to the stream

    // Make a buffer big enough to hold both the header and the data itself
    maHeader.realloc  ( n_ConstHeaderSize +
                        rData->m_aInitVector.getLength() +
                        rData->m_aSalt.getLength() +
                        rData->m_aDigest.getLength() +
                        aMediaType.getLength() * sizeof( sal_Unicode ) );
    sal_Int8 * pHeader = maHeader.getArray();
    ZipFile::StaticFillHeader( rData, rEntry.nSize, aMediaType, pHeader );
    mnHeaderToRead = static_cast < sal_Int16 > ( maHeader.getLength() );
    mnZipSize += mnHeaderToRead;
}

// allows to read package raw stream
XUnbufferedStream::XUnbufferedStream(
                    rtl::Reference< comphelper::RefCountedMutex > aMutexHolder,
                    const Reference < XInputStream >& xRawStream,
                    const ::rtl::Reference< EncryptionData >& rData )
: maMutexHolder(std::move( aMutexHolder ))
, mxZipStream ( xRawStream )
, mxZipSeek ( xRawStream, UNO_QUERY )
, mnBlockSize( 1 )
, maInflater ( true )
, mbRawStream ( false )
, mbWrappedRaw ( false )
, mnHeaderToRead ( 0 )
, mnZipCurrent ( 0 )
, mnZipEnd ( 0 )
, mnZipSize ( 0 )
, mnMyCurrent ( 0 )
, mbCheckCRC( false )
{
    // for this scenario maEntry is not set !!!
    OSL_ENSURE( mxZipSeek.is(), "The stream must be seekable!" );

    // skip raw header, it must be already parsed to rData
    mnZipCurrent = n_ConstHeaderSize + rData->m_aInitVector.getLength() +
                            rData->m_aSalt.getLength() + rData->m_aDigest.getLength();

    try {
        if ( mxZipSeek.is() )
            mnZipSize = mxZipSeek->getLength();
    } catch( const Exception& )
    {
        // in case of problem the size will stay set to 0
        TOOLS_WARN_EXCEPTION("package", "ignoring");
    }

    mnZipEnd = mnZipCurrent + mnZipSize;

    // the raw data will not be decrypted, no need for the cipher
    // m_xCipherContext = ZipFile::StaticGetCipher( xContext, rData, false );
}

XUnbufferedStream::~XUnbufferedStream()
{
}

sal_Int32 SAL_CALL XUnbufferedStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    ::osl::MutexGuard aGuard( maMutexHolder->GetMutex() );

    sal_Int32 nRequestedBytes = nBytesToRead;
    OSL_ENSURE( !mnHeaderToRead || mbWrappedRaw, "Only encrypted raw stream can be provided with header!" );
    if ( mnMyCurrent + nRequestedBytes > mnZipSize + maHeader.getLength() )
        nRequestedBytes = static_cast < sal_Int32 > ( mnZipSize + maHeader.getLength() - mnMyCurrent );

    sal_Int32 nTotal = 0;
    aData.realloc ( nRequestedBytes );
    if ( nRequestedBytes )
    {
        sal_Int32 nRead = 0;
        sal_Int32 nLastRead = 0;
        if ( mbRawStream )
        {
            sal_Int64 nDiff = mnZipEnd - mnZipCurrent;

            if ( mbWrappedRaw && mnHeaderToRead )
            {
                sal_Int16 nHeadRead = static_cast< sal_Int16 >(( nRequestedBytes > mnHeaderToRead ?
                                                                                 mnHeaderToRead : nRequestedBytes ));
                memcpy ( aData.getArray(), maHeader.getConstArray() + maHeader.getLength() - mnHeaderToRead, nHeadRead );
                mnHeaderToRead = mnHeaderToRead - nHeadRead;

                if ( nHeadRead < nRequestedBytes )
                {
                    sal_Int32 nToRead = nRequestedBytes - nHeadRead;
                    nToRead = ( nDiff < nToRead ) ? sal::static_int_cast< sal_Int32 >( nDiff ) : nToRead;

                    Sequence< sal_Int8 > aPureData( nToRead );
                    mxZipSeek->seek ( mnZipCurrent );
                    nRead = mxZipStream->readBytes ( aPureData, nToRead );
                    mnZipCurrent += nRead;

                    aPureData.realloc( nRead );
                    if ( mbCheckCRC )
                        maCRC.update( aPureData );

                    aData.realloc( nHeadRead + nRead );

                    const sal_Int8* pPureBuffer = aPureData.getConstArray();
                    sal_Int8* pBuffer = aData.getArray();
                    for ( sal_Int32 nInd = 0; nInd < nRead; nInd++ )
                        pBuffer[ nHeadRead + nInd ] = pPureBuffer[ nInd ];
                }

                nRead += nHeadRead;
            }
            else
            {
                mxZipSeek->seek ( mnZipCurrent );

                nRead = mxZipStream->readBytes (
                                        aData,
                                        std::min<sal_Int64>(nDiff, nRequestedBytes) );

                mnZipCurrent += nRead;

                aData.realloc( nRead );
                if ( mbWrappedRaw && mbCheckCRC )
                    maCRC.update( aData );
            }
        }
        else
        {
            for (;;)
            {
                nLastRead = maInflater.doInflateSegment( aData, nRead, aData.getLength() - nRead );
                if ( 0 != nLastRead && ( nRead + nLastRead == nRequestedBytes || mnZipCurrent >= mnZipEnd ) )
                    break;
                nRead += nLastRead;
                if ( nRead > nRequestedBytes )
                    throw RuntimeException(
                        u"Should not be possible to read more than requested!"_ustr );

                if ( maInflater.finished() || maInflater.getLastInflateError() )
                    throw ZipIOException(u"The stream seems to be broken!"_ustr );

                if ( maInflater.needsDictionary() )
                    throw ZipIOException(u"Dictionaries are not supported!"_ustr );

                sal_Int32 nDiff = static_cast< sal_Int32 >( mnZipEnd - mnZipCurrent );
                if ( nDiff <= 0 )
                {
                    throw ZipIOException(u"The stream seems to be broken!"_ustr );
                }

                mxZipSeek->seek ( mnZipCurrent );

                sal_Int32 nToRead = std::max( nRequestedBytes, static_cast< sal_Int32 >( 8192 ) );
                if ( mnBlockSize > 1 )
                    nToRead = nToRead + mnBlockSize - nToRead % mnBlockSize;
                nToRead = std::min( nDiff, nToRead );

                sal_Int32 nZipRead = mxZipStream->readBytes( maCompBuffer, nToRead );
                if ( nZipRead < nToRead )
                    throw ZipIOException(u"No expected data!"_ustr );

                mnZipCurrent += nZipRead;
                // maCompBuffer now has the data, check if we need to decrypt
                // before passing to the Inflater
                if ( m_xCipherContext.is() )
                {
                    if ( mbCheckCRC )
                        maCRC.update( maCompBuffer );

                    maCompBuffer = m_xCipherContext->convertWithCipherContext( maCompBuffer );
                    if ( mnZipCurrent == mnZipEnd )
                    {
                        // this should throw if AEAD is in use and the tag fails to validate
                        uno::Sequence< sal_Int8 > aSuffix = m_xCipherContext->finalizeCipherContextAndDispose();
                        if ( aSuffix.hasElements() )
                        {
                            sal_Int32 nOldLen = maCompBuffer.getLength();
                            maCompBuffer.realloc( nOldLen + aSuffix.getLength() );
                            memcpy( maCompBuffer.getArray() + nOldLen, aSuffix.getConstArray(), aSuffix.getLength() );
                        }
                    }
                }
                maInflater.setInput ( maCompBuffer );

            }
        }

        mnMyCurrent += nRead + nLastRead;
        nTotal = nRead + nLastRead;
        if ( nTotal < nRequestedBytes)
            aData.realloc ( nTotal );

        if ( mbCheckCRC && ( !mbRawStream || mbWrappedRaw ) )
        {
            if ( !m_xCipherContext.is() && !mbWrappedRaw )
                maCRC.update( aData );

            if ( mnZipSize + maHeader.getLength() == mnMyCurrent && maCRC.getValue() != maEntry.nCrc )
                throw ZipIOException(u"The stream seems to be broken!"_ustr );
        }
    }

    return nTotal;
}

sal_Int32 SAL_CALL XUnbufferedStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    return readBytes ( aData, nMaxBytesToRead );
}
void SAL_CALL XUnbufferedStream::skipBytes( sal_Int32 nBytesToSkip )
{
    if ( nBytesToSkip )
    {
        Sequence < sal_Int8 > aSequence ( nBytesToSkip );
        readBytes ( aSequence, nBytesToSkip );
    }
}

sal_Int32 SAL_CALL XUnbufferedStream::available(  )
{
    //available size must include the prepended header in case of wrapped raw stream
    return static_cast< sal_Int32 > ( std::min< sal_Int64 >( SAL_MAX_INT32, (mnZipSize + mnHeaderToRead - mnMyCurrent) ) );
}

void SAL_CALL XUnbufferedStream::closeInput(  )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
