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
#include <PackageConstants.hxx>
#include <string.h>

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using com::sun::star::lang::IllegalArgumentException;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

ZipPackageBuffer::ZipPackageBuffer()
: m_nBufferSize (n_ConstBufferSize)
, m_nEnd(0)
, m_nCurrent(0)
, m_bMustInitBuffer ( true )
{
}
ZipPackageBuffer::~ZipPackageBuffer()
{
}

sal_Int32 SAL_CALL ZipPackageBuffer::readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    if (nBytesToRead < 0)
        throw BufferSizeExceededException(THROW_WHERE, *this );

    if (nBytesToRead + m_nCurrent > m_nEnd)
        nBytesToRead = static_cast < sal_Int32 > (m_nEnd - m_nCurrent);

    aData.realloc ( nBytesToRead );
    memcpy(aData.getArray(), m_aBuffer.getConstArray() + m_nCurrent, nBytesToRead);
    m_nCurrent +=nBytesToRead;
    return nBytesToRead;
}

sal_Int32 SAL_CALL ZipPackageBuffer::readSomeBytes( Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    return readBytes(aData, nMaxBytesToRead);
}
void SAL_CALL ZipPackageBuffer::skipBytes( sal_Int32 nBytesToSkip )
{
    if (nBytesToSkip < 0)
        throw BufferSizeExceededException(THROW_WHERE, *this );

    if (nBytesToSkip + m_nCurrent > m_nEnd)
        nBytesToSkip = static_cast < sal_Int32 > (m_nEnd - m_nCurrent);

    m_nCurrent+=nBytesToSkip;
}
sal_Int32 SAL_CALL ZipPackageBuffer::available(  )
{
    return static_cast < sal_Int32 > (m_nEnd - m_nCurrent);
}
void SAL_CALL ZipPackageBuffer::closeInput(  )
{
}
void SAL_CALL ZipPackageBuffer::writeBytes( const Sequence< sal_Int8 >& aData )
{
    sal_Int64 nDataLen = aData.getLength(), nCombined = m_nEnd + nDataLen;

    if ( nCombined > m_nBufferSize)
    {
        do
            m_nBufferSize *=2;
        while (nCombined > m_nBufferSize);
        m_aBuffer.realloc(static_cast < sal_Int32 > (m_nBufferSize));
        m_bMustInitBuffer = false;
    }
    else if (m_bMustInitBuffer)
    {
         m_aBuffer.realloc ( static_cast < sal_Int32 > ( m_nBufferSize ) );
        m_bMustInitBuffer = false;
    }
    memcpy( m_aBuffer.getArray() + m_nCurrent, aData.getConstArray(), static_cast < sal_Int32 > (nDataLen));
    m_nCurrent+=nDataLen;
    if (m_nCurrent>m_nEnd)
        m_nEnd = m_nCurrent;
}
void SAL_CALL ZipPackageBuffer::flush(  )
{
}
void SAL_CALL ZipPackageBuffer::closeOutput(  )
{
}
void SAL_CALL ZipPackageBuffer::seek( sal_Int64 location )
{
    if ( location > m_nEnd || location < 0 )
        throw IllegalArgumentException(THROW_WHERE, uno::Reference< uno::XInterface >(), 1 );
    m_nCurrent = location;
}
sal_Int64 SAL_CALL ZipPackageBuffer::getPosition(  )
{
    return m_nCurrent;
}
sal_Int64 SAL_CALL ZipPackageBuffer::getLength(  )
{
    return m_nEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
