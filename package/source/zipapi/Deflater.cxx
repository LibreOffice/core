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

#include <package/Deflater.hxx>
#include <zlib.h>
#include <com/sun/star/packages/zip/ZipConstants.hpp>
#include <osl/diagnose.h>
#include <string.h>

using namespace com::sun::star::packages::zip::ZipConstants;
using namespace com::sun::star;
using namespace ZipUtils;

/** Provides general purpose compression using the ZLIB compression
 * library.
 */

Deflater::~Deflater()
{
    end();
}
void Deflater::init (sal_Int32 nLevelArg, sal_Int32 nStrategyArg, bool bNowrap)
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

Deflater::Deflater(sal_Int32 nSetLevel, bool bNowrap)
: bFinish(false)
, bFinished(false)
, nStrategy(DEFAULT_STRATEGY)
, nOffset(0)
, nLength(0)
{
    init(nSetLevel, DEFAULT_STRATEGY, bNowrap);
}

sal_Int32 Deflater::doDeflateBytes (uno::Sequence < sal_Int8 > &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    sal_Int32 nResult;
    pStream->next_in   = reinterpret_cast<unsigned char*>(sInBuffer.getArray()) + nOffset;
    pStream->next_out  = reinterpret_cast<unsigned char*>(rBuffer.getArray())+nNewOffset;
    pStream->avail_in  = nLength;
    pStream->avail_out = nNewLength;

#if !defined Z_PREFIX
    nResult = deflate(pStream, bFinish ? Z_FINISH : Z_NO_FLUSH);
#else
    nResult = z_deflate(pStream, bFinish ? Z_FINISH : Z_NO_FLUSH);
#endif
    switch (nResult)
    {
        case Z_STREAM_END:
            bFinished = true;
        case Z_OK:
            nOffset += nLength - pStream->avail_in;
            nLength = pStream->avail_in;
            return nNewLength - pStream->avail_out;
        default:
            return 0;
    }
}

void SAL_CALL Deflater::setInputSegment( const uno::Sequence< sal_Int8 >& rBuffer )
{
    sInBuffer = rBuffer;
    nOffset = 0;
    nLength = rBuffer.getLength();
}

bool SAL_CALL Deflater::needsInput(  )
{
    return nLength <=0;
}
void SAL_CALL Deflater::finish(  )
{
    bFinish = true;
}
sal_Int32 SAL_CALL Deflater::doDeflateSegment( uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength )
{
    OSL_ASSERT( !(nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength()));
    return doDeflateBytes(rBuffer, nNewOffset, nNewLength);
}
sal_Int64 SAL_CALL Deflater::getTotalIn(  )
{
    return pStream->total_in; // FIXME64: zlib doesn't look 64bit clean here
}
sal_Int64 SAL_CALL Deflater::getTotalOut(  )
{
    return pStream->total_out; // FIXME64: zlib doesn't look 64bit clean here
}
void SAL_CALL Deflater::reset(  )
{
#if !defined Z_PREFIX
    deflateReset(pStream);
#else
    z_deflateReset(pStream);
#endif
    bFinish = false;
    bFinished = false;
    nOffset = nLength = 0;
}
void SAL_CALL Deflater::end(  )
{
    if (pStream != nullptr)
    {
#if !defined Z_PREFIX
        deflateEnd(pStream);
#else
        z_deflateEnd(pStream);
#endif
        delete pStream;
    }
    pStream = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
