/*************************************************************************
 *
 *  $RCSfile: streamhelper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:18 $
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
#include <rtl/alloc.h>

#include <assert.h>
#include <string.h>

#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/uno/Exception.hpp>

using namespace ::com::sun::star::uno;

#include "streamhelper.hxx"

namespace io_stm {

void MemFIFO::write( const Sequence< sal_Int8 > &seq )
    throw ( IFIFO_OutOfMemoryException,
            IFIFO_OutOfBoundsException )
{
    try
    {
        writeAt(getSize(), seq );
    }
    catch( IRingBuffer_OutOfMemoryException & )
    {
        throw IFIFO_OutOfMemoryException();
    }
    catch( IRingBuffer_OutOfBoundsException & )
    {
        throw IFIFO_OutOfBoundsException();
    }
}

void MemFIFO::read( Sequence<sal_Int8> &seq , sal_Int32 nBufferLen ) throw (IFIFO_OutOfBoundsException)
{
    try
    {
        readAt(0, seq , nBufferLen);
        forgetFromStart( nBufferLen );
    }
    catch ( IRingBuffer_OutOfBoundsException & )
    {
        throw IFIFO_OutOfBoundsException();
    }
}

void MemFIFO::skip( sal_Int32 nBytesToSkip ) throw ( IFIFO_OutOfBoundsException )
{
    try
    {
        forgetFromStart( nBytesToSkip );
    }
    catch( IRingBuffer_OutOfBoundsException & )
    {
        throw IFIFO_OutOfBoundsException();
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

    if( nPos > 0x80000000 || nPos < 0 ||  nPos + nLen < 0 || nPos + nLen > 0x80000000 )
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
