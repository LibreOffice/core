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
#include <vcl/BitmapWriteAccess.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapTools.hxx>
#include <sal/log.hxx>
#include <rtl/crc.h>

namespace
{
void combineScanlineChannels(Scanline pColorScanline, Scanline pAlphaScanline,
                             std::vector<std::remove_pointer_t<Scanline>>& pResult,
                             sal_uInt32 nBitmapWidth, int colorType)
{
    if (colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
        for (sal_uInt32 i = 0; i < nBitmapWidth; ++i)
        {
            pResult[i * 2] = *pColorScanline++; // Gray
            pResult[i * 2 + 1] = *pAlphaScanline++; // A
        }
        return;
    }

    for (sal_uInt32 i = 0; i < nBitmapWidth; ++i)
    {
        pResult[i * 4] = *pColorScanline++; // R
        pResult[i * 4 + 1] = *pColorScanline++; // G
        pResult[i * 4 + 2] = *pColorScanline++; // B
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

static void writeFctlChunk(std::vector<uint8_t>& aFctlChunk, sal_uInt32 nSequenceNumber, Size aSize,
                           Point aOffset, sal_uInt16 nDelayNum, sal_uInt16 nDelayDen,
                           Disposal nDisposeOp, Blend nBlendOp)
{
    if (aFctlChunk.size() != 26)
        aFctlChunk.resize(26);

    sal_uInt32 nWidth = aSize.Width();
    sal_uInt32 nHeight = aSize.Height();
    sal_uInt32 nXOffset = aOffset.X();
    sal_uInt32 nYOffset = aOffset.Y();

    // Writing each byte separately instead of using memcpy here for clarity
    // about PNG chunks using big endian

    // Write sequence number
    aFctlChunk[0] = (nSequenceNumber >> 24) & 0xFF;
    aFctlChunk[1] = (nSequenceNumber >> 16) & 0xFF;
    aFctlChunk[2] = (nSequenceNumber >> 8) & 0xFF;
    aFctlChunk[3] = nSequenceNumber & 0xFF;

    // Write width
    aFctlChunk[4] = (nWidth >> 24) & 0xFF;
    aFctlChunk[5] = (nWidth >> 16) & 0xFF;
    aFctlChunk[6] = (nWidth >> 8) & 0xFF;
    aFctlChunk[7] = nWidth & 0xFF;

    // Write height
    aFctlChunk[8] = (nHeight >> 24) & 0xFF;
    aFctlChunk[9] = (nHeight >> 16) & 0xFF;
    aFctlChunk[10] = (nHeight >> 8) & 0xFF;
    aFctlChunk[11] = nHeight & 0xFF;

    // Write x offset
    aFctlChunk[12] = (nXOffset >> 24) & 0xFF;
    aFctlChunk[13] = (nXOffset >> 16) & 0xFF;
    aFctlChunk[14] = (nXOffset >> 8) & 0xFF;
    aFctlChunk[15] = nXOffset & 0xFF;

    // Write y offset
    aFctlChunk[16] = (nYOffset >> 24) & 0xFF;
    aFctlChunk[17] = (nYOffset >> 16) & 0xFF;
    aFctlChunk[18] = (nYOffset >> 8) & 0xFF;
    aFctlChunk[19] = nYOffset & 0xFF;

    // Write delay numerator
    aFctlChunk[20] = (nDelayNum >> 8) & 0xFF;
    aFctlChunk[21] = nDelayNum & 0xFF;

    // Write delay denominator
    aFctlChunk[22] = (nDelayDen >> 8) & 0xFF;
    aFctlChunk[23] = nDelayDen & 0xFF;

    // Write disposal method
    aFctlChunk[24] = static_cast<uint8_t>(nDisposeOp);

    // Write blend operation
    aFctlChunk[25] = static_cast<uint8_t>(nBlendOp);
}

static bool pngWrite(SvStream& rStream, const Graphic& rGraphic, int nCompressionLevel,
                     bool bInterlaced, bool bTranslucent,
                     const std::vector<PngChunk>& aAdditionalChunks)
{
    if (rGraphic.IsNone())
        return false;

    sal_uInt32 nSequenceNumber = 0;
    const bool bIsApng = rGraphic.IsAnimated();
    Animation aAnimation = bIsApng ? rGraphic.GetAnimation() : Animation();

    png_structp pPng = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);

    if (!pPng)
        return false;

    png_infop pInfo = png_create_info_struct(pPng);
    if (!pInfo)
    {
        png_destroy_write_struct(&pPng, nullptr);
        return false;
    }

    basegfx::B2DSize const aPPM = rGraphic.GetPPM();
    png_set_pHYs(pPng, pInfo, std::round(aPPM.getWidth()), std::round(aPPM.getHeight()),
                 PNG_RESOLUTION_METER);

    BitmapEx aBitmapEx;
    if (rGraphic.GetBitmapEx().getPixelFormat() == vcl::PixelFormat::N32_BPP)
    {
        if (!vcl::bitmap::convertBitmap32To24Plus8(rGraphic.GetBitmapExRef(), aBitmapEx))
            return false;
    }
    else
    {
        aBitmapEx = rGraphic.GetBitmapExRef();
    }

    if (!bTranslucent)
    {
        // Clear alpha channel
        aBitmapEx.ClearAlpha();
    }

    Bitmap aBitmap;
    AlphaMask aAlphaMask;
    BitmapScopedReadAccess pAccess;
    BitmapScopedReadAccess pAlphaAccess;

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
    if (bTranslucent)
        aAlphaMask = aBitmapEx.GetAlphaMask();

    {
        bool bCombineChannels = false;
        pAccess = aBitmap;
        if (bTranslucent)
            pAlphaAccess = aAlphaMask;
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
            {
                colorType = PNG_COLOR_TYPE_PALETTE;
                bitDepth = 1;
                break;
            }
            case ScanlineFormat::N8BitPal:
            {
                // Calling aBitmap.HasGreyPalette8Bit() hits an assert when
                // using Skia in a debug build so query the palette through
                // the bitmap read access object.
                if (!pAccess->HasPalette() || !pAccess->GetPalette().IsGreyPalette8Bit())
                    colorType = PNG_COLOR_TYPE_PALETTE;
                else
                {
                    colorType = PNG_COLOR_TYPE_GRAY;
                    if (pAlphaAccess)
                    {
                        colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
                        bCombineChannels = true;
                    }
                }
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
            case ScanlineFormat::N32BitTcBgrx:
            {
                png_set_bgr(pPng);
                [[fallthrough]];
            }
            case ScanlineFormat::N32BitTcRgba:
            case ScanlineFormat::N32BitTcRgbx:
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
            if (aPrefSize.Width() && aPrefSize.Height())
            {
                sal_uInt32 nPrefSizeX = o3tl::convert(aSize.Width(), 100000, aPrefSize.Width());
                sal_uInt32 nPrefSizeY = o3tl::convert(aSize.Height(), 100000, aPrefSize.Height());
                png_set_pHYs(pPng, pInfo, nPrefSizeX, nPrefSizeY, 1);
            }
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

        if (bIsApng)
        {
            // Write acTL chunk
            sal_uInt32 nNumFrames = aAnimation.Count();
            sal_uInt32 nNumPlays = aAnimation.GetLoopCount();

            std::vector<uint8_t> aActlChunk;
            aActlChunk.resize(8);

            // Write number of frames
            aActlChunk[0] = (nNumFrames >> 24) & 0xFF;
            aActlChunk[1] = (nNumFrames >> 16) & 0xFF;
            aActlChunk[2] = (nNumFrames >> 8) & 0xFF;
            aActlChunk[3] = nNumFrames & 0xFF;

            // Write number of plays
            aActlChunk[4] = (nNumPlays >> 24) & 0xFF;
            aActlChunk[5] = (nNumPlays >> 16) & 0xFF;
            aActlChunk[6] = (nNumPlays >> 8) & 0xFF;
            aActlChunk[7] = nNumPlays & 0xFF;

            png_write_chunk(pPng, reinterpret_cast<png_const_bytep>("acTL"),
                            reinterpret_cast<png_const_bytep>(aActlChunk.data()),
                            aActlChunk.size());

            // Write first frame fcTL chunk which is corresponding to the IDAT chunk
            std::vector<uint8_t> aFctlChunk;
            const AnimationFrame& rFirstFrame = *aAnimation.GetAnimationFrames()[0];
            writeFctlChunk(aFctlChunk, nSequenceNumber++, rFirstFrame.maSizePixel,
                           rFirstFrame.maPositionPixel, rFirstFrame.mnWait, 100,
                           rFirstFrame.meDisposal, rFirstFrame.meBlend);

            png_write_chunk(pPng, reinterpret_cast<png_const_bytep>("fcTL"),
                            reinterpret_cast<png_const_bytep>(aFctlChunk.data()),
                            aFctlChunk.size());
        }

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
                    // Allocate enough size to fit all channels
                    aCombinedChannels.resize(nBitmapWidth * png_get_channels(pPng, pInfo));
                    Scanline pAlphaPointer = pAlphaAccess->GetScanline(y);
                    if (!pSourcePointer || !pAlphaPointer)
                        return false;
                    // Combine color and alpha channels
                    combineScanlineChannels(pSourcePointer, pAlphaPointer, aCombinedChannels,
                                            nBitmapWidth, colorType);
                    pFinalPointer = aCombinedChannels.data();
                }
                png_write_row(pPng, pFinalPointer);
            }
        }
    }

    if (bIsApng)
    {
        // Already wrote first frame as an IDAT chunk
        // Need to write the rest of the frames as fcTL & fdAT chunks
        const auto& rFrames = aAnimation.GetAnimationFrames();

        for (uint32_t i = 0; i < rFrames.size() - 1; i++)
        {
            const AnimationFrame& rCurrentFrame = *rFrames[1 + i];
            SvMemoryStream aStream;

            if (!pngWrite(aStream, rCurrentFrame.maBitmapEx, nCompressionLevel, bInterlaced,
                          bTranslucent, {}))
                return false;

            std::vector<uint8_t> aFdatChunk;

            aStream.SetEndian(SvStreamEndian::BIG);

            aStream.Seek(STREAM_SEEK_TO_BEGIN);
            aStream.Seek(8); // Skip PNG signature

            while (aStream.good())
            {
                sal_uInt32 nChunkSize;
                char sChunkName[4] = { 0 };
                aStream.ReadUInt32(nChunkSize);
                aStream.ReadBytes(sChunkName, 4);

                if (std::string(sChunkName, 4) == "IDAT")
                {
                    // 4 extra bytes for the sequence number
                    aFdatChunk.resize(nChunkSize + 4);
                    aStream.ReadBytes(aFdatChunk.data() + 4, nChunkSize);
                    break;
                }
                else
                {
                    aStream.SeekRel(nChunkSize + 4);
                }
            }

            std::vector<uint8_t> aFctlChunk;
            writeFctlChunk(aFctlChunk, nSequenceNumber++, rCurrentFrame.maSizePixel,
                           rCurrentFrame.maPositionPixel, rCurrentFrame.mnWait, 100,
                           rCurrentFrame.meDisposal, rCurrentFrame.meBlend);

            // Write sequence number
            aFdatChunk[0] = nSequenceNumber >> 24;
            aFdatChunk[1] = nSequenceNumber >> 16;
            aFdatChunk[2] = nSequenceNumber >> 8;
            aFdatChunk[3] = nSequenceNumber;
            nSequenceNumber++;

            png_write_chunk(pPng, reinterpret_cast<png_const_bytep>("fcTL"),
                            reinterpret_cast<png_const_bytep>(aFctlChunk.data()),
                            aFctlChunk.size());
            png_write_chunk(pPng, reinterpret_cast<png_const_bytep>("fdAT"),
                            reinterpret_cast<png_const_bytep>(aFdatChunk.data()),
                            aFdatChunk.size());
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
                for (const auto& rAdditionalChunk : aAdditionalChunkSequence)
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

bool PngImageWriter::write(const Graphic& rGraphic)
{
    return pngWrite(mrStream, rGraphic, mnCompressionLevel, mbInterlaced, mbTranslucent,
                    maAdditionalChunks);
}

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
