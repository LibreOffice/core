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
#include <rtl/crc.h>
#include <tools/stream.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/alpha.hxx>
#include <vcl/BitmapTools.hxx>
#include <unotools/configmgr.hxx>

#include <bitmap/BitmapWriteAccess.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

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
    {
        // Make sure to not reuse old data (could cause infinite loop).
        memset(pOutBytes + nBytesRead, 0, nBytesToRead - nBytesRead);
        png_warning(pPng, "Error reading");
    }
}

constexpr int PNG_SIGNATURE_SIZE = 8;

bool isPng(SvStream& rStream)
{
    // Check signature bytes
    sal_uInt8 aHeader[PNG_SIGNATURE_SIZE];
    rStream.ReadBytes(aHeader, PNG_SIGNATURE_SIZE);

    return png_sig_cmp(aHeader, 0, PNG_SIGNATURE_SIZE) == 0;
}

bool reader(SvStream& rStream, BitmapEx& rBitmapEx, bool bUseBitmap32)
{
    if (!isPng(rStream))
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

    // All variables holding resources need to be declared here in order to be
    // properly cleaned up in case of an error, otherwise libpng's longjmp()
    // jumps over the destructor calls.
    Bitmap aBitmap;
    AlphaMask aBitmapAlpha;
    Size prefSize;
    BitmapScopedWriteAccess pWriteAccess;
    AlphaScopedWriteAccess pWriteAccessAlpha;
    std::vector<std::vector<png_byte>> aRows;

    if (setjmp(png_jmpbuf(pPng)))
    {
        png_destroy_read_struct(&pPng, &pInfo, nullptr);
        // Set the bitmap if it contains something, even on failure. This allows
        // reading images that are only partially broken.
        pWriteAccess.reset();
        pWriteAccessAlpha.reset();
        if (!aBitmap.IsEmpty() && !aBitmapAlpha.IsEmpty())
            rBitmapEx = BitmapEx(aBitmap, aBitmapAlpha);
        else if (!aBitmap.IsEmpty())
            rBitmapEx = BitmapEx(aBitmap);
        if (!rBitmapEx.IsEmpty() && !prefSize.IsEmpty())
        {
            rBitmapEx.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
            rBitmapEx.SetPrefSize(prefSize);
        }
        return false;
    }

    png_set_option(pPng, PNG_MAXIMUM_INFLATE_WINDOW, PNG_OPTION_ON);

    png_set_read_fn(pPng, &rStream, lclReadStream);

    if (!utl::ConfigManager::IsFuzzing())
        png_set_crc_action(pPng, PNG_CRC_ERROR_QUIT, PNG_CRC_WARN_DISCARD);
    else
        png_set_crc_action(pPng, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);

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

    // Convert gray+alpha to RGBA, keep gray as gray.
    if (colorType == PNG_COLOR_TYPE_GRAY_ALPHA
        || (colorType == PNG_COLOR_TYPE_GRAY && png_get_valid(pPng, pInfo, PNG_INFO_tRNS)))
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
        || (colorType != PNG_COLOR_TYPE_RGB && colorType != PNG_COLOR_TYPE_RGB_ALPHA
            && colorType != PNG_COLOR_TYPE_GRAY))
    {
        png_destroy_read_struct(&pPng, &pInfo, nullptr);
        return false;
    }

    png_uint_32 res_x = 0;
    png_uint_32 res_y = 0;
    int unit_type = 0;
    if (png_get_pHYs(pPng, pInfo, &res_x, &res_y, &unit_type) != 0
        && unit_type == PNG_RESOLUTION_METER && res_x && res_y)
    {
        // convert into MapUnit::Map100thMM
        prefSize = Size(static_cast<sal_Int32>((100000.0 * width) / res_x),
                        static_cast<sal_Int32>((100000.0 * height) / res_y));
    }

    {
        if (colorType == PNG_COLOR_TYPE_RGB)
        {
            aBitmap = Bitmap(Size(width, height), vcl::PixelFormat::N24_BPP);
            {
                pWriteAccess = BitmapScopedWriteAccess(aBitmap);
                if (!pWriteAccess)
                {
                    png_destroy_read_struct(&pPng, &pInfo, nullptr);
                    return false;
                }
                ScanlineFormat eFormat = pWriteAccess->GetScanlineFormat();
                if (eFormat == ScanlineFormat::N24BitTcBgr)
                    png_set_bgr(pPng);

                for (int pass = 0; pass < nNumberOfPasses; pass++)
                {
                    for (png_uint_32 y = 0; y < height; y++)
                    {
                        Scanline pScanline = pWriteAccess->GetScanline(y);
                        png_read_row(pPng, pScanline, nullptr);
                    }
                }
                pWriteAccess.reset();
            }
            rBitmapEx = BitmapEx(aBitmap);
        }
        else if (colorType == PNG_COLOR_TYPE_RGB_ALPHA)
        {
            size_t aRowSizeBytes = png_get_rowbytes(pPng, pInfo);

            if (bUseBitmap32)
            {
                aBitmap = Bitmap(Size(width, height), vcl::PixelFormat::N32_BPP);
                {
                    pWriteAccess = BitmapScopedWriteAccess(aBitmap);
                    if (!pWriteAccess)
                    {
                        png_destroy_read_struct(&pPng, &pInfo, nullptr);
                        return false;
                    }
                    ScanlineFormat eFormat = pWriteAccess->GetScanlineFormat();
                    if (eFormat == ScanlineFormat::N32BitTcAbgr
                        || eFormat == ScanlineFormat::N32BitTcBgra)
                    {
                        png_set_bgr(pPng);
                    }

                    aRows = std::vector<std::vector<png_byte>>(height);
                    for (auto& rRow : aRows)
                        rRow.resize(aRowSizeBytes, 0);

                    auto const alphaFirst = (eFormat == ScanlineFormat::N32BitTcAbgr
                                             || eFormat == ScanlineFormat::N32BitTcArgb);
                    for (int pass = 0; pass < nNumberOfPasses; pass++)
                    {
                        for (png_uint_32 y = 0; y < height; y++)
                        {
                            Scanline pScanline = pWriteAccess->GetScanline(y);
                            png_bytep pRow = aRows[y].data();
                            png_read_row(pPng, pRow, nullptr);
                            size_t iColor = 0;
                            for (size_t i = 0; i < aRowSizeBytes; i += 4)
                            {
                                sal_Int8 alpha = pRow[i + 3];
                                if (alphaFirst)
                                {
                                    pScanline[iColor++] = alpha;
                                }
                                pScanline[iColor++] = vcl::bitmap::premultiply(pRow[i + 0], alpha);
                                pScanline[iColor++] = vcl::bitmap::premultiply(pRow[i + 1], alpha);
                                pScanline[iColor++] = vcl::bitmap::premultiply(pRow[i + 2], alpha);
                                if (!alphaFirst)
                                {
                                    pScanline[iColor++] = alpha;
                                }
                            }
                        }
                    }
                    pWriteAccess.reset();
                }
                rBitmapEx = BitmapEx(aBitmap);
            }
            else
            {
                aBitmap = Bitmap(Size(width, height), vcl::PixelFormat::N24_BPP);
                aBitmapAlpha = AlphaMask(Size(width, height), nullptr);
                {
                    pWriteAccess = BitmapScopedWriteAccess(aBitmap);
                    if (!pWriteAccess)
                    {
                        png_destroy_read_struct(&pPng, &pInfo, nullptr);
                        return false;
                    }
                    ScanlineFormat eFormat = pWriteAccess->GetScanlineFormat();
                    if (eFormat == ScanlineFormat::N24BitTcBgr)
                        png_set_bgr(pPng);

                    pWriteAccessAlpha = AlphaScopedWriteAccess(aBitmapAlpha);

                    aRows = std::vector<std::vector<png_byte>>(height);
                    for (auto& rRow : aRows)
                        rRow.resize(aRowSizeBytes, 0);

                    for (int pass = 0; pass < nNumberOfPasses; pass++)
                    {
                        for (png_uint_32 y = 0; y < height; y++)
                        {
                            Scanline pScanline = pWriteAccess->GetScanline(y);
                            Scanline pScanAlpha = pWriteAccessAlpha->GetScanline(y);
                            png_bytep pRow = aRows[y].data();
                            png_read_row(pPng, pRow, nullptr);
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
                    pWriteAccess.reset();
                    pWriteAccessAlpha.reset();
                }
                rBitmapEx = BitmapEx(aBitmap, aBitmapAlpha);
            }
        }
        else if (colorType == PNG_COLOR_TYPE_GRAY)
        {
            aBitmap = Bitmap(Size(width, height), vcl::PixelFormat::N8_BPP,
                             &Bitmap::GetGreyPalette(256));
            aBitmap.Erase(COL_WHITE);
            {
                pWriteAccess = BitmapScopedWriteAccess(aBitmap);
                if (!pWriteAccess)
                {
                    png_destroy_read_struct(&pPng, &pInfo, nullptr);
                    return false;
                }

                for (int pass = 0; pass < nNumberOfPasses; pass++)
                {
                    for (png_uint_32 y = 0; y < height; y++)
                    {
                        Scanline pScanline = pWriteAccess->GetScanline(y);
                        png_read_row(pPng, pScanline, nullptr);
                    }
                }
                pWriteAccess.reset();
            }
            rBitmapEx = BitmapEx(aBitmap);
        }
    }

    png_read_end(pPng, pInfo);

    png_destroy_read_struct(&pPng, &pInfo, nullptr);

    if (!prefSize.IsEmpty())
    {
        rBitmapEx.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
        rBitmapEx.SetPrefSize(prefSize);
    }

    return true;
}

std::unique_ptr<sal_uInt8[]> getMsGifChunk(SvStream& rStream, sal_Int32* chunkSize)
{
    if (chunkSize)
        *chunkSize = 0;
    if (!isPng(rStream))
        return nullptr;
    // It's easier to read manually the contents and find the chunk than
    // try to get it using libpng.
    // https://en.wikipedia.org/wiki/Portable_Network_Graphics#File_format
    // Each chunk is: 4 bytes length, 4 bytes type, <length> bytes, 4 bytes crc
    bool ignoreCrc = utl::ConfigManager::IsFuzzing();
    for (;;)
    {
        sal_uInt32 length, type, crc;
        rStream.ReadUInt32(length);
        rStream.ReadUInt32(type);
        if (!rStream.good())
            return nullptr;
        constexpr sal_uInt32 PNGCHUNK_msOG = 0x6d734f47; // Microsoft Office Animated GIF
        constexpr sal_uInt64 MSGifHeaderSize = 11; // "MSOFFICE9.0"
        if (type == PNGCHUNK_msOG && length > MSGifHeaderSize)
        {
            // calculate chunktype CRC (swap it back to original byte order)
            sal_uInt32 typeForCrc = type;
#if defined(__LITTLEENDIAN) || defined(OSL_LITENDIAN)
            typeForCrc = OSL_SWAPDWORD(typeForCrc);
#endif
            sal_uInt32 computedCrc = rtl_crc32(0, &typeForCrc, 4);
            const sal_uInt64 pos = rStream.Tell();
            if (pos + length >= rStream.TellEnd())
                return nullptr; // broken PNG

            char msHeader[MSGifHeaderSize];
            if (rStream.ReadBytes(msHeader, MSGifHeaderSize) != MSGifHeaderSize)
                return nullptr;
            computedCrc = rtl_crc32(computedCrc, msHeader, MSGifHeaderSize);
            length -= MSGifHeaderSize;

            std::unique_ptr<sal_uInt8[]> chunk(new sal_uInt8[length]);
            if (rStream.ReadBytes(chunk.get(), length) != length)
                return nullptr;
            computedCrc = rtl_crc32(computedCrc, chunk.get(), length);
            rStream.ReadUInt32(crc);
            if (!ignoreCrc && crc != computedCrc)
                continue; // invalid chunk, ignore
            if (chunkSize)
                *chunkSize = length;
            return chunk;
        }
        if (rStream.remainingSize() < length)
            return nullptr;
        rStream.SeekRel(length);
        rStream.ReadUInt32(crc);
        constexpr sal_uInt32 PNGCHUNK_IEND = 0x49454e44;
        if (type == PNGCHUNK_IEND)
            return nullptr;
    }
}

} // anonymous namespace

namespace vcl
{
PngImageReader::PngImageReader(SvStream& rStream)
    : mrStream(rStream)
{
}

bool PngImageReader::read(BitmapEx& rBitmapEx)
{
    auto pBackendCapabilities = ImplGetSVData()->mpDefInst->GetBackendCapabilities();
    bool bSupportsBitmap32 = pBackendCapabilities->mbSupportsBitmap32;

    return reader(mrStream, rBitmapEx, bSupportsBitmap32);
}

BitmapEx PngImageReader::read()
{
    BitmapEx bitmap;
    read(bitmap);
    return bitmap;
}

std::unique_ptr<sal_uInt8[]> PngImageReader::getMicrosoftGifChunk(SvStream& rStream,
                                                                  sal_Int32* chunkSize)
{
    sal_uInt64 originalPosition = rStream.Tell();
    SvStreamEndian originalEndian = rStream.GetEndian();
    rStream.SetEndian(SvStreamEndian::BIG);
    std::unique_ptr<sal_uInt8[]> chunk = getMsGifChunk(rStream, chunkSize);
    rStream.SetEndian(originalEndian);
    rStream.Seek(originalPosition);
    return chunk;
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
