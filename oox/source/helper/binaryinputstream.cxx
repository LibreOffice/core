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

#include "oox/helper/binaryinputstream.hxx"

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <string.h>
#include <vector>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include "oox/helper/binaryoutputstream.hxx"

namespace oox {

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

namespace {

const sal_Int32 INPUTSTREAM_BUFFERSIZE      = 0x8000;

} // namespace

OUString BinaryInputStream::readNulUnicodeArray()
{
    OUStringBuffer aBuffer;
    for (;;)
    {
      sal_uInt16 nChar = readuInt16();
      if ( mbEof || (nChar == 0) ) break;
      aBuffer.append( static_cast< sal_Unicode >( nChar ) );
    }
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
    if( mxInStrm.is() ) try
    {
        mxInStrm->closeInput();
    }
    catch( Exception& )
    {
        OSL_FAIL( "BinaryXInputStream::close - closing input stream failed" );
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
        sal_uInt8* opnMem = static_cast< sal_uInt8* >( opMem );
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
    mpInStrm = nullptr;
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

} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
