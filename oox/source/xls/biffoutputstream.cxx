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

#include "oox/xls/biffoutputstream.hxx"

namespace oox {
namespace xls {

// ============================================================================

namespace prv {

BiffOutputRecordBuffer::BiffOutputRecordBuffer( BinaryOutputStream& rOutStrm, sal_uInt16 nMaxRecSize ) :
    mrOutStrm( rOutStrm ),
    mnMaxRecSize( nMaxRecSize ),
    mnRecId( BIFF_ID_UNKNOWN ),
    mbInRec( false )
{
    OSL_ENSURE( mrOutStrm.isSeekable(), "BiffOutputRecordBuffer::BiffOutputRecordBuffer - stream must be seekable" );
    maData.reserve( SAL_MAX_UINT16 );
}

void BiffOutputRecordBuffer::startRecord( sal_uInt16 nRecId )
{
    OSL_ENSURE( !mbInRec, "BiffOutputRecordBuffer::startRecord - another record still open" );
    mnRecId = nRecId;
    maData.clear();
    mbInRec = true;
}

void BiffOutputRecordBuffer::endRecord()
{
    OSL_ENSURE( mbInRec, "BiffOutputRecordBuffer::endRecord - no record open" );
    sal_uInt16 nRecSize = getLimitedValue< sal_uInt16, size_t >( maData.size(), 0, SAL_MAX_UINT16 );
    mrOutStrm.seekToEnd();
    mrOutStrm << mnRecId << nRecSize;
    if( nRecSize > 0 )
        mrOutStrm.writeMemory( &maData.front(), nRecSize );
    mbInRec = false;
}

void BiffOutputRecordBuffer::write( const void* pData, sal_uInt16 nBytes )
{
    OSL_ENSURE( mbInRec, "BiffOutputRecordBuffer::write - no record open" );
    OSL_ENSURE( nBytes > 0, "BiffOutputRecordBuffer::write - nothing to write" );
    OSL_ENSURE( nBytes <= getRecLeft(), "BiffOutputRecordBuffer::write - buffer overflow" );
    maData.resize( maData.size() + nBytes );
    memcpy( &*(maData.end() - nBytes), pData, nBytes );
}

void BiffOutputRecordBuffer::fill( sal_uInt8 nValue, sal_uInt16 nBytes )
{
    OSL_ENSURE( mbInRec, "BiffOutputRecordBuffer::write - no record open" );
    OSL_ENSURE( nBytes > 0, "BiffOutputRecordBuffer::write - nothing to write" );
    OSL_ENSURE( nBytes <= getRecLeft(), "BiffOutputRecordBuffer::write - buffer overflow" );
    maData.resize( maData.size() + nBytes, nValue );
}

} // namespace prv

// ============================================================================

BiffOutputStream::BiffOutputStream( BinaryOutputStream& rOutStream, sal_uInt16 nMaxRecSize ) :
    BinaryStreamBase( true ),
    maRecBuffer( rOutStream, nMaxRecSize ),
    mnPortionSize( 0 ),
    mnPortionPos( 0 )
{
}

// record control -------------------------------------------------------------

void BiffOutputStream::startRecord( sal_uInt16 nRecId )
{
    maRecBuffer.startRecord( nRecId );
    setPortionSize( 1 );
}

void BiffOutputStream::endRecord()
{
    setPortionSize( 1 );
    maRecBuffer.endRecord();
}

void BiffOutputStream::setPortionSize( sal_uInt8 nSize )
{
    OSL_ENSURE( mnPortionPos == 0, "BiffOutputStream::setPortionSize - block operation inside portion" );
    mnPortionSize = ::std::max< sal_uInt8 >( nSize, 1 );
    mnPortionPos = 0;
}

// BinaryStreamBase interface (seeking) ---------------------------------------

sal_Int64 BiffOutputStream::tellBase() const
{
    return maRecBuffer.getBaseStream().tell();
}

sal_Int64 BiffOutputStream::sizeBase() const
{
    return maRecBuffer.getBaseStream().size();
}

// BinaryOutputStream interface (stream write access) -------------------------

void BiffOutputStream::writeData( const StreamDataSequence& rData, size_t nAtomSize )
{
    if( rData.hasElements() )
        writeMemory( rData.getConstArray(), rData.getLength(), nAtomSize );
}

void BiffOutputStream::writeMemory( const void* pMem, sal_Int32 nBytes, size_t nAtomSize )
{
    if( pMem && (nBytes > 0) )
    {
        const sal_uInt8* pnBuffer = reinterpret_cast< const sal_uInt8* >( pMem );
        sal_Int32 nBytesLeft = nBytes;
        while( nBytesLeft > 0 )
        {
            sal_uInt16 nBlockSize = prepareWriteBlock( nBytesLeft, nAtomSize );
            maRecBuffer.write( pnBuffer, nBlockSize );
            pnBuffer += nBlockSize;
            nBytesLeft -= nBlockSize;
        }
    }
}

void BiffOutputStream::fill( sal_uInt8 nValue, sal_Int32 nBytes, size_t nAtomSize )
{
    sal_Int32 nBytesLeft = nBytes;
    while( nBytesLeft > 0 )
    {
        sal_uInt16 nBlockSize = prepareWriteBlock( nBytesLeft, nAtomSize );
        maRecBuffer.fill( nValue, nBlockSize );
        nBytesLeft -= nBlockSize;
    }
}

// private --------------------------------------------------------------------

sal_uInt16 BiffOutputStream::prepareWriteBlock( sal_Int32 nTotalSize, size_t nAtomSize )
{
    sal_uInt16 nRecLeft = maRecBuffer.getRecLeft();
    if( mnPortionSize <= 1 )
    {
        // no portions: restrict remaining record size to entire atoms
        nRecLeft = static_cast< sal_uInt16 >( (nRecLeft / nAtomSize) * nAtomSize );
    }
    else
    {
        sal_Int32 nPortionLeft = mnPortionSize - mnPortionPos;
        if( nTotalSize <= nPortionLeft )
        {
            // block fits into the current portion
            OSL_ENSURE( nPortionLeft <= nRecLeft, "BiffOutputStream::prepareWriteBlock - portion exceeds record" );
            mnPortionPos = static_cast< sal_uInt8 >( (mnPortionPos + nTotalSize) % mnPortionSize );
        }
        else
        {
            // restrict remaining record size to entire portions
            OSL_ENSURE( mnPortionPos == 0, "BiffOutputStream::prepareWriteBlock - writing over multiple portions starts inside portion" );
            mnPortionPos = 0;
            // check that atom size matches portion size
            OSL_ENSURE( mnPortionSize % nAtomSize == 0, "BiffOutputStream::prepareWriteBlock - atom size does not match portion size" );
            sal_uInt8 nPortionSize = static_cast< sal_uInt8 >( (mnPortionSize / nAtomSize) * nAtomSize );
            // restrict remaining record size to entire portions
            nRecLeft = (nRecLeft / nPortionSize) * nPortionSize;
        }
    }

    // current record has space for some data: return size of available space
    if( nRecLeft > 0 )
        return getLimitedValue< sal_uInt16, sal_Int32 >( nTotalSize, 0, nRecLeft );

    // finish current record and start a new CONTINUE record
    maRecBuffer.endRecord();
    maRecBuffer.startRecord( BIFF_ID_CONT );
    mnPortionPos = 0;
    return prepareWriteBlock( nTotalSize, nAtomSize );
}

// ============================================================================

} // namespace xls
} // namespace oox
