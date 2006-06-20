/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ByteChucker.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:12:44 $
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
#ifndef _BYTE_CHUCKER_HXX_
#include <ByteChucker.hxx>
#endif
#ifndef _PACKAGE_CONSTANTS_HXX_
#include <PackageConstants.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

ByteChucker::ByteChucker(Reference<XOutputStream> xOstream)
: xStream(xOstream)
, xSeek (xOstream, UNO_QUERY )
, a1Sequence ( 1 )
, a2Sequence ( 2 )
, a4Sequence ( 4 )
, p1Sequence ( a1Sequence.getArray() )
, p2Sequence ( a2Sequence.getArray() )
, p4Sequence ( a4Sequence.getArray() )
{
}

ByteChucker::~ByteChucker()
{
}

// XOutputStream chained...
void SAL_CALL ByteChucker::writeBytes( const Sequence< sal_Int8 >& aData, sal_Int32 /*nLength*/, const sal_Int8 * const /*pData*/ )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    xStream->writeBytes(aData);
}
void SAL_CALL ByteChucker::flush(  )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    xStream->flush();
}
void SAL_CALL ByteChucker::closeOutput(  )
    throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    xStream->closeOutput();
}

// XSeekable chained...
sal_Int64 SAL_CALL ByteChucker::seek( sal_Int64 location )
    throw(IllegalArgumentException, IOException, RuntimeException)
{
    sal_Int64 nLen = xSeek->getLength();
    if ( location < 0 || location > nLen )
        throw IllegalArgumentException();
    if (location > nLen )
        location = nLen;
    xSeek->seek( location );
    return location;
}
sal_Int64 SAL_CALL ByteChucker::getPosition(  )
        throw(IOException, RuntimeException)
{
    return xSeek->getPosition();
}
sal_Int64 SAL_CALL ByteChucker::getLength(  )
        throw(IOException, RuntimeException)
{
    return xSeek->getLength();
}

ByteChucker& ByteChucker::operator << (sal_Int8 nInt8)
{
    p1Sequence[0] = nInt8  & 0xFF;
    writeBytes( a1Sequence, 1, p1Sequence );
    return *this;
}

ByteChucker& ByteChucker::operator << (sal_Int16 nInt16)
{
    p2Sequence[0] = static_cast< sal_Int8 >((nInt16 >>  0 ) & 0xFF);
    p2Sequence[1] = static_cast< sal_Int8 >((nInt16 >>  8 ) & 0xFF);
    writeBytes( a2Sequence, 2, p2Sequence );
    return *this;
}
ByteChucker& ByteChucker::operator << (sal_Int32 nInt32)
{
    p4Sequence[0] = static_cast< sal_Int8 >((nInt32 >>  0 ) & 0xFF);
    p4Sequence[1] = static_cast< sal_Int8 >((nInt32 >>  8 ) & 0xFF);
    p4Sequence[2] = static_cast< sal_Int8 >((nInt32 >> 16 ) & 0xFF);
    p4Sequence[3] = static_cast< sal_Int8 >((nInt32 >> 24 ) & 0xFF);
    writeBytes( a4Sequence, 4, p4Sequence );
    return *this;
}

ByteChucker& ByteChucker::operator << (sal_uInt8 nuInt8)
{
    p1Sequence[0] = nuInt8  & 0xFF;
    writeBytes( a1Sequence, 1, p1Sequence );
    return *this;
}
ByteChucker& ByteChucker::operator << (sal_uInt16 nuInt16)
{
    p2Sequence[0] = static_cast< sal_Int8 >((nuInt16 >>  0 ) & 0xFF);
    p2Sequence[1] = static_cast< sal_Int8 >((nuInt16 >>  8 ) & 0xFF);
    writeBytes( a2Sequence, 2, p2Sequence );
    return *this;
}
ByteChucker& ByteChucker::operator << (sal_uInt32 nuInt32)
{
    p4Sequence[0] = static_cast < sal_Int8 > ((nuInt32 >>  0 ) & 0xFF);
    p4Sequence[1] = static_cast < sal_Int8 > ((nuInt32 >>  8 ) & 0xFF);
    p4Sequence[2] = static_cast < sal_Int8 > ((nuInt32 >> 16 ) & 0xFF);
    p4Sequence[3] = static_cast < sal_Int8 > ((nuInt32 >> 24 ) & 0xFF);
    writeBytes( a4Sequence, 4, p4Sequence );
    return *this;
}
