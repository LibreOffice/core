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
#include <string.h>
#include <vector>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/binaryoutputstream.hxx"

using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OStringToOUString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XSeekable;

namespace oox {

const sal_Int32 INPUTSTREAM_BUFFERSIZE      = 0x8000;

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

    ::std::vector< sal_Char > aBuffer( static_cast< size_t >( nChars ) );
    size_t nCharsRead = static_cast< size_t >( readMemory( &aBuffer.front(), nChars ) );
    if( !bAllowNulChars )
        ::std::replace( aBuffer.begin(), aBuffer.begin() + nCharsRead, '\0', '?' );
    return OString( &aBuffer.front(), nCharsRead );
}

OUString BinaryInputStream::readCharArrayUC( sal_Int32 nChars, rtl_TextEncoding eTextEnc, bool bAllowNulChars )
{
    return OStringToOUString( readCharArray( nChars, bAllowNulChars ), eTextEnc );
}

OUString BinaryInputStream::readUnicodeArray( sal_Int32 nChars, bool bAllowNulChars )
{
    OUStringBuffer aBuffer;
    if( nChars > 0 )
    {
        aBuffer.ensureCapacity( nChars );
        sal_uInt16 nChar;
        for( sal_uInt16 nCharIdx = 0; !mbEof && (nCharIdx < nChars); ++nCharIdx )
        {
            readValue( nChar );
            aBuffer.append( static_cast< sal_Unicode >( (!bAllowNulChars && (nChar == 0)) ? '?' : nChar ) );
        }
    }
    return aBuffer.makeStringAndClear();
}

void BinaryInputStream::copyToStream( BinaryOutputStream& rOutStrm, sal_Int64 nBytes )
{
    if( nBytes > 0 )
    {
        sal_Int32 nBufferSize = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, INPUTSTREAM_BUFFERSIZE );
        StreamDataSequence aBuffer( nBufferSize );
        while( nBytes > 0 )
        {
            sal_Int32 nReadSize = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, nBufferSize );
            sal_Int32 nBytesRead = readData( aBuffer, nReadSize );
            rOutStrm.writeData( aBuffer );
            if( nReadSize == nBytesRead )
                nBytes -= nReadSize;
            else
                nBytes = 0;
        }
    }
}

void BinaryInputStream::readAtom( void* opMem, sal_uInt8 nSize )
{
    readMemory( opMem, nSize );
}

// ============================================================================

BinaryXInputStream::BinaryXInputStream( const Reference< XInputStream >& rxInStrm, bool bAutoClose ) :
    BinaryXSeekableStream( Reference< XSeekable >( rxInStrm, UNO_QUERY ) ),
    maBuffer( INPUTSTREAM_BUFFERSIZE ),
    mxInStrm( rxInStrm ),
    mbAutoClose( bAutoClose )
{
    mbEof = !mxInStrm.is();
}

BinaryXInputStream::~BinaryXInputStream()
{
    if( mbAutoClose )
        close();
}

sal_Int32 BinaryXInputStream::readData( StreamDataSequence& orData, sal_Int32 nBytes )
{
    sal_Int32 nRet = 0;
    if( !mbEof && (nBytes > 0) ) try
    {
        OSL_ENSURE( mxInStrm.is(), "BinaryXInputStream::readData - invalid call" );
        nRet = mxInStrm->readBytes( orData, nBytes );
        mbEof = nRet != nBytes;
    }
    catch( Exception& )
    {
        mbEof = true;
    }
    return nRet;
}

sal_Int32 BinaryXInputStream::readMemory( void* opMem, sal_Int32 nBytes )
{
    sal_Int32 nRet = 0;
    if( !mbEof && (nBytes > 0) )
    {
        sal_Int32 nBufferSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, INPUTSTREAM_BUFFERSIZE );
        sal_uInt8* opnMem = reinterpret_cast< sal_uInt8* >( opMem );
        while( !mbEof && (nBytes > 0) )
        {
            sal_Int32 nReadSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, nBufferSize );
            sal_Int32 nBytesRead = readData( maBuffer, nReadSize );
            if( nBytesRead > 0 )
                memcpy( opnMem, maBuffer.getConstArray(), static_cast< size_t >( nBytesRead ) );
            opnMem += nBytesRead;
            nBytes -= nBytesRead;
            nRet += nBytesRead;
        }
    }
    return nRet;
}

void BinaryXInputStream::skip( sal_Int32 nBytes )
{
    if( !mbEof ) try
    {
        OSL_ENSURE( mxInStrm.is(), "BinaryXInputStream::skip - invalid call" );
        mxInStrm->skipBytes( nBytes );
    }
    catch( Exception& )
    {
        mbEof = true;
    }
}

void BinaryXInputStream::close()
{
    if( mxInStrm.is() ) try
    {
        mxInStrm->closeInput();
        mxInStrm.clear();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryXInputStream::close - closing input stream failed" );
    }
}

// ============================================================================

SequenceInputStream::SequenceInputStream( const StreamDataSequence& rData ) :
    SequenceSeekableStream( rData )
{
}

sal_Int32 SequenceInputStream::readData( StreamDataSequence& orData, sal_Int32 nBytes )
{
    sal_Int32 nReadBytes = 0;
    if( !mbEof )
    {
        nReadBytes = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, mrData.getLength() - mnPos );
        orData.realloc( nReadBytes );
        if( nReadBytes > 0 )
            memcpy( orData.getArray(), mrData.getConstArray() + mnPos, static_cast< size_t >( nReadBytes ) );
        mnPos += nReadBytes;
        mbEof = nReadBytes < nBytes;
    }
    return nReadBytes;
}

sal_Int32 SequenceInputStream::readMemory( void* opMem, sal_Int32 nBytes )
{
    sal_Int32 nReadBytes = 0;
    if( !mbEof )
    {
        nReadBytes = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, mrData.getLength() - mnPos );
        if( nReadBytes > 0 )
            memcpy( opMem, mrData.getConstArray() + mnPos, static_cast< size_t >( nReadBytes ) );
        mnPos += nReadBytes;
        mbEof = nReadBytes < nBytes;
    }
    return nReadBytes;
}

void SequenceInputStream::skip( sal_Int32 nBytes )
{
    if( !mbEof )
    {
        sal_Int32 nSkipBytes = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, mrData.getLength() - mnPos );
        mnPos += nSkipBytes;
        mbEof = nSkipBytes < nBytes;
    }
}

// ============================================================================

RelativeInputStream::RelativeInputStream( BinaryInputStream& rInStrm, sal_Int64 nLength ) :
    mrInStrm( rInStrm ),
    mnStartPos( rInStrm.tell() ),
    mnRelPos( 0 )
{
    sal_Int64 nRemaining = rInStrm.getRemaining();
    mnLength = (nRemaining >= 0) ? ::std::min( nLength, nRemaining ) : nLength;
    mbEof = mnLength < 0;
}

bool RelativeInputStream::isSeekable() const
{
    return mrInStrm.isSeekable();
}

sal_Int64 RelativeInputStream::getLength() const
{
    return mnLength;
}

sal_Int64 RelativeInputStream::tell() const
{
    return mnRelPos;
}

void RelativeInputStream::seek( sal_Int64 nPos )
{
    if( mrInStrm.isSeekable() && (mnStartPos >= 0) )
    {
        mnRelPos = getLimitedValue< sal_Int64, sal_Int64 >( nPos, 0, mnLength );
        mrInStrm.seek( mnStartPos + mnRelPos );
        mbEof = (mnRelPos != nPos) || mrInStrm.isEof();
    }
}

sal_Int32 RelativeInputStream::readData( StreamDataSequence& orData, sal_Int32 nBytes )
{
    sal_Int32 nReadBytes = 0;
    if( !mbEof )
    {
        sal_Int32 nRealBytes = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, mnLength - mnRelPos );
        nReadBytes = mrInStrm.readData( orData, nRealBytes );
        mnRelPos += nReadBytes;
        mbEof = (nRealBytes < nBytes) || mrInStrm.isEof();
    }
    return nReadBytes;
}

sal_Int32 RelativeInputStream::readMemory( void* opMem, sal_Int32 nBytes )
{
    sal_Int32 nReadBytes = 0;
    if( !mbEof )
    {
        sal_Int32 nRealBytes = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, mnLength - mnRelPos );
        nReadBytes = mrInStrm.readMemory( opMem, nRealBytes );
        mnRelPos += nReadBytes;
        mbEof = (nRealBytes < nBytes) || mrInStrm.isEof();
    }
    return nReadBytes;
}

void RelativeInputStream::skip( sal_Int32 nBytes )
{
    if( !mbEof )
    {
        sal_Int32 nSkipBytes = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, mnLength - mnRelPos );
        mrInStrm.skip( nSkipBytes );
        mnRelPos += nSkipBytes;
        mbEof = nSkipBytes < nBytes;
    }
}

// ============================================================================

} // namespace oox

