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
#include <CRC32.hxx>
#ifndef _ZLIB_H
#ifdef SYSTEM_ZLIB
#include <zlib.h>
#else
#include <external/zlib/zlib.h>
#endif
#endif
#include <PackageConstants.hxx>
#include <com/sun/star/io/XInputStream.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::io;

/** A class to compute the CRC32 value of a data stream
 */

CRC32::CRC32()
: nCRC(0)
{
}
CRC32::~CRC32()
{
}
void SAL_CALL CRC32::reset()
    throw(RuntimeException)
{
    nCRC=0;
}
sal_Int32 SAL_CALL CRC32::getValue()
    throw(RuntimeException)
{
    return nCRC & 0xFFFFFFFFL;
}
/** Update CRC32 with specified sequence of bytes
 */
void SAL_CALL CRC32::updateSegment(const Sequence< sal_Int8 > &b,
                                    sal_Int32 off,
                                    sal_Int32 len)
        throw(RuntimeException)
{
    nCRC = crc32(nCRC, (const unsigned char*)b.getConstArray()+off, len );
}
/** Update CRC32 with specified sequence of bytes
 */
void SAL_CALL CRC32::update(const Sequence< sal_Int8 > &b)
        throw(RuntimeException)
{
    nCRC = crc32(nCRC, (const unsigned char*)b.getConstArray(),b.getLength());
}

sal_Int32 SAL_CALL CRC32::updateStream( Reference < XInputStream > & xStream )
    throw ( RuntimeException )
{
    sal_Int32 nLength, nTotal = 0;
    Sequence < sal_Int8 > aSeq ( n_ConstBufferSize );
    do
    {
        nLength = xStream->readBytes ( aSeq, n_ConstBufferSize );
        updateSegment ( aSeq, 0, nLength );
        nTotal += nLength;
    }
    while ( nLength == n_ConstBufferSize );

    return nTotal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
