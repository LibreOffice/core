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

#include <cstdlib>

#include <controller/SlsVisibleAreaManager.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsProperties.hxx>
#include <controller/SlsAnimationFunction.hxx>
#include <controller/SlsScrollBarManager.hxx>
#include <controller/SlsCurrentSlideManager.hxx>
#include <Window.hxx>
#include <SlideSorter.hxx>
#include <view/SlideSorterView.hxx>

namespace sd { namespace slidesorter { namespace controller {

namespace {
    class VisibleAreaScroller
    {
    public:
        VisibleAreaScroller (
            SlideSorter& rSlideSorter,
            const Point& rStart,
            const Point& rEnd);
        void operator() (const double nValue);
    private:
        SlideSorter& mrSlideSorter;
        Point maStart;
        const Point maEnd;
        const ::std::function<double (double)> maAccelerationFunction;
    };

} // end of anonymous namespace

VisibleAreaManager::VisibleAreaManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      maVisibleRequests(),
      maRequestedVisibleTopLeft(),
      mbIsCurrentSlideTrackingActive(true),
      mnDisableCount(0)
{
}

VisibleAreaManager::~VisibleAreaManager()
{
}

void VisibleAreaManager::ActivateCurrentSlideTracking()
{
    mbIsCurrentSlideTrackingActive = true;
}

void VisibleAreaManager::DeactivateCurrentSlideTracking()
{
    mbIsCurrentSlideTrackingActive = false;
}

void VisibleAreaManager::RequestVisible (
    const model::SharedPageDescriptor& rpDescriptor,
    const bool bForce)
{
    if (!rpDescriptor)
        return;

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

void VisibleAreaManager::RequestCurrentSlideVisible()
{
    if (mbIsCurrentSlideTrackingActive && mnDisableCount==0)
        RequestVisible(
            mrSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide());
}

void VisibleAreaManager::MakeVisible()
{
    if (maVisibleRequests.empty())
        return;

    sd::Window *pWindow (mrSlideSorter.GetContentWindow().get());
    if ( ! pWindow)
        return;
    const Point aCurrentTopLeft (pWindow->PixelToLogic(Point(0,0)));

    const ::boost::optional<Point> aNewVisibleTopLeft (GetRequestedTopLeft());
    maVisibleRequests.clear();
    if ( ! aNewVisibleTopLeft)
        return;

    maRequestedVisibleTopLeft = aNewVisibleTopLeft.get();
    VisibleAreaScroller aAnimation(
        mrSlideSorter,
        aCurrentTopLeft,
        maRequestedVisibleTopLeft);
    // Execute the animation at its final value.
    aAnimation(1.0);
}

::boost::optional<Point> VisibleAreaManager::GetRequestedTopLeft() const
{
    sd::Window *pWindow (mrSlideSorter.GetContentWindow().get());
    if ( ! pWindow)
        return ::boost::optional<Point>();

    // Get the currently visible area and the model area.
    const ::tools::Rectangle aVisibleArea (pWindow->PixelToLogic(
        ::tools::Rectangle(
            Point(0,0),
            pWindow->GetOutputSizePixel())));
    const ::tools::Rectangle aModelArea (mrSlideSorter.GetView().GetModelArea());

    sal_Int32 nVisibleTop (aVisibleArea.Top());
    const sal_Int32 nVisibleWidth (aVisibleArea.GetWidth());
    sal_Int32 nVisibleLeft (aVisibleArea.Left());
    const sal_Int32 nVisibleHeight (aVisibleArea.GetHeight());

    // Find the longest run of boxes whose union fits into the visible area.
    for (const auto& rBox : maVisibleRequests)
    {
        if (nVisibleTop+nVisibleHeight <= rBox.Bottom())
            nVisibleTop = rBox.Bottom()-nVisibleHeight;
        if (nVisibleTop > rBox.Top())
            nVisibleTop = rBox.Top();

        if (nVisibleLeft+nVisibleWidth <= rBox.Right())
            nVisibleLeft = rBox.Right()-nVisibleWidth;
        if (nVisibleLeft > rBox.Left())
            nVisibleLeft = rBox.Left();

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

VisibleAreaManager::TemporaryDisabler::TemporaryDisabler (SlideSorter const & rSlideSorter)
    : mrVisibleAreaManager(rSlideSorter.GetController().GetVisibleAreaManager())
{
    ++mrVisibleAreaManager.mnDisableCount;
}

VisibleAreaManager::TemporaryDisabler::~TemporaryDisabler()
{
    --mrVisibleAreaManager.mnDisableCount;
}

//===== VerticalVisibleAreaScroller ===========================================

namespace {

const static sal_Int32 gnMaxScrollDistance = 300;

VisibleAreaScroller::VisibleAreaScroller (
    SlideSorter& rSlideSorter,
    const Point& rStart,
    const Point& rEnd)
    : mrSlideSorter(rSlideSorter),
      maStart(rStart),
      maEnd(rEnd),
      maAccelerationFunction(
          controller::AnimationParametricFunction(
              controller::AnimationBezierFunction (0.1,0.6)))
{
    // When the distance to scroll is larger than a threshold then first
    // jump to within this distance of the final value and start the
    // animation from there.
    if (std::abs(rStart.X()-rEnd.X()) > gnMaxScrollDistance)
    {
        if (rStart.X() < rEnd.X())
            maStart.setX( rEnd.X()-gnMaxScrollDistance );
        else
            maStart.setX( rEnd.X()+gnMaxScrollDistance );
    }
    if (std::abs(rStart.Y()-rEnd.Y()) > gnMaxScrollDistance)
    {
        if (rStart.Y() < rEnd.Y())
            maStart.setY( rEnd.Y()-gnMaxScrollDistance );
        else
            maStart.setY( rEnd.Y()+gnMaxScrollDistance );
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
