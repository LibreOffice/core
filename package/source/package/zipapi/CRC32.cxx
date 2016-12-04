/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

using namespace rtl;
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
