/*************************************************************************
 *
 *  $RCSfile: ByteChucker.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2000-11-22 14:55:50 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _BYTE_CHUCKER_HXX_
#include "ByteChucker.hxx"
#endif

using namespace ::com::sun::star;

/** ByteChucker implements the << operators on an XOutputStream. This is
 *  potentially quite slow and may need to be optimised
 */

ByteChucker::ByteChucker(uno::Reference<io::XOutputStream> xOstream)
: xStream(xOstream)
, xSeek (xOstream, uno::UNO_QUERY )
{
}
ByteChucker::~ByteChucker()
{
    if ( xStream.is() )
        xStream->closeOutput();
}

// XOutputStream chained...
void SAL_CALL ByteChucker::writeBytes( const uno::Sequence< sal_Int8 >& aData )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    xStream->writeBytes(aData);
}
void SAL_CALL ByteChucker::flush(  )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    xStream->flush();
}
void SAL_CALL ByteChucker::closeOutput(  )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    xStream->closeOutput();
}

// XSeekable chained...
sal_Int64 SAL_CALL ByteChucker::seek( sal_Int64 location )
    throw(lang::IllegalArgumentException, io::IOException, uno::RuntimeException)
{
    if (xSeek.is() )
    {
        sal_Int32 nLen = xSeek->getLength();
        if (location > nLen )
            location = nLen;
        xSeek->seek( location );
        return location;
    }
    else
        throw io::IOException();
}
sal_Int64 SAL_CALL ByteChucker::getPosition(  )
        throw(io::IOException, uno::RuntimeException)
{
    if (xSeek.is() )
        return xSeek->getPosition();
    else
        throw io::IOException();
}
sal_Int64 SAL_CALL ByteChucker::getLength(  )
        throw(io::IOException, uno::RuntimeException)
{
    if (xSeek.is() )
        return xSeek->getLength();
    else
        throw io::IOException();
}
void ByteChucker::putInt8( sal_Int8 nInt8)
{
    uno::Sequence< sal_Int8 > aSequence (1);
    aSequence[0]=nInt8 & 0xFF;
    xStream->writeBytes(aSequence);
}
void ByteChucker::putInt16(sal_Int16 nInt16)
{
    uno::Sequence< sal_Int8 > aSequence (2);
    aSequence[0] = (nInt16 >>0 ) & 0xFF;
    aSequence[1] = (nInt16 >>8 ) & 0xFF;
    xStream->writeBytes(aSequence);
}
void ByteChucker::putInt32(sal_Int32 nInt32)
{
    uno::Sequence< sal_Int8 > aSequence (4);
    aSequence[0] = (nInt32 >>  0 ) & 0xFF;
    aSequence[1] = (nInt32 >>  8 ) & 0xFF;
    aSequence[2] = (nInt32 >> 16 ) & 0xFF;
    aSequence[3] = (nInt32 >> 24 ) & 0xFF;
    xStream->writeBytes(aSequence);
}
ByteChucker& ByteChucker::operator << (sal_Int8 nInt8)
{
    uno::Sequence< sal_Int8 > aSequence (1);
    aSequence[0]=nInt8 & 0xFF;
    xStream->writeBytes(aSequence);
    return *this;
}
ByteChucker& ByteChucker::operator << (sal_Int16 nInt16)
{
    uno::Sequence< sal_Int8 > aSequence (2);
    aSequence[0] = (nInt16 >>0 ) & 0xFF;
    aSequence[1] = (nInt16 >>8 ) & 0xFF;
    xStream->writeBytes(aSequence);
    return *this;
}
ByteChucker& ByteChucker::operator << (sal_Int32 nInt32)
{
    uno::Sequence< sal_Int8 > aSequence (4);
    aSequence[0] = (nInt32 >>  0 ) & 0xFF;
    aSequence[1] = (nInt32 >>  8 ) & 0xFF;
    aSequence[2] = (nInt32 >> 16 ) & 0xFF;
    aSequence[3] = (nInt32 >> 24 ) & 0xFF;
    xStream->writeBytes(aSequence);
    return *this;
}

ByteChucker& ByteChucker::operator << (sal_uInt8 nuInt8)
{
    uno::Sequence< sal_Int8 > aSequence (1);
    aSequence[0]=nuInt8 & 0xFF;
    xStream->writeBytes(aSequence);
    return *this;
}
ByteChucker& ByteChucker::operator << (sal_uInt16 nuInt16)
{
    uno::Sequence< sal_Int8 > aSequence (2);
    aSequence[0] = (nuInt16 >>0 ) & 0xFF;
    aSequence[1] = (nuInt16 >>8 ) & 0xFF;
    xStream->writeBytes(aSequence);
    return *this;
}
ByteChucker& ByteChucker::operator << (sal_uInt32 nuInt32)
{
    uno::Sequence< sal_Int8 > aSequence (4);
    aSequence[0] = (nuInt32 >>  0 ) & 0xFF;
    aSequence[1] = (nuInt32 >>  8 ) & 0xFF;
    aSequence[2] = (nuInt32 >> 16 ) & 0xFF;
    aSequence[3] = (nuInt32 >> 24 ) & 0xFF;
    xStream->writeBytes(aSequence);
    return *this;
}
