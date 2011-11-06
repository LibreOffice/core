/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "oox/helper/binaryoutputstream.hxx"

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <osl/diagnose.h>
#include <string.h>

namespace oox {

// ============================================================================

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;

namespace {

const sal_Int32 OUTPUTSTREAM_BUFFERSIZE     = 0x8000;

} // namespace

// ============================================================================

BinaryXOutputStream::BinaryXOutputStream( const Reference< XOutputStream >& rxOutStrm, bool bAutoClose ) :
    BinaryStreamBase( Reference< XSeekable >( rxOutStrm, UNO_QUERY ).is() ),
    BinaryXSeekableStream( Reference< XSeekable >( rxOutStrm, UNO_QUERY ) ),
    maBuffer( OUTPUTSTREAM_BUFFERSIZE ),
    mxOutStrm( rxOutStrm ),
    mbAutoClose( bAutoClose && rxOutStrm.is() )
{
    mbEof = !mxOutStrm.is();
}

BinaryXOutputStream::~BinaryXOutputStream()
{
    close();
}

void BinaryXOutputStream::close()
{
    OSL_ENSURE( !mbAutoClose || mxOutStrm.is(), "BinaryXOutputStream::close - invalid call" );
    if( mxOutStrm.is() ) try
    {
        mxOutStrm->flush();
        if( mbAutoClose )
            mxOutStrm->closeOutput();
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryXOutputStream::close - closing output stream failed" );
    }
    mxOutStrm.clear();
    mbAutoClose = false;
    BinaryXSeekableStream::close();
}

void BinaryXOutputStream::writeData( const StreamDataSequence& rData, size_t /*nAtomSize*/ )
{
    if( mxOutStrm.is() ) try
    {
        mxOutStrm->writeBytes( rData );
    }
    catch( Exception& )
    {
        OSL_ENSURE( false, "BinaryXOutputStream::writeData - stream read error" );
    }
}

void BinaryXOutputStream::writeMemory( const void* pMem, sal_Int32 nBytes, size_t nAtomSize )
{
    if( mxOutStrm.is() && (nBytes > 0) )
    {
        sal_Int32 nBufferSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, (OUTPUTSTREAM_BUFFERSIZE / nAtomSize) * nAtomSize );
        const sal_uInt8* pnMem = reinterpret_cast< const sal_uInt8* >( pMem );
        while( nBytes > 0 )
        {
            sal_Int32 nWriteSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, nBufferSize );
            maBuffer.realloc( nWriteSize );
            memcpy( maBuffer.getArray(), pnMem, static_cast< size_t >( nWriteSize ) );
            writeData( maBuffer, nAtomSize );
            pnMem += nWriteSize;
            nBytes -= nWriteSize;
        }
    }
}

// ============================================================================

SequenceOutputStream::SequenceOutputStream( StreamDataSequence& rData ) :
    BinaryStreamBase( true ),
    SequenceSeekableStream( rData )
{
}

void SequenceOutputStream::writeData( const StreamDataSequence& rData, size_t nAtomSize )
{
    if( mpData && rData.hasElements() )
        writeMemory( rData.getConstArray(), rData.getLength(), nAtomSize );
}

void SequenceOutputStream::writeMemory( const void* pMem, sal_Int32 nBytes, size_t /*nAtomSize*/ )
{
    if( mpData && (nBytes > 0) )
    {
        if( mpData->getLength() - mnPos < nBytes )
            const_cast< StreamDataSequence* >( mpData )->realloc( mnPos + nBytes );
        memcpy( const_cast< StreamDataSequence* >( mpData )->getArray() + mnPos, pMem, static_cast< size_t >( nBytes ) );
        mnPos += nBytes;
    }
}

// ============================================================================

} // namespace oox

