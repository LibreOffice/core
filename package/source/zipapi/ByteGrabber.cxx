/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"
#include <ByteGrabber.hxx>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XInputStream.hpp>

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
    ::osl::MutexGuard aGuard( m_aMutex );
    xStream = xNewStream;
    xSeek = uno::Reference < io::XSeekable > (xNewStream, uno::UNO_QUERY);
}

// XInputStream chained
sal_Int32 SAL_CALL ByteGrabber::readBytes( uno::Sequence< sal_Int8 >& aData,
                                        sal_Int32 nBytesToRead )
    throw(io::NotConnectedException, io::BufferSizeExceededException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return xStream->readBytes(aData, nBytesToRead );
}

// XSeekable chained...
sal_Int64 SAL_CALL ByteGrabber::seek( sal_Int64 location )
    throw(lang::IllegalArgumentException, io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (xSeek.is() )
    {
        sal_Int64 nLen = xSeek->getLength();
        if ( location < 0 || location > nLen )
            throw lang::IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >(), 1 );
        if (location > nLen )
            location = nLen;
        xSeek->seek( location );
        return location;
    }
    else
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

sal_Int64 SAL_CALL ByteGrabber::getPosition(  )
        throw(io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (xSeek.is() )
        return xSeek->getPosition();
    else
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

sal_Int64 SAL_CALL ByteGrabber::getLength(  )
        throw(io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (xSeek.is() )
        return xSeek->getLength();
    else
        throw io::IOException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}

ByteGrabber& ByteGrabber::operator >> (sal_Int8& rInt8)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (xStream->readBytes(aSequence,1) != 1)
        rInt8 = 0;
    else
        rInt8 = aSequence[0] & 0xFF;
    return *this;
}

ByteGrabber& ByteGrabber::operator >> (sal_Int16& rInt16)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (xStream->readBytes ( aSequence, 2) != 2)
        rInt16 = 0;
    else
    {
        pSequence = aSequence.getConstArray();
        rInt16 = static_cast <sal_Int16>
               ( (pSequence[0] & 0xFF)
              | (pSequence[1] & 0xFF) << 8);
    }
    return *this;
}

ByteGrabber& ByteGrabber::operator >> (sal_Int32& rInt32)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (xStream->readBytes(aSequence, 4) != 4)
        rInt32 = 0;
    else
    {
        pSequence = aSequence.getConstArray();
        rInt32 = static_cast < sal_Int32 >
                ( (pSequence[0] & 0xFF)
              | ( pSequence[1] & 0xFF ) << 8
              | ( pSequence[2] & 0xFF ) << 16
              | ( pSequence[3] & 0xFF ) << 24 );
    }
    return *this;
}

ByteGrabber& ByteGrabber::operator >> (sal_uInt8& rInt8)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (xStream->readBytes(aSequence,1) != 1)
        rInt8 = 0;
    else
        rInt8 = static_cast < sal_uInt8 > (aSequence[0] & 0xFF );
    return *this;
}
ByteGrabber& ByteGrabber::operator >> (sal_uInt16& rInt16)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (xStream->readBytes(aSequence, 2) != 2)
        rInt16 = 0;
    else
    {
        pSequence = aSequence.getConstArray();
        rInt16 = static_cast <sal_uInt16>
               ( (pSequence[0] & 0xFF)
              | (pSequence[1] & 0xFF) << 8);
    }
    return *this;
}
ByteGrabber& ByteGrabber::operator >> (sal_uInt32& ruInt32)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if (xStream->readBytes(aSequence, 4) != 4)
        ruInt32 = 0;
    else
    {
        pSequence = aSequence.getConstArray();
        ruInt32 = static_cast < sal_uInt32 >
                ( (pSequence[0] & 0xFF)
              | ( pSequence[1] & 0xFF ) << 8
              | ( pSequence[2] & 0xFF ) << 16
              | ( pSequence[3] & 0xFF ) << 24 );
    }
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
