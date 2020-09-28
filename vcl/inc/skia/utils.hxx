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

#include <SkRegion.h>
#include <tools/sk_app/VulkanWindowContext.h>

namespace SkiaHelper
{
// Get the one shared GrDirectContext instance.
GrDirectContext* getSharedGrDirectContext();

void disableRenderMethod(RenderMethod method);

// Create SkSurface, GPU-backed if possible.
VCL_DLLPUBLIC sk_sp<SkSurface> createSkSurface(int width, int height,
                                               SkColorType type = kN32_SkColorType);

inline sk_sp<SkSurface> createSkSurface(const Size& size, SkColorType type = kN32_SkColorType)
{
    return createSkSurface(size.Width(), size.Height(), type);
}

// Create SkImage, GPU-backed if possible.
VCL_DLLPUBLIC sk_sp<SkImage> createSkImage(const SkBitmap& bitmap);

// Call surface->makeImageSnapshot() and abort on failure.
VCL_DLLPUBLIC sk_sp<SkImage> makeCheckedImageSnapshot(sk_sp<SkSurface> surface);
VCL_DLLPUBLIC sk_sp<SkImage> makeCheckedImageSnapshot(sk_sp<SkSurface> surface,
                                                      const SkIRect& bounds);

// Must be called in any VCL backend before any Skia functionality is used.
// If not set, Skia will be disabled.
VCL_DLLPUBLIC void
    prepareSkia(std::unique_ptr<sk_app::WindowContext> (*createVulkanWindowContext)(bool));

// Shared cache of images.
void addCachedImage(const OString& key, sk_sp<SkImage> image);
sk_sp<SkImage> findCachedImage(const OString& key);
void removeCachedImage(sk_sp<SkImage> image);
constexpr int MAX_CACHE_SIZE = 4 * 2000 * 2000 * 4; // 4x 2000px 32bpp images, 64MiB

#ifdef DBG_UTIL
void prefillSurface(sk_sp<SkSurface>& surface);
VCL_DLLPUBLIC void dump(const SkBitmap& bitmap, const char* file);
VCL_DLLPUBLIC void dump(const sk_sp<SkImage>& image, const char* file);
VCL_DLLPUBLIC void dump(const sk_sp<SkSurface>& surface, const char* file);
#endif

VCL_DLLPUBLIC extern uint32_t vendorId;

inline DriverBlocklist::DeviceVendor getVendor()
{
    return DriverBlocklist::GetVendorFromId(vendorId);
}

} // namespace

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
