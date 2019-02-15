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

#include "PresenterScrollBar.hxx"
#include "PresenterBitmapContainer.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterPaintManager.hxx"
#include "PresenterTimer.hxx"
#include "PresenterUIPainter.hxx"
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>

#include <algorithm>
#include <memory>
#include <math.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

const static double gnScrollBarGap (10);

namespace sdext { namespace presenter {

//===== PresenterScrollBar::MousePressRepeater ================================

class PresenterScrollBar::MousePressRepeater
    : public std::enable_shared_from_this<MousePressRepeater>
{
public:
    explicit MousePressRepeater (const ::rtl::Reference<PresenterScrollBar>& rpScrollBar);
    void Dispose();
    void Start (const PresenterScrollBar::Area& reArea);
    void Stop();
    void SetMouseArea (const PresenterScrollBar::Area& reArea);

private:
    void Callback ();
    void Execute();

    sal_Int32 mnMousePressRepeaterTaskId;
    ::rtl::Reference<PresenterScrollBar> mpScrollBar;
    PresenterScrollBar::Area meMouseArea;
};

//===== PresenterScrollBar ====================================================

std::weak_ptr<PresenterBitmapContainer> PresenterScrollBar::mpSharedBitmaps;

PresenterScrollBar::PresenterScrollBar (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<awt::XWindow>& rxParentWindow,
    const std::shared_ptr<PresenterPaintManager>& rpPaintManager,
    const ::std::function<void (double)>& rThumbMotionListener)
    : PresenterScrollBarInterfaceBase(m_aMutex),
      mxComponentContext(rxComponentContext),
      mxWindow(),
      mxCanvas(),
      mxPresenterHelper(),
      mpPaintManager(rpPaintManager),
      mnThumbPosition(0),
      mnTotalSize(0),
      mnThumbSize(0),
      mnLineHeight(10),
      maDragAnchor(-1,-1),
      maThumbMotionListener(rThumbMotionListener),
      meButtonDownArea(None),
      meMouseMoveArea(None),
      mbIsNotificationActive(false),
      mpBitmaps(),
      mpPrevButtonDescriptor(),
      mpNextButtonDescriptor(),
      mpPagerStartDescriptor(),
      mpPagerCenterDescriptor(),
      mpPagerEndDescriptor(),
      mpThumbStartDescriptor(),
      mpThumbCenterDescriptor(),
      mpThumbEndDescriptor(),
      mpMousePressRepeater(new MousePressRepeater(this)),
      mpBackgroundBitmap(),
      mpCanvasHelper(new PresenterCanvasHelper())
{
    try
    {
        Reference<lang::XMultiComponentFactory> xFactory (rxComponentContext->getServiceManager());
        if ( ! xFactory.is())
            throw RuntimeException();

        mxPresenterHelper.set(
            xFactory->createInstanceWithContext(
                "com.sun.star.comp.Draw.PresenterHelper",
                rxComponentContext),
            UNO_QUERY_THROW);

        if (mxPresenterHelper.is())
            mxWindow = mxPresenterHelper->createWindow(rxParentWindow,
                false,
                false,
                false,
                false);

        // Make the background transparent.  The slide show paints its own background.
        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY_THROW);
        xPeer->setBackground(0xff000000);

        mxWindow->setVisible(true);
        mxWindow->addWindowListener(this);
        mxWindow->addPaintListener(this);
        mxWindow->addMouseListener(this);
        mxWindow->addMouseMotionListener(this);
    }
    catch (RuntimeException&)
    {
    }
}

PresenterScrollBar::~PresenterScrollBar()
{
}

void SAL_CALL PresenterScrollBar::disposing()
{
    mpMousePressRepeater->Dispose();

    if (mxWindow.is())
    {
        mxWindow->removeWindowListener(this);
        mxWindow->removePaintListener(this);
        mxWindow->removeMouseListener(this);
        mxWindow->removeMouseMotionListener(this);

        Reference<lang::XComponent> xComponent (mxWindow, UNO_QUERY);
        mxWindow = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    mpBitmaps.reset();
}

void PresenterScrollBar::SetVisible (const bool bIsVisible)
{
    if (mxWindow.is())
        mxWindow->setVisible(bIsVisible);
}

void PresenterScrollBar::SetPosSize (const css::geometry::RealRectangle2D& rBox)
{
    if (mxWindow.is())
    {
        mxWindow->setPosSize(
            sal_Int32(floor(rBox.X1)),
            sal_Int32(ceil(rBox.Y1)),
            sal_Int32(ceil(rBox.X2-rBox.X1)),
            sal_Int32(floor(rBox.Y2-rBox.Y1)),
            awt::PosSize::POSSIZE);
        UpdateBorders();
    }
}

void PresenterScrollBar::SetThumbPosition (
    double nPosition,
    const bool bAsynchronousUpdate)
{
    nPosition = ValidateThumbPosition(nPosition);

    if (nPosition == mnThumbPosition || mbIsNotificationActive)
        return;

    mnThumbPosition = nPosition;

    UpdateBorders();
    Repaint(GetRectangle(Total), bAsynchronousUpdate);

    mbIsNotificationActive = true;
    try
    {
        maThumbMotionListener(mnThumbPosition);
    }
    catch (Exception&)
    {
    }
    mbIsNotificationActive = false;
}


void PresenterScrollBar::SetTotalSize (const double nTotalSize)
{
    if (mnTotalSize != nTotalSize)
    {
        mnTotalSize = nTotalSize + 1;
        UpdateBorders();
        Repaint(GetRectangle(Total), false);
    }
}

void PresenterScrollBar::SetThumbSize (const double nThumbSize)
{
    OSL_ASSERT(nThumbSize>=0);
    if (mnThumbSize != nThumbSize)
    {
        mnThumbSize = nThumbSize;
        UpdateBorders();
        Repaint(GetRectangle(Total), false);
    }
}


void PresenterScrollBar::SetLineHeight (const double nLineHeight)
{
    mnLineHeight = nLineHeight;
}


void PresenterScrollBar::SetCanvas (const Reference<css::rendering::XCanvas>& rxCanvas)
{
    if (mxCanvas == rxCanvas)
        return;

    mxCanvas = rxCanvas;
    if (!mxCanvas.is())
        return;

    if (mpBitmaps == nullptr)
    {
        if (mpSharedBitmaps.expired())
        {
            try
            {
                mpBitmaps.reset(new PresenterBitmapContainer(
                    OUString("PresenterScreenSettings/ScrollBar/Bitmaps"),
                    std::shared_ptr<PresenterBitmapContainer>(),
                    mxComponentContext,
                    mxCanvas));
                mpSharedBitmaps = mpBitmaps;
            }
            catch(Exception&)
            {
                OSL_ASSERT(false);
            }
        }
        else
            mpBitmaps = std::shared_ptr<PresenterBitmapContainer>(mpSharedBitmaps);
        UpdateBitmaps();
        UpdateBorders();
    }

    Repaint(GetRectangle(Total), false);
}

void PresenterScrollBar::SetBackground (const SharedBitmapDescriptor& rpBackgroundBitmap)
{
    mpBackgroundBitmap = rpBackgroundBitmap;
}

void PresenterScrollBar::CheckValues()
{
    mnThumbPosition = ValidateThumbPosition(mnThumbPosition);
}

double PresenterScrollBar::ValidateThumbPosition (double nPosition)
{
    if (nPosition + mnThumbSize > mnTotalSize)
        nPosition = mnTotalSize - mnThumbSize;
    if (nPosition < 0)
        nPosition = 0;
    return nPosition;
}

void PresenterScrollBar::Paint (
    const awt::Rectangle& rUpdateBox)
{
    if ( ! mxCanvas.is() || ! mxWindow.is())
    {
        OSL_ASSERT(mxCanvas.is());
        OSL_ASSERT(mxWindow.is());
        return;
    }

    if (PresenterGeometryHelper::AreRectanglesDisjoint (rUpdateBox, mxWindow->getPosSize()))
        return;

    PaintBackground(rUpdateBox);
    PaintComposite(rUpdateBox, PagerUp,
        mpPagerStartDescriptor, mpPagerCenterDescriptor, SharedBitmapDescriptor());
    PaintComposite(rUpdateBox, PagerDown,
        SharedBitmapDescriptor(), mpPagerCenterDescriptor, mpPagerEndDescriptor);
    PaintComposite(rUpdateBox, Thumb,
        mpThumbStartDescriptor, mpThumbCenterDescriptor, mpThumbEndDescriptor);
    PaintBitmap(rUpdateBox, PrevButton, mpPrevButtonDescriptor);
    PaintBitmap(rUpdateBox, NextButton, mpNextButtonDescriptor);

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(false);
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterScrollBar::windowResized (const css::awt::WindowEvent&) {}

void SAL_CALL PresenterScrollBar::windowMoved (const css::awt::WindowEvent&) {}

void SAL_CALL PresenterScrollBar::windowShown (const css::lang::EventObject&) {}

void SAL_CALL PresenterScrollBar::windowHidden (const css::lang::EventObject&) {}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterScrollBar::windowPaint (const css::awt::PaintEvent& rEvent)
{
    if (mxWindow.is())
    {
        awt::Rectangle aRepaintBox (rEvent.UpdateRect);
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        aRepaintBox.X += aWindowBox.X;
        aRepaintBox.Y += aWindowBox.Y;
        Paint(aRepaintBox);

        Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
        if (xSpriteCanvas.is())
            xSpriteCanvas->updateScreen(false);
    }
}

//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterScrollBar::mousePressed (const css::awt::MouseEvent& rEvent)
{
    maDragAnchor.X = rEvent.X;
    maDragAnchor.Y = rEvent.Y;
    meButtonDownArea = GetArea(rEvent.X, rEvent.Y);

    mpMousePressRepeater->Start(meButtonDownArea);
}

void SAL_CALL PresenterScrollBar::mouseReleased (const css::awt::MouseEvent&)
{
    mpMousePressRepeater->Stop();

    if (mxPresenterHelper.is())
        mxPresenterHelper->releaseMouse(mxWindow);
}

void SAL_CALL PresenterScrollBar::mouseEntered (const css::awt::MouseEvent&) {}

void SAL_CALL PresenterScrollBar::mouseExited (const css::awt::MouseEvent&)
{
    if (meMouseMoveArea != None)
    {
        const Area eOldMouseMoveArea (meMouseMoveArea);
        meMouseMoveArea = None;
        Repaint(GetRectangle(eOldMouseMoveArea), true);
    }
    meButtonDownArea = None;
    meMouseMoveArea = None;

    mpMousePressRepeater->Stop();
}

//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterScrollBar::mouseMoved (const css::awt::MouseEvent& rEvent)
{
    const Area eArea (GetArea(rEvent.X, rEvent.Y));
    if (eArea != meMouseMoveArea)
    {
        const Area eOldMouseMoveArea (meMouseMoveArea);
        meMouseMoveArea = eArea;
        if (eOldMouseMoveArea != None)
            Repaint(GetRectangle(eOldMouseMoveArea), meMouseMoveArea==None);
        if (meMouseMoveArea != None)
            Repaint(GetRectangle(meMouseMoveArea), true);
    }
    mpMousePressRepeater->SetMouseArea(eArea);
}

void SAL_CALL PresenterScrollBar::mouseDragged (const css::awt::MouseEvent& rEvent)
{
    if (meButtonDownArea != Thumb)
        return;

    mpMousePressRepeater->Stop();

    if (mxPresenterHelper.is())
        mxPresenterHelper->captureMouse(mxWindow);

    const double nDragDistance (GetDragDistance(rEvent.X,rEvent.Y));
    UpdateDragAnchor(nDragDistance);
    if (nDragDistance != 0)
    {
        SetThumbPosition(mnThumbPosition + nDragDistance, false);
    }
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterScrollBar::disposing (const css::lang::EventObject& rEvent)
{
    if (rEvent.Source == mxWindow)
        mxWindow = nullptr;
}


geometry::RealRectangle2D const & PresenterScrollBar::GetRectangle (const Area eArea) const
{
    OSL_ASSERT(eArea>=0 && eArea<AreaCount);

    return maBox[eArea];
}

void PresenterScrollBar::Repaint (
    const geometry::RealRectangle2D& rBox,
    const bool bAsynchronousUpdate)
{
    if (mpPaintManager != nullptr)
        mpPaintManager->Invalidate(
            mxWindow,
            PresenterGeometryHelper::ConvertRectangle(rBox),
            bAsynchronousUpdate);
}

void PresenterScrollBar::PaintBackground(
    const css::awt::Rectangle& rUpdateBox)
{
    if (mpBackgroundBitmap.get() == nullptr)
        return;

    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    mpCanvasHelper->Paint(
        mpBackgroundBitmap,
        mxCanvas,
        rUpdateBox,
        aWindowBox,
        awt::Rectangle());
}

void PresenterScrollBar::PaintBitmap(
    const css::awt::Rectangle& rUpdateBox,
    const Area eArea,
    const SharedBitmapDescriptor& rpBitmaps)
{
    const geometry::RealRectangle2D aLocalBox (GetRectangle(eArea));
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    geometry::RealRectangle2D aBox (aLocalBox);
    aBox.X1 += aWindowBox.X;
    aBox.Y1 += aWindowBox.Y;
    aBox.X2 += aWindowBox.X;
    aBox.Y2 += aWindowBox.Y;

    Reference<rendering::XBitmap> xBitmap (GetBitmap(eArea,rpBitmaps));

    if (!xBitmap.is())
        return;

    Reference<rendering::XPolyPolygon2D> xClipPolygon (
        PresenterGeometryHelper::CreatePolygon(
            PresenterGeometryHelper::Intersection(rUpdateBox,
                PresenterGeometryHelper::ConvertRectangle(aBox)),
            mxCanvas->getDevice()));

    const rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        xClipPolygon);

    const geometry::IntegerSize2D aBitmapSize (xBitmap->getSize());
    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(
            1,0,aBox.X1 + (aBox.X2-aBox.X1 - aBitmapSize.Width)/2,
            0,1,aBox.Y1 + (aBox.Y2-aBox.Y1 - aBitmapSize.Height)/2),
        nullptr,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    mxCanvas->drawBitmap(
        xBitmap,
        aViewState,
        aRenderState);
}

PresenterScrollBar::Area PresenterScrollBar::GetArea (const double nX, const double nY) const
{
    const geometry::RealPoint2D aPoint(nX, nY);

    if (PresenterGeometryHelper::IsInside(GetRectangle(Pager), aPoint))
    {
        if (PresenterGeometryHelper::IsInside(GetRectangle(Thumb), aPoint))
            return Thumb;
        else if (PresenterGeometryHelper::IsInside(GetRectangle(PagerUp), aPoint))
            return PagerUp;
        else if (PresenterGeometryHelper::IsInside(GetRectangle(PagerDown), aPoint))
            return PagerDown;
    }
    else if (PresenterGeometryHelper::IsInside(GetRectangle(PrevButton), aPoint))
        return PrevButton;
    else if (PresenterGeometryHelper::IsInside(GetRectangle(NextButton), aPoint))
        return NextButton;

    return None;
}

void PresenterScrollBar::UpdateWidthOrHeight (
    sal_Int32& rSize,
    const SharedBitmapDescriptor& rpDescriptor)
{
    if (rpDescriptor.get() != nullptr)
    {
        Reference<rendering::XBitmap> xBitmap (rpDescriptor->GetNormalBitmap());
        if (xBitmap.is())
        {
            const geometry::IntegerSize2D aBitmapSize (xBitmap->getSize());
            const sal_Int32 nBitmapSize = static_cast<sal_Int32>(GetMinor(aBitmapSize.Width, aBitmapSize.Height));
            if (nBitmapSize > rSize)
                rSize = nBitmapSize;
        }
    }
}

css::uno::Reference<css::rendering::XBitmap> PresenterScrollBar::GetBitmap (
    const Area eArea,
    const SharedBitmapDescriptor& rpBitmaps) const
{
    if (rpBitmaps.get() == nullptr)
        return nullptr;
    else
        return rpBitmaps->GetBitmap(GetBitmapMode(eArea));
}

PresenterBitmapContainer::BitmapDescriptor::Mode PresenterScrollBar::GetBitmapMode (
    const Area eArea) const
{
    if (IsDisabled(eArea))
        return PresenterBitmapContainer::BitmapDescriptor::Disabled;
    else if (eArea == meMouseMoveArea)
        return PresenterBitmapContainer::BitmapDescriptor::MouseOver;
    else
        return PresenterBitmapContainer::BitmapDescriptor::Normal;
}

bool PresenterScrollBar::IsDisabled (const Area eArea) const
{
    OSL_ASSERT(eArea>=0 && eArea<AreaCount);

    return ! maEnabledState[eArea];
}

//===== PresenterVerticalScrollBar ============================================

PresenterVerticalScrollBar::PresenterVerticalScrollBar (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<awt::XWindow>& rxParentWindow,
    const std::shared_ptr<PresenterPaintManager>& rpPaintManager,
    const ::std::function<void (double)>& rThumbMotionListener)
    : PresenterScrollBar(rxComponentContext, rxParentWindow, rpPaintManager, rThumbMotionListener),
      mnScrollBarWidth(0)
{
}

PresenterVerticalScrollBar::~PresenterVerticalScrollBar()
{
}

double PresenterVerticalScrollBar::GetDragDistance (const sal_Int32, const sal_Int32 nY) const
{
    const double nDistance (nY - maDragAnchor.Y);
    if (nDistance == 0)
        return 0;
    else
    {
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        const double nBarWidth (aWindowBox.Width);
        const double nPagerHeight (aWindowBox.Height - 2*nBarWidth);
        const double nDragDistance (mnTotalSize / nPagerHeight * nDistance);
        if (nDragDistance + mnThumbPosition < 0)
            return -mnThumbPosition;
        else if (mnThumbPosition + nDragDistance > mnTotalSize-mnThumbSize)
            return mnTotalSize-mnThumbSize-mnThumbPosition;
        else
            return nDragDistance;
    }
}

void PresenterVerticalScrollBar::UpdateDragAnchor (const double nDragDistance)
{
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    const double nBarWidth (aWindowBox.Width);
    const double nPagerHeight (aWindowBox.Height - 2*nBarWidth);
    maDragAnchor.Y += nDragDistance * nPagerHeight /  mnTotalSize;
}

sal_Int32 PresenterVerticalScrollBar::GetSize() const
{
    return mnScrollBarWidth;
}

double PresenterVerticalScrollBar::GetMinor (const double nX, const double) const
{
    return nX;
}

void PresenterVerticalScrollBar::UpdateBorders()
{
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    double nBottom = aWindowBox.Height;

    if (mpNextButtonDescriptor.get() != nullptr)
    {
        Reference<rendering::XBitmap> xBitmap (mpNextButtonDescriptor->GetNormalBitmap());
        if (xBitmap.is())
        {
            geometry::IntegerSize2D aSize (xBitmap->getSize());
            maBox[NextButton] = geometry::RealRectangle2D(
                0, nBottom - aSize.Height, aWindowBox.Width, nBottom);
            nBottom -= aSize.Height + gnScrollBarGap;
        }
    }
    if (mpPrevButtonDescriptor.get() != nullptr)
    {
        Reference<rendering::XBitmap> xBitmap (mpPrevButtonDescriptor->GetNormalBitmap());
        if (xBitmap.is())
        {
            geometry::IntegerSize2D aSize (xBitmap->getSize());
            maBox[PrevButton] = geometry::RealRectangle2D(
                0, nBottom - aSize.Height, aWindowBox.Width, nBottom);
            nBottom -= aSize.Height + gnScrollBarGap;
        }
    }
    const double nPagerHeight (nBottom);
    maBox[Pager] = geometry::RealRectangle2D(
        0,0, aWindowBox.Width, nBottom);
    if (mnTotalSize < 1)
    {
        maBox[Thumb] = maBox[Pager];

        // Set up the enabled/disabled states.
        maEnabledState[PrevButton] = false;
        maEnabledState[PagerUp] = false;
        maEnabledState[NextButton] = false;
        maEnabledState[PagerDown] = false;
        maEnabledState[Thumb] = false;
    }
    else
    {
        const double nThumbSize = ::std::min(mnThumbSize,mnTotalSize);
        const double nThumbPosition = ::std::min(::std::max(0.0,mnThumbPosition), mnTotalSize - nThumbSize);
        maBox[Thumb] = geometry::RealRectangle2D(
            0, nThumbPosition / mnTotalSize * nPagerHeight,
            aWindowBox.Width,
                (nThumbPosition+nThumbSize) / mnTotalSize * nPagerHeight);

        // Set up the enabled/disabled states.
        maEnabledState[PrevButton] = nThumbPosition>0;
        maEnabledState[PagerUp] = nThumbPosition>0;
        maEnabledState[NextButton] = nThumbPosition+nThumbSize < mnTotalSize;
        maEnabledState[PagerDown] = nThumbPosition+nThumbSize < mnTotalSize;
        maEnabledState[Thumb] = nThumbSize < mnTotalSize;
    }
    maBox[PagerUp] = geometry::RealRectangle2D(
        maBox[Pager].X1, maBox[Pager].Y1, maBox[Pager].X2, maBox[Thumb].Y1-1);
    maBox[PagerDown] = geometry::RealRectangle2D(
        maBox[Pager].X1, maBox[Thumb].Y2+1, maBox[Pager].X2, maBox[Pager].Y2);
    maBox[Total] = PresenterGeometryHelper::Union(
        PresenterGeometryHelper::Union(maBox[PrevButton], maBox[NextButton]),
        maBox[Pager]);
}

void PresenterVerticalScrollBar::UpdateBitmaps()
{
    if (mpBitmaps == nullptr)
        return;

    mpPrevButtonDescriptor = mpBitmaps->GetBitmap("Up");
    mpNextButtonDescriptor = mpBitmaps->GetBitmap("Down");
    mpPagerStartDescriptor = mpBitmaps->GetBitmap("PagerTop");
    mpPagerCenterDescriptor = mpBitmaps->GetBitmap("PagerVertical");
    mpPagerEndDescriptor = mpBitmaps->GetBitmap("PagerBottom");
    mpThumbStartDescriptor = mpBitmaps->GetBitmap("ThumbTop");
    mpThumbCenterDescriptor = mpBitmaps->GetBitmap("ThumbVertical");
    mpThumbEndDescriptor = mpBitmaps->GetBitmap("ThumbBottom");

    mnScrollBarWidth = 0;
    UpdateWidthOrHeight(mnScrollBarWidth, mpPrevButtonDescriptor);
    UpdateWidthOrHeight(mnScrollBarWidth, mpNextButtonDescriptor);
    UpdateWidthOrHeight(mnScrollBarWidth, mpPagerStartDescriptor);
    UpdateWidthOrHeight(mnScrollBarWidth, mpPagerCenterDescriptor);
    UpdateWidthOrHeight(mnScrollBarWidth, mpPagerEndDescriptor);
    UpdateWidthOrHeight(mnScrollBarWidth, mpThumbStartDescriptor);
    UpdateWidthOrHeight(mnScrollBarWidth, mpThumbCenterDescriptor);
    UpdateWidthOrHeight(mnScrollBarWidth, mpThumbEndDescriptor);
    if (mnScrollBarWidth == 0)
        mnScrollBarWidth = 20;
}

void PresenterVerticalScrollBar::PaintComposite(
    const css::awt::Rectangle& rUpdateBox,
    const Area eArea,
    const SharedBitmapDescriptor& rpStartBitmaps,
    const SharedBitmapDescriptor& rpCenterBitmaps,
    const SharedBitmapDescriptor& rpEndBitmaps)
{
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    geometry::RealRectangle2D aBox (GetRectangle(eArea));
    aBox.X1 += aWindowBox.X;
    aBox.Y1 += aWindowBox.Y;
    aBox.X2 += aWindowBox.X;
    aBox.Y2 += aWindowBox.Y;

    // Get bitmaps and sizes.

    PresenterUIPainter::PaintVerticalBitmapComposite(
        mxCanvas,
        rUpdateBox,
        (eArea == Thumb
            ? PresenterGeometryHelper::ConvertRectangleWithConstantSize(aBox)
            : PresenterGeometryHelper::ConvertRectangle(aBox)),
        GetBitmap(eArea, rpStartBitmaps),
        GetBitmap(eArea, rpCenterBitmaps),
        GetBitmap(eArea, rpEndBitmaps));
}

//===== PresenterScrollBar::MousePressRepeater ================================

PresenterScrollBar::MousePressRepeater::MousePressRepeater (
    const ::rtl::Reference<PresenterScrollBar>& rpScrollBar)
    : mnMousePressRepeaterTaskId(PresenterTimer::NotAValidTaskId),
      mpScrollBar(rpScrollBar),
      meMouseArea(PresenterScrollBar::None)
{
}

void PresenterScrollBar::MousePressRepeater::Dispose()
{
    Stop();
    mpScrollBar = nullptr;
}

void PresenterScrollBar::MousePressRepeater::Start (const PresenterScrollBar::Area& reArea)
{
    meMouseArea = reArea;

    if (mnMousePressRepeaterTaskId == PresenterTimer::NotAValidTaskId)
    {
        // Execute key press operation at least this one time.
        Execute();

        // Schedule repeated executions.
        auto pThis(shared_from_this());
        mnMousePressRepeaterTaskId = PresenterTimer::ScheduleRepeatedTask (
            mpScrollBar->GetComponentContext(),
            [pThis] (TimeValue const&) { return pThis->Callback(); },
            500000000,
            250000000);
    }
    else
    {
        // There is already an active repeating task.
    }
}

void PresenterScrollBar::MousePressRepeater::Stop()
{
    if (mnMousePressRepeaterTaskId != PresenterTimer::NotAValidTaskId)
    {
        const sal_Int32 nTaskId (mnMousePressRepeaterTaskId);
        mnMousePressRepeaterTaskId = PresenterTimer::NotAValidTaskId;
        PresenterTimer::CancelTask(nTaskId);
    }
}

void PresenterScrollBar::MousePressRepeater::SetMouseArea(const PresenterScrollBar::Area& reArea)
{
    if (meMouseArea != reArea)
    {
        if (mnMousePressRepeaterTaskId != PresenterTimer::NotAValidTaskId)
        {
            Stop();
        }
    }
}

void PresenterScrollBar::MousePressRepeater::Callback ()
{
    if (mpScrollBar.get() == nullptr)
    {
        Stop();
        return;
    }

    Execute();
}

void PresenterScrollBar::MousePressRepeater::Execute()
{
    const double nThumbPosition (mpScrollBar->GetThumbPosition());
    switch (meMouseArea)
    {
        case PrevButton:
            mpScrollBar->SetThumbPosition(nThumbPosition - mpScrollBar->GetLineHeight(), true);
            break;

        case NextButton:
            mpScrollBar->SetThumbPosition(nThumbPosition + mpScrollBar->GetLineHeight(), true);
            break;

        case PagerUp:
            mpScrollBar->SetThumbPosition(nThumbPosition - mpScrollBar->GetThumbSize()*0.8, true);
            break;

        case PagerDown:
            mpScrollBar->SetThumbPosition(nThumbPosition + mpScrollBar->GetThumbSize()*0.8, true);
            break;

        default:
            break;
    }
}

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
