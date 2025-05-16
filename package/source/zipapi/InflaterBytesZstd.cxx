/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <InflaterBytesZstd.hxx>
#include <string.h>

using namespace ZipUtils;

InflaterBytesZstd::InflaterBytesZstd()
    : bFinished(false)
    , sInBuffer(nullptr)
    , pDCtx(ZSTD_createDCtx())
    , bStreamInitialized(false)
{
    inBuffer = { nullptr, 0, 0 };
}

InflaterBytesZstd::~InflaterBytesZstd() { end(); }

void InflaterBytesZstd::setInput(const sal_Int8* rBuffer, sal_Int32 nBufLen)
{
    sInBuffer = rBuffer;
    inBuffer.src = sInBuffer;
    inBuffer.size = nBufLen;
    inBuffer.pos = 0;
    bStreamInitialized = true;
}

sal_Int32 InflaterBytesZstd::doInflateSegment(sal_Int8* pOutBuffer, sal_Int32 nBufLen,
                                              sal_Int32 nNewOffset, sal_Int32 nNewLength)
{
    if (nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > nBufLen)
        return 0;

    return doInflateBytes(pOutBuffer, nNewOffset, nNewLength);
}

void InflaterBytesZstd::end()
{
    if (pDCtx)
    {
        ZSTD_freeDCtx(pDCtx);
        pDCtx = nullptr;
    }
    bStreamInitialized = false;
    inBuffer = { nullptr, 0, 0 };
}

sal_Int32 InflaterBytesZstd::doInflateBytes(sal_Int8* pOutBuffer, sal_Int32 nNewOffset,
                                            sal_Int32 nNewLength)
{
    if (!pDCtx || !bStreamInitialized)
        return 0;

    ZSTD_outBuffer outBuffer = { pOutBuffer + nNewOffset, static_cast<size_t>(nNewLength), 0 };
    size_t ret = ZSTD_decompressStream(pDCtx, &outBuffer, &inBuffer);
    if (ZSTD_isError(ret))
    {
        ZSTD_DCtx_reset(pDCtx, ZSTD_reset_session_only);
        return 0;
    }
    if (ret != 0)
    {
        ZSTD_DCtx_reset(pDCtx, ZSTD_reset_session_only);
        return static_cast<sal_Int32>(outBuffer.pos);
    }
    if (ret == 0)
        bFinished = true;

    return static_cast<sal_Int32>(outBuffer.pos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
