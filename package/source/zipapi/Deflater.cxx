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
#include <Deflater.hxx>
#ifndef _ZLIB_H
#ifdef SYSTEM_ZLIB
#include <zlib.h>
#else
#include <external/zlib/zlib.h>
#endif
#endif
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <string.h> // for memset

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star;

/** Provides general purpose compression using the ZLIB compression
 * library.
 */

Deflater::~Deflater(void)
{
    end();
}
void Deflater::init (sal_Int32 nLevelArg, sal_Int32 nStrategyArg, sal_Bool bNowrap)
{
    pStream = new z_stream;
    /* Memset it to 0...sets zalloc/zfree/opaque to NULL */
    memset (pStream, 0, sizeof(*pStream));

    switch (deflateInit2(pStream, nLevelArg, Z_DEFLATED, bNowrap? -MAX_WBITS : MAX_WBITS,
                DEF_MEM_LEVEL, nStrategyArg))
    {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
            delete pStream;
            break;
        case Z_STREAM_ERROR:
            delete pStream;
            break;
        default:
             break;
    }
}

Deflater::Deflater(sal_Int32 nSetLevel, sal_Bool bNowrap)
: bFinish(sal_False)
, bFinished(sal_False)
, bSetParams(sal_False)
, nLevel(nSetLevel)
, nStrategy(DEFAULT_STRATEGY)
, nOffset(0)
, nLength(0)
{
    init(nSetLevel, DEFAULT_STRATEGY, bNowrap);
}

sal_Int32 Deflater::doDeflateBytes (uno::Sequence < sal_Int8 > &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    sal_Int32 nResult;
    if (bSetParams)
    {
        pStream->next_in   = (unsigned char*) sInBuffer.getConstArray() + nOffset;
        pStream->next_out  = (unsigned char*) rBuffer.getArray()+nNewOffset;
        pStream->avail_in  = nLength;
        pStream->avail_out = nNewLength;

#if defined SYSTEM_ZLIB || !defined ZLIB_PREFIX
        nResult = deflateParams(pStream, nLevel, nStrategy);
#else
        nResult = z_deflateParams(pStream, nLevel, nStrategy);
#endif
        switch (nResult)
        {
            case Z_OK:
                bSetParams = sal_False;
                nOffset += nLength - pStream->avail_in;
                nLength = pStream->avail_in;
                return nNewLength - pStream->avail_out;
            case Z_BUF_ERROR:
                bSetParams = sal_False;
                return 0;
            default:
                return 0;
        }
    }
    else
    {
        pStream->next_in   = (unsigned char*) sInBuffer.getConstArray() + nOffset;
        pStream->next_out  = (unsigned char*) rBuffer.getArray()+nNewOffset;
        pStream->avail_in  = nLength;
        pStream->avail_out = nNewLength;

#if defined SYSTEM_ZLIB || !defined ZLIB_PREFIX
        nResult = deflate(pStream, bFinish ? Z_FINISH : Z_NO_FLUSH);
#else
        nResult = z_deflate(pStream, bFinish ? Z_FINISH : Z_NO_FLUSH);
#endif
        switch (nResult)
        {
            case Z_STREAM_END:
                bFinished = sal_True;
            case Z_OK:
                nOffset += nLength - pStream->avail_in;
                nLength = pStream->avail_in;
                return nNewLength - pStream->avail_out;
            case Z_BUF_ERROR:
                bSetParams = sal_False;
                return 0;
            default:
                return 0;
        }
    }
}

void SAL_CALL Deflater::setInputSegment( const uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
{
    OSL_ASSERT( !(nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength()));

    sInBuffer = rBuffer;
    nOffset = nNewOffset;
    nLength = nNewLength;
}
void SAL_CALL Deflater::setLevel( sal_Int32 nNewLevel )
{
    if ((nNewLevel < 0 || nNewLevel > 9) && nNewLevel != DEFAULT_COMPRESSION)
    {
        // do error handling
    }
    if (nNewLevel != nLevel)
    {
        nLevel = nNewLevel;
        bSetParams = sal_True;
    }
}
sal_Bool SAL_CALL Deflater::needsInput(  )
{
    return nLength <=0;
}
void SAL_CALL Deflater::finish(  )
{
    bFinish = sal_True;
}
sal_Bool SAL_CALL Deflater::finished(  )
{
    return bFinished;
}
sal_Int32 SAL_CALL Deflater::doDeflateSegment( uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
{
    OSL_ASSERT( !(nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength()));
    return doDeflateBytes(rBuffer, nNewOffset, nNewLength);
}
sal_Int32 SAL_CALL Deflater::getTotalIn(  )
{
    return pStream->total_in;
}
sal_Int32 SAL_CALL Deflater::getTotalOut(  )
{
    return pStream->total_out;
}
void SAL_CALL Deflater::reset(  )
{
#if defined SYSTEM_ZLIB || !defined ZLIB_PREFIXB
    deflateReset(pStream);
#else
    z_deflateReset(pStream);
#endif
    bFinish = sal_False;
    bFinished = sal_False;
    nOffset = nLength = 0;
}
void SAL_CALL Deflater::end(  )
{
    if (pStream != NULL)
    {
#if defined SYSTEM_ZLIB || !defined ZLIB_PREFIX
        deflateEnd(pStream);
#else
        z_deflateEnd(pStream);
#endif
        delete pStream;
    }
    pStream = NULL;
}
