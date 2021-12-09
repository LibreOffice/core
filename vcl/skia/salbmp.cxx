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
#include <bitmap/bmpfast.hxx>
#include <vcl/BitmapReadAccess.hxx>

#include <skia/utils.hxx>
#include <skia/zone.hxx>

#include <SkCanvas.h>
#include <SkImage.h>
#include <SkPixelRef.h>
#include <SkSurface.h>
#include <SkSwizzle.h>
#include <SkColorFilter.h>
#include <SkColorMatrix.h>
#include <skia_opts.hxx>

#ifdef DBG_UTIL
#include <fstream>
#define CANARY "skia-canary"
#endif

using namespace SkiaHelper;

// As constexpr here, evaluating it directly in code makes Clang warn about unreachable code.
constexpr bool kN32_SkColorTypeIsBGRA = (kN32_SkColorType == kBGRA_8888_SkColorType);

SkiaSalBitmap::SkiaSalBitmap() {}

SkiaSalBitmap::~SkiaSalBitmap() {}

SkiaSalBitmap::SkiaSalBitmap(const sk_sp<SkImage>& image)
{
    ResetAllData();
    mImage = image;
    mPalette = BitmapPalette();
#if SKIA_USE_BITMAP32
    mBitCount = 32;
#else
    mBitCount = 24;
#endif
    mSize = mPixelsSize = Size(image->width(), image->height());
    ComputeScanlineSize();
    mAnyAccessCount = 0;
#ifdef DBG_UTIL
    mWriteAccessCount = 0;
#endif
    SAL_INFO("vcl.skia.trace", "bitmapfromimage(" << this << ")");
}

bool SkiaSalBitmap::Create(const Size& rSize, vcl::PixelFormat ePixelFormat,
                           const BitmapPalette& rPal)
{
    assert(mAnyAccessCount == 0);
    ResetAllData();
    if (ePixelFormat == vcl::PixelFormat::INVALID)
        return false;
    mPalette = rPal;
    mBitCount = vcl::pixelFormatBitCount(ePixelFormat);
    mSize = rSize;
    ResetPendingScaling();
    if (!ComputeScanlineSize())
    {
        mBitCount = 0;
        mSize = mPixelsSize = Size();
        mScanlineSize = 0;
        mPalette = BitmapPalette();
        return false;
    }
    SAL_INFO("vcl.skia.trace", "create(" << this << ")");
    return true;
}

bool SkiaSalBitmap::ComputeScanlineSize()
{
    int bitScanlineWidth;
    if (o3tl::checked_multiply<int>(mPixelsSize.Width(), mBitCount, bitScanlineWidth))
    {
        SAL_WARN("vcl.skia", "checked multiply failed");
        return false;
    }
    mScanlineSize = AlignedWidth4Bytes(bitScanlineWidth);
    return true;
}

void SkiaSalBitmap::CreateBitmapData()
{
    assert(!mBuffer);
    // Make sure code has not missed calling ComputeScanlineSize().
    assert(mScanlineSize == int(AlignedWidth4Bytes(mPixelsSize.Width() * mBitCount)));
    // The pixels could be stored in SkBitmap, but Skia only supports 8bit gray, 16bit and 32bit formats
    // (e.g. 24bpp is actually stored as 32bpp). But some of our code accessing the bitmap assumes that
    // when it asked for 24bpp, the format really will be 24bpp (e.g. the png loader), so we cannot use
    // SkBitmap to store the data. And even 8bpp is problematic, since Skia does not support palettes
    // and a VCL bitmap can change its grayscale status simply by changing the palette.
    // Moreover creating SkImage from SkBitmap does a data copy unless the bitmap is immutable.
    // So just always store pixels in our buffer and convert as necessary.
    if (mScanlineSize == 0 || mPixelsSize.Height() == 0)
        return;

    size_t allocate = mScanlineSize * mPixelsSize.Height();
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

bool SkiaSalBitmap::Create(const SalBitmap& rSalBmp)
{
    return Create(rSalBmp, vcl::bitDepthToPixelFormat(rSalBmp.GetBitCount()));
}

bool SkiaSalBitmap::Create(const SalBitmap& rSalBmp, SalGraphics* pGraphics)
{
    auto ePixelFormat = vcl::PixelFormat::INVALID;
    if (pGraphics)
        ePixelFormat = vcl::bitDepthToPixelFormat(pGraphics->GetBitCount());
    else
        ePixelFormat = vcl::bitDepthToPixelFormat(rSalBmp.GetBitCount());

    return Create(rSalBmp, ePixelFormat);
}

bool SkiaSalBitmap::Create(const SalBitmap& rSalBmp, vcl::PixelFormat eNewPixelFormat)
{
    assert(mAnyAccessCount == 0);
    assert(&rSalBmp != this);
    ResetAllData();
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
    mEraseColorSet = src.mEraseColorSet;
    mEraseColor = src.mEraseColor;
    if (vcl::pixelFormatBitCount(eNewPixelFormat) != src.GetBitCount())
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
    return false;
}

void SkiaSalBitmap::Destroy()
{
    SAL_INFO("vcl.skia.trace", "destroy(" << this << ")");
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    assert(mAnyAccessCount == 0);
    ResetAllData();
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
            assert(mPixelsSize == mSize);
            assert(!mEraseColorSet);
            break;
        case BitmapAccessMode::Read:
            EnsureBitmapData();
            if (!mBuffer)
                return nullptr;
            assert(mPixelsSize == mSize);
            assert(!mEraseColorSet);
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
    if (nMode != BitmapAccessMode::Info)
        buffer->mpBits = mBuffer.get();
    else
        buffer->mpBits = nullptr;
    if (mPixelsSize == mSize)
        buffer->mnScanlineSize = mScanlineSize;
    else
    {
        // The value of mScanlineSize is based on internal mPixelsSize, but the outside
        // world cares about mSize, the size that the report as the size of the bitmap,
        // regardless of any internal state. So report scanline size for that size.
        Size savedPixelsSize = mPixelsSize;
        mPixelsSize = mSize;
        ComputeScanlineSize();
        buffer->mnScanlineSize = mScanlineSize;
        mPixelsSize = savedPixelsSize;
        ComputeScanlineSize();
    }
    switch (mBitCount)
    {
        case 1:
            buffer->mnFormat = ScanlineFormat::N1BitMsbPal;
            break;
        case 8:
            buffer->mnFormat = ScanlineFormat::N8BitPal;
            break;
        case 24:
            // Make the RGB/BGR format match the default Skia 32bpp format, to allow
            // easy conversion later.
            buffer->mnFormat = kN32_SkColorTypeIsBGRA ? ScanlineFormat::N24BitTcBgr
                                                      : ScanlineFormat::N24BitTcRgb;
            break;
        case 32:
            buffer->mnFormat = kN32_SkColorTypeIsBGRA ? ScanlineFormat::N32BitTcBgra
                                                      : ScanlineFormat::N32BitTcRgba;
            break;
        default:
            abort();
    }
    buffer->mnFormat |= ScanlineFormat::TopDown;
    ++mAnyAccessCount;
#ifdef DBG_UTIL
    if (nMode == BitmapAccessMode::Write)
        ++mWriteAccessCount;
#endif
    return buffer;
}

void SkiaSalBitmap::ReleaseBuffer(BitmapBuffer* pBuffer, BitmapAccessMode nMode)
{
    ReleaseBuffer(pBuffer, nMode, false);
}

void SkiaSalBitmap::ReleaseBuffer(BitmapBuffer* pBuffer, BitmapAccessMode nMode,
                                  bool dontChangeToErase)
{
    if (nMode == BitmapAccessMode::Write)
    {
#ifdef DBG_UTIL
        assert(mWriteAccessCount > 0);
        --mWriteAccessCount;
#endif
        mPalette = pBuffer->maPalette;
        ResetToBuffer();
        DataChanged();
    }
    assert(mAnyAccessCount > 0);
    --mAnyAccessCount;
    // Are there any more ground movements underneath us ?
    assert(pBuffer->mnWidth == mSize.Width());
    assert(pBuffer->mnHeight == mSize.Height());
    assert(pBuffer->mnBitCount == mBitCount);
    assert(pBuffer->mpBits == mBuffer.get() || nMode == BitmapAccessMode::Info);
    verify();
    delete pBuffer;
    if (nMode == BitmapAccessMode::Write && !dontChangeToErase)
    {
        // This saves memory and is also used by IsFullyOpaqueAsAlpha() to avoid unnecessary
        // alpha blending.
        if (IsAllBlack())
        {
            SAL_INFO("vcl.skia.trace", "releasebuffer(" << this << "): erasing to black");
            EraseInternal(COL_BLACK);
        }
    }
}

static bool isAllZero(const sal_uInt8* data, size_t size)
{ // For performance, check in larger data chunks.
#ifdef UINT64_MAX
    const int64_t* d = reinterpret_cast<const int64_t*>(data);
#else
    const int32_t* d = reinterpret_cast<const int32_t*>(data);
#endif
    constexpr size_t step = sizeof(*d) * 8;
    for (size_t i = 0; i < size / step; ++i)
    { // Unrolled loop.
        if (d[0] != 0)
            return false;
        if (d[1] != 0)
            return false;
        if (d[2] != 0)
            return false;
        if (d[3] != 0)
            return false;
        if (d[4] != 0)
            return false;
        if (d[5] != 0)
            return false;
        if (d[6] != 0)
            return false;
        if (d[7] != 0)
            return false;
        d += 8;
    }
    for (size_t i = size / step * step; i < size; ++i)
        if (data[i] != 0)
            return false;
    return true;
}

bool SkiaSalBitmap::IsAllBlack() const
{
    if (mBitCount % 8 != 0 || (!!mPalette && mPalette[0] != COL_BLACK))
        return false; // Don't bother.
    if (mSize.Width() * mBitCount / 8 == mScanlineSize)
        return isAllZero(mBuffer.get(), mScanlineSize * mSize.Height());
    for (tools::Long y = 0; y < mSize.Height(); ++y)
        if (!isAllZero(mBuffer.get() + mScanlineSize * y, mSize.Width() * mBitCount / 8))
            return false;
    return true;
}

bool SkiaSalBitmap::GetSystemData(BitmapSystemData&)
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
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

    if (mEraseColorSet)
    { // Simple.
        mSize = newSize;
        ResetPendingScaling();
        EraseInternal(mEraseColor);
        return true;
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
    // The idea here is that the actual scaling will be delayed until the result
    // is actually needed. Usually the scaled bitmap will be drawn somewhere,
    // so delaying will mean the scaling can be done as a part of GetSkImage().
    // That means it can be GPU-accelerated, while done here directly it would need
    // to be either done by CPU, or with the CPU->GPU->CPU roundtrip required
    // by GPU-accelerated scaling.
    // Pending scaling is detected by 'mSize != mPixelsSize' for mBuffer,
    // and 'imageSize(mImage) != mSize' for mImage. It is not intended to have 3 different
    // sizes though, code below keeps only mBuffer or mImage. Note that imageSize(mImage)
    // may or may not be equal to mPixelsSize, depending on whether mImage is set here
    // (sizes will be equal) or whether it's set in GetSkImage() (will not be equal).
    // Pending scaling is considered "done" by the time mBuffer is resized (or created).
    // Resizing of mImage is somewhat independent of this, since mImage is primarily
    // considered to be a cached object (although sometimes it's the only data available).

    // If there is already one scale() pending, use the lowest quality of all requested.
    switch (nScaleFlag)
    {
        case BmpScaleFlag::Fast:
            mScaleQuality = nScaleFlag;
            break;
        case BmpScaleFlag::NearestNeighbor:
            // We handle this the same way as Fast by mapping to Skia's nearest-neighbor,
            // and it's needed for unittests (mScaling and testTdf132367()).
            mScaleQuality = nScaleFlag;
            break;
        case BmpScaleFlag::Default:
            if (mScaleQuality == BmpScaleFlag::BestQuality)
                mScaleQuality = nScaleFlag;
            break;
        case BmpScaleFlag::BestQuality:
            // Best is the maximum, set by default.
            break;
        default:
            SAL_INFO("vcl.skia.trace", "scale(" << this << "): unsupported scale algorithm");
            return false;
    }
    mSize = newSize;
    // If we have both mBuffer and mImage, prefer mImage, since it likely will be drawn later.
    // We could possibly try to keep the buffer as well, but that would complicate things
    // with two different data structures to be scaled on-demand, and it's a question
    // if that'd realistically help with anything.
    if (mImage)
        ResetToSkImage(mImage);
    else
        ResetToBuffer();
    DataChanged();
    // The rest will be handled when the scaled bitmap is actually needed,
    // such as in EnsureBitmapData() or GetSkImage().
    return true;
}

bool SkiaSalBitmap::Replace(const Color&, const Color&, sal_uInt8)
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
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
    if (!mBuffer && mImage && !mEraseColorSet)
    {
        if (mBitCount == 8 && mPalette.IsGreyPalette8Bit())
            return true;
        sk_sp<SkSurface> surface
            = createSkSurface(imageSize(mImage), mImage->imageInfo().alphaType());
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
        surface->getCanvas()->drawImage(mImage, 0, 0, SkSamplingOptions(), &paint);
        mBitCount = 8;
        ComputeScanlineSize();
        mPalette = Bitmap::GetGreyPalette(256);
        ResetToSkImage(makeCheckedImageSnapshot(surface));
        DataChanged();
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
    if (mEraseColorSet)
    {
        mBitCount = 8;
        ComputeScanlineSize();
        mPalette = Bitmap::GetGreyPalette(256);
        EraseInternal(mEraseColor);
        SAL_INFO("vcl.skia.trace", "interpretas8bit(" << this << ") with erase color");
        return true;
    }
    // This is usually used by AlphaMask, the point is just to treat
    // the content as an alpha channel. This is often used
    // by the horrible separate-alpha-outdev hack, where the bitmap comes
    // from SkiaSalGraphicsImpl::GetBitmap(), so only mImage is set,
    // and that is followed by a later call to GetAlphaSkImage().
    // Avoid the costly SkImage->buffer->SkImage conversion and simply
    // just treat the SkImage as being for 8bit bitmap. EnsureBitmapData()
    // will do the conversion if needed, but the normal case will be
    // GetAlphaSkImage() creating kAlpha_8_SkColorType SkImage from it.
    if (mImage)
    {
        mBitCount = 8;
        ComputeScanlineSize();
        mPalette = Bitmap::GetGreyPalette(256);
        ResetToSkImage(mImage); // keep mImage, it will be interpreted as 8bit if needed
        DataChanged();
        SAL_INFO("vcl.skia.trace", "interpretas8bit(" << this << ") with image");
        return true;
    }
    SAL_INFO("vcl.skia.trace", "interpretas8bit(" << this << ") with pixel data, ignoring");
    return false;
}

bool SkiaSalBitmap::Erase(const Color& color)
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    // Optimized variant, just remember the color and apply it when needed,
    // which may save having to do format conversions (e.g. GetSkImage()
    // may directly erase the SkImage).
    EraseInternal(color);
    SAL_INFO("vcl.skia.trace", "erase(" << this << ")");
    return true;
}

void SkiaSalBitmap::EraseInternal(const Color& color)
{
    ResetAllData();
    mEraseColorSet = true;
    mEraseColor = color;
}

bool SkiaSalBitmap::AlphaBlendWith(const SalBitmap& rSalBmp)
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    const SkiaSalBitmap* otherBitmap = dynamic_cast<const SkiaSalBitmap*>(&rSalBmp);
    if (!otherBitmap)
        return false;
    if (mSize != otherBitmap->mSize)
        return false;
    // We're called from AlphaMask, which should ensure 8bit.
    assert(GetBitCount() == 8 && mPalette.IsGreyPalette8Bit());
    // If neither bitmap have Skia images, then AlphaMask::BlendWith() will be faster,
    // as it will operate on mBuffer pixel buffers, while for Skia we'd need to convert it.
    // If one has and one doesn't, do it using Skia, under the assumption that after this
    // the resulting Skia image will be needed for drawing.
    if (!(mImage || mEraseColorSet) && !(otherBitmap->mImage || otherBitmap->mEraseColorSet))
        return false;
    // This is for AlphaMask, which actually stores the alpha as the pixel values.
    // I.e. take value of the color channel (one of them, if >8bit, they should be the same).
    if (mEraseColorSet && otherBitmap->mEraseColorSet)
    {
        const sal_uInt16 nGrey1 = mEraseColor.GetRed();
        const sal_uInt16 nGrey2 = otherBitmap->mEraseColor.GetRed();
        const sal_uInt8 nGrey = static_cast<sal_uInt8>(nGrey1 + nGrey2 - nGrey1 * nGrey2 / 255);
        mEraseColor = Color(nGrey, nGrey, nGrey);
        DataChanged();
        SAL_INFO("vcl.skia.trace",
                 "alphablendwith(" << this << ") : with erase color " << otherBitmap);
        return true;
    }
    std::unique_ptr<SkiaSalBitmap> otherBitmapAllocated;
    if (otherBitmap->GetBitCount() != 8 || !otherBitmap->mPalette.IsGreyPalette8Bit())
    { // Convert/interpret as 8bit if needed.
        otherBitmapAllocated = std::make_unique<SkiaSalBitmap>();
        if (!otherBitmapAllocated->Create(*otherBitmap) || !otherBitmapAllocated->InterpretAs8Bit())
            return false;
        otherBitmap = otherBitmapAllocated.get();
    }
    sk_sp<SkSurface> surface = createSkSurface(mSize);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc); // set as is
    surface->getCanvas()->drawImage(GetSkImage(), 0, 0, SkSamplingOptions(), &paint);
    paint.setBlendMode(SkBlendMode::kScreen); // src+dest - src*dest/255 (in 0..1)
    surface->getCanvas()->drawImage(otherBitmap->GetSkImage(), 0, 0, SkSamplingOptions(), &paint);
    ResetToSkImage(makeCheckedImageSnapshot(surface));
    DataChanged();
    SAL_INFO("vcl.skia.trace", "alphablendwith(" << this << ") : with image " << otherBitmap);
    return true;
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
            if (!bitmap.installPixels(
                    SkImageInfo::MakeS32(mPixelsSize.Width(), mPixelsSize.Height(), alphaType()),
                    data.release(), mScanlineSize,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
        }
        else if (mBitCount == 24)
        {
            // Convert 24bpp RGB/BGR to 32bpp RGBA/BGRA.
            std::unique_ptr<uint32_t[]> data(
                new uint32_t[mPixelsSize.Height() * mPixelsSize.Width()]);
            uint32_t* dest = data.get();
            // SkConvertRGBToRGBA() also works as BGR to BGRA (the function extends 3 bytes to 4
            // by adding 0xFF alpha, so position of B and R doesn't matter).
            if (mPixelsSize.Width() * 3 == mScanlineSize)
                SkConvertRGBToRGBA(dest, mBuffer.get(), mPixelsSize.Height() * mPixelsSize.Width());
            else
            {
                for (tools::Long y = 0; y < mPixelsSize.Height(); ++y)
                {
                    const sal_uInt8* src = mBuffer.get() + mScanlineSize * y;
                    SkConvertRGBToRGBA(dest, src, mPixelsSize.Width());
                    dest += mPixelsSize.Width();
                }
            }
            if (!bitmap.installPixels(
                    SkImageInfo::MakeS32(mPixelsSize.Width(), mPixelsSize.Height(),
                                         kOpaque_SkAlphaType),
                    data.release(), mPixelsSize.Width() * 4,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
        }
        else if (mBitCount == 8 && mPalette.IsGreyPalette8Bit())
        {
            // Convert 8bpp gray to 32bpp RGBA/BGRA.
            // There's also kGray_8_SkColorType, but it's probably simpler to make
            // GetAsSkBitmap() always return 32bpp SkBitmap and then assume mImage
            // is always 32bpp too.
            std::unique_ptr<uint32_t[]> data(
                new uint32_t[mPixelsSize.Height() * mPixelsSize.Width()]);
            uint32_t* dest = data.get();
            if (mPixelsSize.Width() * 1 == mScanlineSize)
                SkConvertGrayToRGBA(dest, mBuffer.get(),
                                    mPixelsSize.Height() * mPixelsSize.Width());
            else
            {
                for (tools::Long y = 0; y < mPixelsSize.Height(); ++y)
                {
                    const sal_uInt8* src = mBuffer.get() + mScanlineSize * y;
                    SkConvertGrayToRGBA(dest, src, mPixelsSize.Width());
                    dest += mPixelsSize.Width();
                }
            }
            if (!bitmap.installPixels(
                    SkImageInfo::MakeS32(mPixelsSize.Width(), mPixelsSize.Height(),
                                         kOpaque_SkAlphaType),
                    data.release(), mPixelsSize.Width() * 4,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
        }
        else
        {
            std::unique_ptr<sal_uInt8[]> data = convertDataBitCount(
                mBuffer.get(), mPixelsSize.Width(), mPixelsSize.Height(), mBitCount, mScanlineSize,
                mPalette, kN32_SkColorTypeIsBGRA ? BitConvert::BGRA : BitConvert::RGBA);
            if (!bitmap.installPixels(
                    SkImageInfo::MakeS32(mPixelsSize.Width(), mPixelsSize.Height(),
                                         kOpaque_SkAlphaType),
                    data.release(), mPixelsSize.Width() * 4,
                    [](void* addr, void*) { delete[] static_cast<sal_uInt8*>(addr); }, nullptr))
                abort();
        }
    }
    bitmap.setImmutable();
    return bitmap;
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

// SkiaSalBitmap can store data in both the SkImage and our mBuffer, which with large
// images can waste quite a lot of memory. Ideally we should store the data in Skia's
// SkBitmap, but LO wants us to support data formats that Skia doesn't support.
// So try to conserve memory by keeping the data only once in that was the most
// recently wanted storage, and drop the other one. Usually the other one won't be needed
// for a long time, and especially with raster the conversion is usually fast.
// Do this only with raster, to avoid GPU->CPU transfer in GPU mode (exception is 32bit
// builds, where memory is more important). Also don't do this with paletted bitmaps,
// where EnsureBitmapData() would be expensive.
// Ideally SalBitmap should be able to say which bitmap formats it supports
// and VCL code should oblige, which would allow reusing the same data.
bool SkiaSalBitmap::ConserveMemory() const
{
    static bool keepBitmapBuffer = getenv("SAL_SKIA_KEEP_BITMAP_BUFFER") != nullptr;
    constexpr bool is32Bit = sizeof(void*) == 4;
    // 16MiB bitmap data at least (set to 0 for easy testing).
    constexpr tools::Long maxBufferSize = 2000 * 2000 * 4;
    return !keepBitmapBuffer && (renderMethodToUse() == RenderRaster || is32Bit)
           && mPixelsSize.Height() * mScanlineSize > maxBufferSize
           && (mBitCount > 8 || (mBitCount == 8 && mPalette.IsGreyPalette8Bit()));
}

const sk_sp<SkImage>& SkiaSalBitmap::GetSkImage(DirectImage direct) const
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    if (direct == DirectImage::Yes)
        return mImage;
    if (mEraseColorSet)
    {
        if (mImage)
        {
            assert(imageSize(mImage) == mSize);
            return mImage;
        }
        SkiaZone zone;
        sk_sp<SkSurface> surface = createSkSurface(
            mSize, mEraseColor.IsTransparent() ? kPremul_SkAlphaType : kOpaque_SkAlphaType);
        assert(surface);
        surface->getCanvas()->clear(toSkColor(mEraseColor));
        SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
        thisPtr->mImage = makeCheckedImageSnapshot(surface);
        SAL_INFO("vcl.skia.trace", "getskimage(" << this << ") from erase color " << mEraseColor);
        return mImage;
    }
    if (mPixelsSize != mSize && !mImage && renderMethodToUse() != RenderRaster)
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
        sk_sp<SkImage> image = createSkImage(GetAsSkBitmap());
        assert(image);
        thisPtr->mSize = savedSize;
        thisPtr->ResetToSkImage(image);
    }
    if (mImage)
    {
        if (imageSize(mImage) != mSize)
        {
            assert(!mBuffer); // This code should be only called if only mImage holds data.
            SkiaZone zone;
            sk_sp<SkSurface> surface = createSkSurface(mSize, mImage->imageInfo().alphaType());
            assert(surface);
            SkPaint paint;
            paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
            surface->getCanvas()->drawImageRect(
                mImage, SkRect::MakeWH(mSize.Width(), mSize.Height()),
                makeSamplingOptions(mScaleQuality, imageSize(mImage), mSize, 1), &paint);
            SAL_INFO("vcl.skia.trace", "getskimage(" << this << "): image scaled "
                                                     << Size(mImage->width(), mImage->height())
                                                     << "->" << mSize << ":"
                                                     << static_cast<int>(mScaleQuality));
            SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
            thisPtr->mImage = makeCheckedImageSnapshot(surface);
        }
        return mImage;
    }
    SkiaZone zone;
    sk_sp<SkImage> image = createSkImage(GetAsSkBitmap());
    assert(image);
    SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
    thisPtr->mImage = image;
    // The data is now stored both in the SkImage and in our mBuffer, so drop the buffer
    // if conserving memory. It'll be converted back by EnsureBitmapData() if needed.
    if (ConserveMemory() && mAnyAccessCount == 0)
    {
        SAL_INFO("vcl.skia.trace", "getskimage(" << this << "): dropping buffer");
        thisPtr->ResetToSkImage(mImage);
    }
    SAL_INFO("vcl.skia.trace", "getskimage(" << this << ")");
    return mImage;
}

const sk_sp<SkImage>& SkiaSalBitmap::GetAlphaSkImage(DirectImage direct) const
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    if (direct == DirectImage::Yes)
        return mAlphaImage;
    if (mEraseColorSet)
    {
        if (mAlphaImage)
        {
            assert(imageSize(mAlphaImage) == mSize);
            return mAlphaImage;
        }
        SkiaZone zone;
        sk_sp<SkSurface> surface = createSkSurface(mSize, kAlpha_8_SkColorType);
        assert(surface);
        surface->getCanvas()->clear(fromEraseColorToAlphaImageColor(mEraseColor));
        SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
        thisPtr->mAlphaImage = makeCheckedImageSnapshot(surface);
        SAL_INFO("vcl.skia.trace",
                 "getalphaskimage(" << this << ") from erase color " << mEraseColor);
        return mAlphaImage;
    }
    if (mAlphaImage)
    {
        if (imageSize(mAlphaImage) == mSize)
            return mAlphaImage;
    }
    if (mImage)
    {
        SkiaZone zone;
        const bool scaling = imageSize(mImage) != mSize;
        SkPixmap pixmap;
        // Note: We cannot do this when 'scaling' because SkCanvas::drawImageRect()
        // with kAlpha_8_SkColorType as source and destination would act as SkBlendMode::kSrcOver
        // despite SkBlendMode::kSrc set (https://bugs.chromium.org/p/skia/issues/detail?id=9692).
        if (mImage->peekPixels(&pixmap) && !scaling)
        {
            assert(pixmap.colorType() == kN32_SkColorType);
            // In non-GPU mode, convert 32bit data to 8bit alpha, this is faster than
            // the SkColorFilter below. Since this is the VCL alpha-vdev alpha, where
            // all R,G,B are the same and in fact mean alpha, this means we simply take one
            // 8bit channel from the input, and that's the output.
            SkBitmap bitmap;
            if (!bitmap.installPixels(pixmap))
                abort();
            SkBitmap alphaBitmap;
            if (!alphaBitmap.tryAllocPixels(SkImageInfo::MakeA8(bitmap.width(), bitmap.height())))
                abort();
            if (int(bitmap.rowBytes()) == bitmap.width() * 4)
            {
                SkConvertRGBAToR(alphaBitmap.getAddr8(0, 0), bitmap.getAddr32(0, 0),
                                 bitmap.width() * bitmap.height());
            }
            else
            {
                for (tools::Long y = 0; y < bitmap.height(); ++y)
                    SkConvertRGBAToR(alphaBitmap.getAddr8(0, y), bitmap.getAddr32(0, y),
                                     bitmap.width());
            }
            alphaBitmap.setImmutable();
            sk_sp<SkImage> alphaImage = createSkImage(alphaBitmap);
            assert(alphaImage);
            SAL_INFO("vcl.skia.trace", "getalphaskimage(" << this << ") from raster image");
            // Don't bother here with ConserveMemory(), mImage -> mAlphaImage conversions should
            // generally only happen with the separate-alpha-outdev hack, and those bitmaps should
            // be temporary.
            SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
            thisPtr->mAlphaImage = alphaImage;
            return mAlphaImage;
        }
        // Move the R channel value to the alpha channel. This seems to be the only
        // way to reinterpret data in SkImage as an alpha SkImage without accessing the pixels.
        // NOTE: The matrix is 4x5 organized as columns (i.e. each line is a column, not a row).
        constexpr SkColorMatrix redToAlpha(0, 0, 0, 0, 0, // R column
                                           0, 0, 0, 0, 0, // G column
                                           0, 0, 0, 0, 0, // B column
                                           1, 0, 0, 0, 0); // A column
        SkPaint paint;
        paint.setColorFilter(SkColorFilters::Matrix(redToAlpha));
        if (scaling)
            assert(!mBuffer); // This code should be only called if only mImage holds data.
        sk_sp<SkSurface> surface = createSkSurface(mSize, kAlpha_8_SkColorType);
        assert(surface);
        paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
        surface->getCanvas()->drawImageRect(
            mImage, SkRect::MakeWH(mSize.Width(), mSize.Height()),
            scaling ? makeSamplingOptions(mScaleQuality, imageSize(mImage), mSize, 1)
                    : SkSamplingOptions(),
            &paint);
        if (scaling)
            SAL_INFO("vcl.skia.trace", "getalphaskimage(" << this << "): image scaled "
                                                          << Size(mImage->width(), mImage->height())
                                                          << "->" << mSize << ":"
                                                          << static_cast<int>(mScaleQuality));
        else
            SAL_INFO("vcl.skia.trace", "getalphaskimage(" << this << ") from image");
        // Don't bother here with ConserveMemory(), mImage -> mAlphaImage conversions should
        // generally only happen with the separate-alpha-outdev hack, and those bitmaps should
        // be temporary.
        SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
        thisPtr->mAlphaImage = makeCheckedImageSnapshot(surface);
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
        sk_sp<SkImage> image = createSkImage(alphaBitmap);
        assert(image);
        const_cast<sk_sp<SkImage>&>(mAlphaImage) = image;
    }
    else
    {
        sk_sp<SkSurface> surface = createSkSurface(mSize, kAlpha_8_SkColorType);
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
        surface->getCanvas()->drawImage(GetAsSkBitmap().asImage(), 0, 0, SkSamplingOptions(),
                                        &paint);
        SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
        thisPtr->mAlphaImage = makeCheckedImageSnapshot(surface);
    }
    // The data is now stored both in the SkImage and in our mBuffer, so drop the buffer
    // if conserving memory and the conversion back would be simple (it'll be converted back
    // by EnsureBitmapData() if needed).
    if (ConserveMemory() && mBitCount == 8 && mPalette.IsGreyPalette8Bit() && mAnyAccessCount == 0)
    {
        SAL_INFO("vcl.skia.trace", "getalphaskimage(" << this << "): dropping buffer");
        SkiaSalBitmap* thisPtr = const_cast<SkiaSalBitmap*>(this);
        thisPtr->mBuffer.reset();
    }
    SAL_INFO("vcl.skia.trace", "getalphaskimage(" << this << ")");
    return mAlphaImage;
}

void SkiaSalBitmap::TryDirectConvertToAlphaNoScaling()
{
    // This is a bit of a hack. Because of the VCL alpha hack where alpha is stored
    // separately, we often convert mImage to mAlphaImage to represent the alpha
    // channel. If code finds out that there is mImage but no mAlphaImage,
    // this will create it from it, without checking for delayed scaling (i.e.
    // it is "direct").
    assert(mImage);
    assert(!mAlphaImage);
    // Set wanted size, trigger conversion.
    Size savedSize = mSize;
    mSize = imageSize(mImage);
    GetAlphaSkImage();
    assert(mAlphaImage);
    mSize = savedSize;
}

// If the bitmap is to be erased, SkShader with the color set is more efficient
// than creating an image filled with the color.
bool SkiaSalBitmap::PreferSkShader() const { return mEraseColorSet; }

sk_sp<SkShader> SkiaSalBitmap::GetSkShader(const SkSamplingOptions& samplingOptions,
                                           DirectImage direct) const
{
    if (mEraseColorSet)
        return SkShaders::Color(toSkColor(mEraseColor));
    return GetSkImage(direct)->makeShader(samplingOptions);
}

sk_sp<SkShader> SkiaSalBitmap::GetAlphaSkShader(const SkSamplingOptions& samplingOptions,
                                                DirectImage direct) const
{
    if (mEraseColorSet)
        return SkShaders::Color(fromEraseColorToAlphaImageColor(mEraseColor));
    return GetAlphaSkImage(direct)->makeShader(samplingOptions);
}

bool SkiaSalBitmap::IsFullyOpaqueAsAlpha() const
{
    if (!mEraseColorSet) // Set from Erase() or ReleaseBuffer().
        return false;
    // If the erase color is set so that this bitmap used as alpha would
    // mean a fully opaque alpha mask (= noop), we can skip using it.
    // Note that for alpha bitmaps we use the VCL "transparency" convention,
    // i.e. alpha 0 is opaque.
    return SkColorGetA(fromEraseColorToAlphaImageColor(mEraseColor)) == 0;
}

SkAlphaType SkiaSalBitmap::alphaType() const
{
    if (mEraseColorSet)
        return mEraseColor.IsTransparent() ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
#if SKIA_USE_BITMAP32
    // The bitmap's alpha matters only if SKIA_USE_BITMAP32 is set, otherwise
    // the alpha is in a separate bitmap.
    if (mBitCount == 32)
        return kPremul_SkAlphaType;
#endif
    return kOpaque_SkAlphaType;
}

void SkiaSalBitmap::PerformErase()
{
    if (mPixelsSize.IsEmpty())
        return;
    BitmapBuffer* bitmapBuffer = AcquireBuffer(BitmapAccessMode::Write);
    if (bitmapBuffer == nullptr)
        abort();
    Color fastColor = mEraseColor;
    if (!!mPalette)
        fastColor = Color(ColorTransparency, mPalette.GetBestIndex(fastColor));
    if (!ImplFastEraseBitmap(*bitmapBuffer, fastColor))
    {
        FncSetPixel setPixel = BitmapReadAccess::SetPixelFunction(bitmapBuffer->mnFormat);
        assert(bitmapBuffer->mnFormat & ScanlineFormat::TopDown);
        // Set first scanline, copy to others.
        Scanline scanline = bitmapBuffer->mpBits;
        for (tools::Long x = 0; x < bitmapBuffer->mnWidth; ++x)
            setPixel(scanline, x, mEraseColor, bitmapBuffer->maColorMask);
        for (tools::Long y = 1; y < bitmapBuffer->mnHeight; ++y)
            memcpy(scanline + y * bitmapBuffer->mnScanlineSize, scanline,
                   bitmapBuffer->mnScanlineSize);
    }
    ReleaseBuffer(bitmapBuffer, BitmapAccessMode::Write, true);
}

void SkiaSalBitmap::EnsureBitmapData()
{
    if (mEraseColorSet)
    {
        SkiaZone zone;
        assert(mPixelsSize == mSize);
        assert(!mBuffer);
        CreateBitmapData();
        // Unset now, so that repeated call will return mBuffer.
        mEraseColorSet = false;
        PerformErase();
        verify();
        SAL_INFO("vcl.skia.trace",
                 "ensurebitmapdata(" << this << ") from erase color " << mEraseColor);
        return;
    }

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

    // Convert from alpha image, if the conversion is simple.
    if (mAlphaImage && imageSize(mAlphaImage) == mSize && mBitCount == 8
        && mPalette.IsGreyPalette8Bit())
    {
        assert(mAlphaImage->colorType() == kAlpha_8_SkColorType);
        SkiaZone zone;
        SkBitmap bitmap;
        SkPixmap pixmap;
        if (mAlphaImage->peekPixels(&pixmap))
            bitmap.installPixels(pixmap);
        else
        {
            if (!bitmap.tryAllocPixels(SkImageInfo::MakeA8(mSize.Width(), mSize.Height())))
                abort();
            SkCanvas canvas(bitmap);
            SkPaint paint;
            paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
            canvas.drawImage(mAlphaImage, 0, 0, SkSamplingOptions(), &paint);
            canvas.flush();
        }
        bitmap.setImmutable();
        ResetPendingScaling();
        CreateBitmapData();
        assert(mBuffer != nullptr);
        assert(mPixelsSize == mSize);
        if (int(bitmap.rowBytes()) == mScanlineSize)
            memcpy(mBuffer.get(), bitmap.getPixels(), mSize.Height() * mScanlineSize);
        else
        {
            for (tools::Long y = 0; y < mSize.Height(); ++y)
            {
                const uint8_t* src = static_cast<uint8_t*>(bitmap.getAddr(0, y));
                sal_uInt8* dest = mBuffer.get() + mScanlineSize * y;
                memcpy(dest, src, mScanlineSize);
            }
        }
        verify();
        // We've created the bitmap data from mAlphaImage, drop the image if conserving memory,
        // it'll be converted back if needed.
        if (ConserveMemory())
        {
            SAL_INFO("vcl.skia.trace", "ensurebitmapdata(" << this << "): dropping images");
            ResetToBuffer();
        }
        SAL_INFO("vcl.skia.trace", "ensurebitmapdata(" << this << "): from alpha image");
        return;
    }

    if (!mImage)
    {
        // No data at all, create uninitialized data.
        CreateBitmapData();
        SAL_INFO("vcl.skia.trace", "ensurebitmapdata(" << this << "): uninitialized");
        return;
    }
    // Try to fill mBuffer from mImage.
    assert(mImage->colorType() == kN32_SkColorType);
    SkiaZone zone;
    // If the source image has no alpha, then use no alpha (faster to convert), otherwise
    // use kUnpremul_SkAlphaType to make Skia convert from premultiplied alpha when reading
    // from the SkImage (the alpha will be ignored if converting to bpp<32 formats, but
    // the color channels must be unpremultiplied. Unless bpp==32 and SKIA_USE_BITMAP32,
    // in which case use kPremul_SkAlphaType, since SKIA_USE_BITMAP32 implies premultiplied alpha.
    SkAlphaType alphaType = kUnpremul_SkAlphaType;
    if (mImage->imageInfo().alphaType() == kOpaque_SkAlphaType)
        alphaType = kOpaque_SkAlphaType;
#if SKIA_USE_BITMAP32
    if (mBitCount == 32)
        alphaType = kPremul_SkAlphaType;
#endif
    SkBitmap bitmap;
    SkPixmap pixmap;
    if (imageSize(mImage) == mSize && mImage->imageInfo().alphaType() == alphaType
        && mImage->peekPixels(&pixmap))
    {
        bitmap.installPixels(pixmap);
    }
    else
    {
        if (!bitmap.tryAllocPixels(SkImageInfo::MakeS32(mSize.Width(), mSize.Height(), alphaType)))
            abort();
        SkCanvas canvas(bitmap);
        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc); // set as is, including alpha
        if (imageSize(mImage) != mSize) // pending scaling?
        {
            canvas.drawImageRect(mImage, SkRect::MakeWH(mSize.getWidth(), mSize.getHeight()),
                                 makeSamplingOptions(mScaleQuality, imageSize(mImage), mSize, 1),
                                 &paint);
            SAL_INFO("vcl.skia.trace",
                     "ensurebitmapdata(" << this << "): image scaled " << imageSize(mImage) << "->"
                                         << mSize << ":" << static_cast<int>(mScaleQuality));
        }
        else
            canvas.drawImage(mImage, 0, 0, SkSamplingOptions(), &paint);
        canvas.flush();
    }
    bitmap.setImmutable();
    ResetPendingScaling();
    CreateBitmapData();
    assert(mBuffer != nullptr);
    assert(mPixelsSize == mSize);
    if (mBitCount == 32)
    {
        if (int(bitmap.rowBytes()) == mScanlineSize)
            memcpy(mBuffer.get(), bitmap.getPixels(), mSize.Height() * mScanlineSize);
        else
        {
            for (tools::Long y = 0; y < mSize.Height(); ++y)
            {
                const uint8_t* src = static_cast<uint8_t*>(bitmap.getAddr(0, y));
                sal_uInt8* dest = mBuffer.get() + mScanlineSize * y;
                memcpy(dest, src, mScanlineSize);
            }
        }
    }
    else if (mBitCount == 24) // non-paletted
    {
        if (int(bitmap.rowBytes()) == mSize.Width() * 4 && mSize.Width() * 3 == mScanlineSize)
        {
            SkConvertRGBAToRGB(mBuffer.get(), bitmap.getAddr32(0, 0),
                               mSize.Height() * mSize.Width());
        }
        else
        {
            for (tools::Long y = 0; y < mSize.Height(); ++y)
            {
                const uint32_t* src = bitmap.getAddr32(0, y);
                sal_uInt8* dest = mBuffer.get() + mScanlineSize * y;
                SkConvertRGBAToRGB(dest, src, mSize.Width());
            }
        }
    }
    else if (mBitCount == 8 && mPalette.IsGreyPalette8Bit())
    { // no actual data conversion, use one color channel as the gray value
        if (int(bitmap.rowBytes()) == mSize.Width() * 4 && mSize.Width() * 1 == mScanlineSize)
        {
            SkConvertRGBAToR(mBuffer.get(), bitmap.getAddr32(0, 0), mSize.Height() * mSize.Width());
        }
        else
        {
            for (tools::Long y = 0; y < mSize.Height(); ++y)
            {
                const uint32_t* src = bitmap.getAddr32(0, y);
                sal_uInt8* dest = mBuffer.get() + mScanlineSize * y;
                SkConvertRGBAToR(dest, src, mSize.Width());
            }
        }
    }
    else
    {
        std::unique_ptr<vcl::ScanlineWriter> pWriter
            = vcl::ScanlineWriter::Create(mBitCount, mPalette);
        for (tools::Long y = 0; y < mSize.Height(); ++y)
        {
            const uint8_t* src = static_cast<uint8_t*>(bitmap.getAddr(0, y));
            sal_uInt8* dest = mBuffer.get() + mScanlineSize * y;
            pWriter->nextLine(dest);
            for (tools::Long x = 0; x < mSize.Width(); ++x)
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
    // We've created the bitmap data from mImage, drop the image if conserving memory,
    // it'll be converted back if needed.
    if (ConserveMemory())
    {
        SAL_INFO("vcl.skia.trace", "ensurebitmapdata(" << this << "): dropping images");
        ResetToBuffer();
    }
    SAL_INFO("vcl.skia.trace", "ensurebitmapdata(" << this << ")");
}

void SkiaSalBitmap::EnsureBitmapUniqueData()
{
#ifdef DBG_UTIL
    assert(mWriteAccessCount == 0);
#endif
    EnsureBitmapData();
    assert(mPixelsSize == mSize);
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

void SkiaSalBitmap::ResetToBuffer()
{
    SkiaZone zone;
    // This should never be called to drop mImage if that's the only data we have.
    assert(mBuffer || !mImage);
    mImage.reset();
    mAlphaImage.reset();
    mEraseColorSet = false;
}

void SkiaSalBitmap::ResetToSkImage(sk_sp<SkImage> image)
{
    assert(mAnyAccessCount == 0); // can't reset mBuffer if there's a read access pointing to it
    SkiaZone zone;
    mBuffer.reset();
    mImage = image;
    mAlphaImage.reset();
    mEraseColorSet = false;
}

void SkiaSalBitmap::ResetAllData()
{
    assert(mAnyAccessCount == 0);
    SkiaZone zone;
    mBuffer.reset();
    mImage.reset();
    mAlphaImage.reset();
    mEraseColorSet = false;
    mPixelsSize = mSize;
    ComputeScanlineSize();
    DataChanged();
}

void SkiaSalBitmap::DataChanged() { InvalidateChecksum(); }

void SkiaSalBitmap::ResetPendingScaling()
{
    if (mPixelsSize == mSize)
        return;
    SkiaZone zone;
    mScaleQuality = BmpScaleFlag::BestQuality;
    mPixelsSize = mSize;
    ComputeScanlineSize();
    // Information about the pending scaling has been discarded, so make sure we do not
    // keep around any cached images that would still need scaling.
    if (mImage && imageSize(mImage) != mSize)
        mImage.reset();
    if (mAlphaImage && imageSize(mAlphaImage) != mSize)
        mAlphaImage.reset();
}

OString SkiaSalBitmap::GetImageKey(DirectImage direct) const
{
    if (mEraseColorSet)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(6)
           << static_cast<sal_uInt32>(mEraseColor.GetRGBColor()) << std::setw(2)
           << static_cast<int>(mEraseColor.GetAlpha());
        return OString::Concat("E") + ss.str().c_str();
    }
    assert(direct == DirectImage::No || mImage);
    sk_sp<SkImage> image = GetSkImage(direct);
    // In some cases drawing code may try to draw the same content but using
    // different bitmaps (even underlying bitmaps), for example canvas apparently
    // copies the same things around in tdf#146095. For pixel-based images
    // it should be still cheaper to compute a checksum and avoid re-caching.
    if (!image->isTextureBacked())
        return OString::Concat("C") + OString::number(getSkImageChecksum(image));
    return OString::Concat("I") + OString::number(image->uniqueID());
}

OString SkiaSalBitmap::GetAlphaImageKey(DirectImage direct) const
{
    if (mEraseColorSet)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(2)
           << static_cast<int>(255 - SkColorGetA(fromEraseColorToAlphaImageColor(mEraseColor)));
        return OString::Concat("E") + ss.str().c_str();
    }
    assert(direct == DirectImage::No || mAlphaImage);
    sk_sp<SkImage> image = GetAlphaSkImage(direct);
    if (!image->isTextureBacked())
        return OString::Concat("C") + OString::number(getSkImageChecksum(image));
    return OString::Concat("I") + OString::number(image->uniqueID());
}

void SkiaSalBitmap::dump(const char* file) const
{
    // Use a copy, so that debugging doesn't affect this instance.
    SkiaSalBitmap copy;
    copy.Create(*this);
    SkiaHelper::dump(copy.GetSkImage(), file);
}

#ifdef DBG_UTIL
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
