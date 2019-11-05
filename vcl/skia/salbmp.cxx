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

static bool isValidBitCount(sal_uInt16 nBitCount)
{
    return (nBitCount == 1) || (nBitCount == 4) || (nBitCount == 8) || (nBitCount == 24)
           || (nBitCount == 32);
}

SkiaSalBitmap::SkiaSalBitmap(const SkImage& image)
{
    if (Create(Size(image.width(), image.height()), 32, BitmapPalette()))
    {
        SkCanvas canvas(mBitmap);
        // TODO makeNonTextureImage() ?
        canvas.drawImage(&image, 0, 0);
    }
}

bool SkiaSalBitmap::Create(const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal)
{
    mBitmap.reset();
    mBuffer.reset();
    if (!isValidBitCount(nBitCount))
        return false;
    // Skia only supports 8bit gray, 16bit and 32bit formats (e.g. 24bpp is actually stored as 32bpp).
    // But some of our code accessing the bitmap assumes that when it asked for 24bpp, the format
    // really will be 24bpp (e.g. the png loader).
    // TODO what is the performance impact of handling 24bpp ourselves instead of in Skia?
    SkColorType colorType = kUnknown_SkColorType;
    switch (nBitCount)
    {
        case 8:
            if (rPal.IsGreyPalette()) // see GetAlphaSkBitmap()
                colorType = kGray_8_SkColorType;
            break;
        case 32:
            colorType = kN32_SkColorType;
            break;
        default:
            break;
    }
    if (colorType != kUnknown_SkColorType)
    {
        if (!mBitmap.tryAllocPixels(
                SkImageInfo::Make(rSize.Width(), rSize.Height(), colorType, kPremul_SkAlphaType)))
        {
            return false;
        }
#ifdef DBG_UTIL
        // fill with random garbage
        sal_uInt8* buffer = static_cast<sal_uInt8*>(mBitmap.getPixels());
        size_t size = mBitmap.rowBytes() & mBitmap.height();
        for (size_t i = 0; i < size; i++)
            buffer[i] = (i & 0xFF);
#endif
    }
    else
    {
        // Image formats not supported by Skia are stored in a buffer and converted as necessary.
        int bitScanlineWidth;
        if (o3tl::checked_multiply<int>(rSize.Width(), nBitCount, bitScanlineWidth))
        {
            SAL_WARN("vcl.skia", "checked multiply failed");
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
#ifdef DBG_UTIL
            // fill with random garbage
            for (size_t i = 0; i < allocate; i++)
                buffer[i] = (i & 0xFF);
            memcpy(buffer + allocate - sizeof(CANARY), CANARY, sizeof(CANARY));
#endif
        }
        mBuffer.reset(buffer);
    }
    mPalette = rPal;
    mBitCount = nBitCount;
    mSize = rSize;
    SAL_INFO("vcl.skia", "create(" << this << ")");
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
        SAL_INFO("vcl.skia", "create(" << this << "): (" << &src << ")");
        return true;
    }
    if (!Create(src.mSize, src.mBitCount, src.mPalette))
        return false;
    // TODO copy data
    SAL_INFO("vcl.skia", "copy(" << this << "): (" << &src << ")");
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
    SAL_INFO("vcl.skia", "destroy(" << this << ")");
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
            buffer->mnFormat = ScanlineFormat::N8BitPal;
            break;
        case 24:
        {
// Make the RGB/BGR format match the default Skia 32bpp format, to allow
// easy conversion later.
// Use a macro to hide an unreachable code warning.
#define GET_FORMAT                                                                                 \
    (kN32_SkColorType == kBGRA_8888_SkColorType ? ScanlineFormat::N24BitTcBgr                      \
                                                : ScanlineFormat::N24BitTcRgb)
            buffer->mnFormat = GET_FORMAT;
#undef GET_FORMAT
            break;
        }
        case 32:
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

bool SkiaSalBitmap::ConvertToGreyscale()
{
    // Skia can convert color SkBitmap to a greyscale one (draw using SkCanvas),
    // but it uses different coefficients for the color->grey conversion than VCL.
    // So just let VCL do it.
    return false;
}

const SkBitmap& SkiaSalBitmap::GetSkBitmap() const
{
    if (mBuffer && mBitmap.drawsNothing())
    {
        if (mBitCount == 24)
        {
            // Convert 24bpp RGB/BGR to 32bpp RGBA/BGRA.
            std::unique_ptr<sal_uInt8[]> data(new sal_uInt8[mSize.Height() * mSize.Width() * 4]);
            sal_uInt8* dest = data.get();
            for (int y = 0; y < mSize.Height(); ++y)
            {
                const sal_uInt8* src = mBuffer.get() + mScanlineSize * y;
                for (int x = 0; x < mSize.Width(); ++x)
                {
                    *dest++ = *src++;
                    *dest++ = *src++;
                    *dest++ = *src++;
                    *dest++ = 0xff;
                }
            }
            if (!const_cast<SkBitmap&>(mBitmap).installPixels(
                    SkImageInfo::MakeS32(mSize.Width(), mSize.Height(), kOpaque_SkAlphaType),
                    data.release(), mSize.Width() * 4,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
            SAL_INFO("vcl.skia", "skbitmap(" << this << ")");
        }
        else
        {
// Use a macro to hide an unreachable code warning.
#define GET_FORMAT                                                                                 \
    (kN32_SkColorType == kBGRA_8888_SkColorType ? BitConvert::BGRA : BitConvert::RGBA)
            std::unique_ptr<sal_uInt8[]> data
                = convertDataBitCount(mBuffer.get(), mSize.Width(), mSize.Height(), mBitCount,
                                      mScanlineSize, mPalette, GET_FORMAT);
#undef GET_FORMAT
            if (!const_cast<SkBitmap&>(mBitmap).installPixels(
                    SkImageInfo::MakeS32(mSize.Width(), mSize.Height(), kOpaque_SkAlphaType),
                    data.release(), mSize.Width() * 4,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
            SAL_INFO("vcl.skia", "skbitmap(" << this << ")");
        }
    }
    return mBitmap;
}

const SkBitmap& SkiaSalBitmap::GetAlphaSkBitmap() const
{
    if (mAlphaBitmap.drawsNothing())
    {
        if (mBuffer && mBitCount <= 8)
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
            SAL_INFO("vcl.skia", "skalphabitmap(" << this << ")");
        }
        else
        {
            GetSkBitmap(); // make sure we have mBitmap, in case (mBuffer && mBitCount > 8)
            // To make things more interesting, some LO code creates masks as 24bpp,
            // so we first need to convert to 8bit to be able to convert that to 8bit alpha.
            SkBitmap* convertedBitmap = nullptr;
            const SkBitmap* bitmap8 = &mBitmap;
            if (mBitmap.colorType() != kGray_8_SkColorType)
            {
                convertedBitmap = new SkBitmap;
                if (!convertedBitmap->tryAllocPixels(SkImageInfo::Make(
                        mSize.Width(), mSize.Height(), kGray_8_SkColorType, kOpaque_SkAlphaType)))
                    abort();
                SkCanvas canvas(*convertedBitmap);
                SkPaint paint;
                paint.setBlendMode(SkBlendMode::kSrc); // copy and convert depth
                canvas.drawBitmap(mBitmap, 0, 0, &paint);
                bitmap8 = convertedBitmap;
            }
            // Skia uses a bitmap as an alpha channel only if it's set as kAlpha_8_SkColorType.
            // But in SalBitmap::Create() it's not quite clear if the 8-bit image will be used
            // as a mask or as a real bitmap. So mBitmap is always kGray_8_SkColorType for 8bpp
            // and mAlphaBitmap is kAlpha_8_SkColorType that can be used as a mask.
            // Make mAlphaBitmap share mBitmap's data.
            const_cast<SkBitmap&>(mAlphaBitmap)
                .setInfo(bitmap8->info().makeColorType(kAlpha_8_SkColorType), bitmap8->rowBytes());
            const_cast<SkBitmap&>(mAlphaBitmap)
                .setPixelRef(sk_ref_sp(bitmap8->pixelRef()), bitmap8->pixelRefOrigin().x(),
                             bitmap8->pixelRefOrigin().y());
            delete convertedBitmap;
            SAL_INFO("vcl.skia", "skalphabitmap(" << this << ")");
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
