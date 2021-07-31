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

#define ZLIB_CONST

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
void Deflater::init (sal_Int32 nLevelArg, bool bNowrap)
{
    pStream.reset(new z_stream);
    /* Memset it to 0...sets zalloc/zfree/opaque to NULL */
    memset (pStream.get(), 0, sizeof(*pStream));

    switch (deflateInit2(pStream.get(), nLevelArg, Z_DEFLATED, bNowrap? -MAX_WBITS : MAX_WBITS,
                DEF_MEM_LEVEL, DEFAULT_STRATEGY))
    {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
            pStream.reset();
            break;
        case Z_STREAM_ERROR:
            pStream.reset();
            break;
        default:
             break;
    }
}

Deflater::Deflater(sal_Int32 nSetLevel, bool bNowrap)
: bFinish(false)
, bFinished(false)
, nOffset(0)
, nLength(0)
{
    init(nSetLevel, bNowrap);
}

sal_Int32 Deflater::doDeflateBytes (uno::Sequence < sal_Int8 > &rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    sal_Int32 nResult;
    pStream->next_in   = reinterpret_cast<const unsigned char*>( sInBuffer.getConstArray() + nOffset );
    pStream->next_out  = reinterpret_cast<unsigned char*>(rBuffer.getArray())+nNewOffset;
    pStream->avail_in  = nLength;
    pStream->avail_out = nNewLength;

#if !defined Z_PREFIX
    nResult = deflate(pStream.get(), bFinish ? Z_FINISH : Z_NO_FLUSH);
#else
    nResult = z_deflate(pStream.get(), bFinish ? Z_FINISH : Z_NO_FLUSH);
#endif
    switch (nResult)
    {
        case Z_STREAM_END:
            bFinished = true;
            [[fallthrough]];
        case Z_OK:
            nOffset += nLength - pStream->avail_in;
            nLength = pStream->avail_in;
            return nNewLength - pStream->avail_out;
        default:
            return 0;
    }
}

void Deflater::setInputSegment( const uno::Sequence< sal_Int8 >& rBuffer )
{
    sInBuffer = rBuffer;
    nOffset = 0;
    nLength = rBuffer.getLength();
}

bool Deflater::needsInput() const
{
    return nLength <=0;
}
void Deflater::finish(  )
{
    bFinish = true;
}
sal_Int32 Deflater::doDeflateSegment( uno::Sequence< sal_Int8 >& rBuffer, sal_Int32 nNewLength )
{
    OSL_ASSERT( !(nNewLength < 0 || nNewLength > rBuffer.getLength()));
    return doDeflateBytes(rBuffer, /*nNewOffset*/0, nNewLength);
}
sal_Int64 Deflater::getTotalIn() const
{
    return pStream->total_in; // FIXME64: zlib doesn't look 64bit clean here
}
sal_Int64 Deflater::getTotalOut() const
{
    return pStream->total_out; // FIXME64: zlib doesn't look 64bit clean here
}
void Deflater::reset(  )
{
#if !defined Z_PREFIX
    deflateReset(pStream.get());
#else
    z_deflateReset(pStream.get());
#endif
    bFinish = false;
    bFinished = false;
    nOffset = nLength = 0;
}
void Deflater::end(  )
{
    if (pStream)
    {
#if !defined Z_PREFIX
        deflateEnd(pStream.get());
#else
        z_deflateEnd(pStream.get());
#endif
        pStream.reset();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
