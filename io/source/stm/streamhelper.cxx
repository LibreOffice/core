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
#include "precompiled_io.hxx"
#include <rtl/alloc.h>

#include <limits>
#include <string.h>

#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/uno/Exception.hpp>

using namespace ::com::sun::star::uno;

#include "streamhelper.hxx"

namespace io_stm {

void MemFIFO::write( const Sequence< sal_Int8 > &seq )
    throw ( I_FIFO_OutOfMemoryException,
            I_FIFO_OutOfBoundsException )
{
    try
    {
        writeAt(getSize(), seq );
    }
    catch( IRingBuffer_OutOfMemoryException & )
    {
        throw I_FIFO_OutOfMemoryException();
    }
    catch( IRingBuffer_OutOfBoundsException & )
    {
        throw I_FIFO_OutOfBoundsException();
    }
}

void MemFIFO::read( Sequence<sal_Int8> &seq , sal_Int32 nBufferLen ) throw (I_FIFO_OutOfBoundsException)
{
    try
    {
        readAt(0, seq , nBufferLen);
        forgetFromStart( nBufferLen );
    }
    catch ( IRingBuffer_OutOfBoundsException & )
    {
        throw I_FIFO_OutOfBoundsException();
    }
}

void MemFIFO::skip( sal_Int32 nBytesToSkip ) throw ( I_FIFO_OutOfBoundsException )
{
    try
    {
        forgetFromStart( nBytesToSkip );
    }
    catch( IRingBuffer_OutOfBoundsException & )
    {
        throw I_FIFO_OutOfBoundsException();
    }
}



MemRingBuffer::MemRingBuffer()
{
    m_nBufferLen            = 0;
    m_p                     = 0;
    m_nStart                = 0;
    m_nOccupiedBuffer       = 0;
}

MemRingBuffer::~MemRingBuffer()
{
    if( m_p ) {
        rtl_freeMemory( m_p );
    }
}

void MemRingBuffer::resizeBuffer( sal_Int32 nMinSize ) throw( IRingBuffer_OutOfMemoryException)
{
    sal_Int32 nNewLen = 1;

    while( nMinSize > nNewLen ) {
        nNewLen = nNewLen << 1;
    }

    // buffer never shrinks !
    if( nNewLen < m_nBufferLen ) {
        nNewLen = m_nBufferLen;
    }

    if( nNewLen != m_nBufferLen ) {
        m_p = ( sal_Int8 * ) rtl_reallocateMemory( m_p , nNewLen );
        if( !m_p ) {
            throw IRingBuffer_OutOfMemoryException();
        }

        if( m_nStart + m_nOccupiedBuffer > m_nBufferLen ) {
            memmove( &( m_p[m_nStart+(nNewLen-m_nBufferLen)]) , &(m_p[m_nStart]) , m_nBufferLen - m_nStart );
            m_nStart += nNewLen - m_nBufferLen;
        }
        m_nBufferLen = nNewLen;
    }
}


void MemRingBuffer::readAt( sal_Int32 nPos, Sequence<sal_Int8> &seq , sal_Int32 nBytesToRead ) const
                                                        throw(IRingBuffer_OutOfBoundsException)
{
    if( nPos + nBytesToRead > m_nOccupiedBuffer ) {
        throw IRingBuffer_OutOfBoundsException();
    }

    sal_Int32 nStartReadingPos = nPos + m_nStart;
    if( nStartReadingPos >= m_nBufferLen ) {
        nStartReadingPos -= m_nBufferLen;
    }

    seq.realloc( nBytesToRead );

    if( nStartReadingPos + nBytesToRead > m_nBufferLen ) {
        sal_Int32 nDeltaLen = m_nBufferLen - nStartReadingPos;
        memcpy( seq.getArray() , &(m_p[nStartReadingPos]) , nDeltaLen );
        memcpy( &(seq.getArray()[nDeltaLen]), m_p , nBytesToRead - nDeltaLen );
    }
    else {
        memcpy( seq.getArray() , &(m_p[nStartReadingPos]) , nBytesToRead );
    }
}


void MemRingBuffer::writeAt( sal_Int32 nPos, const Sequence<sal_Int8> &seq )
                                                        throw (IRingBuffer_OutOfBoundsException,
                                                                IRingBuffer_OutOfMemoryException )
{
    checkInvariants();
    sal_Int32 nLen = seq.getLength();

    if( nPos < 0 || nPos > std::numeric_limits< sal_Int32 >::max() - nLen )
    {
        throw IRingBuffer_OutOfBoundsException();
    }

    if( nPos + nLen - m_nOccupiedBuffer > 0 ) {
        resizeBuffer( nPos + seq.getLength() );
    }

    sal_Int32 nStartWritingIndex = m_nStart + nPos;
    if( nStartWritingIndex >= m_nBufferLen ) {
        nStartWritingIndex -= m_nBufferLen;
    }

    if( nLen + nStartWritingIndex > m_nBufferLen ) {
        // two area copy
        memcpy( &(m_p[nStartWritingIndex]) , seq.getConstArray(), m_nBufferLen-nStartWritingIndex );
        memcpy( m_p , &( seq.getConstArray()[m_nBufferLen-nStartWritingIndex] ),
                                        nLen - (m_nBufferLen-nStartWritingIndex) );

    }
    else {
        // one area copy
        memcpy( &( m_p[nStartWritingIndex]), seq.getConstArray() , nLen );
    }
    m_nOccupiedBuffer = Max( nPos + seq.getLength() , m_nOccupiedBuffer );
    checkInvariants();
}


sal_Int32 MemRingBuffer::getSize()  const throw()
{
    return m_nOccupiedBuffer;
}

void MemRingBuffer::forgetFromStart( sal_Int32 nBytesToForget ) throw (IRingBuffer_OutOfBoundsException)
{
    checkInvariants();
    if( nBytesToForget > m_nOccupiedBuffer ) {
        throw IRingBuffer_OutOfBoundsException();
    }
    m_nStart += nBytesToForget;
    if( m_nStart >= m_nBufferLen ) {
        m_nStart = m_nStart - m_nBufferLen;
    }
    m_nOccupiedBuffer -= nBytesToForget;
    checkInvariants();
}


void MemRingBuffer::forgetFromEnd( sal_Int32 nBytesToForget ) throw (IRingBuffer_OutOfBoundsException)
{
    checkInvariants();
    if( nBytesToForget > m_nOccupiedBuffer ) {
        throw IRingBuffer_OutOfBoundsException();
    }
    m_nOccupiedBuffer -= nBytesToForget;
    checkInvariants();
}


void MemRingBuffer::shrink() throw ()
{
    checkInvariants();

    // Up to now, only shrinking of while buffer works.
    // No other shrinking supported up to now.
    if( ! m_nOccupiedBuffer ) {
        if( m_p ) {
            free( m_p );
        }
        m_p = 0;
        m_nBufferLen = 0;
        m_nStart = 0;
    }

    checkInvariants();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
