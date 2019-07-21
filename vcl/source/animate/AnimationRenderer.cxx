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

#include <tools/helpers.hxx>

#include <vcl/animate/AnimationRenderer.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>

#include <AnimationData.hxx>
#include <window.h>

#include <memory>

AnimationRenderer::AnimationRenderer(Animation* pParent, OutputDevice* pOut, const Point& rPt,
                                     const Size& rSz, sal_uLong nCallerId,
                                     OutputDevice* pFirstFrameOutDev)
    : mpParent(pParent)
    , mpRenderContext(pFirstFrameOutDev ? pFirstFrameOutDev : pOut)
    , maClip(mpRenderContext->GetClipRegion())
    , maSizePx(mpRenderContext->LogicToPixel(maLogicalSize))
    , mnCallerId(nCallerId)
    , maOriginPt(rPt)
    , maLogicalSize(rSz)
    , mpBackground(VclPtr<VirtualDevice>::Create())
    , mpRestore(VclPtr<VirtualDevice>::Create())
    , meLastDisposal(Disposal::Back)
    , mbIsPaused(false)
    , mbIsMarked(false)
    , mbMirroredHorizontally(maLogicalSize.Width() < 0)
    , mbMirroredVertically(maLogicalSize.Height() < 0)
{
    Animation::ImplIncAnimCount();

    // Mirrored horizontally?
    if (mbMirroredHorizontally)
    {
        maDispPt.setX(maOriginPt.X() + maLogicalSize.Width() + 1);
        maDispSz.setWidth(-maLogicalSize.Width());
        maSizePx.setWidth(-maSizePx.Width());
    }
    else
    {
        maDispPt.setX(maOriginPt.X());
        maDispSz.setWidth(maLogicalSize.Width());
    }

    // Mirrored vertically?
    if (mbMirroredVertically)
    {
        maDispPt.setY(maOriginPt.Y() + maLogicalSize.Height() + 1);
        maDispSz.setHeight(-maLogicalSize.Height());
        maSizePx.setHeight(-maSizePx.Height());
    }
    else
    {
        maDispPt.setY(maOriginPt.Y());
        maDispSz.setHeight(maLogicalSize.Height());
    }

    // save background
    mpBackground->SetOutputSizePixel(maSizePx);
    mpRenderContext->SaveBackground(*mpBackground, maDispPt, maDispSz, maSizePx);

    // Initialize drawing to actual position
    DrawToIndex(mpParent->ImplGetCurPos());

    // If first frame OutputDevice is set, update variables now for real OutputDevice
    if (pFirstFrameOutDev)
    {
        mpRenderContext = pOut;
        maClip = mpRenderContext->GetClipRegion();
    }
}

AnimationRenderer::~AnimationRenderer()
{
    mpBackground.disposeAndClear();
    mpRestore.disposeAndClear();

    Animation::ImplDecAnimCount();
}

bool AnimationRenderer::Matches(OutputDevice* pOut, long nCallerId) const
{
    bool bRet = false;

    if (nCallerId)
    {
        if ((mnCallerId == nCallerId) && (!pOut || (pOut == mpRenderContext)))
            bRet = true;
    }
    else if (!pOut || (pOut == mpRenderContext))
        bRet = true;

    return bRet;
}

void AnimationRenderer::getPosSize(const AnimationBitmap& rAnimationBitmap, Point& rPosPix,
                                   Size& rSizePix)
{
    const Size& rAnmSize = mpParent->GetDisplaySizePixel();
    Point aPt2(rAnimationBitmap.maPositionPixel.X() + rAnimationBitmap.maSizePixel.Width() - 1,
               rAnimationBitmap.maPositionPixel.Y() + rAnimationBitmap.maSizePixel.Height() - 1);
    double fFactX, fFactY;

    // calculate x scaling
    if (rAnmSize.Width() > 1)
        fFactX = static_cast<double>(maSizePx.Width() - 1) / (rAnmSize.Width() - 1);
    else
        fFactX = 1.0;

    // calculate y scaling
    if (rAnmSize.Height() > 1)
        fFactY = static_cast<double>(maSizePx.Height() - 1) / (rAnmSize.Height() - 1);
    else
        fFactY = 1.0;

    rPosPix.setX(FRound(rAnimationBitmap.maPositionPixel.X() * fFactX));
    rPosPix.setY(FRound(rAnimationBitmap.maPositionPixel.Y() * fFactY));

    aPt2.setX(FRound(aPt2.X() * fFactX));
    aPt2.setY(FRound(aPt2.Y() * fFactY));

    rSizePix.setWidth(aPt2.X() - rPosPix.X() + 1);
    rSizePix.setHeight(aPt2.Y() - rPosPix.Y() + 1);

    // Mirrored horizontally?
    if (mbMirroredHorizontally)
        rPosPix.setX(maSizePx.Width() - 1 - aPt2.X());

    // Mirrored vertically?
    if (mbMirroredVertically)
        rPosPix.setY(maSizePx.Height() - 1 - aPt2.Y());
}

void AnimationRenderer::DrawToIndex(sal_uLong nIndex)
{
    VclPtr<vcl::RenderContext> pRenderContext = mpRenderContext;

    ScopedVclPtrInstance<VirtualDevice> aVDev;
    std::unique_ptr<vcl::Region> xOldClip(
        !maClip.IsNull() ? new vcl::Region(pRenderContext->GetClipRegion()) : nullptr);

    aVDev->SetOutputSizePixel(maSizePx, false);
    nIndex = std::min(nIndex, static_cast<sal_uLong>(mpParent->Count()) - 1);

    for (sal_uLong i = 0; i <= nIndex; i++)
        Draw(i, aVDev.get());

    if (xOldClip)
        pRenderContext->SetClipRegion(maClip);

    pRenderContext->DrawOutDev(maDispPt, maDispSz, Point(), maSizePx, *aVDev);

    if (xOldClip)
        pRenderContext->SetClipRegion(*xOldClip);
}

void AnimationRenderer::Draw(sal_uLong nIndex, VirtualDevice* pVDev)
{
    VclPtr<vcl::RenderContext> pRenderContext = mpRenderContext;

    std::unique_ptr<PaintBufferGuard> pGuard;
    if (!pVDev && mpRenderContext->GetOutDevType() == OUTDEV_WINDOW)
    {
        vcl::Window* pWindow = static_cast<vcl::Window*>(mpRenderContext.get());
        pGuard.reset(new PaintBufferGuard(pWindow->ImplGetWindowImpl()->mpFrameData, pWindow));
        pRenderContext = pGuard->GetRenderContext();
    }

    tools::Rectangle aOutRect(pRenderContext->PixelToLogic(Point()),
                              pRenderContext->GetOutputSize());

    // check, if output lies out of display
    if (aOutRect.Intersection(tools::Rectangle(maDispPt, maDispSz)).IsEmpty())
        setMarked(true);
    else if (!mbIsPaused)
    {
        VclPtr<VirtualDevice> pDev;
        Point aPosPix;
        Point aBmpPosPix;
        Size aSizePix;
        Size aBmpSizePix;
        const sal_uLong nLastPos = mpParent->Count() - 1;
        mnActIndex = std::min(nIndex, nLastPos);
        const AnimationBitmap& rAnimationBitmap
            = mpParent->Get(static_cast<sal_uInt16>(mnActIndex));

        getPosSize(rAnimationBitmap, aPosPix, aSizePix);

        // Mirrored horizontally?
        if (mbMirroredHorizontally)
        {
            aBmpPosPix.setX(aPosPix.X() + aSizePix.Width() - 1);
            aBmpSizePix.setWidth(-aSizePix.Width());
        }
        else
        {
            aBmpPosPix.setX(aPosPix.X());
            aBmpSizePix.setWidth(aSizePix.Width());
        }

        // Mirrored vertically?
        if (mbMirroredVertically)
        {
            aBmpPosPix.setY(aPosPix.Y() + aSizePix.Height() - 1);
            aBmpSizePix.setHeight(-aSizePix.Height());
        }
        else
        {
            aBmpPosPix.setY(aPosPix.Y());
            aBmpSizePix.setHeight(aSizePix.Height());
        }

        // get output device
        if (!pVDev)
        {
            pDev = VclPtr<VirtualDevice>::Create();
            pDev->SetOutputSizePixel(maSizePx, false);
            pDev->DrawOutDev(Point(), maSizePx, maDispPt, maDispSz, *pRenderContext);
        }
        else
            pDev = pVDev;

        // restore background after each run
        if (!nIndex)
        {
            meLastDisposal = Disposal::Back;
            maRestPt = Point();
            maRestSz = maSizePx;
        }

        // restore
        if ((Disposal::Not != meLastDisposal) && maRestSz.Width() && maRestSz.Height())
        {
            if (Disposal::Back == meLastDisposal)
                pDev->DrawOutDev(maRestPt, maRestSz, maRestPt, maRestSz, *mpBackground);
            else
                pDev->DrawOutDev(maRestPt, maRestSz, Point(), maRestSz, *mpRestore);
        }

        meLastDisposal = rAnimationBitmap.meDisposal;
        maRestPt = aPosPix;
        maRestSz = aSizePix;

        // What do we need to restore the next time?
        // Put it into a bitmap if needed, else delete
        // SaveBitmap to conserve memory
        if ((meLastDisposal == Disposal::Back) || (meLastDisposal == Disposal::Not))
            mpRestore->SetOutputSizePixel(Size(1, 1), false);
        else
        {
            mpRestore->SetOutputSizePixel(maRestSz, false);
            mpRestore->DrawOutDev(Point(), maRestSz, aPosPix, aSizePix, *pDev);
        }

        pDev->DrawBitmapEx(aBmpPosPix, aBmpSizePix, rAnimationBitmap.maBitmapEx);

        if (!pVDev)
        {
            std::unique_ptr<vcl::Region> xOldClip(
                !maClip.IsNull() ? new vcl::Region(pRenderContext->GetClipRegion()) : nullptr);

            if (xOldClip)
                pRenderContext->SetClipRegion(maClip);

            pRenderContext->DrawOutDev(maDispPt, maDispSz, Point(), maSizePx, *pDev);
            if (pGuard)
                pGuard->SetPaintRect(tools::Rectangle(maDispPt, maDispSz));

            if (xOldClip)
            {
                pRenderContext->SetClipRegion(*xOldClip);
                xOldClip.reset();
            }

            pDev.disposeAndClear();

            if (pRenderContext->GetOutDevType() == OUTDEV_WINDOW)
                static_cast<vcl::Window*>(pRenderContext.get())->Flush();
        }
    }
}

void AnimationRenderer::repaint()
{
    const bool bOldPause = mbIsPaused;

    mpRenderContext->SaveBackground(*mpBackground, maDispPt, maDispSz, maSizePx);

    mbIsPaused = false;
    DrawToIndex(mnActIndex);
    mbIsPaused = bOldPause;
}

AnimationData* AnimationRenderer::createAnimationData() const
{
    AnimationData* pAnimationData = new AnimationData;

    pAnimationData->maOriginStartPt = maOriginPt;
    pAnimationData->aStartSize = maLogicalSize;
    pAnimationData->mpRenderContext = mpRenderContext;
    pAnimationData->mpAnimationRenderer = const_cast<AnimationRenderer*>(this);
    pAnimationData->mnCallerId = mnCallerId;
    pAnimationData->mbIsPaused = mbIsPaused;

    return pAnimationData;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
