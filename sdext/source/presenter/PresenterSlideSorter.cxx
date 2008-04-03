/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterSlideSorter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:05:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "PresenterSlideSorter.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterHelper.hxx"
#include "PresenterScrollBar.hxx"
#include "PresenterWindowManager.hxx"
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/XSlideSorterBase.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/util/Color.hpp>
#include <algorithm>
#include <cmath>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//===== PresenterSlideSorter::Layout ==========================================

class PresenterSlideSorter::Layout
{
public:
    enum Orientation { Horizontal, Vertical };
    Layout (
        const Orientation eOrientation,
        const ::rtl::Reference<PresenterScrollBar>& rpHorizontalScrollBar,
        const ::rtl::Reference<PresenterScrollBar>& rpVerticalScrollBar);

    void Update (const awt::Rectangle& rWindowBox, const double nSlideAspectRatio);
    void SetupVisibleArea (void);
    sal_Int32 GetSlideIndexForPosition (const css::geometry::RealPoint2D& rPoint);
    css::geometry::RealPoint2D GetPoint (
        const sal_Int32 nSlideIndex,
        const sal_Int32 nRelativeHorizontalPosition,
        const sal_Int32 nRelativeVerticalPosition);
    enum DistanceType { Norm1, Norm2, XOnly, YOnly };
    double GetDistanceFromCenter (
        const css::geometry::RealPoint2D& rPoint,
        const sal_Int32 nSlideIndex,
        const DistanceType eDistanceType);
    css::awt::Rectangle GetBoundingBox (const sal_Int32 nSlideIndex);
    void ForAllVisibleSlides (const ::boost::function<void(sal_Int32)>& rAction);
    sal_Int32 GetFirstVisibleSlideIndex (void);
    sal_Int32 GetLastVisibleSlideIndex (void);
    bool SetHorizontalOffset (const double nOffset);

    css::geometry::RealRectangle2D maInnerBorder;
    css::geometry::RealRectangle2D maOuterBorder;
    css::geometry::RealRectangle2D maViewport;
    css::geometry::IntegerSize2D maPreviewSize;
    double mnHorizontalOffset;
    double mnVerticalOffset;
    double mnHorizontalGap;
    double mnVerticalGap;
    sal_Int32 mnRowCount;
    sal_Int32 mnColumnCount;
    sal_Int32 mnSlideCount;
    sal_Int32 mnSlideIndexAtMouse;
    sal_Int32 mnFirstVisibleColumn;
    sal_Int32 mnLastVisibleColumn;
    sal_Int32 mnFirstVisibleRow;
    sal_Int32 mnLastVisibleRow;
    double mnScrollBarHeight;

private:
    Orientation meOrientation;
    ::rtl::Reference<PresenterScrollBar> mpHorizontalScrollBar;
    ::rtl::Reference<PresenterScrollBar> mpVerticalScrollBar;
    void UpdateScrollBars (void);
    sal_Int32 GetIndex (const sal_Int32 nRow, const sal_Int32 nColumn);
    sal_Int32 GetRow (const sal_Int32 nSlideIndex);
    sal_Int32 GetColumn (const sal_Int32 nSlideIndex);
};




//==== PresenterSlideSorter::MouseOverManager =================================

namespace {
    typedef cppu::WeakComponentImplHelper1<
        css::drawing::XSlidePreviewCacheListener
        > MouseOverManagerInterfaceBase;
}

class PresenterSlideSorter::MouseOverManager
    : private ::cppu::BaseMutex,
      public MouseOverManagerInterfaceBase
{
public:
    const static double mnMinScale;
    const static double mnMaxScale;

    MouseOverManager (
        const Reference<XComponentContext>& rxContext,
        const Reference<container::XIndexAccess>& rxSlides,
        const Reference<frame::XModel>& rxModel);
    ~MouseOverManager (void);

    void SetCanvas (
        const Reference<rendering::XSpriteCanvas>& rxCanvas);

    void SetSlide (
        const sal_Int32 nSlideIndex,
        const awt::Rectangle& rBox,
        const double nDistance);

    void SetActiveState (const bool bIsActive);

    // XSlidePreviewCacheListener

    virtual void SAL_CALL notifyPreviewCreation (
        sal_Int32 nSlideIndex)
        throw(css::uno::RuntimeException);

private:
    Reference<rendering::XSpriteCanvas> mxCanvas;
    Reference<drawing::XSlidePreviewCache> mxPreviewCache;
    sal_Int32 mnSlideIndex;
    awt::Rectangle maBoundingBox;
    double mnDistance;
    Reference<rendering::XCustomSprite> mxSprite;
    geometry::RealSize2D maSpriteSize;
    bool mbIsActive;

    void CreateSprite (const sal_Int32 nSlideIndex);
    void DisposeSprite (void);
};

const double PresenterSlideSorter::MouseOverManager::mnMinScale = 1.0;
const double PresenterSlideSorter::MouseOverManager::mnMaxScale = 2.0;



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
      mpLayout(),
      mpHorizontalScrollBar(),
      mpMouseOverManager(),
      mnSlideIndexMousePressed(-1),
      mnCurrentSlideIndex(-1)
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

        // Create the scroll bar.
        mpHorizontalScrollBar = ::rtl::Reference<PresenterScrollBar>(
            new PresenterHorizontalScrollBar(
                rxContext,
                mxWindow,
                ::boost::bind(&PresenterSlideSorter::SetHorizontalOffset,this,_1)));

        // Remember the current slide.
        mnCurrentSlideIndex = mxSlideShowController->getCurrentSlideIndex();

        // Create the layout.
        mpLayout.reset(new Layout(
            Layout::Horizontal,
            mpHorizontalScrollBar,
            ::rtl::Reference<PresenterScrollBar>()));

        // Create the preview cache.
        mxPreviewCache = Reference<drawing::XSlidePreviewCache>(
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.drawing.PresenterPreviewCache"),
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
        mpMouseOverManager = ::rtl::Reference<MouseOverManager>(new MouseOverManager(
            mxComponentContext,
            Reference<container::XIndexAccess>(mxSlideShowController, UNO_QUERY),
            rxController->getModel()));

        // Listen for changes of the current slide.
        Reference<beans::XPropertySet> xControllerProperties (rxController, UNO_QUERY_THROW);
        xControllerProperties->addPropertyChangeListener(
            OUString::createFromAscii("CurrentPage"),
            this);

        UpdateLayout();

        // Move the current slide in the center of the window.
        const awt::Rectangle aCurrentSlideBBox (mpLayout->GetBoundingBox(mnCurrentSlideIndex));
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        SetHorizontalOffset(aCurrentSlideBBox.X - aWindowBox.Width/2.0);

        Paint();
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
    mpHorizontalScrollBar = NULL;
    if (mpMouseOverManager.is())
    {
        mpMouseOverManager->dispose();
        mpMouseOverManager = NULL;
    }

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
    if (mxPreviewCache.is())
        if (bIsActive)
            mxPreviewCache->resume();
        else
            mxPreviewCache->pause();

    if (mpMouseOverManager.get() != NULL)
        mpMouseOverManager->SetActiveState(bIsActive);
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
        if (mpMouseOverManager.get() != NULL)
            mpMouseOverManager->SetCanvas(NULL);
        if (mpHorizontalScrollBar.is())
            mpHorizontalScrollBar->SetCanvas(NULL);
        mbIsPaintPending = true;

        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->invalidate(awt::InvalidateStyle::CHILDREN);
    }
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterSlideSorter::windowResized (const awt::WindowEvent& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    // The canvas should eventually be (or already has been) disposed by the
    // pane.  We listen for that and request a repaint when that happens.
    mbIsPaintPending = true;
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
    mbIsPaintPending = true;
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

    if ( ! ProvideCanvas())
        return;

    if (mbIsPaintPending)
    {
        UpdateLayout();
        Paint();
    }

    /*
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
    */
}




//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterSlideSorter::mousePressed (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    const geometry::RealPoint2D aPosition (rEvent.X, rEvent.Y);
    mnSlideIndexMousePressed = mpLayout->GetSlideIndexForPosition(aPosition);
}




void SAL_CALL PresenterSlideSorter::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    const geometry::RealPoint2D aPosition (rEvent.X, rEvent.Y);
    const sal_Int32 nSlideIndex (mpLayout->GetSlideIndexForPosition(aPosition));

    if (nSlideIndex == mnSlideIndexMousePressed && mnSlideIndexMousePressed >= 0)
    {
        mpMouseOverManager->SetSlide(-1, awt::Rectangle(0,0,0,0), 0);
        GotoSlide(nSlideIndex);
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
        mpMouseOverManager->SetSlide(mnSlideIndexMousePressed, awt::Rectangle(0,0,0,0), 0);
}




//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterSlideSorter::mouseMoved (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    if (mpMouseOverManager.get() != NULL)
    {
        const geometry::RealPoint2D aPosition (rEvent.X, rEvent.Y);
        sal_Int32 nSlideIndex (mpLayout->GetSlideIndexForPosition(aPosition));
        double nDistanceFromCenter (2);

        if (nSlideIndex >= 0)
        {
            const double nDistance (mpLayout->GetDistanceFromCenter(aPosition, nSlideIndex,
                    Layout::Norm1));
            if (nDistance < 1)
                nDistanceFromCenter = nDistance;
            else
                nSlideIndex = -1;
        }
        else
        {
            mnSlideIndexMousePressed = -1;
        }

        if (nSlideIndex < 0)
        {
            mpMouseOverManager->SetSlide(nSlideIndex, awt::Rectangle(0,0,0,0), 0);
        }
        else
        {
            mpMouseOverManager->SetSlide(
                nSlideIndex,
                mpLayout->GetBoundingBox(nSlideIndex),
                nDistanceFromCenter);
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
    // Close the slide sorter view.
}




//----- XSlidePreviewCacheListener --------------------------------------------

void SAL_CALL PresenterSlideSorter::notifyPreviewCreation (
    sal_Int32 nSlideIndex)
    throw(css::uno::RuntimeException)
{
    PaintPreview(nSlideIndex);
}




//-----------------------------------------------------------------------------

void PresenterSlideSorter::UpdateLayout (void)
{
    if ( ! mxWindow.is())
        return;

    mpLayout->Update(
        mxWindow->getPosSize(),
        GetSlideAspectRatio());

    // Tell the preview cache about some of the values.
    mxPreviewCache->setPreviewSize(mpLayout->maPreviewSize);
    mxPreviewCache->setVisibleRange(
        mpLayout->GetFirstVisibleSlideIndex(),
        mpLayout->GetLastVisibleSlideIndex());
}




void PresenterSlideSorter::ClearBackground (
    const Reference<rendering::XCanvas>& rxCanvas,
    const awt::Rectangle& rUpdateBox)
{
    OSL_ASSERT(rxCanvas.is());

    util::Color aColor (
        mpPresenterController->GetViewBackgroundColor(mxViewId->getResourceURL()));
    Sequence<double> aBackgroundColor(3);
    aBackgroundColor[0] = ((aColor >> 16) & 0x0ff) / 255.0;
    aBackgroundColor[1] = ((aColor >> 8) & 0x0ff) / 255.0;
    aBackgroundColor[2] = ((aColor >> 0) & 0x0ff) / 255.0;

    rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        aBackgroundColor,
        rendering::CompositeOperation::SOURCE);

    Reference<rendering::XPolyPolygon2D> xPolygon (
        PresenterGeometryHelper::CreatePolygon(rUpdateBox, rxCanvas->getDevice()));
    if (xPolygon.is())
        rxCanvas->fillPolyPolygon(
            xPolygon,
            aViewState,
            aRenderState);
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
            if ((xProperties->getPropertyValue(OUString::createFromAscii("Width")) >>= nWidth)
                && (xProperties->getPropertyValue(OUString::createFromAscii("Height")) >>= nHeight)
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
    const sal_Int32 nSlideIndex)
{
    if ( ! ProvideCanvas())
        return;

    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    PaintPreview(mxCanvas, nSlideIndex);

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}




void PresenterSlideSorter::PaintPreview (
    const Reference<rendering::XCanvas>& rxCanvas,
    const sal_Int32 nSlideIndex)
{
    OSL_ASSERT(rxCanvas.is());

    Reference<rendering::XBitmap> xPreview (GetPreview(nSlideIndex));

    const geometry::RealPoint2D aTopLeft (mpLayout->GetPoint(nSlideIndex, -1, -1));
    rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(
            1, 0, aTopLeft.X,
            0, 1, aTopLeft.Y),
        NULL,
        Sequence<double>(3),
        rendering::CompositeOperation::SOURCE);

    geometry::IntegerSize2D aSize (mpLayout->maPreviewSize);

    if (nSlideIndex == mnCurrentSlideIndex)
    {
        // Paint a frame around the slide preview that indicates that this
        // is the current slide.
        const double nFrameWidth (3);
        const geometry::RealRectangle2D aBox (-nFrameWidth, -nFrameWidth,
            aSize.Width+nFrameWidth, aSize.Height+nFrameWidth);
        Reference<rendering::XPolyPolygon2D> xPolygon (
            PresenterGeometryHelper::CreatePolygon(aBox, rxCanvas->getDevice()));
        if (xPolygon.is())
        {
            const util::Color aColor (
                mpPresenterController->GetViewFontColor(mxViewId->getResourceURL()));
            aRenderState.DeviceColor[0] = ((aColor >> 16) & 0x0ff) / 255.0;
            aRenderState.DeviceColor[1] = ((aColor >> 8) & 0x0ff) / 255.0;
            aRenderState.DeviceColor[2] = ((aColor >> 0) & 0x0ff) / 255.0;
            rxCanvas->fillPolyPolygon(xPolygon, aViewState, aRenderState);
        }
    }

    if (xPreview.is())
    {
        aSize = xPreview->getSize();
        if (aSize.Width > 0 && aSize.Height > 0)
        {
            rxCanvas->drawBitmap(xPreview, aViewState, aRenderState);
        }
    }

    // Paint a border around the preview.
    const geometry::RealRectangle2D aBox (0, 0, aSize.Width, aSize.Height);
    Reference<rendering::XPolyPolygon2D> xPolygon (
        PresenterGeometryHelper::CreatePolygon(aBox, rxCanvas->getDevice()));
    if (xPolygon.is())
    {
        const util::Color aColor (
            mpPresenterController->GetViewFontColor(mxViewId->getResourceURL()));
        aRenderState.DeviceColor[0] = ((aColor >> 16) & 0x0ff) / 255.0;
        aRenderState.DeviceColor[1] = ((aColor >> 8) & 0x0ff) / 255.0;
        aRenderState.DeviceColor[2] = ((aColor >> 0) & 0x0ff) / 255.0;
        rxCanvas->drawPolyPolygon(xPolygon, aViewState, aRenderState);
    }
}




void PresenterSlideSorter::Paint (void)
{
    if ( ! ProvideCanvas())
        return;

    if (mpLayout->mnRowCount<=0 || mpLayout->mnColumnCount<=0)
    {
        OSL_ASSERT(mpLayout->mnRowCount>0 || mpLayout->mnColumnCount>0);
        return;
    }

    mbIsPaintPending = false;

    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    ClearBackground(mxCanvas, awt::Rectangle(0,0,aWindowBox.Width,aWindowBox.Height));

    // Paint the horizontal scroll bar.
    if (mpHorizontalScrollBar.is())
    {
        mpHorizontalScrollBar->SetCanvas(mxCanvas);
        mpHorizontalScrollBar->Paint(aWindowBox);
    }

    // Paint the slides.
    mpLayout->ForAllVisibleSlides(
        ::boost::bind(&PresenterSlideSorter::PaintPreview, this, mxCanvas, _1));

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}




void PresenterSlideSorter::Invalidate (const awt::Rectangle& rBBox)
{
    Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
    if (xPeer.is())
        xPeer->invalidateRect(rBBox, awt::InvalidateStyle::UPDATE);
}




void PresenterSlideSorter::SetHorizontalOffset (const double nXOffset)
{
    if (mpLayout->SetHorizontalOffset(nXOffset))
    {
        mxPreviewCache->setVisibleRange(
            mpLayout->GetFirstVisibleSlideIndex(),
            mpLayout->GetLastVisibleSlideIndex());

        Paint();
    }
}




void PresenterSlideSorter::GotoSlide (const sal_Int32 nSlideIndex)
{
    mxSlideShowController->gotoSlideIndex(nSlideIndex);
    mpPresenterController->HideSlideSorter();
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

        // New canvas => new MouseOverManager.
        if (mpMouseOverManager.get() != NULL)
            mpMouseOverManager->SetCanvas(
                Reference<rendering::XSpriteCanvas>(mxCanvas, UNO_QUERY));

        // Tell the scrollbar about the canvas.
        if (mpHorizontalScrollBar.is())
            mpHorizontalScrollBar->SetCanvas(mxCanvas);
    }
    return mxCanvas.is();
}




void PresenterSlideSorter::ThrowIfDisposed (void)
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterSlideSorter has been already disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




//===== PresenterSlideSorter::Layout ==========================================

PresenterSlideSorter::Layout::Layout (
    const Orientation eOrientation,
    const ::rtl::Reference<PresenterScrollBar>& rpHorizontalScrollBar,
    const ::rtl::Reference<PresenterScrollBar>& rpVerticalScrollBar)
    : maInnerBorder(),
      maOuterBorder(),
      maViewport(),
      maPreviewSize(),
      mnHorizontalOffset(0),
      mnVerticalOffset(0),
      mnHorizontalGap(0),
      mnVerticalGap(0),
      mnRowCount(1),
      mnColumnCount(1),
      mnSlideCount(0),
      mnSlideIndexAtMouse(-1),
      mnFirstVisibleColumn(-1),
      mnLastVisibleColumn(-1),
      mnFirstVisibleRow(-1),
      mnLastVisibleRow(-1),
      mnScrollBarHeight(25),
      meOrientation(eOrientation),
      mpHorizontalScrollBar(rpHorizontalScrollBar),
      mpVerticalScrollBar(rpVerticalScrollBar)
{
}




void PresenterSlideSorter::Layout::Update (
    const awt::Rectangle& rWindowBox,
    const double nSlideAspectRatio)
{
    if (rWindowBox.Width <= 0 || rWindowBox.Height <= 0)
        return;

    const static sal_Int32 nFrameSize (10);
    maInnerBorder = geometry::RealRectangle2D(nFrameSize,nFrameSize,nFrameSize,nFrameSize);
    maOuterBorder = geometry::RealRectangle2D(50,50,50,50);
    const static double nMinGap(15);
    mnRowCount = 1;
    mnColumnCount = 6;

    maViewport = geometry::RealRectangle2D(
        maInnerBorder.X1 + maOuterBorder.X1,
        maInnerBorder.Y1 + maInnerBorder.Y1,
        rWindowBox.Width - maInnerBorder.X2 - maOuterBorder.X2,
        rWindowBox.Height - maInnerBorder.Y2 - mnScrollBarHeight - maOuterBorder.Y2);
    const double nAvailableWidth (maViewport.X2 - maViewport.X1);
    const double nAvailableHeight (maViewport.Y2 - maViewport.Y1);
    const double nMaxWidth (::std::max(nAvailableWidth  / mnColumnCount - nMinGap, 50.0));
    const double nMaxHeight (::std::max(nAvailableHeight / mnRowCount - nMinGap, 30.0));

    // Calculate the preview size.
    if ((double(nMaxWidth) / double(nMaxHeight)) <= nSlideAspectRatio)
    {
        maPreviewSize.Width = sal_Int32(nMaxWidth);
        maPreviewSize.Height = sal_Int32(nMaxWidth / nSlideAspectRatio);
    }
    else
    {
        maPreviewSize.Width = sal_Int32(nMaxHeight * nSlideAspectRatio);
        maPreviewSize.Height = sal_Int32(nMaxHeight);
    }
    if (maPreviewSize.Width <= 0 || maPreviewSize.Height <= 0)
        return;

    // Calculate the numbers of visible rows and columns.
    mnColumnCount = sal_Int32((nAvailableWidth+nMinGap) / (maPreviewSize.Width + nMinGap));
    if (mnColumnCount < 1)
        mnColumnCount = 1;
    mnRowCount = sal_Int32((nAvailableHeight+nMinGap) / (maPreviewSize.Height + nMinGap));
    if (mnRowCount < 1)
        mnRowCount = 1;

    // Calculate the gaps between adjacent previews.
    if (mnColumnCount >= 2)
        mnHorizontalGap = (nAvailableWidth - mnColumnCount*maPreviewSize.Width) / (mnColumnCount-1);
    else
        mnHorizontalGap = 0;
    if (mnRowCount >= 2)
        mnVerticalGap = (nAvailableHeight - mnRowCount*maPreviewSize.Height) / (mnRowCount-1);
    else
        mnVerticalGap = 0;

    // Reset the offset.
    if (meOrientation == Horizontal)
    {
        mnVerticalOffset = -(nAvailableHeight
            - mnRowCount*maPreviewSize.Height - (mnRowCount-1)*mnVerticalGap)
            / 2;
        mnHorizontalOffset = 0;
    }
    else
    {
        mnVerticalOffset = 0;
        mnHorizontalOffset = -(nAvailableWidth
            - mnColumnCount*maPreviewSize.Width - (mnColumnCount-1)*mnHorizontalGap)
            / 2;
    }

    SetupVisibleArea();
    UpdateScrollBars();
}




void PresenterSlideSorter::Layout::SetupVisibleArea (void)
{
    geometry::RealPoint2D aPoint (0,0);
    if (meOrientation == Horizontal)
    {
        mnFirstVisibleColumn = ::std::max(
            sal_Int32(0),
            sal_Int32((aPoint.X + mnHorizontalOffset + mnHorizontalGap)
                / (maPreviewSize.Width+mnHorizontalGap)));
        mnFirstVisibleRow = 0;
    }
    else
    {
        mnFirstVisibleColumn = 0;
        mnFirstVisibleRow = ::std::max(
            sal_Int32(0),
            sal_Int32((aPoint.Y + mnVerticalOffset + mnVerticalGap)
                / (maPreviewSize.Height+mnVerticalGap)));
    }

    aPoint = geometry::RealPoint2D(maViewport.X2-maViewport.X1, maViewport.Y2-maViewport.Y1);
    if (meOrientation == Horizontal)
    {
        mnLastVisibleColumn = sal_Int32((aPoint.X + mnHorizontalOffset)
            / (maPreviewSize.Width+mnHorizontalGap));
        mnLastVisibleRow = mnRowCount - 1;
    }
    else
    {
        mnLastVisibleColumn = mnColumnCount - 1;
        mnLastVisibleRow = sal_Int32((aPoint.Y + mnVerticalOffset)
            / (maPreviewSize.Height+mnVerticalGap));
    }
}




sal_Int32 PresenterSlideSorter::Layout::GetSlideIndexForPosition (
    const css::geometry::RealPoint2D& rPoint)
{
    if (rPoint.X < maViewport.X1
        || rPoint.X > maViewport.X2
        || rPoint.Y < maViewport.Y1
        || rPoint.Y > maViewport.Y2)
    {
        return -1;
    }

    const double nX (rPoint.X - maViewport.X1 + mnHorizontalOffset);
    const double nY (rPoint.Y - maViewport.Y1 + mnVerticalOffset);
    const sal_Int32 nColumn (sal_Int32(
        (nX + mnHorizontalGap/2.0) / (maPreviewSize.Width+mnHorizontalGap)));
    const sal_Int32 nRow (sal_Int32(
        (nY + mnVerticalGap/2.0) / (maPreviewSize.Height+mnVerticalGap)));

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
    const sal_Int32 nRelativeVerticalPosition)
{
    sal_Int32 nColumn (GetColumn(nSlideIndex));
    sal_Int32 nRow (GetRow(nSlideIndex));

    geometry::RealPoint2D aPosition(
        maViewport.X1 + nColumn*(maPreviewSize.Width+mnHorizontalGap) - mnHorizontalOffset,
        maViewport.Y1 + nRow*(maPreviewSize.Height+mnVerticalGap) - mnVerticalOffset);

    if (nRelativeHorizontalPosition >= 0)
        if (nRelativeHorizontalPosition > 0)
            aPosition.X += maPreviewSize.Width;
        else
            aPosition.X += maPreviewSize.Width / 2.0;
    if (nRelativeVerticalPosition >= 0)
        if (nRelativeVerticalPosition > 0)
            aPosition.Y += maPreviewSize.Height;
        else
            aPosition.Y += maPreviewSize.Height / 2.0;

    return aPosition;
}


namespace { double sqr (const double nValue) { return nValue*nValue; } }

double PresenterSlideSorter::Layout::GetDistanceFromCenter (
    const css::geometry::RealPoint2D& rPoint,
    const sal_Int32 nSlideIndex,
    const DistanceType eDistanceType)
{
    const geometry::RealPoint2D aCenter (GetPoint(nSlideIndex, 0,0));

    switch (eDistanceType)
    {
        case Norm1:
        default:
            return ::std::max(
                fabs(rPoint.X - aCenter.X) / maPreviewSize.Width * 2,
                fabs(rPoint.Y - aCenter.Y) / maPreviewSize.Height * 2);

        case Norm2:
            return sqrt(
                sqr((rPoint.X - aCenter.X) / maPreviewSize.Width * 2)
                    + sqr((rPoint.Y - aCenter.Y) / maPreviewSize.Height * 2));

        case XOnly:
            return fabs(rPoint.X - aCenter.X) / maPreviewSize.Width * 2;

        case YOnly:
            return fabs(rPoint.Y - aCenter.Y) / maPreviewSize.Height * 2;
    }
}




awt::Rectangle PresenterSlideSorter::Layout::GetBoundingBox (const sal_Int32 nSlideIndex)
{
    const geometry::RealPoint2D aPosition(GetPoint(nSlideIndex, 0, 0));
    return awt::Rectangle (
        sal_Int32(floor(aPosition.X)),
        sal_Int32(floor(aPosition.Y)),
        sal_Int32(ceil(aPosition.X + maPreviewSize.Width) - floor(aPosition.X)),
        sal_Int32(ceil(aPosition.Y + maPreviewSize.Height) - floor(aPosition.Y)));
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




sal_Int32 PresenterSlideSorter::Layout::GetFirstVisibleSlideIndex (void)
{
    return GetIndex(mnFirstVisibleRow, mnFirstVisibleColumn);
}




sal_Int32 PresenterSlideSorter::Layout::GetLastVisibleSlideIndex (void)
{
    return ::std::min(
        GetIndex(mnLastVisibleRow, mnLastVisibleColumn),
        mnSlideCount);
}




bool PresenterSlideSorter::Layout::SetHorizontalOffset (const double nOffset)
{
    if (mnHorizontalOffset != nOffset)
    {
        mnHorizontalOffset = nOffset;
        SetupVisibleArea();
        UpdateScrollBars();
        return true;
    }
    else
        return false;
}




void PresenterSlideSorter::Layout::UpdateScrollBars (void)
{
    sal_Int32 nTotalColumnCount (0);
    sal_Int32 nTotalRowCount (0);
    if (meOrientation == Horizontal)
    {
        nTotalColumnCount = sal_Int32(ceil(double(mnSlideCount) / double(mnRowCount)));
        nTotalRowCount = mnRowCount;
    }
    else
    {
        nTotalColumnCount = mnColumnCount;
        nTotalRowCount = sal_Int32(ceil(double(mnSlideCount) / double(mnColumnCount)));
    }

    if (mpHorizontalScrollBar.get() != NULL)
    {
        mpHorizontalScrollBar->SetTotalSize(
            nTotalColumnCount * maPreviewSize.Width + (nTotalColumnCount-1) * mnHorizontalGap);
        mpHorizontalScrollBar->SetThumbPosition(mnHorizontalOffset);
        mpHorizontalScrollBar->SetThumbSize(
            mnColumnCount * (maPreviewSize.Width + mnHorizontalGap) - mnHorizontalGap);
    }
    if (mpVerticalScrollBar.get() != NULL)
    {
        mpVerticalScrollBar->SetTotalSize(
            nTotalRowCount * maPreviewSize.Height + (nTotalRowCount-1) * mnVerticalGap);
        mpVerticalScrollBar->SetThumbPosition(mnVerticalOffset);
        mpVerticalScrollBar->SetThumbSize(
            mnRowCount * (maPreviewSize.Height + mnVerticalGap) - mnVerticalGap);
    }


    // Place the scroll bars.
    if (mpHorizontalScrollBar.get() != NULL)
    {
        mpHorizontalScrollBar->SetPosSize(geometry::RealRectangle2D(
            maViewport.X1,
            maViewport.Y2 + maInnerBorder.Y2,
            maViewport.X2,
            maViewport.Y2 + maInnerBorder.Y2 + mnScrollBarHeight));
    }

    // No place yet for the vertical scroll bar.
}




sal_Int32 PresenterSlideSorter::Layout::GetIndex (const sal_Int32 nRow, const sal_Int32 nColumn)
{
    if (meOrientation == Horizontal)
        return nColumn * mnRowCount + nRow;
    else
        return nRow * mnColumnCount + nColumn;
}




sal_Int32 PresenterSlideSorter::Layout::GetRow (const sal_Int32 nSlideIndex)
{
    if (meOrientation == Horizontal)
        return nSlideIndex % mnRowCount;
    else
        return nSlideIndex / mnColumnCount;
}




sal_Int32 PresenterSlideSorter::Layout::GetColumn (const sal_Int32 nSlideIndex)
{
    if (meOrientation == Horizontal)
        return nSlideIndex / mnRowCount;
    else
        return nSlideIndex % mnColumnCount;
}




//===== PresenterSlideSorter::MouseOverManager ================================

PresenterSlideSorter::MouseOverManager::MouseOverManager (
    const Reference<XComponentContext>& rxContext,
    const Reference<container::XIndexAccess>& rxSlides,
    const Reference<frame::XModel>& rxModel)
    : MouseOverManagerInterfaceBase(m_aMutex),
      mxCanvas(),
      mxPreviewCache(),
      mnSlideIndex(-1),
      mnDistance(2),
      mxSprite(NULL),
      maSpriteSize(0,0),
      mbIsActive(true)
{
    Reference<lang::XMultiComponentFactory> xFactory (rxContext->getServiceManager(), UNO_QUERY);
    if (xFactory.is())
    {
        // Create the preview cache.
        mxPreviewCache = Reference<drawing::XSlidePreviewCache>(
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.drawing.PresenterPreviewCache"),
                rxContext),
            UNO_QUERY_THROW);
        mxPreviewCache->setDocumentSlides(rxSlides, rxModel);
        mxPreviewCache->addPreviewCreationNotifyListener(this);
    }
}




PresenterSlideSorter::MouseOverManager::~MouseOverManager (void)
{
    Reference<lang::XComponent> xComponent (mxPreviewCache, UNO_QUERY);
    mxPreviewCache = NULL;
    if (xComponent.is())
        xComponent->dispose();

    DisposeSprite();
}




void PresenterSlideSorter::MouseOverManager::DisposeSprite (void)
{
    if (mxSprite.is())
    {
        mxSprite->hide();
        if (mxCanvas.is())
            mxCanvas->updateScreen(sal_False);

        Reference<lang::XComponent> xComponent (mxSprite, UNO_QUERY);
        mxSprite = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }
}




void PresenterSlideSorter::MouseOverManager::SetCanvas (
    const Reference<rendering::XSpriteCanvas>& rxCanvas)
{
    mxCanvas = rxCanvas;
}




void PresenterSlideSorter::MouseOverManager::SetSlide (
    const sal_Int32 nSlideIndex,
    const awt::Rectangle& rBox,
    const double nDistance)
{
    mnDistance = nDistance;
    if ( ! mxSprite.is()
        || nSlideIndex != mnSlideIndex
        || rBox.X != maBoundingBox.X
        || rBox.Y != maBoundingBox.Y)
    {
        DisposeSprite();
        mnSlideIndex = nSlideIndex;
        maBoundingBox = rBox;

        if (mnSlideIndex < 0)
            return;

        if ( ! mbIsActive)
            return;

        CreateSprite(nSlideIndex);
        if ( ! mxSprite.is())
            return;

        mxSprite->move(
            geometry::RealPoint2D(maBoundingBox.X, maBoundingBox.Y),
            rendering::ViewState(
                geometry::AffineMatrix2D(1,0,0, 0,1,0),
                NULL),
            rendering::RenderState(
                geometry::AffineMatrix2D(1,0,0, 0,1,0),
                NULL,
                Sequence<double>(3),
                rendering::CompositeOperation::SOURCE)
            );
    }

    if ( ! mxSprite.is())
        return;

    double nDownScale (maBoundingBox.Width / maSpriteSize.Width);
    double nUpScale (1.0);
    if (nDistance < 1.0)
        if (nDistance < 0.8)
            nUpScale = mnMaxScale;
        else
        {
            const double nWeight ((nDistance-0.8) / 0.2);
            nUpScale = mnMinScale*nWeight + mnMaxScale*(1-nWeight);
        }

    double nScale (nDownScale * nUpScale);
    mxSprite->transform(
        geometry::AffineMatrix2D(
            nScale,0, - nScale*maSpriteSize.Width/2.0,
            0,nScale, - nScale*maSpriteSize.Height/2.0));

    mxCanvas->updateScreen(sal_False);
}




void PresenterSlideSorter::MouseOverManager::SetActiveState (const bool bIsActive)
{
    if (mbIsActive != bIsActive)
    {
        mbIsActive = bIsActive;
        if ( ! mbIsActive)
            DisposeSprite();
        else if (mnSlideIndex >= 0)
            SetSlide (mnSlideIndex, maBoundingBox, mnDistance);
    }
}




void PresenterSlideSorter::MouseOverManager::CreateSprite (const sal_Int32 nSlideIndex)
{
    if ( ! mxPreviewCache.is())
        return;
    maSpriteSize = geometry::RealSize2D(maBoundingBox.Width*2, maBoundingBox.Height*2);
    mxPreviewCache->setPreviewSize(geometry::IntegerSize2D(
        sal_Int32(maSpriteSize.Width), sal_Int32(maSpriteSize.Height)));
    if (mxCanvas.is() && ! mxSprite.is())
    {
        mxSprite = mxCanvas->createCustomSprite(maSpriteSize);
        if (mxSprite.is())
        {
            mxSprite->setAlpha(1.0);
            mxSprite->setPriority(+10);
            mxSprite->show();
        }
    }
    if (mxSprite.is())
    {
        Reference<rendering::XCanvas> xSpriteCanvas (mxSprite->getContentCanvas());
        if (xSpriteCanvas.is())
        {
            Reference<rendering::XBitmap> xBitmap (
                mxPreviewCache->getSlidePreview(nSlideIndex, xSpriteCanvas));
            if (xBitmap.is())
            {
                rendering::ViewState aViewState(
                    geometry::AffineMatrix2D(1,0,0, 0,1,0),
                    NULL);

                rendering::RenderState aRenderState (
                    geometry::AffineMatrix2D(1,0,0, 0,1,0),
                    NULL,
                    Sequence<double>(3),
                    rendering::CompositeOperation::SOURCE);

                xSpriteCanvas->drawBitmap(xBitmap, aViewState, aRenderState);
            }
        }
    }
}




void SAL_CALL PresenterSlideSorter::MouseOverManager::notifyPreviewCreation (
    sal_Int32 nSlideIndex)
    throw(css::uno::RuntimeException)
{
    if (nSlideIndex == mnSlideIndex)
    {
        CreateSprite(mnSlideIndex);

        Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
        if (xSpriteCanvas.is())
            xSpriteCanvas->updateScreen(sal_False);
    }
}

} } // end of namespace ::sdext::presenter
