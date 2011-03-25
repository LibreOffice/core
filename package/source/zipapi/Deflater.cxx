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
