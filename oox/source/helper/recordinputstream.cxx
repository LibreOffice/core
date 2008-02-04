/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: recordinputstream.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:36:34 $
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

#include "oox/helper/recordinputstream.hxx"
#include <vector>
#include <string.h>

using ::rtl::OUString;

namespace oox {

// ============================================================================

RecordInputStream::RecordInputStream( const RecordDataSequence& rData ) :
    maData( rData ),
    mnRecSize( rData.getLength() ),
    mnRecPos( 0 ),
    mbValid( true )
{
}

sal_Int32 RecordInputStream::read( void* opData, sal_Int32 nBytes )
{
    sal_Int32 nReadSize = ::std::min( nBytes, getRecLeft() );
    OSL_ENSURE( !mbValid || (nReadSize == nBytes), "RecordInputStream::read - buffer overflow" );
    mbValid = nReadSize == nBytes;
    if( mbValid && opData && (nReadSize > 0) )
        memcpy( opData, maData.getConstArray() + mnRecPos, nReadSize );
    mnRecPos += nReadSize;
    return nReadSize;
}

OUString RecordInputStream::readString( bool b32BitLen )
{
    OUString aString;
    sal_Int32 nCharCount = b32BitLen ? readValue< sal_Int32 >() : readValue< sal_Int16 >();
    // string length -1 is often used to indicate a missing string
    OSL_ENSURE( !mbValid || (nCharCount >= -1), "RecordInputStream::readString - invalid string length" );
    if( mbValid && (nCharCount >= 0) )
    {
        ::std::vector< sal_Unicode > aBuffer;
        aBuffer.reserve( getLimitedValue< size_t, sal_Int32 >( nCharCount + 1, 0, 0xFFFF ) );
        for( sal_Int32 nCharIdx = 0; mbValid && (nCharIdx < nCharCount); ++nCharIdx )
        {
            sal_uInt16 nChar;
            readValue( nChar );
            aBuffer.push_back( static_cast< sal_Unicode >( nChar ) );
        }
        aBuffer.push_back( 0 );
        aString = OUString( &aBuffer.front() );
    }
    return aString;
}

void RecordInputStream::seek( sal_Int32 nRecPos )
{
    mnRecPos = getLimitedValue< sal_Int32, sal_Int32 >( nRecPos, 0, mnRecSize );
    OSL_ENSURE( !mbValid || (nRecPos == mnRecPos), "RecordInputStream::seek - invalid position" );
    mbValid = nRecPos == mnRecPos;
}

void RecordInputStream::skip( sal_Int32 nBytes )
{
    sal_Int32 nSkipSize = getLimitedValue< sal_Int32, sal_Int32 >( nBytes, 0, getRecLeft() );
    OSL_ENSURE( !mbValid || (nSkipSize == nBytes), "RecordInputStream::skip - buffer overflow" );
    mbValid = nSkipSize == nBytes;
    mnRecPos += nSkipSize;
}

// ============================================================================

} // namespace oox

