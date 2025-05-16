/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <package/Inflater.hxx>
#include <zstd.h>

namespace ZipUtils
{
class DLLPUBLIC_PACKAGE InflaterBytesZstd : public InflaterBytes
{
private:
    bool bFinished;
    const sal_Int8* sInBuffer;
    ZSTD_DCtx* pDCtx;
    ZSTD_inBuffer inBuffer;
    bool bStreamInitialized;

    sal_Int32 doInflateBytes(sal_Int8* pOutBuffer, sal_Int32 nNewOffset, sal_Int32 nNewLength);

public:
    InflaterBytesZstd();
    virtual ~InflaterBytesZstd() override;

    virtual void setInput(const sal_Int8* pBuffer, sal_Int32 nLen) override;
    virtual bool finished() const override { return bFinished; }
    virtual sal_Int32 doInflateSegment(sal_Int8* pOutBuffer, sal_Int32 nBufLen,
                                       sal_Int32 nNewOffset, sal_Int32 nNewLength) override;
    virtual void end() override final;
};

} // namespace ZipUtils

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
