/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: binaryinputstream.cxx,v $
 * $Revision: 1.4.22.2 $
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
#include <osl/diagnose.h>
#include <string.h>

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XSeekable;

namespace oox {

const sal_Int32 INPUTSTREAM_BUFFERSIZE      = 0x8000;

// ============================================================================

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

SequenceInputStream::SequenceInputStream( StreamDataSequence& rData ) :
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
        nReadBytes = ::std::min< sal_Int32 >( nBytes, mrData.getLength() - mnPos );
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
        sal_Int32 nSkipBytes = ::std::min< sal_Int32 >( nBytes, mrData.getLength() - mnPos );
        mnPos += nSkipBytes;
        mbEof = nSkipBytes < nBytes;
    }
}

// ============================================================================

} // namespace oox

