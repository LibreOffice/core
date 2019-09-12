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
#include <boost/smart_ptr/make_shared.hpp>

#include <salgdi.hxx>
#include <salinst.hxx>
#include <scanlinewriter.hxx>
#include <svdata.hxx>

#include <SkCanvas.h>
#include <SkImage.h>
#include <SkPixelRef.h>
#include <SkSurface.h>

#include <skia/utils.hxx>
#include <skia/zone.hxx>

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

SkiaSalBitmap::SkiaSalBitmap(const sk_sp<SkImage>& image)
{
    ResetSkImages();
    mBitmap.reset();
    mBuffer.reset();
    mImage = image;
    mPalette = BitmapPalette();
    mBitCount = 32;
    mSize = Size(image->width(), image->height());
#ifdef DBG_UTIL
    mWriteAccessCount = 0;
#endif
    SAL_INFO("vcl.skia", "bitmapfromimage(" << this << ")");
}

bool SkiaSalBitmap::Create(const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal)
{
    ResetSkImages();
    mBitmap.reset();
    mBuffer.reset();
    if (!isValidBitCount(nBitCount))
        return false;
    mPalette = rPal;
    mBitCount = nBitCount;
    mSize = rSize;
#ifdef DBG_UTIL
    mWriteAccessCount = 0;
#endif
    if (!CreateBitmapData())
    {
        mBitCount = 0;
        mSize = Size();
        mPalette = BitmapPalette();
        return false;
    }
    SAL_INFO("vcl.skia", "create(" << this << ")");
    return true;
}

bool SkiaSalBitmap::CreateBitmapData()
{
    assert(mBitmap.isNull());
    assert(!mBuffer);
    // Skia only supports 8bit gray, 16bit and 32bit formats (e.g. 24bpp is actually stored as 32bpp).
    // But some of our code accessing the bitmap assumes that when it asked for 24bpp, the format
    // really will be 24bpp (e.g. the png loader), so we cannot use SkBitmap to store the data.
    // And even 8bpp is problematic, since Skia does not support palettes and a VCL bitmap can change
    // its grayscale status simply by changing the palette.
    // So basically use Skia only for 32bpp bitmaps.
    // TODO what is the performance impact of handling 24bpp ourselves instead of in Skia?
    SkColorType colorType = kUnknown_SkColorType;
    switch (mBitCount)
    {
        case 32:
            colorType = kN32_SkColorType;
            break;
        default:
            break;
    }
    if (colorType != kUnknown_SkColorType)
    {
    // If vcl::BackendCapabilities::mbSupportsBitmap32 is set,
    // BitmapReadAccess::ImplSetAccessPointers() uses functions that use premultiplied
    // alpha. If not set, it would use functions that would read just RGB, so using
    // premultiplied alpha here would change those values.
#if SKIA_USE_BITMAP32
        assert(ImplGetSVData()->mpDefInst->GetBackendCapabilities()->mbSupportsBitmap32);
        if (!mBitmap.tryAllocPixels(
                SkImageInfo::Make(mSize.Width(), mSize.Height(), colorType, kPremul_SkAlphaType)))
#else
        assert(!ImplGetSVData()->mpDefInst->GetBackendCapabilities()->mbSupportsBitmap32);
        if (!mBitmap.tryAllocPixels(
                SkImageInfo::Make(mSize.Width(), mSize.Height(), colorType, kUnpremul_SkAlphaType)))
#endif
        {
            return false;
        }
#ifdef DBG_UTIL
        // fill with random garbage
        sal_uInt8* buffer = static_cast<sal_uInt8*>(mBitmap.getPixels());
        size_t size = mBitmap.rowBytes() * mBitmap.height();
        for (size_t i = 0; i < size; i++)
            buffer[i] = (i & 0xFF);
#endif
    }
    else
    {
        // Image formats not supported by Skia are stored in a buffer and converted as necessary.
        int bitScanlineWidth;
        if (o3tl::checked_multiply<int>(mSize.Width(), mBitCount, bitScanlineWidth))
        {
            SAL_WARN("vcl.skia", "checked multiply failed");
            return false;
        }
        mScanlineSize = AlignedWidth4Bytes(bitScanlineWidth);
        if (mScanlineSize != 0 && mSize.Height() != 0)
        {
            size_t allocate = mScanlineSize * mSize.Height();
#ifdef DBG_UTIL
            allocate += sizeof(CANARY);
#endif
            mBuffer = boost::make_shared<sal_uInt8[]>(allocate);
#ifdef DBG_UTIL
            // fill with random garbage
            sal_uInt8* buffer = mBuffer.get();
            for (size_t i = 0; i < allocate; i++)
                buffer[i] = (i & 0xFF);
            memcpy(buffer + allocate - sizeof(CANARY), CANARY, sizeof(CANARY));
#endif
        }
    }
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
    mBitmap = src.mBitmap;
    // SkBitmap shares pixels on copy.
    assert(mBitmap.getPixels() == src.mBitmap.getPixels());
    mImage = src.mImage;
    mAlphaImage = src.mAlphaImage;
    mBuffer = src.mBuffer;
    mPalette = src.mPalette;
    mBitCount = src.mBitCount;
    mSize = src.mSize;
    mScanlineSize = src.mScanlineSize;
#ifdef DBG_UTIL
    mWriteAccessCount = 0;
#endif
    if (nNewBitCount != src.GetBitCount())
    {
        // This appears to be unused(?). Implement this just in case, but be lazy
        // about it and rely on EnsureBitmapData() doing the conversion from mImage
        // if needed, even if that may need unnecessary to- and from- SkImage
        // conversion.
        GetSkImage(); // create mImage
        mBitmap.reset();
        mBuffer.reset();
    }
    SAL_INFO("vcl.skia", "create(" << this << "): (" << &src << ")");
    return true;
}

bool SkiaSalBitmap::Create(const css::uno::Reference<css::rendering::XBitmapCanvas>&, Size&, bool)
{
    // TODO?
    return false;
}

void SkiaSalBitmap::Destroy()
{
    SAL_INFO("vcl.skia", "destroy(" << this << ")");
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    ResetSkImages();
    mBitmap.reset();
    mBuffer.reset();
}

Size SkiaSalBitmap::GetSize() const { return mSize; }

sal_uInt16 SkiaSalBitmap::GetBitCount() const { return mBitCount; }

BitmapBuffer* SkiaSalBitmap::AcquireBuffer(BitmapAccessMode nMode)
{
    switch (nMode)
    {
        case BitmapAccessMode::Write:
            EnsureBitmapUniqueData();
            break;
        case BitmapAccessMode::Read:
            EnsureBitmapData();
            break;
        default:
            break;
    }
    if (mBitmap.isNull() && !mBuffer)
        return nullptr;
#ifdef DBG_UTIL
    // BitmapWriteAccess stores also a copy of the palette and it can
    // be modified, so concurrent reading of it might result in inconsistencies.
    assert(mWriteAccessCount == 0 || nMode == BitmapAccessMode::Write);
#endif
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
            assert(false);
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
#ifdef DBG_UTIL
    if (nMode == BitmapAccessMode::Write)
        ++mWriteAccessCount;
#endif
    return buffer;
}

void SkiaSalBitmap::ReleaseBuffer(BitmapBuffer* pBuffer, BitmapAccessMode nMode)
{
    if (nMode == BitmapAccessMode::Write)
    {
#ifdef DBG_UTIL
        assert(mWriteAccessCount > 0);
        --mWriteAccessCount;
#endif
        mPalette = pBuffer->maPalette;
        ResetSkImages();
    }
    // Are there any more ground movements underneath us ?
    assert(pBuffer->mnWidth == mSize.Width());
    assert(pBuffer->mnHeight == mSize.Height());
    assert(pBuffer->mnBitCount == mBitCount);
    verify();
    delete pBuffer;
}

bool SkiaSalBitmap::GetSystemData(BitmapSystemData&)
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    // TODO?
    return false;
}

bool SkiaSalBitmap::ScalingSupported() const { return true; }

bool SkiaSalBitmap::Scale(const double& rScaleX, const double& rScaleY, BmpScaleFlag nScaleFlag)
{
    SkiaZone zone;
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    Size newSize(FRound(mSize.Width() * rScaleX), FRound(mSize.Height() * rScaleY));
    if (mSize == newSize)
        return true;

    SAL_INFO("vcl.skia", "scale(" << this << "): " << mSize << "->" << newSize << ":"
                                  << static_cast<int>(nScaleFlag));

    SkPaint paint;
    switch (nScaleFlag)
    {
        case BmpScaleFlag::Fast:
            paint.setFilterQuality(kNone_SkFilterQuality);
            break;
        case BmpScaleFlag::Default:
            paint.setFilterQuality(kMedium_SkFilterQuality);
            break;
        case BmpScaleFlag::BestQuality:
            paint.setFilterQuality(kHigh_SkFilterQuality);
            break;
        default:
            return false;
    }
    sk_sp<SkSurface> surface = SkiaHelper::createSkSurface(newSize);
    assert(surface);
    paint.setBlendMode(SkBlendMode::kSrc); // draw as is, including alpha
    surface->getCanvas()->drawImageRect(
        GetSkImage(), SkRect::MakeXYWH(0, 0, mSize.Width(), mSize.Height()),
        SkRect::MakeXYWH(0, 0, newSize.Width(), newSize.Height()), &paint);
    // This will get generated from mImage if needed.
    mBitmap.reset();
    mBuffer.reset();
    ResetSkImages();
    mImage = surface->makeImageSnapshot();
    mSize = newSize;
    return true;
}

bool SkiaSalBitmap::Replace(const Color&, const Color&, sal_uInt8)
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    // TODO?
    return false;
}

bool SkiaSalBitmap::ConvertToGreyscale()
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    // Skia can convert color SkBitmap to a greyscale one (draw using SkCanvas),
    // but it uses different coefficients for the color->grey conversion than VCL.
    // So just let VCL do it.
    return false;
}

SkBitmap SkiaSalBitmap::GetAsSkBitmap() const
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    EnsureBitmapData();
    if (!mBitmap.isNull())
        return mBitmap;
    SkiaZone zone;
    SkBitmap bitmap;
    if (mBuffer)
    {
        if (mBitCount == 24)
        {
            // Convert 24bpp RGB/BGR to 32bpp RGBA/BGRA.
            std::unique_ptr<sal_uInt8[]> data(new sal_uInt8[mSize.Height() * mSize.Width() * 4]);
            sal_uInt8* dest = data.get();
            for (long y = 0; y < mSize.Height(); ++y)
            {
                const sal_uInt8* src = mBuffer.get() + mScanlineSize * y;
                for (long x = 0; x < mSize.Width(); ++x)
                {
                    *dest++ = *src++;
                    *dest++ = *src++;
                    *dest++ = *src++;
                    *dest++ = 0xff;
                }
            }
            if (!bitmap.installPixels(
                    SkImageInfo::MakeS32(mSize.Width(), mSize.Height(), kOpaque_SkAlphaType),
                    data.release(), mSize.Width() * 4,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
            bitmap.setImmutable();
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
            if (!bitmap.installPixels(
                    SkImageInfo::MakeS32(mSize.Width(), mSize.Height(), kOpaque_SkAlphaType),
                    data.release(), mSize.Width() * 4,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
            bitmap.setImmutable();
        }
    }
    return bitmap;
}

const sk_sp<SkImage>& SkiaSalBitmap::GetSkImage() const
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    if (mImage)
        return mImage;
    SkiaZone zone;
    sk_sp<SkSurface> surface = SkiaHelper::createSkSurface(mSize);
    assert(surface);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    surface->getCanvas()->drawBitmap(GetAsSkBitmap(), 0, 0, &paint);
    const_cast<sk_sp<SkImage>&>(mImage) = surface->makeImageSnapshot();
    SAL_INFO("vcl.skia", "getskimage(" << this << ")");
    return mImage;
}

const sk_sp<SkImage>& SkiaSalBitmap::GetAlphaSkImage() const
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    if (mAlphaImage)
        return mAlphaImage;
    SkiaZone zone;
    // TODO can we convert directly mImage -> mAlphaImage?
    EnsureBitmapData();
    SkBitmap alphaBitmap;
    if (mBuffer && mBitCount <= 8)
    {
        assert(mBuffer.get());
        verify();
        std::unique_ptr<sal_uInt8[]> data
            = convertDataBitCount(mBuffer.get(), mSize.Width(), mSize.Height(), mBitCount,
                                  mScanlineSize, mPalette, BitConvert::A8);
        if (!alphaBitmap.installPixels(
                SkImageInfo::MakeA8(mSize.Width(), mSize.Height()), data.release(), mSize.Width(),
                [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
            abort();
        alphaBitmap.setImmutable();
    }
    else
    {
        SkBitmap originalBitmap = GetAsSkBitmap();
        // To make things more interesting, some LO code creates masks as 24bpp,
        // so we first need to convert to 8bit to be able to convert that to 8bit alpha.
        SkBitmap* convertedBitmap = nullptr;
        const SkBitmap* bitmap8 = &originalBitmap;
        if (originalBitmap.colorType() != kGray_8_SkColorType)
        {
            convertedBitmap = new SkBitmap;
            if (!convertedBitmap->tryAllocPixels(SkImageInfo::Make(
                    mSize.Width(), mSize.Height(), kGray_8_SkColorType, kOpaque_SkAlphaType)))
                abort();
            SkCanvas canvas(*convertedBitmap);
            SkPaint paint;
            paint.setBlendMode(SkBlendMode::kSrc); // copy and convert depth
            canvas.drawBitmap(originalBitmap, 0, 0, &paint);
            convertedBitmap->setImmutable();
            bitmap8 = convertedBitmap;
        }
        // Skia uses a bitmap as an alpha channel only if it's set as kAlpha_8_SkColorType.
        // But in SalBitmap::Create() it's not quite clear if the 8-bit image will be used
        // as a mask or as a real bitmap. So mBitmap is always kGray_8_SkColorType for 8bpp
        // and alphaBitmap is kAlpha_8_SkColorType that can be used as a mask.
        // Make alphaBitmap share bitmap8's data.
        alphaBitmap.setInfo(
            bitmap8->info().makeColorType(kAlpha_8_SkColorType).makeAlphaType(kPremul_SkAlphaType),
            bitmap8->rowBytes());
        alphaBitmap.setPixelRef(sk_ref_sp(bitmap8->pixelRef()), bitmap8->pixelRefOrigin().x(),
                                bitmap8->pixelRefOrigin().y());
        delete convertedBitmap;
        alphaBitmap.setImmutable();
    }
    sk_sp<SkSurface> surface = SkiaHelper::createSkSurface(mSize, kAlpha_8_SkColorType);
    assert(surface);
    // https://bugs.chromium.org/p/skia/issues/detail?id=9692
    // Raster kAlpha_8_SkColorType surfaces need empty contents for SkBlendMode::kSrc.
    if (!surface->getCanvas()->getGrContext())
        surface->getCanvas()->clear(SkColorSetARGB(0x00, 0x00, 0x00, 0x00));
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    surface->getCanvas()->drawBitmap(alphaBitmap, 0, 0, &paint);
    const_cast<sk_sp<SkImage>&>(mAlphaImage) = surface->makeImageSnapshot();
    SAL_INFO("vcl.skia", "getalphaskbitmap(" << this << ")");
    return mAlphaImage;
}

void SkiaSalBitmap::EnsureBitmapData()
{
    if (!mBitmap.isNull() || mBuffer)
        return;
    if (!mImage)
        return;
    SkiaZone zone;
    if (!CreateBitmapData())
        abort();
    if (!mBitmap.isNull())
    {
        SkCanvas canvas(mBitmap);
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
        canvas.drawImage(mImage, 0, 0, &paint);
        SAL_INFO("vcl.skia", "ensurebitmapdata1(" << this << ")");
    }
    else
    {
        SkBitmap tmpBitmap;
        if (!tmpBitmap.tryAllocPixels(SkImageInfo::Make(mSize.Width(), mSize.Height(),
                                                        kN32_SkColorType, kUnpremul_SkAlphaType)))
            abort();
        SkCanvas canvas(tmpBitmap);
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
        canvas.drawImage(mImage, 0, 0, &paint);
        assert(mBuffer != nullptr);
        if (mBitCount == 24) // non-paletted
        {
            for (long y = 0; y < mSize.Height(); ++y)
            {
                const uint8_t* src = static_cast<uint8_t*>(tmpBitmap.getAddr(0, y));
                sal_uInt8* dest = mBuffer.get() + mScanlineSize * y;
                for (long x = 0; x < mSize.Width(); ++x)
                {
                    *dest++ = *src++;
                    *dest++ = *src++;
                    *dest++ = *src++;
                    ++src; // skip alpha
                }
            }
        }
        else
        {
            std::unique_ptr<vcl::ScanlineWriter> pWriter
                = vcl::ScanlineWriter::Create(mBitCount, mPalette);
            for (long y = 0; y < mSize.Height(); ++y)
            {
                const uint8_t* src = static_cast<uint8_t*>(tmpBitmap.getAddr(0, y));
                sal_uInt8* dest = mBuffer.get() + mScanlineSize * y;
                pWriter->nextLine(dest);
                for (long x = 0; x < mSize.Width(); ++x)
                {
                    sal_uInt8 r = *src++;
                    sal_uInt8 g = *src++;
                    sal_uInt8 b = *src++;
                    ++src; // skip alpha
                    pWriter->writeRGB(r, g, b);
                }
            }
        }
        verify();
        SAL_INFO("vcl.skia", "ensurebitmapdata2(" << this << ")");
    }
}

void SkiaSalBitmap::EnsureBitmapUniqueData()
{
    EnsureBitmapData();
    // TODO thread safety?
    if (mBitmap.pixelRef() && !mBitmap.pixelRef()->unique())
    {
        // SkBitmap copies share pixels, so make a deep copy.
        SkBitmap newBitmap;
        if (!newBitmap.tryAllocPixels(mBitmap.info()))
            abort();
        newBitmap.writePixels(mBitmap.pixmap());
        assert(newBitmap.getPixels() != mBitmap.getPixels());
        mBitmap = newBitmap;
    }
    if (mBuffer.use_count() > 1)
    {
        sal_uInt32 allocate = mScanlineSize * mSize.Height();
#ifdef DBG_UTIL
        assert(memcmp(mBuffer.get() + allocate, CANARY, sizeof(CANARY)) == 0);
        allocate += sizeof(CANARY);
#endif
        boost::shared_ptr<sal_uInt8[]> newBuffer = boost::make_shared<sal_uInt8[]>(allocate);
        memcpy(newBuffer.get(), mBuffer.get(), allocate);
        mBuffer = newBuffer;
    }
}

void SkiaSalBitmap::ResetSkImages()
{
    SkiaZone zone;
    mAlphaImage.reset();
    mImage.reset();
}

#ifdef DBG_UTIL
void SkiaSalBitmap::dump(const char* file) const
{
    sk_sp<SkImage> saveImage = mImage;
    sk_sp<SkImage> saveAlphaImage = mAlphaImage;
    SkBitmap saveBitmap = mBitmap;
    bool resetBuffer = !mBuffer;
    int saveWriteAccessCount = mWriteAccessCount;
    SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
    // avoid possible assert
    thisPtr->mWriteAccessCount = 0;
    SkiaHelper::dump(GetSkImage(), file);
    // restore old state, so that debugging doesn't affect it
    thisPtr->mBitmap = saveBitmap;
    if (resetBuffer)
        thisPtr->mBuffer.reset();
    thisPtr->mImage = saveImage;
    thisPtr->mAlphaImage = saveAlphaImage;
    thisPtr->mWriteAccessCount = saveWriteAccessCount;
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
