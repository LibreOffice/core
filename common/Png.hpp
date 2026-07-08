/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2003 University of Southern California
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is University of Southern
 * California.
 *
 * Contributor(s):
 *        Carl D. Worth <cworth@cworth.org>
 *        Kristian Høgsberg <krh@redhat.com>
 *        Chris Wilson <chris@chris-wilson.co.uk>
 */

#define PNG_SKIP_SETJMP_CHECK
#include <png.h>
#include <zlib.h>

#include <cassert>
#include <chrono>
#include <iomanip>
#include <fstream>

#include <common/Log.hpp>
#include <common/TraceEvent.hpp>

namespace Png
{

// Callback functions for libpng
extern "C"
{
    static void user_write_status_fn(png_structp, png_uint_32, int)
    {
    }

    static void user_write_fn(png_structp png_ptr, png_bytep data, png_size_t length)
    {
        std::vector<char>* outputp = static_cast<std::vector<char>*>(png_get_io_ptr(png_ptr));
        const size_t oldsize = outputp->size();
        outputp->resize(oldsize + length);
        std::memcpy(outputp->data() + oldsize, data, length);
    }

    static void user_flush_fn(png_structp)
    {
    }
}


/* Unpremultiplies data and converts native endian BGRA => RGBA bytes */
static void
unpremultiply_bgra_data (png_structp /*png*/, png_row_infop row_info, png_bytep data)
{
    unsigned int i;

    for (i = 0; i < row_info->rowbytes; i += 4)
    {
        uint8_t *b = &data[i];
        uint32_t pix;
        uint8_t  alpha;

        std::memcpy (&pix, b, sizeof (uint32_t));

        alpha = (pix & 0xff000000) >> 24;
        if (alpha == 255)
        {
            b[0] = ((pix & 0xff0000) >> 16);
            b[1] = ((pix & 0x00ff00) >>  8);
            b[2] = ((pix & 0x0000ff) >>  0);
            b[3] = 255;
        }
        else if (alpha == 0)
        {
            b[0] = b[1] = b[2] = b[3] = 0;
        }
        else
        {
            b[0] = (((pix & 0xff0000) >> 16) * 255 + alpha / 2) / alpha;
            b[1] = (((pix & 0x00ff00) >>  8) * 255 + alpha / 2) / alpha;
            b[2] = (((pix & 0x0000ff) >>  0) * 255 + alpha / 2) / alpha;
            b[3] = alpha;
        }
    }
}

/* Unpremultiplies data already in RGBA order*/
static void
unpremultiply_rgba_data (png_structp /*png*/, png_row_infop row_info, png_bytep data)
{
    unsigned int i;

    for (i = 0; i < row_info->rowbytes; i += 4)
    {
        uint8_t *b = &data[i];
        uint32_t pix;
        uint8_t  alpha;

        std::memcpy (&pix, b, sizeof (uint32_t));

        alpha = (pix & 0xff000000) >> 24;
        if (alpha == 255)
        {
            std::memcpy(b, &pix, sizeof (uint32_t));
        }
        else if (alpha == 0)
        {
            b[0] = b[1] = b[2] = b[3] = 0;
        }
        else
        {
            b[0] = (((pix & 0x0000ff) >>  0) * 255 + alpha / 2) / alpha;
            b[1] = (((pix & 0x00ff00) >>  8) * 255 + alpha / 2) / alpha;
            b[2] = (((pix & 0xff0000) >> 16) * 255 + alpha / 2) / alpha;
            b[3] = alpha;
        }
    }
}


/// This function uses setjmp which may clobbers non-trivial objects.
/// So we can't use logging or create complex C++ objects in this frame.
/// Specifically, logging uses std::string objects, and GCC gives the following:
/// error: variable ‘__capacity’ might be clobbered by ‘longjmp’ or ‘vfork’ [-Werror=clobbered]
/// In practice, this is bogus, since __capacity is has a trivial type, but this error
/// shows up with the sanitizers. But technically we shouldn't mix C++ objects with setjmp.
/// Sadly, older libpng headers don't use const for the pixmap pointer parameter to
/// png_write_row(), so can't use const here for pixmap.
inline bool impl_encodeSubBufferToPNG(unsigned char* pixmap, size_t startX, size_t startY,
                                      int width, int height, int bufferWidth, int bufferHeight,
                                      std::vector<char>& output, COKitTileMode mode)
{
    if (bufferWidth < width || bufferHeight < height)
    {
        return false;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        png_destroy_write_struct(&png_ptr, nullptr);
        return false;
    }

    if constexpr (Util::isMobileApp())
        png_set_compression_level(png_ptr, Z_BEST_SPEED);
    else
    {
        // Level 4 gives virtually identical compression
        // ratio to level 6, but is between 5-10% faster.
        // Level 3 runs almost twice as fast, but the
        // output is typically 2-3x larger.
        png_set_compression_level(png_ptr, 4);
    }

    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_set_write_fn(png_ptr, &output, user_write_fn, user_flush_fn);
    png_set_write_status_fn(png_ptr, user_write_status_fn);

    png_write_info(png_ptr, info_ptr);

    switch (mode)
    {
        case KIT_TILEMODE_BGRA:
            png_set_write_user_transform_fn (png_ptr, unpremultiply_bgra_data);
            break;
        case KIT_TILEMODE_RGBA:
            png_set_write_user_transform_fn (png_ptr, unpremultiply_rgba_data);
            break;
    }

    for (int y = 0; y < height; ++y)
    {
        size_t position = ((startY + y) * bufferWidth * 4) + (startX * 4);
        png_write_row(png_ptr, pixmap + position);
    }

    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    return true;
}

/// Sadly, older libpng headers don't use const for the pixmap pointer parameter to
/// png_write_row(), so can't use const here for pixmap.
inline bool encodeSubBufferToPNG(unsigned char* pixmap, size_t startX, size_t startY, int width,
                                 int height, int bufferWidth, int bufferHeight,
                                 std::vector<char>& output, COKitTileMode mode)
{
    ProfileZone pz("encodeSubBufferToPNG");

    const auto start = std::chrono::steady_clock::now();

    const bool res = impl_encodeSubBufferToPNG(pixmap, startX, startY, width, height, bufferWidth,
                                               bufferHeight, output, mode);
    if (Log::traceEnabled())
    {
        const auto end = std::chrono::steady_clock::now();

        std::chrono::milliseconds duration
            = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        static std::chrono::milliseconds totalDuration;
        static int calls = 0;

        totalDuration += duration;
        ++calls;

        static uint64_t totalPixelBytes = 0;
        static uint64_t totalOutputBytes = 0;

        totalPixelBytes += (static_cast<uint64_t>(width) * height * 4);
        totalOutputBytes += output.size();

        LOG_TRC("PNG compression took "
                << duration << " (" << output.size() << " bytes from " << (width * height * 4) << "). Average after " << calls
                << " calls: " << (totalDuration.count() / static_cast<double>(calls)) << "ms, "
                << (totalOutputBytes / static_cast<double>(calls)) << " bytes, "
                << std::setprecision(2) << (100. * totalOutputBytes / totalPixelBytes) << "% compression.");
    }

    return res;
}

inline
bool encodeBufferToPNG(unsigned char* pixmap, int width, int height,
                       std::vector<char>& output, COKitTileMode mode)
{
    return encodeSubBufferToPNG(pixmap, 0, 0, width, height, width, height, output, mode);
}

static
void readTileData(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    assert(io_ptr);
    std::stringstream& streamTile = *static_cast<std::stringstream*>(io_ptr);
    streamTile.read(reinterpret_cast<char*>(data), length);
}

inline
std::vector<png_bytep> decodePNG(std::stringstream& stream, png_uint_32& height, png_uint_32& width, png_uint_32& rowBytes)
{
    png_byte signature[0x08];
    stream.read(reinterpret_cast<char *>(signature), 0x08);
    if (png_sig_cmp(signature, 0x00, 0x08))
    {
        throw std::runtime_error("Invalid PNG signature.");
    }

    png_structp ptrPNG = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (ptrPNG == nullptr)
    {
        throw std::runtime_error("png_create_read_struct failed.");
    }

    png_infop ptrInfo = png_create_info_struct(ptrPNG);
    if (ptrInfo == nullptr)
    {
        throw std::runtime_error("png_create_info_struct failed.");
    }

    png_infop ptrEnd = png_create_info_struct(ptrPNG);
    if (ptrEnd == nullptr)
    {
        throw std::runtime_error("png_create_info_struct failed.");
    }

    png_set_read_fn(ptrPNG, &stream, readTileData);
    png_set_sig_bytes(ptrPNG, 0x08);

    png_read_info(ptrPNG, ptrInfo);

    width = png_get_image_width(ptrPNG, ptrInfo);
    height = png_get_image_height(ptrPNG, ptrInfo);

    png_set_interlace_handling(ptrPNG);
    png_read_update_info(ptrPNG, ptrInfo);

    rowBytes = png_get_rowbytes(ptrPNG, ptrInfo);
    assert(width == rowBytes / 4);

    const size_t dataSize = (rowBytes + sizeof(png_bytep)) * height / sizeof(png_bytep);
    const size_t size = dataSize + height + sizeof(png_bytep);

    std::vector<png_bytep> rows;
    rows.resize(size);

    // rows
    for (png_uint_32 itRow = 0; itRow < height; itRow++)
    {
        const size_t index = height + (itRow * rowBytes + sizeof(png_bytep) - 1) / sizeof(png_bytep);
        rows[itRow] = reinterpret_cast<png_bytep>(&rows[index]);
    }

    png_read_image(ptrPNG, rows.data());
    png_read_end(ptrPNG, ptrEnd);
    png_destroy_read_struct(&ptrPNG, &ptrInfo, &ptrEnd);

    return rows;
}

inline std::vector<char> loadPng(const char *relpath,
                                 uint32_t& height,
                                 uint32_t& width,
                                 uint32_t& rowBytes)
{
    std::ifstream file(relpath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    png_uint_32 h, w, rb;
    std::vector<png_bytep> rows = decodePNG(buffer, h, w, rb);
    height = h; width = w; rowBytes = rb;
    std::vector<char> output;
    for (png_uint_32 y = 0; y < height; ++y)
        for (png_uint_32 i = 0; i < width * 4; ++i)
            output.push_back(rows[y][i]);
    return output;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
