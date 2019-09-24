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
#include <SkPixelRef.h>

#ifdef DBG_UTIL
#include <fstream>
#define CANARY "skia-canary"
#endif

SkiaSalBitmap::SkiaSalBitmap() {}

SkiaSalBitmap::~SkiaSalBitmap() {}

static SkColorType getSkColorType(int bitCount)
{
    switch (bitCount)
    {
        case 8:
            return kGray_8_SkColorType; // see GetAlphaSkBitmap()
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
    // Skia does not support paletted images, so convert only types Skia supports.
    if (nBitCount > 8 || (nBitCount == 8 && (!rPal || rPal.IsGreyPalette())))
    {
        if (!mBitmap.tryAllocPixels(SkImageInfo::Make(
                rSize.Width(), rSize.Height(), getSkColorType(nBitCount), kPremul_SkAlphaType)))
        {
            return false;
        }
    }
    else
    {
        // Paletted images are stored in a buffer and converted as necessary.
        int bitScanlineWidth;
        if (o3tl::checked_multiply<int>(rSize.Width(), nBitCount, bitScanlineWidth))
        {
            SAL_WARN("vcl.gdi", "checked multiply failed");
            return false;
        }
        mScanlineSize = AlignedWidth4Bytes(bitScanlineWidth);
        sal_uInt8* buffer = nullptr;
        if (mScanlineSize != 0 && rSize.Height() != 0)
        {
            size_t allocate = mScanlineSize * rSize.Height();
#ifdef DBG_UTIL
            allocate += sizeof(CANARY);
#endif
            buffer = new sal_uInt8[allocate];
#if OSL_DEBUG_LEVEL > 0
            memcpy(buffer + allocate - sizeof(CANARY), CANARY, sizeof(CANARY));
#endif
        }
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
            sal_uInt32 allocate = src.mScanlineSize * src.mSize.Height();
#ifdef DBG_UTIL
            assert(memcmp(src.mBuffer.get() + allocate, CANARY, sizeof(CANARY)) == 0);
            allocate += sizeof(CANARY);
#endif
            sal_uInt8* newBuffer = new sal_uInt8[allocate];
            memcpy(newBuffer, src.mBuffer.get(), allocate);
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
    if (mBuffer)
    {
        buffer->mpBits = mBuffer.get();
        buffer->mnScanlineSize = mScanlineSize;
    }
    else
    {
        buffer->mpBits = static_cast<sal_uInt8*>(mBitmap.getPixels());
        buffer->mnScanlineSize = mBitmap.rowBytes();
    }
    switch (mBitCount)
    {
        case 1:
            buffer->mnFormat = ScanlineFormat::N1BitMsbPal;
            break;
        case 4:
            buffer->mnFormat = ScanlineFormat::N4BitMsnPal;
            break;
        case 8:
            // TODO or always N8BitPal?
            //            buffer->mnFormat = !mPalette ? ScanlineFormat::N8BitTcMask : ScanlineFormat::N8BitPal;
            buffer->mnFormat = ScanlineFormat::N8BitPal;
            break;
        case 24:
            buffer->mnFormat = ScanlineFormat::N24BitTcRgb;
            break;
        case 32:
            // TODO are these correct?
            buffer->mnFormat = mBitmap.colorType() == kRGBA_8888_SkColorType
                                   ? ScanlineFormat::N32BitTcRgba
                                   : ScanlineFormat::N32BitTcBgra;
            break;
        default:
            abort();
    }
    buffer->mnFormat |= ScanlineFormat::TopDown;
    return buffer;
}

void SkiaSalBitmap::ReleaseBuffer(BitmapBuffer* pBuffer, BitmapAccessMode nMode)
{
    if (nMode == BitmapAccessMode::Write) // TODO something more?
    {
        mPalette = pBuffer->maPalette;
        ResetCachedBitmap();
    }
    // Are there any more ground movements underneath us ?
    assert(pBuffer->mnWidth == mSize.Width());
    assert(pBuffer->mnHeight == mSize.Height());
    assert(pBuffer->mnBitCount == mBitCount);
    verify();
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

const SkBitmap& SkiaSalBitmap::GetSkBitmap() const
{
    if (mBuffer && mBitmap.drawsNothing())
    {
        std::unique_ptr<sal_uInt8[]> data = convertDataBitCount(
            mBuffer.get(), mSize.Width(), mSize.Height(), mBitCount, mScanlineSize, mPalette,
            kN32_SkColorType == kBGRA_8888_SkColorType ? BitConvert::BGRA
                                                       : BitConvert::RGBA); // TODO
        if (!const_cast<SkBitmap&>(mBitmap).installPixels(
                SkImageInfo::MakeS32(mSize.Width(), mSize.Height(), kOpaque_SkAlphaType),
                data.release(), mSize.Width() * 4,
                [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
            abort();
    }
    return mBitmap;
}

const SkBitmap& SkiaSalBitmap::GetAlphaSkBitmap() const
{
    assert(mBitCount <= 8);
    if (mAlphaBitmap.drawsNothing())
    {
        if (mBuffer)
        {
            assert(mBuffer.get());
            verify();
            std::unique_ptr<sal_uInt8[]> data
                = convertDataBitCount(mBuffer.get(), mSize.Width(), mSize.Height(), mBitCount,
                                      mScanlineSize, mPalette, BitConvert::A8);
            if (!const_cast<SkBitmap&>(mAlphaBitmap)
                     .installPixels(
                         SkImageInfo::MakeA8(mSize.Width(), mSize.Height()), data.release(),
                         mSize.Width(),
                         [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); },
                         nullptr))
                abort();
        }
        else
        {
            assert(mBitmap.colorType() == kGray_8_SkColorType);
            // Skia uses a bitmap as an alpha channel only if it's set as kAlpha_8_SkColorType.
            // But in SalBitmap::Create() it's not quite clear if the 8-bit image will be used
            // as a mask or as a real bitmap. So mBitmap is always kGray_8_SkColorType
            // and mAlphaBitmap is kAlpha_8_SkColorType that can be used as a mask.
            // Make mAlphaBitmap share mBitmap's data.
            const_cast<SkBitmap&>(mAlphaBitmap)
                .setInfo(mBitmap.info().makeColorType(kAlpha_8_SkColorType), mBitmap.rowBytes());
            const_cast<SkBitmap&>(mAlphaBitmap)
                .setPixelRef(sk_ref_sp(mBitmap.pixelRef()), mBitmap.pixelRefOrigin().x(),
                             mBitmap.pixelRefOrigin().y());
            return mAlphaBitmap;
        }
    }
    return mAlphaBitmap;
}

// Reset the cached bitmap allocated in GetSkBitmap().
void SkiaSalBitmap::ResetCachedBitmap()
{
    mAlphaBitmap.reset();
    if (mBuffer)
        mBitmap.reset();
}

#ifdef DBG_UTIL
void SkiaSalBitmap::dump(const char* file) const
{
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(GetSkBitmap());
    sk_sp<SkData> data = image->encodeToData();
    std::ofstream ostream(file, std::ios::binary);
    ostream.write(static_cast<const char*>(data->data()), data->size());
}

void SkiaSalBitmap::verify() const
{
    if (!mBuffer)
        return;
    size_t canary = mScanlineSize * mSize.Height();
    assert(memcmp(mBuffer.get() + canary, CANARY, sizeof(CANARY)) == 0);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
