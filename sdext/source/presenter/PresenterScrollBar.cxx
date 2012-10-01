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
#include "PresenterComponent.hxx"
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
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/weak_ptr.hpp>
#include <math.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(pString))

const static double gnScrollBarGap (10);

namespace sdext { namespace presenter {

//===== PresenterScrollBar::MousePressRepeater ================================

class PresenterScrollBar::MousePressRepeater
    : public ::boost::enable_shared_from_this<MousePressRepeater>
{
public:
    MousePressRepeater (const ::rtl::Reference<PresenterScrollBar>& rpScrollBar);
    void Dispose (void);
    void Start (const PresenterScrollBar::Area& reArea);
    void Stop (void);
    void SetMouseArea (const PresenterScrollBar::Area& reArea);

private:
    void Callback (const TimeValue& rCurrentTime);
    void Execute (void);

    sal_Int32 mnMousePressRepeaterTaskId;
    ::rtl::Reference<PresenterScrollBar> mpScrollBar;
    PresenterScrollBar::Area meMouseArea;
};

//===== PresenterScrollBar ====================================================

boost::weak_ptr<PresenterBitmapContainer> PresenterScrollBar::mpSharedBitmaps;

PresenterScrollBar::PresenterScrollBar (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<awt::XWindow>& rxParentWindow,
    const ::boost::shared_ptr<PresenterPaintManager>& rpPaintManager,
    const ::boost::function<void(double)>& rThumbMotionListener)
    : PresenterScrollBarInterfaceBase(m_aMutex),
      mxComponentContext(rxComponentContext),
      mxParentWindow(rxParentWindow),
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

        mxPresenterHelper = Reference<drawing::XPresenterHelper>(
            xFactory->createInstanceWithContext(
                OUString("com.sun.star.comp.Draw.PresenterHelper"),
                rxComponentContext),
            UNO_QUERY_THROW);

        if (mxPresenterHelper.is())
            mxWindow = mxPresenterHelper->createWindow(rxParentWindow,
                sal_False,
                sal_False,
                sal_False,
                sal_False);

        // Make the background transparent.  The slide show paints its own background.
        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY_THROW);
        if (xPeer.is())
        {
            xPeer->setBackground(0xff000000);
        }

        mxWindow->setVisible(sal_True);
        mxWindow->addWindowListener(this);
        mxWindow->addPaintListener(this);
        mxWindow->addMouseListener(this);
        mxWindow->addMouseMotionListener(this);
    }
    catch (RuntimeException&)
    {
    }
}

PresenterScrollBar::~PresenterScrollBar (void)
{
}

void SAL_CALL PresenterScrollBar::disposing (void)
{
    mpMousePressRepeater->Dispose();

    if (mxWindow.is())
    {
        mxWindow->removeWindowListener(this);
        mxWindow->removePaintListener(this);
        mxWindow->removeMouseListener(this);
        mxWindow->removeMouseMotionListener(this);

        Reference<lang::XComponent> xComponent (mxWindow, UNO_QUERY);
        mxWindow = NULL;
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
    SetThumbPosition(nPosition, bAsynchronousUpdate, true, true);
}

void PresenterScrollBar::SetThumbPosition (
    double nPosition,
    const bool bAsynchronousUpdate,
    const bool bValidate,
    const bool bNotify)
{
    if (bValidate)
        nPosition = ValidateThumbPosition(nPosition);

    if (nPosition != mnThumbPosition && ! mbIsNotificationActive)
    {
        mnThumbPosition = nPosition;

        UpdateBorders();
        Repaint(GetRectangle(Total), bAsynchronousUpdate);
        if (bNotify)
            NotifyThumbPositionChange();
    }
}

double PresenterScrollBar::GetThumbPosition (void) const
{
    return mnThumbPosition;
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

double PresenterScrollBar::GetThumbSize (void) const
{
    return mnThumbSize;
}

void PresenterScrollBar::SetLineHeight (const double nLineHeight)
{
    mnLineHeight = nLineHeight;
}

double PresenterScrollBar::GetLineHeight (void) const
{
    return mnLineHeight;
}

void PresenterScrollBar::SetCanvas (const Reference<css::rendering::XCanvas>& rxCanvas)
{
    if (mxCanvas != rxCanvas)
    {
        mxCanvas = rxCanvas;
        if (mxCanvas.is())
        {
            if (mpBitmaps.get()==NULL)
            {
                if (mpSharedBitmaps.expired())
                {
                    try
                    {
                        mpBitmaps.reset(new PresenterBitmapContainer(
                            OUString("PresenterScreenSettings/ScrollBar/Bitmaps"),
                            ::boost::shared_ptr<PresenterBitmapContainer>(),
                            mxComponentContext,
                            mxCanvas,
                            PresenterComponent::GetBasePath(mxComponentContext)));
                        mpSharedBitmaps = mpBitmaps;
                    }
                    catch(Exception&)
                    {
                        OSL_ASSERT(false);
                    }
                }
                else
                    mpBitmaps = ::boost::shared_ptr<PresenterBitmapContainer>(mpSharedBitmaps);
                UpdateBitmaps();
                UpdateBorders();
            }

            Repaint(GetRectangle(Total), false);
        }
    }
}

void PresenterScrollBar::SetBackground (const SharedBitmapDescriptor& rpBackgroundBitmap)
{
    mpBackgroundBitmap = rpBackgroundBitmap;
}

void PresenterScrollBar::CheckValues (void)
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
    const awt::Rectangle& rUpdateBox,
    const bool bNoClip)
{
    if ( ! mxCanvas.is() || ! mxWindow.is())
    {
        OSL_ASSERT(mxCanvas.is());
        OSL_ASSERT(mxWindow.is());
        return;
    }

    if ( ! bNoClip)
    {
        if (PresenterGeometryHelper::AreRectanglesDisjoint (rUpdateBox, mxWindow->getPosSize()))
            return;
    }

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
        xSpriteCanvas->updateScreen(sal_False);
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterScrollBar::windowResized (const css::awt::WindowEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}

void SAL_CALL PresenterScrollBar::windowMoved (const css::awt::WindowEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}

void SAL_CALL PresenterScrollBar::windowShown (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}

void SAL_CALL PresenterScrollBar::windowHidden (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterScrollBar::windowPaint (const css::awt::PaintEvent& rEvent)
    throw (css::uno::RuntimeException)
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
            xSpriteCanvas->updateScreen(sal_False);
    }
}

//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterScrollBar::mousePressed (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    maDragAnchor.X = rEvent.X;
    maDragAnchor.Y = rEvent.Y;
    meButtonDownArea = GetArea(rEvent.X, rEvent.Y);

    mpMousePressRepeater->Start(meButtonDownArea);
}

void SAL_CALL PresenterScrollBar::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;

    mpMousePressRepeater->Stop();

    if (mxPresenterHelper.is())
        mxPresenterHelper->releaseMouse(mxWindow);
}

void SAL_CALL PresenterScrollBar::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
}

void SAL_CALL PresenterScrollBar::mouseExited (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
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
    throw (css::uno::RuntimeException)
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
    throw (css::uno::RuntimeException)
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
        SetThumbPosition(mnThumbPosition + nDragDistance, false, true, true);
    }
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterScrollBar::disposing (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    if (rEvent.Source == mxWindow)
        mxWindow = NULL;
}

//-----------------------------------------------------------------------------

geometry::RealRectangle2D PresenterScrollBar::GetRectangle (const Area eArea) const
{
    OSL_ASSERT(eArea>=0 && eArea<__AreaCount__);

    return maBox[eArea];
}

void PresenterScrollBar::Repaint (
    const geometry::RealRectangle2D aBox,
    const bool bAsynchronousUpdate)
{
    if (mpPaintManager.get() != NULL)
        mpPaintManager->Invalidate(
            mxWindow,
            PresenterGeometryHelper::ConvertRectangle(aBox),
            bAsynchronousUpdate);
}

void PresenterScrollBar::PaintBackground(
    const css::awt::Rectangle& rUpdateBox)
{
    if (mpBackgroundBitmap.get() == NULL)
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

    if (xBitmap.is())
    {
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
            NULL,
            Sequence<double>(4),
            rendering::CompositeOperation::SOURCE);

        mxCanvas->drawBitmap(
            xBitmap,
            aViewState,
            aRenderState);
    }
}

void PresenterScrollBar::NotifyThumbPositionChange (void)
{
    if ( ! mbIsNotificationActive)
    {
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
    if (rpDescriptor.get() != NULL)
    {
        Reference<rendering::XBitmap> xBitmap (rpDescriptor->GetNormalBitmap());
        if (xBitmap.is())
        {
            const geometry::IntegerSize2D aBitmapSize (xBitmap->getSize());
            const sal_Int32 nBitmapSize = (sal_Int32)GetMinor(aBitmapSize.Width, aBitmapSize.Height);
            if (nBitmapSize > rSize)
                rSize = nBitmapSize;
        }
    }
}

css::uno::Reference<css::rendering::XBitmap> PresenterScrollBar::GetBitmap (
    const Area eArea,
    const SharedBitmapDescriptor& rpBitmaps) const
{
    if (rpBitmaps.get() == NULL)
        return NULL;
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
    OSL_ASSERT(eArea>=0 && eArea<__AreaCount__);

    return ! maEnabledState[eArea];
}

//===== PresenterVerticalScrollBar ============================================

PresenterVerticalScrollBar::PresenterVerticalScrollBar (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<awt::XWindow>& rxParentWindow,
    const ::boost::shared_ptr<PresenterPaintManager>& rpPaintManager,
    const ::boost::function<void(double)>& rThumbMotionListener)
    : PresenterScrollBar(rxComponentContext, rxParentWindow, rpPaintManager, rThumbMotionListener),
      mnScrollBarWidth(0)
{
}

PresenterVerticalScrollBar::~PresenterVerticalScrollBar (void)
{
}

double PresenterVerticalScrollBar::GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const
{
    (void)nX;
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

sal_Int32 PresenterVerticalScrollBar::GetSize (void) const
{
    return mnScrollBarWidth;
}

geometry::RealPoint2D PresenterVerticalScrollBar::GetPoint (
    const double nMajor, const double nMinor) const
{
    return geometry::RealPoint2D(nMinor, nMajor);
}

double PresenterVerticalScrollBar::GetMajor (const double nX, const double nY) const
{
    (void)nX;
    return nY;
}

double PresenterVerticalScrollBar::GetMinor (const double nX, const double nY) const
{
    (void)nY;
    return nX;
}

void PresenterVerticalScrollBar::UpdateBorders (void)
{
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    double nBottom = aWindowBox.Height;

    if (mpNextButtonDescriptor.get() != NULL)
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
    if (mpPrevButtonDescriptor.get() != NULL)
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

void PresenterVerticalScrollBar::UpdateBitmaps (void)
{
    if (mpBitmaps.get() != NULL)
    {
        mpPrevButtonDescriptor = mpBitmaps->GetBitmap(A2S("Up"));
        mpNextButtonDescriptor = mpBitmaps->GetBitmap(A2S("Down"));
        mpPagerStartDescriptor = mpBitmaps->GetBitmap(A2S("PagerTop"));
        mpPagerCenterDescriptor = mpBitmaps->GetBitmap(A2S("PagerVertical"));
        mpPagerEndDescriptor = mpBitmaps->GetBitmap(A2S("PagerBottom"));
        mpThumbStartDescriptor = mpBitmaps->GetBitmap(A2S("ThumbTop"));
        mpThumbCenterDescriptor = mpBitmaps->GetBitmap(A2S("ThumbVertical"));
        mpThumbEndDescriptor = mpBitmaps->GetBitmap(A2S("ThumbBottom"));

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

void PresenterScrollBar::MousePressRepeater::Dispose (void)
{
    Stop();
    mpScrollBar = NULL;
}

void PresenterScrollBar::MousePressRepeater::Start (const PresenterScrollBar::Area& reArea)
{
    meMouseArea = reArea;

    if (mnMousePressRepeaterTaskId == PresenterTimer::NotAValidTaskId)
    {
        // Execute key press operation at least this one time.
        Execute();

        // Schedule repeated executions.
        mnMousePressRepeaterTaskId = PresenterTimer::ScheduleRepeatedTask (
            ::boost::bind(&PresenterScrollBar::MousePressRepeater::Callback, shared_from_this(), _1),
            500000000,
            250000000);
    }
    else
    {
        // There is already an active repeating task.
    }
}

void PresenterScrollBar::MousePressRepeater::Stop (void)
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

void PresenterScrollBar::MousePressRepeater::Callback (const TimeValue& rCurrentTime)
{
    (void)rCurrentTime;

    if (mpScrollBar.get() == NULL)
    {
        Stop();
        return;
    }

    Execute();
}

void PresenterScrollBar::MousePressRepeater::Execute (void)
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
