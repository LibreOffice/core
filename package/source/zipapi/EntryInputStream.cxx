/*************************************************************************
 *
 *  $RCSfile: EntryInputStream.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 14:13:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _ENTRY_INPUT_STREAM_HXX
#include <EntryInputStream.hxx>
#endif
#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPCONSTANTS_HPP_
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#endif
#ifndef _RTL_CIPHER_H_
#include <rtl/cipher.h>
#endif
#ifndef _RTL_DIGEST_H_
#include <rtl/digest.h>
#endif
#include <memory.h> // for memcpy

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::packages::zip;
using namespace com::sun::star::packages::zip::ZipConstants;

/** Provides access to the compressed data in a zipfile.
 *
 * 04/12/00 - uncompresses the stream into memory and seeks on it 'in memory'
 * This and the ZipPackageBuffer used in the ZipOutputStream are memory hogs
 * and will hopefully be replaced eventually
 *
 * Acts on the same underlying XInputStream as both the full Zip File and other
 * EntryInputStreams, and thus must maintain its current position in the stream and
 * seek to it before performing any reads.
 */

EntryInputStream::EntryInputStream( Reference < io::XInputStream > xNewInput,
                                    const ZipEntry & rNewEntry,
                                    const vos::ORef < EncryptionData > &xEncryptData,
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
