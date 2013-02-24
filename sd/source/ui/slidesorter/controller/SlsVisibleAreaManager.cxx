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


#include "controller/SlsVisibleAreaManager.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsAnimationFunction.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsCurrentSlideManager.hxx"


namespace sd { namespace slidesorter { namespace controller {

namespace {
    class VisibleAreaScroller
    {
    public:
        VisibleAreaScroller (
            SlideSorter& rSlideSorter,
            const Point aStart,
            const Point aEnd);
        void operator() (const double nValue);
    private:
        SlideSorter& mrSlideSorter;
        Point maStart;
        const Point maEnd;
        const ::boost::function<double(double)> maAccelerationFunction;
    };

} // end of anonymous namespace



VisibleAreaManager::VisibleAreaManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      maVisibleRequests(),
      mnScrollAnimationId(Animator::NotAnAnimationId),
      maRequestedVisibleTopLeft(),
      meRequestedAnimationMode(Animator::AM_Immediate),
      mbIsCurrentSlideTrackingActive(true),
      mnDisableCount(0)
{
}




VisibleAreaManager::~VisibleAreaManager (void)
{
}




void VisibleAreaManager::ActivateCurrentSlideTracking (void)
{
    mbIsCurrentSlideTrackingActive = true;
}




void VisibleAreaManager::DeactivateCurrentSlideTracking (void)
{
    mbIsCurrentSlideTrackingActive = false;
}




bool VisibleAreaManager::IsCurrentSlideTrackingActive (void) const
{
    return mbIsCurrentSlideTrackingActive;
}




void VisibleAreaManager::RequestVisible (
    const model::SharedPageDescriptor& rpDescriptor,
    const bool bForce)
{
    if (rpDescriptor)
    {
        if (mnDisableCount == 0)
        {
            maVisibleRequests.push_back(
                mrSlideSorter.GetView().GetLayouter().GetPageObjectBox(
                    rpDescriptor->GetPageIndex(),
                    true));
        }
        if (bForce && ! mbIsCurrentSlideTrackingActive)
            ActivateCurrentSlideTracking();
        MakeVisible();
    }
}




void VisibleAreaManager::RequestCurrentSlideVisible (void)
{
    if (mbIsCurrentSlideTrackingActive && mnDisableCount==0)
        RequestVisible(
            mrSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide());
}




void VisibleAreaManager::MakeVisible (void)
{
    if (maVisibleRequests.empty())
        return;

    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if ( ! pWindow)
        return;
    const Point aCurrentTopLeft (pWindow->PixelToLogic(Point(0,0)));

    const ::boost::optional<Point> aNewVisibleTopLeft (GetRequestedTopLeft());
    maVisibleRequests.clear();
    if ( ! aNewVisibleTopLeft)
        return;

    // We now know what the visible area shall be.  Scroll accordingly
    // unless that is not already the visible area or a running scroll
    // animation has it as its target area.
    if (mnScrollAnimationId!=Animator::NotAnAnimationId
        && maRequestedVisibleTopLeft==aNewVisibleTopLeft)
        return;

    // Stop a running animation.
    if (mnScrollAnimationId != Animator::NotAnAnimationId)
        mrSlideSorter.GetController().GetAnimator()->RemoveAnimation(mnScrollAnimationId);

    maRequestedVisibleTopLeft = aNewVisibleTopLeft.get();
    VisibleAreaScroller aAnimation(
        mrSlideSorter,
        aCurrentTopLeft,
        maRequestedVisibleTopLeft);
    if (meRequestedAnimationMode==Animator::AM_Animated
        && mrSlideSorter.GetProperties()->IsSmoothSelectionScrolling())
    {
        mnScrollAnimationId = mrSlideSorter.GetController().GetAnimator()->AddAnimation(
            aAnimation,
            0,
            300);
    }
    else
    {
        // Execute the animation at its final value.
        aAnimation(1.0);
    }
    meRequestedAnimationMode = Animator::AM_Immediate;
}




::boost::optional<Point> VisibleAreaManager::GetRequestedTopLeft (void) const
{
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if ( ! pWindow)
        return ::boost::optional<Point>();

    // Get the currently visible area and the model area.
    const Rectangle aVisibleArea (pWindow->PixelToLogic(
        Rectangle(
            Point(0,0),
            pWindow->GetOutputSizePixel())));
    const Rectangle aModelArea (mrSlideSorter.GetView().GetModelArea());

    sal_Int32 nVisibleTop (aVisibleArea.Top());
    const sal_Int32 nVisibleWidth (aVisibleArea.GetWidth());
    sal_Int32 nVisibleLeft (aVisibleArea.Left());
    const sal_Int32 nVisibleHeight (aVisibleArea.GetHeight());

    // Find the longest run of boxes whose union fits into the visible area.
    for (::std::vector<Rectangle>::const_iterator
             iBox(maVisibleRequests.begin()),
             iEnd(maVisibleRequests.end());
         iBox!=iEnd;
         ++iBox)
    {
        if (nVisibleTop+nVisibleHeight <= iBox->Bottom())
            nVisibleTop = iBox->Bottom()-nVisibleHeight;
        if (nVisibleTop > iBox->Top())
            nVisibleTop = iBox->Top();

        if (nVisibleLeft+nVisibleWidth <= iBox->Right())
            nVisibleLeft = iBox->Right()-nVisibleWidth;
        if (nVisibleLeft > iBox->Left())
            nVisibleLeft = iBox->Left();

        // Make sure the visible area does not move outside the model area.
        if (nVisibleTop + nVisibleHeight > aModelArea.Bottom())
            nVisibleTop = aModelArea.Bottom() - nVisibleHeight;
        if (nVisibleTop < aModelArea.Top())
            nVisibleTop = aModelArea.Top();

        if (nVisibleLeft + nVisibleWidth > aModelArea.Right())
            nVisibleLeft = aModelArea.Right() - nVisibleWidth;
        if (nVisibleLeft < aModelArea.Left())
            nVisibleLeft = aModelArea.Left();
    }

    const Point aRequestedTopLeft (nVisibleLeft, nVisibleTop);
    if (aRequestedTopLeft == aVisibleArea.TopLeft())
        return ::boost::optional<Point>();
    else
        return ::boost::optional<Point>(aRequestedTopLeft);
}




//===== VisibleAreaManager::TemporaryDisabler =================================

VisibleAreaManager::TemporaryDisabler::TemporaryDisabler (SlideSorter& rSlideSorter)
    : mrVisibleAreaManager(rSlideSorter.GetController().GetVisibleAreaManager())
{
    ++mrVisibleAreaManager.mnDisableCount;
}




VisibleAreaManager::TemporaryDisabler::~TemporaryDisabler (void)
{
    --mrVisibleAreaManager.mnDisableCount;
}



//===== VerticalVisibleAreaScroller ===========================================

namespace {

const static sal_Int32 gnMaxScrollDistance = 300;

VisibleAreaScroller::VisibleAreaScroller (
    SlideSorter& rSlideSorter,
    const Point aStart,
    const Point aEnd)
    : mrSlideSorter(rSlideSorter),
      maStart(aStart),
      maEnd(aEnd),
      maAccelerationFunction(
          controller::AnimationParametricFunction(
              controller::AnimationBezierFunction (0.1,0.6)))
{
    // When the distance to scroll is larger than a threshold then first
    // jump to within this distance of the final value and start the
    // animation from there.
    if (abs(aStart.X()-aEnd.X()) > gnMaxScrollDistance)
    {
        if (aStart.X() < aEnd.X())
            maStart.X() = aEnd.X()-gnMaxScrollDistance;
        else
            maStart.X() = aEnd.X()+gnMaxScrollDistance;
    }
    if (abs(aStart.Y()-aEnd.Y()) > gnMaxScrollDistance)
    {
        if (aStart.Y() < aEnd.Y())
            maStart.Y() = aEnd.Y()-gnMaxScrollDistance;
        else
            maStart.Y() = aEnd.Y()+gnMaxScrollDistance;
    }
}




void VisibleAreaScroller::operator() (const double nTime)
{
    const double nLocalTime (maAccelerationFunction(nTime));
    mrSlideSorter.GetController().GetScrollBarManager().SetTopLeft(
        Point(
            sal_Int32(0.5 + maStart.X() * (1.0 - nLocalTime) + maEnd.X() * nLocalTime),
            sal_Int32 (0.5 + maStart.Y() * (1.0 - nLocalTime) + maEnd.Y() * nLocalTime)));
}

} // end of anonymous namespace

} } } // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
