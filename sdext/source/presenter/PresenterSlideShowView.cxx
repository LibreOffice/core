/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterSlideShowView.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:04:41 $
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

#include "PresenterSlideShowView.hxx"

#include "PresenterGeometryHelper.hxx"
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/CanvasFeature.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//===== Service ===============================================================

OUString PresenterSlideShowView::getImplementationName_static (void)
{
    return OUString::createFromAscii("com.sun.star.comp.Presenter.SlideShowView");
}




Sequence<OUString> PresenterSlideShowView::getSupportedServiceNames_static (void)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.presenter.SlideShowView"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




Reference<XInterface> PresenterSlideShowView::Create (
    const Reference<XComponentContext>& rxContext)
    SAL_THROW((Exception))
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterSlideShowView(rxContext)));
}




//===== PresenterSlideShowView ================================================

PresenterSlideShowView::PresenterSlideShowView (const Reference<XComponentContext>& rxContext)
    : PresenterSlideShowViewInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mxViewId(),
      mxController(),
      mxSlideShowController(),
      mxSlideShow(),
      mxCanvas(),
      mxViewCanvas(),
      mxPointer(),
      mxWindow(),
      mxViewWindow(),
      mxBackgroundPolyPolygon(),
      mnPageAspectRatio(1),
      maBroadcaster(m_aMutex),
      maBackgroundColor(0x00ffffff),
      mbIsInModifyNotification(false),
      mbIsForcedPaintPending(false),
      mbIsPaintPending(true)
{
}




void SAL_CALL PresenterSlideShowView::initialize (const Sequence<Any>& rArguments)
    throw (Exception, RuntimeException)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    if (rArguments.getLength() == 5)
    {
        try
        {
            // Extract the given arguments.
            if ( ! (rArguments[0] >>= mxViewId))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterSlideShowView: invalid view id"),
                    static_cast<XWeak*>(this),
                    0);
            }
            if ( ! (rArguments[1] >>= mxController))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterSlideShowView: invalid XController"),
                    static_cast<XWeak*>(this),
                    1);
            }
            if ( ! (rArguments[2] >>= mxSlideShowController))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterSlideShowView: invalid XSlideShowController"),
                    static_cast<XWeak*>(this),
                    2);
            }
            if ( ! (rArguments[3] >>= mnPageAspectRatio))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterSlideShowView: invalid aspect ratio"),
                    static_cast<XWeak*>(this),
                    3);
            }
            if ( ! (rArguments[4] >>= maBackgroundColor))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterSlideShowView: invalid background color"),
                    static_cast<XWeak*>(this),
                    4);
            }

            mxSlideShow = Reference<presentation::XSlideShow> (
                mxSlideShowController->getSlideShow(), UNO_QUERY_THROW);

            // Use view id and controller to retrieve window and canvas from
            // configuration controller.
            Reference<XControllerManager> xCM (mxController, UNO_QUERY_THROW);
            Reference<XConfigurationController> xCC (xCM->getConfigurationController());

            if (xCC.is())
            {
                Reference<XPane> xPane (xCC->getResource(mxViewId->getAnchor()), UNO_QUERY_THROW);

                mxWindow = xPane->getWindow();
                mxCanvas = xPane->getCanvas();
            }

            if (mxWindow.is())
            {
                // Register listeners at window.
                mxWindow->addPaintListener(this);
                mxWindow->addMouseListener(this);
                mxWindow->addMouseMotionListener(this);
                mxWindow->addWindowListener(this);

                mxWindow->setVisible(sal_True);

                Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
                if (xPeer.is())
                    xPeer->setBackground(util::Color(0xff000000));
            }

            mxViewWindow = CreateViewWindow(mxWindow);
            if (mxViewWindow.is())
                Resize();

            // Create a canvas for the view window.
            Reference<lang::XMultiComponentFactory> xFactory (
                mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
            Reference<drawing::XPresenterHelper> xPresenterHelper(
                xFactory->createInstanceWithContext(
                    OUString::createFromAscii("com.sun.star.comp.Draw.PresenterHelper"),
                    mxComponentContext),
                UNO_QUERY_THROW);
            Reference<XPane> xParentPane (
                xCC->getResource(mxViewId->getAnchor()->getAnchor()),
                UNO_QUERY_THROW);
            mxViewCanvas = xPresenterHelper->createSharedCanvas(
                Reference<rendering::XSpriteCanvas>(xParentPane->getCanvas(), UNO_QUERY),
                xParentPane->getWindow(),
                xParentPane->getCanvas(),
                xParentPane->getWindow(),
                mxViewWindow);
        }
        catch (RuntimeException&)
        {
            OSL_ENSURE(false, "PresenterSlideShowView::initialize(): caught exception");
            disposing();
            throw;
        }
    }
    else
    {
        throw RuntimeException(
            OUString::createFromAscii("PresenterSlideShowView: invalid number of arguments"),
                static_cast<XWeak*>(this));
    }
}




PresenterSlideShowView::~PresenterSlideShowView (void)
{
}




void PresenterSlideShowView::disposing (void)
{
    if (mxWindow.is())
    {
        mxWindow->removePaintListener(this);
        mxWindow->removeMouseListener(this);
        mxWindow->removeMouseMotionListener(this);
        mxWindow->removeWindowListener(this);
        mxWindow = NULL;
    }
    if (mxViewWindow.is())
    {
        Reference<XComponent> xComponent (mxViewWindow, UNO_QUERY);
        mxViewWindow = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }
    if (mxViewCanvas.is())
    {
        Reference<XComponent> xComponent (mxViewCanvas, UNO_QUERY);
        mxViewCanvas = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }
    lang::EventObject aEvent;
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
          = maBroadcaster.getContainer(getCppuType((Reference<awt::XPaintListener>*)NULL));
    if (pIterator != NULL)
        pIterator->disposeAndClear(aEvent);
    // Do this for
    // XModifyListener,XMouseListener,XMouseMotionListener,XWindowListener?
    mxComponentContext = NULL;
}




Reference<rendering::XSpriteCanvas> SAL_CALL PresenterSlideShowView::getCanvas (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    return Reference<rendering::XSpriteCanvas>(mxViewCanvas, UNO_QUERY);
}




void SAL_CALL PresenterSlideShowView::clear (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    mbIsForcedPaintPending = false;
    mbIsPaintPending = false;

    if (mxViewCanvas.is() && mxViewWindow.is())
    {
        // Create a polygon for the window outline.
        awt::Rectangle aViewWindowBox (mxViewWindow->getPosSize());
        Reference<rendering::XPolyPolygon2D> xPolygon (PresenterGeometryHelper::CreatePolygon(
            awt::Rectangle(0,0, aViewWindowBox.Width,aViewWindowBox.Height),
            mxViewCanvas->getDevice()));

        rendering::ViewState aViewState (
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL);
        double aColor[3] = {0,0,0};
        rendering::RenderState aRenderState(
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL,
            Sequence<double>(aColor,3),
            rendering::CompositeOperation::SOURCE);
        mxViewCanvas->fillPolyPolygon(xPolygon, aViewState, aRenderState);
    }
}




geometry::AffineMatrix2D SAL_CALL PresenterSlideShowView::getTransformation (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    if (mxViewWindow.is())
    {
        // When the mbIsInModifyNotification is set then a slightly modifed
        // version of the transformation is returned in order to get past
        // optimizations the avoid updates when the transformation is
        // unchanged (when the window size changes then due to the constant
        // aspect ratio the size of the preview may remain the same while
        // the position changes.  The position, however, is repesented by
        // the position of the view window.  This transformation is given
        // relative to the view window and therefore does not contain the
        // position.)
        const awt::Rectangle aWindowBox = mxViewWindow->getPosSize();
        return geometry::AffineMatrix2D(
            aWindowBox.Width-1, 0, (mbIsInModifyNotification ? 1 : 0),
            0, aWindowBox.Height-1, 0);
    }
    else
    {
        return geometry::AffineMatrix2D(1,0,0, 0,1,0);
    }
}




void SAL_CALL PresenterSlideShowView::addTransformationChangedListener(
    const Reference<util::XModifyListener>& rxListener)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    maBroadcaster.addListener(
        getCppuType((Reference<util::XModifyListener>*)NULL),
        rxListener);
}




void SAL_CALL PresenterSlideShowView::removeTransformationChangedListener(
    const Reference<util::XModifyListener>& rxListener)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    maBroadcaster.removeListener(
        getCppuType((Reference<util::XModifyListener>*)NULL),
        rxListener);
}




void SAL_CALL PresenterSlideShowView::addPaintListener(
    const Reference<awt::XPaintListener>& rxListener)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    maBroadcaster.addListener(
        getCppuType((Reference<awt::XPaintListener>*)NULL),
        rxListener);
}




void SAL_CALL PresenterSlideShowView::removePaintListener(
    const Reference<awt::XPaintListener>& rxListener)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    maBroadcaster.removeListener(
        getCppuType((Reference<awt::XPaintListener>*)NULL),
        rxListener);
}




void SAL_CALL PresenterSlideShowView::addMouseListener(
    const Reference<awt::XMouseListener>& rxListener)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    maBroadcaster.addListener(
        getCppuType((Reference<awt::XMouseListener>*)NULL),
        rxListener);
}




void SAL_CALL PresenterSlideShowView::removeMouseListener(
    const Reference<awt::XMouseListener>& rxListener)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    maBroadcaster.removeListener(
        getCppuType((Reference<awt::XMouseListener>*)NULL),
        rxListener);
}




void SAL_CALL PresenterSlideShowView::addMouseMotionListener(
    const Reference<awt::XMouseMotionListener>& rxListener)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    maBroadcaster.addListener(
        getCppuType((Reference<awt::XMouseMotionListener>*)NULL),
        rxListener);
}




void SAL_CALL PresenterSlideShowView::removeMouseMotionListener(
    const Reference<awt::XMouseMotionListener>& rxListener)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    maBroadcaster.removeListener(
        getCppuType((Reference<awt::XMouseMotionListener>*)NULL),
        rxListener);
}




void SAL_CALL PresenterSlideShowView::setMouseCursor(::sal_Int16 nPointerShape)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    // Create a pointer when it does not yet exist.
    if ( ! mxPointer.is())
    {
        Reference<lang::XMultiServiceFactory> xFactory (
            mxComponentContext, UNO_QUERY);
        if (xFactory.is())
            mxPointer = Reference<awt::XPointer>(
                xFactory->createInstance(OUString::createFromAscii("com.sun.star.awt.Pointer")),
                UNO_QUERY);
    }

    // Set the pointer to the given shape and the window(peer) to the
    // pointer.
    Reference<awt::XWindowPeer> xPeer (mxViewWindow, UNO_QUERY);
    if (mxPointer.is() && xPeer.is())
    {
        mxPointer->setType(nPointerShape);
        xPeer->setPointer(mxPointer);
    }
}




//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterSlideShowView::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxViewWindow)
        mxViewWindow = NULL;
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterSlideShowView::windowPaint (const awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    awt::Rectangle aViewWindowBox (mxViewWindow->getPosSize());
    if (aViewWindowBox.Width <= 0 || aViewWindowBox.Height <= 0)
        return;

    // Paint the background
    if (mxCanvas.is() && mxBackgroundPolyPolygon.is())
    {
        const rendering::ViewState aViewState(
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            PresenterGeometryHelper::CreatePolygon(rEvent.UpdateRect, mxCanvas->getDevice()));

        Sequence<double> aBackgroundColor(3);
        aBackgroundColor[0] = ((maBackgroundColor >> 16) & 0x0ff) / 255.0;
        aBackgroundColor[1] = ((maBackgroundColor >> 8) & 0x0ff) / 255.0;
        aBackgroundColor[2] = ((maBackgroundColor >> 0) & 0x0ff) / 255.0;

        rendering::RenderState aRenderState (
            geometry::AffineMatrix2D(1, 0, 0, 0, 1, 0),
            NULL,
            aBackgroundColor,
            rendering::CompositeOperation::SOURCE);

        mxCanvas->fillPolyPolygon(mxBackgroundPolyPolygon, aViewState, aRenderState);
    }

    // Forward windowpaint to listeners.
    awt::PaintEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(getCppuType((Reference<awt::XPaintListener>*)NULL));
    if (pIterator != NULL)
    {
        pIterator->notifyEach(&awt::XPaintListener::windowPaint, aEvent);
    }

    if (mbIsForcedPaintPending)
        ForceRepaint();

    // Schedule the processing of the events caused by the previous notifies.
    if (mxSlideShow.is())
    {
        double nTimeToNextUpdateCall (0);
        mxSlideShow->update(nTimeToNextUpdateCall);
        // We do not call update regularly, so we ignore the returned values.
    }

    // Finally, in double buffered environments, request the changes to be
    // made visible.
    Reference<rendering::XSpriteCanvas> mxSpriteCanvas (mxCanvas, UNO_QUERY);
    if (mxSpriteCanvas.is())
        mxSpriteCanvas->updateScreen(sal_True);
}




//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterSlideShowView::mousePressed (const awt::MouseEvent& rEvent)
    throw (RuntimeException)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(getCppuType((Reference<awt::XMouseListener>*)NULL));
    if (pIterator != NULL)
    {
        pIterator->notifyEach(&awt::XMouseListener::mousePressed, aEvent);
    }
}




void SAL_CALL PresenterSlideShowView::mouseReleased (const awt::MouseEvent& rEvent)
    throw (RuntimeException)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(getCppuType((Reference<awt::XMouseListener>*)NULL));
    if (pIterator != NULL)
    {
        pIterator->notifyEach(&awt::XMouseListener::mouseReleased, aEvent);
    }
}




void SAL_CALL PresenterSlideShowView::mouseEntered (const awt::MouseEvent& rEvent)
    throw (RuntimeException)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(getCppuType((Reference<awt::XMouseListener>*)NULL));
    if (pIterator != NULL)
    {
        pIterator->notifyEach(&awt::XMouseListener::mouseEntered, aEvent);
    }
}




void SAL_CALL PresenterSlideShowView::mouseExited (const awt::MouseEvent& rEvent)
    throw (RuntimeException)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(getCppuType((Reference<awt::XMouseListener>*)NULL));
    if (pIterator != NULL)
    {
        pIterator->notifyEach(&awt::XMouseListener::mouseExited, aEvent);
    }
}




//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterSlideShowView::mouseDragged (const awt::MouseEvent& rEvent)
    throw (RuntimeException)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(getCppuType((Reference<awt::XMouseMotionListener>*)NULL));
    if (pIterator != NULL)
    {
        pIterator->notifyEach(&awt::XMouseMotionListener::mouseDragged, aEvent);
    }
}




void SAL_CALL PresenterSlideShowView::mouseMoved (const awt::MouseEvent& rEvent)
    throw (RuntimeException)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(getCppuType((Reference<awt::XMouseMotionListener>*)NULL));
    if (pIterator != NULL)
    {
        pIterator->notifyEach(&awt::XMouseMotionListener::mouseMoved, aEvent);
    }
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterSlideShowView::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;

    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    Resize();
}





void SAL_CALL PresenterSlideShowView::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    if ( ! mbIsPaintPending)
        mbIsForcedPaintPending = true;
}




void SAL_CALL PresenterSlideShowView::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    Resize();
}




void SAL_CALL PresenterSlideShowView::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




//----- XView -----------------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterSlideShowView::getResourceId (void)
    throw(RuntimeException)
{
    return mxViewId;
}




sal_Bool SAL_CALL PresenterSlideShowView::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}




//-----------------------------------------------------------------------------

Reference<awt::XWindow> PresenterSlideShowView::CreateViewWindow (
    const Reference<awt::XWindow>& rxParentWindow)
{
    Reference<awt::XWindow> xViewWindow;
    try
    {
        Reference<lang::XMultiComponentFactory> xFactory (mxComponentContext->getServiceManager());
        if ( ! xFactory.is())
            return xViewWindow;

        Reference<awt::XToolkit> xToolkit (
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.awt.Toolkit"),
                mxComponentContext),
            UNO_QUERY_THROW);
        awt::WindowDescriptor aWindowDescriptor (
            awt::WindowClass_CONTAINER,
            OUString(),
            Reference<awt::XWindowPeer>(rxParentWindow,UNO_QUERY_THROW),
            -1, // parent index not available
            awt::Rectangle(0,0,10,10),
            awt::WindowAttribute::SIZEABLE
                | awt::WindowAttribute::MOVEABLE
                | awt::WindowAttribute::NODECORATION);
        xViewWindow = Reference<awt::XWindow>(
            xToolkit->createWindow(aWindowDescriptor),UNO_QUERY_THROW);

        // Make the background transparent.  The slide show paints its own background.
        Reference<awt::XWindowPeer> xPeer (xViewWindow, UNO_QUERY_THROW);
        if (xPeer.is())
        {
            xPeer->setBackground(0xff000000);
        }

        xViewWindow->setVisible(sal_True);
    }
    catch (RuntimeException&)
    {
    }
    return xViewWindow;
}




void PresenterSlideShowView::Resize (void)
{
    if ( ! mxWindow.is() || ! mxViewWindow.is())
        return;

    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    awt::Rectangle aViewWindowBox;
    if (aWindowBox.Height > 0)
    {
        const double nWindowAspectRatio (
            double(aWindowBox.Width) / double(aWindowBox.Height));
        if (nWindowAspectRatio > mnPageAspectRatio)
        {
            // Slides will be painted with the full parent window height.
            aViewWindowBox.Width = sal_Int32(aWindowBox.Height * mnPageAspectRatio + 0.5);
            aViewWindowBox.Height = aWindowBox.Height;
            aViewWindowBox.X = (aWindowBox.Width - aViewWindowBox.Width) / 2;
            aViewWindowBox.Y = 0;
        }
        else
        {
            // Slides will be painted with the full parent window width.
            aViewWindowBox.Width = aWindowBox.Width;
            aViewWindowBox.Height = sal_Int32(aWindowBox.Width / mnPageAspectRatio + 0.5);
            aViewWindowBox.X = 0;
            aViewWindowBox.Y = (aWindowBox.Height - aViewWindowBox.Height) / 2;
        }
        mxViewWindow->setPosSize(
            aViewWindowBox.X,
            aViewWindowBox.Y,
            aViewWindowBox.Width,
            aViewWindowBox.Height,
            awt::PosSize::POSSIZE);
    }

    // Clear the background polygon so that on the next paint it is created
    // for the new size.
    mxBackgroundPolyPolygon = CreateBackgroundPolyPolygon();


    // Due to constant aspect ratio resizing may lead a preview that changes
    // its position but not its size.  This invalidates the back buffer and
    // we have to enforce a complete repaint.
    if ( ! mbIsPaintPending)
        mbIsForcedPaintPending = true;
}




void PresenterSlideShowView::ForceRepaint (void)
{
    // The modify listeners are called twice.  The first time the
    // mbIsInModifyNotification flag leads to a slightly modified
    // transformation returned by getTransformation().  With this hack we
    // get past the optimization that otherwise prevents a proper repaint
    // when the size of the preview does not change.
    mbIsInModifyNotification = true;
    try
    {
        lang::EventObject aEvent;
        aEvent.Source = static_cast<XWeak*>(this);
        ::cppu::OInterfaceContainerHelper* pIterator
              = maBroadcaster.getContainer(getCppuType((Reference<util::XModifyListener>*)NULL));
        if (pIterator != NULL)
            pIterator->notifyEach(&util::XModifyListener::modified, aEvent);
    }
    catch (Exception&)
    {
    }
    mbIsInModifyNotification = false;

    lang::EventObject aEvent;
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
          = maBroadcaster.getContainer(getCppuType((Reference<util::XModifyListener>*)NULL));
    if (pIterator != NULL)
        pIterator->notifyEach(&util::XModifyListener::modified, aEvent);
}




Reference<rendering::XPolyPolygon2D>
    PresenterSlideShowView::CreateBackgroundPolyPolygon (void) const
{
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    const awt::Rectangle aViewWindowBox (mxViewWindow->getPosSize());
    ::std::vector<awt::Rectangle> aBackgroundBoxes;
    if (aWindowBox.Height == aViewWindowBox.Height)
    {
        // Paint two boxes to the left and right of the view window.
        aBackgroundBoxes.push_back(
            awt::Rectangle(
                0,
                0,
                aViewWindowBox.X,
                aWindowBox.Height));
        aBackgroundBoxes.push_back(
            awt::Rectangle(
                aViewWindowBox.X + aViewWindowBox.Width,
                0,
                aWindowBox.Width - aViewWindowBox.X - aViewWindowBox.Width,
                aWindowBox.Height));
    }
    else
    {
        // Paint two boxes above and below the view window.
        aBackgroundBoxes.push_back(
            awt::Rectangle(
                0,
                0,
                aWindowBox.Width,
                aViewWindowBox.Y));
        aBackgroundBoxes.push_back(
            awt::Rectangle(
                0,
                aViewWindowBox.Y + aViewWindowBox.Height,
                aWindowBox.Width,
                aWindowBox.Height - aViewWindowBox.Y - aViewWindowBox.Height));
    }

    return PresenterGeometryHelper::CreatePolygon(aBackgroundBoxes, mxCanvas->getDevice());
}




void PresenterSlideShowView::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString::createFromAscii("PresenterSlideShowView object has already been disposed"),
            static_cast<uno::XWeak*>(this));
    }
}


} } // end of namespace ::sd::presenter
