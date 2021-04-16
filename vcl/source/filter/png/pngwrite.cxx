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

#include <vcl/pngwrite.hxx>
#include <vcl/bitmapex.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <limits>
#include <rtl/crc.h>
#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/alpha.hxx>
#include <osl/endian.h>
#include <memory>
#include <vcl/BitmapTools.hxx>

#define PNG_DEF_COMPRESSION 6

#define PNGCHUNK_IHDR 0x49484452
#define PNGCHUNK_PLTE 0x504c5445
#define PNGCHUNK_IDAT 0x49444154
#define PNGCHUNK_IEND 0x49454e44
#define PNGCHUNK_pHYs 0x70485973

namespace vcl
{
class PNGWriterImpl
{
public:
    PNGWriterImpl(const BitmapEx& BmpEx,
                  const css::uno::Sequence<css::beans::PropertyValue>* pFilterData);

    bool Write(SvStream& rOutStream);

    std::vector<vcl::PNGWriter::ChunkData>& GetChunks() { return maChunkSeq; }

private:
    std::vector<vcl::PNGWriter::ChunkData> maChunkSeq;

    sal_Int32 mnCompLevel;
    sal_Int32 mnInterlaced;
    sal_uInt32 mnMaxChunkSize;
    bool mbStatus;

    Bitmap::ScopedReadAccess mpAccess;
    BitmapReadAccess* mpMaskAccess;
    ZCodec mpZCodec;

    std::unique_ptr<sal_uInt8[]>
        mpDeflateInBuf; // as big as the size of a scanline + alphachannel + 1
    std::unique_ptr<sal_uInt8[]> mpPreviousScan; // as big as mpDeflateInBuf
    std::unique_ptr<sal_uInt8[]> mpCurrentScan;
    sal_uLong mnDeflateInSize;

    sal_uLong mnWidth;
    sal_uLong mnHeight;
    sal_uInt8 mnBitsPerPixel;
    sal_uInt8 mnFilterType; // 0 or 4;
    sal_uLong mnBBP; // bytes per pixel ( needed for filtering )
    bool mbTrueAlpha;

    void ImplWritepHYs(const BitmapEx& rBitmapEx);
    void ImplWriteIDAT();
    sal_uLong ImplGetFilter(sal_uLong nY, sal_uLong nXStart = 0, sal_uLong nXAdd = 1);
    void ImplClearFirstScanline();
    bool ImplWriteHeader();
    void ImplWritePalette();
    void ImplOpenChunk(sal_uLong nChunkType);
    void ImplWriteChunk(sal_uInt8 nNumb);
    void ImplWriteChunk(sal_uInt32 nNumb);
    void ImplWriteChunk(unsigned char const* pSource, sal_uInt32 nDatSize);
};

PNGWriterImpl::PNGWriterImpl(const BitmapEx& rBitmapEx,
                             const css::uno::Sequence<css::beans::PropertyValue>* pFilterData)
    : mnCompLevel(PNG_DEF_COMPRESSION)
    , mnInterlaced(0)
    , mnMaxChunkSize(0)
    , mbStatus(true)
    , mpMaskAccess(nullptr)
    , mnDeflateInSize(0)
    , mnWidth(0)
    , mnHeight(0)
    , mnBitsPerPixel(0)
    , mnFilterType(0)
    , mnBBP(0)
    , mbTrueAlpha(false)
{
    if (rBitmapEx.IsEmpty())
        return;

    BitmapEx aBitmapEx;

    if (rBitmapEx.GetBitmap().getPixelFormat() == vcl::PixelFormat::N32_BPP)
    {
        if (!vcl::bitmap::convertBitmap32To24Plus8(rBitmapEx, aBitmapEx))
            return;
    }
    else
    {
        aBitmapEx = rBitmapEx;
    }

    Bitmap aBmp(aBitmapEx.GetBitmap());

    mnMaxChunkSize = std::numeric_limits<sal_uInt32>::max();

    if (pFilterData)
    {
        for (const auto& rPropVal : *pFilterData)
        {
            if (rPropVal.Name == "Compression")
                rPropVal.Value >>= mnCompLevel;
            else if (rPropVal.Name == "Interlaced")
                rPropVal.Value >>= mnInterlaced;
            else if (rPropVal.Name == "MaxChunkSize")
            {
                sal_Int32 nVal = 0;
                if (rPropVal.Value >>= nVal)
                    mnMaxChunkSize = static_cast<sal_uInt32>(nVal);
            }
        }
    }
    mnBitsPerPixel = sal_uInt8(vcl::pixelFormatBitCount(aBmp.getPixelFormat()));

    if (aBitmapEx.IsAlpha())
    {
        if (mnBitsPerPixel <= 8)
        {
            aBmp.Convert(BmpConversion::N24Bit);
            mnBitsPerPixel = 24;
        }

        mpAccess = Bitmap::ScopedReadAccess(aBmp); // true RGB with alphachannel
        if (mpAccess)
        {
            mbTrueAlpha = true;
            AlphaMask aMask(aBitmapEx.GetAlpha());
            mpMaskAccess = aMask.AcquireReadAccess();
            if (mpMaskAccess)
            {
                if (ImplWriteHeader())
                {
                    ImplWritepHYs(aBitmapEx);
                    ImplWriteIDAT();
                }
                aMask.ReleaseAccess(mpMaskAccess);
                mpMaskAccess = nullptr;
            }
            else
            {
                mbStatus = false;
            }
            mpAccess.reset();
        }
        else
        {
            mbStatus = false;
        }
    }
    else
    {
        mpAccess = Bitmap::ScopedReadAccess(aBmp); // palette + RGB without alphachannel
        if (mpAccess)
        {
            if (ImplWriteHeader())
            {
                ImplWritepHYs(aBitmapEx);
                if (mpAccess->HasPalette())
                    ImplWritePalette();

                ImplWriteIDAT();
            }
            mpAccess.reset();
        }
        else
        {
            mbStatus = false;
        }
    }

    if (mbStatus)
    {
        ImplOpenChunk(PNGCHUNK_IEND); // create an IEND chunk
    }
}

bool PNGWriterImpl::Write(SvStream& rOStm)
{
    /* png signature is always an array of 8 bytes */
    SvStreamEndian nOldMode = rOStm.GetEndian();
    rOStm.SetEndian(SvStreamEndian::BIG);
    rOStm.WriteUInt32(0x89504e47);
    rOStm.WriteUInt32(0x0d0a1a0a);

    for (auto const& chunk : maChunkSeq)
    {
        sal_uInt32 nType = chunk.nType;
#if defined(__LITTLEENDIAN) || defined(OSL_LITENDIAN)
        nType = OSL_SWAPDWORD(nType);
#endif
        sal_uInt32 nCRC = rtl_crc32(0, &nType, 4);
        sal_uInt32 nDataSize = chunk.aData.size();
        if (nDataSize)
            nCRC = rtl_crc32(nCRC, chunk.aData.data(), nDataSize);
        rOStm.WriteUInt32(nDataSize);
        rOStm.WriteUInt32(chunk.nType);
        if (nDataSize)
            rOStm.WriteBytes(chunk.aData.data(), nDataSize);
        rOStm.WriteUInt32(nCRC);
    }
    rOStm.SetEndian(nOldMode);
    return mbStatus;
}

bool PNGWriterImpl::ImplWriteHeader()
{
    ImplOpenChunk(PNGCHUNK_IHDR);
    mnWidth = mpAccess->Width();
    ImplWriteChunk(sal_uInt32(mnWidth));
    mnHeight = mpAccess->Height();
    ImplWriteChunk(sal_uInt32(mnHeight));

    if (mnWidth && mnHeight && mnBitsPerPixel && mbStatus)
    {
        sal_uInt8 nBitDepth = mnBitsPerPixel;
        if (mnBitsPerPixel <= 8)
            mnFilterType = 0;
        else
            mnFilterType = 4;

        sal_uInt8 nColorType = 2; // colortype:

        // bit 0 -> palette is used
        if (mpAccess->HasPalette()) // bit 1 -> color is used
            nColorType |= 1; // bit 2 -> alpha channel is used
        else
            nBitDepth /= 3;

        if (mpMaskAccess)
            nColorType |= 4;

        ImplWriteChunk(nBitDepth);
        ImplWriteChunk(nColorType); // colortype
        ImplWriteChunk(static_cast<sal_uInt8>(0)); // compression type
        ImplWriteChunk(static_cast<sal_uInt8>(0)); // filter type - is not supported in this version
        ImplWriteChunk(static_cast<sal_uInt8>(mnInterlaced)); // interlace type
    }
    else
    {
        mbStatus = false;
    }
    return mbStatus;
}

void PNGWriterImpl::ImplWritePalette()
{
    const sal_uLong nCount = mpAccess->GetPaletteEntryCount();
    std::unique_ptr<sal_uInt8[]> pTempBuf(new sal_uInt8[nCount * 3]);
    sal_uInt8* pTmp = pTempBuf.get();

    ImplOpenChunk(PNGCHUNK_PLTE);

    for (sal_uLong i = 0; i < nCount; i++)
    {
        const BitmapColor& rColor = mpAccess->GetPaletteColor(i);
        *pTmp++ = rColor.GetRed();
        *pTmp++ = rColor.GetGreen();
        *pTmp++ = rColor.GetBlue();
    }
    ImplWriteChunk(pTempBuf.get(), nCount * 3);
}

void PNGWriterImpl::ImplWritepHYs(const BitmapEx& rBmpEx)
{
    if (rBmpEx.GetPrefMapMode().GetMapUnit() != MapUnit::Map100thMM)
        return;

    Size aPrefSize(rBmpEx.GetPrefSize());

    if (aPrefSize.Width() && aPrefSize.Height() && mnWidth && mnHeight)
    {
        ImplOpenChunk(PNGCHUNK_pHYs);
        sal_uInt32 nPrefSizeX = static_cast<sal_uInt32>(
            100000.0 / (static_cast<double>(aPrefSize.Width()) / mnWidth) + 0.5);
        sal_uInt32 nPrefSizeY = static_cast<sal_uInt32>(
            100000.0 / (static_cast<double>(aPrefSize.Height()) / mnHeight) + 0.5);
        ImplWriteChunk(nPrefSizeX);
        ImplWriteChunk(nPrefSizeY);
        ImplWriteChunk(sal_uInt8(1)); // nMapUnit
    }
}

void PNGWriterImpl::ImplWriteIDAT()
{
    mnDeflateInSize = mnBitsPerPixel;

    if (mpMaskAccess)
        mnDeflateInSize += 8;

    mnBBP = (mnDeflateInSize + 7) >> 3;

    mnDeflateInSize = mnBBP * mnWidth + 1;

    mpDeflateInBuf.reset(new sal_uInt8[mnDeflateInSize]);

    if (mnFilterType) // using filter type 4 we need memory for the scanline 3 times
    {
        mpPreviousScan.reset(new sal_uInt8[mnDeflateInSize]);
        mpCurrentScan.reset(new sal_uInt8[mnDeflateInSize]);
        ImplClearFirstScanline();
    }
    mpZCodec.BeginCompression(mnCompLevel);
    SvMemoryStream aOStm;
    if (mnInterlaced == 0)
    {
        for (sal_uLong nY = 0; nY < mnHeight; nY++)
        {
            mpZCodec.Write(aOStm, mpDeflateInBuf.get(), ImplGetFilter(nY));
        }
    }
    else
    {
        // interlace mode
        sal_uLong nY;
        for (nY = 0; nY < mnHeight; nY += 8) // pass 1
        {
            mpZCodec.Write(aOStm, mpDeflateInBuf.get(), ImplGetFilter(nY, 0, 8));
        }
        ImplClearFirstScanline();

        for (nY = 0; nY < mnHeight; nY += 8) // pass 2
        {
            mpZCodec.Write(aOStm, mpDeflateInBuf.get(), ImplGetFilter(nY, 4, 8));
        }
        ImplClearFirstScanline();

        if (mnHeight >= 5) // pass 3
        {
            for (nY = 4; nY < mnHeight; nY += 8)
            {
                mpZCodec.Write(aOStm, mpDeflateInBuf.get(), ImplGetFilter(nY, 0, 4));
            }
            ImplClearFirstScanline();
        }

        for (nY = 0; nY < mnHeight; nY += 4) // pass 4
        {
            mpZCodec.Write(aOStm, mpDeflateInBuf.get(), ImplGetFilter(nY, 2, 4));
        }
        ImplClearFirstScanline();

        if (mnHeight >= 3) // pass 5
        {
            for (nY = 2; nY < mnHeight; nY += 4)
            {
                mpZCodec.Write(aOStm, mpDeflateInBuf.get(), ImplGetFilter(nY, 0, 2));
            }
            ImplClearFirstScanline();
        }

        for (nY = 0; nY < mnHeight; nY += 2) // pass 6
        {
            mpZCodec.Write(aOStm, mpDeflateInBuf.get(), ImplGetFilter(nY, 1, 2));
        }
        ImplClearFirstScanline();

        if (mnHeight >= 2) // pass 7
        {
            for (nY = 1; nY < mnHeight; nY += 2)
            {
                mpZCodec.Write(aOStm, mpDeflateInBuf.get(), ImplGetFilter(nY));
            }
        }
    }
    mpZCodec.EndCompression();

    if (mnFilterType) // using filter type 4 we need memory for the scanline 3 times
    {
        mpCurrentScan.reset();
        mpPreviousScan.reset();
    }
    mpDeflateInBuf.reset();

    sal_uInt32 nIDATSize = aOStm.Tell();
    sal_uInt32 nBytes, nBytesToWrite = nIDATSize;
    while (nBytesToWrite)
    {
        nBytes = nBytesToWrite <= mnMaxChunkSize ? nBytesToWrite : mnMaxChunkSize;
        ImplOpenChunk(PNGCHUNK_IDAT);
        ImplWriteChunk(
            const_cast<unsigned char*>(static_cast<unsigned char const*>(aOStm.GetData()))
                + (nIDATSize - nBytesToWrite),
            nBytes);
        nBytesToWrite -= nBytes;
    }
}

// ImplGetFilter writes the complete Scanline (nY) - in interlace mode the parameter nXStart and nXAdd
// appends to the currently used pass
// the complete size of scanline will be returned - in interlace mode zero is possible!

sal_uLong PNGWriterImpl::ImplGetFilter(sal_uLong nY, sal_uLong nXStart, sal_uLong nXAdd)
{
    sal_uInt8* pDest;

    if (mnFilterType)
        pDest = mpCurrentScan.get();
    else
        pDest = mpDeflateInBuf.get();

    if (nXStart < mnWidth)
    {
        *pDest++ = mnFilterType; // in this version the filter type is either 0 or 4

        if (mpAccess
                ->HasPalette()) // alphachannel is not allowed by pictures including palette entries
        {
            switch (mnBitsPerPixel)
            {
                case 1:
                {
                    Scanline pScanline = mpAccess->GetScanline(nY);
                    sal_uLong nX, nXIndex;
                    for (nX = nXStart, nXIndex = 0; nX < mnWidth; nX += nXAdd, nXIndex++)
                    {
                        sal_uLong nShift = (nXIndex & 7) ^ 7;
                        if (nShift == 7)
                            *pDest = mpAccess->GetIndexFromData(pScanline, nX) << nShift;
                        else if (nShift == 0)
                            *pDest++ |= mpAccess->GetIndexFromData(pScanline, nX) << nShift;
                        else
                            *pDest |= mpAccess->GetIndexFromData(pScanline, nX) << nShift;
                    }
                    if ((nXIndex & 7) != 0)
                        pDest++; // byte is not completely used, so the bufferpointer is to correct
                }
                break;

                case 4:
                {
                    Scanline pScanline = mpAccess->GetScanline(nY);
                    sal_uLong nX, nXIndex;
                    for (nX = nXStart, nXIndex = 0; nX < mnWidth; nX += nXAdd, nXIndex++)
                    {
                        if (nXIndex & 1)
                            *pDest++ |= mpAccess->GetIndexFromData(pScanline, nX);
                        else
                            *pDest = mpAccess->GetIndexFromData(pScanline, nX) << 4;
                    }
                    if (nXIndex & 1)
                        pDest++;
                }
                break;

                case 8:
                {
                    Scanline pScanline = mpAccess->GetScanline(nY);
                    for (sal_uLong nX = nXStart; nX < mnWidth; nX += nXAdd)
                    {
                        *pDest++ = mpAccess->GetIndexFromData(pScanline, nX);
                    }
                }
                break;

                default:
                    mbStatus = false;
                    break;
            }
        }
        else
        {
            if (mpMaskAccess) // mpMaskAccess != NULL -> alphachannel is to create
            {
                if (mbTrueAlpha)
                {
                    Scanline pScanline = mpAccess->GetScanline(nY);
                    Scanline pScanlineMask = mpMaskAccess->GetScanline(nY);
                    for (sal_uLong nX = nXStart; nX < mnWidth; nX += nXAdd)
                    {
                        const BitmapColor& rColor = mpAccess->GetPixelFromData(pScanline, nX);
                        *pDest++ = rColor.GetRed();
                        *pDest++ = rColor.GetGreen();
                        *pDest++ = rColor.GetBlue();
                        *pDest++ = mpMaskAccess->GetIndexFromData(pScanlineMask, nX);
                    }
                }
                else
                {
                    const BitmapColor aTrans(mpMaskAccess->GetBestMatchingColor(COL_WHITE));
                    Scanline pScanline = mpAccess->GetScanline(nY);
                    Scanline pScanlineMask = mpMaskAccess->GetScanline(nY);

                    for (sal_uLong nX = nXStart; nX < mnWidth; nX += nXAdd)
                    {
                        const BitmapColor& rColor = mpAccess->GetPixelFromData(pScanline, nX);
                        *pDest++ = rColor.GetRed();
                        *pDest++ = rColor.GetGreen();
                        *pDest++ = rColor.GetBlue();

                        if (mpMaskAccess->GetPixelFromData(pScanlineMask, nX) == aTrans)
                            *pDest++ = 0;
                        else
                            *pDest++ = 0xff;
                    }
                }
            }
            else
            {
                Scanline pScanline = mpAccess->GetScanline(nY);
                for (sal_uLong nX = nXStart; nX < mnWidth; nX += nXAdd)
                {
                    const BitmapColor& rColor = mpAccess->GetPixelFromData(pScanline, nX);
                    *pDest++ = rColor.GetRed();
                    *pDest++ = rColor.GetGreen();
                    *pDest++ = rColor.GetBlue();
                }
            }
        }
    }
    // filter type4 ( PAETH ) will be used only for 24bit graphics
    if (mnFilterType)
    {
        mnDeflateInSize = pDest - mpCurrentScan.get();
        pDest = mpDeflateInBuf.get();
        *pDest++ = 4; // filter type

        sal_uInt8* p1 = mpCurrentScan.get() + 1; // Current Pixel
        sal_uInt8* p2 = p1 - mnBBP; // left pixel
        sal_uInt8* p3 = mpPreviousScan.get(); // upper pixel
        sal_uInt8* p4 = p3 - mnBBP; // upperleft Pixel;

        while (pDest < mpDeflateInBuf.get() + mnDeflateInSize)
        {
            sal_uLong nb = *p3++;
            sal_uLong na, nc;
            if (p2 >= mpCurrentScan.get() + 1)
            {
                na = *p2;
                nc = *p4;
            }
            else
            {
                na = nc = 0;
            }

            tools::Long np = na + nb - nc;
            tools::Long npa = np - na;
            tools::Long npb = np - nb;
            tools::Long npc = np - nc;

            if (npa < 0)
                npa = -npa;
            if (npb < 0)
                npb = -npb;
            if (npc < 0)
                npc = -npc;

            if (npa <= npb && npa <= npc)
                *pDest++ = *p1++ - static_cast<sal_uInt8>(na);
            else if (npb <= npc)
                *pDest++ = *p1++ - static_cast<sal_uInt8>(nb);
            else
                *pDest++ = *p1++ - static_cast<sal_uInt8>(nc);

            p4++;
            p2++;
        }
        for (tools::Long i = 0; i < static_cast<tools::Long>(mnDeflateInSize - 1); i++)
        {
            mpPreviousScan[i] = mpCurrentScan[i + 1];
        }
    }
    else
    {
        mnDeflateInSize = pDest - mpDeflateInBuf.get();
    }
    return mnDeflateInSize;
}

void PNGWriterImpl::ImplClearFirstScanline()
{
    if (mnFilterType)
        memset(mpPreviousScan.get(), 0, mnDeflateInSize);
}

void PNGWriterImpl::ImplOpenChunk(sal_uLong nChunkType)
{
    maChunkSeq.emplace_back();
    maChunkSeq.back().nType = nChunkType;
}

void PNGWriterImpl::ImplWriteChunk(sal_uInt8 nSource)
{
    maChunkSeq.back().aData.push_back(nSource);
}

void PNGWriterImpl::ImplWriteChunk(sal_uInt32 nSource)
{
    vcl::PNGWriter::ChunkData& rChunkData = maChunkSeq.back();
    rChunkData.aData.push_back(static_cast<sal_uInt8>(nSource >> 24));
    rChunkData.aData.push_back(static_cast<sal_uInt8>(nSource >> 16));
    rChunkData.aData.push_back(static_cast<sal_uInt8>(nSource >> 8));
    rChunkData.aData.push_back(static_cast<sal_uInt8>(nSource));
}

void PNGWriterImpl::ImplWriteChunk(unsigned char const* pSource, sal_uInt32 nDatSize)
{
    if (nDatSize)
    {
        vcl::PNGWriter::ChunkData& rChunkData = maChunkSeq.back();
        sal_uInt32 nSize = rChunkData.aData.size();
        rChunkData.aData.resize(nSize + nDatSize);
        memcpy(&rChunkData.aData[nSize], pSource, nDatSize);
    }
}

PNGWriter::PNGWriter(const BitmapEx& rBmpEx,
                     const css::uno::Sequence<css::beans::PropertyValue>* pFilterData)
    : mpImpl(new vcl::PNGWriterImpl(rBmpEx, pFilterData))
{
}

PNGWriter::~PNGWriter() {}

bool PNGWriter::Write(SvStream& rStream) { return mpImpl->Write(rStream); }

std::vector<vcl::PNGWriter::ChunkData>& PNGWriter::GetChunks() { return mpImpl->GetChunks(); }

} // namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
