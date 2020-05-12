/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <comphelper/threadpool.hxx>
#include <sal/log.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/BitmapBasicMorphologyFilter.hxx>

#include <bitmapwriteaccess.hxx>

#include <algorithm>

namespace
{
struct FilterSharedData
{
    BitmapReadAccess* mpReadAccess;
    BitmapWriteAccess* mpWriteAccess;
    long mnRadius;
    sal_uInt8 mnOutsideVal;
    Color maOutsideColor;

    FilterSharedData(BitmapReadAccess* pReadAccess, BitmapWriteAccess* pWriteAccess, long nRadius,
                     sal_uInt8 nOutsideVal)
        : mpReadAccess(pReadAccess)
        , mpWriteAccess(pWriteAccess)
        , mnRadius(nRadius)
        , mnOutsideVal(nOutsideVal)
        , maOutsideColor(nOutsideVal, nOutsideVal, nOutsideVal, nOutsideVal)
    {
    }
};

// Black is foreground, white is background

struct ErodeOp
{
    static sal_uInt8 apply(sal_uInt8 v1, sal_uInt8 v2) { return std::max(v1, v2); }
    static constexpr sal_uInt8 initVal = 0;
    static constexpr Color initColor = COL_BLACK;
};

struct DilateOp
{
    static sal_uInt8 apply(sal_uInt8 v1, sal_uInt8 v2) { return std::min(v1, v2); }
    static constexpr sal_uInt8 initVal{ SAL_MAX_UINT8 };
    static constexpr Color initColor = COL_TRANSPARENT;
};

template <typename MorphologyOp> struct OpHelper
{
    template <int n> static void apply(sal_uInt8 (&rResult)[n], sal_uInt8* pSource)
    {
        std::transform(pSource, pSource + n, rResult, rResult, MorphologyOp::apply);
    }

    static void apply(Color& rResult, const Color& rSource)
    {
        rResult = Color(MorphologyOp::apply(rSource.GetTransparency(), rResult.GetTransparency()),
                        MorphologyOp::apply(rSource.GetRed(), rResult.GetRed()),
                        MorphologyOp::apply(rSource.GetGreen(), rResult.GetGreen()),
                        MorphologyOp::apply(rSource.GetBlue(), rResult.GetBlue()));
    }

    template <int n> static void init(sal_uInt8 (&rResult)[n], sal_uInt8 val)
    {
        std::fill_n(rResult, n, val);
    }

    template <int n> static void copy(sal_uInt8 (&rResult)[n], sal_uInt8* pDest)
    {
        std::copy_n(rResult, n, pDest);
    }
};

bool GetMinMax(long nCenter, long nRadius, long nMaxLimit, long& nMin, long& nMax)
{
    nMin = nCenter - nRadius;
    nMax = nCenter + nRadius;
    bool bLookOutside = false;
    if (nMin < 0)
    {
        bLookOutside = true;
        nMin = 0;
    }
    if (nMax > nMaxLimit)
    {
        bLookOutside = true;
        nMax = nMaxLimit;
    }
    return bLookOutside;
}

// 8 bit per channel case

template <typename MorphologyOp, int nComponentWidth> struct pass
{
    static constexpr int nWidthBytes = nComponentWidth / 8;
    static_assert(nWidthBytes * 8 == nComponentWidth);
    static void Horizontal(FilterSharedData const& rShared, const long nStart, const long nEnd)
    {
        BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
        BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

        const long nLastIndex = pReadAccess->Width() - 1;

        for (long y = nStart; y <= nEnd; y++)
        {
            const Scanline pScanline = pReadAccess->GetScanline(y);
            for (long x = 0; x <= nLastIndex; x++)
            {
                // This processes [nRadius * 2 + 1] pixels of source per resulting pixel
                // TODO: try to optimize this to not process same pixels repeatedly
                long iMin, iMax;
                const bool bLookOutside = GetMinMax(x, rShared.mnRadius, nLastIndex, iMin, iMax);

                sal_uInt8 aResult[nWidthBytes];
                // If we are at the start or at the end of the line, consider outside value
                OpHelper<MorphologyOp>::init(aResult, bLookOutside ? rShared.mnOutsideVal
                                                                   : MorphologyOp::initVal);
                for (long i = iMin; i <= iMax; ++i)
                    OpHelper<MorphologyOp>::apply(aResult, pScanline + nWidthBytes * i);

                Scanline pDestinationPointer = pWriteAccess->GetScanline(y) + nWidthBytes * x;
                OpHelper<MorphologyOp>::copy(aResult, pDestinationPointer);
            }
        }
    }

    static void Vertical(FilterSharedData const& rShared, const long nStart, const long nEnd)
    {
        BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
        BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

        const long nLastIndex = pReadAccess->Height() - 1;

        for (long x = nStart; x <= nEnd; x++)
        {
            for (long y = 0; y <= nLastIndex; y++)
            {
                // This processes [nRadius * 2 + 1] pixels of source per resulting pixel
                // TODO: try to optimize this to not process same pixels repeatedly
                long iMin, iMax;
                const bool bLookOutside = GetMinMax(y, rShared.mnRadius, nLastIndex, iMin, iMax);

                sal_uInt8 aResult[nWidthBytes];
                // If we are at the start or at the end of the line, consider outside value
                OpHelper<MorphologyOp>::init(aResult, bLookOutside ? rShared.mnOutsideVal
                                                                   : MorphologyOp::initVal);
                for (long i = iMin; i <= iMax; ++i)
                    OpHelper<MorphologyOp>::apply(aResult,
                                                  pReadAccess->GetScanline(i) + nWidthBytes * x);

                Scanline pDestinationPointer = pWriteAccess->GetScanline(y) + nWidthBytes * x;
                OpHelper<MorphologyOp>::copy(aResult, pDestinationPointer);
            }
        }
    }
};

// Partial specializations for nComponentWidth == 0, using access' GetColor/SetPixel

template <typename MorphologyOp> struct pass<MorphologyOp, 0>
{
    static void Horizontal(FilterSharedData const& rShared, const long nStart, const long nEnd)
    {
        BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
        BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

        const long nLastIndex = pReadAccess->Width() - 1;

        for (long y = nStart; y <= nEnd; y++)
        {
            for (long x = 0; x <= nLastIndex; x++)
            {
                // This processes [nRadius * 2 + 1] pixels of source per resulting pixel
                // TODO: try to optimize this to not process same pixels repeatedly
                long iMin, iMax;
                const bool bLookOutside = GetMinMax(x, rShared.mnRadius, nLastIndex, iMin, iMax);

                // If we are at the start or at the end of the line, consider outside value
                Color aResult = bLookOutside ? rShared.maOutsideColor : MorphologyOp::initColor;
                for (long i = iMin; i <= iMax; ++i)
                    OpHelper<MorphologyOp>::apply(aResult, pReadAccess->GetColor(y, i));

                pWriteAccess->SetPixel(y, x, aResult);
            }
        }
    }

    static void Vertical(FilterSharedData const& rShared, const long nStart, const long nEnd)
    {
        BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
        BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

        const long nLastIndex = pReadAccess->Height() - 1;

        for (long x = nStart; x <= nEnd; x++)
        {
            for (long y = 0; y <= nLastIndex; y++)
            {
                // This processes [nRadius * 2 + 1] pixels of source per resulting pixel
                // TODO: try to optimize this to not process same pixels repeatedly
                long iMin, iMax;
                const bool bLookOutside = GetMinMax(y, rShared.mnRadius, nLastIndex, iMin, iMax);

                // If we are at the start or at the end of the line, consider outside value
                Color aResult = bLookOutside ? rShared.maOutsideColor : MorphologyOp::initColor;
                for (long i = iMin; i <= iMax; ++i)
                    OpHelper<MorphologyOp>::apply(aResult, pReadAccess->GetColor(i, x));

                pWriteAccess->SetPixel(y, x, aResult);
            }
        }
    }
};

typedef void (*passFn)(FilterSharedData const& rShared, long nStart, long nEnd);

class FilterTask : public comphelper::ThreadTask
{
    passFn mpFunction;
    FilterSharedData& mrShared;
    long mnStart;
    long mnEnd;

public:
    explicit FilterTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag, passFn pFunction,
                        FilterSharedData& rShared, long nStart, long nEnd)
        : comphelper::ThreadTask(pTag)
        , mpFunction(pFunction)
        , mrShared(rShared)
        , mnStart(nStart)
        , mnEnd(nEnd)
    {
    }

    virtual void doWork() override { mpFunction(mrShared, mnStart, mnEnd); }
};

constexpr long nThreadStrip = 16;

template <typename MorphologyOp, int nComponentWidth>
void runFilter(Bitmap& rBitmap, const long nRadius, const bool bParallel, bool bUseValueOutside,
               sal_uInt8 nValueOutside)
{
    using myPass = pass<MorphologyOp, nComponentWidth>;
    const sal_uInt8 nOutsideVal = bUseValueOutside ? nValueOutside : MorphologyOp::initVal;
    if (bParallel)
    {
        try
        {
            comphelper::ThreadPool& rShared = comphelper::ThreadPool::getSharedOptimalPool();
            auto pTag = comphelper::ThreadPool::createThreadTaskTag();

            {
                Bitmap::ScopedReadAccess pReadAccess(rBitmap);
                BitmapScopedWriteAccess pWriteAccess(rBitmap);
                FilterSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                             nOutsideVal);

                const long nLastIndex = pReadAccess->Height() - 1;
                long nStripStart = 0;
                for (; nStripStart < nLastIndex - nThreadStrip; nStripStart += nThreadStrip)
                {
                    long nStripEnd = nStripStart + nThreadStrip - 1;
                    auto pTask(std::make_unique<FilterTask>(pTag, myPass::Horizontal, aSharedData,
                                                            nStripStart, nStripEnd));
                    rShared.pushTask(std::move(pTask));
                }
                // Do the last (or the only) strip in main thread without threading overhead
                myPass::Horizontal(aSharedData, nStripStart, nLastIndex);
                rShared.waitUntilDone(pTag);
            }
            {
                Bitmap::ScopedReadAccess pReadAccess(rBitmap);
                BitmapScopedWriteAccess pWriteAccess(rBitmap);
                FilterSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                             nOutsideVal);

                const long nLastIndex = pReadAccess->Width() - 1;
                long nStripStart = 0;
                for (; nStripStart < nLastIndex - nThreadStrip; nStripStart += nThreadStrip)
                {
                    long nStripEnd = nStripStart + nThreadStrip - 1;
                    auto pTask(std::make_unique<FilterTask>(pTag, myPass::Vertical, aSharedData,
                                                            nStripStart, nStripEnd));
                    rShared.pushTask(std::move(pTask));
                }
                // Do the last (or the only) strip in main thread without threading overhead
                myPass::Vertical(aSharedData, nStripStart, nLastIndex);
                rShared.waitUntilDone(pTag);
            }
        }
        catch (...)
        {
            SAL_WARN("vcl.gdi", "threaded bitmap blurring failed");
        }
    }
    else
    {
        {
            Bitmap::ScopedReadAccess pReadAccess(rBitmap);
            BitmapScopedWriteAccess pWriteAccess(rBitmap);
            FilterSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                         nOutsideVal);
            long nFirstIndex = 0;
            long nLastIndex = pReadAccess->Height() - 1;
            myPass::Horizontal(aSharedData, nFirstIndex, nLastIndex);
        }
        {
            Bitmap::ScopedReadAccess pReadAccess(rBitmap);
            BitmapScopedWriteAccess pWriteAccess(rBitmap);
            FilterSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius,
                                         nOutsideVal);
            long nFirstIndex = 0;
            long nLastIndex = pReadAccess->Width() - 1;
            myPass::Vertical(aSharedData, nFirstIndex, nLastIndex);
        }
    }
}

template <int nComponentWidth>
void runFilter(Bitmap& rBitmap, BasicMorphologyOp op, sal_Int32 nRadius, bool bUseValueOutside,
               sal_uInt8 nValueOutside)
{
    const bool bParallel = true;

    if (op == BasicMorphologyOp::erode)
        runFilter<ErodeOp, nComponentWidth>(rBitmap, nRadius, bParallel, bUseValueOutside,
                                            nValueOutside);
    else if (op == BasicMorphologyOp::dilate)
        runFilter<DilateOp, nComponentWidth>(rBitmap, nRadius, bParallel, bUseValueOutside,
                                             nValueOutside);
}

} // end anonymous namespace

BitmapBasicMorphologyFilter::BitmapBasicMorphologyFilter(BasicMorphologyOp op, sal_Int32 nRadius)
    : m_eOp(op)
    , m_nRadius(nRadius)
{
}

BitmapBasicMorphologyFilter::BitmapBasicMorphologyFilter(BasicMorphologyOp op, sal_Int32 nRadius,
                                                         sal_uInt8 nValueOutside)
    : m_eOp(op)
    , m_nRadius(nRadius)
    , m_nValueOutside(nValueOutside)
    , m_bUseValueOutside(true)
{
}

BitmapBasicMorphologyFilter::~BitmapBasicMorphologyFilter() = default;

BitmapEx BitmapBasicMorphologyFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap = rBitmapEx.GetBitmap();
    Bitmap result = filter(aBitmap);
    return BitmapEx(result, rBitmapEx.GetMask());
}

Bitmap BitmapBasicMorphologyFilter::filter(Bitmap const& rBitmap) const
{
    Bitmap bitmapCopy(rBitmap);
    ScanlineFormat nScanlineFormat;
    {
        Bitmap::ScopedReadAccess pReadAccess(bitmapCopy);
        nScanlineFormat = pReadAccess->GetScanlineFormat();
    }

    switch (nScanlineFormat)
    {
        case ScanlineFormat::N24BitTcRgb:
        case ScanlineFormat::N24BitTcBgr:
            runFilter<24>(bitmapCopy, m_eOp, m_nRadius, m_bUseValueOutside, m_nValueOutside);
            break;
        case ScanlineFormat::N32BitTcMask:
        case ScanlineFormat::N32BitTcBgra:
            runFilter<32>(bitmapCopy, m_eOp, m_nRadius, m_bUseValueOutside, m_nValueOutside);
            break;
        case ScanlineFormat::N8BitPal:
            runFilter<8>(bitmapCopy, m_eOp, m_nRadius, m_bUseValueOutside, m_nValueOutside);
            break;
        // TODO: handle 1-bit images
        default:
            // Use access' GetColor/SetPixel fallback
            runFilter<0>(bitmapCopy, m_eOp, m_nRadius, m_bUseValueOutside, m_nValueOutside);
            break;
    }

    return bitmapCopy;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
