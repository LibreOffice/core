/*************************************************************************
 *
 *  $RCSfile: ByteGrabber.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: mtg $ $Date: 2001-08-08 18:22:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/
#ifndef _BYTE_GRABBER_HXX_
#include "ByteGrabber.hxx"
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
