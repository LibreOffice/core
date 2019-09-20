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

#include <skia/salbmp.hxx>

#include <o3tl/safeint.hxx>
#include <tools/helpers.hxx>

#include <salgdi.hxx>

#include <SkCanvas.h>
#include <SkImage.h>

#ifdef DBG_UTIL
#include <fstream>
#endif

SkiaSalBitmap::SkiaSalBitmap() {}

SkiaSalBitmap::~SkiaSalBitmap() {}

static SkColorType getSkColorType(int bitCount, const BitmapPalette& palette)
{
    switch (bitCount)
    {
        case 8:
            return palette.IsGreyPalette() ? kGray_8_SkColorType : kAlpha_8_SkColorType;
        case 24:
            return kRGB_888x_SkColorType;
        case 32:
            return kN32_SkColorType;
        default:
            abort();
    }
}

static bool isValidBitCount(sal_uInt16 nBitCount)
{
    return (nBitCount == 1) || (nBitCount == 4) || (nBitCount == 8) || (nBitCount == 24)
           || (nBitCount == 32);
}

SkiaSalBitmap::SkiaSalBitmap(const SkImage& image)
{
    assert(image.colorType() == kN32_SkColorType);
    if (Create(Size(image.width(), image.height()), 32, BitmapPalette()))
    {
        SkCanvas canvas(mBitmap);
        // TODO makeNonTextureImage() ?
        canvas.drawImage(&image, 0, 0);
    }
}

bool SkiaSalBitmap::Create(const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal)
{
    Destroy();
    if (!isValidBitCount(nBitCount))
        return false;
    if (nBitCount >= 8)
    {
        if (!mBitmap.tryAllocPixels(
                SkImageInfo::Make(rSize.Width(), rSize.Height(), getSkColorType(nBitCount, rPal),
                                  nBitCount == 32 ? kPremul_SkAlphaType : kOpaque_SkAlphaType)))
        {
            return false;
        }
    }
    else
    {
        // Skia doesn't support the (ancient) low bpp bit counts, so handle them manually
        int bitScanlineWidth;
        if (o3tl::checked_multiply<int>(rSize.Width(), nBitCount, bitScanlineWidth))
        {
            SAL_WARN("vcl.gdi", "checked multiply failed");
            return false;
        }
        mScanlineSize = AlignedWidth4Bytes(bitScanlineWidth);
        sal_uInt8* buffer = nullptr;
        if (mScanlineSize != 0 && rSize.Height() != 0)
            buffer = new sal_uInt8[mScanlineSize * rSize.Height()];
        mBuffer.reset(buffer);
    }
    mPalette = rPal;
    mBitCount = nBitCount;
    mSize = rSize;
    return true;
}

bool SkiaSalBitmap::Create(const SalBitmap& rSalBmp)
{
    return Create(rSalBmp, rSalBmp.GetBitCount());
}

bool SkiaSalBitmap::Create(const SalBitmap& rSalBmp, SalGraphics* pGraphics)
{
    return Create(rSalBmp, pGraphics ? pGraphics->GetBitCount() : rSalBmp.GetBitCount());
}

bool SkiaSalBitmap::Create(const SalBitmap& rSalBmp, sal_uInt16 nNewBitCount)
{
    const SkiaSalBitmap& src = static_cast<const SkiaSalBitmap&>(rSalBmp);
    if (nNewBitCount == src.GetBitCount())
    {
        mBitmap = src.mBitmap; // TODO unshare?
        mPalette = src.mPalette;
        mBitCount = src.mBitCount;
        mSize = src.mSize;
        if (src.mBuffer != nullptr)
        {
            sal_uInt32 dataSize = src.mScanlineSize * src.mSize.Height();
            sal_uInt8* newBuffer = new sal_uInt8[dataSize];
            memcpy(newBuffer, src.mBuffer.get(), dataSize);
            mBuffer.reset(newBuffer);
            mScanlineSize = src.mScanlineSize;
        }
        return true;
    }
    if (!Create(src.mSize, src.mBitCount, src.mPalette))
        return false;
    // TODO copy data
    abort();
    return true;
}

bool SkiaSalBitmap::Create(const css::uno::Reference<css::rendering::XBitmapCanvas>& rBitmapCanvas,
                           Size& rSize, bool bMask)
{
    (void)rBitmapCanvas;
    (void)rSize;
    (void)bMask;
    return false;
}

void SkiaSalBitmap::Destroy()
{
    mBitmap.reset();
    mBuffer.reset();
}

Size SkiaSalBitmap::GetSize() const { return mSize; }

sal_uInt16 SkiaSalBitmap::GetBitCount() const { return mBitCount; }

BitmapBuffer* SkiaSalBitmap::AcquireBuffer(BitmapAccessMode nMode)
{
    (void)nMode; // TODO
    if (mBitmap.drawsNothing() && !mBuffer)
        return nullptr;
    BitmapBuffer* buffer = new BitmapBuffer;
    buffer->mnWidth = mSize.Width();
    buffer->mnHeight = mSize.Height();
    buffer->mnBitCount = mBitCount;
    buffer->maPalette = mPalette;
    if (mBitCount >= 8)
    {
        buffer->mpBits = static_cast<sal_uInt8*>(mBitmap.getPixels());
        buffer->mnScanlineSize = mBitmap.rowBytes();
    }
    else
    {
        buffer->mpBits = mBuffer.get();
        buffer->mnScanlineSize = mScanlineSize;
    }
    switch (mBitCount)
    {
        case 1:
#ifdef OSL_BIGENDIAN
            buffer->mnFormat = ScanlineFormat::N1BitMsbPal | ScanlineFormat::TopDown;
#else
            buffer->mnFormat = ScanlineFormat::N1BitLsbPal | ScanlineFormat::TopDown;
#endif
            break;
        case 4:
#ifdef OSL_BIGENDIAN
            buffer->mnFormat = ScanlineFormat::N4BitMsnPal | ScanlineFormat::TopDown;
#else
            buffer->mnFormat = ScanlineFormat::N4BitLsnPal | ScanlineFormat::TopDown;
#endif
            break;
        case 8:
            buffer->mnFormat = ScanlineFormat::N8BitPal | ScanlineFormat::TopDown;
            break;
        case 24:
            buffer->mnFormat = ScanlineFormat::N24BitTcRgb | ScanlineFormat::TopDown;
            break;
        case 32:
            // TODO are these correct?
            buffer->mnFormat = mBitmap.colorType() == kRGBA_8888_SkColorType
                                   ? ScanlineFormat::N32BitTcBgra
                                   : ScanlineFormat::N32BitTcArgb;
            buffer->mnFormat |= ScanlineFormat::TopDown;
            break;
        default:
            abort();
    }
    return buffer;
}

void SkiaSalBitmap::ReleaseBuffer(BitmapBuffer* pBuffer, BitmapAccessMode nMode)
{
    mPalette = pBuffer->maPalette;
    (void)nMode; // TODO?
    // Are there any more ground movements underneath us ?
    assert(pBuffer->mnWidth == mSize.Width());
    assert(pBuffer->mnHeight == mSize.Height());
    assert(pBuffer->mnBitCount == mBitCount);
    delete pBuffer;
}

bool SkiaSalBitmap::GetSystemData(BitmapSystemData& rData)
{
    (void)rData;
    return false;
}

bool SkiaSalBitmap::ScalingSupported() const { return false; }

bool SkiaSalBitmap::Scale(const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag)
{
    (void)rScaleX;
    (void)rScaleY;
    (void)nScaleFlag;
    return false;
}

bool SkiaSalBitmap::Replace(const Color& rSearchColor, const Color& rReplaceColor, sal_uInt8 nTol)
{
    (void)rSearchColor;
    (void)rReplaceColor;
    (void)nTol;
    return false;
}

bool SkiaSalBitmap::ConvertToGreyscale() { return false; }

#ifdef DBG_UTIL
void SkiaSalBitmap::dump(const char* file) const
{
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(mBitmap);
    sk_sp<SkData> data = image->encodeToData();
    std::ofstream ostream(file, std::ios::binary);
    ostream.write(static_cast<const char*>(data->data()), data->size());
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
