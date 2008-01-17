/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: biffoutputstream.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "oox/xls/biffoutputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"

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
        mrOutStrm.write( &maData.front(), nRecSize );
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

BiffOutputStream::~BiffOutputStream()
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
    maRecBuffer.endRecord();
}

void BiffOutputStream::setPortionSize( sal_uInt16 nSize )
{
    mnPortionSize = nSize;
    mnPortionPos = 0;
}

// stream/record state and info -----------------------------------------------

// stream write access --------------------------------------------------------

void BiffOutputStream::write( const void* pData, sal_uInt32 nBytes )
{
    if( pData && (nBytes > 0) )
    {
        const sal_uInt8* pnBuffer = reinterpret_cast< const sal_uInt8* >( pData );
        sal_uInt32 nBytesLeft = nBytes;
        while( nBytesLeft > 0 )
        {
            sal_uInt16 nBlockSize = prepareRawBlock( nBytesLeft );
            maRecBuffer.write( pnBuffer, nBlockSize );
            pnBuffer += nBlockSize;
            nBytesLeft -= nBlockSize;
        }
    }
}

void BiffOutputStream::fill( sal_uInt8 nValue, sal_uInt32 nBytes )
{
    sal_uInt32 nBytesLeft = nBytes;
    while( nBytesLeft > 0 )
    {
        sal_uInt16 nBlockSize = prepareRawBlock( nBytesLeft );
        maRecBuffer.fill( nValue, nBlockSize );
        nBytesLeft -= nBlockSize;
    }
}

// private --------------------------------------------------------------------

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
        OSL_ENSURE( mnPortionPos + nSize <= mnPortionSize, "BiffOutputStreamI::ensureRawBlock - portion overflow" );
        mnPortionPos = (mnPortionPos + nSize) % mnPortionSize;  // prevent compiler warning, do not use operator+=, operator%=
    }
}

sal_uInt16 BiffOutputStream::prepareRawBlock( sal_uInt32 nTotalSize )
{
    sal_uInt16 nRecLeft = maRecBuffer.getRecLeft();
    if( mnPortionSize > 0 )
    {
        OSL_ENSURE( mnPortionPos == 0, "BiffOutputStream::prepareRawBlock - block operation inside portion" );
        OSL_ENSURE( nTotalSize % mnPortionSize == 0, "BiffOutputStream::prepareRawBlock - portion size does not match block size" );
        nRecLeft = (nRecLeft / mnPortionSize) * mnPortionSize;
    }
    sal_uInt16 nSize = getLimitedValue< sal_uInt16, sal_uInt32 >( nTotalSize, 0, nRecLeft );
    ensureRawBlock( nSize );
    return nSize;
}
// ============================================================================

} // namespace xls
} // namespace oox

