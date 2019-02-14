/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/filter/PngImageReader.hxx>
#include <png.h>
#include <vcl/bitmapaccess.hxx>
#include <bitmapwriteaccess.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/alpha.hxx>

namespace
{
void lclReadStream(png_structp pPng, png_bytep pOutBytes, png_size_t nBytesToRead)
{
    png_voidp pIO = png_get_io_ptr(pPng);

    if (pIO == nullptr)
        return;

    SvStream* pStream = static_cast<SvStream*>(pIO);

    sal_Size nBytesRead = pStream->ReadBytes(pOutBytes, nBytesToRead);

    if (nBytesRead != nBytesToRead)
        png_error(pPng, "Error reading");
}

bool reader(SvStream& rStream, BitmapEx& rBitmapEx)
{
    enum
    {
        PNG_SIGNATURE_SIZE = 8
    };

    sal_uInt8 aHeader[PNG_SIGNATURE_SIZE];
    rStream.ReadBytes(aHeader, PNG_SIGNATURE_SIZE);

    if (png_sig_cmp(aHeader, 0, PNG_SIGNATURE_SIZE))
        return false;

    png_structp pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!pPng)
        return false;

    png_infop pInfo = png_create_info_struct(pPng);
    if (!pInfo)
    {
        png_destroy_read_struct(&pPng, nullptr, nullptr);
        return false;
    }

    if (setjmp(png_jmpbuf(pPng)))
    {
        png_destroy_read_struct(&pPng, &pInfo, nullptr);
        return false;
    }

    png_set_read_fn(pPng, &rStream, lclReadStream);

    png_set_crc_action(pPng, PNG_CRC_WARN_USE, PNG_CRC_WARN_DISCARD);

    png_set_sig_bytes(pPng, PNG_SIGNATURE_SIZE);

    png_read_info(pPng, pInfo);

    png_uint_32 width = 0;
    png_uint_32 height = 0;
    int bitDepth = 0;
    int colorType = -1;
    int interlace = -1;

    png_uint_32 returnValue = png_get_IHDR(pPng, pInfo, &width, &height, &bitDepth, &colorType,
                                           &interlace, nullptr, nullptr);

    if (returnValue != 1)
    {
        png_destroy_read_struct(&pPng, &pInfo, nullptr);
        return false;
    }

    Bitmap aBitmap(Size(width, height), 24);
    AlphaMask aBitmapAlpha(Size(width, height), nullptr);

    if (colorType == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(pPng);

    if (colorType == PNG_COLOR_TYPE_GRAY)
        png_set_expand_gray_1_2_4_to_8(pPng);

    if (png_get_valid(pPng, pInfo, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(pPng);

    if (bitDepth == 16)
        png_set_scale_16(pPng);

    if (bitDepth < 8)
        png_set_packing(pPng);

    if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        png_set_gray_to_rgb(pPng);
    }

    // Sets the filler byte - if RGB it converts to RGBA
    // png_set_filler(pPng, 0xFF, PNG_FILLER_AFTER);

    int nNumberOfPasses = png_set_interlace_handling(pPng);

    png_read_update_info(pPng, pInfo);
    returnValue = png_get_IHDR(pPng, pInfo, &width, &height, &bitDepth, &colorType, nullptr,
                               nullptr, nullptr);

    if (returnValue != 1)
    {
        png_destroy_read_struct(&pPng, &pInfo, nullptr);
        return false;
    }

    if (bitDepth != 8
        || !(colorType == PNG_COLOR_TYPE_RGB || colorType == PNG_COLOR_TYPE_RGB_ALPHA))
    {
        png_destroy_read_struct(&pPng, &pInfo, nullptr);
        return false;
    }

    {
        if (colorType == PNG_COLOR_TYPE_RGB)
        {
            size_t aRowSizeBytes = png_get_rowbytes(pPng, pInfo);

            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            ScanlineFormat eFormat = pWriteAccess->GetScanlineFormat();
            if (eFormat == ScanlineFormat::N24BitTcBgr)
                png_set_bgr(pPng);

            std::vector<png_byte> aRow(aRowSizeBytes, 0);

            for (int pass = 0; pass < nNumberOfPasses; pass++)
            {
                for (png_uint_32 y = 0; y < height; y++)
                {
                    Scanline pScanline = pWriteAccess->GetScanline(y);
                    png_bytep pRow = aRow.data();
                    png_read_rows(pPng, &pRow, nullptr, 1);
                    size_t iColor = 0;
                    for (size_t i = 0; i < aRowSizeBytes; i += 3)
                    {
                        pScanline[iColor++] = pRow[i + 0];
                        pScanline[iColor++] = pRow[i + 1];
                        pScanline[iColor++] = pRow[i + 2];
                    }
                }
            }
        }
        else if (colorType == PNG_COLOR_TYPE_RGB_ALPHA)
        {
            size_t aRowSizeBytes = png_get_rowbytes(pPng, pInfo);

            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            AlphaScopedWriteAccess pWriteAccessAlpha(aBitmapAlpha);

            ScanlineFormat eFormat = pWriteAccess->GetScanlineFormat();
            if (eFormat == ScanlineFormat::N24BitTcBgr)
                png_set_bgr(pPng);

            std::vector<png_byte> aRow(aRowSizeBytes, 0);

            for (int pass = 0; pass < nNumberOfPasses; pass++)
            {
                for (png_uint_32 y = 0; y < height; y++)
                {
                    Scanline pScanAlpha = pWriteAccessAlpha->GetScanline(y);
                    Scanline pScanline = pWriteAccess->GetScanline(y);
                    png_bytep pRow = aRow.data();
                    png_read_rows(pPng, &pRow, nullptr, 1);
                    size_t iAlpha = 0;
                    size_t iColor = 0;
                    for (size_t i = 0; i < aRowSizeBytes; i += 4)
                    {
                        pScanline[iColor++] = pRow[i + 0];
                        pScanline[iColor++] = pRow[i + 1];
                        pScanline[iColor++] = pRow[i + 2];
                        pScanAlpha[iAlpha++] = 0xFF - pRow[i + 3];
                    }
                }
            }
        }
    }

    png_read_end(pPng, pInfo);

    png_destroy_read_struct(&pPng, &pInfo, nullptr);

    rBitmapEx = BitmapEx(aBitmap, aBitmapAlpha);

    return true;
}

} // anonymous namespace

namespace vcl
{
PngImageReader::PngImageReader(SvStream& rStream)
    : mrStream(rStream)
{
}

bool PngImageReader::read(BitmapEx& rBitmapEx) { return reader(mrStream, rBitmapEx); }

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
