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

#include "oox/helper/binaryinputstream.hxx"

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <string.h>
#include <vector>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/binaryoutputstream.hxx"

namespace oox {

// ============================================================================

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OStringToOUString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace {

const sal_Int32 INPUTSTREAM_BUFFERSIZE      = 0x8000;

} // namespace

// ============================================================================

OString BinaryInputStream::readNulCharArray()
{
    OStringBuffer aBuffer;
    for( sal_uInt8 nChar = readuInt8(); !mbEof && (nChar > 0); readValue( nChar ) )
        aBuffer.append( static_cast< sal_Char >( nChar ) );
    return aBuffer.makeStringAndClear();
}

OUString BinaryInputStream::readNulCharArrayUC( rtl_TextEncoding eTextEnc )
{
    return OStringToOUString( readNulCharArray(), eTextEnc );
}

OUString BinaryInputStream::readNulUnicodeArray()
{
    OUStringBuffer aBuffer;
    for( sal_uInt16 nChar = readuInt16(); !mbEof && (nChar > 0); readValue( nChar ) )
        aBuffer.append( static_cast< sal_Unicode >( nChar ) );
    return aBuffer.makeStringAndClear();
}

OString BinaryInputStream::readCharArray( sal_Int32 nChars, bool bAllowNulChars )
{
    if( nChars <= 0 )
        return OString();

    ::std::vector< sal_uInt8 > aBuffer;
    sal_Int32 nCharsRead = readArray( aBuffer, nChars );
    if( nCharsRead <= 0 )
        return OString();

    aBuffer.resize( static_cast< size_t >( nCharsRead ) );
    if( !bAllowNulChars )
        ::std::replace( aBuffer.begin(), aBuffer.end(), '\0', '?' );

    return OString( reinterpret_cast< sal_Char* >( &aBuffer.front() ), nCharsRead );
}

OUString BinaryInputStream::readCharArrayUC( sal_Int32 nChars, rtl_TextEncoding eTextEnc, bool bAllowNulChars )
{
    return OStringToOUString( readCharArray( nChars, bAllowNulChars ), eTextEnc );
}

OUString BinaryInputStream::readUnicodeArray( sal_Int32 nChars, bool bAllowNulChars )
{
    if( nChars <= 0 )
        return OUString();

    ::std::vector< sal_uInt16 > aBuffer;
    sal_Int32 nCharsRead = readArray( aBuffer, nChars );
    if( nCharsRead <= 0 )
        return OUString();

    aBuffer.resize( static_cast< size_t >( nCharsRead ) );
    if( !bAllowNulChars )
        ::std::replace( aBuffer.begin(), aBuffer.begin() + nCharsRead, '\0', '?' );

    OUStringBuffer aStringBuffer;
    aStringBuffer.ensureCapacity( nCharsRead );
    for( ::std::vector< sal_uInt16 >::iterator aIt = aBuffer.begin(), aEnd = aBuffer.end(); aIt != aEnd; ++aIt )
        aStringBuffer.append( static_cast< sal_Unicode >( *aIt ) );
    return aStringBuffer.makeStringAndClear();
}

OUString BinaryInputStream::readCompressedUnicodeArray( sal_Int32 nChars, bool bCompressed, bool bAllowNulChars )
{
    return bCompressed ?
         // ISO-8859-1 maps all byte values 0xHH to the same Unicode code point U+00HH
        readCharArrayUC( nChars, RTL_TEXTENCODING_ISO_8859_1, bAllowNulChars ) :
        readUnicodeArray( nChars, bAllowNulChars );
}

void BinaryInputStream::copyToStream( BinaryOutputStream& rOutStrm, sal_Int64 nBytes, sal_Int32 nAtomSize )
{
    if( nBytes > 0 )
    {
        // make buffer size a multiple of the passed atom size
        sal_Int32 nBufferSize = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, (INPUTSTREAM_BUFFERSIZE / nAtomSize) * nAtomSize );
        StreamDataSequence aBuffer( nBufferSize );
        while( nBytes > 0 )
        {
            sal_Int32 nReadSize = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, nBufferSize );
            sal_Int32 nBytesRead = readData( aBuffer, nReadSize, nAtomSize );
            rOutStrm.writeData( aBuffer );
            if( nReadSize == nBytesRead )
                nBytes -= nReadSize;
            else
                nBytes = 0;
        }
    }
}

// ============================================================================

BinaryXInputStream::BinaryXInputStream( const Reference< XInputStream >& rxInStrm, bool bAutoClose ) :
    BinaryStreamBase( Reference< XSeekable >( rxInStrm, UNO_QUERY ).is() ),
    BinaryXSeekableStream( Reference< XSeekable >( rxInStrm, UNO_QUERY ) ),
    maBuffer( INPUTSTREAM_BUFFERSIZE ),
    mxInStrm( rxInStrm ),
    mbAutoClose( bAutoClose && rxInStrm.is() )
{
    mbEof = !mxInStrm.is();
}

BinaryXInputStream::~BinaryXInputStream()
{
    close();
}

void BinaryXInputStream::close()
{
    OSL_ENSURE( !mbAutoClose || mxInStrm.is(), "BinaryXInputStream::close - invalid call" );
    if( mbAutoClose && mxInStrm.is() ) try
    {
        mxInStrm->closeInput();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryXInputStream::close - closing input stream failed" );
    }
    mxInStrm.clear();
    mbAutoClose = false;
    BinaryXSeekableStream::close();
}

sal_Int32 BinaryXInputStream::readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t /*nAtomSize*/ )
{
    sal_Int32 nRet = 0;
    if( !mbEof && (nBytes > 0) ) try
    {
        nRet = mxInStrm->readBytes( orData, nBytes );
        mbEof = nRet != nBytes;
    }
    catch( Exception& )
    {
        mbEof = true;
    }
    return nRet;
}

sal_Int32 BinaryXInputStream::readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize )
{
    sal_Int32 nRet = 0;
    if( !mbEof && (nBytes > 0) )
    {
        sal_Int32 nBufferSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, INPUTSTREAM_BUFFERSIZE );
        sal_uInt8* opnMem = reinterpret_cast< sal_uInt8* >( opMem );
        while( !mbEof && (nBytes > 0) )
        {
            sal_Int32 nReadSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, nBufferSize );
            sal_Int32 nBytesRead = readData( maBuffer, nReadSize, nAtomSize );
            if( nBytesRead > 0 )
                memcpy( opnMem, maBuffer.getConstArray(), static_cast< size_t >( nBytesRead ) );
            opnMem += nBytesRead;
            nBytes -= nBytesRead;
            nRet += nBytesRead;
        }
    }
    return nRet;
}

void BinaryXInputStream::skip( sal_Int32 nBytes, size_t /*nAtomSize*/ )
{
    if( !mbEof ) try
    {
        mxInStrm->skipBytes( nBytes );
    }
    catch( Exception& )
    {
        mbEof = true;
    }
}

// ============================================================================

SequenceInputStream::SequenceInputStream( const StreamDataSequence& rData ) :
    BinaryStreamBase( true ),
    SequenceSeekableStream( rData )
{
}

sal_Int32 SequenceInputStream::readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t /*nAtomSize*/ )
{
    sal_Int32 nReadBytes = 0;
    if( !mbEof )
    {
        nReadBytes = getMaxBytes( nBytes );
        orData.realloc( nReadBytes );
        if( nReadBytes > 0 )
            memcpy( orData.getArray(), mpData->getConstArray() + mnPos, static_cast< size_t >( nReadBytes ) );
        mnPos += nReadBytes;
        mbEof = nReadBytes < nBytes;
    }
    return nReadBytes;
}

sal_Int32 SequenceInputStream::readMemory( void* opMem, sal_Int32 nBytes, size_t /*nAtomSize*/ )
{
    sal_Int32 nReadBytes = 0;
    if( !mbEof )
    {
        nReadBytes = getMaxBytes( nBytes );
        if( nReadBytes > 0 )
            memcpy( opMem, mpData->getConstArray() + mnPos, static_cast< size_t >( nReadBytes ) );
        mnPos += nReadBytes;
        mbEof = nReadBytes < nBytes;
    }
    return nReadBytes;
}

void SequenceInputStream::skip( sal_Int32 nBytes, size_t /*nAtomSize*/ )
{
    if( !mbEof )
    {
        sal_Int32 nSkipBytes = getMaxBytes( nBytes );
        mnPos += nSkipBytes;
        mbEof = nSkipBytes < nBytes;
    }
}

// ============================================================================

RelativeInputStream::RelativeInputStream( BinaryInputStream& rInStrm, sal_Int64 nSize ) :
    BinaryStreamBase( rInStrm.isSeekable() ),
    mpInStrm( &rInStrm ),
    mnStartPos( rInStrm.tell() ),
    mnRelPos( 0 )
{
    sal_Int64 nRemaining = rInStrm.getRemaining();
    mnSize = (nRemaining >= 0) ? ::std::min( nSize, nRemaining ) : nSize;
    mbEof = mbEof || rInStrm.isEof() || (mnSize < 0);
}

sal_Int64 RelativeInputStream::size() const
{
    return mpInStrm ? mnSize : -1;
}

sal_Int64 RelativeInputStream::tell() const
{
    return mpInStrm ? mnRelPos : -1;
}

void RelativeInputStream::seek( sal_Int64 nPos )
{
    if( mpInStrm && isSeekable() && (mnStartPos >= 0) )
    {
        mnRelPos = getLimitedValue< sal_Int64, sal_Int64 >( nPos, 0, mnSize );
        mpInStrm->seek( mnStartPos + mnRelPos );
        mbEof = (mnRelPos != nPos) || mpInStrm->isEof();
    }
}

void RelativeInputStream::close()
{
    mpInStrm = 0;
    mbEof = true;
}

sal_Int32 RelativeInputStream::readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize )
{
    sal_Int32 nReadBytes = 0;
    if( !mbEof )
    {
        sal_Int32 nMaxBytes = getMaxBytes( nBytes );
        nReadBytes = mpInStrm->readData( orData, nMaxBytes, nAtomSize );
        mnRelPos += nReadBytes;
        mbEof = (nMaxBytes < nBytes) || mpInStrm->isEof();
    }
    return nReadBytes;
}

sal_Int32 RelativeInputStream::readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize )
{
    sal_Int32 nReadBytes = 0;
    if( !mbEof )
    {
        sal_Int32 nMaxBytes = getMaxBytes( nBytes );
        nReadBytes = mpInStrm->readMemory( opMem, nMaxBytes, nAtomSize );
        mnRelPos += nReadBytes;
        mbEof = (nMaxBytes < nBytes) || mpInStrm->isEof();
    }
    return nReadBytes;
}

void RelativeInputStream::skip( sal_Int32 nBytes, size_t nAtomSize )
{
    if( !mbEof )
    {
        sal_Int32 nSkipBytes = getMaxBytes( nBytes );
        mpInStrm->skip( nSkipBytes, nAtomSize );
        mnRelPos += nSkipBytes;
        mbEof = nSkipBytes < nBytes;
    }
}

// ============================================================================

} // namespace oox
