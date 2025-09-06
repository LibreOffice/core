/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <sal/types.h>
#include <cassert>

namespace vcl
{
/** Pixel format of the bitmap in bits per pixel.

    The N32_BPP value here is tricky, because some of the backends do not support 3-byte pixels, and will instead
    use 4-byte pixels and ignore the 4th byte.
    So, when creating a Bitmap, it means "give me something with 3 8-bit color channels and a 8-bit alpha channel"
    But when getting the format of a bitmap, just means that the underlying bitmap has 32-bits per pixel,
    it does not necessarily mean that the underlying bitmap has an active alpha channel.
*/
enum class PixelFormat
{
    INVALID = 0,
    N8_BPP = 8,
    N24_BPP = 24,
    N32_BPP = 32
};

/** Is it a pixel format that forces creation of a palette */
constexpr bool isPalettePixelFormat(PixelFormat ePixelFormat)
{
    assert(ePixelFormat != PixelFormat::INVALID);
    return sal_uInt16(ePixelFormat) <= 8;
}

constexpr sal_uInt16 pixelFormatBitCount(PixelFormat ePixelFormat)
{
    return sal_uInt16(ePixelFormat);
}

constexpr sal_Int64 numberOfColors(PixelFormat ePixelFormat)
{
    return sal_Int64(1) << sal_Int64(ePixelFormat);
}

constexpr PixelFormat bitDepthToPixelFormat(sal_uInt16 nBitDepth)
{
    switch (nBitDepth)
    {
        case 1:
        case 4:
            assert(false && "no longer supported");
            break;
        case 8:
            return PixelFormat::N8_BPP;
        case 24:
            return PixelFormat::N24_BPP;
        case 32:
            return PixelFormat::N32_BPP;
        default:
            break;
    }
    return PixelFormat::INVALID;
}

} // end namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
