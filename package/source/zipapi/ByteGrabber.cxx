/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
            throw lang::IllegalArgumentException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 );
        if (location > nLen )
            location = nLen;
        xSeek->seek( location );
        return location;
    }
    else
        throw io::IOException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
}

sal_Int64 SAL_CALL ByteGrabber::getPosition(  )
        throw(io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (xSeek.is() )
        return xSeek->getPosition();
    else
        throw io::IOException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
}

sal_Int64 SAL_CALL ByteGrabber::getLength(  )
        throw(io::IOException, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (xSeek.is() )
        return xSeek->getLength();
    else
        throw io::IOException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >() );
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
