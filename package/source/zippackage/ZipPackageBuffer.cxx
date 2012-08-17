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

#include <ZipPackageBuffer.hxx>
#include <string.h> // for memcpy

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using com::sun::star::lang::IllegalArgumentException;

ZipPackageBuffer::ZipPackageBuffer(sal_Int64 nNewBufferSize )
: m_nBufferSize (nNewBufferSize)
, m_nEnd(0)
, m_nCurrent(0)
, m_bMustInitBuffer ( sal_True )
{
}
ZipPackageBuffer::~ZipPackageBuffer(void)
{
}

sal_Int32 SAL_CALL ZipPackageBuffer::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if (nBytesToRead < 0)
        throw BufferSizeExceededException(OSL_LOG_PREFIX, *this );

    if (nBytesToRead + m_nCurrent > m_nEnd)
        nBytesToRead = static_cast < sal_Int32 > (m_nEnd - m_nCurrent);

    aData.realloc ( nBytesToRead );
    memcpy(aData.getArray(), m_aBuffer.getConstArray() + m_nCurrent, nBytesToRead);
    m_nCurrent +=nBytesToRead;
    return nBytesToRead;
}

sal_Int32 SAL_CALL ZipPackageBuffer::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    return readBytes(aData, nMaxBytesToRead);
}
void SAL_CALL ZipPackageBuffer::skipBytes( sal_Int32 nBytesToSkip )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    if (nBytesToSkip < 0)
        throw BufferSizeExceededException(OSL_LOG_PREFIX, *this );

    if (nBytesToSkip + m_nCurrent > m_nEnd)
        nBytesToSkip = static_cast < sal_Int32 > (m_nEnd - m_nCurrent);

    m_nCurrent+=nBytesToSkip;
}
sal_Int32 SAL_CALL ZipPackageBuffer::available(  )
        throw(NotConnectedException, IOException, RuntimeException)
{
    return static_cast < sal_Int32 > (m_nEnd - m_nCurrent);
}
void SAL_CALL ZipPackageBuffer::closeInput(  )
        throw(NotConnectedException, IOException, RuntimeException)
{
}
void SAL_CALL ZipPackageBuffer::writeBytes( const Sequence< sal_Int8 >& aData )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
    sal_Int64 nDataLen = aData.getLength(), nCombined = m_nEnd + nDataLen;

    if ( nCombined > m_nBufferSize)
    {
        do
            m_nBufferSize *=2;
        while (nCombined > m_nBufferSize);
        m_aBuffer.realloc(static_cast < sal_Int32 > (m_nBufferSize));
        m_bMustInitBuffer = sal_False;
    }
    else if (m_bMustInitBuffer)
    {
         m_aBuffer.realloc ( static_cast < sal_Int32 > ( m_nBufferSize ) );
        m_bMustInitBuffer = sal_False;
    }
    memcpy( m_aBuffer.getArray() + m_nCurrent, aData.getConstArray(), static_cast < sal_Int32 > (nDataLen));
    m_nCurrent+=nDataLen;
    if (m_nCurrent>m_nEnd)
        m_nEnd = m_nCurrent;
}
void SAL_CALL ZipPackageBuffer::flush(  )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
}
void SAL_CALL ZipPackageBuffer::closeOutput(  )
        throw(NotConnectedException, BufferSizeExceededException, IOException, RuntimeException)
{
}
void SAL_CALL ZipPackageBuffer::seek( sal_Int64 location )
        throw( IllegalArgumentException, IOException, RuntimeException)
{
    if ( location > m_nEnd || location < 0 )
        throw IllegalArgumentException(OSL_LOG_PREFIX, uno::Reference< uno::XInterface >(), 1 );
    m_nCurrent = location;
}
sal_Int64 SAL_CALL ZipPackageBuffer::getPosition(  )
        throw(IOException, RuntimeException)
{
    return m_nCurrent;
}
sal_Int64 SAL_CALL ZipPackageBuffer::getLength(  )
        throw(IOException, RuntimeException)
{
    return m_nEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
