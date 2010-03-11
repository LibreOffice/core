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
    maRecBuffer( rOutStream, nMaxRecSize ),
    mnPortionSize( 0 ),
    mnPortionPos( 0 )
{
}

// record control -------------------------------------------------------------

void BiffOutputStream::startRecord( sal_uInt16 nRecId )
{
    maRecBuffer.startRecord( nRecId );
    setPortionSize( 0 );
}

void BiffOutputStream::endRecord()
{
    setPortionSize( 0 );
    maRecBuffer.endRecord();
}

void BiffOutputStream::setPortionSize( sal_uInt16 nSize )
{
    OSL_ENSURE( mnPortionPos == 0, "BiffOutputStream::setPortionSize - block operation inside portion" );
    mnPortionSize = nSize;
    mnPortionPos = 0;
}

// BinaryStreamBase interface (seeking) ---------------------------------------

sal_Int64 BiffOutputStream::tellBase() const
{
    return maRecBuffer.getBaseStream().tell();
}

sal_Int64 BiffOutputStream::getBaseLength() const
{
    return maRecBuffer.getBaseStream().getLength();
}

// BinaryOutputStream interface (stream write access) -------------------------

void BiffOutputStream::writeData( const StreamDataSequence& rData )
{
    if( rData.hasElements() )
        writeMemory( rData.getConstArray(), rData.getLength() );
}

void BiffOutputStream::writeMemory( const void* pMem, sal_Int32 nBytes )
{
    if( pMem && (nBytes > 0) )
    {
        const sal_uInt8* pnBuffer = reinterpret_cast< const sal_uInt8* >( pMem );
        sal_Int32 nBytesLeft = nBytes;
        while( nBytesLeft > 0 )
        {
            sal_uInt16 nBlockSize = prepareRawBlock( nBytesLeft );
            maRecBuffer.write( pnBuffer, nBlockSize );
            pnBuffer += nBlockSize;
            nBytesLeft -= nBlockSize;
        }
    }
}

void BiffOutputStream::fill( sal_uInt8 nValue, sal_Int32 nBytes )
{
    sal_Int32 nBytesLeft = nBytes;
    while( nBytesLeft > 0 )
    {
        sal_uInt16 nBlockSize = prepareRawBlock( nBytesLeft );
        maRecBuffer.fill( nValue, nBlockSize );
        nBytesLeft -= nBlockSize;
    }
}

void BiffOutputStream::writeBlock( const void* pMem, sal_uInt16 nBytes )
{
    ensureRawBlock( nBytes );
    maRecBuffer.write( pMem, nBytes );
}

// private --------------------------------------------------------------------

void BiffOutputStream::writeAtom( const void* pMem, sal_uInt8 nSize )
{
    // byte swapping is done in calling BinaryOutputStream::writeValue() template function
    writeBlock( pMem, nSize );
}

void BiffOutputStream::ensureRawBlock( sal_uInt16 nSize )
{
    if( (maRecBuffer.getRecLeft() < nSize) ||
        ((mnPortionSize > 0) && (mnPortionPos == 0) && (maRecBuffer.getRecLeft() < mnPortionSize)) )
    {
        maRecBuffer.endRecord();
        maRecBuffer.startRecord( BIFF_ID_CONT );
    }
    if( mnPortionSize > 0 )
    {
        OSL_ENSURE( mnPortionPos + nSize <= mnPortionSize, "BiffOutputStream::ensureRawBlock - portion overflow" );
        mnPortionPos = (mnPortionPos + nSize) % mnPortionSize;  // prevent compiler warning, do not use operator+=, operator%=
    }
}

sal_uInt16 BiffOutputStream::prepareRawBlock( sal_Int32 nTotalSize )
{
    sal_uInt16 nRecLeft = maRecBuffer.getRecLeft();
    if( mnPortionSize > 0 )
    {
        OSL_ENSURE( mnPortionPos == 0, "BiffOutputStream::prepareRawBlock - block operation inside portion" );
        OSL_ENSURE( nTotalSize % mnPortionSize == 0, "BiffOutputStream::prepareRawBlock - portion size does not match block size" );
        nRecLeft = (nRecLeft / mnPortionSize) * mnPortionSize;
    }
    sal_uInt16 nSize = getLimitedValue< sal_uInt16, sal_Int32 >( nTotalSize, 0, nRecLeft );
    ensureRawBlock( nSize );
    return nSize;
}
// ============================================================================

} // namespace xls
} // namespace oox

