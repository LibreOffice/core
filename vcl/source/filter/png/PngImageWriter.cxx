/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/filter/PngImageWriter.hxx>
#include <png.h>
#include <bitmap/BitmapWriteAccess.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/BitmapTools.hxx>

namespace
{
void combineScanlineChannels(Scanline pRGBScanline, Scanline pAlphaScanline,
                             std::vector<std::remove_pointer_t<Scanline>>& pResult,
                             sal_uInt32 nBitmapWidth)
{
    for (sal_uInt32 i = 0; i < nBitmapWidth; ++i)
    {
        pResult[i * 4] = *pRGBScanline++; // R
        pResult[i * 4 + 1] = *pRGBScanline++; // G
        pResult[i * 4 + 2] = *pRGBScanline++; // B
        pResult[i * 4 + 3] = *pAlphaScanline++; // A
    }
}
}

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

static bool pngWrite(SvStream& rStream, const BitmapEx& rBitmapEx, int nCompressionLevel,
                     bool bInterlaced, bool bTranslucent,
                     const std::vector<PngChunk>& aAdditionalChunks)
{
    if (rBitmapEx.IsAlpha() && !bTranslucent)
        return false;
    if (rBitmapEx.IsEmpty())
        return false;

    png_structp pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!pPng)
        return false;

    png_infop pInfo = png_create_info_struct(pPng);
    if (!pInfo)
    {
        png_destroy_write_struct(&pPng, nullptr);
        return false;
    }

    BitmapEx aBitmapEx;
    if (rBitmapEx.GetBitmap().getPixelFormat() == vcl::PixelFormat::N32_BPP)
    {
        if (!vcl::bitmap::convertBitmap32To24Plus8(rBitmapEx, aBitmapEx))
            return false;
    }
    else
    {
        aBitmapEx = rBitmapEx;
    }

    Bitmap aBitmap;
    AlphaMask aAlphaMask;
    Bitmap::ScopedReadAccess pAccess;
    Bitmap::ScopedReadAccess pAlphaAccess;

    if (setjmp(png_jmpbuf(pPng)))
    {
        pAccess.reset();
        pAlphaAccess.reset();
        png_destroy_read_struct(&pPng, &pInfo, nullptr);
        return false;
    }

    // Set our custom stream writer
    png_set_write_fn(pPng, &rStream, lclWriteStream, nullptr);

    aBitmap = aBitmapEx.GetBitmap();
    aAlphaMask = aBitmapEx.GetAlpha();

    {
        bool bCombineChannels = false;
        pAccess = Bitmap::ScopedReadAccess(aBitmap);
        pAlphaAccess = Bitmap::ScopedReadAccess(aAlphaMask);
        Size aSize = aBitmapEx.GetSizePixel();

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
        switch (eScanlineFormat)
        {
            case ScanlineFormat::N1BitMsbPal:
            case ScanlineFormat::N1BitLsbPal:
            {
                colorType = PNG_COLOR_TYPE_PALETTE;
                bitDepth = 1;
                break;
            }
            case ScanlineFormat::N8BitPal:
            {
                if (!aBitmap.HasGreyPalette8Bit())
                    colorType = PNG_COLOR_TYPE_PALETTE;
                else
                    colorType = PNG_COLOR_TYPE_GRAY;
                bitDepth = 8;
                break;
            }
            case ScanlineFormat::N24BitTcBgr:
            {
                png_set_bgr(pPng);
                [[fallthrough]];
            }
            case ScanlineFormat::N24BitTcRgb:
            {
                colorType = PNG_COLOR_TYPE_RGB;
                bitDepth = 8;
                if (pAlphaAccess)
                {
                    colorType = PNG_COLOR_TYPE_RGBA;
                    bCombineChannels = true;
                }
                break;
            }
            case ScanlineFormat::N32BitTcBgra:
            {
                png_set_bgr(pPng);
                [[fallthrough]];
            }
            case ScanlineFormat::N32BitTcRgba:
            {
                colorType = PNG_COLOR_TYPE_RGBA;
                bitDepth = 8;
                break;
            }
            default:
            {
                return false;
            }
        }

        if (aBitmapEx.GetPrefMapMode().GetMapUnit() == MapUnit::Map100thMM)
        {
            Size aPrefSize(aBitmapEx.GetPrefSize());
            sal_uInt32 nPrefSizeX = o3tl::convert(aSize.Width(), 100000, aPrefSize.Width());
            sal_uInt32 nPrefSizeY = o3tl::convert(aSize.Height(), 100000, aPrefSize.Height());
            png_set_pHYs(pPng, pInfo, nPrefSizeX, nPrefSizeY, 1);
        }

        png_set_compression_level(pPng, nCompressionLevel);

        int interlaceType = bInterlaced ? PNG_INTERLACE_ADAM7 : PNG_INTERLACE_NONE;
        int compressionType = PNG_COMPRESSION_TYPE_DEFAULT;
        int filterMethod = PNG_FILTER_TYPE_DEFAULT;

        // Convert BitmapPalette to png_color*
        if (colorType == PNG_COLOR_TYPE_PALETTE)
        {
            // Reserve enough space for 3 channels for each palette entry
            auto aBitmapPalette = pAccess->GetPalette();
            auto nEntryCount = aBitmapPalette.GetEntryCount();
            std::unique_ptr<png_color[]> aPngPaletteArray(new png_color[nEntryCount * 3]);
            for (sal_uInt16 i = 0; i < nEntryCount; i++)
            {
                aPngPaletteArray[i].red = aBitmapPalette[i].GetRed();
                aPngPaletteArray[i].green = aBitmapPalette[i].GetGreen();
                aPngPaletteArray[i].blue = aBitmapPalette[i].GetBlue();
            }
            // Palette is copied over so it can be safely discarded
            png_set_PLTE(pPng, pInfo, aPngPaletteArray.get(), nEntryCount);
        }

        png_set_IHDR(pPng, pInfo, aSize.Width(), aSize.Height(), bitDepth, colorType, interlaceType,
                     compressionType, filterMethod);

        png_write_info(pPng, pInfo);

        int nNumberOfPasses = 1;

        Scanline pSourcePointer;

        tools::Long nHeight = pAccess->Height();

        for (int nPass = 0; nPass < nNumberOfPasses; nPass++)
        {
            for (tools::Long y = 0; y < nHeight; y++)
            {
                pSourcePointer = pAccess->GetScanline(y);
                Scanline pFinalPointer = pSourcePointer;
                std::vector<std::remove_pointer_t<Scanline>> aCombinedChannels;
                if (bCombineChannels)
                {
                    auto nBitmapWidth = pAccess->Width();
                    // Allocate enough size to fit all 4 channels
                    aCombinedChannels.resize(nBitmapWidth * 4);
                    Scanline pAlphaPointer = pAlphaAccess->GetScanline(y);
                    if (!pSourcePointer || !pAlphaPointer)
                        return false;
                    // Combine RGB and alpha channels
                    combineScanlineChannels(pSourcePointer, pAlphaPointer, aCombinedChannels,
                                            nBitmapWidth);
                    pFinalPointer = aCombinedChannels.data();
                    // Invert alpha channel (255 - a)
                    png_set_invert_alpha(pPng);
                }
                png_write_row(pPng, pFinalPointer);
            }
        }
    }

    if (!aAdditionalChunks.empty())
    {
        for (const auto& aChunk : aAdditionalChunks)
        {
            png_write_chunk(pPng, aChunk.name.data(), aChunk.data.data(), aChunk.size);
        }
    }

    png_write_end(pPng, pInfo);

    png_destroy_write_struct(&pPng, &pInfo);

    return true;
}

void PngImageWriter::setParameters(css::uno::Sequence<css::beans::PropertyValue> const& rParameters)
{
    for (auto const& rValue : rParameters)
    {
        if (rValue.Name == "Compression")
            rValue.Value >>= mnCompressionLevel;
        else if (rValue.Name == "Interlaced")
            rValue.Value >>= mbInterlaced;
        else if (rValue.Name == "Translucent")
        {
            tools::Long nTmp = 0;
            rValue.Value >>= nTmp;
            if (!nTmp)
                mbTranslucent = false;
        }
        else if (rValue.Name == "AdditionalChunks")
        {
            css::uno::Sequence<css::beans::PropertyValue> aAdditionalChunkSequence;
            if (rValue.Value >>= aAdditionalChunkSequence)
            {
                for (const auto& rAdditionalChunk : std::as_const(aAdditionalChunkSequence))
                {
                    if (rAdditionalChunk.Name.getLength() == 4)
                    {
                        vcl::PngChunk aChunk;
                        for (sal_Int32 k = 0; k < 4; k++)
                        {
                            aChunk.name[k] = static_cast<sal_uInt8>(rAdditionalChunk.Name[k]);
                        }
                        aChunk.name[4] = '\0';

                        css::uno::Sequence<sal_Int8> aByteSeq;
                        if (rAdditionalChunk.Value >>= aByteSeq)
                        {
                            sal_uInt32 nChunkSize = aByteSeq.getLength();
                            aChunk.size = nChunkSize;
                            if (nChunkSize)
                            {
                                const sal_Int8* pSource = aByteSeq.getConstArray();
                                std::vector<sal_uInt8> aData(pSource, pSource + nChunkSize);
                                aChunk.data = std::move(aData);
                                maAdditionalChunks.push_back(aChunk);
                            }
                        }
                    }
                }
            }
        }
    }
}

PngImageWriter::PngImageWriter(SvStream& rStream)
    : mrStream(rStream)
    , mnCompressionLevel(6)
    , mbInterlaced(false)
    , mbTranslucent(true)
{
}

bool PngImageWriter::write(const BitmapEx& rBitmapEx)
{
    return pngWrite(mrStream, rBitmapEx, mnCompressionLevel, mbInterlaced, mbTranslucent,
                    maAdditionalChunks);
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
