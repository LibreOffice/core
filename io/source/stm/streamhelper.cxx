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

#include <rtl/alloc.h>

#include <limits>
#include <string.h>

#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/uno/Exception.hpp>

using namespace ::com::sun::star::uno;

#include "streamhelper.hxx"

namespace io_stm {

void MemFIFO::write( const Sequence< sal_Int8 > &seq )
{
    writeAt(getSize(), seq);
}

void MemFIFO::read( Sequence<sal_Int8> &seq , sal_Int32 nBufferLen )
{
    readAt(0, seq , nBufferLen);
    forgetFromStart( nBufferLen );
}

void MemFIFO::skip( sal_Int32 nBytesToSkip )
{
    forgetFromStart( nBytesToSkip );
}

MemRingBuffer::MemRingBuffer()
{
    m_nBufferLen            = 0;
    m_p                     = nullptr;
    m_nStart                = 0;
    m_nOccupiedBuffer       = 0;
}

MemRingBuffer::~MemRingBuffer()
{
    std::free( m_p );
}

void MemRingBuffer::resizeBuffer( sal_Int32 nMinSize )
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
        if (auto p = static_cast<sal_Int8*>(std::realloc(m_p, nNewLen)))
            m_p = p;
        else
        {
            throw css::io::BufferSizeExceededException(
                "MemRingBuffer::resizeBuffer BufferSizeExceededException");
        }

        if( m_nStart + m_nOccupiedBuffer > m_nBufferLen ) {
            memmove( &( m_p[m_nStart+(nNewLen-m_nBufferLen)]) , &(m_p[m_nStart]) , m_nBufferLen - m_nStart );
            m_nStart += nNewLen - m_nBufferLen;
        }
        m_nBufferLen = nNewLen;
    }
}


void MemRingBuffer::readAt( sal_Int32 nPos, Sequence<sal_Int8> &seq , sal_Int32 nBytesToRead ) const
{
    if( nPos + nBytesToRead > m_nOccupiedBuffer ) {
        throw css::io::BufferSizeExceededException(
            "MemRingBuffer::readAt BufferSizeExceededException");
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
{
    checkInvariants();
    sal_Int32 nLen = seq.getLength();

    if( nPos < 0 || nPos > std::numeric_limits< sal_Int32 >::max() - nLen )
    {
        throw css::io::BufferSizeExceededException(
            "MemRingBuffer::writeAt BufferSizeExceededException");
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
    m_nOccupiedBuffer = std::max( nPos + seq.getLength() , m_nOccupiedBuffer );
    checkInvariants();
}


sal_Int32 MemRingBuffer::getSize()  const throw()
{
    return m_nOccupiedBuffer;
}

void MemRingBuffer::forgetFromStart( sal_Int32 nBytesToForget )
{
    checkInvariants();
    if( nBytesToForget > m_nOccupiedBuffer ) {
        throw css::io::BufferSizeExceededException(
            "MemRingBuffer::forgetFromStart BufferSizeExceededException");
    }
    m_nStart += nBytesToForget;
    if( m_nStart >= m_nBufferLen ) {
        m_nStart = m_nStart - m_nBufferLen;
    }
    m_nOccupiedBuffer -= nBytesToForget;
    checkInvariants();
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
