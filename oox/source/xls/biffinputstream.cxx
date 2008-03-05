/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: biffinputstream.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:57:06 $
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

#include "oox/xls/biffinputstream.hxx"
#include <algorithm>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/binaryinputstream.hxx"

using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace oox {
namespace xls {

// ============================================================================

namespace prv {

BiffInputRecordBuffer::BiffInputRecordBuffer( BinaryInputStream& rInStrm ) :
    mrInStrm( rInStrm ),
    mpCurrentData( 0 ),
    mnHeaderPos( -1 ),
    mnBodyPos( 0 ),
    mnBufferBodyPos( 0 ),
    mnNextHeaderPos( 0 ),
    mnRecId( BIFF_ID_UNKNOWN ),
    mnRecSize( 0 ),
    mnRecPos( 0 ),
    mbValidHeader( false )
{
    OSL_ENSURE( mrInStrm.isSeekable(), "BiffInputRecordBuffer::BiffInputRecordBuffer - stream must be seekable" );
    mrInStrm.seekToStart();
    maOriginalData.reserve( SAL_MAX_UINT16 );
    maDecodedData.reserve( SAL_MAX_UINT16 );
    enableDecoder( false );     // updates mpCurrentData
}

void BiffInputRecordBuffer::restartAt( sal_Int64 nPos )
{
    mnHeaderPos = -1;
    mnBodyPos = mnBufferBodyPos = 0;
    mnNextHeaderPos = nPos;
    mnRecId = BIFF_ID_UNKNOWN;
    mnRecSize = mnRecPos = 0;
    mbValidHeader = false;
}

void BiffInputRecordBuffer::setDecoder( BiffDecoderRef xDecoder )
{
    mxDecoder = xDecoder;
    enableDecoder( true );
    updateDecoded();
}

void BiffInputRecordBuffer::enableDecoder( bool bEnable )
{
    mpCurrentData = (bEnable && mxDecoder.get() && mxDecoder->isValid()) ? &maDecodedData : &maOriginalData;
}

bool BiffInputRecordBuffer::startRecord( sal_Int64 nHeaderPos )
{
    mbValidHeader = (0 <= nHeaderPos) && (nHeaderPos + 4 <= mrInStrm.getLength());
    if( mbValidHeader )
    {
        mnHeaderPos = nHeaderPos;
        mrInStrm.seek( nHeaderPos );
        mrInStrm >> mnRecId >> mnRecSize;
        mnBodyPos = mrInStrm.tell();
        mnNextHeaderPos = mnBodyPos + mnRecSize;
        mbValidHeader = mnNextHeaderPos <= mrInStrm.getLength();
    }
    if( !mbValidHeader )
    {
        mnHeaderPos = mnBodyPos = -1;
        mnNextHeaderPos = 0;
        mnRecId = BIFF_ID_UNKNOWN;
        mnRecSize = 0;
    }
    mnRecPos = 0;
    return mbValidHeader;
}

bool BiffInputRecordBuffer::startNextRecord()
{
    return startRecord( mnNextHeaderPos );
}

sal_uInt16 BiffInputRecordBuffer::getNextRecId()
{
    sal_uInt16 nRecId = BIFF_ID_UNKNOWN;
    if( mbValidHeader && (mnNextHeaderPos + 4 <= mrInStrm.getLength()) )
    {
        mrInStrm.seek( mnNextHeaderPos );
        mrInStrm >> nRecId;
    }
    return nRecId;
}

void BiffInputRecordBuffer::read( void* opData, sal_uInt16 nBytes )
{
    updateBuffer();
    OSL_ENSURE( nBytes > 0, "BiffInputRecordBuffer::read - nothing to read" );
    OSL_ENSURE( nBytes <= getRecLeft(), "BiffInputRecordBuffer::read - buffer overflow" );
    memcpy( opData, &(*mpCurrentData)[ mnRecPos ], nBytes );
    mnRecPos = mnRecPos + nBytes;
}

inline void BiffInputRecordBuffer::skip( sal_uInt16 nBytes )
{
    OSL_ENSURE( nBytes > 0, "BiffInputRecordBuffer::skip - nothing to skip" );
    OSL_ENSURE( nBytes <= getRecLeft(), "BiffInputRecordBuffer::skip - buffer overflow" );
    mnRecPos = mnRecPos + nBytes;
}

void BiffInputRecordBuffer::updateBuffer()
{
    OSL_ENSURE( mbValidHeader, "BiffInputRecordBuffer::updateBuffer - invalid access" );
    if( mnBodyPos != mnBufferBodyPos )
    {
        mrInStrm.seek( mnBodyPos );
        maOriginalData.resize( mnRecSize );
        if( mnRecSize > 0 )
            mrInStrm.read( &maOriginalData.front(), static_cast< sal_Int32 >( mnRecSize ) );
        mnBufferBodyPos = mnBodyPos;
        updateDecoded();
    }
}

void BiffInputRecordBuffer::updateDecoded()
{
    if( mxDecoder.get() && mxDecoder->isValid() )
    {
        maDecodedData.resize( mnRecSize );
        if( mnRecSize > 0 )
            mxDecoder->decode( &maDecodedData.front(), &maOriginalData.front(), mnBodyPos, mnRecSize );
    }
}

} // namespace prv

// ============================================================================

BiffInputStream::BiffInputStream( BinaryInputStream& rInStream, bool bContLookup ) :
    maRecBuffer( rInStream ),
    mnRecHandle( -1 ),
    mnRecId( BIFF_ID_UNKNOWN ),
    mnAltContId( BIFF_ID_UNKNOWN ),
    mnCurrRecSize( 0 ),
    mnComplRecSize( 0 ),
    mbHasComplRec( false ),
    mbCont( bContLookup ),
    mbNulChars( false ),
    mbValid( false )
{
}

BiffInputStream::~BiffInputStream()
{
}

// record control -------------------------------------------------------------

bool BiffInputStream::startNextRecord()
{
    bool bValidRec = false;
    /*  #i4266# ignore zero records (id==len==0) (e.g. the application
        "Crystal Report" writes zero records between other records) */
    bool bIsZeroRec = false;
    do
    {
        // record header is never encrypted
        maRecBuffer.enableDecoder( false );
        // read header of next raw record, returns false at end of stream
        bValidRec = maRecBuffer.startNextRecord();
        // ignore record, if identifier and size are zero
        bIsZeroRec = (maRecBuffer.getRecId() == 0) && (maRecBuffer.getRecSize() == 0);
    }
    while( bValidRec && ((mbCont && isContinueId( maRecBuffer.getRecId() )) || bIsZeroRec) );

    // setup other class members
    setupRecord();
    return isInRecord();
}

bool BiffInputStream::startRecordByHandle( sal_Int64 nRecHandle )
{
    rewindToRecord( nRecHandle );
    return startNextRecord();
}

void BiffInputStream::resetRecord( bool bContLookup, sal_uInt16 nAltContId )
{
    if( isInRecord() )
    {
        mbCont = bContLookup;
        mnAltContId = nAltContId;
        restartRecord( true );
        maRecBuffer.enableDecoder( true );
    }
}

void BiffInputStream::rewindRecord()
{
    rewindToRecord( mnRecHandle );
}

// decoder --------------------------------------------------------------------

void BiffInputStream::setDecoder( BiffDecoderRef xDecoder )
{
    maRecBuffer.setDecoder( xDecoder );
}

BiffDecoderRef BiffInputStream::getDecoder() const
{
    return maRecBuffer.getDecoder();
}

void BiffInputStream::enableDecoder( bool bEnable )
{
    maRecBuffer.enableDecoder( bEnable );
}

// stream/record state and info -----------------------------------------------

sal_uInt32 BiffInputStream::getRecPos() const
{
    return mbValid ? (mnCurrRecSize - maRecBuffer.getRecLeft()) : BIFF_REC_SEEK_TO_END;
}

sal_uInt32 BiffInputStream::getRecSize()
{
    if( !mbHasComplRec )
    {
        sal_uInt32 nCurrPos = getRecPos();      // save current position in record
        while( jumpToNextContinue() );          // jumpToNextContinue() adds up mnCurrRecSize
        mnComplRecSize = mnCurrRecSize;
        mbHasComplRec = true;
        seek( nCurrPos );                       // restore position, seek() resets old mbValid state
    }
    return mnComplRecSize;
}

sal_uInt32 BiffInputStream::getRecLeft()
{
    return mbValid ? (getRecSize() - getRecPos()) : 0;
}

sal_uInt16 BiffInputStream::getNextRecId()
{
    sal_uInt16 nRecId = BIFF_ID_UNKNOWN;
    if( isInRecord() )
    {
        sal_uInt32 nCurrPos = getRecPos();      // save current position in record
        while( jumpToNextContinue() );          // skip following CONTINUE records
        if( maRecBuffer.startNextRecord() )     // read header of next record
            nRecId = maRecBuffer.getRecId();
        seek( nCurrPos );                       // restore position, seek() resets old mbValid state
    }
    return nRecId;
}

sal_Int64 BiffInputStream::getCoreStreamPos() const
{
    return maRecBuffer.getCoreStream().tell();
}

sal_Int64 BiffInputStream::getCoreStreamSize() const
{
    return maRecBuffer.getCoreStream().getLength();
}

// stream read access ---------------------------------------------------------

sal_uInt32 BiffInputStream::read( void* opData, sal_uInt32 nBytes )
{
    sal_uInt32 nRet = 0;
    if( mbValid && opData && (nBytes > 0) )
    {
        sal_uInt8* pnBuffer = reinterpret_cast< sal_uInt8* >( opData );
        sal_uInt32 nBytesLeft = nBytes;

        while( mbValid && (nBytesLeft > 0) )
        {
            sal_uInt16 nReadSize = getMaxRawReadSize( nBytesLeft );
            // check nReadSize, stream may already be located at end of a raw record
            if( nReadSize > 0 )
            {
                maRecBuffer.read( pnBuffer, nReadSize );
                nRet += nReadSize;
                pnBuffer += nReadSize;
                nBytesLeft -= nReadSize;
            }
            if( nBytesLeft > 0 )
                jumpToNextContinue();
            OSL_ENSURE( mbValid, "BiffInputStream::read - record overread" );
        }
    }
    return nRet;
}

// seeking --------------------------------------------------------------------

BiffInputStream& BiffInputStream::seek( sal_uInt32 nRecPos )
{
    if( isInRecord() )
    {
        if( !mbValid || (nRecPos < getRecPos()) )
            restartRecord( false );
        if( mbValid && (nRecPos > getRecPos()) )
            skip( nRecPos - getRecPos() );
    }
    return *this;
}

BiffInputStream& BiffInputStream::skip( sal_uInt32 nBytes )
{
    sal_uInt32 nBytesLeft = nBytes;
    while( mbValid && (nBytesLeft > 0) )
    {
        sal_uInt16 nSkipSize = getMaxRawReadSize( nBytesLeft );
        // check nSkipSize, stream may already be located at end of a raw record
        if( nSkipSize > 0 )
        {
            maRecBuffer.skip( nSkipSize );
            nBytesLeft -= nSkipSize;
        }
        if( nBytesLeft > 0 )
            jumpToNextContinue();
        OSL_ENSURE( mbValid, "BiffInputStream::skip - record overread" );
    }
    return *this;
}

// byte strings ---------------------------------------------------------------

OString BiffInputStream::readCharArray( sal_uInt16 nChars )
{
    if( nChars == 0 )
        return OString();

    ::std::vector< sal_Char > aBuffer( static_cast< size_t >( nChars ) );
    size_t nCharsRead = static_cast< size_t >( read( &aBuffer.front(), nChars ) );
    aBuffer.resize( nCharsRead );
    if( !mbNulChars )
        ::std::replace( aBuffer.begin(), aBuffer.end(), '\0', '?' );
    return OString( &aBuffer.front(), nCharsRead );
}

OUString BiffInputStream::readCharArray( sal_uInt16 nChars, rtl_TextEncoding eTextEnc )
{
    return OStringToOUString( readCharArray( nChars ), eTextEnc );
}

OString BiffInputStream::readByteString( bool b16BitLen )
{
    sal_uInt16 nStrLen = b16BitLen ? readuInt16() : readuInt8();
    return readCharArray( nStrLen );
}

OUString BiffInputStream::readByteString( bool b16BitLen, rtl_TextEncoding eTextEnc )
{
    return OStringToOUString( readByteString( b16BitLen ), eTextEnc );
}

void BiffInputStream::skipByteString( bool b16BitLen )
{
    skip( b16BitLen ? readuInt16() : readuInt8() );
}

// Unicode strings ------------------------------------------------------------

OUString BiffInputStream::readUnicodeArray( sal_uInt16 nChars )
{
    OUStringBuffer aBuffer;
    appendUnicodeArray( aBuffer, nChars, true );
    return aBuffer.makeStringAndClear();
}

OUString BiffInputStream::readUniStringChars( sal_uInt16 nChars, bool b16Bit )
{
    OUStringBuffer aBuffer;
    aBuffer.ensureCapacity( nChars );

    /*  This function has to react on CONTINUE records to read the repeated
        flags field, so readUnicodeArray() cannot be used here. */
    sal_uInt16 nCharsLeft = nChars;
    while( mbValid && (nCharsLeft > 0) )
    {
        sal_uInt16 nPortionCount = 0;
        if( b16Bit )
        {
            nPortionCount = ::std::min< sal_uInt16 >( nCharsLeft, maRecBuffer.getRecLeft() / 2 );
            OSL_ENSURE( (nPortionCount <= nCharsLeft) || ((maRecBuffer.getRecLeft() & 1) == 0),
                "BiffInputStream::readUniStringChars - missing a byte" );
        }
        else
        {
            nPortionCount = getMaxRawReadSize( nCharsLeft );
        }

        // read the character array
        appendUnicodeArray( aBuffer, nPortionCount, b16Bit );

        // prepare for next CONTINUE record
        nCharsLeft = nCharsLeft - nPortionCount;
        if( nCharsLeft > 0 )
            jumpToNextStringContinue( b16Bit );
    }

    return aBuffer.makeStringAndClear();
}

OUString BiffInputStream::readUniString( sal_uInt16 nChars )
{
    bool b16Bit;
    sal_uInt32 nAddSize;
    readUniStringHeader( b16Bit, nAddSize );
    OUString aString = readUniStringChars( nChars, b16Bit );
    skip( nAddSize );
    return aString;
}

OUString BiffInputStream::readUniString()
{
    return readUniString( readuInt16() );
}

void BiffInputStream::skipUniStringChars( sal_uInt16 nChars, bool b16Bit )
{
    sal_uInt16 nCharsLeft = nChars;
    while( mbValid && (nCharsLeft > 0) )
    {
        sal_uInt16 nPortionCount;
        if( b16Bit )
        {
            nPortionCount = ::std::min< sal_uInt16 >( nCharsLeft, maRecBuffer.getRecLeft() / 2 );
            OSL_ENSURE( (nPortionCount <= nCharsLeft) || ((maRecBuffer.getRecLeft() & 1) == 0),
                "BiffInputStream::skipUniStringChars - missing a byte" );
            skip( 2 * nPortionCount );
        }
        else
        {
            nPortionCount = getMaxRawReadSize( nCharsLeft );
            skip( nPortionCount );
        }

        // prepare for next CONTINUE record
        nCharsLeft = nCharsLeft - nPortionCount;
        if( nCharsLeft > 0 )
            jumpToNextStringContinue( b16Bit );
    }
}

void BiffInputStream::skipUniString( sal_uInt16 nChars )
{
    bool b16Bit;
    sal_uInt32 nAddSize;
    readUniStringHeader( b16Bit, nAddSize );
    skipUniStringChars( nChars, b16Bit );
    skip( nAddSize );
}

void BiffInputStream::skipUniString()
{
    skipUniString( readuInt16() );
}

// private --------------------------------------------------------------------

void BiffInputStream::setupRecord()
{
    // initialize class members
    mnRecHandle = maRecBuffer.getRecHeaderPos();
    mnRecId = maRecBuffer.getRecId();
    mnAltContId = BIFF_ID_UNKNOWN;
    mnCurrRecSize = mnComplRecSize = maRecBuffer.getRecSize();
    mbHasComplRec = !mbCont;
    mbValid = isInRecord();
    mbNulChars = false;
    // enable decoder in new record
    enableDecoder( true );
}

void BiffInputStream::restartRecord( bool bInvalidateRecSize )
{
    if( isInRecord() )
    {
        maRecBuffer.startRecord( getRecHandle() );
        mnCurrRecSize = maRecBuffer.getRecSize();
        if( bInvalidateRecSize )
        {
            mnComplRecSize = mnCurrRecSize;
            mbHasComplRec = !mbCont;
        }
        mbValid = true;
    }
}

void BiffInputStream::rewindToRecord( sal_Int64 nRecHandle )
{
    if( nRecHandle >= 0 )
    {
        maRecBuffer.restartAt( nRecHandle );
        mnRecHandle = -1;
        mbValid = false;
    }
}

bool BiffInputStream::isContinueId( sal_uInt16 nRecId ) const
{
    return (nRecId == BIFF_ID_CONT) || (nRecId == mnAltContId);
}

bool BiffInputStream::jumpToNextContinue()
{
    mbValid = mbValid && mbCont && isContinueId( maRecBuffer.getNextRecId() ) && maRecBuffer.startNextRecord();
    if( mbValid )
        mnCurrRecSize += maRecBuffer.getRecSize();
    return mbValid;
}

bool BiffInputStream::jumpToNextStringContinue( bool& rb16Bit )
{
    OSL_ENSURE( maRecBuffer.getRecLeft() == 0, "BiffInputStream::jumpToNextStringContinue - unexpected garbage" );

    if( mbCont && (getRecLeft() > 0) )
    {
        jumpToNextContinue();
    }
    else if( mnRecId == BIFF_ID_CONT )
    {
        /*  CONTINUE handling is off, but we have started reading in a CONTINUE
            record -> start next CONTINUE for TXO import. We really start a new
            record here - no chance to return to string origin. */
        mbValid = mbValid && (maRecBuffer.getNextRecId() == BIFF_ID_CONT) && maRecBuffer.startNextRecord();
        if( mbValid )
            setupRecord();
    }

    // trying to read the flags invalidates stream, if no CONTINUE record has been found
    sal_uInt8 nFlags;
    readValue( nFlags );
    rb16Bit = getFlag( nFlags, BIFF_STRF_16BIT );
    return mbValid;
}

bool BiffInputStream::ensureRawReadSize( sal_uInt16 nBytes )
{
    if( mbValid && (nBytes > 0) )
    {
        while( mbValid && (maRecBuffer.getRecLeft() == 0) ) jumpToNextContinue();
        mbValid = mbValid && (nBytes <= maRecBuffer.getRecLeft());
        OSL_ENSURE( mbValid, "BiffInputStream::ensureRawReadSize - record overread" );
    }
    return mbValid;
}

sal_uInt16 BiffInputStream::getMaxRawReadSize( sal_uInt32 nBytes ) const
{
    return static_cast< sal_uInt16 >( ::std::min< sal_uInt32 >( nBytes, maRecBuffer.getRecLeft() ) );
}

void BiffInputStream::appendUnicodeArray( OUStringBuffer& orBuffer, sal_uInt16 nChars, bool b16Bit )
{
    orBuffer.ensureCapacity( orBuffer.getLength() + nChars );
    sal_uInt16 nChar;
    for( sal_uInt16 nCharIdx = 0; mbValid && (nCharIdx < nChars); ++nCharIdx )
    {
        if( b16Bit ) readValue( nChar ); else nChar = readuInt8();
        orBuffer.append( static_cast< sal_Unicode >( (!mbNulChars && (nChar == 0)) ? '?' : nChar ) );
    }
}

void BiffInputStream::readUniStringHeader( bool& orb16Bit, sal_uInt32& ornAddSize )
{
    sal_uInt8 nFlags = readuInt8();
    OSL_ENSURE( !getFlag( nFlags, BIFF_STRF_UNKNOWN ), "BiffInputStream::readUniStringHeader - unknown flags" );
    orb16Bit = getFlag( nFlags, BIFF_STRF_16BIT );
    sal_uInt16 nFontCount = getFlag( nFlags, BIFF_STRF_RICH ) ? readuInt16() : 0;
    sal_uInt32 nPhoneticSize = getFlag( nFlags, BIFF_STRF_PHONETIC ) ? readuInt32() : 0;
    ornAddSize = 4 * nFontCount + nPhoneticSize;
}

// ============================================================================

BiffInputStreamPos::BiffInputStreamPos( BiffInputStream& rStrm ) :
    mrStrm( rStrm ),
    mnRecHandle( rStrm.getRecHandle() ),
    mnRecPos( rStrm.getRecPos() )
{
}

bool BiffInputStreamPos::restorePosition()
{
    bool bValidRec = mrStrm.startRecordByHandle( mnRecHandle );
    if( bValidRec )
        mrStrm.seek( mnRecPos );
    return bValidRec && mrStrm.isValid();
}

// ============================================================================

BiffInputStreamGuard::BiffInputStreamGuard( BiffInputStream& rStrm ) :
    BiffInputStreamPos( rStrm )
{
}

BiffInputStreamGuard::~BiffInputStreamGuard()
{
    restorePosition();
}

// ============================================================================

} // namespace xls
} // namespace oox

