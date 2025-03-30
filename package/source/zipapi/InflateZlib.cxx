/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <package/InflateZlib.hxx>
#include <string.h>
#include <zlib.h>

using namespace com::sun::star::uno;
using namespace ZipUtils;

InflateZlib::InflateZlib(bool bNoWrap)
    : bFinished(false),
      bNeedDict(false),
      nOffset(0),
      nLength(0),
      nLastInflateError(0),
      pStream(std::make_unique<z_stream>())
{
    memset(pStream.get(), 0, sizeof(*pStream));
    sal_Int32 nRes = inflateInit2(pStream.get(), bNoWrap ? -MAX_WBITS : MAX_WBITS);
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

InflateZlib::~InflateZlib()
{
    end();
}

void InflateZlib::setInput(const Sequence<sal_Int8>& rBuffer)
{
    sInBuffer = rBuffer;
    nOffset   = 0;
    nLength   = rBuffer.getLength();
}

sal_Int32 InflateZlib::doInflateSegment(Sequence<sal_Int8>& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    if (nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength())
    {
        return 0;
    }
    return doInflateBytes(rBuffer, nNewOffset, nNewLength);
}

void InflateZlib::end()
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

sal_Int32 InflateZlib::doInflateBytes(Sequence<sal_Int8>& rBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    if (!pStream)
    {
        nLastInflateError = Z_STREAM_ERROR;
        return 0;
    }
    nLastInflateError = 0;

    pStream->next_in   = reinterpret_cast<const unsigned char*>(sInBuffer.getConstArray() + nOffset);
    pStream->avail_in  = nLength;
    pStream->next_out  = reinterpret_cast<unsigned char*>(rBuffer.getArray() + nNewOffset);
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
            bNeedDict = true;
            nOffset   += nLength - pStream->avail_in;
            nLength   = pStream->avail_in;
            return 0;
        default:
            if (nLength && nNewLength)
                nLastInflateError = nResult;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
