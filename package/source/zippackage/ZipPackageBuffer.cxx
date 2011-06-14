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
#include "precompiled_package.hxx"
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
        throw BufferSizeExceededException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), *this );

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
        throw BufferSizeExceededException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), *this );

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
        throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >(), 1 );
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
