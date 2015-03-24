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
#ifndef INCLUDED_PACKAGE_SOURCE_ZIPAPI_MEMORYBYTEGRABBER_HXX
#define INCLUDED_PACKAGE_SOURCE_ZIPAPI_MEMORYBYTEGRABBER_HXX

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <string.h>

class MemoryByteGrabber
{
protected:
    const com::sun::star::uno::Sequence < sal_Int8 > maBuffer;
    const sal_Int8 *mpBuffer;
    sal_Int32 mnCurrent, mnEnd;
public:
    MemoryByteGrabber ( const com::sun::star::uno::Sequence < sal_Int8 > & rBuffer )
    : maBuffer ( rBuffer )
    , mpBuffer ( rBuffer.getConstArray() )
    , mnCurrent ( 0 )
    , mnEnd ( rBuffer.getLength() )
    {
    }
    MemoryByteGrabber()
    {
    }
    const sal_Int8 * getCurrentPos () { return mpBuffer + mnCurrent; }

    // XInputStream chained
    sal_Int32 SAL_CALL readBytes( com::sun::star::uno::Sequence< sal_Int8 >& aData,
                                            sal_Int32 nBytesToRead )
        throw(com::sun::star::io::NotConnectedException, com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
    {
        if ( nBytesToRead < 0)
            throw com::sun::star::io::BufferSizeExceededException();

        if (nBytesToRead + mnCurrent > mnEnd)
            nBytesToRead = mnEnd - mnCurrent;

        aData.realloc ( nBytesToRead );
        memcpy( aData.getArray(), mpBuffer + mnCurrent, nBytesToRead );
        mnCurrent += nBytesToRead;
        return nBytesToRead;
    }

    sal_Int32 SAL_CALL readSomeBytes( com::sun::star::uno::Sequence< sal_Int8 >& aData,
                                                    sal_Int32 nMaxBytesToRead )
        throw(com::sun::star::io::NotConnectedException, com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
    {
        return readBytes( aData, nMaxBytesToRead );
    }
    void SAL_CALL skipBytes( sal_Int32 nBytesToSkip )
        throw(com::sun::star::io::NotConnectedException, com::sun::star::io::BufferSizeExceededException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
    {
        mnCurrent += nBytesToSkip;
    }
    sal_Int32 SAL_CALL available(  )
        throw(com::sun::star::io::NotConnectedException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
    {
        return mnEnd - mnCurrent;
    }

    // XSeekable chained...
    sal_Int64 SAL_CALL seek( sal_Int64 location )
        throw(com::sun::star::lang::IllegalArgumentException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
    {
        if ( location < 0 || location > mnEnd )
            throw com::sun::star::lang::IllegalArgumentException ();
        mnCurrent = static_cast < sal_Int32 > ( location );
        return mnCurrent;
    }
    sal_Int64 SAL_CALL getPosition(  )
            throw(com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
    {
        return mnCurrent;
    }
    sal_Int64 SAL_CALL getLength(  )
            throw(com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
    {
        return mnEnd;
    }
    sal_Int8 ReadInt8()
    {
        if (mnCurrent + 1 > mnEnd )
            return 0;
        return mpBuffer [mnCurrent++] & 0xFF;
    }
    sal_Int16 ReadInt16()
    {
        if (mnCurrent + 2 > mnEnd )
            return 0;
        sal_Int16 nInt16  =   mpBuffer[mnCurrent++] & 0xFF;
        nInt16 |= ( mpBuffer[mnCurrent++] & 0xFF ) << 8;
        return nInt16;
    }
    sal_Int32 ReadInt32()
    {
        if (mnCurrent + 4 > mnEnd )
            return 0;

        sal_Int32 nInt32  =   mpBuffer[mnCurrent++] & 0xFF;
        nInt32 |= ( mpBuffer[mnCurrent++] & 0xFF ) << 8;
        nInt32 |= ( mpBuffer[mnCurrent++] & 0xFF ) << 16;
        nInt32 |= ( mpBuffer[mnCurrent++] & 0xFF ) << 24;
        return nInt32;
    }

    sal_uInt8 ReadUInt8()
    {
        if (mnCurrent + 1 > mnEnd )
            return 0;
        return mpBuffer [mnCurrent++] & 0xFF;
    }
    sal_uInt16 ReadUInt16()
    {
        if (mnCurrent + 2 > mnEnd )
            return 0;

        sal_uInt16 nInt16  =   mpBuffer [mnCurrent++] & 0xFF;
        nInt16 |= ( mpBuffer [mnCurrent++] & 0xFF ) << 8;
        return nInt16;
    }
    sal_uInt32 ReadUInt32()
    {
        if (mnCurrent + 4 > mnEnd )
            return 0;

        sal_uInt32 nInt32  =   mpBuffer [mnCurrent++] & 0xFF;
        nInt32 |= ( mpBuffer [mnCurrent++] & 0xFF ) << 8;
        nInt32 |= ( mpBuffer [mnCurrent++] & 0xFF ) << 16;
        nInt32 |= ( mpBuffer [mnCurrent++] & 0xFF ) << 24;
        return nInt32;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
