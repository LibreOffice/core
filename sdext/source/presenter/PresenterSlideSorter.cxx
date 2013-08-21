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

#include "vcl/svapp.hxx"

#include "PresenterSlideSorter.hxx"
#include "PresenterButton.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterHelper.hxx"
#include "PresenterPaintManager.hxx"
#include "PresenterPaneBase.hxx"
#include "PresenterScrollBar.hxx"
#include "PresenterUIPainter.hxx"
#include "PresenterWindowManager.hxx"
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/drawing/XSlideSorterBase.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/util/Color.hpp>
#include <algorithm>
#include <math.h>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace {
    const static sal_Int32 gnVerticalGap (10);
    const static sal_Int32 gnVerticalBorder (10);
    const static sal_Int32 gnHorizontalGap (10);
    const static sal_Int32 gnHorizontalBorder (10);

    const static double gnMinimalPreviewWidth (200);
    const static double gnPreferredPreviewWidth (300);
    const static double gnMaximalPreviewWidth (400);
    const static sal_Int32 gnPreferredColumnCount (6);
    const static double gnMinimalHorizontalPreviewGap(15);
    const static double gnPreferredHorizontalPreviewGap(25);
    const static double gnMaximalHorizontalPreviewGap(50);
    const static double gnMinimalVerticalPreviewGap(15);
    const static double gnPreferredVerticalPreviewGap(25);
    const static double gnMaximalVerticalPreviewGap(50);

    const static sal_Int32 gnHorizontalLabelBorder (3);
    const static sal_Int32 gnHorizontalLabelPadding (5);

    const static sal_Int32 gnVerticalButtonPadding (gnVerticalGap);
}

namespace sdext { namespace presenter {

namespace {
    sal_Int32 round (const double nValue) { return sal::static_int_cast<sal_Int32>(0.5 + nValue); }
    sal_Int32 floor (const double nValue) { return sal::static_int_cast<sal_Int32>(nValue); }
}

//===== PresenterSlideSorter::Layout ==========================================

class PresenterSlideSorter::Layout
{
public:
    Layout (const ::rtl::Reference<PresenterScrollBar>& rpVerticalScrollBar);

    void Update (const geometry::RealRectangle2D& rBoundingBox, const double nSlideAspectRatio);
    void SetupVisibleArea (void);
    void UpdateScrollBars (void);
    bool IsScrollBarNeeded (const sal_Int32 nSlideCount);
    geometry::RealPoint2D GetLocalPosition (const geometry::RealPoint2D& rWindowPoint) const;
    geometry::RealPoint2D GetWindowPosition(const geometry::RealPoint2D& rLocalPoint) const;
    sal_Int32 GetColumn (const geometry::RealPoint2D& rLocalPoint,
        const bool bReturnInvalidValue = false) const;
    sal_Int32 GetRow (const geometry::RealPoint2D& rLocalPoint,
        const bool bReturnInvalidValue = false) const;
    sal_Int32 GetSlideIndexForPosition (const css::geometry::RealPoint2D& rPoint) const;
    css::geometry::RealPoint2D GetPoint (
        const sal_Int32 nSlideIndex,
        const sal_Int32 nRelativeHorizontalPosition,
        const sal_Int32 nRelativeVerticalPosition) const;
    css::awt::Rectangle GetBoundingBox (const sal_Int32 nSlideIndex) const;
    void ForAllVisibleSlides (const ::boost::function<void(sal_Int32)>& rAction);
    sal_Int32 GetFirstVisibleSlideIndex (void) const;
    sal_Int32 GetLastVisibleSlideIndex (void) const;
    bool SetHorizontalOffset (const double nOffset);
    bool SetVerticalOffset (const double nOffset);

    css::geometry::RealRectangle2D maBoundingBox;
    css::geometry::IntegerSize2D maPreviewSize;
    sal_Int32 mnHorizontalOffset;
    sal_Int32 mnVerticalOffset;
    sal_Int32 mnHorizontalGap;
    sal_Int32 mnVerticalGap;
    sal_Int32 mnHorizontalBorder;
    sal_Int32 mnVerticalBorder;
    sal_Int32 mnRowCount;
    sal_Int32 mnColumnCount;
    sal_Int32 mnSlideCount;
    sal_Int32 mnSlideIndexAtMouse;
    sal_Int32 mnFirstVisibleColumn;
    sal_Int32 mnLastVisibleColumn;
    sal_Int32 mnFirstVisibleRow;
    sal_Int32 mnLastVisibleRow;

private:
    ::rtl::Reference<PresenterScrollBar> mpVerticalScrollBar;

    sal_Int32 GetIndex (const sal_Int32 nRow, const sal_Int32 nColumn) const;
    sal_Int32 GetRow (const sal_Int32 nSlideIndex) const;
    sal_Int32 GetColumn (const sal_Int32 nSlideIndex) const;
};

//==== PresenterSlideSorter::MouseOverManager =================================

class PresenterSlideSorter::MouseOverManager
    : ::boost::noncopyable
{
public:
    MouseOverManager (
        const Reference<container::XIndexAccess>& rxSlides,
        const ::boost::shared_ptr<PresenterTheme>& rpTheme,
        const Reference<awt::XWindow>& rxInvalidateTarget,
        const ::boost::shared_ptr<PresenterPaintManager>& rpPaintManager);
    ~MouseOverManager (void);

    void Paint (
        const sal_Int32 nSlideIndex,
        const Reference<rendering::XCanvas>& rxCanvas,
        const Reference<rendering::XPolyPolygon2D>& rxClip);

    void SetSlide (
        const sal_Int32 nSlideIndex,
        const awt::Rectangle& rBox);

private:
    Reference<rendering::XCanvas> mxCanvas;
    const Reference<container::XIndexAccess> mxSlides;
    SharedBitmapDescriptor mpLeftLabelBitmap;
    SharedBitmapDescriptor mpCenterLabelBitmap;
    SharedBitmapDescriptor mpRightLabelBitmap;
    PresenterTheme::SharedFontDescriptor mpFont;
    sal_Int32 mnSlideIndex;
    awt::Rectangle maSlideBoundingBox;
    OUString msText;
    Reference<rendering::XBitmap> mxBitmap;
    Reference<awt::XWindow> mxInvalidateTarget;
    ::boost::shared_ptr<PresenterPaintManager> mpPaintManager;

    void SetCanvas (
        const Reference<rendering::XCanvas>& rxCanvas);
    /** Create a bitmap that shows the given text and is not wider than the
        given maximal width.
    */
    Reference<rendering::XBitmap> CreateBitmap (
        const OUString& rsText,
        const sal_Int32 nMaximalWidth) const;
    void Invalidate (void);
    geometry::IntegerSize2D CalculateLabelSize (
        const OUString& rsText) const;
    OUString GetFittingText (const OUString& rsText, const double nMaximalWidth) const;
    void PaintButtonBackground (
        const Reference<rendering::XBitmapCanvas>& rxCanvas,
        const geometry::IntegerSize2D& rSize) const;
};

//==== PresenterSlideSorter::CurrentSlideFrameRenderer ========================

class PresenterSlideSorter::CurrentSlideFrameRenderer
{
public:
    CurrentSlideFrameRenderer (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);
    ~CurrentSlideFrameRenderer (void);

    void PaintCurrentSlideFrame (
        const awt::Rectangle& rSlideBoundingBox,
        const Reference<rendering::XCanvas>& rxCanvas,
        const geometry::RealRectangle2D& rClipBox);

    /** Enlarge the given rectangle to include the current slide indicator.
    */
    awt::Rectangle GetBoundingBox (
        const awt::Rectangle& rSlideBoundingBox);

private:
    SharedBitmapDescriptor mpTopLeft;
    SharedBitmapDescriptor mpTop;
    SharedBitmapDescriptor mpTopRight;
    SharedBitmapDescriptor mpLeft;
    SharedBitmapDescriptor mpRight;
    SharedBitmapDescriptor mpBottomLeft;
    SharedBitmapDescriptor mpBottom;
    SharedBitmapDescriptor mpBottomRight;
    sal_Int32 mnTopFrameSize;
    sal_Int32 mnLeftFrameSize;
    sal_Int32 mnRightFrameSize;
    sal_Int32 mnBottomFrameSize;

    void PaintBitmapOnce(
        const css::uno::Reference<css::rendering::XBitmap>& rxBitmap,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const Reference<rendering::XPolyPolygon2D>& rxClip,
        const double nX,
        const double nY);
    void PaintBitmapTiled(
        const css::uno::Reference<css::rendering::XBitmap>& rxBitmap,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const geometry::RealRectangle2D& rClipBox,
        const double nX,
        const double nY,
        const double nWidth,
        const double nHeight);
};

//===== PresenterSlideSorter ==================================================

PresenterSlideSorter::PresenterSlideSorter (
    const Reference<uno::XComponentContext>& rxContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterSlideSorterInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mxViewId(rxViewId),
      mxPane(),
      mxCanvas(),
      mxWindow(),
      mpPresenterController(rpPresenterController),
      mxSlideShowController(mpPresenterController->GetSlideShowController()),
      mxPreviewCache(),
      mbIsPaintPending(true),
      mbIsLayoutPending(true),
      mpLayout(),
      mpVerticalScrollBar(),
      mpCloseButton(),
      mpMouseOverManager(),
      mnSlideIndexMousePressed(-1),
      mnCurrentSlideIndex(-1),
      mnSeparatorY(0),
      maSeparatorColor(0x00ffffff),
      maCloseButtonCenter(),
      maCurrentSlideFrameBoundingBox(),
      mpCurrentSlideFrameRenderer(),
      mxPreviewFrame()
{
    if ( ! rxContext.is()
        || ! rxViewId.is()
        || ! rxController.is()
        || rpPresenterController.get()==NULL)
    {
        throw lang::IllegalArgumentException();
    }

    if ( ! mxSlideShowController.is())
        throw RuntimeException();

    try
    {
        // Get pane and window.
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        Reference<XConfigurationController> xCC (
            xCM->getConfigurationController(), UNO_QUERY_THROW);
        Reference<lang::XMultiComponentFactory> xFactory (
            mxComponentContext->getServiceManager(), UNO_QUERY_THROW);

        mxPane = Reference<XPane>(xCC->getResource(rxViewId->getAnchor()), UNO_QUERY_THROW);
        mxWindow = mxPane->getWindow();

        // Add window listener.
        mxWindow->addWindowListener(this);
        mxWindow->addPaintListener(this);
        mxWindow->addMouseListener(this);
        mxWindow->addMouseMotionListener(this);
        mxWindow->setVisible(sal_True);

        // Remember the current slide.
        mnCurrentSlideIndex = mxSlideShowController->getCurrentSlideIndex();

        // Create the scroll bar.
        mpVerticalScrollBar = ::rtl::Reference<PresenterScrollBar>(
            new PresenterVerticalScrollBar(
                rxContext,
                mxWindow,
                mpPresenterController->GetPaintManager(),
                ::boost::bind(&PresenterSlideSorter::SetVerticalOffset,this,_1)));

        mpCloseButton = PresenterButton::Create(
            rxContext,
            mpPresenterController,
            mpPresenterController->GetTheme(),
            mxWindow,
            mxCanvas,
            "SlideSorterCloser");

        if (mpPresenterController->GetTheme().get() != NULL)
        {
            PresenterTheme::SharedFontDescriptor pFont (
                mpPresenterController->GetTheme()->GetFont("ButtonFont"));
            if (pFont.get() != NULL)
                maSeparatorColor = pFont->mnColor;
        }

        // Create the layout.
        mpLayout.reset(new Layout(mpVerticalScrollBar));

        // Create the preview cache.
        mxPreviewCache = Reference<drawing::XSlidePreviewCache>(
            xFactory->createInstanceWithContext(
                OUString("com.sun.star.drawing.PresenterPreviewCache"),
                mxComponentContext),
            UNO_QUERY_THROW);
        Reference<container::XIndexAccess> xSlides (mxSlideShowController, UNO_QUERY);
        mxPreviewCache->setDocumentSlides(xSlides, rxController->getModel());
        mxPreviewCache->addPreviewCreationNotifyListener(this);
        if (xSlides.is())
        {
            mpLayout->mnSlideCount = xSlides->getCount();
        }

        // Create the mouse over manager.
        mpMouseOverManager.reset(new MouseOverManager(
            Reference<container::XIndexAccess>(mxSlideShowController, UNO_QUERY),
            mpPresenterController->GetTheme(),
            mxWindow,
            mpPresenterController->GetPaintManager()));

        // Listen for changes of the current slide.
        Reference<beans::XPropertySet> xControllerProperties (rxController, UNO_QUERY_THROW);
        xControllerProperties->addPropertyChangeListener(
            OUString("CurrentPage"),
            this);

        // Move the current slide in the center of the window.
        const awt::Rectangle aCurrentSlideBBox (mpLayout->GetBoundingBox(mnCurrentSlideIndex));
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        SetHorizontalOffset(aCurrentSlideBBox.X - aWindowBox.Width/2.0);
    }
    catch (RuntimeException&)
    {
        disposing();
        throw;
    }
}

PresenterSlideSorter::~PresenterSlideSorter (void)
{
}

void SAL_CALL PresenterSlideSorter::disposing (void)
{
    mxComponentContext = NULL;
    mxViewId = NULL;
    mxPane = NULL;

    if (mpVerticalScrollBar.is())
    {
        Reference<lang::XComponent> xComponent (
            static_cast<XWeak*>(mpVerticalScrollBar.get()), UNO_QUERY);
        mpVerticalScrollBar = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }
    if (mpCloseButton.is())
    {
        Reference<lang::XComponent> xComponent (
            static_cast<XWeak*>(mpCloseButton.get()), UNO_QUERY);
        mpCloseButton = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    if (mxCanvas.is())
    {
        Reference<lang::XComponent> xComponent (mxCanvas, UNO_QUERY);
        if (xComponent.is())
            xComponent->removeEventListener(static_cast<awt::XWindowListener*>(this));
        mxCanvas = NULL;
    }
    mpPresenterController = NULL;
    mxSlideShowController = NULL;
    mpLayout.reset();
    mpMouseOverManager.reset();

    if (mxPreviewCache.is())
    {
        mxPreviewCache->removePreviewCreationNotifyListener(this);

        Reference<XComponent> xComponent (mxPreviewCache, UNO_QUERY);
        mxPreviewCache = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    if (mxWindow.is())
    {
        mxWindow->removeWindowListener(this);
        mxWindow->removePaintListener(this);
        mxWindow->removeMouseListener(this);
        mxWindow->removeMouseMotionListener(this);
    }
}

void PresenterSlideSorter::SetActiveState (const bool bIsActive)
{
    (void)bIsActive;
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterSlideSorter::disposing (const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (rEventObject.Source == mxWindow)
    {
        mxWindow = NULL;
        dispose();
    }
    else if (rEventObject.Source == mxPreviewCache)
    {
        mxPreviewCache = NULL;
        dispose();
    }
    else if (rEventObject.Source == mxCanvas)
    {
        mxCanvas = NULL;
        mbIsLayoutPending = true;
        mbIsPaintPending = true;

        mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
    }
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterSlideSorter::windowResized (const awt::WindowEvent& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    mbIsLayoutPending = true;
    mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
}

void SAL_CALL PresenterSlideSorter::windowMoved (const awt::WindowEvent& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}

void SAL_CALL PresenterSlideSorter::windowShown (const lang::EventObject& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    mbIsLayoutPending = true;
    mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
}

void SAL_CALL PresenterSlideSorter::windowHidden (const lang::EventObject& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterSlideSorter::windowPaint (const css::awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;

    // Deactivated views must not be painted.
    if ( ! mbIsPresenterViewActive)
        return;

    Paint(rEvent.UpdateRect);

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}

//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterSlideSorter::mousePressed (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    css::awt::MouseEvent rTemp =rEvent;
    /// check whether RTL interface or not
    if(Application::GetSettings().GetLayoutRTL()){
        awt::Rectangle aBox = mxWindow->getPosSize();
        rTemp.X=aBox.Width-rEvent.X;
    }
    const geometry::RealPoint2D aPosition(rTemp.X, rEvent.Y);
    mnSlideIndexMousePressed = mpLayout->GetSlideIndexForPosition(aPosition);
}

void SAL_CALL PresenterSlideSorter::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    css::awt::MouseEvent rTemp =rEvent;
    /// check whether RTL interface or not
    if(Application::GetSettings().GetLayoutRTL()){
        awt::Rectangle aBox = mxWindow->getPosSize();
        rTemp.X=aBox.Width-rEvent.X;
    }
    const geometry::RealPoint2D aPosition(rTemp.X, rEvent.Y);
    const sal_Int32 nSlideIndex (mpLayout->GetSlideIndexForPosition(aPosition));

    if (nSlideIndex == mnSlideIndexMousePressed && mnSlideIndexMousePressed >= 0)
    {
        switch (rEvent.ClickCount)
        {
            case 1:
            default:
                GotoSlide(nSlideIndex);
                break;

            case 2:
                OSL_ASSERT(mpPresenterController.get()!=NULL);
                OSL_ASSERT(mpPresenterController->GetWindowManager().get()!=NULL);
                mpPresenterController->GetWindowManager()->SetSlideSorterState(false);
                GotoSlide(nSlideIndex);
                break;
        }
    }
}

void SAL_CALL PresenterSlideSorter::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
}

void SAL_CALL PresenterSlideSorter::mouseExited (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    mnSlideIndexMousePressed = -1;
    if (mpMouseOverManager.get() != NULL)
        mpMouseOverManager->SetSlide(mnSlideIndexMousePressed, awt::Rectangle(0,0,0,0));
}

//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterSlideSorter::mouseMoved (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    if (mpMouseOverManager.get() != NULL)
    {
        css::awt::MouseEvent rTemp =rEvent;
        /// check whether RTL interface or not
        if(Application::GetSettings().GetLayoutRTL()){
            awt::Rectangle aBox = mxWindow->getPosSize();
            rTemp.X=aBox.Width-rEvent.X;
        }
        const geometry::RealPoint2D aPosition(rTemp.X, rEvent.Y);
        sal_Int32 nSlideIndex (mpLayout->GetSlideIndexForPosition(aPosition));

        if (nSlideIndex < 0)
            mnSlideIndexMousePressed = -1;

        if (nSlideIndex < 0)
        {
            mpMouseOverManager->SetSlide(nSlideIndex, awt::Rectangle(0,0,0,0));
        }
        else
        {
            mpMouseOverManager->SetSlide(
                nSlideIndex,
                mpLayout->GetBoundingBox(nSlideIndex));
        }
    }
}

void SAL_CALL PresenterSlideSorter::mouseDragged (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}

//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterSlideSorter::getResourceId (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return mxViewId;
}

sal_Bool SAL_CALL PresenterSlideSorter::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}

//----- XPropertyChangeListener -----------------------------------------------

void SAL_CALL PresenterSlideSorter::propertyChange (
    const css::beans::PropertyChangeEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
}

//----- XSlidePreviewCacheListener --------------------------------------------

void SAL_CALL PresenterSlideSorter::notifyPreviewCreation (
    sal_Int32 nSlideIndex)
    throw(css::uno::RuntimeException)
{
    OSL_ASSERT(mpLayout.get()!=NULL);

    awt::Rectangle aBBox (mpLayout->GetBoundingBox(nSlideIndex));
    mpPresenterController->GetPaintManager()->Invalidate(mxWindow, aBBox, true);
}

//----- XDrawView -------------------------------------------------------------

void SAL_CALL PresenterSlideSorter::setCurrentPage (const Reference<drawing::XDrawPage>& rxSlide)
    throw (RuntimeException)
{
    (void)rxSlide;

    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    if (mxSlideShowController.is())
    {
        const sal_Int32 nNewCurrentSlideIndex (mxSlideShowController->getCurrentSlideIndex());
        if (nNewCurrentSlideIndex != mnCurrentSlideIndex)
        {
            mnCurrentSlideIndex = nNewCurrentSlideIndex;

            // Request a repaint of the previous current slide to hide its
            // current slide indicator.
            mpPresenterController->GetPaintManager()->Invalidate(
                mxWindow,
                maCurrentSlideFrameBoundingBox);

            // Request a repaint of the new current slide to show its
            // current slide indicator.
            maCurrentSlideFrameBoundingBox = mpCurrentSlideFrameRenderer->GetBoundingBox(
                mpLayout->GetBoundingBox(mnCurrentSlideIndex));
            mpPresenterController->GetPaintManager()->Invalidate(
                mxWindow,
                maCurrentSlideFrameBoundingBox);
        }
    }
}

Reference<drawing::XDrawPage> SAL_CALL PresenterSlideSorter::getCurrentPage (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return NULL;
}

//-----------------------------------------------------------------------------

void PresenterSlideSorter::UpdateLayout (void)
{
    if ( ! mxWindow.is())
        return;

    mbIsLayoutPending = false;
    mbIsPaintPending = true;

    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    awt::Rectangle aCenterBox (aWindowBox);
    sal_Int32 nLeftBorderWidth (aWindowBox.X);

    // Get border width.
    PresenterPaneContainer::SharedPaneDescriptor pPane (
        mpPresenterController->GetPaneContainer()->FindViewURL(
            mxViewId->getResourceURL()));
    do
    {
        if (pPane.get() == NULL)
            break;
        if ( ! pPane->mxPane.is())
            break;

        Reference<drawing::framework::XPaneBorderPainter> xBorderPainter (
            pPane->mxPane->GetPaneBorderPainter());
        if ( ! xBorderPainter.is())
            break;
        aCenterBox = xBorderPainter->addBorder (
            mxViewId->getAnchor()->getResourceURL(),
            awt::Rectangle(0, 0, aWindowBox.Width, aWindowBox.Height),
            drawing::framework::BorderType_INNER_BORDER);
    }
    while(false);

    // Place vertical separator.
    mnSeparatorY = aWindowBox.Height - mpCloseButton->GetSize().Height - gnVerticalButtonPadding;

    PlaceCloseButton(pPane, aWindowBox, nLeftBorderWidth);

    geometry::RealRectangle2D aUpperBox(
        gnHorizontalBorder,
        gnVerticalBorder,
        aWindowBox.Width - 2*gnHorizontalBorder,
        mnSeparatorY - gnVerticalGap);

    // Determine whether the scroll bar has to be displayed.
    aUpperBox = PlaceScrollBars(aUpperBox);

    mpLayout->Update(aUpperBox, GetSlideAspectRatio());
    mpLayout->SetupVisibleArea();
    mpLayout->UpdateScrollBars();

    // Tell the preview cache about some of the values.
    mxPreviewCache->setPreviewSize(mpLayout->maPreviewSize);
    mxPreviewCache->setVisibleRange(
        mpLayout->GetFirstVisibleSlideIndex(),
        mpLayout->GetLastVisibleSlideIndex());

    // Clear the frame polygon so that it is re-created on the next paint.
    mxPreviewFrame = NULL;
}

geometry::RealRectangle2D PresenterSlideSorter::PlaceScrollBars (
    const geometry::RealRectangle2D& rUpperBox)
{
    mpLayout->Update(rUpperBox, GetSlideAspectRatio());
    bool bIsScrollBarNeeded (false);
    Reference<container::XIndexAccess> xSlides (mxSlideShowController, UNO_QUERY_THROW);
    if (xSlides.is())
        bIsScrollBarNeeded = mpLayout->IsScrollBarNeeded(xSlides->getCount());
    if (mpVerticalScrollBar.get() != NULL)
        {
            if (bIsScrollBarNeeded)
                {
                    if(Application::GetSettings().GetLayoutRTL())
                        {
                            mpVerticalScrollBar->SetPosSize(geometry::RealRectangle2D(
                                                                                      rUpperBox.X1,
                                                                                      rUpperBox.Y1,
                                                                                      rUpperBox.X1 +  mpVerticalScrollBar->GetSize(),
                                                                                      rUpperBox.Y2));
                            mpVerticalScrollBar->SetVisible(true);
                            // Reduce area covered by the scroll bar from the available
                            // space.
                            return geometry::RealRectangle2D(
                                                             rUpperBox.X1 + gnHorizontalGap + mpVerticalScrollBar->GetSize(),
                                                             rUpperBox.Y1,
                                                             rUpperBox.X2,
                                                             rUpperBox.Y2);
                        }
                    else
                        {
                            // if its not RTL place vertical scroll bar at right border.
                            mpVerticalScrollBar->SetPosSize(geometry::RealRectangle2D(
                                                                                      rUpperBox.X2 - mpVerticalScrollBar->GetSize(),
                                                                                      rUpperBox.Y1,
                                                                                      rUpperBox.X2,
                                                                                      rUpperBox.Y2));
                            mpVerticalScrollBar->SetVisible(true);
                            // Reduce area covered by the scroll bar from the available
                            // space.
                            return geometry::RealRectangle2D(
                                                             rUpperBox.X1,
                                                             rUpperBox.Y1,
                                                             rUpperBox.X2 - mpVerticalScrollBar->GetSize() - gnHorizontalGap,
                                                             rUpperBox.Y2);
                        }
                }
            else
                mpVerticalScrollBar->SetVisible(false);
        }
    return rUpperBox;
}

void PresenterSlideSorter::PlaceCloseButton (
    const PresenterPaneContainer::SharedPaneDescriptor& rpPane,
    const awt::Rectangle& rCenterBox,
    const sal_Int32 nLeftBorderWidth)
{
    // Place button.  When the callout is near the center then the button is
    // centered over the callout.  Otherwise it is centered with respect to
    // the whole window.
    sal_Int32 nCloseButtonCenter (rCenterBox.Width/2);
    if (rpPane.get() != NULL && rpPane->mxPane.is())
    {
        const sal_Int32 nCalloutCenter (rpPane->mxPane->GetCalloutAnchor().X - nLeftBorderWidth);
        const sal_Int32 nDistanceFromWindowCenter (abs(nCalloutCenter - rCenterBox.Width/2));
        const sal_Int32 nButtonWidth (mpCloseButton->GetSize().Width);
        const static sal_Int32 nMaxDistanceForCalloutCentering (nButtonWidth * 2);
        if (nDistanceFromWindowCenter < nMaxDistanceForCalloutCentering)
        {
            if (nCalloutCenter < nButtonWidth/2)
                nCloseButtonCenter = nButtonWidth/2;
            else if (nCalloutCenter > rCenterBox.Width-nButtonWidth/2)
                nCloseButtonCenter = rCenterBox.Width-nButtonWidth/2;
            else
                nCloseButtonCenter = nCalloutCenter;
        }
    }
    mpCloseButton->SetCenter(geometry::RealPoint2D(
        nCloseButtonCenter,
        rCenterBox.Height - mpCloseButton->GetSize().Height/ 2));
}

void PresenterSlideSorter::ClearBackground (
    const Reference<rendering::XCanvas>& rxCanvas,
    const awt::Rectangle& rUpdateBox)
{
    OSL_ASSERT(rxCanvas.is());

    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    mpPresenterController->GetCanvasHelper()->Paint(
        mpPresenterController->GetViewBackground(mxViewId->getResourceURL()),
        rxCanvas,
        rUpdateBox,
        awt::Rectangle(0,0,aWindowBox.Width,aWindowBox.Height),
        awt::Rectangle());
}

double PresenterSlideSorter::GetSlideAspectRatio (void) const
{
    double nSlideAspectRatio (28.0/21.0);

    try
    {
        Reference<container::XIndexAccess> xSlides(mxSlideShowController, UNO_QUERY_THROW);
        if (mxSlideShowController.is() && xSlides->getCount()>0)
        {
            Reference<beans::XPropertySet> xProperties(xSlides->getByIndex(0),UNO_QUERY_THROW);
            sal_Int32 nWidth (28000);
            sal_Int32 nHeight (21000);
            if ((xProperties->getPropertyValue("Width") >>= nWidth)
                && (xProperties->getPropertyValue("Height") >>= nHeight)
                && nHeight > 0)
            {
                nSlideAspectRatio = double(nWidth) / double(nHeight);
            }
        }
    }
    catch (RuntimeException&)
    {
        OSL_ASSERT(false);
    }

    return nSlideAspectRatio;
}

Reference<rendering::XBitmap> PresenterSlideSorter::GetPreview (const sal_Int32 nSlideIndex)
{
    if (nSlideIndex < 0 || nSlideIndex>=mpLayout->mnSlideCount)
        return NULL;
    else if (mxPane.is())
        return mxPreviewCache->getSlidePreview(nSlideIndex, mxPane->getCanvas());
    else
        return NULL;
}

void PresenterSlideSorter::PaintPreview (
    const Reference<rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rUpdateBox,
    const sal_Int32 nSlideIndex)
{
    OSL_ASSERT(rxCanvas.is());

    geometry::IntegerSize2D aSize (mpLayout->maPreviewSize);

    if (PresenterGeometryHelper::AreRectanglesDisjoint(
        rUpdateBox,
        mpLayout->GetBoundingBox(nSlideIndex)))
    {
        return;
    }

    Reference<rendering::XBitmap> xPreview (GetPreview(nSlideIndex));
    bool isRTL = Application::GetSettings().GetLayoutRTL();

    const geometry::RealPoint2D aTopLeft (
                                          mpLayout->GetWindowPosition(
                                                                      mpLayout->GetPoint(nSlideIndex, isRTL?1:-1, -1)));

    PresenterBitmapContainer aContainer (
        "PresenterScreenSettings/ScrollBar/Bitmaps",
        ::boost::shared_ptr<PresenterBitmapContainer>(),
        mxComponentContext,
        rxCanvas);
    Reference<container::XIndexAccess> xIndexAccess(mxSlideShowController, UNO_QUERY);
    Reference<drawing::XDrawPage> xPage = Reference<drawing::XDrawPage>(
        xIndexAccess->getByIndex(nSlideIndex), UNO_QUERY);
    bool bTransition = mpPresenterController->HasTransition(xPage);
    bool bCustomAnimation = mpPresenterController->HasCustomAnimation(xPage);

    // Create clip rectangle as intersection of the current update area and
    // the bounding box of all previews.
    geometry::RealRectangle2D aBoundingBox (mpLayout->maBoundingBox);
    aBoundingBox.Y2 += 1;
    const geometry::RealRectangle2D aClipBox (
        PresenterGeometryHelper::Intersection(
            PresenterGeometryHelper::ConvertRectangle(rUpdateBox),
            aBoundingBox));
    Reference<rendering::XPolyPolygon2D> xClip (
        PresenterGeometryHelper::CreatePolygon(aClipBox, rxCanvas->getDevice()));

    const rendering::ViewState aViewState (geometry::AffineMatrix2D(1,0,0, 0,1,0), xClip);

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(
            1, 0, aTopLeft.X,
            0, 1, aTopLeft.Y),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    // Emphasize the current slide.
    if (nSlideIndex == mnCurrentSlideIndex)
    {
        if (mpCurrentSlideFrameRenderer.get() != NULL)
        {
            const awt::Rectangle aSlideBoundingBox(
                sal::static_int_cast<sal_Int32>(0.5 + aTopLeft.X),
                sal::static_int_cast<sal_Int32>(0.5 + aTopLeft.Y),
                aSize.Width,
                aSize.Height);
            maCurrentSlideFrameBoundingBox
                = mpCurrentSlideFrameRenderer->GetBoundingBox(aSlideBoundingBox);
            mpCurrentSlideFrameRenderer->PaintCurrentSlideFrame (
                aSlideBoundingBox,
                mxCanvas,
                aClipBox);
        }
    }

    // Paint the preview.
    if (xPreview.is())
    {
        aSize = xPreview->getSize();
        if (aSize.Width > 0 && aSize.Height > 0)
        {
            rxCanvas->drawBitmap(xPreview, aViewState, aRenderState);
            if( bCustomAnimation )
            {
                const awt::Rectangle aAnimationPreviewBox(aTopLeft.X+3, aTopLeft.Y+aSize.Height-40, 0, 0);
                SharedBitmapDescriptor aAnimationDescriptor = aContainer.GetBitmap("Animation");
                Reference<rendering::XBitmap> xAnimationIcon (aAnimationDescriptor->GetNormalBitmap());
                rendering::RenderState aAnimationRenderState (
                    geometry::AffineMatrix2D(
                    1, 0, aAnimationPreviewBox.X,
                    0, 1, aAnimationPreviewBox.Y),
                    NULL,
                    Sequence<double>(4),
                    rendering::CompositeOperation::SOURCE);
                    rxCanvas->drawBitmap(xAnimationIcon, aViewState, aAnimationRenderState);
            }
            if( bTransition )
            {
                const awt::Rectangle aTransitionPreviewBox(aTopLeft.X+3, aTopLeft.Y+aSize.Height-20, 0, 0);
                SharedBitmapDescriptor aTransitionDescriptor = aContainer.GetBitmap("Transition");
                Reference<rendering::XBitmap> xTransitionIcon (aTransitionDescriptor->GetNormalBitmap());
                rendering::RenderState aTransitionRenderState (
                    geometry::AffineMatrix2D(
                    1, 0, aTransitionPreviewBox.X,
                    0, 1, aTransitionPreviewBox.Y),
                    NULL,
                    Sequence<double>(4),
                    rendering::CompositeOperation::SOURCE);
                    rxCanvas->drawBitmap(xTransitionIcon, aViewState, aTransitionRenderState);
            }
        }
    }

    // Create a polygon that is used to paint a frame around previews.  Its
    // coordinates are chosen in the local coordinate system of a preview.
    if ( ! mxPreviewFrame.is())
        mxPreviewFrame = PresenterGeometryHelper::CreatePolygon(
            awt::Rectangle(-1, -1, aSize.Width+2, aSize.Height+2),
            rxCanvas->getDevice());

    // Paint a border around the preview.
    if (mxPreviewFrame.is())
    {
        const geometry::RealRectangle2D aBox (0, 0, aSize.Width, aSize.Height);
        const util::Color aFrameColor (0x00000000);
        PresenterCanvasHelper::SetDeviceColor(aRenderState, aFrameColor);
        rxCanvas->drawPolyPolygon(mxPreviewFrame, aViewState, aRenderState);
    }

    // Paint mouse over effect.
    mpMouseOverManager->Paint(nSlideIndex, mxCanvas, xClip);
}

void PresenterSlideSorter::Paint (const awt::Rectangle& rUpdateBox)
{
    const bool bCanvasChanged ( ! mxCanvas.is());
    if ( ! ProvideCanvas())
        return;

    if (mpLayout->mnRowCount<=0 || mpLayout->mnColumnCount<=0)
    {
        OSL_ASSERT(mpLayout->mnRowCount>0 || mpLayout->mnColumnCount>0);
        return;
    }

    mbIsPaintPending = false;

    ClearBackground(mxCanvas, rUpdateBox);

    // Give the canvas to the controls.
    if (bCanvasChanged)
    {
        if (mpVerticalScrollBar.is())
            mpVerticalScrollBar->SetCanvas(mxCanvas);
        if (mpCloseButton.is())
            mpCloseButton->SetCanvas(mxCanvas, mxWindow);
    }

    // Now that the controls have a canvas we can do the layouting.
    if (mbIsLayoutPending)
        UpdateLayout();

    // Paint the horizontal separator.
    rendering::RenderState aRenderState (geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL, Sequence<double>(4), rendering::CompositeOperation::SOURCE);
    PresenterCanvasHelper::SetDeviceColor(aRenderState, maSeparatorColor);
    mxCanvas->drawLine(
        geometry::RealPoint2D(0, mnSeparatorY),
        geometry::RealPoint2D(mxWindow->getPosSize().Width, mnSeparatorY),
        rendering::ViewState(geometry::AffineMatrix2D(1,0,0, 0,1,0), NULL),
        aRenderState);

    // Paint the slides.
    if ( ! PresenterGeometryHelper::AreRectanglesDisjoint(
        rUpdateBox,
        PresenterGeometryHelper::ConvertRectangle(mpLayout->maBoundingBox)))
    {
        mpLayout->ForAllVisibleSlides(
            ::boost::bind(&PresenterSlideSorter::PaintPreview, this, mxCanvas, rUpdateBox, _1));
    }

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}

void PresenterSlideSorter::SetHorizontalOffset (const double nXOffset)
{
    if (mpLayout->SetHorizontalOffset(nXOffset))
    {
        mxPreviewCache->setVisibleRange(
            mpLayout->GetFirstVisibleSlideIndex(),
            mpLayout->GetLastVisibleSlideIndex());

        mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
    }
}

void PresenterSlideSorter::SetVerticalOffset (const double nYOffset)
{
    if (mpLayout->SetVerticalOffset(nYOffset))
    {
        mxPreviewCache->setVisibleRange(
            mpLayout->GetFirstVisibleSlideIndex(),
            mpLayout->GetLastVisibleSlideIndex());

        mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
    }
}

void PresenterSlideSorter::GotoSlide (const sal_Int32 nSlideIndex)
{
    mxSlideShowController->gotoSlideIndex(nSlideIndex);
}

bool PresenterSlideSorter::ProvideCanvas (void)
{
    if ( ! mxCanvas.is())
    {
        if (mxPane.is())
            mxCanvas = mxPane->getCanvas();

        // Register as event listener so that we are informed when the
        // canvas is disposed (and we have to fetch another one).
        Reference<lang::XComponent> xComponent (mxCanvas, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(static_cast<awt::XWindowListener*>(this));

        mpCurrentSlideFrameRenderer.reset(
            new CurrentSlideFrameRenderer(mxComponentContext, mxCanvas));
    }
    return mxCanvas.is();
}

void PresenterSlideSorter::ThrowIfDisposed (void)
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(
                "PresenterSlideSorter has been already disposed"),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

//===== PresenterSlideSorter::Layout ==========================================

PresenterSlideSorter::Layout::Layout (
    const ::rtl::Reference<PresenterScrollBar>& rpVerticalScrollBar)
    : maBoundingBox(),
      maPreviewSize(),
      mnHorizontalOffset(0),
      mnVerticalOffset(0),
      mnHorizontalGap(0),
      mnVerticalGap(0),
      mnHorizontalBorder(0),
      mnVerticalBorder(0),
      mnRowCount(1),
      mnColumnCount(1),
      mnSlideCount(0),
      mnSlideIndexAtMouse(-1),
      mnFirstVisibleColumn(-1),
      mnLastVisibleColumn(-1),
      mnFirstVisibleRow(-1),
      mnLastVisibleRow(-1),
      mpVerticalScrollBar(rpVerticalScrollBar)
{
}

void PresenterSlideSorter::Layout::Update (
    const geometry::RealRectangle2D& rBoundingBox,
    const double nSlideAspectRatio)
{
    maBoundingBox = rBoundingBox;

    mnHorizontalBorder = gnHorizontalBorder;
    mnVerticalBorder = gnVerticalBorder;

    const double nWidth (rBoundingBox.X2 - rBoundingBox.X1 - 2*mnHorizontalBorder);
    const double nHeight (rBoundingBox.Y2 - rBoundingBox.Y1 - 2*mnVerticalBorder);
    if (nWidth<=0 || nHeight<=0)
        return;

    double nPreviewWidth;

    // Determine column count, preview width, and horizontal gap (borders
    // are half the gap).  Try to use the preferred values.  Try more to
    // stay in the valid intervalls.  This last constraint may be not
    // fullfilled in some cases.
    const double nElementWidth = nWidth / gnPreferredColumnCount;
    if (nElementWidth < gnMinimalPreviewWidth + gnMinimalHorizontalPreviewGap)
    {
        // The preferred column count is too large.
        // Can we use the preferred preview width?
        if (nWidth - gnMinimalHorizontalPreviewGap >= gnPreferredPreviewWidth)
        {
            // Yes.
            nPreviewWidth = gnPreferredPreviewWidth;
            mnColumnCount = floor((nWidth+gnPreferredHorizontalPreviewGap)
                / (nPreviewWidth+gnPreferredHorizontalPreviewGap));
            mnHorizontalGap = round((nWidth - mnColumnCount*nPreviewWidth) / mnColumnCount);
        }
        else
        {
            // No.  Set the column count to 1 and adapt preview width and
            // gap.
            mnColumnCount = 1;
            mnHorizontalGap = floor(gnMinimalHorizontalPreviewGap);
            if (nWidth - gnMinimalHorizontalPreviewGap >= gnPreferredPreviewWidth)
                nPreviewWidth = nWidth - gnMinimalHorizontalPreviewGap;
            else
                nPreviewWidth = ::std::max(gnMinimalPreviewWidth, nWidth-mnHorizontalGap);
        }
    }
    else if (nElementWidth > gnMaximalPreviewWidth + gnMaximalHorizontalPreviewGap)
    {
        // The preferred column count is too small.
        nPreviewWidth = gnPreferredPreviewWidth;
        mnColumnCount = floor((nWidth+gnPreferredHorizontalPreviewGap)
            / (nPreviewWidth+gnPreferredHorizontalPreviewGap));
        mnHorizontalGap = round((nWidth - mnColumnCount*nPreviewWidth) / mnColumnCount);
    }
    else
    {
        // The preferred column count is possible.  Determine gap and
        // preview width.
        mnColumnCount = gnPreferredColumnCount;
        if (nElementWidth - gnPreferredPreviewWidth < gnMinimalHorizontalPreviewGap)
        {
            // Use the minimal gap and adapt the preview width.
            mnHorizontalGap = floor(gnMinimalHorizontalPreviewGap);
            nPreviewWidth = (nWidth - mnColumnCount*mnHorizontalGap) / mnColumnCount;
        }
        else if (nElementWidth - gnPreferredPreviewWidth <= gnMaximalHorizontalPreviewGap)
        {
            // Use the maximal gap and adapt the preview width.
            mnHorizontalGap = round(gnMaximalHorizontalPreviewGap);
            nPreviewWidth = (nWidth - mnColumnCount*mnHorizontalGap) / mnColumnCount;
        }
        else
        {
            // Use the preferred preview width and adapt the gap.
            nPreviewWidth = gnPreferredPreviewWidth;
            mnHorizontalGap = round((nWidth - mnColumnCount*nPreviewWidth) / mnColumnCount);
        }
    }

    // Now determine the row count, preview height, and vertical gap.
    const double nPreviewHeight = nPreviewWidth / nSlideAspectRatio;
    mnRowCount = ::std::max(
        sal_Int32(1),
        sal_Int32(ceil((nHeight+gnPreferredVerticalPreviewGap)
                / (nPreviewHeight + gnPreferredVerticalPreviewGap))));
    mnVerticalGap = round(gnPreferredVerticalPreviewGap);

    maPreviewSize = geometry::IntegerSize2D(floor(nPreviewWidth), floor(nPreviewHeight));

    // Reset the offset.
    mnVerticalOffset = 0;
    mnHorizontalOffset = round(-(nWidth
        - mnColumnCount*maPreviewSize.Width
        - (mnColumnCount-1)*mnHorizontalGap)
        / 2);
}

void PresenterSlideSorter::Layout::SetupVisibleArea (void)
{
    geometry::RealPoint2D aPoint (GetLocalPosition(
        geometry::RealPoint2D(maBoundingBox.X1, maBoundingBox.Y1)));
    mnFirstVisibleColumn = 0;
    mnFirstVisibleRow = ::std::max(sal_Int32(0), GetRow(aPoint));

    aPoint = GetLocalPosition(geometry::RealPoint2D( maBoundingBox.X2, maBoundingBox.Y2));
    mnLastVisibleColumn = mnColumnCount - 1;
    mnLastVisibleRow = GetRow(aPoint, true);
}

bool PresenterSlideSorter::Layout::IsScrollBarNeeded (const sal_Int32 nSlideCount)
{
    geometry::RealPoint2D aBottomRight;
    aBottomRight = GetPoint(
        mnColumnCount * (GetRow(nSlideCount)+1) - 1, +1, +1);
    return aBottomRight.X > maBoundingBox.X2-maBoundingBox.X1
        || aBottomRight.Y > maBoundingBox.Y2-maBoundingBox.Y1;
}

geometry::RealPoint2D PresenterSlideSorter::Layout::GetLocalPosition(
    const geometry::RealPoint2D& rWindowPoint) const
{
    if(Application::GetSettings().GetLayoutRTL())
        {
            return css::geometry::RealPoint2D(
                                              -rWindowPoint.X  + maBoundingBox.X2 + mnHorizontalOffset,
                                              rWindowPoint.Y - maBoundingBox.Y1 + mnVerticalOffset);
        }
    else
        {
            return css::geometry::RealPoint2D(
                                              rWindowPoint.X - maBoundingBox.X1 + mnHorizontalOffset,
                                              rWindowPoint.Y - maBoundingBox.Y1 + mnVerticalOffset);
        }
}

geometry::RealPoint2D PresenterSlideSorter::Layout::GetWindowPosition(
    const geometry::RealPoint2D& rLocalPoint) const
{
    if(Application::GetSettings().GetLayoutRTL())
        {
            return css::geometry::RealPoint2D(
                                              -rLocalPoint.X + mnHorizontalOffset + maBoundingBox.X2,
                                              rLocalPoint.Y - mnVerticalOffset + maBoundingBox.Y1);
        }
    else
        {
            return css::geometry::RealPoint2D(
                                              rLocalPoint.X - mnHorizontalOffset + maBoundingBox.X1,
                                              rLocalPoint.Y - mnVerticalOffset + maBoundingBox.Y1);
        }
}

sal_Int32 PresenterSlideSorter::Layout::GetColumn (
    const css::geometry::RealPoint2D& rLocalPoint,
    const bool bReturnInvalidValue) const
{
    const sal_Int32 nColumn(floor(
        (rLocalPoint.X + mnHorizontalGap/2.0) / (maPreviewSize.Width+mnHorizontalGap)));
    if (bReturnInvalidValue
        || (nColumn>=mnFirstVisibleColumn && nColumn<=mnLastVisibleColumn))
    {
        return nColumn;
    }
    else
        return -1;
}

sal_Int32 PresenterSlideSorter::Layout::GetRow (
    const css::geometry::RealPoint2D& rLocalPoint,
    const bool bReturnInvalidValue) const
{
    const sal_Int32 nRow (floor(
        (rLocalPoint.Y + mnVerticalGap/2.0) / (maPreviewSize.Height+mnVerticalGap)));
    if (bReturnInvalidValue
        || (nRow>=mnFirstVisibleRow && nRow<=mnLastVisibleRow))
    {
        return nRow;
    }
    else
        return -1;
}

sal_Int32 PresenterSlideSorter::Layout::GetSlideIndexForPosition (
    const css::geometry::RealPoint2D& rWindowPoint) const
{
    if ( ! PresenterGeometryHelper::IsInside(maBoundingBox, rWindowPoint))
        return -1;

    const css::geometry::RealPoint2D aLocalPosition (GetLocalPosition(rWindowPoint));
    const sal_Int32 nColumn (GetColumn(aLocalPosition));
    const sal_Int32 nRow (GetRow(aLocalPosition));

    if (nColumn < 0 || nRow < 0)
        return -1;
    else
    {
        sal_Int32 nIndex (GetIndex(nRow, nColumn));
        if (nIndex >= mnSlideCount)
            return -1;
        else
            return nIndex;
    }
}

geometry::RealPoint2D PresenterSlideSorter::Layout::GetPoint (
    const sal_Int32 nSlideIndex,
    const sal_Int32 nRelativeHorizontalPosition,
    const sal_Int32 nRelativeVerticalPosition) const
{
    sal_Int32 nColumn (GetColumn(nSlideIndex));
    sal_Int32 nRow (GetRow(nSlideIndex));

    geometry::RealPoint2D aPosition (
        mnHorizontalBorder + nColumn*(maPreviewSize.Width+mnHorizontalGap),
        mnVerticalBorder + nRow*(maPreviewSize.Height+mnVerticalGap));

    if (nRelativeHorizontalPosition >= 0)
    {
        if (nRelativeHorizontalPosition > 0)
            aPosition.X += maPreviewSize.Width;
        else
            aPosition.X += maPreviewSize.Width / 2.0;
    }
    if (nRelativeVerticalPosition >= 0)
    {
        if (nRelativeVerticalPosition > 0)
            aPosition.Y += maPreviewSize.Height;
        else
            aPosition.Y += maPreviewSize.Height / 2.0;
    }

    return aPosition;
}

awt::Rectangle PresenterSlideSorter::Layout::GetBoundingBox (const sal_Int32 nSlideIndex) const
{
    bool isRTL = Application::GetSettings().GetLayoutRTL();
    const geometry::RealPoint2D aWindowPosition(GetWindowPosition(GetPoint(nSlideIndex, isRTL?1:-1, -1)));
    return PresenterGeometryHelper::ConvertRectangle(
                                                     geometry::RealRectangle2D(
                                                                               aWindowPosition.X,
                                                                               aWindowPosition.Y,
                                                                               aWindowPosition.X + maPreviewSize.Width,
                                                                               aWindowPosition.Y + maPreviewSize.Height));
}

void PresenterSlideSorter::Layout::ForAllVisibleSlides (const ::boost::function<void(sal_Int32)>& rAction)
{
    for (sal_Int32 nRow=mnFirstVisibleRow; nRow<=mnLastVisibleRow; ++nRow)
    {
        for (sal_Int32 nColumn=mnFirstVisibleColumn; nColumn<=mnLastVisibleColumn; ++nColumn)
        {
            const sal_Int32 nSlideIndex (GetIndex(nRow, nColumn));
            if (nSlideIndex >= mnSlideCount)
                return;
            rAction(nSlideIndex);
        }
    }
}

sal_Int32 PresenterSlideSorter::Layout::GetFirstVisibleSlideIndex (void) const
{
    return GetIndex(mnFirstVisibleRow, mnFirstVisibleColumn);
}

sal_Int32 PresenterSlideSorter::Layout::GetLastVisibleSlideIndex (void) const
{
    return ::std::min(
        GetIndex(mnLastVisibleRow, mnLastVisibleColumn),
        mnSlideCount);
}

bool PresenterSlideSorter::Layout::SetHorizontalOffset (const double nOffset)
{
    if (mnHorizontalOffset != nOffset)
    {
        mnHorizontalOffset = round(nOffset);
        SetupVisibleArea();
        UpdateScrollBars();
        return true;
    }
    else
        return false;
}

bool PresenterSlideSorter::Layout::SetVerticalOffset (const double nOffset)
{
    if (mnVerticalOffset != nOffset)
    {
        mnVerticalOffset = round(nOffset);
        SetupVisibleArea();
        UpdateScrollBars();
        return true;
    }
    else
        return false;
}

void PresenterSlideSorter::Layout::UpdateScrollBars (void)
{
    sal_Int32 nTotalRowCount (0);
    nTotalRowCount = sal_Int32(ceil(double(mnSlideCount) / double(mnColumnCount)));

    if (mpVerticalScrollBar.get() != NULL)
    {
        mpVerticalScrollBar->SetTotalSize(
            nTotalRowCount * maPreviewSize.Height
                + (nTotalRowCount-1) * mnVerticalGap
            + 2*mnVerticalGap);
        mpVerticalScrollBar->SetThumbPosition(mnVerticalOffset, false);
        mpVerticalScrollBar->SetThumbSize(maBoundingBox.Y2 - maBoundingBox.Y1 + 1);
        mpVerticalScrollBar->SetLineHeight(maPreviewSize.Height);
    }

    // No place yet for the vertical scroll bar.
}

sal_Int32 PresenterSlideSorter::Layout::GetIndex (
    const sal_Int32 nRow,
    const sal_Int32 nColumn) const
{
    return nRow * mnColumnCount + nColumn;
}

sal_Int32 PresenterSlideSorter::Layout::GetRow (const sal_Int32 nSlideIndex) const
{
    return nSlideIndex / mnColumnCount;
}

sal_Int32 PresenterSlideSorter::Layout::GetColumn (const sal_Int32 nSlideIndex) const
{
    return nSlideIndex % mnColumnCount;
}

//===== PresenterSlideSorter::MouseOverManager ================================

PresenterSlideSorter::MouseOverManager::MouseOverManager (
    const Reference<container::XIndexAccess>& rxSlides,
    const ::boost::shared_ptr<PresenterTheme>& rpTheme,
    const Reference<awt::XWindow>& rxInvalidateTarget,
    const ::boost::shared_ptr<PresenterPaintManager>& rpPaintManager)
    : mxCanvas(),
      mxSlides(rxSlides),
      mpLeftLabelBitmap(),
      mpCenterLabelBitmap(),
      mpRightLabelBitmap(),
      mpFont(),
      mnSlideIndex(-1),
      maSlideBoundingBox(),
      mxInvalidateTarget(rxInvalidateTarget),
      mpPaintManager(rpPaintManager)
{
    if (rpTheme.get()!=NULL)
    {
        ::boost::shared_ptr<PresenterBitmapContainer> pBitmaps (rpTheme->GetBitmapContainer());
        if (pBitmaps.get() != NULL)
        {
            mpLeftLabelBitmap = pBitmaps->GetBitmap("LabelLeft");
            mpCenterLabelBitmap = pBitmaps->GetBitmap("LabelCenter");
            mpRightLabelBitmap = pBitmaps->GetBitmap("LabelRight");
        }

        mpFont = rpTheme->GetFont("SlideSorterLabelFont");
    }
}

PresenterSlideSorter::MouseOverManager::~MouseOverManager (void)
{
}

void PresenterSlideSorter::MouseOverManager::Paint (
    const sal_Int32 nSlideIndex,
    const Reference<rendering::XCanvas>& rxCanvas,
    const Reference<rendering::XPolyPolygon2D>& rxClip)
{
    if (nSlideIndex != mnSlideIndex)
        return;

    if (mxCanvas != rxCanvas)
        SetCanvas(rxCanvas);
    if (rxCanvas != NULL)
    {
        if ( ! mxBitmap.is())
            mxBitmap = CreateBitmap(msText, maSlideBoundingBox.Width);
        if (mxBitmap.is())
        {
            geometry::IntegerSize2D aSize (mxBitmap->getSize());
            const double nXOffset (maSlideBoundingBox.X
                + (maSlideBoundingBox.Width - aSize.Width) / 2.0);
            const double nYOffset (maSlideBoundingBox.Y
                + (maSlideBoundingBox.Height - aSize.Height) / 2.0);
            rxCanvas->drawBitmap(
                mxBitmap,
                rendering::ViewState(
                    geometry::AffineMatrix2D(1,0,0, 0,1,0),
                    rxClip),
                rendering::RenderState(
                    geometry::AffineMatrix2D(1,0,nXOffset, 0,1,nYOffset),
                    NULL,
                    Sequence<double>(4),
                    rendering::CompositeOperation::SOURCE));
        }
    }
}

void PresenterSlideSorter::MouseOverManager::SetCanvas (
    const Reference<rendering::XCanvas>& rxCanvas)
{
    mxCanvas = rxCanvas;
    if (mpFont.get() != NULL)
        mpFont->PrepareFont(Reference<rendering::XCanvas>(mxCanvas, UNO_QUERY));
}

void PresenterSlideSorter::MouseOverManager::SetSlide (
    const sal_Int32 nSlideIndex,
    const awt::Rectangle& rBox)
{
    if (mnSlideIndex == nSlideIndex)
        return;

    mnSlideIndex = -1;
    Invalidate();

    maSlideBoundingBox = rBox;
    mnSlideIndex = nSlideIndex;

    if (nSlideIndex >= 0)
    {
        if (mxSlides.get() != NULL)
        {
            msText = OUString();

            Reference<beans::XPropertySet> xSlideProperties(mxSlides->getByIndex(nSlideIndex), UNO_QUERY);
            if (xSlideProperties.is())
                xSlideProperties->getPropertyValue("LinkDisplayName") >>= msText;

            if (msText.isEmpty())
                msText = "Slide " + OUString::number(nSlideIndex + 1);
        }
    }
    else
    {
        msText = OUString();
    }
    mxBitmap = NULL;

    Invalidate();
}

Reference<rendering::XBitmap> PresenterSlideSorter::MouseOverManager::CreateBitmap (
    const OUString& rsText,
    const sal_Int32 nMaximalWidth) const
{
    if ( ! mxCanvas.is())
        return NULL;

    if (mpFont.get()==NULL || !mpFont->mxFont.is())
        return NULL;

    // Long text has to be shortened.
    const OUString sText (GetFittingText(rsText, nMaximalWidth
            - 2*gnHorizontalLabelBorder
            - 2*gnHorizontalLabelPadding));

    // Determine the size of the label.  Its height is defined by the
    // bitmaps that are used to paints its background.  The width is defined
    // by the text.
    geometry::IntegerSize2D aLabelSize (CalculateLabelSize(sText));

    // Create a new bitmap that will contain the complete label.
    Reference<rendering::XBitmap> xBitmap (
        mxCanvas->getDevice()->createCompatibleAlphaBitmap(aLabelSize));

    if ( ! xBitmap.is())
        return NULL;

    Reference<rendering::XBitmapCanvas> xBitmapCanvas (xBitmap, UNO_QUERY);
    if ( ! xBitmapCanvas.is())
        return NULL;

    // Paint the background.
    PaintButtonBackground(xBitmapCanvas, aLabelSize);

    // Paint the text.
    if (!sText.isEmpty())
    {

        const rendering::StringContext aContext (sText, 0, sText.getLength());
        const Reference<rendering::XTextLayout> xLayout (mpFont->mxFont->createTextLayout(
            aContext, rendering::TextDirection::WEAK_LEFT_TO_RIGHT,0));
        const geometry::RealRectangle2D aTextBBox (xLayout->queryTextBounds());

        const double nXOffset = (aLabelSize.Width - aTextBBox.X2 + aTextBBox.X1) / 2;
        const double nYOffset = aLabelSize.Height
            - (aLabelSize.Height - aTextBBox.Y2 + aTextBBox.Y1)/2 - aTextBBox.Y2;

        const rendering::ViewState aViewState(
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL);

        rendering::RenderState aRenderState (
            geometry::AffineMatrix2D(1,0,nXOffset, 0,1,nYOffset),
            NULL,
            Sequence<double>(4),
            rendering::CompositeOperation::SOURCE);
        PresenterCanvasHelper::SetDeviceColor(aRenderState, mpFont->mnColor);

        xBitmapCanvas->drawTextLayout (
            xLayout,
            aViewState,
            aRenderState);
    }

    return xBitmap;
}

OUString PresenterSlideSorter::MouseOverManager::GetFittingText (
    const OUString& rsText,
    const double nMaximalWidth) const
{
    const double nTextWidth (
        PresenterCanvasHelper::GetTextSize(mpFont->mxFont, rsText).Width);
    if (nTextWidth > nMaximalWidth)
    {
        // Text is too wide.  Shorten it by removing characters from the end
        // and replacing them by ellipses.

        // Guess a start value of the final string length.
        double nBestWidth (0);
        OUString sBestCandidate;
        sal_Int32 nLength (round(rsText.getLength() * nMaximalWidth / nTextWidth));
        const OUString sEllipses ("...");
        while (true)
        {
            const OUString sCandidate (rsText.copy(0,nLength) + sEllipses);
            const double nWidth (
                PresenterCanvasHelper::GetTextSize(mpFont->mxFont, sCandidate).Width);
            if (nWidth > nMaximalWidth)
            {
                // Candidate still too wide, shorten it.
                nLength -= 1;
                if (nLength <= 0)
                    break;
            }
            else if (nWidth < nMaximalWidth)
            {
                // Candidate short enough.
                if (nWidth > nBestWidth)
                {
                    // Best length so far.
                    sBestCandidate = sCandidate;
                    nBestWidth = nWidth;
                    nLength += 1;
                    if (nLength >= rsText.getLength())
                        break;
                }
                else
                    break;
            }
            else
            {
                // Candidate is exactly as long as it may be.  Use it
                // without looking any further.
                sBestCandidate = sCandidate;
                break;
            }
        }
        return sBestCandidate;
    }
    else
        return rsText;
}

geometry::IntegerSize2D PresenterSlideSorter::MouseOverManager::CalculateLabelSize (
    const OUString& rsText) const
{
    // Height is specified by the label bitmaps.
    sal_Int32 nHeight (32);
    if (mpCenterLabelBitmap.get() != NULL)
    {
        Reference<rendering::XBitmap> xBitmap (mpCenterLabelBitmap->GetNormalBitmap());
        if (xBitmap.is())
            nHeight = xBitmap->getSize().Height;
    }

    // Width is specified by text width and maximal width.
    const geometry::RealSize2D aTextSize (
        PresenterCanvasHelper::GetTextSize(mpFont->mxFont, rsText));

    const sal_Int32 nWidth (round(aTextSize.Width + 2*gnHorizontalLabelPadding));

    return geometry::IntegerSize2D(nWidth, nHeight);
}

void PresenterSlideSorter::MouseOverManager::PaintButtonBackground (
    const Reference<rendering::XBitmapCanvas>& rxCanvas,
    const geometry::IntegerSize2D& rSize) const
{
    // Get the bitmaps for painting the label background.
    Reference<rendering::XBitmap> xLeftLabelBitmap;
    if (mpLeftLabelBitmap.get() != NULL)
        xLeftLabelBitmap = mpLeftLabelBitmap->GetNormalBitmap();

    Reference<rendering::XBitmap> xCenterLabelBitmap;
    if (mpCenterLabelBitmap.get() != NULL)
        xCenterLabelBitmap = mpCenterLabelBitmap->GetNormalBitmap();

    Reference<rendering::XBitmap> xRightLabelBitmap;
    if (mpRightLabelBitmap.get() != NULL)
        xRightLabelBitmap = mpRightLabelBitmap->GetNormalBitmap();

    PresenterUIPainter::PaintHorizontalBitmapComposite (
        Reference<rendering::XCanvas>(rxCanvas, UNO_QUERY),
        awt::Rectangle(0,0, rSize.Width,rSize.Height),
        awt::Rectangle(0,0, rSize.Width,rSize.Height),
        xLeftLabelBitmap,
        xCenterLabelBitmap,
        xRightLabelBitmap);
}

void PresenterSlideSorter::MouseOverManager::Invalidate (void)
{
    if (mpPaintManager.get() != NULL)
        mpPaintManager->Invalidate(mxInvalidateTarget, maSlideBoundingBox, true);
}

//===== PresenterSlideSorter::CurrentSlideFrameRenderer =======================

PresenterSlideSorter::CurrentSlideFrameRenderer::CurrentSlideFrameRenderer (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas)
    : mpTopLeft(),
      mpTop(),
      mpTopRight(),
      mpLeft(),
      mpRight(),
      mpBottomLeft(),
      mpBottom(),
      mpBottomRight(),
      mnTopFrameSize(0),
      mnLeftFrameSize(0),
      mnRightFrameSize(0),
      mnBottomFrameSize(0)
{
    PresenterConfigurationAccess aConfiguration (
        rxContext,
        OUString("/org.openoffice.Office.PresenterScreen/"),
        PresenterConfigurationAccess::READ_ONLY);
    Reference<container::XHierarchicalNameAccess> xBitmaps (
        aConfiguration.GetConfigurationNode(
            "PresenterScreenSettings/SlideSorter/CurrentSlideBorderBitmaps"),
        UNO_QUERY);
    if ( ! xBitmaps.is())
        return;

    PresenterBitmapContainer aContainer (
        "PresenterScreenSettings/SlideSorter/CurrentSlideBorderBitmaps",
        ::boost::shared_ptr<PresenterBitmapContainer>(),
        rxContext,
        rxCanvas);

    mpTopLeft = aContainer.GetBitmap("TopLeft");
    mpTop = aContainer.GetBitmap("Top");
    mpTopRight = aContainer.GetBitmap("TopRight");
    mpLeft = aContainer.GetBitmap("Left");
    mpRight = aContainer.GetBitmap("Right");
    mpBottomLeft = aContainer.GetBitmap("BottomLeft");
    mpBottom = aContainer.GetBitmap("Bottom");
    mpBottomRight = aContainer.GetBitmap("BottomRight");

    // Determine size of frame.
    if (mpTop.get() != NULL)
        mnTopFrameSize = mpTop->mnHeight;
    if (mpLeft.get() != NULL)
        mnLeftFrameSize = mpLeft->mnWidth;
    if (mpRight.get() != NULL)
        mnRightFrameSize = mpRight->mnWidth;
    if (mpBottom.get() != NULL)
        mnBottomFrameSize = mpBottom->mnHeight;

    if (mpTopLeft.get() != NULL)
    {
        mnTopFrameSize = ::std::max(mnTopFrameSize, mpTopLeft->mnHeight);
        mnLeftFrameSize = ::std::max(mnLeftFrameSize, mpTopLeft->mnWidth);
    }
    if (mpTopRight.get() != NULL)
    {
        mnTopFrameSize = ::std::max(mnTopFrameSize, mpTopRight->mnHeight);
        mnRightFrameSize = ::std::max(mnRightFrameSize, mpTopRight->mnWidth);
    }
    if (mpBottomLeft.get() != NULL)
    {
        mnLeftFrameSize = ::std::max(mnLeftFrameSize, mpBottomLeft->mnWidth);
        mnBottomFrameSize = ::std::max(mnBottomFrameSize, mpBottomLeft->mnHeight);
    }
    if (mpBottomRight.get() != NULL)
    {
        mnRightFrameSize = ::std::max(mnRightFrameSize, mpBottomRight->mnWidth);
        mnBottomFrameSize = ::std::max(mnBottomFrameSize, mpBottomRight->mnHeight);
    }
}

PresenterSlideSorter::CurrentSlideFrameRenderer::~CurrentSlideFrameRenderer (void)
{
}

void PresenterSlideSorter::CurrentSlideFrameRenderer::PaintCurrentSlideFrame (
    const awt::Rectangle& rSlideBoundingBox,
    const Reference<rendering::XCanvas>& rxCanvas,
    const geometry::RealRectangle2D& rClipBox)
{
    if ( ! rxCanvas.is())
        return;

    const Reference<rendering::XPolyPolygon2D> xClip (
        PresenterGeometryHelper::CreatePolygon(rClipBox, rxCanvas->getDevice()));

    if (mpTop.get() != NULL)
    {
        PaintBitmapTiled(
            mpTop->GetNormalBitmap(),
            rxCanvas,
            rClipBox,
            rSlideBoundingBox.X,
            rSlideBoundingBox.Y - mpTop->mnHeight,
            rSlideBoundingBox.Width,
            mpTop->mnHeight);
    }
    if (mpLeft.get() != NULL)
    {
        PaintBitmapTiled(
            mpLeft->GetNormalBitmap(),
            rxCanvas,
            rClipBox,
            rSlideBoundingBox.X - mpLeft->mnWidth,
            rSlideBoundingBox.Y,
            mpLeft->mnWidth,
            rSlideBoundingBox.Height);
    }
    if (mpRight.get() != NULL)
    {
        PaintBitmapTiled(
            mpRight->GetNormalBitmap(),
            rxCanvas,
            rClipBox,
            rSlideBoundingBox.X + rSlideBoundingBox.Width,
            rSlideBoundingBox.Y,
            mpRight->mnWidth,
            rSlideBoundingBox.Height);
    }
    if (mpBottom.get() != NULL)
    {
        PaintBitmapTiled(
            mpBottom->GetNormalBitmap(),
            rxCanvas,
            rClipBox,
            rSlideBoundingBox.X,
            rSlideBoundingBox.Y + rSlideBoundingBox.Height,
            rSlideBoundingBox.Width,
            mpBottom->mnHeight);
    }
    if (mpTopLeft.get() != NULL)
    {
        PaintBitmapOnce(
            mpTopLeft->GetNormalBitmap(),
            rxCanvas,
            xClip,
            rSlideBoundingBox.X - mpTopLeft->mnWidth,
            rSlideBoundingBox.Y - mpTopLeft->mnHeight);
    }
    if (mpTopRight.get() != NULL)
    {
        PaintBitmapOnce(
            mpTopRight->GetNormalBitmap(),
            rxCanvas,
            xClip,
            rSlideBoundingBox.X + rSlideBoundingBox.Width,
            rSlideBoundingBox.Y - mpTopLeft->mnHeight);
    }
    if (mpBottomLeft.get() != NULL)
    {
        PaintBitmapOnce(
            mpBottomLeft->GetNormalBitmap(),
            rxCanvas,
            xClip,
            rSlideBoundingBox.X - mpBottomLeft->mnWidth,
            rSlideBoundingBox.Y + rSlideBoundingBox.Height);
    }
    if (mpBottomRight.get() != NULL)
    {
        PaintBitmapOnce(
            mpBottomRight->GetNormalBitmap(),
            rxCanvas,
            xClip,
            rSlideBoundingBox.X + rSlideBoundingBox.Width,
            rSlideBoundingBox.Y + rSlideBoundingBox.Height);
    }
}

awt::Rectangle PresenterSlideSorter::CurrentSlideFrameRenderer::GetBoundingBox (
    const awt::Rectangle& rSlideBoundingBox)
{
    return awt::Rectangle(
        rSlideBoundingBox.X - mnLeftFrameSize,
        rSlideBoundingBox.Y - mnTopFrameSize,
        rSlideBoundingBox.Width + mnLeftFrameSize + mnRightFrameSize,
        rSlideBoundingBox.Height + mnTopFrameSize + mnBottomFrameSize);
}

void PresenterSlideSorter::CurrentSlideFrameRenderer::PaintBitmapOnce(
    const css::uno::Reference<css::rendering::XBitmap>& rxBitmap,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const Reference<rendering::XPolyPolygon2D>& rxClip,
    const double nX,
    const double nY)
{
    OSL_ASSERT(rxCanvas.is());
    if ( ! rxBitmap.is())
        return;

    const rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        rxClip);

    const rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(
            1, 0, nX,
            0, 1, nY),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    rxCanvas->drawBitmap(
        rxBitmap,
        aViewState,
        aRenderState);
}

void PresenterSlideSorter::CurrentSlideFrameRenderer::PaintBitmapTiled(
    const css::uno::Reference<css::rendering::XBitmap>& rxBitmap,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const geometry::RealRectangle2D& rClipBox,
    const double nX0,
    const double nY0,
    const double nWidth,
    const double nHeight)
{
    OSL_ASSERT(rxCanvas.is());
    if ( ! rxBitmap.is())
        return;

    geometry::IntegerSize2D aSize (rxBitmap->getSize());

    const rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(
            PresenterGeometryHelper::Intersection(
                rClipBox,
                geometry::RealRectangle2D(nX0,nY0,nX0+nWidth,nY0+nHeight)),
            rxCanvas->getDevice()));

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(
            1, 0, nX0,
            0, 1, nY0),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    const double nX1 = nX0 + nWidth;
    const double nY1 = nY0 + nHeight;
    for (double nY=nY0; nY<nY1; nY+=aSize.Height)
        for (double nX=nX0; nX<nX1; nX+=aSize.Width)
        {
            aRenderState.AffineTransform.m02 = nX;
            aRenderState.AffineTransform.m12 = nY;
            rxCanvas->drawBitmap(
                rxBitmap,
                aViewState,
                aRenderState);
        }
}

} } // end of namespace ::sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
