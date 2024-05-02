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
#include <comphelper/configuration.hxx>
#include <comphelper/scopeguard.hxx>
#include <osl/endian.h>
#include <vcl/BitmapWriteAccess.hxx>

#include <svdata.hxx>
#include <salinst.hxx>

#include "png.hxx"

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
        if (!nBytesRead)
            png_error(pPng, "Error reading");
        else
        {
            // Make sure to not reuse old data (could cause infinite loop).
            memset(pOutBytes + nBytesRead, 0, nBytesToRead - nBytesRead);
            png_warning(pPng, "Short read");
        }
    }
}

bool isPng(SvStream& rStream)
{
    // Check signature bytes
    sal_uInt8 aHeader[PNG_SIGNATURE_SIZE];
    if (rStream.ReadBytes(aHeader, PNG_SIGNATURE_SIZE) != PNG_SIGNATURE_SIZE)
        return false;
    return png_sig_cmp(aHeader, 0, PNG_SIGNATURE_SIZE) == 0;
}

struct PngDestructor
{
    ~PngDestructor() { png_destroy_read_struct(&pPng, &pInfo, nullptr); }
    png_structp pPng;
    png_infop pInfo;
};

/// Animation Control chunk for APNG files
struct acTLChunk
{
    sal_uInt32 num_frames;
    sal_uInt32 num_plays;
};

/// Base class for fcTL and fdAT chunks since both of these chunks use a sequence number for ordering
struct FrameDataChunk
{
    sal_uInt32 sequence_number;
    virtual ~FrameDataChunk() = default;
};

/// fcTL (Frame Control) chunk for APNG files
struct fcTLChunk : public FrameDataChunk
{
    sal_uInt32 width;
    sal_uInt32 height;
    sal_uInt32 x_offset;
    sal_uInt32 y_offset;
    sal_uInt16 delay_num;
    sal_uInt16 delay_den;
    sal_uInt8 dispose_op;
    sal_uInt8 blend_op;
};

/// fdAT (Frame Data) chunk for APNG files
struct fdATChunk : public FrameDataChunk
{
    std::vector<sal_uInt8> frame_data;
};

/// APNG chunk holder class, used for the user pointer in the libpng callback function
struct APNGInfo
{
    bool mbIsApng = false;
    acTLChunk maACTLChunk;
    std::vector<std::unique_ptr<FrameDataChunk>> maFrameData;
};

int handle_unknown_chunk(png_structp png, png_unknown_chunkp chunk)
{
    std::string sName(chunk->name, chunk->name + 4);
    APNGInfo* aAPNGInfo = static_cast<APNGInfo*>(png_get_user_chunk_ptr(png));
    if (sName == "acTL")
    {
        if (chunk->size < sizeof(acTLChunk))
            return -1;
        aAPNGInfo->maACTLChunk = *reinterpret_cast<acTLChunk*>(chunk->data);
        aAPNGInfo->maACTLChunk.num_frames = OSL_SWAPDWORD(aAPNGInfo->maACTLChunk.num_frames);
        aAPNGInfo->maACTLChunk.num_plays = OSL_SWAPDWORD(aAPNGInfo->maACTLChunk.num_plays);
        aAPNGInfo->mbIsApng = true;
    }
    else
    {
        std::unique_ptr<FrameDataChunk> pBaseChunk;

        if (sName == "fcTL")
        {
            // Can't check with sizeof(fcTLChunk) because it may not be packed
            if (chunk->size != 26)
            {
                return -1;
            }

            // byte
            //  0    sequence_number      (unsigned int)   Sequence number of the animation chunk, starting from 0
            //  4    width                (unsigned int)   Width of the following frame
            //  8    height               (unsigned int)   Height of the following frame
            //  12   x_offset             (unsigned int)   X position at which to render the following frame
            //  16   y_offset             (unsigned int)   Y position at which to render the following frame
            //  20   delay_num            (unsigned short) Frame delay fraction numerator
            //  22   delay_den            (unsigned short) Frame delay fraction denominator
            //  24   dispose_op           (byte)           Type of frame area disposal to be done after rendering this frame
            //  25   blend_op             (byte)           Type of frame area rendering for this frame

            // memcpy each member instead of reinterpret_cast because struct may not be packed
            std::unique_ptr<fcTLChunk> aChunk = std::make_unique<fcTLChunk>();
            std::memcpy(&aChunk->width, chunk->data + 4, 4);
            std::memcpy(&aChunk->height, chunk->data + 8, 4);
            std::memcpy(&aChunk->x_offset, chunk->data + 12, 4);
            std::memcpy(&aChunk->y_offset, chunk->data + 16, 4);
            std::memcpy(&aChunk->delay_num, chunk->data + 20, 2);
            std::memcpy(&aChunk->delay_den, chunk->data + 22, 2);
            std::memcpy(&aChunk->dispose_op, chunk->data + 24, 1);
            std::memcpy(&aChunk->blend_op, chunk->data + 25, 1);
            aChunk->width = OSL_SWAPDWORD(aChunk->width);
            aChunk->height = OSL_SWAPDWORD(aChunk->height);
            aChunk->x_offset = OSL_SWAPDWORD(aChunk->x_offset);
            aChunk->y_offset = OSL_SWAPDWORD(aChunk->y_offset);
            aChunk->delay_num = OSL_SWAPWORD(aChunk->delay_num);
            aChunk->delay_den = OSL_SWAPWORD(aChunk->delay_den);
            pBaseChunk = std::move(aChunk);
        }
        else if (sName == "fdAT")
        {
            size_t nDataSize = chunk->size;
            if (nDataSize < 4)
                return -1;

            std::unique_ptr<fdATChunk> aChunk = std::make_unique<fdATChunk>();
            aChunk->frame_data.resize(nDataSize);
            // Replace sequence number with the IDAT signature
            sal_uInt32 nIDATSwapped = OSL_SWAPDWORD(PNG_IDAT_SIGNATURE);
            std::memcpy(aChunk->frame_data.data(), &nIDATSwapped, 4);
            // Skip sequence number when copying
            std::memcpy(aChunk->frame_data.data() + 4, chunk->data + 4, nDataSize - 4);
            pBaseChunk = std::move(aChunk);
        }
        else
        {
            // Unknown ancillary chunk
            return 0;
        }

        sal_uInt32 nSequenceNumber = 0;
        std::memcpy(&nSequenceNumber, chunk->data, 4);
        nSequenceNumber = OSL_SWAPDWORD(nSequenceNumber);

        pBaseChunk->sequence_number = nSequenceNumber;
        if (pBaseChunk->sequence_number < aAPNGInfo->maFrameData.size())
        {
            // Make sure chunks are ordered based on their sequence number because the
            // png specification does not impose ordering restrictions on ancillary chunks
            aAPNGInfo->maFrameData.insert(aAPNGInfo->maFrameData.begin()
                                              + pBaseChunk->sequence_number,
                                          std::move(pBaseChunk));
        }
        else
        {
            aAPNGInfo->maFrameData.push_back(std::move(pBaseChunk));
        }
    }
    return 1;
}

/// Gets the important chunks (IHDR, PLTE etc.) to a stream so that a stream can be constructed for each APNG frame
void getImportantChunks(SvStream& rInStream, SvStream& rOutStream, sal_uInt32 nWidth,
                        sal_uInt32 nHeight)
{
    sal_uInt64 nPos = rInStream.Tell();
    rInStream.SetEndian(SvStreamEndian::BIG);
    rOutStream.SetEndian(SvStreamEndian::BIG);
    rOutStream.WriteUInt64(PNG_SIGNATURE);
    rOutStream.WriteUInt32(PNG_IHDR_SIZE);
    rOutStream.WriteUInt32(PNG_IHDR_SIGNATURE);
    rOutStream.WriteUInt32(nWidth);
    rOutStream.WriteUInt32(nHeight);
    rInStream.Seek(rOutStream.Tell());
    sal_uInt32 nIHDRData1;
    sal_uInt8 nIHDRData2;
    rInStream.ReadUInt32(nIHDRData1);
    rInStream.ReadUChar(nIHDRData2);
    rOutStream.WriteUInt32(nIHDRData1);
    rOutStream.WriteUChar(nIHDRData2);
    rOutStream.SeekRel(-PNG_IHDR_SIZE - PNG_CRC_SIZE);
    std::vector<uint8_t> aIHDRData(PNG_IHDR_SIZE + PNG_CRC_SIZE);
    rOutStream.ReadBytes(aIHDRData.data(), aIHDRData.size());
    rOutStream.WriteUInt32(rtl_crc32(0, aIHDRData.data(), aIHDRData.size()));
    rInStream.Seek(PNG_SIGNATURE_SIZE + PNG_TYPE_SIZE + PNG_SIZE_SIZE + PNG_IHDR_SIZE
                   + PNG_CRC_SIZE);
    while (rInStream.good())
    {
        sal_uInt32 nChunkSize(0), nChunkType(0);
        rInStream.ReadUInt32(nChunkSize);
        rInStream.ReadUInt32(nChunkType);
        bool bBreakOuter = false;
        switch (nChunkType)
        {
            case PNG_ACTL_SIGNATURE:
            case PNG_FCTL_SIGNATURE:
            case PNG_FDAT_SIGNATURE:
            {
                // skip apng chunks
                rInStream.SeekRel(nChunkSize + PNG_CRC_SIZE);
                continue;
            }
            case PNG_IDAT_SIGNATURE:
            {
                // IDAT chunk hit, no more important png chunks
                bBreakOuter = true;
                break;
            }
            default:
            {
                // Seek back to start of chunk
                rInStream.SeekRel(-PNG_TYPE_SIZE - PNG_SIZE_SIZE);
                const size_t nDataSize = PNG_SIZE_SIZE + PNG_TYPE_SIZE
                                         + static_cast<size_t>(nChunkSize) + PNG_CRC_SIZE;
                if (nDataSize > rInStream.remainingSize())
                {
                    SAL_WARN("vcl.filter", "png claims record of size: "
                                               << nDataSize << ", but only "
                                               << rInStream.remainingSize() << " available.");
                    bBreakOuter = true;
                    break;
                }
                // Copy chunk to rOutStream
                std::vector<uint8_t> aData(nDataSize);
                rInStream.ReadBytes(aData.data(), nDataSize);
                rOutStream.WriteBytes(aData.data(), nDataSize);
                break;
            }
        }
        if (bBreakOuter)
        {
            break;
        }
    }
    rInStream.Seek(nPos);
}

sal_uInt32 NumDenToTime(sal_uInt16 nNumerator, sal_uInt16 nDenominator)
{
    if (nDenominator == 0)
        nDenominator = 100;
    return (static_cast<double>(nNumerator) / nDenominator) * 100;
}

bool fcTLbeforeIDAT(SvStream& rStream)
{
    sal_uInt64 nPos = rStream.Tell();
    SvStreamEndian originalEndian = rStream.GetEndian();
    comphelper::ScopeGuard aGuard([&rStream, nPos, originalEndian] {
        rStream.Seek(nPos);
        rStream.SetEndian(originalEndian);
    });
    // Skip PNG header and IHDR
    rStream.SetEndian(SvStreamEndian::BIG);
    if (!checkSeek(rStream, PNG_SIGNATURE_SIZE + PNG_TYPE_SIZE + PNG_SIZE_SIZE + PNG_IHDR_SIZE
                                + PNG_CRC_SIZE))
        return false;
    do
    {
        sal_uInt32 nChunkSize(0), nChunkType(0);
        rStream.ReadUInt32(nChunkSize);
        rStream.ReadUInt32(nChunkType);
        switch (nChunkType)
        {
            case PNG_FCTL_SIGNATURE:
                return true;
            case PNG_IDAT_SIGNATURE:
                return false;
            default:
            {
                if (!checkSeek(rStream, rStream.Tell() + nChunkSize + PNG_CRC_SIZE))
                    return false;
                break;
            }
        }
    } while (rStream.good());
    return false;
}

#if defined __GNUC__ && __GNUC__ <= 14 && !defined __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclobbered"
#endif
bool reader(SvStream& rStream, Graphic& rGraphic,
            GraphicFilterImportFlags nImportFlags = GraphicFilterImportFlags::NONE,
            BitmapScopedWriteAccess* pAccess = nullptr,
            BitmapScopedWriteAccess* pAlphaAccess = nullptr)
{
    if (!isPng(rStream))
        return false;

    png_structp pPng = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!pPng)
        return false;

    APNGInfo aAPNGInfo;
    png_set_read_user_chunk_fn(pPng, &aAPNGInfo, &handle_unknown_chunk);
    // don't complain about vpAg and exIf chunks
    png_set_keep_unknown_chunks(pPng, 2, nullptr, 0);

    png_infop pInfo = png_create_info_struct(pPng);
    if (!pInfo)
    {
        png_destroy_read_struct(&pPng, nullptr, nullptr);
        return false;
    }

    PngDestructor pngDestructor = { pPng, pInfo };

    // All variables holding resources need to be declared here in order to be
    // properly cleaned up in case of an error, otherwise libpng's longjmp()
    // jumps over the destructor calls.
    BitmapEx aBitmapEx;
    Bitmap aBitmap;
    AlphaMask aBitmapAlpha;
    Size prefSize;
    BitmapScopedWriteAccess pWriteAccessInstance;
    BitmapScopedWriteAccess pWriteAccessAlphaInstance;
    const bool bFuzzing = comphelper::IsFuzzing();
    const bool bSupportsBitmap32 = bFuzzing || ImplGetSVData()->mpDefInst->supportsBitmap32();
    const bool bOnlyCreateBitmap
        = static_cast<bool>(nImportFlags & GraphicFilterImportFlags::OnlyCreateBitmap);
    const bool bUseExistingBitmap
        = static_cast<bool>(nImportFlags & GraphicFilterImportFlags::UseExistingBitmap);

    if (setjmp(png_jmpbuf(pPng)))
    {
        if (!bUseExistingBitmap)
        {
            // Set the bitmap if it contains something, even on failure. This allows
            // reading images that are only partially broken.
            pWriteAccessInstance.reset();
            pWriteAccessAlphaInstance.reset();
            if (!aBitmap.IsEmpty() && !aBitmapAlpha.IsEmpty())
                aBitmapEx = BitmapEx(aBitmap, aBitmapAlpha);
            else if (!aBitmap.IsEmpty())
                aBitmapEx = BitmapEx(aBitmap);
            if (!aBitmapEx.IsEmpty() && !prefSize.IsEmpty())
            {
                aBitmapEx.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
                aBitmapEx.SetPrefSize(prefSize);
            }
            rGraphic = aBitmapEx;
        }
        return false;
    }

    png_set_option(pPng, PNG_MAXIMUM_INFLATE_WINDOW, PNG_OPTION_ON);

    png_set_read_fn(pPng, &rStream, lclReadStream);

    if (!bFuzzing)
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
        return false;

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
        return false;

    if (bitDepth != 8
        || (colorType != PNG_COLOR_TYPE_RGB && colorType != PNG_COLOR_TYPE_RGB_ALPHA
            && colorType != PNG_COLOR_TYPE_GRAY))
    {
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

    if (!bUseExistingBitmap)
    {
        switch (colorType)
        {
            case PNG_COLOR_TYPE_RGB:
                aBitmap = Bitmap(Size(width, height), vcl::PixelFormat::N24_BPP);
                break;
            case PNG_COLOR_TYPE_RGBA:
                if (bSupportsBitmap32)
                    aBitmap = Bitmap(Size(width, height), vcl::PixelFormat::N32_BPP);
                else
                {
                    aBitmap = Bitmap(Size(width, height), vcl::PixelFormat::N24_BPP);
                    aBitmapAlpha = AlphaMask(Size(width, height), nullptr);
                    aBitmapAlpha.Erase(0); // opaque
                }
                break;
            case PNG_COLOR_TYPE_GRAY:
                aBitmap = Bitmap(Size(width, height), vcl::PixelFormat::N8_BPP,
                                 &Bitmap::GetGreyPalette(256));
                break;
            default:
                abort();
        }

        if (bOnlyCreateBitmap)
        {
            if (!aBitmapAlpha.IsEmpty())
                aBitmapEx = BitmapEx(aBitmap, aBitmapAlpha);
            else
                aBitmapEx = BitmapEx(aBitmap);
            if (!prefSize.IsEmpty())
            {
                aBitmapEx.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
                aBitmapEx.SetPrefSize(prefSize);
            }
            rGraphic = aBitmapEx;
            return true;
        }

        pWriteAccessInstance = aBitmap;
        if (!pWriteAccessInstance)
            return false;
        if (!aBitmapAlpha.IsEmpty())
        {
            pWriteAccessAlphaInstance = aBitmapAlpha;
            if (!pWriteAccessAlphaInstance)
                return false;
        }
    }
    BitmapScopedWriteAccess& pWriteAccess = pAccess ? *pAccess : pWriteAccessInstance;
    BitmapScopedWriteAccess& pWriteAccessAlpha
        = pAlphaAccess ? *pAlphaAccess : pWriteAccessAlphaInstance;

    if (colorType == PNG_COLOR_TYPE_RGB)
    {
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
    }
    else if (colorType == PNG_COLOR_TYPE_RGB_ALPHA)
    {
        size_t aRowSizeBytes = png_get_rowbytes(pPng, pInfo);

        if (bSupportsBitmap32)
        {
            ScanlineFormat eFormat = pWriteAccess->GetScanlineFormat();
            if (eFormat == ScanlineFormat::N32BitTcAbgr || eFormat == ScanlineFormat::N32BitTcBgra)
                png_set_bgr(pPng);

            for (int pass = 0; pass < nNumberOfPasses; pass++)
            {
                for (png_uint_32 y = 0; y < height; y++)
                {
                    Scanline pScanline = pWriteAccess->GetScanline(y);
                    png_read_row(pPng, pScanline, nullptr);
                }
            }
#if !ENABLE_WASM_STRIP_PREMULTIPLY
            const vcl::bitmap::lookup_table& premultiply = vcl::bitmap::get_premultiply_table();
#endif
            if (eFormat == ScanlineFormat::N32BitTcAbgr || eFormat == ScanlineFormat::N32BitTcArgb)
            { // alpha first and premultiply
                for (png_uint_32 y = 0; y < height; y++)
                {
                    Scanline pScanline = pWriteAccess->GetScanline(y);
                    for (size_t i = 0; i < aRowSizeBytes; i += 4)
                    {
                        const sal_uInt8 alpha = pScanline[i + 3];
#if ENABLE_WASM_STRIP_PREMULTIPLY
                        pScanline[i + 3] = vcl::bitmap::premultiply(alpha, pScanline[i + 2]);
                        pScanline[i + 2] = vcl::bitmap::premultiply(alpha, pScanline[i + 1]);
                        pScanline[i + 1] = vcl::bitmap::premultiply(alpha, pScanline[i]);
#else
                        pScanline[i + 3] = premultiply[alpha][pScanline[i + 2]];
                        pScanline[i + 2] = premultiply[alpha][pScanline[i + 1]];
                        pScanline[i + 1] = premultiply[alpha][pScanline[i]];
#endif
                        pScanline[i] = alpha;
                    }
                }
            }
            else
            { // keep alpha last, only premultiply
                for (png_uint_32 y = 0; y < height; y++)
                {
                    Scanline pScanline = pWriteAccess->GetScanline(y);
                    for (size_t i = 0; i < aRowSizeBytes; i += 4)
                    {
                        const sal_uInt8 alpha = pScanline[i + 3];
#if ENABLE_WASM_STRIP_PREMULTIPLY
                        pScanline[i] = vcl::bitmap::premultiply(alpha, pScanline[i]);
                        pScanline[i + 1] = vcl::bitmap::premultiply(alpha, pScanline[i + 1]);
                        pScanline[i + 2] = vcl::bitmap::premultiply(alpha, pScanline[i + 2]);
#else
                        pScanline[i] = premultiply[alpha][pScanline[i]];
                        pScanline[i + 1] = premultiply[alpha][pScanline[i + 1]];
                        pScanline[i + 2] = premultiply[alpha][pScanline[i + 2]];
#endif
                    }
                }
            }
        }
        else
        {
            ScanlineFormat eFormat = pWriteAccess->GetScanlineFormat();
            if (eFormat == ScanlineFormat::N24BitTcBgr)
                png_set_bgr(pPng);

            if (nNumberOfPasses == 1)
            {
                // optimise the common case, where we can use a buffer of only a single row
                std::vector<png_byte> aRow(aRowSizeBytes, 0);
                for (png_uint_32 y = 0; y < height; y++)
                {
                    Scanline pScanline = pWriteAccess->GetScanline(y);
                    Scanline pScanAlpha = pWriteAccessAlpha->GetScanline(y);
                    png_bytep pRow = aRow.data();
                    png_read_row(pPng, pRow, nullptr);
                    size_t iAlpha = 0;
                    size_t iColor = 0;
                    for (size_t i = 0; i < aRowSizeBytes; i += 4)
                    {
                        pScanline[iColor++] = pRow[i + 0];
                        pScanline[iColor++] = pRow[i + 1];
                        pScanline[iColor++] = pRow[i + 2];
                        pScanAlpha[iAlpha++] = pRow[i + 3];
                    }
                }
            }
            else
            {
                std::vector<std::vector<png_byte>> aRows(height);
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
                            pScanAlpha[iAlpha++] = pRow[i + 3];
                        }
                    }
                }
            }
        }
    }
    else if (colorType == PNG_COLOR_TYPE_GRAY)
    {
        for (int pass = 0; pass < nNumberOfPasses; pass++)
        {
            for (png_uint_32 y = 0; y < height; y++)
            {
                Scanline pScanline = pWriteAccess->GetScanline(y);
                png_read_row(pPng, pScanline, nullptr);
            }
        }
    }

    png_read_end(pPng, pInfo);

    if (!bUseExistingBitmap)
    {
        pWriteAccess.reset();
        pWriteAccessAlpha.reset();
        if (!aBitmapAlpha.IsEmpty())
            aBitmapEx = BitmapEx(aBitmap, aBitmapAlpha);
        else
            aBitmapEx = BitmapEx(aBitmap);
        if (!prefSize.IsEmpty())
        {
            aBitmapEx.SetPrefMapMode(MapMode(MapUnit::Map100thMM));
            aBitmapEx.SetPrefSize(prefSize);
        }
    }

    if (aAPNGInfo.mbIsApng)
    {
        Animation aAnimation;
        // We create new pngs for each frame and use the PngImageReader to create
        // the BitmapExs for each frame
        bool bFctlBeforeIDAT = fcTLbeforeIDAT(rStream);
        size_t nSequenceIndex = static_cast<size_t>(bFctlBeforeIDAT);
        sal_uInt32 nFrames
            = aAPNGInfo.maACTLChunk.num_frames - static_cast<sal_uInt32>(bFctlBeforeIDAT);
        {
            if (aAPNGInfo.maFrameData.empty())
                return false;
            fcTLChunk* aFctlChunk = dynamic_cast<fcTLChunk*>(aAPNGInfo.maFrameData[0].get());
            if (!aFctlChunk)
                return false;
            Size aCanvasSize(aFctlChunk->width, aFctlChunk->height);
            aAnimation.SetDisplaySizePixel(aCanvasSize);
            aAnimation.SetLoopCount(aAPNGInfo.maACTLChunk.num_plays);
            if (bFctlBeforeIDAT)
            {
                Point aFirstPoint(0, 0);
                auto aDisposal = static_cast<Disposal>(aFctlChunk->dispose_op);
                auto aBlend = static_cast<Blend>(aFctlChunk->blend_op);
                if (aDisposal == Disposal::Previous)
                    aDisposal = Disposal::Back;
                AnimationFrame aAnimationFrame(
                    aBitmapEx, aFirstPoint, aCanvasSize,
                    NumDenToTime(aFctlChunk->delay_num, aFctlChunk->delay_den), aDisposal, aBlend);
                aAnimation.Insert(aAnimationFrame);
            }
        }
        for (sal_uInt32 i = 0; i < nFrames; i++)
        {
            fcTLChunk* aFctlChunk
                = nSequenceIndex < aAPNGInfo.maFrameData.size()
                      ? dynamic_cast<fcTLChunk*>(aAPNGInfo.maFrameData[nSequenceIndex++].get())
                      : nullptr;
            if (!aFctlChunk)
                return false;
            Disposal aDisposal = static_cast<Disposal>(aFctlChunk->dispose_op);
            Blend aBlend = static_cast<Blend>(aFctlChunk->blend_op);
            if (i == 0 && aDisposal == Disposal::Back)
                aDisposal = Disposal::Previous;
            SvMemoryStream aFrameStream;
            getImportantChunks(rStream, aFrameStream, aFctlChunk->width, aFctlChunk->height);
            // A single frame can have multiple fdAT chunks
            while (fdATChunk* pFdatChunk
                   = nSequenceIndex < aAPNGInfo.maFrameData.size()
                         ? dynamic_cast<fdATChunk*>(aAPNGInfo.maFrameData[nSequenceIndex].get())
                         : nullptr)
            {
                // Write fdAT chunks as IDAT chunks
                auto nDataSize = pFdatChunk->frame_data.size();
                aFrameStream.WriteUInt32(nDataSize - PNG_TYPE_SIZE);
                aFrameStream.WriteBytes(pFdatChunk->frame_data.data(), nDataSize);
                sal_uInt32 nCrc = rtl_crc32(0, pFdatChunk->frame_data.data(), nDataSize);
                aFrameStream.WriteUInt32(nCrc);
                nSequenceIndex++;
            }
            aFrameStream.WriteUInt32(PNG_IEND_SIZE);
            aFrameStream.WriteUInt32(PNG_IEND_SIGNATURE);
            aFrameStream.WriteUInt32(PNG_IEND_CRC);
            Graphic aFrameGraphic;
            aFrameStream.Seek(0);
            bool bSuccess = reader(aFrameStream, aFrameGraphic);
            if (!bSuccess)
                return false;
            BitmapEx aFrameBitmapEx = aFrameGraphic.GetBitmapEx();
            Point aStartPoint(aFctlChunk->x_offset, aFctlChunk->y_offset);
            Size aSize(aFctlChunk->width, aFctlChunk->height);
            AnimationFrame aAnimationFrame(
                aFrameBitmapEx, aStartPoint, aSize,
                NumDenToTime(aFctlChunk->delay_num, aFctlChunk->delay_den), aDisposal, aBlend);
            aAnimation.Insert(aAnimationFrame);
        }
        rGraphic = aAnimation;
        return true;
    }
    else
    {
        rGraphic = aBitmapEx;
    }

    return true;
}

BinaryDataContainer getMsGifChunk(SvStream& rStream)
{
    if (!isPng(rStream))
        return {};
    // It's easier to read manually the contents and find the chunk than
    // try to get it using libpng.
    // https://en.wikipedia.org/wiki/Portable_Network_Graphics#File_format
    // Each chunk is: 4 bytes length, 4 bytes type, <length> bytes, 4 bytes crc
    bool ignoreCrc = comphelper::IsFuzzing();
    for (;;)
    {
        sal_uInt32 length(0), type(0), crc(0);
        rStream.ReadUInt32(length);
        rStream.ReadUInt32(type);
        if (!rStream.good())
            return {};
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
                return {}; // broken PNG

            char msHeader[MSGifHeaderSize];
            if (rStream.ReadBytes(msHeader, MSGifHeaderSize) != MSGifHeaderSize)
                return {};
            computedCrc = rtl_crc32(computedCrc, msHeader, MSGifHeaderSize);
            length -= MSGifHeaderSize;

            BinaryDataContainer chunk(rStream, length);
            if (chunk.isEmpty())
                return {};
            computedCrc = rtl_crc32(computedCrc, chunk.getData(), chunk.getSize());
            rStream.ReadUInt32(crc);
            if (!ignoreCrc && crc != computedCrc)
                continue; // invalid chunk, ignore
            return chunk;
        }
        if (rStream.remainingSize() < length)
            return {};
        rStream.SeekRel(length);
        rStream.ReadUInt32(crc);
        if (type == PNG_IEND_SIGNATURE)
            return {};
    }
}
#if defined __GNUC__ && __GNUC__ <= 14 && !defined __clang__
#pragma GCC diagnostic pop
#endif

} // anonymous namespace

namespace vcl
{
PngImageReader::PngImageReader(SvStream& rStream)
    : mrStream(rStream)
{
}

bool PngImageReader::read(BitmapEx& rBitmapEx)
{
    Graphic aGraphic;
    bool bRet = reader(mrStream, aGraphic);
    rBitmapEx = aGraphic.GetBitmapEx();
    return bRet;
}

bool PngImageReader::read(Graphic& rGraphic) { return reader(mrStream, rGraphic); }

BitmapEx PngImageReader::read()
{
    Graphic aGraphic;
    read(aGraphic);
    return aGraphic.GetBitmapEx();
}

BinaryDataContainer PngImageReader::getMicrosoftGifChunk(SvStream& rStream)
{
    sal_uInt64 originalPosition = rStream.Tell();
    SvStreamEndian originalEndian = rStream.GetEndian();
    rStream.SetEndian(SvStreamEndian::BIG);
    auto chunk = getMsGifChunk(rStream);
    rStream.SetEndian(originalEndian);
    rStream.Seek(originalPosition);
    return chunk;
}

bool ImportPNG(SvStream& rInputStream, Graphic& rGraphic, GraphicFilterImportFlags nImportFlags,
               BitmapScopedWriteAccess* pAccess, BitmapScopedWriteAccess* pAlphaAccess)
{
    // Creating empty bitmaps should be practically a no-op, and thus thread-safe.
    Graphic aGraphic;
    if (reader(rInputStream, aGraphic, nImportFlags, pAccess, pAlphaAccess))
    {
        if (!(nImportFlags & GraphicFilterImportFlags::UseExistingBitmap))
            rGraphic = aGraphic;
        return true;
    }
    return false;
}

bool PngImageReader::isAPng(SvStream& rStream)
{
    auto nStmPos = rStream.Tell();
    SvStreamEndian originalEndian = rStream.GetEndian();
    comphelper::ScopeGuard aGuard([&rStream, nStmPos, originalEndian] {
        rStream.Seek(nStmPos);
        rStream.SetEndian(originalEndian);
    });
    if (!isPng(rStream))
        return false;
    rStream.SetEndian(SvStreamEndian::BIG);
    sal_uInt32 nChunkSize, nChunkType;
    rStream.ReadUInt32(nChunkSize);
    rStream.ReadUInt32(nChunkType);
    if (!rStream.good() || nChunkType != PNG_IHDR_SIGNATURE)
        return false;
    if (!checkSeek(rStream, rStream.Tell() + nChunkSize))
        return false;
    // Skip IHDR CRC
    if (!checkSeek(rStream, rStream.Tell() + PNG_CRC_SIZE))
        return false;
    // Look for acTL chunk that exists before the first IDAT chunk
    while (true)
    {
        rStream.ReadUInt32(nChunkSize);
        if (!rStream.good())
            return false;
        rStream.ReadUInt32(nChunkType);
        if (!rStream.good())
            return false;
        // Check if it's an IDAT chunk -> regular PNG
        if (nChunkType == PNG_IDAT_SIGNATURE)
            return false;
        else if (nChunkType == PNG_ACTL_SIGNATURE)
            return true;
        else
        {
            if (!checkSeek(rStream, rStream.Tell() + nChunkSize + PNG_CRC_SIZE))
                return false;
        }
    }
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
