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
#include <vcl/BitmapBasicMorphologyFilter.hxx>

#include <bitmap/BitmapWriteAccess.hxx>

#include <algorithm>

/* TODO: Use round kernel instead of square one.
   This would make the result more natural, e.g. not making rounded square out of circle.
 */

namespace
{
struct FilterSharedData
{
    BitmapReadAccess* mpReadAccess;
    BitmapWriteAccess* mpWriteAccess;
    tools::Long mnRadius;
    sal_uInt8 mnOutsideVal;
    Color maOutsideColor;

    FilterSharedData(Bitmap::ScopedReadAccess& rReadAccess, BitmapScopedWriteAccess& rWriteAccess,
                     tools::Long nRadius, sal_uInt8 nOutsideVal)
        : mpReadAccess(rReadAccess.get())
        , mpWriteAccess(rWriteAccess.get())
        , mnRadius(nRadius)
        , mnOutsideVal(nOutsideVal)
        , maOutsideColor(ColorTransparency, nOutsideVal, nOutsideVal, nOutsideVal, nOutsideVal)
    {
    }
};

// Black is foreground, white is background

struct ErodeOp
{
    static sal_uInt8 apply(sal_uInt8 v1, sal_uInt8 v2) { return std::max(v1, v2); }
    static constexpr sal_uInt8 initVal = 0;
};

struct DilateOp
{
    static sal_uInt8 apply(sal_uInt8 v1, sal_uInt8 v2) { return std::min(v1, v2); }
    static constexpr sal_uInt8 initVal = SAL_MAX_UINT8;
};

// 8 bit per channel case

template <typename MorphologyOp, int nComponentWidth> struct Value
{
    static constexpr int nWidthBytes = nComponentWidth / 8;
    static_assert(nWidthBytes * 8 == nComponentWidth);

    sal_uInt8 aResult[nWidthBytes];

    // If we are at the start or at the end of the line, consider outside value
    Value(FilterSharedData const& rShared, bool bLookOutside)
    {
        std::fill_n(aResult, nWidthBytes,
                    bLookOutside ? rShared.mnOutsideVal : MorphologyOp::initVal);
    }

    void apply(const BitmapReadAccess* pReadAccess, tools::Long x, tools::Long y,
               sal_uInt8* pHint = nullptr)
    {
        sal_uInt8* pSource = (pHint ? pHint : pReadAccess->GetScanline(y)) + nWidthBytes * x;
        std::transform(pSource, pSource + nWidthBytes, aResult, aResult, MorphologyOp::apply);
    }

    void copy(const BitmapWriteAccess* pWriteAccess, tools::Long x, tools::Long y,
              sal_uInt8* pHint = nullptr)
    {
        sal_uInt8* pDest = (pHint ? pHint : pWriteAccess->GetScanline(y)) + nWidthBytes * x;
        std::copy_n(aResult, nWidthBytes, pDest);
    }
};

// Partial specializations for nComponentWidth == 0, using access' GetColor/SetPixel

template <typename MorphologyOp> struct Value<MorphologyOp, 0>
{
    static constexpr Color initColor{ ColorTransparency, MorphologyOp::initVal,
                                      MorphologyOp::initVal, MorphologyOp::initVal,
                                      MorphologyOp::initVal };

    Color aResult;

    // If we are at the start or at the end of the line, consider outside value
    Value(FilterSharedData const& rShared, bool bLookOutside)
        : aResult(bLookOutside ? rShared.maOutsideColor : initColor)
    {
    }

    void apply(const BitmapReadAccess* pReadAccess, tools::Long x, tools::Long y,
               sal_uInt8* /*pHint*/ = nullptr)
    {
        const auto& rSource = pReadAccess->GetColor(y, x);
        aResult = Color(ColorAlpha, MorphologyOp::apply(rSource.GetAlpha(), aResult.GetAlpha()),
                        MorphologyOp::apply(rSource.GetRed(), aResult.GetRed()),
                        MorphologyOp::apply(rSource.GetGreen(), aResult.GetGreen()),
                        MorphologyOp::apply(rSource.GetBlue(), aResult.GetBlue()));
    }

    void copy(BitmapWriteAccess* pWriteAccess, tools::Long x, tools::Long y,
              sal_uInt8* /*pHint*/ = nullptr)
    {
        pWriteAccess->SetPixel(y, x, aResult);
    }
};

bool GetMinMax(tools::Long nCenter, tools::Long nRadius, tools::Long nMaxLimit, tools::Long& nMin,
               tools::Long& nMax)
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

template <typename MorphologyOp, int nComponentWidth> struct pass
{
    static void Horizontal(FilterSharedData const& rShared, const tools::Long nStart,
                           const tools::Long nEnd)
    {
        BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
        BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

        const tools::Long nLastIndex = pReadAccess->Width() - 1;

        for (tools::Long y = nStart; y <= nEnd; y++)
        {
            // Optimization
            sal_uInt8* const pSourceHint = pReadAccess->GetScanline(y);
            sal_uInt8* const pDestHint = pWriteAccess->GetScanline(y);
            for (tools::Long x = 0; x <= nLastIndex; x++)
            {
                // This processes [nRadius * 2 + 1] pixels of source per resulting pixel
                // TODO: try to optimize this to not process same pixels repeatedly
                tools::Long iMin, iMax;
                const bool bLookOutside = GetMinMax(x, rShared.mnRadius, nLastIndex, iMin, iMax);
                Value<MorphologyOp, nComponentWidth> aResult(rShared, bLookOutside);
                for (tools::Long i = iMin; i <= iMax; ++i)
                    aResult.apply(pReadAccess, i, y, pSourceHint);

                aResult.copy(pWriteAccess, x, y, pDestHint);
            }
        }
    }

    static void Vertical(FilterSharedData const& rShared, const tools::Long nStart,
                         const tools::Long nEnd)
    {
        BitmapReadAccess* pReadAccess = rShared.mpReadAccess;
        BitmapWriteAccess* pWriteAccess = rShared.mpWriteAccess;

        const tools::Long nLastIndex = pReadAccess->Height() - 1;

        for (tools::Long x = nStart; x <= nEnd; x++)
        {
            for (tools::Long y = 0; y <= nLastIndex; y++)
            {
                // This processes [nRadius * 2 + 1] pixels of source per resulting pixel
                // TODO: try to optimize this to not process same pixels repeatedly
                tools::Long iMin, iMax;
                const bool bLookOutside = GetMinMax(y, rShared.mnRadius, nLastIndex, iMin, iMax);
                Value<MorphologyOp, nComponentWidth> aResult(rShared, bLookOutside);
                for (tools::Long i = iMin; i <= iMax; ++i)
                    aResult.apply(pReadAccess, x, i);

                aResult.copy(pWriteAccess, x, y);
            }
        }
    }
};

typedef void (*passFn)(FilterSharedData const& rShared, tools::Long nStart, tools::Long nEnd);

class FilterTask : public comphelper::ThreadTask
{
    passFn mpFunction;
    FilterSharedData& mrShared;
    tools::Long mnStart;
    tools::Long mnEnd;

public:
    explicit FilterTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag, passFn pFunction,
                        FilterSharedData& rShared, tools::Long nStart, tools::Long nEnd)
        : comphelper::ThreadTask(pTag)
        , mpFunction(pFunction)
        , mrShared(rShared)
        , mnStart(nStart)
        , mnEnd(nEnd)
    {
    }

    virtual void doWork() override { mpFunction(mrShared, mnStart, mnEnd); }
};

constexpr tools::Long nThreadStrip = 16;

template <typename MorphologyOp, int nComponentWidth>
void runFilter(Bitmap& rBitmap, const tools::Long nRadius, const bool bParallel,
               bool bUseValueOutside, sal_uInt8 nValueOutside)
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
                FilterSharedData aSharedData(pReadAccess, pWriteAccess, nRadius, nOutsideVal);

                const tools::Long nLastIndex = pReadAccess->Height() - 1;
                tools::Long nStripStart = 0;
                for (; nStripStart < nLastIndex - nThreadStrip; nStripStart += nThreadStrip)
                {
                    tools::Long nStripEnd = nStripStart + nThreadStrip - 1;
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
                FilterSharedData aSharedData(pReadAccess, pWriteAccess, nRadius, nOutsideVal);

                const tools::Long nLastIndex = pReadAccess->Width() - 1;
                tools::Long nStripStart = 0;
                for (; nStripStart < nLastIndex - nThreadStrip; nStripStart += nThreadStrip)
                {
                    tools::Long nStripEnd = nStripStart + nThreadStrip - 1;
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
            FilterSharedData aSharedData(pReadAccess, pWriteAccess, nRadius, nOutsideVal);
            tools::Long nFirstIndex = 0;
            tools::Long nLastIndex = pReadAccess->Height() - 1;
            myPass::Horizontal(aSharedData, nFirstIndex, nLastIndex);
        }
        {
            Bitmap::ScopedReadAccess pReadAccess(rBitmap);
            BitmapScopedWriteAccess pWriteAccess(rBitmap);
            FilterSharedData aSharedData(pReadAccess, pWriteAccess, nRadius, nOutsideVal);
            tools::Long nFirstIndex = 0;
            tools::Long nLastIndex = pReadAccess->Width() - 1;
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
    Bitmap result = filter(rBitmapEx.GetBitmap());
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
