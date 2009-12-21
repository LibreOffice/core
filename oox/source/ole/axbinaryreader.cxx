/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: axbinaryreader.cxx,v $
 * $Revision: 1.1 $
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

#include "oox/ole/axbinaryreader.hxx"
#include "oox/ole/olehelper.hxx"

using ::rtl::OUString;

namespace oox {
namespace ole {

// ============================================================================

namespace {

const sal_uInt32 AX_STRING_SIZEMASK         = 0x7FFFFFFF;
const sal_uInt32 AX_STRING_COMPRESSED       = 0x80000000;

} // namespace

// ============================================================================

AxAlignedInputStream::AxAlignedInputStream( BinaryInputStream& rInStrm ) :
    mrInStrm( rInStrm ),
    mnStrmPos( 0 )
{
}

sal_Int64 AxAlignedInputStream::tell() const
{
    return mnStrmPos;
}

void AxAlignedInputStream::seek( sal_Int64 nPos )
{
    mbEof = mbEof || (nPos < mnStrmPos);
    if( !mbEof )
        skip( static_cast< sal_Int32 >( nPos - mnStrmPos ) );
}

sal_Int32 AxAlignedInputStream::readData( StreamDataSequence& orData, sal_Int32 nBytes )
{
    sal_Int32 nReadSize = mrInStrm.readData( orData, nBytes );
    mnStrmPos += nReadSize;
    return nReadSize;
}

sal_Int32 AxAlignedInputStream::readMemory( void* opMem, sal_Int32 nBytes )
{
    sal_Int32 nReadSize = mrInStrm.readMemory( opMem, nBytes );
    mnStrmPos += nReadSize;
    return nReadSize;
}

void AxAlignedInputStream::skip( sal_Int32 nBytes )
{
    mrInStrm.skip( nBytes );
    mnStrmPos += nBytes;
}

void AxAlignedInputStream::align( size_t nSize )
{
    skip( static_cast< sal_Int32 >( (nSize - (mnStrmPos % nSize)) % nSize ) );
}

// ============================================================================

AxBinaryPropertyReader::ComplexProperty::~ComplexProperty()
{
}

bool AxBinaryPropertyReader::PairProperty::readProperty( AxAlignedInputStream& rInStrm )
{
    rInStrm >> mrnValue1 >> mrnValue2;
    return true;
}

bool AxBinaryPropertyReader::StringProperty::readProperty( AxAlignedInputStream& rInStrm )
{
    bool bCompressed = getFlag( mnSize, AX_STRING_COMPRESSED );
    sal_uInt32 nBufSize = mnSize & AX_STRING_SIZEMASK;
    sal_Int64 nEndPos = rInStrm.tell() + nBufSize;
    sal_Int32 nChars = static_cast< sal_Int32 >( nBufSize / (bCompressed ? 1 : 2) );
    bool bValidChars = nChars <= 65536;
    OSL_ENSURE( bValidChars, "StringProperty::readProperty - string too long" );
    nChars = ::std::min< sal_Int32 >( nChars, 65536 );
    mrValue = bCompressed ?
        // ISO-8859-1 maps all byte values xx to the same Unicode code point U+00xx
        rInStrm.readCharArrayUC( nChars, RTL_TEXTENCODING_ISO_8859_1 ) :
        rInStrm.readUnicodeArray( nChars );
    rInStrm.seek( nEndPos );
    return bValidChars;
}

bool AxBinaryPropertyReader::PictureProperty::readProperty( AxAlignedInputStream& rInStrm )
{
    return OleHelper::importStdPic( mrPicData, rInStrm, true );
}

// ----------------------------------------------------------------------------

AxBinaryPropertyReader::AxBinaryPropertyReader( BinaryInputStream& rInStrm, bool b64BitPropFlags ) :
    maInStrm( rInStrm ),
    mbValid( true )
{
    // version and size of property block
    maInStrm.skip( 2 );
    sal_uInt16 nBlockSize = maInStrm.readValue< sal_uInt16 >();
    mnPropsEnd = maInStrm.tell() + nBlockSize;
    // flagfield containing existing properties
    if( b64BitPropFlags )
        maInStrm >> mnPropFlags;
    else
        mnPropFlags = maInStrm.readuInt32();
    mnNextProp = 1;
}

void AxBinaryPropertyReader::readBoolProperty( bool& orbValue, bool bReverse )
{
    // there is no data, the boolean value is equivalent to the property flag itself
    orbValue = startNextProperty() != bReverse;
}

void AxBinaryPropertyReader::readPairProperty( sal_Int32& ornValue1, sal_Int32& ornValue2 )
{
    if( startNextProperty() )
        maLargeProps.push_back( ComplexPropVector::value_type( new PairProperty( ornValue1, ornValue2 ) ) );
}

void AxBinaryPropertyReader::readStringProperty( OUString& orValue )
{
    if( startNextProperty() )
    {
        sal_uInt32 nSize = maInStrm.readAligned< sal_uInt32 >();
        maLargeProps.push_back( ComplexPropVector::value_type( new StringProperty( orValue, nSize ) ) );
    }
}

void AxBinaryPropertyReader::readPictureProperty( StreamDataSequence& orPicData )
{
    if( startNextProperty() )
    {
        sal_Int16 nData = maInStrm.readAligned< sal_Int16 >();
        if( ensureValid( nData == -1 ) )
            maStreamProps.push_back( ComplexPropVector::value_type( new PictureProperty( orPicData ) ) );
    }
}

bool AxBinaryPropertyReader::finalizeImport()
{
    // read large properties
    maInStrm.align( 4 );
    if( ensureValid( mnPropFlags == 0 ) && !maLargeProps.empty() )
    {
        for( ComplexPropVector::iterator aIt = maLargeProps.begin(), aEnd = maLargeProps.end(); ensureValid() && (aIt != aEnd); ++aIt )
        {
            ensureValid( (*aIt)->readProperty( maInStrm ) );
            maInStrm.align( 4 );
        }
    }
    maInStrm.seek( mnPropsEnd );

    // read stream properties (no stream alignment between properties!)
    if( ensureValid() && !maStreamProps.empty() )
        for( ComplexPropVector::iterator aIt = maStreamProps.begin(), aEnd = maStreamProps.end(); ensureValid() && (aIt != aEnd); ++aIt )
            ensureValid( (*aIt)->readProperty( maInStrm ) );

    return mbValid;
}

bool AxBinaryPropertyReader::ensureValid( bool bCondition )
{
    mbValid = mbValid && bCondition && !maInStrm.isEof();
    return mbValid;
}

bool AxBinaryPropertyReader::startNextProperty()
{
    bool bHasProp = getFlag( mnPropFlags, mnNextProp );
    setFlag( mnPropFlags, mnNextProp, false );
    mnNextProp <<= 1;
    return ensureValid() && bHasProp;
}

// ============================================================================

} // namespace ole
} // namespace oox

