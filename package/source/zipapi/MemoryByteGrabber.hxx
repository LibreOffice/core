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
#ifndef _MEMORY_BYTE_GRABBER_HXX_
#define _MEMORY_BYTE_GRABBER_HXX_

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
        rtl_copyMemory( aData.getArray(), mpBuffer + mnCurrent, nBytesToRead );
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
    void SAL_CALL closeInput(  )
        throw(com::sun::star::io::NotConnectedException, com::sun::star::io::IOException, com::sun::star::uno::RuntimeException)
    {
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
    MemoryByteGrabber& operator >> (sal_Int8& rInt8)
    {
        if (mnCurrent + 1 > mnEnd )
            rInt8 = 0;
        else
            rInt8 = mpBuffer [mnCurrent++] & 0xFF;
        return *this;
    }
    MemoryByteGrabber& operator >> (sal_Int16& rInt16)
    {
        if (mnCurrent + 2 > mnEnd )
            rInt16 = 0;
        else
        {
            rInt16  =   mpBuffer[mnCurrent++] & 0xFF;
            rInt16 |= ( mpBuffer[mnCurrent++] & 0xFF ) << 8;
        }
        return *this;
    }
    MemoryByteGrabber& operator >> (sal_Int32& rInt32)
    {
        if (mnCurrent + 4 > mnEnd )
            rInt32 = 0;
        else
        {
            rInt32  =   mpBuffer[mnCurrent++] & 0xFF;
            rInt32 |= ( mpBuffer[mnCurrent++] & 0xFF ) << 8;
            rInt32 |= ( mpBuffer[mnCurrent++] & 0xFF ) << 16;
            rInt32 |= ( mpBuffer[mnCurrent++] & 0xFF ) << 24;
        }
        return *this;
    }

    MemoryByteGrabber& operator >> (sal_uInt8& rInt8)
    {
        if (mnCurrent + 1 > mnEnd )
            rInt8 = 0;
        else
            rInt8 = mpBuffer [mnCurrent++] & 0xFF;
        return *this;
    }
    MemoryByteGrabber& operator >> (sal_uInt16& rInt16)
    {
        if (mnCurrent + 2 > mnEnd )
            rInt16 = 0;
        else
        {
            rInt16  =   mpBuffer [mnCurrent++] & 0xFF;
            rInt16 |= ( mpBuffer [mnCurrent++] & 0xFF ) << 8;
        }
        return *this;
    }
    MemoryByteGrabber& operator >> (sal_uInt32& rInt32)
    {
        if (mnCurrent + 4 > mnEnd )
            rInt32 = 0;
        else
        {
            rInt32  =   mpBuffer [mnCurrent++] & 0xFF;
            rInt32 |= ( mpBuffer [mnCurrent++] & 0xFF ) << 8;
            rInt32 |= ( mpBuffer [mnCurrent++] & 0xFF ) << 16;
            rInt32 |= ( mpBuffer [mnCurrent++] & 0xFF ) << 24;
        }
        return *this;
    }
};

#endif
