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

namespace vcl
{
/** Pixel format of the bitmap in bits per pixel */
enum class PixelFormat
{
    INVALID = 0,
    N1_BPP = 1,
    N4_BPP = 4,
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

constexpr PixelFormat bitDepthToPixelFormat(sal_uInt16 nBitDepth)
{
    switch (nBitDepth)
    {
        case 1:
            return PixelFormat::N1_BPP;
        case 4:
            return PixelFormat::N4_BPP;
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
