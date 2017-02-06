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

#include <sal/config.h>

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <comphelper/seqstream.hxx>

#include <osl/diagnose.h>

#include <memory.h>

namespace comphelper
{
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::osl;


// class SequenceInputStream


SequenceInputStream::SequenceInputStream(
    css::uno::Sequence<sal_Int8> const & rData)
:   m_aData(rData)
,   m_nPos(0)
{
}

// checks if closed, returns available size, not mutex-protected

inline sal_Int32 SequenceInputStream::avail()
{
    if (m_nPos == -1)
        throw NotConnectedException(OUString(), *this);

    return m_aData.getLength() - m_nPos;
}

// css::io::XInputStream

sal_Int32 SAL_CALL SequenceInputStream::readBytes( Sequence<sal_Int8>& aData, sal_Int32 nBytesToRead )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nAvail = avail();

    if (nBytesToRead < 0)
        throw BufferSizeExceededException(OUString(),*this);

    if (nAvail < nBytesToRead)
        nBytesToRead = nAvail;

    aData.realloc(nBytesToRead);
    memcpy(aData.getArray(), m_aData.getConstArray() + m_nPos, nBytesToRead);
    m_nPos += nBytesToRead;

    return nBytesToRead;
}


sal_Int32 SAL_CALL SequenceInputStream::readSomeBytes( Sequence<sal_Int8>& aData, sal_Int32 nMaxBytesToRead )
{
    // all data is available at once
    return readBytes(aData, nMaxBytesToRead);
}


void SAL_CALL SequenceInputStream::skipBytes( sal_Int32 nBytesToSkip )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nAvail = avail();

    if (nBytesToSkip < 0)
        throw BufferSizeExceededException(OUString(),*this);

    if (nAvail < nBytesToSkip)
        nBytesToSkip = nAvail;

    m_nPos += nBytesToSkip;
}


sal_Int32 SAL_CALL SequenceInputStream::available(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return avail();
}


void SAL_CALL SequenceInputStream::closeInput(  )
{
    if (m_nPos == -1)
        throw NotConnectedException(OUString(), *this);

    m_nPos = -1;
}

void SAL_CALL SequenceInputStream::seek( sal_Int64 location )
{
    if ( location > m_aData.getLength() || location < 0 || location > SAL_MAX_INT32 )
        throw IllegalArgumentException();
    m_nPos = (sal_Int32) location;
}

sal_Int64 SAL_CALL SequenceInputStream::getPosition()
{
    return m_nPos;
}

sal_Int64 SAL_CALL SequenceInputStream::getLength(  )
{
    return m_aData.getLength();
}


OSequenceOutputStream::OSequenceOutputStream(Sequence< sal_Int8 >& _rSeq, double _nResizeFactor, sal_Int32 _nMinimumResize)
    :m_rSequence(_rSeq)
    ,m_nResizeFactor(_nResizeFactor)
    ,m_nMinimumResize(_nMinimumResize)
    ,m_nSize(0) // starting at position 0
    ,m_bConnected(true)
{
    OSL_ENSURE(m_nResizeFactor > 1, "OSequenceOutputStream::OSequenceOutputStream : invalid resize factor !");

    if (m_nResizeFactor <= 1)
        m_nResizeFactor = 1.3;
}


void SAL_CALL OSequenceOutputStream::writeBytes( const Sequence< sal_Int8 >& _rData )
{
    MutexGuard aGuard(m_aMutex);
    if (!m_bConnected)
        throw NotConnectedException();

    // ensure the sequence has enough space left
    if (m_nSize + _rData.getLength() > m_rSequence.getLength())
    {
        sal_Int32 nCurrentLength = m_rSequence.getLength();
        sal_Int32 nNewLength = static_cast< sal_Int32 >(
            nCurrentLength * m_nResizeFactor);

        if (m_nMinimumResize > nNewLength - nCurrentLength)
            // we have a minimum so it's not too inefficient for small sequences and small write requests
            nNewLength = nCurrentLength + m_nMinimumResize;

        if (nNewLength < m_nSize + _rData.getLength())
        {   // it's not enough .... the data would not fit

            // let's take the double amount of the length of the data to be written, as the next write
            // request could be as large as this one
            sal_Int32 nNewGrowth = _rData.getLength() * 2;
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


void SAL_CALL OSequenceOutputStream::flush(  )
{
    MutexGuard aGuard(m_aMutex);
    if (!m_bConnected)
        throw NotConnectedException();

    // cut the sequence to the real size
    m_rSequence.realloc(m_nSize);
}


void SAL_CALL OSequenceOutputStream::closeOutput(  )
{
    MutexGuard aGuard(m_aMutex);
    if (!m_bConnected)
        throw NotConnectedException();

    // cut the sequence to the real size
    m_rSequence.realloc(m_nSize);
    // and don't allow any further accesses
    m_bConnected = false;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
