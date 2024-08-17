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
#include <sal/log.hxx>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace ::com::sun::star;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

/** ByteGrabber implements the >> operators on an XOutputStream. This is
 *  potentially quite slow and may need to be optimised
 */

ByteGrabber::ByteGrabber(uno::Reference  < io::XInputStream > const & xIstream)
: xStream(xIstream)
, xSeek (xIstream, uno::UNO_QUERY )
{
    mpByteReader = dynamic_cast<comphelper::ByteReader*>(xStream.get());
    assert(mpByteReader);
}

ByteGrabber::~ByteGrabber()
{
}

void ByteGrabber::setInputStream (const uno::Reference < io::XInputStream >& xNewStream)
{
    xStream = xNewStream;
    xSeek.set(xNewStream, uno::UNO_QUERY);
    mpByteReader = dynamic_cast<comphelper::ByteReader*>(xStream.get());
    assert(mpByteReader);
}

// XInputStream chained
sal_Int32 ByteGrabber::readBytes( uno::Sequence< sal_Int8 >& aData,
                                        sal_Int32 nBytesToRead )
{
    return xStream->readBytes(aData, nBytesToRead );
}

sal_Int32 ByteGrabber::readBytes( sal_Int8* aData,
                                        sal_Int32 nBytesToRead )
{
    return mpByteReader->readSomeBytes(aData, nBytesToRead );
}

// XSeekable chained...
void ByteGrabber::seek( sal_Int64 location )
{
    if (!xSeek.is() )
        throw io::IOException(THROW_WHERE );

    xSeek->seek( location );
}

sal_Int64 ByteGrabber::getPosition(  )
{
    if (!xSeek.is() )
        throw io::IOException(THROW_WHERE );

    return xSeek->getPosition();
}

sal_Int64 ByteGrabber::getLength(  )
{
    if (!xSeek.is() )
        throw io::IOException(THROW_WHERE );

    return xSeek->getLength();
}

sal_uInt16 ByteGrabber::ReadUInt16()
{
    if (mpByteReader->readSomeBytes(maBuffer.data(), 2) != 2)
        return 0;

    return static_cast <sal_uInt16>
           ( (maBuffer[0] & 0xFF)
          | (maBuffer[1] & 0xFF) << 8);
}

sal_uInt32 ByteGrabber::ReadUInt32()
{
    if (mpByteReader->readSomeBytes(maBuffer.data(), 4) != 4)
        return 0;

    return static_cast < sal_uInt32 >
            ( (maBuffer[0] & 0xFF)
          | ( maBuffer[1] & 0xFF ) << 8
          | ( maBuffer[2] & 0xFF ) << 16
          | ( maBuffer[3] & 0xFF ) << 24 );
}

sal_uInt64 ByteGrabber::ReadUInt64()
{
    if (mpByteReader->readSomeBytes(maBuffer.data(), 8) != 8)
        return 0;

    return  static_cast<sal_uInt64>(maBuffer[0] & 0xFF)
          | static_cast<sal_uInt64>(maBuffer[1] & 0xFF) << 8
          | static_cast<sal_uInt64>(maBuffer[2] & 0xFF) << 16
          | static_cast<sal_uInt64>(maBuffer[3] & 0xFF) << 24
          | static_cast<sal_uInt64>(maBuffer[4] & 0xFF) << 32
          | static_cast<sal_uInt64>(maBuffer[5] & 0xFF) << 40
          | static_cast<sal_uInt64>(maBuffer[6] & 0xFF) << 48
          | static_cast<sal_uInt64>(maBuffer[7] & 0xFF) << 56;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
