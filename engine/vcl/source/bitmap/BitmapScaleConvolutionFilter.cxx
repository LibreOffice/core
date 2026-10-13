/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <sal/log.hxx>
#include <tools/cpuid.hxx>
#include <tools/helpers.hxx>
#include <comphelper/threadpool.hxx>

#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/BitmapScaleConvolutionFilter.hxx>
#include <bitmap/BitmapScaleConvolutionRows.hxx>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <memory>
#include <vector>

namespace vcl
{

namespace
{

constexpr sal_Int32 constStripRows = 32;

// About a millisecond of work, below which threads are not worth it.
constexpr sal_Int64 constThreadedWorkMinimum = 1 << 20;

/// Row kernels for the current CPU.
struct RowFunctions
{
    void (*mpConvolveRow)(const uint8_t*, uint8_t*, int32_t, const int16_t*, const int32_t*,
                          int32_t);
    void (*mpBlendRows)(const uint8_t* const*, const int16_t*, int32_t, uint8_t*, int32_t);
    void (*mpBoxHalveRow)(const uint8_t* const*, uint8_t*, int32_t);
    void (*mpBoxQuarterRow)(const uint8_t* const*, uint8_t*, int32_t);
    void (*mpExpandRow24)(const uint8_t*, uint8_t*, int32_t, bool);
    void (*mpPackRow24)(const uint8_t*, uint8_t*, int32_t);
};

const RowFunctions& getRowFunctions()
{
#ifdef LO_X86_SIMD_AVAILABLE
    static const bool bHasAVX2 = cpuid::hasAVX2();
    static const RowFunctions aAVX2Functions{
        bitmap::convolution::convolveRowN32AVX2,    bitmap::convolution::blendRowsN32AVX2,
        bitmap::convolution::boxHalveRowN32AVX2,    bitmap::convolution::boxQuarterRowN32AVX2,
        bitmap::convolution::expandRowN24ToN32AVX2, bitmap::convolution::packRowN32ToN24AVX2
    };
    if (bHasAVX2)
        return aAVX2Functions;
#endif
    static const RowFunctions aFunctions{
        bitmap::convolution::convolveRowN32,    bitmap::convolution::blendRowsN32,
        bitmap::convolution::boxHalveRowN32,    bitmap::convolution::boxQuarterRowN32,
        bitmap::convolution::expandRowN24ToN32, bitmap::convolution::packRowN32ToN24
    };
    return aFunctions;
}

bool useThreads(sal_Int64 nWork)
{
    static const bool bThreadsDisabled = std::getenv("VCL_NO_THREAD_SCALE") != nullptr;
    return !bThreadsDisabled && nWork >= constThreadedWorkMinimum;
}

class StripTask : public comphelper::ThreadTask
{
    const std::function<void(sal_Int32, sal_Int32)>& mrProcess;
    sal_Int32 mnFirst;
    sal_Int32 mnLast;

public:
    StripTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag,
              const std::function<void(sal_Int32, sal_Int32)>& rProcess, sal_Int32 nFirst,
              sal_Int32 nLast)
        : comphelper::ThreadTask(pTag)
        , mrProcess(rProcess)
        , mnFirst(nFirst)
        , mnLast(nLast)
    {
    }

    void doWork() override { mrProcess(mnFirst, mnLast); }
};

/// Calls rProcess(nFirst, nLast) for strips of nStripRows rows, in parallel when bThreaded is set.
/// The strips are independent.
void processStrips(sal_Int32 nRows, sal_Int32 nStripRows, bool bThreaded,
                   const std::function<void(sal_Int32 nFirst, sal_Int32 nLast)>& rProcess)
{
    if (bThreaded && nRows > nStripRows)
    {
        comphelper::ThreadPool& rPool = comphelper::ThreadPool::getSharedOptimalPool();
        std::shared_ptr<comphelper::ThreadTaskTag> pTag
            = comphelper::ThreadPool::createThreadTaskTag();
        try
        {
            sal_Int32 nFirst = 0;
            for (; nFirst + nStripRows < nRows; nFirst += nStripRows)
            {
                rPool.pushTask(
                    std::make_unique<StripTask>(pTag, rProcess, nFirst, nFirst + nStripRows - 1));
            }
            // The calling thread takes the last strip.
            rProcess(nFirst, nRows - 1);
            rPool.waitUntilDone(pTag);
            return;
        }
        catch (...)
        {
            SAL_WARN("vcl.gdi", "threaded convolution scaling failed, scaling in one thread");
            rPool.waitUntilDone(pTag);
        }
    }

    rProcess(0, nRows - 1);
}

/// Weights along one axis. Destination pixel i is made of maCounts[i] source pixels from
/// maStarts[i], with weights from maWeights[i * mnStride] that sum to 1 << constWeightShift.
/// Empty means the axis is not scaled.
struct Contributions
{
    std::vector<sal_Int16> maWeights;
    std::vector<sal_Int32> maStarts;
    std::vector<sal_Int32> maCounts;
    sal_Int32 mnStride = 0;
};

Contributions calculateContributions(sal_Int32 nSourceSize, sal_Int32 nDestinationSize,
                                     const Kernel& rKernel)
{
    const double fSamplingRadius = rKernel.GetWidth();
    const double fScale = nDestinationSize / double(nSourceSize);
    const double fScaledRadius = (fScale < 1.0) ? fSamplingRadius / fScale : fSamplingRadius;
    const double fFilterFactor = std::min(fScale, 1.0);
    constexpr sal_Int32 constWeightSum = 1 << bitmap::convolution::constWeightShift;

    Contributions aContributions;
    aContributions.mnStride = std::min(nSourceSize, sal_Int32(std::ceil(fScaledRadius)) * 2 + 3);
    aContributions.maWeights.resize(size_t(nDestinationSize) * aContributions.mnStride);
    aContributions.maStarts.resize(nDestinationSize);
    aContributions.maCounts.resize(nDestinationSize);

    std::vector<double> aSampled(aContributions.mnStride);
    std::vector<sal_Int32> aRounded(aContributions.mnStride);

    for (sal_Int32 i = 0; i < nDestinationSize; i++)
    {
        const double fCenter = i / fScale;
        const sal_Int32 nLeft = sal_Int32(std::floor(fCenter - fScaledRadius));
        const sal_Int32 nRight = sal_Int32(std::ceil(fCenter + fScaledRadius));
        const sal_Int32 nFirst = std::clamp<sal_Int32>(nLeft, 0, nSourceSize - 1);
        const sal_Int32 nLast = std::clamp<sal_Int32>(nRight, 0, nSourceSize - 1);
        const sal_Int32 nSize = nLast - nFirst + 1;
        assert(nSize <= aContributions.mnStride);

        // Samples past the edge add their weight to the edge pixel.
        std::fill_n(aSampled.begin(), nSize, 0.0);
        double fTotal = 0.0;
        for (sal_Int32 j = nLeft; j <= nRight; j++)
        {
            const double fWeight = rKernel.Calculate(fFilterFactor * (fCenter - j));
            aSampled[std::clamp<sal_Int32>(j, 0, nSourceSize - 1) - nFirst] += fWeight;
            fTotal += fWeight;
        }
        assert(fTotal > 0.0);

        sal_Int32 nRoundedTotal = 0;
        sal_Int32 nLargest = 0;
        for (sal_Int32 k = 0; k < nSize; k++)
        {
            aRounded[k] = std::lround(aSampled[k] / fTotal * constWeightSum);
            nRoundedTotal += aRounded[k];
            if (aRounded[k] > aRounded[nLargest])
                nLargest = k;
        }
        // Sum to exactly one, so an area of one color keeps its color.
        aRounded[nLargest] += constWeightSum - nRoundedTotal;

        sal_Int32 nBegin = 0;
        while (aRounded[nBegin] == 0)
            nBegin++;
        sal_Int32 nEnd = nSize;
        while (aRounded[nEnd - 1] == 0)
            nEnd--;

        aContributions.maStarts[i] = nFirst + nBegin;
        aContributions.maCounts[i] = nEnd - nBegin;
        sal_Int16* pWeights
            = aContributions.maWeights.data() + size_t(i) * aContributions.mnStride;
        for (sal_Int32 k = nBegin; k < nEnd; k++)
        {
            assert(aRounded[k] >= SAL_MIN_INT16 && aRounded[k] <= SAL_MAX_INT16);
            pWeights[k - nBegin] = sal_Int16(aRounded[k]);
        }
    }

    return aContributions;
}

/// Averages blocks of nFactor (2 or 4) pixels. A partial block at the end joins the last block.
Contributions calculateBlockContributions(sal_Int32 nSourceSize, sal_Int32 nFactor)
{
    const sal_Int32 nDestinationSize = nSourceSize / nFactor;
    const sal_Int32 nLastCount = nSourceSize - (nDestinationSize - 1) * nFactor;
    constexpr sal_Int32 constWeightSum = 1 << bitmap::convolution::constWeightShift;

    Contributions aContributions;
    aContributions.mnStride = nLastCount;
    aContributions.maWeights.assign(size_t(nDestinationSize) * nLastCount, 0);
    aContributions.maStarts.resize(nDestinationSize);
    aContributions.maCounts.assign(nDestinationSize, nFactor);
    aContributions.maCounts.back() = nLastCount;
    for (sal_Int32 i = 0; i < nDestinationSize; i++)
    {
        const sal_Int32 nCount = aContributions.maCounts[i];
        sal_Int16* pWeights = aContributions.maWeights.data() + size_t(i) * nLastCount;
        std::fill_n(pWeights, nCount, sal_Int16(constWeightSum / nCount));
        // The first pixel takes the remainder of the division, so the weights sum to exactly one.
        pWeights[0] += sal_Int16(constWeightSum - constWeightSum / nCount * nCount);
        aContributions.maStarts[i] = i * nFactor;
    }

    return aContributions;
}

/// Horizontal weights in the row kernel layout. Destination pixel x is made of mnWindow source
/// pixels from maStarts[x], with weights from maWeights[x * windowWeightCount(mnWindow)].
struct WindowContributions
{
    std::vector<sal_Int16> maWeights;
    std::vector<int32_t> maStarts;
    sal_Int32 mnWindow = 0;
};

WindowContributions calculateWindowContributions(const Contributions& rContributions,
                                                 sal_Int32 nSourceSize, sal_Int32 nWindowMultiple)
{
    const sal_Int32 nDestinationSize = sal_Int32(rContributions.maStarts.size());
    const sal_Int32 nLargestCount
        = *std::max_element(rContributions.maCounts.begin(), rContributions.maCounts.end());

    // One window for all pixels, rounded up to nWindowMultiple and capped at the source width.
    WindowContributions aWindow;
    aWindow.mnWindow = std::min(nSourceSize, (nLargestCount + nWindowMultiple - 1) / nWindowMultiple
                                                 * nWindowMultiple);
    const sal_Int32 nWeightCount = bitmap::convolution::windowWeightCount(aWindow.mnWindow);
    aWindow.maStarts.resize(nDestinationSize);
    aWindow.maWeights.resize(size_t(nDestinationSize) * nWeightCount);

    std::vector<sal_Int16> aTaps(aWindow.mnWindow);
    for (sal_Int32 x = 0; x < nDestinationSize; x++)
    {
        // A window past the source end starts earlier, with leading zero weights.
        const sal_Int32 nStart
            = std::min(rContributions.maStarts[x], nSourceSize - aWindow.mnWindow);
        const sal_Int32 nOffset = rContributions.maStarts[x] - nStart;
        std::fill(aTaps.begin(), aTaps.end(), 0);
        std::copy_n(rContributions.maWeights.data() + size_t(x) * rContributions.mnStride,
                    rContributions.maCounts[x], aTaps.begin() + nOffset);

        aWindow.maStarts[x] = nStart;
        bitmap::convolution::spreadWindowWeights(aTaps.data(), aWindow.mnWindow,
                                         aWindow.maWeights.data() + size_t(x) * nWeightCount);
    }

    return aWindow;
}

/// The 32-bit format the rows are scaled in. For 24-bit, the same color order with alpha last.
ScanlineFormat rowFormatFor(ScanlineFormat eDestination)
{
    switch (eDestination)
    {
        case ScanlineFormat::N24BitTcBgr:
            return ScanlineFormat::N32BitTcBgra;
        case ScanlineFormat::N24BitTcRgb:
            return ScanlineFormat::N32BitTcRgba;
        default:
            return eDestination;
    }
}

/// Reads source rows as premultiplied 32-bit pixels in the row format.
class SourceRowReader
{
    enum class Mode
    {
        // Already in the row format.
        Direct,
        // 24-bit, same color order.
        From24Bit,
        // 24-bit, red and blue swapped.
        From24BitSwapped,
        // 32-bit with alpha last, red and blue swapped.
        From32BitSwapped,
        // Anything else, pixel by pixel.
        General
    };

    const BitmapReadAccess& mrReadAccess;
    const RowFunctions& mrFunctions;
    FncSetPixel mpSetPixel;
    sal_Int32 mnWidth;
    Mode meMode;

    static Mode selectMode(ScanlineFormat eSource, ScanlineFormat eRow)
    {
        if (eSource == eRow)
            return Mode::Direct;

        const bool bRowBlueFirst
            = eRow == ScanlineFormat::N32BitTcBgra || eRow == ScanlineFormat::N32BitTcBgrx;
        const bool bRowRedFirst
            = eRow == ScanlineFormat::N32BitTcRgba || eRow == ScanlineFormat::N32BitTcRgbx;

        switch (eSource)
        {
            case ScanlineFormat::N32BitTcBgra:
                return eRow == ScanlineFormat::N32BitTcRgba ? Mode::From32BitSwapped
                                                            : Mode::General;
            case ScanlineFormat::N32BitTcRgba:
                return eRow == ScanlineFormat::N32BitTcBgra ? Mode::From32BitSwapped
                                                            : Mode::General;
            case ScanlineFormat::N24BitTcBgr:
                if (bRowBlueFirst)
                    return Mode::From24Bit;
                return bRowRedFirst ? Mode::From24BitSwapped : Mode::General;
            case ScanlineFormat::N24BitTcRgb:
                if (bRowRedFirst)
                    return Mode::From24Bit;
                return bRowBlueFirst ? Mode::From24BitSwapped : Mode::General;
            default:
                return Mode::General;
        }
    }

public:
    SourceRowReader(const BitmapReadAccess& rReadAccess, ScanlineFormat eRowFormat)
        : mrReadAccess(rReadAccess)
        , mrFunctions(getRowFunctions())
        , mpSetPixel(BitmapReadAccess::SetPixelFunction(eRowFormat))
        , mnWidth(rReadAccess.Width())
        , meMode(selectMode(rReadAccess.GetScanlineFormat(), eRowFormat))
    {
    }

    /// Rows come straight from the source and need no buffer.
    bool isDirect() const { return meMode == Mode::Direct; }

    /// Returns row nY, converted into pBuffer (4 bytes per pixel) when needed.
    const sal_uInt8* readRow(sal_Int32 nY, sal_uInt8* pBuffer) const
    {
        const sal_uInt8* pSource = mrReadAccess.GetScanline(nY);
        switch (meMode)
        {
            case Mode::Direct:
                return pSource;
            case Mode::From24Bit:
            case Mode::From24BitSwapped:
                mrFunctions.mpExpandRow24(pSource, pBuffer, mnWidth,
                                          meMode == Mode::From24BitSwapped);
                break;
            case Mode::From32BitSwapped:
                for (sal_Int32 x = 0; x < mnWidth; x++)
                {
                    pBuffer[x * 4] = pSource[x * 4 + 2];
                    pBuffer[x * 4 + 1] = pSource[x * 4 + 1];
                    pBuffer[x * 4 + 2] = pSource[x * 4];
                    pBuffer[x * 4 + 3] = pSource[x * 4 + 3];
                }
                break;
            case Mode::General:
                for (sal_Int32 x = 0; x < mnWidth; x++)
                    mpSetPixel(pBuffer, x, mrReadAccess.GetColorFromData(pSource, x));
                break;
        }
        return pBuffer;
    }
};

/// Averages the block of nRowCount rows by nPixelCount pixels that starts at pixel nFirst of each
/// row in pRows.
void averageBlockN32(const sal_uInt8* const* pRows, sal_Int32 nRowCount, sal_Int32 nFirst,
                     sal_Int32 nPixelCount, sal_uInt8* pDestination)
{
    const sal_Int32 nTotal = nRowCount * nPixelCount;
    for (sal_Int32 c = 0; c < 4; c++)
    {
        sal_Int32 nSum = 0;
        for (sal_Int32 nRow = 0; nRow < nRowCount; nRow++)
        {
            const sal_uInt8* pPixel = pRows[nRow] + nFirst * 4 + c;
            for (sal_Int32 i = 0; i < nPixelCount; i++)
                nSum += pPixel[i * 4];
        }
        pDestination[c] = sal_uInt8((nSum + nTotal / 2) / nTotal);
    }
}

/// Shrinks rBitmap to 32-bit by averaging nFactor x nFactor blocks (nFactor is 2 or 4). A partial
/// block at the right or bottom edge joins the last block there.
bool ImplShrinkByBlocks(Bitmap& rBitmap, sal_Int32 nFactor)
{
    const sal_Int32 nWidth = rBitmap.GetSizePixel().Width();
    const sal_Int32 nHeight = rBitmap.GetSizePixel().Height();
    const sal_Int32 nNewWidth = nWidth / nFactor;
    const sal_Int32 nNewHeight = nHeight / nFactor;
    const sal_Int32 nLastColumns = nWidth - (nNewWidth - 1) * nFactor;
    const sal_Int32 nLastRows = nHeight - (nNewHeight - 1) * nFactor;

    BitmapScopedReadAccess pReadAccess(rBitmap);
    if (!pReadAccess)
        return false;

    Bitmap aResult(Size(nNewWidth, nNewHeight), vcl::PixelFormat::N32_BPP);
    BitmapScopedWriteAccess pWriteAccess(aResult);
    if (!pWriteAccess)
        return false;

    const SourceRowReader aReader(*pReadAccess, pWriteAccess->GetScanlineFormat());
    const RowFunctions& rFunctions = getRowFunctions();
    const auto pShrinkRow = nFactor == 4 ? rFunctions.mpBoxQuarterRow : rFunctions.mpBoxHalveRow;
    const size_t nRowBytes = size_t(nWidth) * 4;

    processStrips(
        nNewHeight, constStripRows, useThreads(sal_Int64(nWidth) * nHeight),
        [&](sal_Int32 nFirst, sal_Int32 nLast)
        {
            // The last destination row reads up to nFactor + 3 source rows.
            std::vector<sal_uInt8> aBuffer(aReader.isDirect() ? 0 : nLastRows * nRowBytes);
            const sal_uInt8* aRows[7];
            for (sal_Int32 y = nFirst; y <= nLast; y++)
            {
                const sal_Int32 nRowCount = y == nNewHeight - 1 ? nLastRows : nFactor;
                for (sal_Int32 i = 0; i < nRowCount; i++)
                {
                    sal_uInt8* pBuffer
                        = aReader.isDirect() ? nullptr : aBuffer.data() + i * nRowBytes;
                    aRows[i] = aReader.readRow(y * nFactor + i, pBuffer);
                }

                sal_uInt8* pDestination = pWriteAccess->GetScanline(y);
                if (nRowCount == nFactor)
                {
                    pShrinkRow(aRows, pDestination, nNewWidth);
                    if (nLastColumns != nFactor)
                    {
                        averageBlockN32(aRows, nFactor, (nNewWidth - 1) * nFactor, nLastColumns,
                                        pDestination + (nNewWidth - 1) * 4);
                    }
                }
                else
                {
                    for (sal_Int32 x = 0; x < nNewWidth; x++)
                    {
                        averageBlockN32(aRows, nRowCount, x * nFactor,
                                        x == nNewWidth - 1 ? nLastColumns : nFactor,
                                        pDestination + x * 4);
                    }
                }
            }
        });

    pWriteAccess.reset();
    pReadAccess.reset();
    rBitmap = std::move(aResult);
    return true;
}

/// Scales rSource to rTarget in ePixelFormat (N24_BPP or N32_BPP), in strips of destination rows.
/// A strip scales its source rows horizontally into a buffer that stays in the cache, then blends
/// them vertically. Rows are 32-bit and packed to 24-bit at the end when needed.
bool ImplConvolveRows(const Bitmap& rSource, Bitmap& rTarget, const Contributions& rHorizontal,
                      const Contributions& rVertical, vcl::PixelFormat ePixelFormat)
{
    const sal_Int32 nWidth = rSource.GetSizePixel().Width();
    const sal_Int32 nHeight = rSource.GetSizePixel().Height();
    const bool bScaleHorizontal = !rHorizontal.maStarts.empty();
    const bool bScaleVertical = !rVertical.maStarts.empty();
    const sal_Int32 nNewWidth = bScaleHorizontal ? sal_Int32(rHorizontal.maStarts.size()) : nWidth;
    const sal_Int32 nNewHeight = bScaleVertical ? sal_Int32(rVertical.maStarts.size()) : nHeight;

    BitmapScopedReadAccess pReadAccess(rSource);
    if (!pReadAccess)
        return false;

    rTarget = Bitmap(Size(nNewWidth, nNewHeight), ePixelFormat);
    BitmapScopedWriteAccess pWriteAccess(rTarget);
    if (!pWriteAccess)
        return false;

    const ScanlineFormat eRowFormat = rowFormatFor(pWriteAccess->GetScanlineFormat());
    const bool bPack = eRowFormat != pWriteAccess->GetScanlineFormat();
    const SourceRowReader aReader(*pReadAccess, eRowFormat);
    const RowFunctions& rFunctions = getRowFunctions();
    const size_t nRowBytes = size_t(nWidth) * 4;
    const size_t nNewRowBytes = size_t(nNewWidth) * 4;

    // A 24-bit row goes to pPackBuffer first and finishRow packs it into the bitmap.
    auto destinationRow = [&](sal_Int32 y, sal_uInt8* pPackBuffer)
    { return bPack ? pPackBuffer : pWriteAccess->GetScanline(y); };
    auto finishRow = [&](sal_Int32 y, const sal_uInt8* pPackBuffer)
    {
        if (bPack)
            rFunctions.mpPackRow24(pPackBuffer, pWriteAccess->GetScanline(y), nNewWidth);
    };

    // The vector kernel needs a window that is a multiple of 4, the scalar one is fastest with the
    // smallest window. A window capped at a narrow source width uses the scalar kernel.
    const bool bVectorKernel = rFunctions.mpConvolveRow != bitmap::convolution::convolveRowN32;
    WindowContributions aWindow;
    if (bScaleHorizontal)
        aWindow = calculateWindowContributions(rHorizontal, nWidth, bVectorKernel ? 4 : 1);
    const auto pConvolveRow = aWindow.mnWindow % 4 == 0 ? rFunctions.mpConvolveRow
                                                        : bitmap::convolution::convolveRowN32;

    auto scaleRow = [&](const sal_uInt8* pSource, sal_uInt8* pDestination)
    {
        if (bScaleHorizontal)
        {
            pConvolveRow(pSource, pDestination, nNewWidth, aWindow.maWeights.data(),
                         aWindow.maStarts.data(), aWindow.mnWindow);
        }
        else
            std::copy_n(pSource, nRowBytes, pDestination);
    };

    sal_Int64 nWork = 0;
    if (bScaleHorizontal)
        nWork += sal_Int64(nHeight) * nNewWidth * aWindow.mnWindow;
    if (bScaleVertical)
        nWork += sal_Int64(nNewHeight) * nNewWidth * rVertical.mnStride;
    const bool bThreaded = useThreads(nWork);

    if (!bScaleVertical)
    {
        processStrips(nNewHeight, constStripRows, bThreaded,
                      [&](sal_Int32 nFirst, sal_Int32 nLast)
                      {
                          std::vector<sal_uInt8> aConverted(aReader.isDirect() ? 0 : nRowBytes);
                          std::vector<sal_uInt8> aPackBuffer(bPack ? nNewRowBytes : 0);
                          for (sal_Int32 y = nFirst; y <= nLast; y++)
                          {
                              scaleRow(aReader.readRow(y, aConverted.data()),
                                       destinationRow(y, aPackBuffer.data()));
                              finishRow(y, aPackBuffer.data());
                          }
                      });
        return true;
    }

    // Neighboring strips both scale the source rows they share. Strips of 4 times the vertical
    // filter size keep the repeated work small, and at least 16 strips keep the threads busy
    // for a large upscale.
    const sal_Int32 nFilterRows
        = sal_Int32(std::ceil(4.0 * rVertical.mnStride * nNewHeight / nHeight));
    const sal_Int32 nStripRows
        = std::clamp(nFilterRows, constStripRows, std::max(constStripRows, nNewHeight / 16));

    processStrips(
        nNewHeight, nStripRows, bThreaded,
        [&](sal_Int32 nFirst, sal_Int32 nLast)
        {
            sal_Int32 nFirstSourceRow = nHeight;
            sal_Int32 nLastSourceRow = 0;
            for (sal_Int32 y = nFirst; y <= nLast; y++)
            {
                nFirstSourceRow = std::min(nFirstSourceRow, rVertical.maStarts[y]);
                nLastSourceRow
                    = std::max(nLastSourceRow, rVertical.maStarts[y] + rVertical.maCounts[y] - 1);
            }
            const sal_Int32 nSourceRows = nLastSourceRow - nFirstSourceRow + 1;

            // Rows used as they are need no copy.
            const bool bCopyRows = bScaleHorizontal || !aReader.isDirect();
            std::vector<sal_uInt8> aRows(bCopyRows ? nSourceRows * nNewRowBytes : 0);
            std::vector<sal_uInt8> aConverted(bScaleHorizontal && !aReader.isDirect() ? nRowBytes
                                                                                      : 0);
            std::vector<const sal_uInt8*> aRowPointers(nSourceRows);
            std::vector<sal_uInt8> aPackBuffer(bPack ? nNewRowBytes : 0);

            for (sal_Int32 i = 0; i < nSourceRows; i++)
            {
                sal_uInt8* pRow = bCopyRows ? aRows.data() + i * nNewRowBytes : nullptr;
                if (bScaleHorizontal)
                {
                    scaleRow(aReader.readRow(nFirstSourceRow + i, aConverted.data()), pRow);
                    aRowPointers[i] = pRow;
                }
                else
                    aRowPointers[i] = aReader.readRow(nFirstSourceRow + i, pRow);
            }

            for (sal_Int32 y = nFirst; y <= nLast; y++)
            {
                rFunctions.mpBlendRows(
                    aRowPointers.data() + (rVertical.maStarts[y] - nFirstSourceRow),
                    rVertical.maWeights.data() + size_t(y) * rVertical.mnStride,
                    rVertical.maCounts[y], destinationRow(y, aPackBuffer.data()), nNewWidth);
                finishRow(y, aPackBuffer.data());
            }
        });

    return true;
}

/// The largest block size (4 or 2) that leaves at least 2 * nNewSize pixels, or 1.
sal_Int32 blockFactor(sal_Int32 nSize, sal_Int32 nNewSize)
{
    if (nSize / 8 >= nNewSize)
        return 4;
    return nSize / 4 >= nNewSize ? 2 : 1;
}

/// Averages blocks along each axis that is at least 4 times the destination size. The wide filter
/// of a large reduction is slow and its fixed point weights get too small to be precise. The
/// result stays at least twice the destination size for the convolution to finish.
bool ImplShrinkBeforeConvolution(Bitmap& rBitmap, sal_Int32 nNewWidth, sal_Int32 nNewHeight)
{
    while (true)
    {
        const sal_Int32 nWidth = rBitmap.GetSizePixel().Width();
        const sal_Int32 nHeight = rBitmap.GetSizePixel().Height();
        const sal_Int32 nFactorX = blockFactor(nWidth, nNewWidth);
        const sal_Int32 nFactorY = blockFactor(nHeight, nNewHeight);

        if (nFactorX == 1 && nFactorY == 1)
            return true;

        if (nFactorX == nFactorY)
        {
            if (!ImplShrinkByBlocks(rBitmap, nFactorX))
                return false;
        }
        else
        {
            Bitmap aResult;
            if (!ImplConvolveRows(
                    rBitmap, aResult,
                    nFactorX > 1 ? calculateBlockContributions(nWidth, nFactorX) : Contributions(),
                    nFactorY > 1 ? calculateBlockContributions(nHeight, nFactorY) : Contributions(),
                    vcl::PixelFormat::N32_BPP))
            {
                return false;
            }
            rBitmap = std::move(aResult);
        }
    }
}

bool ImplScaleConvolution(Bitmap& rBitmap, const double& rScaleX, const double& rScaleY, const Kernel& rKernel)
{
    const bool bMirrorHor(rScaleX < 0.0);
    const bool bMirrorVer(rScaleY < 0.0);
    const double fScaleX(bMirrorHor ? -rScaleX : rScaleX);
    const double fScaleY(bMirrorVer ? -rScaleY : rScaleY);
    const sal_Int32 nWidth(rBitmap.GetSizePixel().Width());
    const sal_Int32 nHeight(rBitmap.GetSizePixel().Height());
    const sal_Int32 nNewWidth(basegfx::fround(nWidth * fScaleX));
    const sal_Int32 nNewHeight(basegfx::fround(nHeight * fScaleY));
    const bool bScaleHor(nWidth != nNewWidth);
    const bool bScaleVer(nHeight != nNewHeight);
    const bool bMirror(bMirrorHor || bMirrorVer);

    if (!bMirror && !bScaleHor && !bScaleVer)
    {
        return true;
    }

    bool bResult(true);
    BmpMirrorFlags nMirrorFlags(BmpMirrorFlags::NONE);
    bool bMirrorAfter(false);

    if (bMirror)
    {
        if(bMirrorHor)
        {
            nMirrorFlags |= BmpMirrorFlags::Horizontal;
        }

        if(bMirrorVer)
        {
            nMirrorFlags |= BmpMirrorFlags::Vertical;
        }

        const sal_Int32 nStartSize(nWidth * nHeight);
        const sal_Int32 nEndSize(nNewWidth * nNewHeight);

        bMirrorAfter = nStartSize > nEndSize;

        if(!bMirrorAfter)
        {
            bResult = rBitmap.Mirror(nMirrorFlags);
        }
    }

    Bitmap aResult;

    if (bResult && (bScaleHor || bScaleVer))
    {
        Bitmap aSource(rBitmap);

        bResult = nNewWidth > 0 && nNewHeight > 0
                  && ImplShrinkBeforeConvolution(aSource, nNewWidth, nNewHeight);
        if (bResult)
        {
            const sal_Int32 nSourceWidth = aSource.GetSizePixel().Width();
            const sal_Int32 nSourceHeight = aSource.GetSizePixel().Height();
            // 24-bit is scaled straight to 24-bit. Other formats go through 32-bit and are
            // converted back at the end.
            const vcl::PixelFormat eResultFormat
                = rBitmap.getPixelFormat() == vcl::PixelFormat::N24_BPP ? vcl::PixelFormat::N24_BPP
                                                                        : vcl::PixelFormat::N32_BPP;
            bResult = ImplConvolveRows(
                aSource, aResult,
                nSourceWidth != nNewWidth ? calculateContributions(nSourceWidth, nNewWidth, rKernel)
                                          : Contributions(),
                nSourceHeight != nNewHeight
                    ? calculateContributions(nSourceHeight, nNewHeight, rKernel)
                    : Contributions(),
                eResultFormat);
        }
    }
    else if (bResult)
    {
        // Mirror only, already done in place.
        aResult = rBitmap;
    }

    if(bResult && bMirrorAfter)
    {
        bResult = aResult.Mirror(nMirrorFlags);
    }

    if(bResult)
    {
        rBitmap.AdaptBitCount(aResult);
        rBitmap = std::move(aResult);
    }

    return bResult;
}

} // end anonymous namespace

Bitmap BitmapScaleConvolutionFilter::execute(Bitmap const& rBitmap) const
{
    bool bRetval = false;
    Bitmap aBitmap(rBitmap);

    bRetval = ImplScaleConvolution(aBitmap, mrScaleX, mrScaleY, *mxKernel);

    if (bRetval)
        return aBitmap;

    return Bitmap();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
