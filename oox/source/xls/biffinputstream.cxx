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

#include "oox/xls/biffinputstream.hxx"

#include <algorithm>
#include <rtl/ustrbuf.hxx>

namespace oox {
namespace xls {

// ============================================================================

using ::rtl::OString;
using ::rtl::OStringToOUString;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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

void BiffInputRecordBuffer::setDecoder( const BiffDecoderRef& rxDecoder )
{
    mxDecoder = rxDecoder;
    enableDecoder( true );
    updateDecoded();
}

void BiffInputRecordBuffer::enableDecoder( bool bEnable )
{
    mpCurrentData = (bEnable && mxDecoder.get() && mxDecoder->isValid()) ? &maDecodedData : &maOriginalData;
}

bool BiffInputRecordBuffer::startRecord( sal_Int64 nHeaderPos )
{
    mbValidHeader = (0 <= nHeaderPos) && (nHeaderPos + 4 <= mrInStrm.size());
    if( mbValidHeader )
    {
        mnHeaderPos = nHeaderPos;
        mrInStrm.seek( nHeaderPos );
        mrInStrm >> mnRecId >> mnRecSize;
        mnBodyPos = mrInStrm.tell();
        mnNextHeaderPos = mnBodyPos + mnRecSize;
        mbValidHeader = !mrInStrm.isEof() && (mnNextHeaderPos <= mrInStrm.size());
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
    if( mbValidHeader && (mnNextHeaderPos + 4 <= mrInStrm.size()) )
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

void BiffInputRecordBuffer::skip( sal_uInt16 nBytes )
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
            mrInStrm.readMemory( &maOriginalData.front(), static_cast< sal_Int32 >( mnRecSize ) );
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
    BinaryStreamBase( true ),
    maRecBuffer( rInStream ),
    mnRecHandle( -1 ),
    mnRecId( BIFF_ID_UNKNOWN ),
    mnAltContId( BIFF_ID_UNKNOWN ),
    mnCurrRecSize( 0 ),
    mnComplRecSize( 0 ),
    mbHasComplRec( false ),
    mbCont( bContLookup )
{
    mbEof = true;   // EOF will be true if stream is not inside a record
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

void BiffInputStream::setDecoder( const BiffDecoderRef& rxDecoder )
{
    maRecBuffer.setDecoder( rxDecoder );
}

void BiffInputStream::enableDecoder( bool bEnable )
{
    maRecBuffer.enableDecoder( bEnable );
}

// stream/record state and info -----------------------------------------------

sal_uInt16 BiffInputStream::getNextRecId()
{
    sal_uInt16 nRecId = BIFF_ID_UNKNOWN;
    if( isInRecord() )
    {
        sal_Int64 nCurrPos = tell();            // save current position in record
        while( jumpToNextContinue() ) {}        // skip following CONTINUE records
        if( maRecBuffer.startNextRecord() )     // read header of next record
            nRecId = maRecBuffer.getRecId();
        seek( nCurrPos );                       // restore position, seek() resets old mbValid state
    }
    return nRecId;
}

// BinaryStreamBase interface (seeking) ---------------------------------------

sal_Int64 BiffInputStream::size() const
{
    if( !mbHasComplRec )
        const_cast< BiffInputStream* >( this )->calcRecordLength();
    return mnComplRecSize;
}

sal_Int64 BiffInputStream::tell() const
{
    return mbEof ? -1 : (mnCurrRecSize - maRecBuffer.getRecLeft());
}

void BiffInputStream::seek( sal_Int64 nRecPos )
{
    if( isInRecord() )
    {
        if( mbEof || (nRecPos < tell()) )
            restartRecord( false );
        if( !mbEof && (nRecPos > tell()) )
            skip( static_cast< sal_Int32 >( nRecPos - tell() ) );
    }
}

void BiffInputStream::close()
{
}

sal_Int64 BiffInputStream::tellBase() const
{
    return maRecBuffer.getBaseStream().tell();
}

sal_Int64 BiffInputStream::sizeBase() const
{
    return maRecBuffer.getBaseStream().size();
}

// BinaryInputStream interface (stream read access) ---------------------------

sal_Int32 BiffInputStream::readData( StreamDataSequence& orData, sal_Int32 nBytes, size_t nAtomSize )
{
    sal_Int32 nRet = 0;
    if( !mbEof )
    {
        orData.realloc( ::std::max< sal_Int32 >( nBytes, 0 ) );
        if( nBytes > 0 )
            nRet = readMemory( orData.getArray(), nBytes, nAtomSize );
    }
    return nRet;
}

sal_Int32 BiffInputStream::readMemory( void* opMem, sal_Int32 nBytes, size_t nAtomSize )
{
    sal_Int32 nRet = 0;
    if( !mbEof && opMem && (nBytes > 0) )
    {
        sal_uInt8* pnBuffer = reinterpret_cast< sal_uInt8* >( opMem );
        sal_Int32 nBytesLeft = nBytes;

        while( !mbEof && (nBytesLeft > 0) )
        {
            sal_uInt16 nReadSize = getMaxRawReadSize( nBytesLeft, nAtomSize );
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
            OSL_ENSURE( !mbEof, "BiffInputStream::readMemory - record overread" );
        }
    }
    return nRet;
}

void BiffInputStream::skip( sal_Int32 nBytes, size_t nAtomSize )
{
    sal_Int32 nBytesLeft = nBytes;
    while( !mbEof && (nBytesLeft > 0) )
    {
        sal_uInt16 nSkipSize = getMaxRawReadSize( nBytesLeft, nAtomSize );
        // check nSkipSize, stream may already be located at end of a raw record
        if( nSkipSize > 0 )
        {
            maRecBuffer.skip( nSkipSize );
            nBytesLeft -= nSkipSize;
        }
        if( nBytesLeft > 0 )
            jumpToNextContinue();
        OSL_ENSURE( !mbEof, "BiffInputStream::skip - record overread" );
    }
}

// byte strings ---------------------------------------------------------------

OString BiffInputStream::readByteString( bool b16BitLen, bool bAllowNulChars )
{
    sal_Int32 nStrLen = b16BitLen ? readuInt16() : readuInt8();
    return readCharArray( nStrLen, bAllowNulChars );
}

OUString BiffInputStream::readByteStringUC( bool b16BitLen, rtl_TextEncoding eTextEnc, bool bAllowNulChars )
{
    return OStringToOUString( readByteString( b16BitLen, bAllowNulChars ), eTextEnc );
}

void BiffInputStream::skipByteString( bool b16BitLen )
{
    skip( b16BitLen ? readuInt16() : readuInt8() );
}

// Unicode strings ------------------------------------------------------------

OUString BiffInputStream::readUniStringChars( sal_uInt16 nChars, bool b16BitChars, bool bAllowNulChars )
{
    OUStringBuffer aBuffer;
    aBuffer.ensureCapacity( nChars );

    /*  This function has to react on CONTINUE records which repeat the flags
        field in their first byte and may change the 8bit/16bit character mode,
        thus a plain call to readCompressedUnicodeArray() cannot be used here. */
    sal_Int32 nCharsLeft = nChars;
    while( !mbEof && (nCharsLeft > 0) )
    {
        /*  Read the character array from the remaining part of the current raw
            record. First, calculate the maximum number of characters that can
            be read without triggering to start a following CONTINUE record. */
        sal_Int32 nRawChars = b16BitChars ? (getMaxRawReadSize( nCharsLeft * 2, 2 ) / 2) : getMaxRawReadSize( nCharsLeft, 1 );
        aBuffer.append( readCompressedUnicodeArray( nRawChars, !b16BitChars, bAllowNulChars ) );

        /*  Prepare for next CONTINUE record. Calling jumpToNextStringContinue()
            reads the leading byte in the following CONTINUE record and updates
            the b16BitChars flag. */
        nCharsLeft -= nRawChars;
        if( nCharsLeft > 0 )
            jumpToNextStringContinue( b16BitChars );
    }

    return aBuffer.makeStringAndClear();
}

OUString BiffInputStream::readUniStringBody( sal_uInt16 nChars, bool bAllowNulChars )
{
    bool b16BitChars;
    sal_Int32 nAddSize;
    readUniStringHeader( b16BitChars, nAddSize );
    OUString aString = readUniStringChars( nChars, b16BitChars, bAllowNulChars );
    skip( nAddSize );
    return aString;
}

OUString BiffInputStream::readUniString( bool bAllowNulChars )
{
    return readUniStringBody( readuInt16(), bAllowNulChars );
}

void BiffInputStream::skipUniStringChars( sal_uInt16 nChars, bool b16BitChars )
{
    sal_Int32 nCharsLeft = nChars;
    while( !mbEof && (nCharsLeft > 0) )
    {
        // skip the character array
        sal_Int32 nSkipSize = b16BitChars ? getMaxRawReadSize( 2 * nCharsLeft, 2 ) : getMaxRawReadSize( nCharsLeft, 1 );
        skip( nSkipSize );

        // prepare for next CONTINUE record
        nCharsLeft -= (b16BitChars ? (nSkipSize / 2) : nSkipSize);
        if( nCharsLeft > 0 )
            jumpToNextStringContinue( b16BitChars );
    }
}

void BiffInputStream::skipUniStringBody( sal_uInt16 nChars )
{
    bool b16BitChars;
    sal_Int32 nAddSize;
    readUniStringHeader( b16BitChars, nAddSize );
    skipUniStringChars( nChars, b16BitChars );
    skip( nAddSize );
}

void BiffInputStream::skipUniString()
{
    skipUniStringBody( readuInt16() );
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
    mbEof = !isInRecord();
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
        mbEof = false;
    }
}

void BiffInputStream::rewindToRecord( sal_Int64 nRecHandle )
{
    if( nRecHandle >= 0 )
    {
        maRecBuffer.restartAt( nRecHandle );
        mnRecHandle = -1;
        mbEof = true;   // as long as the record is not started
    }
}

bool BiffInputStream::isContinueId( sal_uInt16 nRecId ) const
{
    return (nRecId == BIFF_ID_CONT) || (nRecId == mnAltContId);
}

bool BiffInputStream::jumpToNextContinue()
{
    mbEof = mbEof || !mbCont || !isContinueId( maRecBuffer.getNextRecId() ) || !maRecBuffer.startNextRecord();
    if( !mbEof )
        mnCurrRecSize += maRecBuffer.getRecSize();
    return !mbEof;
}

bool BiffInputStream::jumpToNextStringContinue( bool& rb16BitChars )
{
    OSL_ENSURE( maRecBuffer.getRecLeft() == 0, "BiffInputStream::jumpToNextStringContinue - alignment error" );

    if( mbCont && (getRemaining() > 0) )
    {
        jumpToNextContinue();
    }
    else if( mnRecId == BIFF_ID_CONT )
    {
        /*  CONTINUE handling is off, but we have started reading in a CONTINUE
            record -> start next CONTINUE for TXO import. We really start a new
            record here - no chance to return to string origin. */
        mbEof = mbEof || (maRecBuffer.getNextRecId() != BIFF_ID_CONT) || !maRecBuffer.startNextRecord();
        if( !mbEof )
            setupRecord();
    }

    // trying to read the flags invalidates stream, if no CONTINUE record has been found
    sal_uInt8 nFlags;
    readValue( nFlags );
    rb16BitChars = getFlag( nFlags, BIFF_STRF_16BIT );
    return !mbEof;
}

void BiffInputStream::calcRecordLength()
{
    sal_Int64 nCurrPos = tell();            // save current position in record
    while( jumpToNextContinue() ) {}        // jumpToNextContinue() adds up mnCurrRecSize
    mnComplRecSize = mnCurrRecSize;
    mbHasComplRec = true;
    seek( nCurrPos );                       // restore position, seek() resets old mbValid state
}

sal_uInt16 BiffInputStream::getMaxRawReadSize( sal_Int32 nBytes, size_t nAtomSize ) const
{
    sal_uInt16 nMaxSize = getLimitedValue< sal_uInt16, sal_Int32 >( nBytes, 0, maRecBuffer.getRecLeft() );
    if( (0 < nMaxSize) && (nMaxSize < nBytes) && (nAtomSize > 1) )
    {
        // check that remaining data in record buffer is a multiple of the passed atom size
        sal_uInt16 nPadding = static_cast< sal_uInt16 >( nMaxSize % nAtomSize );
        OSL_ENSURE( nPadding == 0, "BiffInputStream::getMaxRawReadSize - alignment error" );
        nMaxSize = nMaxSize - nPadding;
    }
    return nMaxSize;
}

void BiffInputStream::readUniStringHeader( bool& orb16BitChars, sal_Int32& ornAddSize )
{
    sal_uInt8 nFlags = readuInt8();
    OSL_ENSURE( !getFlag( nFlags, BIFF_STRF_UNKNOWN ), "BiffInputStream::readUniStringHeader - unknown flags" );
    orb16BitChars = getFlag( nFlags, BIFF_STRF_16BIT );
    sal_uInt16 nFontCount = getFlag( nFlags, BIFF_STRF_RICH ) ? readuInt16() : 0;
    sal_Int32 nPhoneticSize = getFlag( nFlags, BIFF_STRF_PHONETIC ) ? readInt32() : 0;
    ornAddSize = 4 * nFontCount + ::std::max< sal_Int32 >( 0, nPhoneticSize );
}

// ============================================================================

BiffInputStreamPos::BiffInputStreamPos( BiffInputStream& rStrm ) :
    mrStrm( rStrm ),
    mnRecHandle( rStrm.getRecHandle() ),
    mnRecPos( rStrm.tell() )
{
}

bool BiffInputStreamPos::restorePosition()
{
    bool bValidRec = mrStrm.startRecordByHandle( mnRecHandle );
    if( bValidRec )
        mrStrm.seek( mnRecPos );
    return bValidRec && !mrStrm.isEof();
}

// ============================================================================

BiffInputStreamPosGuard::BiffInputStreamPosGuard( BiffInputStream& rStrm ) :
    BiffInputStreamPos( rStrm )
{
}

BiffInputStreamPosGuard::~BiffInputStreamPosGuard()
{
    restorePosition();
}

// ============================================================================

} // namespace xls
} // namespace oox
