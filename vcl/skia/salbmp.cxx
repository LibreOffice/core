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
#include <SkSwizzle.h>
#include <SkColorFilter.h>
#include <SkColorMatrix.h>

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
    ResetCachedData();
    mBuffer.reset();
    mImage = image;
    mPalette = BitmapPalette();
    mBitCount = 32;
    mSize = mPixelsSize = Size(image->width(), image->height());
#ifdef DBG_UTIL
    mWriteAccessCount = 0;
#endif
    SAL_INFO("vcl.skia.trace", "bitmapfromimage(" << this << ")");
}

bool SkiaSalBitmap::Create(const Size& rSize, sal_uInt16 nBitCount, const BitmapPalette& rPal)
{
    ResetCachedData();
    mBuffer.reset();
    if (!isValidBitCount(nBitCount))
        return false;
    mPalette = rPal;
    mBitCount = nBitCount;
    mSize = mPixelsSize = rSize;
#ifdef DBG_UTIL
    mWriteAccessCount = 0;
#endif
    if (!CreateBitmapData())
    {
        mBitCount = 0;
        mSize = mPixelsSize = Size();
        mPalette = BitmapPalette();
        return false;
    }
    SAL_INFO("vcl.skia.trace", "create(" << this << ")");
    return true;
}

bool SkiaSalBitmap::CreateBitmapData()
{
    assert(!mBuffer);
    // The pixels could be stored in SkBitmap, but Skia only supports 8bit gray, 16bit and 32bit formats
    // (e.g. 24bpp is actually stored as 32bpp). But some of our code accessing the bitmap assumes that
    // when it asked for 24bpp, the format really will be 24bpp (e.g. the png loader), so we cannot use
    // SkBitmap to store the data. And even 8bpp is problematic, since Skia does not support palettes
    // and a VCL bitmap can change its grayscale status simply by changing the palette.
    // Moreover creating SkImage from SkBitmap does a data copy unless the bitmap is immutable.
    // So just always store pixels in our buffer and convert as necessary.
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
        mBuffer = boost::make_shared_noinit<sal_uInt8[]>(allocate);
#ifdef DBG_UTIL
        // fill with random garbage
        sal_uInt8* buffer = mBuffer.get();
        for (size_t i = 0; i < allocate; i++)
            buffer[i] = (i & 0xFF);
        memcpy(buffer + allocate - sizeof(CANARY), CANARY, sizeof(CANARY));
#endif
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
    mImage = src.mImage;
    mAlphaImage = src.mAlphaImage;
    mBuffer = src.mBuffer;
    mPalette = src.mPalette;
    mBitCount = src.mBitCount;
    mSize = src.mSize;
    mPixelsSize = src.mPixelsSize;
    mScanlineSize = src.mScanlineSize;
    mScaleQuality = src.mScaleQuality;
#ifdef DBG_UTIL
    mWriteAccessCount = 0;
#endif
    if (nNewBitCount != src.GetBitCount())
    {
        // This appears to be unused(?). Implement this just in case, but be lazy
        // about it and rely on EnsureBitmapData() doing the conversion from mImage
        // if needed, even if that may need unnecessary to- and from- SkImage
        // conversion.
        ResetToSkImage(GetSkImage());
    }
    SAL_INFO("vcl.skia.trace", "create(" << this << "): (" << &src << ")");
    return true;
}

bool SkiaSalBitmap::Create(const css::uno::Reference<css::rendering::XBitmapCanvas>&, Size&, bool)
{
    // TODO?
    return false;
}

void SkiaSalBitmap::Destroy()
{
    SAL_INFO("vcl.skia.trace", "destroy(" << this << ")");
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    ResetCachedData();
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
            if (!mBuffer)
                return nullptr;
            break;
        case BitmapAccessMode::Read:
            EnsureBitmapData();
            if (!mBuffer)
                return nullptr;
            break;
        case BitmapAccessMode::Info:
            break;
    }
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
    buffer->mpBits = mBuffer.get();
    buffer->mnScanlineSize = mScanlineSize;
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
#define GET_FORMAT                                                                                 \
    (kN32_SkColorType == kBGRA_8888_SkColorType ? ScanlineFormat::N32BitTcBgra                     \
                                                : ScanlineFormat::N32BitTcRgba)
            buffer->mnFormat = GET_FORMAT;
#undef GET_FORMAT
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
        ResetCachedData();
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

    SAL_INFO("vcl.skia.trace", "scale(" << this << "): " << mSize << "/" << mBitCount << "->"
                                        << newSize << ":" << static_cast<int>(nScaleFlag));

    // The idea here is that the actual scaling will be delayed until the result
    // is actually needed. Usually the scaled bitmap will be drawn somewhere,
    // so delaying will mean the scaling can be done as a part of GetSkImage().
    // That means it can be GPU-accelerated, while done here directly it would need
    // to be either done by CPU, or with the CPU->GPU->CPU roundtrip required
    // by GPU-accelerated scaling.
    // Pending scaling is detected by 'mSize != mPixelsSize'.
    SkFilterQuality currentQuality;
    switch (nScaleFlag)
    {
        case BmpScaleFlag::Fast:
            currentQuality = kNone_SkFilterQuality;
            break;
        case BmpScaleFlag::Default:
            currentQuality = kMedium_SkFilterQuality;
            break;
        case BmpScaleFlag::BestQuality:
            currentQuality = kHigh_SkFilterQuality;
            break;
        default:
            SAL_INFO("vcl.skia.trace", "scale(" << this << "): unsupported scale algorithm");
            return false;
    }
    if (mBitCount < 24 && !mPalette.IsGreyPalette8Bit())
    {
        // Scaling can introduce additional colors not present in the original
        // bitmap (e.g. when smoothing). If the bitmap is indexed (has non-trivial palette),
        // this would break the bitmap, because the actual scaling is done only somewhen later.
        // Linear 8bit palette (grey) is ok, since there we use directly the values as colors.
        SAL_INFO("vcl.skia.trace", "scale(" << this << "): indexed bitmap");
        return false;
    }
    // if there is already one scale() pending, use the lowest quality of all requested
    static_assert(kMedium_SkFilterQuality < kHigh_SkFilterQuality);
    mScaleQuality = std::min(mScaleQuality, currentQuality);
    // scaling will be actually done on-demand when needed, the need will be recognized
    // by mSize != mPixelsSize
    mSize = newSize;
    // Do not reset cached data if mImage is possibly the only data we have.
    if (mBuffer)
        ResetCachedData();
    // The rest will be handled when the scaled bitmap is actually needed,
    // such as in EnsureBitmapData() or GetSkImage().
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
    // Normally this would need to convert contents of mBuffer for all possible formats,
    // so just let the VCL algorithm do it.
    // Avoid the costly SkImage->buffer->SkImage conversion.
    if (!mBuffer && mImage)
    {
        if (mBitCount == 8 && mPalette.IsGreyPalette8Bit())
            return true;
        sk_sp<SkSurface> surface = SkiaHelper::createSkSurface(mPixelsSize);
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
        // VCL uses different coefficients for conversion to gray than Skia, so use the VCL
        // values from Bitmap::ImplMakeGreyscales(). Do not use kGray_8_SkColorType,
        // Skia would use its gray conversion formula.
        // NOTE: The matrix is 4x5 organized as columns (i.e. each line is a column, not a row).
        constexpr SkColorMatrix toGray(77 / 256.0, 151 / 256.0, 28 / 256.0, 0, 0, // R column
                                       77 / 256.0, 151 / 256.0, 28 / 256.0, 0, 0, // G column
                                       77 / 256.0, 151 / 256.0, 28 / 256.0, 0, 0, // B column
                                       0, 0, 0, 1, 0); // don't modify alpha
        paint.setColorFilter(SkColorFilters::Matrix(toGray));
        surface->getCanvas()->drawImage(mImage, 0, 0, &paint);
        mBitCount = 8;
        mPalette = Bitmap::GetGreyPalette(256);
        ResetToSkImage(surface->makeImageSnapshot());
        SAL_INFO("vcl.skia.trace", "converttogreyscale(" << this << ")");
        return true;
    }
    return false;
}

bool SkiaSalBitmap::InterpretAs8Bit()
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    if (mBitCount == 8 && mPalette.IsGreyPalette8Bit())
        return true;
    // This is usually used by AlphaMask, the point is just to treat
    // the content as an alpha channel. This is often used
    // by the horrible separate-alpha-outdev hack, where the bitmap comes
    // from SkiaSalGraphicsImpl::GetBitmap(), so only mImage is set,
    // and that is followed by a later call to GetAlphaSkImage().
    // Avoid the costly SkImage->buffer->SkImage conversion and simply
    // just treat the SkImage as being for 8bit bitmap. EnsureBitmapData()
    // will do the conversion if needed, but the normal case will be
    // GetAlphaSkImage() creating kAlpha_8_SkColorType SkImage from it.
    if (!mBuffer && mImage)
    {
        mBitCount = 8;
        mPalette = Bitmap::GetGreyPalette(256);
        ResetToSkImage(mImage); // keep mImage, it will be interpreted as 8bit if needed
        SAL_INFO("vcl.skia.trace", "interpretas8bit(" << this << ")");
        return true;
    }
    return false;
}

SkBitmap SkiaSalBitmap::GetAsSkBitmap() const
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    EnsureBitmapData();
    assert(mSize == mPixelsSize); // data has already been scaled if needed
    SkiaZone zone;
    SkBitmap bitmap;
    if (mBuffer)
    {
        if (mBitCount == 32)
        {
            // Make a copy, the bitmap should be immutable (otherwise converting it
            // to SkImage will make a copy anyway).
            const size_t bytes = mPixelsSize.Height() * mScanlineSize;
            std::unique_ptr<sal_uInt8[]> data(new sal_uInt8[bytes]);
            memcpy(data.get(), mBuffer.get(), bytes);
#if SKIA_USE_BITMAP32
            SkAlphaType alphaType = kPremul_SkAlphaType;
#else
            SkAlphaType alphaType = kUnpremul_SkAlphaType;
#endif
            if (!bitmap.installPixels(
                    SkImageInfo::MakeS32(mPixelsSize.Width(), mPixelsSize.Height(), alphaType),
                    data.release(), mPixelsSize.Width() * 4,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
            bitmap.setImmutable();
        }
        else if (mBitCount == 24)
        {
            // Convert 24bpp RGB/BGR to 32bpp RGBA/BGRA.
            std::unique_ptr<uint32_t[]> data(
                new uint32_t[mPixelsSize.Height() * mPixelsSize.Width()]);
            uint32_t* dest = data.get();
            for (long y = 0; y < mPixelsSize.Height(); ++y)
            {
                const sal_uInt8* src = mBuffer.get() + mScanlineSize * y;
                // This also works as BGR to BGRA (the function extends 3 bytes to 4
                // by adding 0xFF alpha, so position of B and R doesn't matter).
                SkExtendRGBToRGBA(dest, src, mPixelsSize.Width());
                dest += mPixelsSize.Width();
            }
            if (!bitmap.installPixels(
                    SkImageInfo::MakeS32(mPixelsSize.Width(), mPixelsSize.Height(),
                                         kOpaque_SkAlphaType),
                    data.release(), mPixelsSize.Width() * 4,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
            bitmap.setImmutable();
        }
        // Skia has a format for 8bit grayscale SkBitmap, but it seems to cause a problem
        // with our PNG loader (tdf#121120), so convert it to RGBA below as well.
        else
        {
// Use a macro to hide an unreachable code warning.
#define GET_FORMAT                                                                                 \
    (kN32_SkColorType == kBGRA_8888_SkColorType ? BitConvert::BGRA : BitConvert::RGBA)
            std::unique_ptr<sal_uInt8[]> data
                = convertDataBitCount(mBuffer.get(), mPixelsSize.Width(), mPixelsSize.Height(),
                                      mBitCount, mScanlineSize, mPalette, GET_FORMAT);
#undef GET_FORMAT
            if (!bitmap.installPixels(
                    SkImageInfo::MakeS32(mPixelsSize.Width(), mPixelsSize.Height(),
                                         kOpaque_SkAlphaType),
                    data.release(), mPixelsSize.Width() * 4,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
            bitmap.setImmutable();
        }
    }
    return bitmap;
}

<<<<<<< HEAD   (cb8bf2 tdf#133564: Text coloring of buttons within non key windows )
=======
static SkColor toSkColor(Color color)
{
    return SkColorSetARGB(255 - color.GetTransparency(), color.GetRed(), color.GetGreen(),
                          color.GetBlue());
}

// If mEraseColor is set, this is the color to use when the bitmap is used as alpha bitmap.
// E.g. COL_BLACK actually means fully opaque and COL_WHITE means fully transparent.
// This is because the alpha value is set as the color itself, not the alpha of the color.
// Additionally VCL actually uses transparency and not opacity, so we should use "255 - value",
// but we account for this by doing SkBlendMode::kDstOut when using alpha images (which
// basically does another "255 - alpha"), so do not do it here.
static SkColor fromEraseColorToAlphaImageColor(Color color)
{
    return SkColorSetARGB(color.GetBlue(), 0, 0, 0);
}

>>>>>>> CHANGE (2e3111 handle properly cleared bitmap used as alpha for Skia (tdf#1)
const sk_sp<SkImage>& SkiaSalBitmap::GetSkImage() const
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    if (mPixelsSize != mSize && !mImage
        && SkiaHelper::renderMethodToUse() != SkiaHelper::RenderRaster)
    {
        // The bitmap has a pending scaling, but no image. This function would below call GetAsSkBitmap(),
        // which would do CPU-based pixel scaling, and then it would get converted to an image.
        // Be more efficient, first convert to an image and then the block below will scale on the GPU.
        SAL_INFO("vcl.skia.trace", "getskimage(" << this << "): shortcut image scaling "
                                                 << mPixelsSize << "->" << mSize);
        SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
        Size savedSize = mSize;
        thisPtr->mSize = mPixelsSize; // block scaling
        SkiaZone zone;
        sk_sp<SkImage> image = SkiaHelper::createSkImage(GetAsSkBitmap());
        assert(image);
        thisPtr->mSize = savedSize;
        thisPtr->ResetToSkImage(image);
    }
    if (mImage)
    {
        if (mImage->width() != mSize.Width() || mImage->height() != mSize.Height())
        {
            assert(!mBuffer); // This code should be only called if only mImage holds data.
            SkiaZone zone;
            sk_sp<SkSurface> surface = SkiaHelper::createSkSurface(mSize);
            assert(surface);
            SkPaint paint;
            paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
            paint.setFilterQuality(mScaleQuality);
            surface->getCanvas()->drawImageRect(
                mImage, SkRect::MakeWH(mImage->width(), mImage->height()),
                SkRect::MakeWH(mSize.Width(), mSize.Height()), &paint);
            SAL_INFO("vcl.skia.trace", "getskimage(" << this << "): image scaled "
                                                     << Size(mImage->width(), mImage->height())
                                                     << "->" << mSize << ":"
                                                     << static_cast<int>(mScaleQuality));
            SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
            thisPtr->mImage = surface->makeImageSnapshot();
        }
        return mImage;
    }
    SkiaZone zone;
    sk_sp<SkImage> image = SkiaHelper::createSkImage(GetAsSkBitmap());
    assert(image);
    const_cast<sk_sp<SkImage>&>(mImage) = image;
    SAL_INFO("vcl.skia.trace", "getskimage(" << this << ")");
    return mImage;
}

const sk_sp<SkImage>& SkiaSalBitmap::GetAlphaSkImage() const
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
<<<<<<< HEAD   (cb8bf2 tdf#133564: Text coloring of buttons within non key windows )
=======
    if (mEraseColorSet)
    {
        SkiaZone zone;
        sk_sp<SkSurface> surface = SkiaHelper::createSkSurface(mSize, kAlpha_8_SkColorType);
        assert(surface);
        surface->getCanvas()->clear(fromEraseColorToAlphaImageColor(mEraseColor));
        SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
        thisPtr->mAlphaImage = surface->makeImageSnapshot();
        SAL_INFO("vcl.skia.trace",
                 "getalphaskimage(" << this << ") from erase color " << mEraseColor);
        return mAlphaImage;
    }
>>>>>>> CHANGE (2e3111 handle properly cleared bitmap used as alpha for Skia (tdf#1)
    if (mAlphaImage)
    {
        assert(mSize == mPixelsSize); // data has already been scaled if needed
        return mAlphaImage;
    }
    if (mImage)
    {
        SkiaZone zone;
        sk_sp<SkSurface> surface = SkiaHelper::createSkSurface(mSize, kAlpha_8_SkColorType);
        assert(surface);
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
        // Move the R channel value to the alpha channel. This seems to be the only
        // way to reinterpret data in SkImage as an alpha SkImage without accessing the pixels.
        // NOTE: The matrix is 4x5 organized as columns (i.e. each line is a column, not a row).
        constexpr SkColorMatrix redToAlpha(0, 0, 0, 0, 0, // R column
                                           0, 0, 0, 0, 0, // G column
                                           0, 0, 0, 0, 0, // B column
                                           1, 0, 0, 0, 0); // A column
        paint.setColorFilter(SkColorFilters::Matrix(redToAlpha));
        bool scaling = mImage->width() != mSize.Width() || mImage->height() != mSize.Height();
        if (scaling)
        {
            assert(!mBuffer); // This code should be only called if only mImage holds data.
            paint.setFilterQuality(mScaleQuality);
        }
        surface->getCanvas()->drawImageRect(mImage,
                                            SkRect::MakeWH(mImage->width(), mImage->height()),
                                            SkRect::MakeWH(mSize.Width(), mSize.Height()), &paint);
        if (scaling)
            SAL_INFO("vcl.skia.trace", "getalphaskimage(" << this << "): image scaled "
                                                          << Size(mImage->width(), mImage->height())
                                                          << "->" << mSize << ":"
                                                          << static_cast<int>(mScaleQuality));
        else
            SAL_INFO("vcl.skia.trace", "getalphaskimage(" << this << ") from image");
        SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
        thisPtr->mAlphaImage = surface->makeImageSnapshot();
        return mAlphaImage;
    }
    SkiaZone zone;
    EnsureBitmapData();
    assert(mSize == mPixelsSize); // data has already been scaled if needed
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
        sk_sp<SkImage> image = SkiaHelper::createSkImage(alphaBitmap);
        assert(image);
        const_cast<sk_sp<SkImage>&>(mAlphaImage) = image;
    }
    else
    {
        sk_sp<SkSurface> surface = SkiaHelper::createSkSurface(mSize, kAlpha_8_SkColorType);
        assert(surface);
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
        // Move the R channel value to the alpha channel. This seems to be the only
        // way to reinterpret data in SkImage as an alpha SkImage without accessing the pixels.
        // NOTE: The matrix is 4x5 organized as columns (i.e. each line is a column, not a row).
        constexpr SkColorMatrix redToAlpha(0, 0, 0, 0, 0, // R column
                                           0, 0, 0, 0, 0, // G column
                                           0, 0, 0, 0, 0, // B column
                                           1, 0, 0, 0, 0); // A column
        paint.setColorFilter(SkColorFilters::Matrix(redToAlpha));
        surface->getCanvas()->drawBitmap(GetAsSkBitmap(), 0, 0, &paint);
        SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
        thisPtr->mAlphaImage = surface->makeImageSnapshot();
    }
    SAL_INFO("vcl.skia.trace", "getalphaskimage(" << this << ")");
    return mAlphaImage;
}

<<<<<<< HEAD   (cb8bf2 tdf#133564: Text coloring of buttons within non key windows )
=======
// If the bitmap is to be erased, SkShader with the color set is more efficient
// than creating an image filled with the color.
bool SkiaSalBitmap::PreferSkShader() const { return mEraseColorSet; }

sk_sp<SkShader> SkiaSalBitmap::GetSkShader() const
{
    if (mEraseColorSet)
        return SkShaders::Color(toSkColor(mEraseColor));
    return GetSkImage()->makeShader();
}

sk_sp<SkShader> SkiaSalBitmap::GetAlphaSkShader() const
{
    if (mEraseColorSet)
        return SkShaders::Color(fromEraseColorToAlphaImageColor(mEraseColor));
    return GetAlphaSkImage()->makeShader();
}

void SkiaSalBitmap::EraseInternal()
{
    if (mPixelsSize.IsEmpty())
        return;
    BitmapBuffer* bitmapBuffer = AcquireBuffer(BitmapAccessMode::Write);
    if (bitmapBuffer == nullptr)
        abort();
    Color fastColor = mEraseColor;
    if (!!mPalette)
        fastColor = mPalette.GetBestIndex(fastColor);
    if (!ImplFastEraseBitmap(*bitmapBuffer, fastColor))
    {
        FncSetPixel setPixel = BitmapReadAccess::SetPixelFunction(bitmapBuffer->mnFormat);
        assert(bitmapBuffer->mnFormat & ScanlineFormat::TopDown);
        // Set first scanline, copy to others.
        Scanline scanline = bitmapBuffer->mpBits;
        for (long x = 0; x < bitmapBuffer->mnWidth; ++x)
            setPixel(scanline, x, mEraseColor, bitmapBuffer->maColorMask);
        for (long y = 1; y < bitmapBuffer->mnHeight; ++y)
            memcpy(scanline + y * bitmapBuffer->mnScanlineSize, scanline,
                   bitmapBuffer->mnScanlineSize);
    }
    ReleaseBuffer(bitmapBuffer, BitmapAccessMode::Write);
}

>>>>>>> CHANGE (2e3111 handle properly cleared bitmap used as alpha for Skia (tdf#1)
void SkiaSalBitmap::EnsureBitmapData()
{
    if (mBuffer)
    {
        if (mSize == mPixelsSize)
            return;
        // Pending scaling. Create raster SkImage from the bitmap data
        // at the pixel size and then the code below will scale at the correct
        // bpp from the image.
        SAL_INFO("vcl.skia.trace", "ensurebitmapdata(" << this << "): pixels to be scaled "
                                                       << mPixelsSize << "->" << mSize << ":"
                                                       << static_cast<int>(mScaleQuality));
        Size savedSize = mSize;
        mSize = mPixelsSize;
        ResetToSkImage(SkImage::MakeFromBitmap(GetAsSkBitmap()));
        mSize = savedSize;
    }
    // Try to fill mBuffer from mImage.
    if (!mImage)
        return;
    SkiaZone zone;
    if (!CreateBitmapData())
        abort();
    SkAlphaType alphaType = kUnpremul_SkAlphaType;
#if SKIA_USE_BITMAP32
    if (mBitCount == 32)
        alphaType = kPremul_SkAlphaType;
#endif
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixels(SkImageInfo::MakeS32(mSize.Width(), mSize.Height(), alphaType)))
        abort();
    SkCanvas canvas(bitmap);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
    if (mSize != mPixelsSize) // pending scaling?
    {
        paint.setFilterQuality(mScaleQuality);
        canvas.drawImageRect(mImage,
                             SkRect::MakeWH(mPixelsSize.getWidth(), mPixelsSize.getHeight()),
                             SkRect::MakeWH(mSize.getWidth(), mSize.getHeight()), &paint);
        SAL_INFO("vcl.skia.trace", "ensurebitmapdata(" << this << "): image scaled " << mPixelsSize
                                                       << "->" << mSize << ":"
                                                       << static_cast<int>(mScaleQuality));
        mPixelsSize = mSize;
        mScaleQuality = kHigh_SkFilterQuality;
        // Information about the pending scaling has been discarded, so make sure we do not
        // keep around any cached images that would still need scaling.
        ResetCachedDataBySize();
    }
    else
        canvas.drawImage(mImage, 0, 0, &paint);
    canvas.flush();
    bitmap.setImmutable();
    assert(mBuffer != nullptr);
    if (mBitCount == 32)
    {
        for (long y = 0; y < mSize.Height(); ++y)
        {
            const uint8_t* src = static_cast<uint8_t*>(bitmap.getAddr(0, y));
            sal_uInt8* dest = mBuffer.get() + mScanlineSize * y;
            memcpy(dest, src, mScanlineSize);
        }
    }
    else if (mBitCount == 24) // non-paletted
    {
        for (long y = 0; y < mSize.Height(); ++y)
        {
            const uint8_t* src = static_cast<uint8_t*>(bitmap.getAddr(0, y));
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
    else if (mBitCount == 8 && mPalette.IsGreyPalette8Bit())
    {
        for (long y = 0; y < mSize.Height(); ++y)
        {
            const uint8_t* src = static_cast<uint8_t*>(bitmap.getAddr(0, y));
            sal_uInt8* dest = mBuffer.get() + mScanlineSize * y;
            // no actual data conversion, use one color channel as the gray value
            for (long x = 0; x < mSize.Width(); ++x)
                dest[x] = src[x * 4];
        }
    }
    else
    {
        std::unique_ptr<vcl::ScanlineWriter> pWriter
            = vcl::ScanlineWriter::Create(mBitCount, mPalette);
        for (long y = 0; y < mSize.Height(); ++y)
        {
            const uint8_t* src = static_cast<uint8_t*>(bitmap.getAddr(0, y));
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
    SAL_INFO("vcl.skia.trace", "ensurebitmapdata(" << this << ")");
}

void SkiaSalBitmap::EnsureBitmapUniqueData()
{
    EnsureBitmapData();
    if (mBuffer.use_count() > 1)
    {
        sal_uInt32 allocate = mScanlineSize * mSize.Height();
#ifdef DBG_UTIL
        assert(memcmp(mBuffer.get() + allocate, CANARY, sizeof(CANARY)) == 0);
        allocate += sizeof(CANARY);
#endif
        boost::shared_ptr<sal_uInt8[]> newBuffer = boost::make_shared_noinit<sal_uInt8[]>(allocate);
        memcpy(newBuffer.get(), mBuffer.get(), allocate);
        mBuffer = newBuffer;
    }
}

void SkiaSalBitmap::ResetCachedData()
{
    SkiaZone zone;
    // There may be a case when only mImage is set and CreatBitmapData() will create
    // mBuffer from it if needed, in that case ResetToSkImage() should be used.
    assert(mBuffer.get() || !mImage);
    mImage.reset();
    mAlphaImage.reset();
}

void SkiaSalBitmap::ResetToSkImage(sk_sp<SkImage> image)
{
    SkiaZone zone;
    mBuffer.reset();
    mImage = image;
    mAlphaImage.reset();
}

void SkiaSalBitmap::ResetCachedDataBySize()
{
    SkiaZone zone;
    assert(mSize == mPixelsSize);
    if (mImage && (mImage->width() != mSize.getWidth() || mImage->height() != mSize.getHeight()))
        mImage.reset();
    if (mAlphaImage
        && (mAlphaImage->width() != mSize.getWidth() || mAlphaImage->height() != mSize.getHeight()))
        mAlphaImage.reset();
}

<<<<<<< HEAD   (cb8bf2 tdf#133564: Text coloring of buttons within non key windows )
=======
OString SkiaSalBitmap::GetImageKey() const
{
    if (mEraseColorSet)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(2) << (255 - mEraseColor.GetTransparency())
           << std::setw(6) << sal_uInt32(mEraseColor.GetRGBColor());
        return OStringLiteral("E") + ss.str().c_str();
    }
    return OStringLiteral("I") + OString::number(GetSkImage()->uniqueID());
}

OString SkiaSalBitmap::GetAlphaImageKey() const
{
    if (mEraseColorSet)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(2)
           << (255 - SkColorGetA(fromEraseColorToAlphaImageColor(mEraseColor)));
        return OStringLiteral("E") + ss.str().c_str();
    }
    return OStringLiteral("I") + OString::number(GetAlphaSkImage()->uniqueID());
}

>>>>>>> CHANGE (2e3111 handle properly cleared bitmap used as alpha for Skia (tdf#1)
#ifdef DBG_UTIL
void SkiaSalBitmap::dump(const char* file) const
{
    sk_sp<SkImage> saveImage = mImage;
    sk_sp<SkImage> saveAlphaImage = mAlphaImage;
    bool resetBuffer = !mBuffer;
    int saveWriteAccessCount = mWriteAccessCount;
    Size savePrescaleSize = mPixelsSize;
    SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
    // avoid possible assert
    thisPtr->mWriteAccessCount = 0;
    SkiaHelper::dump(GetSkImage(), file);
    // restore old state, so that debugging doesn't affect it
    if (resetBuffer)
        thisPtr->mBuffer.reset();
    thisPtr->mImage = saveImage;
    thisPtr->mAlphaImage = saveAlphaImage;
    thisPtr->mWriteAccessCount = saveWriteAccessCount;
    thisPtr->mPixelsSize = savePrescaleSize;
}

void SkiaSalBitmap::verify() const
{
    if (!mBuffer)
        return;
    // Use mPixelsSize, that describes the size of the actual data.
    assert(memcmp(mBuffer.get() + mScanlineSize * mPixelsSize.Height(), CANARY, sizeof(CANARY))
           == 0);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
