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

#ifndef INCLUDED_VCL_INC_SKIA_UTILS_H
#define INCLUDED_VCL_INC_SKIA_UTILS_H

#include <vcl/skia/SkiaHelper.hxx>

#include <tools/gen.hxx>
#include <driverblocklist.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/salgtype.hxx>

#include <premac.h>
#include <SkRegion.h>
#include <SkSurface.h>
#include <tools/sk_app/WindowContext.h>
#include <postmac.h>

#include <string_view>

namespace SkiaHelper
{
// Get the one shared GrDirectContext instance.
GrDirectContext* getSharedGrDirectContext();

void disableRenderMethod(RenderMethod method);

// Create SkSurface, GPU-backed if possible.
VCL_DLLPUBLIC sk_sp<SkSurface> createSkSurface(int width, int height,
                                               SkColorType type = kN32_SkColorType,
                                               SkAlphaType alpha = kPremul_SkAlphaType);

inline sk_sp<SkSurface> createSkSurface(const Size& size, SkColorType type = kN32_SkColorType,
                                        SkAlphaType alpha = kPremul_SkAlphaType)
{
    return createSkSurface(size.Width(), size.Height(), type, alpha);
}

inline sk_sp<SkSurface> createSkSurface(int width, int height, SkAlphaType alpha)
{
    return createSkSurface(width, height, kN32_SkColorType, alpha);
}

inline sk_sp<SkSurface> createSkSurface(const Size& size, SkAlphaType alpha)
{
    return createSkSurface(size.Width(), size.Height(), kN32_SkColorType, alpha);
}

// Create SkImage, GPU-backed if possible.
VCL_DLLPUBLIC sk_sp<SkImage> createSkImage(const SkBitmap& bitmap);

// Call surface->makeImageSnapshot() and abort on failure.
VCL_DLLPUBLIC sk_sp<SkImage> makeCheckedImageSnapshot(sk_sp<SkSurface> surface);
VCL_DLLPUBLIC sk_sp<SkImage> makeCheckedImageSnapshot(sk_sp<SkSurface> surface,
                                                      const SkIRect& bounds);

inline Size imageSize(const sk_sp<SkImage>& image) { return Size(image->width(), image->height()); }

inline SkColor toSkColor(Color color)
{
    return SkColorSetARGB(color.GetAlpha(), color.GetRed(), color.GetGreen(), color.GetBlue());
}

inline SkColor toSkColorWithTransparency(Color aColor, double fTransparency)
{
    return SkColorSetA(toSkColor(aColor), 255 * (1.0 - fTransparency));
}

inline SkColor toSkColorWithIntensity(Color color, int intensity)
{
    return SkColorSetARGB(color.GetAlpha(), color.GetRed() * intensity / 100,
                          color.GetGreen() * intensity / 100, color.GetBlue() * intensity / 100);
}

inline Color fromSkColor(SkColor color)
{
    return Color(ColorAlpha, SkColorGetA(color), SkColorGetR(color), SkColorGetG(color),
                 SkColorGetB(color));
}

// Whether to use GetSkImage() that checks for delayed scaling or whether to access
// the stored image directly without checks.
enum DirectImage
{
    Yes,
    No
};

// Sets SkBlender that will do an invert operation.
void setBlenderInvert(SkPaint* paint);
// Sets SkBlender that will do a xor operation.
void setBlenderXor(SkPaint* paint);

// Must be called in any VCL backend before any Skia functionality is used.
// If not set, Skia will be disabled.
VCL_DLLPUBLIC void
    prepareSkia(std::unique_ptr<sk_app::WindowContext> (*createGpuWindowContext)(bool));

// Shared cache of images.
void addCachedImage(const OString& key, sk_sp<SkImage> image);
sk_sp<SkImage> findCachedImage(const OString& key);
void removeCachedImage(sk_sp<SkImage> image);
tools::Long maxImageCacheSize();

// Get checksum of the image content, only for raster images. Is cached,
// but may still be somewhat expensive.
uint32_t getSkImageChecksum(sk_sp<SkImage> image);

// SkSurfaceProps to be used by all Skia surfaces.
VCL_DLLPUBLIC const SkSurfaceProps* surfaceProps();
// Set pixel geometry to be used by SkSurfaceProps.
VCL_DLLPUBLIC void setPixelGeometry(SkPixelGeometry pixelGeometry);

inline bool isUnitTestRunning(const char* name = nullptr)
{
    if (name == nullptr)
    {
        static const char* const testname = getenv("LO_TESTNAME");
        return testname != nullptr;
    }
    const char* const testname = getenv("LO_TESTNAME");
    return testname != nullptr && std::string_view(name) == testname;
}

// Scaling done on the GPU is fast, but bicubic done in raster mode can be slow
// if done too much, and it generally shouldn't be needed for to-screen drawing.
// In that case use only BmpScaleFlag::Default, which is bilinear+mipmap,
// which should be good enough (and that's what the "super" bitmap scaling
// algorithm done by VCL does as well).
inline BmpScaleFlag goodScalingQuality(bool isGPU)
{
    return isGPU ? BmpScaleFlag::BestQuality : BmpScaleFlag::Default;
}

// Normal scaling algorithms have a poor quality when downscaling a lot.
// https://bugs.chromium.org/p/skia/issues/detail?id=11810 suggests to use mipmaps
// in such a case, which is annoying to do explicitly instead of Skia deciding which
// algorithm would be the best, but now with Skia removing SkFilterQuality and requiring
// explicitly being told what algorithm to use this appears to be the best we can do.
// Anything scaled down at least this ratio will use linear+mipmaps.
constexpr int downscaleRatioThreshold = 4;

inline SkSamplingOptions makeSamplingOptions(BmpScaleFlag scalingType, SkMatrix matrix,
                                             int scalingFactor)
{
    switch (scalingType)
    {
        case BmpScaleFlag::BestQuality:
            if (scalingFactor != 1)
                matrix.postScale(scalingFactor, scalingFactor);
            if (matrix.getScaleX() <= 1.0 / downscaleRatioThreshold
                || matrix.getScaleY() <= 1.0 / downscaleRatioThreshold)
                return SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear);
            return SkSamplingOptions(SkCubicResampler::Mitchell());
        case BmpScaleFlag::Default:
            // Use SkMipmapMode::kNearest for better quality when downscaling. SkMipmapMode::kLinear
            // would be even better, but it is not specially optimized in raster mode.
            return SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest);
        case BmpScaleFlag::Fast:
        case BmpScaleFlag::NearestNeighbor:
            return SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone);
        default:
            assert(false);
            return SkSamplingOptions();
    }
}

inline SkSamplingOptions makeSamplingOptions(BmpScaleFlag scalingType, const Size& srcSize,
                                             Size destSize, int scalingFactor)
{
    switch (scalingType)
    {
        case BmpScaleFlag::BestQuality:
            if (scalingFactor != 1)
                destSize *= scalingFactor;
            if (srcSize.Width() / destSize.Width() >= downscaleRatioThreshold
                || srcSize.Height() / destSize.Height() >= downscaleRatioThreshold)
                return SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear);
            return SkSamplingOptions(SkCubicResampler::Mitchell());
        case BmpScaleFlag::Default:
            // As in the first overload, use kNearest.
            return SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest);
        case BmpScaleFlag::Fast:
        case BmpScaleFlag::NearestNeighbor:
            return SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone);
        default:
            assert(false);
            return SkSamplingOptions();
    }
}

inline SkSamplingOptions makeSamplingOptions(const SalTwoRect& rPosAry, int scalingFactor,
                                             int srcScalingFactor, bool isGPU)
{
    // If there will be scaling, make it smooth, but not in unittests, as those often
    // require exact color values and would be confused by this.
    if (isUnitTestRunning())
        return SkSamplingOptions(); // none
    Size srcSize(rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
    Size destSize(rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    if (scalingFactor != 1)
        destSize *= scalingFactor;
    if (srcScalingFactor != 1)
        srcSize *= srcScalingFactor;
    if (srcSize != destSize)
        return makeSamplingOptions(goodScalingQuality(isGPU), srcSize, destSize, 1);
    return SkSamplingOptions(); // none
}

inline SkRect scaleRect(const SkRect& rect, int scaling)
{
    return SkRect::MakeXYWH(rect.x() * scaling, rect.y() * scaling, rect.width() * scaling,
                            rect.height() * scaling);
}

inline SkIRect scaleRect(const SkIRect& rect, int scaling)
{
    return SkIRect::MakeXYWH(rect.x() * scaling, rect.y() * scaling, rect.width() * scaling,
                             rect.height() * scaling);
}

#ifdef DBG_UTIL
void prefillSurface(const sk_sp<SkSurface>& surface);
#endif

VCL_DLLPUBLIC void dump(const SkBitmap& bitmap, const char* file);
VCL_DLLPUBLIC void dump(const sk_sp<SkImage>& image, const char* file);
VCL_DLLPUBLIC void dump(const sk_sp<SkSurface>& surface, const char* file);

VCL_DLLPUBLIC extern uint32_t vendorId;

inline DriverBlocklist::DeviceVendor getVendor()
{
    return DriverBlocklist::GetVendorFromId(vendorId);
}

} // namespace SkiaHelper

// For unittests.
namespace SkiaTests
{
VCL_DLLPUBLIC bool matrixNeedsHighQuality(const SkMatrix& matrix);
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const SkRect& rectangle)
{
    if (rectangle.isEmpty())
        return stream << "EMPTY";
    else
        return stream << rectangle.width() << 'x' << rectangle.height() << "@(" << rectangle.x()
                      << ',' << rectangle.y() << ")";
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const SkIRect& rectangle)
{
    if (rectangle.isEmpty())
        return stream << "EMPTY";
    else
        return stream << rectangle.width() << 'x' << rectangle.height() << "@(" << rectangle.x()
                      << ',' << rectangle.y() << ")";
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const SkRegion& region)
{
    if (region.isEmpty())
        return stream << "EMPTY";
    stream << "(";
    SkRegion::Iterator it(region);
    for (int i = 0; !it.done(); it.next(), ++i)
        stream << "[" << i << "] " << it.rect();
    stream << ")";
    return stream;
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const SkMatrix& matrix)
{
    return stream << "[" << matrix[0] << " " << matrix[1] << " " << matrix[2] << "]"
                  << "[" << matrix[3] << " " << matrix[4] << " " << matrix[5] << "]"
                  << "[" << matrix[6] << " " << matrix[7] << " " << matrix[8] << "]";
}

template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const SkImage& image)
{
    // G - on GPU
    return stream << static_cast<const void*>(&image) << " " << Size(image.width(), image.height())
                  << "/" << (SkColorTypeBytesPerPixel(image.imageInfo().colorType()) * 8)
                  << (image.isTextureBacked() ? "G" : "");
}
template <typename charT, typename traits>
inline std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& stream,
                                                     const sk_sp<SkImage>& image)
{
    if (image == nullptr)
        return stream << "(null)";
    return stream << *image;
}

#endif // INCLUDED_VCL_INC_SKIA_UTILS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
