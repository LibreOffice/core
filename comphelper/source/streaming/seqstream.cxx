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
#include "precompiled_comphelper.hxx"
#include <comphelper/seqstream.hxx>

#include <memory.h> // for memcpy

namespace comphelper
{
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::osl;

//---------------------------------------------------------------------------------------------
// class SequenceInputStream
//---------------------------------------------------------------------------------------------

//------------------------------------------------------------------
SequenceInputStream::SequenceInputStream(const ByteSequence& rData)
:   m_aData(rData)
,   m_nPos(0)
{
}

// checks if closed, returns available size, not mutex-protected
//------------------------------------------------------------------
inline sal_Int32 SequenceInputStream::avail()
{
    if (m_nPos == -1)
        throw NotConnectedException(::rtl::OUString(), *this);

    return m_aData.getLength() - m_nPos;
}

// com::sun::star::io::XInputStream
//------------------------------------------------------------------
sal_Int32 SAL_CALL SequenceInputStream::readBytes( Sequence<sal_Int8>& aData, sal_Int32 nBytesToRead )
    throw(NotConnectedException, BufferSizeExceededException,
          IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nAvail = avail();

    if (nBytesToRead < 0)
        throw BufferSizeExceededException(::rtl::OUString(),*this);

    if (nAvail < nBytesToRead)
        nBytesToRead = nAvail;

    aData.realloc(nBytesToRead);
    memcpy(aData.getArray(), m_aData.getConstArray() + m_nPos, nBytesToRead);
    m_nPos += nBytesToRead;

    return nBytesToRead;
}

//------------------------------------------------------------------
sal_Int32 SAL_CALL SequenceInputStream::readSomeBytes( Sequence<sal_Int8>& aData, sal_Int32 nMaxBytesToRead )
    throw(NotConnectedException, BufferSizeExceededException,
          IOException, RuntimeException)
{
    // all data is available at once
    return readBytes(aData, nMaxBytesToRead);
}

//------------------------------------------------------------------
void SAL_CALL SequenceInputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw(NotConnectedException, BufferSizeExceededException,
          IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nAvail = avail();

    if (nBytesToSkip < 0)
        throw BufferSizeExceededException(::rtl::OUString(),*this);

    if (nAvail < nBytesToSkip)
        nBytesToSkip = nAvail;

    m_nPos += nBytesToSkip;
}

//------------------------------------------------------------------
sal_Int32 SAL_CALL SequenceInputStream::available(  )
    throw(NotConnectedException, IOException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return avail();
}

//------------------------------------------------------------------
void SAL_CALL SequenceInputStream::closeInput(  )
    throw(NotConnectedException, IOException, RuntimeException)
{
    if (m_nPos == -1)
        throw NotConnectedException(::rtl::OUString(), *this);

    m_nPos = -1;
}

void SAL_CALL SequenceInputStream::seek( sal_Int64 location ) throw (IllegalArgumentException, IOException, RuntimeException)
{
    if ( location > m_aData.getLength() || location < 0 || location > SAL_MAX_INT32 )
        throw IllegalArgumentException();
    m_nPos = (sal_Int32) location;
}

sal_Int64 SAL_CALL SequenceInputStream::getPosition() throw (IOException, RuntimeException)
{
    return m_nPos;
}

sal_Int64 SAL_CALL SequenceInputStream::getLength(  ) throw (IOException, RuntimeException)
{
    return m_aData.getLength();
}

//--------------------------------------------------------------------------
OSequenceOutputStream::OSequenceOutputStream(Sequence< sal_Int8 >& _rSeq, double _nResizeFactor, sal_Int32 _nMinimumResize, sal_Int32 _nMaximumResize)
    :m_rSequence(_rSeq)
    ,m_nResizeFactor(_nResizeFactor)
    ,m_nMinimumResize(_nMinimumResize)
    ,m_nMaximumResize(_nMaximumResize)
    ,m_nSize(0) // starting at position 0
    ,m_bConnected(sal_True)
{
    OSL_ENSURE(m_nResizeFactor > 1, "OSequenceOutputStream::OSequenceOutputStream : invalid resize factor !");
    OSL_ENSURE((m_nMaximumResize < 0) || (m_nMaximumResize > m_nMinimumResize),
        "OSequenceOutputStream::OSequenceOutputStream : these limits don't make any sense !");

    if (m_nResizeFactor <= 1)
        m_nResizeFactor = 1.3;
    if ((m_nMaximumResize >= 0) && (m_nMaximumResize <= m_nMinimumResize))
        m_nMaximumResize = m_nMinimumResize * 2;
        // this heuristic is as good as any other ... supply better parameters if you don't like it :)
}

//--------------------------------------------------------------------------
void SAL_CALL OSequenceOutputStream::writeBytes( const Sequence< sal_Int8 >& _rData ) throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (!m_bConnected)
        throw NotConnectedException();

    // ensure the sequence has enoungh space left
    if (m_nSize + _rData.getLength() > m_rSequence.getLength())
    {
        sal_Int32 nCurrentLength = m_rSequence.getLength();
        sal_Int32 nNewLength = static_cast< sal_Int32 >(
            nCurrentLength * m_nResizeFactor);

        if (m_nMinimumResize > nNewLength - nCurrentLength)
            // we have a minimum so it's not too inefficient for small sequences and small write requests
            nNewLength = nCurrentLength + m_nMinimumResize;

        if ((m_nMaximumResize > 0) && (nNewLength - nCurrentLength > m_nMaximumResize))
            // such a large step is not allowed
            nNewLength = nCurrentLength + m_nMaximumResize;

        if (nNewLength < m_nSize + _rData.getLength())
        {   // it's not enough .... the data would not fit

            // let's take the double amount of the length of the data to be written, as the next write
            // request could be as large as this one
            sal_Int32 nNewGrowth = _rData.getLength() * 2;
            if ((m_nMaximumResize > 0) && (nNewGrowth > m_nMaximumResize))
            {   // we came to the limit, again ...
                nNewGrowth = m_nMaximumResize;
                if (nNewGrowth + nCurrentLength < m_nSize + _rData.getLength())
                    // but it would not fit if we respect the limit
                    nNewGrowth = m_nSize + _rData.getLength() - nCurrentLength;
            }
            nNewLength = nCurrentLength + nNewGrowth;
        }

        // round it off to the next multiple of 4 ...
        nNewLength = (nNewLength + 3) / 4 * 4;

        m_rSequence.realloc(nNewLength);
    }

    OSL_ENSURE(m_rSequence.getLength() >= m_nSize + _rData.getLength(),
        "ooops ... the realloc algorithm seems to be wrong :( !");

    memcpy(m_rSequence.getArray() + m_nSize, _rData.getConstArray(), _rData.getLength());
    m_nSize += _rData.getLength();
}

//--------------------------------------------------------------------------
void SAL_CALL OSequenceOutputStream::flush(  ) throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (!m_bConnected)
        throw NotConnectedException();

    // cut the sequence to the real size
    m_rSequence.realloc(m_nSize);
}

//--------------------------------------------------------------------------
void SAL_CALL OSequenceOutputStream::closeOutput(  ) throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if (!m_bConnected)
        throw NotConnectedException();

    // cut the sequence to the real size
    m_rSequence.realloc(m_nSize);
    // and don't allow any further accesses
    m_bConnected = sal_False;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
