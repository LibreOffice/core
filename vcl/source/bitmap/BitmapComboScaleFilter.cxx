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

#include <BitmapComboScaleFilter.hxx>

#include <vcl/BitmapWriteAccess.hxx>
#include <bitmap/BitmapScaleSuperFilter.hxx>

#include <tools/helpers.hxx>
#include <algorithm>
#include <memory>
#include <sal/log.hxx>
#include <tools/ScopedNanoTimer.hxx>
#include <tools/cpuid.hxx>

#include <bitmap/ScanlineHalfScaler.hxx>

namespace vcl
{
namespace
{
constexpr long constScaleThreadStrip = 4;

constexpr bool ALLOW_HORIZONTAL_VERTICAL = false;

typedef void (*ScaleRangeFn)(ScaleContext& rContext, long nStartY, long nEndY);

class ScaleTask : public comphelper::ThreadTask
{
    ScaleRangeFn const mpScaleRangeFunction;
    ScaleContext& mrContext;
    const long mnStartY;
    const long mnEndY;

public:
    explicit ScaleTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag,
                       ScaleRangeFn pScaleRangeFunction, ScaleContext& rContext, long nStartY,
                       long nEndY)
        : comphelper::ThreadTask(pTag)
        , mpScaleRangeFunction(pScaleRangeFunction)
        , mrContext(rContext)
        , mnStartY(nStartY)
        , mnEndY(nEndY)
    {
    }

    virtual void doWork() override { mpScaleRangeFunction(mrContext, mnStartY, mnEndY); }
};

} // anonymous namespace

BitmapComboScaleFilter::BitmapComboScaleFilter(const double& rScaleX, const double& rScaleY)
    : mrScaleX(rScaleX)
    , mrScaleY(rScaleY)
    , mpThreadPoolTag(comphelper::ThreadPool::createThreadTaskTag())
{
}

BitmapComboScaleFilter::~BitmapComboScaleFilter() {}

bool BitmapComboScaleFilter::scale(ScaleType eType, Bitmap& rBitmap)
{
    if (eType == ScaleType::NONE)
        return false;

    bool bResult = false;

    const Size aSizePix(rBitmap.GetSizePixel());

    bool bMirrorHorizontal = mrScaleX < 0;
    bool bMirrorVertical = mrScaleY < 0;

    double nScaleX = 0.0;
    double nScaleY = 0.0;

    switch (eType)
    {
        case ScaleType::OCTAL:
            nScaleX = 0.125;
            nScaleY = 0.125;
            break;
        case ScaleType::QUARTER:
            nScaleX = 0.25;
            nScaleY = 0.25;
            break;
        case ScaleType::QUARTER_HORIZONTAL:
            nScaleX = 0.25;
            nScaleY = 1.0;
            break;
        case ScaleType::QUARTER_VERTICAL:
            nScaleX = 1.0;
            nScaleY = 0.25;
            break;
        case ScaleType::HALF:
            nScaleX = 0.5;
            nScaleY = 0.5;
            break;
        case ScaleType::HALF_HORIZONTAL:
            nScaleX = 0.5;
            nScaleY = 1.0;
            break;
        case ScaleType::HALF_VERTICAL:
            nScaleX = 1.0;
            nScaleY = 0.5;
            break;
        case ScaleType::NONE:
            break;
    }

    const Size aDestinationSize(std::lround(aSizePix.Width() * nScaleX),
                                std::lround(aSizePix.Height() * nScaleY));

    if (aDestinationSize.Width() == aSizePix.Width()
        && aDestinationSize.Height() == aSizePix.Height())
    {
        return true;
    }

    if (aDestinationSize.Width() <= 1L || aDestinationSize.Height() <= 1L)
    {
        return false;
    }

    auto eSourcePixelFormat = rBitmap.getPixelFormat();
    auto ePixelFormat = eSourcePixelFormat;
    if (sal_uInt16(eSourcePixelFormat) < 24)
        ePixelFormat = vcl::PixelFormat::N24_BPP;

    Bitmap aOutBmp(aDestinationSize, ePixelFormat);
    Size aOutSize = aOutBmp.GetSizePixel();

    const long nStartY = 0;
    const long nEndY = aDestinationSize.Height() - 1;

    if (!aOutSize.Width() || !aOutSize.Height())
    {
        SAL_WARN("vcl.gdi", "bmp creation failed");
        return false;
    }

    {
        BitmapScopedReadAccess pReadAccess(rBitmap);
        BitmapScopedWriteAccess pWriteAccess(aOutBmp);

        if (pReadAccess && pWriteAccess)
        {
            ScaleRangeFn pScaleRangeFn;
            ScaleContext aContext(pReadAccess.get(), pWriteAccess.get(), pReadAccess->Width(),
                                  pWriteAccess->Width(), pReadAccess->Height(),
                                  pWriteAccess->Height(), bMirrorVertical, bMirrorHorizontal);

            switch (eType)
            {
                case ScaleType::OCTAL:
                {
                    switch (pReadAccess->GetScanlineFormat())
                    {
                        case ScanlineFormat::N24BitTcBgr:
                        case ScanlineFormat::N24BitTcRgb:
                            pScaleRangeFn = scaleOctal24;
                            break;
                        case ScanlineFormat::N32BitTcRgba:
                        case ScanlineFormat::N32BitTcBgra:
                        case ScanlineFormat::N32BitTcArgb:
                        case ScanlineFormat::N32BitTcAbgr:
                            pScaleRangeFn = scaleOctal32;
                            break;
                        default:
                            pScaleRangeFn = scaleOctalGeneral;
                            break;
                    }

                    break;
                }
                case ScaleType::QUARTER:
                    switch (pReadAccess->GetScanlineFormat())
                    {
                        case ScanlineFormat::N24BitTcBgr:
                        case ScanlineFormat::N24BitTcRgb:
                            pScaleRangeFn = scaleQuarter24;
                            break;
                        case ScanlineFormat::N32BitTcRgba:
                        case ScanlineFormat::N32BitTcBgra:
                        case ScanlineFormat::N32BitTcArgb:
                        case ScanlineFormat::N32BitTcAbgr:
                            pScaleRangeFn = scaleQuarter32_SSE;
                            break;
                        default:
                            pScaleRangeFn = scaleQuarterGeneral;
                            break;
                    }

                    break;
                case ScaleType::QUARTER_HORIZONTAL:
                    pScaleRangeFn = scaleQuarterGeneralHorizontal;
                    break;
                case ScaleType::QUARTER_VERTICAL:
                    pScaleRangeFn = scaleQuarterGeneralVertical;
                    break;
                case ScaleType::HALF:
                    switch (pReadAccess->GetScanlineFormat())
                    {
                        case ScanlineFormat::N24BitTcBgr:
                        case ScanlineFormat::N24BitTcRgb:
                            pScaleRangeFn = scaleHalf24;
                            break;
                        case ScanlineFormat::N32BitTcRgba:
                        case ScanlineFormat::N32BitTcBgra:
                        case ScanlineFormat::N32BitTcArgb:
                        case ScanlineFormat::N32BitTcAbgr:
                            pScaleRangeFn = scaleHalf32_SSE2;
                            break;
                        default:
                            pScaleRangeFn = scaleHalfGeneral;
                            break;
                    }

                    break;
                case ScaleType::HALF_HORIZONTAL:
                    pScaleRangeFn = scaleHalfGeneralHorizontal;
                    break;
                case ScaleType::HALF_VERTICAL:
                    pScaleRangeFn = scaleHalfGeneralVertical;
                    break;
                default:
                    return false;
            }

            bool bUseThreads = true;

            static bool bDisableThreadedScaling = getenv("VCL_NO_THREAD_SCALE");
            if (bDisableThreadedScaling)
            {
                SAL_INFO("vcl.gdi", "Scale in main thread");
                bUseThreads = false;
            }

            if (bUseThreads)
            {
                try
                {
                    comphelper::ThreadPool& rShared
                        = comphelper::ThreadPool::getSharedOptimalPool();
                    // partition and queue work
                    long nStripYStart = nStartY;
                    long nStripYEnd = nStripYStart + constScaleThreadStrip - 1;

                    while (nStripYEnd < nEndY)
                    {
                        std::unique_ptr<ScaleTask> pTask(new ScaleTask(
                            mpThreadPoolTag, pScaleRangeFn, aContext, nStripYStart, nStripYEnd));
                        rShared.pushTask(std::move(pTask));
                        nStripYStart += constScaleThreadStrip;
                        nStripYEnd += constScaleThreadStrip;
                    }
                    if (nStripYStart <= nEndY)
                    {
                        std::unique_ptr<ScaleTask> pTask(new ScaleTask(
                            mpThreadPoolTag, pScaleRangeFn, aContext, nStripYStart, nEndY));
                        rShared.pushTask(std::move(pTask));
                    }
                    rShared.waitUntilDone(mpThreadPoolTag);
                    SAL_INFO("vcl.gdi", "All threaded scaling tasks complete");
                }
                catch (...)
                {
                    SAL_WARN("vcl.gdi", "threaded bitmap scaling failed");
                    bUseThreads = false;
                }
            }

            if (!bUseThreads)
                pScaleRangeFn(aContext, nStartY, nEndY);

            bResult = true;
        }
    }

    if (bResult)
    {
        rBitmap = aOutBmp;
    }
    return bResult;
}

bool BitmapComboScaleFilter::fastPrescale(Bitmap& rBitmap)
{
    const Size aSize(rBitmap.GetSizePixel());

    const long nDestinationWidth = std::lround(aSize.Width() * std::fabs(mrScaleX));
    const long nDestinationHeight = std::lround(aSize.Height() * std::fabs(mrScaleY));

    bool bResult = false;
    ScaleType eType = ScaleType::NONE;
    if (mrScaleX <= 0.125 && mrScaleY <= 0.125)
    {
        eType = ScaleType::OCTAL;
    }
    else if (mrScaleX <= 0.25 && mrScaleY <= 0.25)
    {
        eType = ScaleType::QUARTER;
    }
    else if (mrScaleX <= 0.25 && ALLOW_HORIZONTAL_VERTICAL)
    {
        eType = ScaleType::QUARTER_HORIZONTAL;
    }
    else if (mrScaleY <= 0.25 && ALLOW_HORIZONTAL_VERTICAL)
    {
        eType = ScaleType::QUARTER_VERTICAL;
    }
    else if (mrScaleX <= 0.5 && mrScaleY <= 0.5)
    {
        eType = ScaleType::HALF;
    }
    else if (mrScaleX <= 0.5 && ALLOW_HORIZONTAL_VERTICAL)
    {
        eType = ScaleType::HALF_HORIZONTAL;
    }
    else if (mrScaleY <= 0.5 && ALLOW_HORIZONTAL_VERTICAL)
    {
        eType = ScaleType::HALF_VERTICAL;
    }

    bResult = scale(eType, rBitmap);

    if (bResult)
    {
        const Size aNewSize(rBitmap.GetSizePixel());
        mrScaleX = nDestinationWidth / double(aNewSize.Width());
        mrScaleY = nDestinationHeight / double(aNewSize.Height());
        printf("Combo: %ld %ld -> %ld %ld\n", aSize.Width(), aSize.Height(), aNewSize.Width(),
               aNewSize.Height());
    }

    return bResult;
}

BitmapEx BitmapComboScaleFilter::execute(BitmapEx const& rBitmapEx) const
{
    Bitmap aBitmap(rBitmapEx.GetBitmap());

    while (const_cast<BitmapComboScaleFilter*>(this)->fastPrescale(aBitmap))
        ;

    BitmapEx aBitmapEx(aBitmap);
    BitmapScaleSuperFilter aScaleSuper(mrScaleX, mrScaleY);
    BitmapEx aResult = aScaleSuper.execute(aBitmapEx);

    return aResult;
}

} // end namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
