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

    FilterSharedData(BitmapReadAccess* pReadAccess, BitmapWriteAccess* pWriteAccess, long nRadius)
        : mpReadAccess(pReadAccess)
        , mpWriteAccess(pWriteAccess)
        , mnRadius(nRadius)
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
    template <int n> static void apply(sal_uInt8 (&rResult)[n], Scanline pSource)
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

    template <int n> static void init(sal_uInt8 (&rResult)[n])
    {
        std::fill_n(rResult, n, MorphologyOp::initVal);
    }
};

// 8 bit per channel case

template <typename MorphologyOp, int nComponentWidth> struct pass
{
    static constexpr int nWidthBytes = nComponentWidth / 8;
    static_assert(nWidthBytes * 8 == nComponentWidth);
    static void Horizontal(FilterSharedData const& rShared, const long nStart, const long nEnd)
    {
        BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
        BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

        const long nWidth = pReadAccess->Width();
        const long nLastIndex = nWidth - 1;

        const long nRadius = rShared.mnRadius;

        for (long y = nStart; y <= nEnd; y++)
        {
            const Scanline pScanline = pReadAccess->GetScanline(y);
            for (long x = 0; x < nWidth; x++)
            {
                // This processes [nRadius * 2 + 1] pixels of source per resulting pixel
                // TODO: try to optimize this to not process same pixels repeatedly
                sal_uInt8 aResult[nWidthBytes];
                OpHelper<MorphologyOp>::init(aResult);
                const long iMax = std::min(x + nRadius, nLastIndex);
                for (long i = std::max(x - nRadius, 0L); i <= iMax; ++i)
                    OpHelper<MorphologyOp>::apply(aResult, pScanline + nWidthBytes * i);

                Scanline pDestinationPointer = pWriteAccess->GetScanline(y) + nWidthBytes * x;
                for (const auto& val : aResult)
                    *pDestinationPointer++ = val;
            }
        }
    }

    static void Vertical(FilterSharedData const& rShared, const long nStart, const long nEnd)
    {
        BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
        BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

        const long nHeight = pReadAccess->Height();
        const long nLastIndex = nHeight - 1;

        const long nRadius = rShared.mnRadius;

        for (long x = nStart; x <= nEnd; x++)
        {
            for (long y = 0; y < nHeight; y++)
            {
                // This processes [nRadius * 2 + 1] pixels of source per resulting pixel
                // TODO: try to optimize this to not process same pixels repeatedly
                sal_uInt8 aResult[nWidthBytes];
                OpHelper<MorphologyOp>::init(aResult);
                const long iMax = std::min(y + nRadius, nLastIndex);
                for (long i = std::max(y - nRadius, 0L); i <= iMax; ++i)
                    OpHelper<MorphologyOp>::apply(aResult,
                                                  pReadAccess->GetScanline(i) + nWidthBytes * x);

                Scanline pDestinationPointer = pWriteAccess->GetScanline(y) + nWidthBytes * x;
                for (auto& val : aResult)
                    *pDestinationPointer++ = val;
            }
        }
    }
};

// Partial specializations for nComponentWidth == 0, using acess' GetColor/SetPixel

template <typename MorphologyOp> struct pass<MorphologyOp, 0>
{
    static void Horizontal(FilterSharedData const& rShared, const long nStart, const long nEnd)
    {
        BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
        BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

        const long nWidth = pReadAccess->Width();
        const long nLastIndex = nWidth - 1;

        const long nRadius = rShared.mnRadius;

        for (long y = nStart; y <= nEnd; y++)
        {
            for (long x = 0; x < nWidth; x++)
            {
                // This processes [nRadius * 2 + 1] pixels of source per resulting pixel
                // TODO: try to optimize this to not process same pixels repeatedly
                Color aResult = MorphologyOp::initColor;
                const long iMax = std::min(x + nRadius, nLastIndex);
                for (long i = std::max(x - nRadius, 0L); i <= iMax; ++i)
                    OpHelper<MorphologyOp>::apply(aResult, pReadAccess->GetColor(y, i));

                pWriteAccess->SetPixel(y, x, aResult);
            }
        }
    }

    static void Vertical(FilterSharedData const& rShared, const long nStart, const long nEnd)
    {
        BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
        BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

        const long nHeight = pReadAccess->Height();
        const long nLastIndex = nHeight - 1;

        const long nRadius = rShared.mnRadius;

        for (long x = nStart; x <= nEnd; x++)
        {
            for (long y = 0; y < nHeight; y++)
            {
                // This processes [nRadius * 2 + 1] pixels of source per resulting pixel
                // TODO: try to optimize this to not process same pixels repeatedly
                Color aResult = MorphologyOp::initColor;
                const long iMax = std::min(y + nRadius, nLastIndex);
                for (long i = std::max(y - nRadius, 0L); i <= iMax; ++i)
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
void runFilter(Bitmap& rBitmap, const long nRadius, const bool bParallel)
{
    using myPass = pass<MorphologyOp, nComponentWidth>;
    if (bParallel)
    {
        try
        {
            comphelper::ThreadPool& rShared = comphelper::ThreadPool::getSharedOptimalPool();
            auto pTag = comphelper::ThreadPool::createThreadTaskTag();

            {
                Bitmap::ScopedReadAccess pReadAccess(rBitmap);
                BitmapScopedWriteAccess pWriteAccess(rBitmap);
                FilterSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius);

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
                FilterSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius);

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
            FilterSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius);
            long nFirstIndex = 0;
            long nLastIndex = pReadAccess->Height() - 1;
            myPass::Horizontal(aSharedData, nFirstIndex, nLastIndex);
        }
        {
            Bitmap::ScopedReadAccess pReadAccess(rBitmap);
            BitmapScopedWriteAccess pWriteAccess(rBitmap);
            FilterSharedData aSharedData(pReadAccess.get(), pWriteAccess.get(), nRadius);
            long nFirstIndex = 0;
            long nLastIndex = pReadAccess->Width() - 1;
            myPass::Vertical(aSharedData, nFirstIndex, nLastIndex);
        }
    }
}

template <int nComponentWidth>
void runFilter(Bitmap& rBitmap, BasicMorphologyOp op, sal_Int32 nRadius)
{
    const bool bParallel = true;

    if (op == BasicMorphologyOp::erode)
        runFilter<ErodeOp, nComponentWidth>(rBitmap, nRadius, bParallel);
    else if (op == BasicMorphologyOp::dilate)
        runFilter<DilateOp, nComponentWidth>(rBitmap, nRadius, bParallel);
}

} // end anonymous namespace

BitmapBasicMorphologyFilter::BitmapBasicMorphologyFilter(BasicMorphologyOp op, sal_Int32 nRadius)
    : m_eOp(op)
    , m_nRadius(nRadius)
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
            runFilter<24>(bitmapCopy, m_eOp, m_nRadius);
            break;
        case ScanlineFormat::N32BitTcMask:
        case ScanlineFormat::N32BitTcBgra:
            runFilter<32>(bitmapCopy, m_eOp, m_nRadius);
            break;
        case ScanlineFormat::N8BitPal:
            runFilter<8>(bitmapCopy, m_eOp, m_nRadius);
            break;
        // TODO: handle 1-bit images
        default:
            // Use access' GetColor/SetPixel fallback
            runFilter<0>(bitmapCopy, m_eOp, m_nRadius);
            break;
    }

    return bitmapCopy;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
