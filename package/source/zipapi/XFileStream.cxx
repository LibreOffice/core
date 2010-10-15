/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <XFileStream.hxx>
#include <EncryptionData.hxx>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <PackageConstants.hxx>
#include <rtl/cipher.h>
#include <ZipFile.hxx>
#include <EncryptedDataHeader.hxx>
#include <com/sun/star/io/XOutputStream.hpp>

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star::io;
using namespace com::sun::star::uno;
using com::sun::star::lang::IllegalArgumentException;
using ::rtl::OUString;

XFileStream::XFileStream( ZipEntry & rEntry,
                           com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xNewZipStream,
                           com::sun::star::uno::Reference < com::sun::star::io::XInputStream > xNewTempStream,
                           const rtl::Reference < EncryptionData > &rData,
                           sal_Bool bNewRawStream,
                           sal_Bool bIsEncrypted )
: maEntry ( rEntry )
, mxData ( rData )
, mbRawStream ( bNewRawStream )
, mbFinished ( sal_False )
, mxTempIn ( xNewTempStream )
, mxTempSeek ( xNewTempStream, UNO_QUERY )
, mxTempOut ( xNewTempStream, UNO_QUERY )
, mxZipStream ( xNewZipStream )
, mxZipSeek ( xNewZipStream, UNO_QUERY )
, maInflater ( sal_True )
, maCipher ( NULL )
{
    mnZipCurrent = maEntry.nOffset;
    if (mbRawStream)
    {
        mnZipSize = maEntry.nMethod == DEFLATED ? maEntry.nCompressedSize : maEntry.nSize;
        mnZipEnd = maEntry.nOffset + mnZipSize;
    }
    else
    {
        mnZipSize = maEntry.nSize;
        mnZipEnd = maEntry.nMethod == DEFLATED ? maEntry.nOffset + maEntry.nCompressedSize : maEntry.nOffset + maEntry.nSize;
    }

    if ( bIsEncrypted )
    {
        sal_Bool bHaveEncryptData = ( !rData.isEmpty() && rData->aSalt.getLength() && rData->aInitVector.getLength() && rData->nIterationCount != 0 ) ? sal_True : sal_False;

        // if we have all the encrypted data, and want a raw stream, then prepend it to the stream, otherwise
        // make a cipher so we can decrypt it
        if ( bHaveEncryptData )
        {
            if ( !bNewRawStream )
                ZipFile::StaticGetCipher ( rData, maCipher, sal_True );
            else
            {
                // Put in the EncryptedDataHeader
                Sequence < sal_Int8 > aEncryptedDataHeader ( n_ConstHeaderSize +
                                                             rData->aInitVector.getLength() +
                                                             rData->aSalt.getLength() +
                                                             rData->aDigest.getLength() );
                sal_Int8 * pHeader = aEncryptedDataHeader.getArray();
                ZipFile::StaticFillHeader ( rData, rEntry.nSize, pHeader );
                mxTempOut->writeBytes ( aEncryptedDataHeader );
                mnZipSize += mxTempSeek->getPosition();
                mxTempSeek->seek ( 0 );
            }
        }
    }
}

XFileStream::~XFileStream()
{
    if ( maCipher )
        rtl_cipher_destroy ( maCipher );
}

void XFileStream::fill( sal_Int64 nUntil)
{
    sal_Int32 nRead;
    sal_Int64 nPosition = mxTempSeek->getPosition();
    mxTempSeek->seek ( mxTempSeek->getLength() );
    maBuffer.realloc ( n_ConstBufferSize );

    while ( mxTempSeek->getLength() < nUntil )
    {
        if ( !mbRawStream )
        {
            while ( 0 == ( nRead = maInflater.doInflate( maBuffer ) ) )
            {
                if ( maInflater.finished() || maInflater.needsDictionary() )
                {
                    // some error handling ?
                    return;
                }

                sal_Int64 nDiff = mnZipEnd - mnZipCurrent;
                if ( nDiff > 0 )
                {
                    mxZipSeek->seek ( mnZipCurrent );
                    nRead = mxZipStream->readBytes ( maCompBuffer, static_cast < sal_Int32 > ( nDiff < n_ConstBufferSize ? nDiff : n_ConstBufferSize ) );
                    mnZipCurrent += nRead;
                    // maCompBuffer now has the uncompressed data, check if we need to decrypt
                    // before passing to the Inflater
                    if ( maCipher )
                    {
                        Sequence < sal_Int8 > aCryptBuffer ( nRead );
                        rtlCipherError aResult = rtl_cipher_decode ( maCipher,
                                      maCompBuffer.getConstArray(),
                                      nRead,
                                      reinterpret_cast < sal_uInt8 * > (aCryptBuffer.getArray()),
                                      nRead);
                        OSL_ASSERT (aResult == rtl_Cipher_E_None);
                        maCompBuffer = aCryptBuffer; // Now it holds the decrypted data

                    }
                    maInflater.setInput ( maCompBuffer );
                }
                else
                {
                    // some error handling ?
                    return;
                }
            }
        }
        else
        {
            sal_Int64 nDiff = mnZipEnd - mnZipCurrent;
            mxZipSeek->seek ( mnZipCurrent );
            nRead = mxZipStream->readBytes ( maBuffer, static_cast < sal_Int32 > ( nDiff < n_ConstBufferSize ? nDiff : n_ConstBufferSize ) );
            mnZipCurrent += nRead;
        }
        Sequence < sal_Int8 > aTmpBuffer ( maBuffer.getConstArray(), nRead );
        mxTempOut->writeBytes ( aTmpBuffer );
    }
    mxTempSeek->seek ( nPosition );
}

sal_Int32 SAL_CALL XFileStream::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    sal_Int64 nPosition = mxTempSeek->getPosition();
    if ( nPosition + nBytesToRead > mnZipSize )
        nBytesToRead = static_cast < sal_Int32 > ( mnZipSize - nPosition );

    sal_Int64 nUntil = nBytesToRead + nPosition + n_ConstBufferSize;
    if (nUntil > mnZipSize )
        nUntil = mnZipSize;
    if ( nUntil > mxTempSeek->getLength() )
        fill ( nUntil );
    sal_Int32 nRead = mxTempIn->readBytes ( aData, nBytesToRead );
    return nRead;
}

sal_Int32 SAL_CALL XFileStream::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return readBytes ( aData, nMaxBytesToRead );
}
void SAL_CALL XFileStream::skipBytes( sal_Int32 nBytesToSkip )
        throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    seek ( mxTempSeek->getPosition() + nBytesToSkip );
}

sal_Int32 SAL_CALL XFileStream::available(  )
        throw( NotConnectedException, IOException, RuntimeException)
{
    return static_cast < sal_Int32 > ( mnZipSize - mxTempSeek->getPosition() );
}

void SAL_CALL XFileStream::closeInput(  )
        throw( NotConnectedException, IOException, RuntimeException)
{
}
void SAL_CALL XFileStream::seek( sal_Int64 location )
        throw( IllegalArgumentException, IOException, RuntimeException)
{
    if ( location > mnZipSize || location < 0 )
        throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >(), 1 );
    if ( location > mxTempSeek->getLength() )
    {
        sal_Int64 nUntil = location + n_ConstBufferSize > mnZipSize ? mnZipSize : location + n_ConstBufferSize;
        fill ( nUntil );
    }
    mxTempSeek->seek ( location );
}
sal_Int64 SAL_CALL XFileStream::getPosition(  )
        throw(IOException, RuntimeException)
{
    return mxTempSeek->getPosition();
}
sal_Int64 SAL_CALL XFileStream::getLength(  )
        throw(IOException, RuntimeException)
{
    return mnZipSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
