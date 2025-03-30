/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>
#include <InflaterBytesZlib.hxx>
#include <string.h>
#include <zlib.h>

using namespace ZipUtils;

InflaterBytesZlib::InflaterBytesZlib()
    : bFinished(false),
      nOffset(0),
      nLength(0),
      sInBuffer(nullptr),
      pStream(std::make_unique<z_stream>())
{
    memset(pStream.get(), 0, sizeof(*pStream));
    sal_Int32 nRes = inflateInit2(pStream.get(), -MAX_WBITS);
    switch (nRes)
    {
        case Z_OK:
            break;
        case Z_MEM_ERROR:
        case Z_STREAM_ERROR:
            pStream.reset();
            break;
        default:
            break;
    }
}

InflaterBytesZlib::~InflaterBytesZlib() { end(); }

void InflaterBytesZlib::setInput(const sal_Int8* rBuffer, sal_Int32 nBufLen)
{
    sInBuffer = rBuffer;
    nOffset   = 0;
    nLength   = nBufLen;
}

sal_Int32 InflaterBytesZlib::doInflateSegment(sal_Int8* pOutBuffer, sal_Int32 nBufLen, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    if (nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > nBufLen)
    {
        return 0;
    }
    return doInflateBytes(pOutBuffer, nNewOffset, nNewLength);
}

void InflaterBytesZlib::end()
{
    if (pStream)
    {
#if !defined Z_PREFIX
        inflateEnd(pStream.get());
#else
        z_inflateEnd(pStream.get());
#endif
        pStream.reset();
    }
}

sal_Int32 InflaterBytesZlib::doInflateBytes(sal_Int8* pOutBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    if (!pStream)
        return 0;

    pStream->next_in   = reinterpret_cast<const unsigned char*>(sInBuffer + nOffset);
    pStream->avail_in  = nLength;
    pStream->next_out  = reinterpret_cast<unsigned char*>(pOutBuffer + nNewOffset);
    pStream->avail_out = nNewLength;

#if !defined Z_PREFIX
    sal_Int32 nResult = ::inflate(pStream.get(), Z_PARTIAL_FLUSH);
#else
    sal_Int32 nResult = ::z_inflate(pStream.get(), Z_PARTIAL_FLUSH);
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
        case Z_NEED_DICT:
            nOffset += nLength - pStream->avail_in;
            nLength = pStream->avail_in;
            return 0;
        default:
            break;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
