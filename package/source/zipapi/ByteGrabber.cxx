/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ByteGrabber.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:26:15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"
#ifndef _BYTE_GRABBER_HXX_
#include <ByteGrabber.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

using namespace ::com::sun::star;

/** ByteGrabber implements the >> operators on an XOutputStream. This is
 *  potentially quite slow and may need to be optimised
 */

ByteGrabber::ByteGrabber(uno::Reference  < io::XInputStream > xIstream)
: xStream(xIstream)
, xSeek (xIstream, uno::UNO_QUERY )
, aSequence ( 4 )
{
    pSequence = aSequence.getArray();
}

ByteGrabber::~ByteGrabber()
{
}
void ByteGrabber::setInputStream (uno::Reference < io::XInputStream > xNewStream)
{
    xStream = xNewStream;
    xSeek = uno::Reference < io::XSeekable > (xNewStream, uno::UNO_QUERY);
}

// XInputStream chained
sal_Int32 SAL_CALL ByteGrabber::readBytes( uno::Sequence< sal_Int8 >& aData,
                                        sal_Int32 nBytesToRead )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    return xStream->readBytes(aData, nBytesToRead );
}
sal_Int32 SAL_CALL ByteGrabber::readSomeBytes( uno::Sequence< sal_Int8 >& aData,
                                                sal_Int32 nMaxBytesToRead )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    return xStream->readSomeBytes( aData, nMaxBytesToRead );
}
void SAL_CALL ByteGrabber::skipBytes( sal_Int32 nBytesToSkip )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    xStream->skipBytes( nBytesToSkip );
}
sal_Int32 SAL_CALL ByteGrabber::available(  )
    throw(io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    return xStream->available();
}
void SAL_CALL ByteGrabber::closeInput(  )
    throw(io::NotConnectedException, io::IOException, uno::RuntimeException)
{
    xStream->closeInput();
}

// XSeekable chained...
sal_Int64 SAL_CALL ByteGrabber::seek( sal_Int64 location )
    throw(lang::IllegalArgumentException, io::IOException, uno::RuntimeException)
{
    if (xSeek.is() )
    {
        sal_Int64 nLen = xSeek->getLength();
        if ( location < 0 || location > nLen )
            throw lang::IllegalArgumentException();
        if (location > nLen )
            location = nLen;
        xSeek->seek( location );
        return location;
    }
    else
        throw io::IOException();
}
sal_Int64 SAL_CALL ByteGrabber::getPosition(  )
        throw(io::IOException, uno::RuntimeException)
{
    if (xSeek.is() )
        return xSeek->getPosition();
    else
        throw io::IOException();
}
sal_Int64 SAL_CALL ByteGrabber::getLength(  )
        throw(io::IOException, uno::RuntimeException)
{
    if (xSeek.is() )
        return xSeek->getLength();
    else
        throw io::IOException();
}
ByteGrabber& ByteGrabber::operator >> (sal_Int8& rInt8)
{
    if (xStream->readBytes(aSequence,1) != 1)
        rInt8 = 0;
    else
        rInt8 = aSequence[0] & 0xFF;
    return *this;
}
ByteGrabber& ByteGrabber::operator >> (sal_Int16& rInt16)
{
    if (xStream->readBytes ( aSequence, 2) != 2)
        rInt16 = 0;
    else
    {
        pSequence = aSequence.getConstArray();
        rInt16 = static_cast <sal_Int16>
               ( pSequence[0] & 0xFF
              | (pSequence[1] & 0xFF) << 8);
    }
    return *this;
}
ByteGrabber& ByteGrabber::operator >> (sal_Int32& rInt32)
{
    if (xStream->readBytes(aSequence, 4) != 4)
        rInt32 = 0;
    else
    {
        pSequence = aSequence.getConstArray();
        rInt32 = static_cast < sal_Int32 >
                ( pSequence[0] & 0xFF
              | ( pSequence[1] & 0xFF ) << 8
              | ( pSequence[2] & 0xFF ) << 16
              | ( pSequence[3] & 0xFF ) << 24 );
    }
    return *this;
}

ByteGrabber& ByteGrabber::operator >> (sal_uInt8& rInt8)
{
    if (xStream->readBytes(aSequence,1) != 1)
        rInt8 = 0;
    else
        rInt8 = static_cast < sal_uInt8 > (aSequence[0] & 0xFF );
    return *this;
}
ByteGrabber& ByteGrabber::operator >> (sal_uInt16& rInt16)
{
    if (xStream->readBytes(aSequence, 2) != 2)
        rInt16 = 0;
    else
    {
        pSequence = aSequence.getConstArray();
        rInt16 = static_cast <sal_uInt16>
               ( pSequence[0] & 0xFF
              | (pSequence[1] & 0xFF) << 8);
    }
    return *this;
}
ByteGrabber& ByteGrabber::operator >> (sal_uInt32& ruInt32)
{
    if (xStream->readBytes(aSequence, 4) != 4)
        ruInt32 = 0;
    else
    {
        pSequence = aSequence.getConstArray();
        ruInt32 = static_cast < sal_uInt32 >
                ( pSequence[0] & 0xFF
              | ( pSequence[1] & 0xFF ) << 8
              | ( pSequence[2] & 0xFF ) << 16
              | ( pSequence[3] & 0xFF ) << 24 );
    }
    return *this;
}
