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
#include <EntryInputStream.hxx>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <rtl/cipher.h>
#include <rtl/digest.h>
#include <memory.h> // for memcpy

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::zip::ZipConstants;

using ::rtl::OUString;

/** Provides access to the compressed data in a zipfile.
 *
 * uncompresses the stream into memory and seeks on it 'in memory'
 * This and the ZipPackageBuffer used in the ZipOutputStream are memory hogs
 * and will hopefully be replaced eventually
 *
 * Acts on the same underlying XInputStream as both the full Zip File and other
 * EntryInputStreams, and thus must maintain its current position in the stream and
 * seek to it before performing any reads.
 */

EntryInputStream::EntryInputStream( Reference < io::XInputStream > xNewInput,
                                    const ZipEntry & rNewEntry,
                                    const rtl::Reference < EncryptionData > &xEncryptData,
                                    sal_Bool bGetRawStream)
: xStream( xNewInput )
, xSeek( xNewInput, UNO_QUERY )
, aEntry (rNewEntry )
, nCurrent( 0 )
, bHaveInMemory ( sal_False )
, aInflater( sal_True )
, aBuffer( 0 )
, xEncryptionData (xEncryptData)
, bRawStream (bGetRawStream)
{
    if (bGetRawStream)
    {
        nUncompressedSize = aEntry.nMethod == DEFLATED ? aEntry.nCompressedSize : aEntry.nSize;
        nEnd = aEntry.nOffset + nUncompressedSize;
    }
    else
    {
        nEnd = aEntry.nMethod == DEFLATED ? aEntry.nOffset + aEntry.nCompressedSize : aEntry.nOffset + aEntry.nSize;
        nUncompressedSize = aEntry.nSize;
    }
}
void EntryInputStream::readIntoMemory()
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, RuntimeException)
{
    if (!bHaveInMemory)
    {
        Sequence < sal_Int8 > aReadBuffer;
        xSeek->seek(aEntry.nOffset);
        sal_Int32 nSize = aEntry.nMethod == DEFLATED ? aEntry.nCompressedSize : aEntry.nSize;

        if (nSize <0)
            throw io::BufferSizeExceededException(::rtl::OUString(), *this);

        xStream->readBytes( aReadBuffer, nSize ); // Now it holds the raw stuff from disk

        if (xEncryptionData->aSalt.getLength())
        {
            // Have salt, will travel
            Sequence < sal_uInt8 > aDerivedKey (16);
            rtlCipherError aResult;
            Sequence < sal_Int8 > aDecryptBuffer;

            // Get the key
            rtl_digest_PBKDF2 ( aDerivedKey.getArray(), 16,
                                reinterpret_cast < const sal_uInt8 * > (xEncryptionData->aKey.getConstArray()),
                                xEncryptionData->aKey.getLength(),
                                xEncryptionData->aSalt.getConstArray(),
                                xEncryptionData->aSalt.getLength(),
                                xEncryptionData->nIterationCount );

            rtlCipher aCipher = rtl_cipher_create (rtl_Cipher_AlgorithmBF, rtl_Cipher_ModeStream);
            aResult = rtl_cipher_init( aCipher, rtl_Cipher_DirectionDecode,
                                       aDerivedKey.getConstArray(),
                                       aDerivedKey.getLength(),
                                       xEncryptionData->aInitVector.getConstArray(),
                                       xEncryptionData->aInitVector.getLength());
            OSL_ASSERT (aResult == rtl_Cipher_E_None);
            aDecryptBuffer.realloc ( nSize );
            aResult = rtl_cipher_decode ( aCipher,
                                          aReadBuffer.getConstArray(),
                                          nSize,
                                          reinterpret_cast < sal_uInt8 * > (aDecryptBuffer.getArray()),
                                          nSize);
            OSL_ASSERT (aResult == rtl_Cipher_E_None);
            aReadBuffer = aDecryptBuffer; // Now it holds the decrypted data
        }
        if (bRawStream || aEntry.nMethod == STORED)
            aBuffer = aReadBuffer; // bRawStream means the caller doesn't want it decompressed
        else
        {
            aInflater.setInputSegment(aReadBuffer, 0, nSize );
            aBuffer.realloc( aEntry.nSize );
            aInflater.doInflate(aBuffer);
            aInflater.end();
        }
        bHaveInMemory = sal_True;
    }
}
EntryInputStream::~EntryInputStream( void )
{
}

sal_Int32 SAL_CALL EntryInputStream::readBytes( Sequence< sal_Int8 >& aData,
                                        sal_Int32 nBytesToRead )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, RuntimeException)
{
    if (nBytesToRead <0)
        throw io::BufferSizeExceededException(::rtl::OUString(), *this);
    if (!bHaveInMemory)
        readIntoMemory();
    if (nBytesToRead + nCurrent > nUncompressedSize)
        nBytesToRead = static_cast < sal_Int32> ( nUncompressedSize - nCurrent );

    aData.realloc( nBytesToRead );
    memcpy(aData.getArray(), aBuffer.getConstArray() + nCurrent, nBytesToRead);
    nCurrent+=nBytesToRead;

    return nBytesToRead;
}
sal_Int32 SAL_CALL EntryInputStream::readSomeBytes( Sequence< sal_Int8 >& aData,
                                                sal_Int32 nMaxBytesToRead )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, RuntimeException)
{
    return readBytes( aData, nMaxBytesToRead );
}
void SAL_CALL EntryInputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, RuntimeException)
{
    if (nBytesToSkip < 0)
        throw io::BufferSizeExceededException(::rtl::OUString(), *this);

    if (nBytesToSkip + nCurrent > nUncompressedSize)
        nBytesToSkip = static_cast < sal_Int32 > (nUncompressedSize- nCurrent);

    nCurrent+=nBytesToSkip;
}
sal_Int32 SAL_CALL EntryInputStream::available(  )
    throw(io::NotConnectedException, io::IOException, RuntimeException)
{
    return static_cast < sal_Int32 > (nUncompressedSize - nCurrent);
}
void SAL_CALL EntryInputStream::closeInput(  )
    throw(io::NotConnectedException, io::IOException, RuntimeException)
{
}

void SAL_CALL EntryInputStream::seek( sal_Int64 location )
    throw(lang::IllegalArgumentException, io::IOException, RuntimeException)
{
    if (location > nUncompressedSize)
        location = nUncompressedSize;
    if (location <0)
        location = 0;
    nCurrent = location;
}
sal_Int64 SAL_CALL EntryInputStream::getPosition(  )
        throw(io::IOException, RuntimeException)
{
    return nCurrent;
}
sal_Int64 SAL_CALL EntryInputStream::getLength(  )
        throw(io::IOException, RuntimeException)
{
    return nUncompressedSize;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
