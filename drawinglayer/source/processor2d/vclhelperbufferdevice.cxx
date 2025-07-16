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

#include <sal/config.h>
#include <sal/log.hxx>

#include <algorithm>
#include <map>
#include <vector>

#include "vclhelperbufferdevice.hxx"
#include <basegfx/range/b2drange.hxx>
#include <vcl/bitmapex.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/timer.hxx>
#include <tools/lazydelete.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/skia/SkiaHelper.hxx>
#include <mutex>

#ifdef DBG_UTIL
#include <o3tl/environment.hxx>
#include <tools/stream.hxx>
#endif

// #define SPEED_COMPARE
#ifdef SPEED_COMPARE
#include <tools/time.hxx>
#endif

// buffered VDev usage
namespace
{
class VDevBuffer : public Timer
{
private:
    struct Entry
    {
        VclPtr<VirtualDevice> buf;
        Entry(const VclPtr<VirtualDevice>& vdev)
            : buf(vdev)
        {
        }
    };

    std::mutex m_aMutex;

    // available buffers
    std::vector<Entry> maFreeBuffers;

    // allocated/used buffers (remembered to allow deleting them in destructor)
    std::vector<Entry> maUsedBuffers;

    // remember what outputdevice was the template passed to VirtualDevice::Create
    // so we can test if that OutputDevice was disposed before reusing a
    // virtualdevice because that isn't safe to do at least for Gtk2
    std::map<VclPtr<VirtualDevice>, VclPtr<OutputDevice>> maDeviceTemplates;

    static bool isSizeSuitable(const VclPtr<VirtualDevice>& device, const Size& size);

public:
    VDevBuffer();
    virtual ~VDevBuffer() override;

    VclPtr<VirtualDevice> alloc(OutputDevice& rOutDev, const Size& rSizePixel);
    void free(VirtualDevice& rDevice);

    // Timer virtuals
    virtual void Invoke() override;
};

VDevBuffer::VDevBuffer()
    : Timer("drawinglayer::VDevBuffer via Invoke()")
{
    SetTimeout(10L * 1000L); // ten seconds
}

VDevBuffer::~VDevBuffer()
{
    std::unique_lock aGuard(m_aMutex);
    Stop();

    while (!maFreeBuffers.empty())
    {
        maFreeBuffers.back().buf.disposeAndClear();
        maFreeBuffers.pop_back();
    }

    while (!maUsedBuffers.empty())
    {
        maUsedBuffers.back().buf.disposeAndClear();
        maUsedBuffers.pop_back();
    }
}

bool VDevBuffer::isSizeSuitable(const VclPtr<VirtualDevice>& device, const Size& rSizePixel)
{
    if (device->GetOutputWidthPixel() >= rSizePixel.getWidth()
        && device->GetOutputHeightPixel() >= rSizePixel.getHeight())
    {
        bool requireSmall = false;
#if defined(UNX)
        // HACK: See the small size handling in SvpSalVirtualDevice::CreateSurface().
        // Make sure to not reuse a larger device when a small one should be preferred.
        if (device->GetRenderBackendName() == "svp")
            requireSmall = true;
#endif
        // The same for Skia, see renderMethodToUseForSize().
        if (SkiaHelper::isVCLSkiaEnabled())
            requireSmall = true;
        if (requireSmall)
        {
            if (rSizePixel.getWidth() <= 32 && rSizePixel.getHeight() <= 32
                && (device->GetOutputWidthPixel() > 32 || device->GetOutputHeightPixel() > 32))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

VclPtr<VirtualDevice> VDevBuffer::alloc(OutputDevice& rOutDev, const Size& rSizePixel)
{
    std::unique_lock aGuard(m_aMutex);
    VclPtr<VirtualDevice> pRetval;

    sal_Int32 nBits = rOutDev.GetBitCount();

    bool bOkay(false);
    if (!maFreeBuffers.empty())
    {
        auto aFound(maFreeBuffers.end());

        for (auto a = maFreeBuffers.begin(); a != maFreeBuffers.end(); ++a)
        {
            assert(a->buf && "Empty pointer in VDevBuffer (!)");

            if (nBits == a->buf->GetBitCount())
            {
                // candidate is valid due to bit depth
                if (aFound != maFreeBuffers.end())
                {
                    // already found
                    if (bOkay)
                    {
                        // found is valid
                        const bool bCandidateOkay = isSizeSuitable(a->buf, rSizePixel);

                        if (bCandidateOkay)
                        {
                            // found and candidate are valid
                            const sal_uLong aSquare(aFound->buf->GetOutputWidthPixel()
                                                    * aFound->buf->GetOutputHeightPixel());
                            const sal_uLong aCandidateSquare(a->buf->GetOutputWidthPixel()
                                                             * a->buf->GetOutputHeightPixel());

                            if (aCandidateSquare < aSquare)
                            {
                                // candidate is valid and smaller, use it
                                aFound = a;
                            }
                        }
                        else
                        {
                            // found is valid, candidate is not. Keep found
                        }
                    }
                    else
                    {
                        // found is invalid, use candidate
                        aFound = a;
                        bOkay = isSizeSuitable(aFound->buf, rSizePixel);
                    }
                }
                else
                {
                    // none yet, use candidate
                    aFound = a;
                    bOkay = isSizeSuitable(aFound->buf, rSizePixel);
                }
            }
        }

        if (aFound != maFreeBuffers.end())
        {
            pRetval = aFound->buf;
            maFreeBuffers.erase(aFound);
        }
    }

    if (pRetval)
    {
        // found a suitable cached virtual device, but the
        // outputdevice it was based on has been disposed,
        // drop it and create a new one instead as reusing
        // such devices is unsafe under at least Gtk2
        if (maDeviceTemplates[pRetval]->isDisposed())
        {
            maDeviceTemplates.erase(pRetval);
            pRetval.disposeAndClear();
        }
        else
        {
            if (bOkay)
            {
                pRetval->Erase(pRetval->PixelToLogic(
                    tools::Rectangle(0, 0, rSizePixel.getWidth(), rSizePixel.getHeight())));
            }
            else
            {
                pRetval->SetOutputSizePixel(rSizePixel, true);
            }
        }
    }

    // no success yet, create new buffer
    if (!pRetval)
    {
        pRetval = VclPtr<VirtualDevice>::Create(rOutDev, DeviceFormat::WITHOUT_ALPHA);
        maDeviceTemplates[pRetval] = &rOutDev;
        pRetval->SetOutputSizePixel(rSizePixel, true);
    }
    else
    {
        // reused, reset some values
        pRetval->SetMapMode();
        pRetval->SetRasterOp(RasterOp::OverPaint);
    }

    // remember allocated buffer
    maUsedBuffers.emplace_back(pRetval);

    return pRetval;
}

void VDevBuffer::free(VirtualDevice& rDevice)
{
    std::unique_lock aGuard(m_aMutex);
    const auto aUsedFound
        = std::find_if(maUsedBuffers.begin(), maUsedBuffers.end(),
                       [&rDevice](const Entry& el) { return el.buf == &rDevice; });
    SAL_WARN_IF(aUsedFound == maUsedBuffers.end(), "drawinglayer",
                "OOps, non-registered buffer freed (!)");
    if (aUsedFound != maUsedBuffers.end())
    {
        maFreeBuffers.emplace_back(*aUsedFound);
        maUsedBuffers.erase(aUsedFound);
        SAL_WARN_IF(maFreeBuffers.size() > 1000, "drawinglayer",
                    "excessive cached buffers, " << maFreeBuffers.size() << " entries!");
    }
    Start();
}

void VDevBuffer::Invoke()
{
    std::unique_lock aGuard(m_aMutex);

    while (!maFreeBuffers.empty())
    {
        auto aLastOne = maFreeBuffers.back();
        maDeviceTemplates.erase(aLastOne.buf);
        aLastOne.buf.disposeAndClear();
        maFreeBuffers.pop_back();
    }
}

#ifdef SPEED_COMPARE
void doSpeedCompare(double fTrans, const Bitmap& rContent, const tools::Rectangle& rDestPixel,
                    OutputDevice& rOutDev)
{
    const int nAvInd(500);
    static double fFactors[nAvInd];
    static int nIndex(nAvInd + 1);
    static int nRepeat(5);
    static int nWorseTotal(0);
    static int nBetterTotal(0);
    int a(0);

    const Size aSizePixel(rDestPixel.GetSize());

    // init statics
    if (nIndex > nAvInd)
    {
        for (a = 0; a < nAvInd; a++)
            fFactors[a] = 1.0;
        nIndex = 0;
    }

    // get start time
    const sal_uInt64 nTimeA(tools::Time::GetSystemTicks());

    // loop nRepeat times to get somewhat better timings, else
    // numbers are pretty small
    for (a = 0; a < nRepeat; a++)
    {
        // "Former" method using a temporary AlphaMask & DrawBitmapEx
        sal_uInt8 nMaskValue(static_cast<sal_uInt8>(basegfx::fround(fTrans * 255.0)));
        const AlphaMask aAlphaMask(aSizePixel, &nMaskValue);
        rOutDev.DrawBitmapEx(rDestPixel.TopLeft(), BitmapEx(rContent, aAlphaMask));
    }

    // get intermediate time
    const sal_uInt64 nTimeB(tools::Time::GetSystemTicks());

    // loop nRepeat times
    for (a = 0; a < nRepeat; a++)
    {
        // New method using DrawTransformedBitmapEx & fTrans directly
        rOutDev.DrawTransformedBitmapEx(basegfx::utils::createScaleTranslateB2DHomMatrix(
                                            aSizePixel.Width(), aSizePixel.Height(),
                                            rDestPixel.TopLeft().X(), rDestPixel.TopLeft().Y()),
                                        BitmapEx(rContent), 1 - fTrans);
    }

    // get end time
    const sal_uInt64 nTimeC(tools::Time::GetSystemTicks());

    // calculate deltas
    const sal_uInt64 nTimeFormer(nTimeB - nTimeA);
    const sal_uInt64 nTimeNew(nTimeC - nTimeB);

    // compare & note down
    if (nTimeFormer != nTimeNew && 0 != nTimeFormer && 0 != nTimeNew)
    {
        if ((nTimeFormer < 10 || nTimeNew < 10) && nRepeat < 500)
        {
            nRepeat += 1;
            SAL_INFO("drawinglayer.processor2d", "Increment nRepeat to " << nRepeat);
            return;
        }

        const double fNewFactor((double)nTimeFormer / nTimeNew);
        fFactors[nIndex % nAvInd] = fNewFactor;
        nIndex++;
        double fAverage(0.0);
        {
            for (a = 0; a < nAvInd; a++)
                fAverage += fFactors[a];
            fAverage /= nAvInd;
        }
        if (fNewFactor < 1.0)
            nWorseTotal++;
        else
            nBetterTotal++;

        char buf[300];
        sprintf(buf,
                "Former: %ld New: %ld It got %s (factor %f) (av. last %d Former/New is %f, "
                "WorseTotal: %d, BetterTotal: %d)",
                nTimeFormer, nTimeNew, fNewFactor < 1.0 ? "WORSE" : "BETTER",
                fNewFactor < 1.0 ? 1.0 / fNewFactor : fNewFactor, nAvInd, fAverage, nWorseTotal,
                nBetterTotal);
        SAL_INFO("drawinglayer.processor2d", buf);
    }
}
#endif
}

// support for rendering Bitmap and BitmapEx contents
namespace drawinglayer
{
// static global VDev buffer for VclProcessor2D/VclPixelProcessor2D
VDevBuffer& getVDevBuffer()
{
    // secure global instance with Vcl's safe destroyer of external (seen by
    // library base) stuff, the remembered VDevs need to be deleted before
    // Vcl's deinit
    static tools::DeleteOnDeinit<VDevBuffer> aVDevBuffer{};
    return *aVDevBuffer.get();
}

impBufferDevice::impBufferDevice(OutputDevice& rOutDev, const basegfx::B2DRange& rRange)
    : mrOutDev(rOutDev)
    , mpContent(nullptr)
    , mpAlpha(nullptr)
{
    basegfx::B2DRange aRangePixel(rRange);
    aRangePixel.transform(mrOutDev.GetViewTransformation());
    maDestPixel = tools::Rectangle(floor(aRangePixel.getMinX()), floor(aRangePixel.getMinY()),
                                   ceil(aRangePixel.getMaxX()), ceil(aRangePixel.getMaxY()));
    maDestPixel.Intersection(tools::Rectangle{ Point{}, mrOutDev.GetOutputSizePixel() });

    if (!isVisible())
        return;

    mpContent = getVDevBuffer().alloc(mrOutDev, maDestPixel.GetSize());

    // #i93485# assert when copying from window to VDev is used
    SAL_WARN_IF(
        mrOutDev.GetOutDevType() == OUTDEV_WINDOW, "drawinglayer",
        "impBufferDevice render helper: Copying from Window to VDev, this should be avoided (!)");

    // initialize buffer by blitting content of source to prepare for
    // transparence/ copying back
    const bool bWasEnabledSrc(mrOutDev.IsMapModeEnabled());
    mrOutDev.EnableMapMode(false);
    mpContent->DrawOutDev(Point(), maDestPixel.GetSize(), maDestPixel.TopLeft(),
                          maDestPixel.GetSize(), mrOutDev);
    mrOutDev.EnableMapMode(bWasEnabledSrc);

    MapMode aNewMapMode(mrOutDev.GetMapMode());

    const Point aLogicTopLeft(mrOutDev.PixelToLogic(maDestPixel.TopLeft()));
    aNewMapMode.SetOrigin(Point(-aLogicTopLeft.X(), -aLogicTopLeft.Y()));

    mpContent->SetMapMode(aNewMapMode);

    // copy AA flag for new target
    mpContent->SetAntialiasing(mrOutDev.GetAntialiasing());

    // copy RasterOp (e.g. may be RasterOp::Xor on destination)
    mpContent->SetRasterOp(mrOutDev.GetRasterOp());
}

impBufferDevice::~impBufferDevice()
{
    if (mpContent)
    {
        getVDevBuffer().free(*mpContent);
    }

    if (mpAlpha)
    {
        getVDevBuffer().free(*mpAlpha);
    }
}

void impBufferDevice::paint(double fTrans)
{
    if (!isVisible())
        return;

    const Point aEmptyPoint;
    const Size aSizePixel(maDestPixel.GetSize());
    const bool bWasEnabledDst(mrOutDev.IsMapModeEnabled());

    mrOutDev.EnableMapMode(false);
    mpContent->EnableMapMode(false);

#ifdef DBG_UTIL
    // VCL_DUMP_BMP_PATH should be like C:/path/ or ~/path/
    static bool bDoSaveForVisualControl(false); // loplugin:constvars:ignore
    static const OUString sDumpPath(o3tl::getEnvironment(u"VCL_DUMP_BMP_PATH"_ustr));

    if (!sDumpPath.isEmpty() && bDoSaveForVisualControl)
    {
        SvFileStream aNew(sDumpPath + "content.bmp", StreamMode::WRITE | StreamMode::TRUNC);
        Bitmap aContent(mpContent->GetBitmap(aEmptyPoint, aSizePixel));
        WriteDIB(aContent, aNew, false, true);
    }
#endif

    // during painting the buffer, disable evtl. set RasterOp (may be RasterOp::Xor)
    const RasterOp aOrigRasterOp(mrOutDev.GetRasterOp());
    mrOutDev.SetRasterOp(RasterOp::OverPaint);

    if (mpAlpha)
    {
        mpAlpha->EnableMapMode(false);
        AlphaMask aAlphaMask(mpAlpha->GetBitmap(aEmptyPoint, aSizePixel));
        aAlphaMask.Invert(); // convert transparency to alpha

#ifdef DBG_UTIL
        if (!sDumpPath.isEmpty() && bDoSaveForVisualControl)
        {
            SvFileStream aNew(sDumpPath + "transparence.bmp",
                              StreamMode::WRITE | StreamMode::TRUNC);
            WriteDIB(aAlphaMask.GetBitmap(), aNew, false, true);
        }
#endif

        Bitmap aContent(mpContent->GetBitmap(aEmptyPoint, aSizePixel));
        mrOutDev.DrawBitmapEx(maDestPixel.TopLeft(), BitmapEx(aContent, aAlphaMask));
    }
    else if (0.0 != fTrans)
    {
        const Bitmap aContent(mpContent->GetBitmap(aEmptyPoint, aSizePixel));

#ifdef SPEED_COMPARE
        static bool bCompareFormerAndNewTimings(true);

        if (bCompareFormerAndNewTimings)
        {
            doSpeedCompare(fTrans, aContent, maDestPixel, mrOutDev);
        }
        else
#endif
        // Note: this extra scope is needed due to 'clang plugin indentation'. It complains
        //       that lines 494 and (now) 539 are 'statement mis-aligned compared to neighbours'.
        //       That is true if SPEED_COMPARE is not defined. Not nice, but have to fix this.
        {
            // For the case we have a unified transparency value there is a former
            // and new method to paint that which can be used. To decide on measurements,
            // I added 'doSpeedCompare' above which can be activated by defining
            // SPEED_COMPARE at the top of this file.
            // I added the used Testdoc: blurplay3.odg as
            //     https://bugs.documentfoundation.org/attachment.cgi?id=182463
            // I did measure on
            //
            // Linux Dbg:
            // Former: 21 New: 32 It got WORSE (factor 1.523810) (av. last 500 Former/New is 0.968533, WorseTotal: 515, BetterTotal: 934)
            //
            // Linux Pro:
            // Former: 27 New: 44 It got WORSE (factor 1.629630) (av. last 500 Former/New is 0.923256, WorseTotal: 433, BetterTotal: 337)
            //
            // Win Dbg:
            // Former: 21 New: 78 It got WORSE (factor 3.714286) (av. last 500 Former/New is 1.007176, WorseTotal: 85, BetterTotal: 1428)
            //
            // Win Pro:
            // Former: 3 New: 4 It got WORSE (factor 1.333333) (av. last 500 Former/New is 1.054167, WorseTotal: 143, BetterTotal: 3909)
            //
            // Note: I am aware that the Dbg are of limited usefulness, but include them here
            // for reference.
            //
            // The important part is "av. last 500 Former/New is %ld" which describes the averaged factor from Former/New
            // over the last 500 measurements. When < 1.0 Former is better (Linux), > 1.0 (Win) New is better. Since the
            // factor on Win is still close to 1.0 what means we lose nearly nothing and Linux Former is better, I will
            // use Former for now.
            //
            // To easily allow to change this (maybe system-dependent) I add a static switch here,
            // also for eventually experimenting (hint: can be changed in the debugger).
            static bool bUseNew(false);

            if (bUseNew)
            {
                // New method using DrawTransformedBitmapEx & fTrans directly
                mrOutDev.DrawTransformedBitmapEx(basegfx::utils::createScaleTranslateB2DHomMatrix(
                                                     aSizePixel.Width(), aSizePixel.Height(),
                                                     maDestPixel.TopLeft().X(),
                                                     maDestPixel.TopLeft().Y()),
                                                 BitmapEx(aContent), 1 - fTrans);
            }
            else
            {
                // "Former" method using a temporary AlphaMask & DrawBitmapEx
                sal_uInt8 nMaskValue(static_cast<sal_uInt8>(basegfx::fround(fTrans * 255.0)));
                const AlphaMask aAlphaMask(aSizePixel, &nMaskValue);
                mrOutDev.DrawBitmapEx(maDestPixel.TopLeft(), BitmapEx(aContent, aAlphaMask));
            }
        }
    }
    else
    {
        mrOutDev.DrawOutDev(maDestPixel.TopLeft(), aSizePixel, aEmptyPoint, aSizePixel, *mpContent);
    }

    mrOutDev.SetRasterOp(aOrigRasterOp);
    mrOutDev.EnableMapMode(bWasEnabledDst);
}

VirtualDevice& impBufferDevice::getContent()
{
    SAL_WARN_IF(!mpContent, "drawinglayer",
                "impBufferDevice: No content, check isVisible() before accessing (!)");
    return *mpContent;
}

VirtualDevice& impBufferDevice::getTransparence()
{
    SAL_WARN_IF(!mpContent, "drawinglayer",
                "impBufferDevice: No content, check isVisible() before accessing (!)");
    if (!mpAlpha)
    {
        mpAlpha = getVDevBuffer().alloc(mrOutDev, maDestPixel.GetSize());
        mpAlpha->SetMapMode(mpContent->GetMapMode());

        // copy AA flag for new target; masking needs to be smooth
        mpAlpha->SetAntialiasing(mpContent->GetAntialiasing());
    }

    return *mpAlpha;
}
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
