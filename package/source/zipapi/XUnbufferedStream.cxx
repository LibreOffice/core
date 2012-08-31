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

#include <XUnbufferedStream.hxx>
#include <EncryptionData.hxx>
#include <PackageConstants.hxx>
#include <ZipFile.hxx>
#include <EncryptedDataHeader.hxx>
#include <algorithm>
#include <string.h>

#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::packages::zip::ZipIOException;
using ::rtl::OUString;

XUnbufferedStream::XUnbufferedStream(
                      const uno::Reference< lang::XMultiServiceFactory >& xFactory,
                      SotMutexHolderRef aMutexHolder,
                      ZipEntry & rEntry,
                      Reference < XInputStream > xNewZipStream,
                      const ::rtl::Reference< EncryptionData >& rData,
                      sal_Int8 nStreamMode,
                      sal_Bool bIsEncrypted,
                      const ::rtl::OUString& aMediaType,
                      sal_Bool bRecoveryMode )
: maMutexHolder( aMutexHolder.Is() ? aMutexHolder : SotMutexHolderRef( new SotMutexHolder ) )
, mxZipStream ( xNewZipStream )
, mxZipSeek ( xNewZipStream, UNO_QUERY )
, maEntry ( rEntry )
, mxData ( rData )
, mnBlockSize( 1 )
, maInflater ( sal_True )
, mbRawStream ( nStreamMode == UNBUFF_STREAM_RAW || nStreamMode == UNBUFF_STREAM_WRAPPEDRAW )
, mbWrappedRaw ( nStreamMode == UNBUFF_STREAM_WRAPPEDRAW )
, mbFinished ( sal_False )
, mnHeaderToRead ( 0 )
, mnZipCurrent ( 0 )
, mnZipEnd ( 0 )
, mnZipSize ( 0 )
, mnMyCurrent ( 0 )
, mbCheckCRC( !bRecoveryMode )
{
    mnZipCurrent = maEntry.nOffset;
    if ( mbRawStream )
    {
        mnZipSize = maEntry.nMethod == DEFLATED ? maEntry.nCompressedSize : maEntry.nSize;
        mnZipEnd = maEntry.nOffset + mnZipSize;
    }
    else
    {
        mnZipSize = maEntry.nSize;
        mnZipEnd = maEntry.nMethod == DEFLATED ? maEntry.nOffset + maEntry.nCompressedSize : maEntry.nOffset + maEntry.nSize;
    }

    if (mnZipSize < 0)
        throw ZipIOException(OUString("The stream seems to be broken!"), uno::Reference< XInterface >());

    sal_Bool bHaveEncryptData = ( rData.is() && rData->m_aSalt.getLength() && rData->m_aInitVector.getLength() && rData->m_nIterationCount != 0 ) ? sal_True : sal_False;
    sal_Bool bMustDecrypt = ( nStreamMode == UNBUFF_STREAM_DATA && bHaveEncryptData && bIsEncrypted ) ? sal_True : sal_False;

    if ( bMustDecrypt )
    {
        m_xCipherContext = ZipFile::StaticGetCipher( xFactory, rData, false );
        mnBlockSize = ( rData->m_nEncAlg == xml::crypto::CipherID::AES_CBC_W3C_PADDING ? 16 : 1 );
    }

    if ( bHaveEncryptData && mbWrappedRaw && bIsEncrypted )
    {
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
    }
}

// allows to read package raw stream
XUnbufferedStream::XUnbufferedStream(
                    const uno::Reference< lang::XMultiServiceFactory >& /*xFactory*/,
                    const Reference < XInputStream >& xRawStream,
                    const ::rtl::Reference< EncryptionData >& rData )
: maMutexHolder( new SotMutexHolder )
, mxZipStream ( xRawStream )
, mxZipSeek ( xRawStream, UNO_QUERY )
, mxData ( rData )
, mnBlockSize( 1 )
, maInflater ( sal_True )
, mbRawStream ( sal_False )
, mbWrappedRaw ( sal_False )
, mbFinished ( sal_False )
, mnHeaderToRead ( 0 )
, mnZipCurrent ( 0 )
, mnZipEnd ( 0 )
, mnZipSize ( 0 )
, mnMyCurrent ( 0 )
, mbCheckCRC( sal_False )
{
    // for this scenario maEntry is not set !!!
    OSL_ENSURE( mxZipSeek.is(), "The stream must be seekable!\n" );

    // skip raw header, it must be already parsed to rData
    mnZipCurrent = n_ConstHeaderSize + rData->m_aInitVector.getLength() +
                            rData->m_aSalt.getLength() + rData->m_aDigest.getLength();

    try {
        if ( mxZipSeek.is() )
            mnZipSize = mxZipSeek->getLength();
    } catch( Exception& )
    {
        // in case of problem the size will stay set to 0
    }

    mnZipEnd = mnZipCurrent + mnZipSize;

    // the raw data will not be decrypted, no need for the cipher
    // m_xCipherContext = ZipFile::StaticGetCipher( xFactory, rData, false );
}

XUnbufferedStream::~XUnbufferedStream()
{
}

sal_Int32 SAL_CALL XUnbufferedStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
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

                    sal_Int8* pPureBuffer = aPureData.getArray();
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
                                        static_cast < sal_Int32 > ( nDiff < nRequestedBytes ? nDiff : nRequestedBytes ) );

                mnZipCurrent += nRead;

                aData.realloc( nRead );
                if ( mbWrappedRaw && mbCheckCRC )
                    maCRC.update( aData );
            }
        }
        else
        {
            while ( 0 == ( nLastRead = maInflater.doInflateSegment( aData, nRead, aData.getLength() - nRead ) ) ||
                  ( nRead + nLastRead != nRequestedBytes && mnZipCurrent < mnZipEnd ) )
            {
                nRead += nLastRead;

                if ( nRead > nRequestedBytes )
                    throw RuntimeException(
                        "Should not be possible to read more then requested!",
                        Reference< XInterface >() );

                if ( maInflater.finished() || maInflater.getLastInflateError() )
                    throw ZipIOException("The stream seems to be broken!",
                                        Reference< XInterface >() );

                if ( maInflater.needsDictionary() )
                    throw ZipIOException("Dictionaries are not supported!",
                                        Reference< XInterface >() );

                sal_Int32 nDiff = static_cast< sal_Int32 >( mnZipEnd - mnZipCurrent );
                if ( nDiff > 0 )
                {
                    mxZipSeek->seek ( mnZipCurrent );

                    sal_Int32 nToRead = std::max( nRequestedBytes, static_cast< sal_Int32 >( 8192 ) );
                    if ( mnBlockSize > 1 )
                        nToRead = nToRead + mnBlockSize - nToRead % mnBlockSize;
                    nToRead = std::min( nDiff, nToRead );

                    sal_Int32 nZipRead = mxZipStream->readBytes( maCompBuffer, nToRead );
                    if ( nZipRead < nToRead )
                        throw ZipIOException("No expected data!",
                                            Reference< XInterface >() );

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
                            uno::Sequence< sal_Int8 > aSuffix = m_xCipherContext->finalizeCipherContextAndDispose();
                            if ( aSuffix.getLength() )
                            {
                                sal_Int32 nOldLen = maCompBuffer.getLength();
                                maCompBuffer.realloc( nOldLen + aSuffix.getLength() );
                                memcpy( maCompBuffer.getArray() + nOldLen, aSuffix.getConstArray(), aSuffix.getLength() );
                            }
                        }
                    }
                    maInflater.setInput ( maCompBuffer );
                }
                else
                {
                    throw ZipIOException("The stream seems to be broken!",
                                        Reference< XInterface >() );
                }
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
                throw ZipIOException("The stream seems to be broken!",
                                    Reference< XInterface >() );
        }
    }

    return nTotal;
}

sal_Int32 SAL_CALL XUnbufferedStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return readBytes ( aData, nMaxBytesToRead );
}
void SAL_CALL XUnbufferedStream::skipBytes( sal_Int32 nBytesToSkip )
        throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if ( nBytesToSkip )
    {
        Sequence < sal_Int8 > aSequence ( nBytesToSkip );
        readBytes ( aSequence, nBytesToSkip );
    }
}

sal_Int32 SAL_CALL XUnbufferedStream::available(  )
        throw( NotConnectedException, IOException, RuntimeException)
{
    return static_cast < sal_Int32 > ( mnZipSize - mnMyCurrent );
}

void SAL_CALL XUnbufferedStream::closeInput(  )
        throw( NotConnectedException, IOException, RuntimeException)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
