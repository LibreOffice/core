/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/filter/PngImageWriter.hxx>
#include <png.h>
#include <bitmap/BitmapWriteAccess.hxx>
#include <vcl/bitmap.hxx>

namespace vcl
{
static void lclWriteStream(png_structp pPng, png_bytep pData, png_size_t pDataSize)
{
    png_voidp pIO = png_get_io_ptr(pPng);

    if (pIO == nullptr)
        return;

    SvStream* pStream = static_cast<SvStream*>(pIO);

    sal_Size nBytesWritten = pStream->WriteBytes(pData, pDataSize);

    if (nBytesWritten != pDataSize)
        png_error(pPng, "Write Error");
}

bool pngWrite(SvStream& rStream, BitmapEx& rBitmapEx, int nCompressionLevel)
{
    png_structp pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!pPng)
        return false;

    png_infop pInfo = png_create_info_struct(pPng);
    if (!pInfo)
    {
        png_destroy_write_struct(&pPng, nullptr);
        return false;
    }

    Bitmap aBitmap;

    if (setjmp(png_jmpbuf(pPng)))
    {
        png_destroy_read_struct(&pPng, &pInfo, nullptr);
        return false;
    }

    // Set our custom stream writer
    png_set_write_fn(pPng, &rStream, lclWriteStream, nullptr);

    aBitmap = rBitmapEx.GetBitmap();

    {
        Bitmap::ScopedReadAccess pAccess(aBitmap);
        Size aSize = rBitmapEx.GetSizePixel();

        int bitDepth = -1;
        int colorType = -1;

        /* PNG_COLOR_TYPE_GRAY (1, 2, 4, 8, 16)
           PNG_COLOR_TYPE_GRAY_ALPHA (8, 16)
           PNG_COLOR_TYPE_PALETTE (bit depths 1, 2, 4, 8)
           PNG_COLOR_TYPE_RGB (bit_depths 8, 16)
           PNG_COLOR_TYPE_RGB_ALPHA (bit_depths 8, 16)
           PNG_COLOR_MASK_PALETTE
           PNG_COLOR_MASK_COLOR
           PNG_COLOR_MASK_ALPHA
        */
        auto eScanlineFormat = pAccess->GetScanlineFormat();
        if (eScanlineFormat == ScanlineFormat::N8BitPal && aBitmap.HasGreyPalette8Bit())
        {
            colorType = PNG_COLOR_TYPE_GRAY;
            bitDepth = 8;
        }
        else if (eScanlineFormat == ScanlineFormat::N24BitTcBgr
                 || eScanlineFormat == ScanlineFormat::N24BitTcRgb)
        {
            colorType = PNG_COLOR_TYPE_RGB;
            bitDepth = 8;
            if (eScanlineFormat == ScanlineFormat::N24BitTcBgr)
                png_set_bgr(pPng);
        }
        else
        {
            return false;
        }

        png_set_compression_level(pPng, nCompressionLevel);

        int interlaceType = PNG_INTERLACE_NONE;
        int compressionType = PNG_COMPRESSION_TYPE_DEFAULT;
        int filterMethod = PNG_FILTER_TYPE_DEFAULT;

        png_set_IHDR(pPng, pInfo, aSize.Width(), aSize.Height(), bitDepth, colorType, interlaceType,
                     compressionType, filterMethod);

        png_write_info(pPng, pInfo);

        int nNumberOfPasses = 1;

        Scanline pSourcePointer;

        tools::Long nHeight = pAccess->Height();

        for (int nPass = 0; nPass < nNumberOfPasses; nPass++)
        {
            for (tools::Long y = 0; y <= nHeight; y++)
            {
                pSourcePointer = pAccess->GetScanline(y);
                png_write_rows(pPng, &pSourcePointer, 1);
            }
        }
    }

    png_write_end(pPng, pInfo);

    png_destroy_write_struct(&pPng, &pInfo);

    return true;
}

PngImageWriter::PngImageWriter(SvStream& rStream)
    : mrStream(rStream)
    , mnCompressionLevel(6)
    , mbInterlaced(false)
{
}

bool PngImageWriter::write(BitmapEx& rBitmapEx)
{
    return pngWrite(mrStream, rBitmapEx, mnCompressionLevel);
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
