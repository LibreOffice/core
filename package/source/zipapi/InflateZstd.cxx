/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <package/InflateZstd.hxx>
#include <string.h>

using namespace com::sun::star::uno;
using namespace ZipUtils;

InflateZstd::InflateZstd(bool)
    : bFinished(false)
    , nLastInflateError(0)
    , pDCtx(ZSTD_createDCtx())
    , bStreamInitialized(false)
{
    if (!pDCtx)
    {
        nLastInflateError = static_cast<sal_Int32>(ZSTD_error_memory_allocation);
    }
    inBuffer = { nullptr, 0, 0 };
}

InflateZstd::~InflateZstd() { end(); }

void InflateZstd::setInput(const Sequence<sal_Int8>& rBuffer)
{
    if (!pDCtx)
    {
        bStreamInitialized = false;
        return;
    }
    sInBuffer = rBuffer;
    inBuffer.src = sInBuffer.getConstArray();
    inBuffer.size = sInBuffer.getLength();
    inBuffer.pos = 0;
    bStreamInitialized = true;
}

sal_Int32 InflateZstd::doInflateSegment(Sequence<sal_Int8>& rBuffer, sal_Int32 nNewOffset,
                                        sal_Int32 nNewLength)
{
    if (nNewOffset < 0 || nNewLength < 0 || nNewOffset + nNewLength > rBuffer.getLength())
        return 0;

    return doInflateBytes(rBuffer, nNewOffset, nNewLength);
}

void InflateZstd::end()
{
    if (pDCtx)
    {
        ZSTD_freeDCtx(pDCtx);
        pDCtx = nullptr;
    }
    bStreamInitialized = false;
    inBuffer = { nullptr, 0, 0 };
}

sal_Int32 InflateZstd::doInflateBytes(Sequence<sal_Int8>& rBuffer, sal_Int32 nNewOffset,
                                      sal_Int32 nNewLength)
{
    if (bFinished)
    {
        return 0;
    }
    if (!pDCtx || !bStreamInitialized)
    {
        nLastInflateError = 1;
        return 0;
    }
    nLastInflateError = 0;
    ZSTD_outBuffer outBuffer
        = { rBuffer.getArray() + nNewOffset, static_cast<size_t>(nNewLength), 0 };
    size_t ret = ZSTD_decompressStream(pDCtx, &outBuffer, &inBuffer);
    if (ZSTD_isError(ret))
    {
        nLastInflateError = static_cast<sal_Int32>(ret);
        ZSTD_DCtx_reset(pDCtx, ZSTD_reset_session_only);
        return 0;
    }
    if (ret == 0)
        bFinished = true;

    return static_cast<sal_Int32>(outBuffer.pos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
