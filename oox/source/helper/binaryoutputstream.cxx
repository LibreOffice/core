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

#include "oox/helper/binaryoutputstream.hxx"
#include <osl/diagnose.h>
#include "oox/helper/binaryinputstream.hxx"
#include <string.h>

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::XSeekable;

namespace oox {

const sal_Int32 OUTPUTSTREAM_BUFFERSIZE     = 0x8000;

// ============================================================================

void BinaryOutputStream::copyStream( BinaryInputStream& rInStrm, sal_Int64 nBytes )
{
    if( nBytes > 0 )
    {
        sal_Int32 nBufferSize = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, OUTPUTSTREAM_BUFFERSIZE );
        StreamDataSequence aBuffer( nBufferSize );
        while( nBytes > 0 )
        {
            sal_Int32 nReadSize = getLimitedValue< sal_Int32, sal_Int64 >( nBytes, 0, nBufferSize );
            sal_Int32 nBytesRead = rInStrm.readData( aBuffer, nReadSize );
            writeData( aBuffer );
            if( nReadSize == nBytesRead )
                nBytes -= nReadSize;
            else
                nBytes = 0;
        }
    }
}

void BinaryOutputStream::writeAtom( const void* pMem, sal_uInt8 nSize )
{
    writeMemory( pMem, nSize );
}

// ============================================================================

BinaryXOutputStream::BinaryXOutputStream( const Reference< XOutputStream >& rxOutStrm, bool bAutoClose ) :
    BinaryXSeekableStream( Reference< XSeekable >( rxOutStrm, UNO_QUERY ) ),
    maBuffer( OUTPUTSTREAM_BUFFERSIZE ),
    mxOutStrm( rxOutStrm ),
    mbAutoClose( bAutoClose )
{
    mbEof = !mxOutStrm.is();
}

BinaryXOutputStream::~BinaryXOutputStream()
{
    if( mbAutoClose )
        close();
}

void BinaryXOutputStream::writeData( const StreamDataSequence& rData )
{
    try
    {
        OSL_ENSURE( mxOutStrm.is(), "BinaryXOutputStream::writeData - invalid call" );
        mxOutStrm->writeBytes( rData );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryXOutputStream::writeData - stream read error" );
    }
}

void BinaryXOutputStream::writeMemory( const void* pMem, sal_Int32 nBytes )
{
    if( nBytes > 0 )
    {
        sal_Int32 nBufferSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, OUTPUTSTREAM_BUFFERSIZE );
        const sal_uInt8* pnMem = reinterpret_cast< const sal_uInt8* >( pMem );
        while( nBytes > 0 )
        {
            sal_Int32 nWriteSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, nBufferSize );
            maBuffer.realloc( nWriteSize );
            memcpy( maBuffer.getArray(), pnMem, static_cast< size_t >( nWriteSize ) );
            writeData( maBuffer );
            pnMem += nWriteSize;
            nBytes -= nWriteSize;
        }
    }
}

void BinaryXOutputStream::close()
{
    if( mxOutStrm.is() ) try
    {
        mxOutStrm->flush();
        mxOutStrm->closeOutput();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryXOutputStream::close - closing output stream failed" );
    }
}

// ============================================================================

SequenceOutputStream::SequenceOutputStream( StreamDataSequence& rData ) :
    SequenceSeekableStream( rData )
{
}

void SequenceOutputStream::writeData( const StreamDataSequence& rData )
{
    if( rData.hasElements() )
        writeMemory( rData.getConstArray(), rData.getLength() );
}

void SequenceOutputStream::writeMemory( const void* pMem, sal_Int32 nBytes )
{
    if( nBytes > 0 )
    {
        if( mrData.getLength() - mnPos < nBytes )
            const_cast< StreamDataSequence& >( mrData ).realloc( mnPos + nBytes );
        memcpy( const_cast< StreamDataSequence& >( mrData ).getArray() + mnPos, pMem, static_cast< size_t >( nBytes ) );
        mnPos += nBytes;
    }
}

// ============================================================================

} // namespace oox

