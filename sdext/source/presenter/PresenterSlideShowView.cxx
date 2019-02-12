
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

#include "PresenterSlideShowView.hxx"
#include <vcl/svapp.hxx>
#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterHelper.hxx"
#include "PresenterPaneContainer.hxx"
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/Pointer.hpp>
#include <com/sun/star/awt/Toolkit.hpp>
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
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sdext { namespace presenter {

//===== PresenterSlideShowView ================================================

PresenterSlideShowView::PresenterSlideShowView (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
    const css::uno::Reference<css::frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterSlideShowViewInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mpPresenterController(rpPresenterController),
      mxViewId(rxViewId),
      mxController(rxController),
      mxSlideShowController(PresenterHelper::GetSlideShowController(rxController)),
      mxSlideShow(),
      mxCanvas(),
      mxViewCanvas(),
      mxPointer(),
      mxWindow(),
      mxViewWindow(),
      mxTopPane(),
      mxPresenterHelper(),
      mxBackgroundPolygon1(),
      mxBackgroundPolygon2(),
      mbIsViewAdded(false),
      mnPageAspectRatio(28.0/21.0),
      maBroadcaster(m_aMutex),
      mpBackground(),
      mbIsForcedPaintPending(false),
      mbIsPaintPending(true),
      msClickToExitPresentationText(),
      msClickToExitPresentationTitle(),
      msTitleTemplate(),
      mbIsEndSlideVisible(false),
      mxCurrentSlide()
{
    if (mpPresenterController.get() != nullptr)
    {
        mnPageAspectRatio = mpPresenterController->GetSlideAspectRatio();
        mpBackground = mpPresenterController->GetViewBackground(mxViewId->getResourceURL());
    }
}

void PresenterSlideShowView::LateInit()
{
    mxSlideShow.set( mxSlideShowController->getSlideShow(), UNO_QUERY_THROW);
    Reference<lang::XComponent> xSlideShowComponent (mxSlideShow, UNO_QUERY);
    xSlideShowComponent->addEventListener(static_cast<awt::XWindowListener*>(this));

    Reference<lang::XMultiComponentFactory> xFactory (
        mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
    mxPresenterHelper.set (xFactory->createInstanceWithContext(
                   "com.sun.star.comp.Draw.PresenterHelper",
                   mxComponentContext),
               UNO_QUERY_THROW);

    // Use view id and controller to retrieve window and canvas from
    // configuration controller.
    Reference<XControllerManager> xCM (mxController, UNO_QUERY_THROW);
    Reference<XConfigurationController> xCC (xCM->getConfigurationController());

    if (xCC.is())
    {
        mxTopPane.set(xCC->getResource(mxViewId->getAnchor()->getAnchor()), UNO_QUERY);

        Reference<XPane> xPane (xCC->getResource(mxViewId->getAnchor()), UNO_QUERY_THROW);

        mxWindow = xPane->getWindow();
        mxCanvas = xPane->getCanvas();

        if (mxWindow.is())
        {
            mxWindow->addPaintListener(this);
            mxWindow->addWindowListener(this);
        }

        // The window does not have to paint a background.  We do
        // that ourself.
        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->setBackground(util::Color(0xff000000));
    }

    // Create a window for the actual slide show view.  It is places
    // centered and with maximal size inside the pane.
    mxViewWindow = CreateViewWindow(mxWindow);

    mxViewCanvas = CreateViewCanvas(mxViewWindow);

    if (mxViewWindow.is())
    {
        // Register listeners at window.
        mxViewWindow->addPaintListener(this);
        mxViewWindow->addMouseListener(this);
        mxViewWindow->addMouseMotionListener(this);
    }

    if (mxViewWindow.is())
        Resize();

    if (mxWindow.is())
        mxWindow->setVisible(true);

    // Add the new slide show view to the slide show.
    if (mxSlideShow.is() && ! mbIsViewAdded)
    {
        impl_addAndConfigureView();
        mbIsViewAdded = true;
    }

    // Read text for one past last slide.
    PresenterConfigurationAccess aConfiguration (
        mxComponentContext,
        PresenterConfigurationAccess::msPresenterScreenRootName,
        PresenterConfigurationAccess::READ_ONLY);
    aConfiguration.GetConfigurationNode(
        "Presenter/Views/CurrentSlidePreview/"
        "Strings/ClickToExitPresentationText/String")
        >>= msClickToExitPresentationText;
    aConfiguration.GetConfigurationNode(
        "Presenter/Views/CurrentSlidePreview/"
        "Strings/ClickToExitPresentationTitle/String")
        >>= msClickToExitPresentationTitle;
}

PresenterSlideShowView::~PresenterSlideShowView()
{
}

void PresenterSlideShowView::disposing()
{
    // Tell all listeners that we are disposed.
    lang::EventObject aEvent;
    aEvent.Source = static_cast<XWeak*>(this);

    ::cppu::OInterfaceContainerHelper* pIterator
          = maBroadcaster.getContainer(cppu::UnoType<lang::XEventListener>::get());
    if (pIterator != nullptr)
        pIterator->disposeAndClear(aEvent);

    // Do this for
    // XPaintListener, XModifyListener,XMouseListener,XMouseMotionListener,XWindowListener?

    if (mxWindow.is())
    {
        mxWindow->removePaintListener(this);
        mxWindow->removeMouseListener(this);
        mxWindow->removeMouseMotionListener(this);
        mxWindow->removeWindowListener(this);
        mxWindow = nullptr;
    }
    mxSlideShowController = nullptr;
    mxSlideShow = nullptr;
    if (mxViewCanvas.is())
    {
        Reference<XComponent> xComponent (mxViewCanvas, UNO_QUERY);
        mxViewCanvas = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }
    if (mxViewWindow.is())
    {
        Reference<XComponent> xComponent (mxViewWindow, UNO_QUERY);
        mxViewWindow = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }
    if (mxPointer.is())
    {
        Reference<XComponent> xComponent (mxPointer, UNO_QUERY);
        mxPointer = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }
    if (mxBackgroundPolygon1.is())
    {
        Reference<XComponent> xComponent (mxBackgroundPolygon1, UNO_QUERY);
        mxBackgroundPolygon1 = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }
    if (mxBackgroundPolygon2.is())
    {
        Reference<XComponent> xComponent (mxBackgroundPolygon2, UNO_QUERY);
        mxBackgroundPolygon2 = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    mxComponentContext = nullptr;
    mpPresenterController = nullptr;
    mxViewId = nullptr;
    mxController = nullptr;
    mxCanvas = nullptr;
    mpBackground.reset();
    msClickToExitPresentationText.clear();
    msClickToExitPresentationTitle.clear();
    msTitleTemplate.clear();
    mxCurrentSlide = nullptr;
}

//----- XDrawView -------------------------------------------------------------

void SAL_CALL PresenterSlideShowView::setCurrentPage (
    const css::uno::Reference<css::drawing::XDrawPage>& rxSlide)
{
    mxCurrentSlide = rxSlide;
    if (mpPresenterController.get() != nullptr
        && mxSlideShowController.is()
        && ! mpPresenterController->GetCurrentSlide().is()
        && ! mxSlideShowController->isPaused())
    {
        mbIsEndSlideVisible = true;
        Reference<awt::XWindowPeer> xPeer (mxViewWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->invalidate(awt::InvalidateStyle::NOTRANSPARENT);

        // For the end slide we use a special title, without the (n of m)
        // part.  Save the title template for the case that the user goes
        // backwards.
        PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
            mpPresenterController->GetPaneContainer()->FindViewURL(mxViewId->getResourceURL()));
        if (pDescriptor.get() != nullptr)
        {
            msTitleTemplate = pDescriptor->msTitleTemplate;
            pDescriptor->msTitleTemplate = msClickToExitPresentationTitle;
            mpPresenterController->UpdatePaneTitles();
        }
    }
    else if (mbIsEndSlideVisible)
    {
        mbIsEndSlideVisible = false;

        // Restore the title template.
        PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
            mpPresenterController->GetPaneContainer()->FindViewURL(mxViewId->getResourceURL()));
        if (pDescriptor.get() != nullptr)
        {
            pDescriptor->msTitleTemplate = msTitleTemplate;
            pDescriptor->msTitle.clear();
            mpPresenterController->UpdatePaneTitles();
        }
    }
}

css::uno::Reference<css::drawing::XDrawPage> SAL_CALL PresenterSlideShowView::getCurrentPage()
{
    return mxCurrentSlide;
}

//----- CachablePresenterView -------------------------------------------------

void PresenterSlideShowView::ReleaseView()
{
    if (mxSlideShow.is() && mbIsViewAdded)
    {
        mxSlideShow->removeView(this);
        mbIsViewAdded = false;
    }
}

//----- XSlideShowView --------------------------------------------------------

Reference<rendering::XSpriteCanvas> SAL_CALL PresenterSlideShowView::getCanvas()
{
    ThrowIfDisposed();

    return Reference<rendering::XSpriteCanvas>(mxViewCanvas, UNO_QUERY);
}

void SAL_CALL PresenterSlideShowView::clear()
{
    ThrowIfDisposed();
    mbIsForcedPaintPending = false;
    mbIsPaintPending = false;

    if (!(mxViewCanvas.is() && mxViewWindow.is()))
        return;

    // Create a polygon for the window outline.
    awt::Rectangle aViewWindowBox (mxViewWindow->getPosSize());
    Reference<rendering::XPolyPolygon2D> xPolygon (PresenterGeometryHelper::CreatePolygon(
        awt::Rectangle(0,0, aViewWindowBox.Width,aViewWindowBox.Height),
        mxViewCanvas->getDevice()));

    rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        nullptr);
    double const aColor[4] = {0,0,0,0};
    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        nullptr,
        Sequence<double>(aColor,4),
        rendering::CompositeOperation::SOURCE);
    mxViewCanvas->fillPolyPolygon(xPolygon, aViewState, aRenderState);
}

geometry::AffineMatrix2D SAL_CALL PresenterSlideShowView::getTransformation()
{
    ThrowIfDisposed();

    if (mxViewWindow.is())
    {
        // When the mbIsInModifyNotification is set then a slightly modified
        // version of the transformation is returned in order to get past
        // optimizations the avoid updates when the transformation is
        // unchanged (when the window size changes then due to the constant
        // aspect ratio the size of the preview may remain the same while
        // the position changes.  The position, however, is represented by
        // the position of the view window.  This transformation is given
        // relative to the view window and therefore does not contain the
        // position.)
        const awt::Rectangle aWindowBox = mxViewWindow->getPosSize();
        return geometry::AffineMatrix2D(
            aWindowBox.Width-1, 0, 0,
            0, aWindowBox.Height-1, 0);
    }
    else
    {
        return geometry::AffineMatrix2D(1,0,0, 0,1,0);
    }
}

geometry::IntegerSize2D SAL_CALL PresenterSlideShowView::getTranslationOffset()
{
    ThrowIfDisposed();
    return geometry::IntegerSize2D(0,0);
}

void SAL_CALL PresenterSlideShowView::addTransformationChangedListener(
    const Reference<util::XModifyListener>& rxListener)
{
    ThrowIfDisposed();
    maBroadcaster.addListener(
        cppu::UnoType<util::XModifyListener>::get(),
        rxListener);
}

void SAL_CALL PresenterSlideShowView::removeTransformationChangedListener(
    const Reference<util::XModifyListener>& rxListener)
{
    ThrowIfDisposed();
    maBroadcaster.removeListener(
        cppu::UnoType<util::XModifyListener>::get(),
        rxListener);
}

void SAL_CALL PresenterSlideShowView::addPaintListener(
    const Reference<awt::XPaintListener>& rxListener)
{
    ThrowIfDisposed();
    maBroadcaster.addListener(
        cppu::UnoType<awt::XPaintListener>::get(),
        rxListener);
}

void SAL_CALL PresenterSlideShowView::removePaintListener(
    const Reference<awt::XPaintListener>& rxListener)
{
    ThrowIfDisposed();
    maBroadcaster.removeListener(
        cppu::UnoType<awt::XPaintListener>::get(),
        rxListener);
}

void SAL_CALL PresenterSlideShowView::addMouseListener(
    const Reference<awt::XMouseListener>& rxListener)
{
    ThrowIfDisposed();
    maBroadcaster.addListener(
        cppu::UnoType<awt::XMouseListener>::get(),
        rxListener);
}

void SAL_CALL PresenterSlideShowView::removeMouseListener(
    const Reference<awt::XMouseListener>& rxListener)
{
    ThrowIfDisposed();
    maBroadcaster.removeListener(
        cppu::UnoType<awt::XMouseListener>::get(),
        rxListener);
}

void SAL_CALL PresenterSlideShowView::addMouseMotionListener(
    const Reference<awt::XMouseMotionListener>& rxListener)
{
    ThrowIfDisposed();
    maBroadcaster.addListener(
        cppu::UnoType<awt::XMouseMotionListener>::get(),
        rxListener);
}

void SAL_CALL PresenterSlideShowView::removeMouseMotionListener(
    const Reference<awt::XMouseMotionListener>& rxListener)
{
    ThrowIfDisposed();
    maBroadcaster.removeListener(
        cppu::UnoType<awt::XMouseMotionListener>::get(),
        rxListener);
}

void SAL_CALL PresenterSlideShowView::setMouseCursor(::sal_Int16 nPointerShape)
{
    ThrowIfDisposed();

    // Create a pointer when it does not yet exist.
    if ( ! mxPointer.is())
    {
        mxPointer = awt::Pointer::create(mxComponentContext);
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

awt::Rectangle SAL_CALL PresenterSlideShowView::getCanvasArea(  )
{
    if( mxViewWindow.is() && mxTopPane.is() )
        return mxPresenterHelper->getWindowExtentsRelative( mxViewWindow, mxTopPane->getWindow() );

    awt::Rectangle aRectangle;

    aRectangle.X = aRectangle.Y = aRectangle.Width = aRectangle.Height = 0;

    return aRectangle;
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterSlideShowView::disposing (const lang::EventObject& rEvent)
{
    if (rEvent.Source == mxViewWindow)
        mxViewWindow = nullptr;
    else if (rEvent.Source == mxSlideShow)
        mxSlideShow = nullptr;
}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterSlideShowView::windowPaint (const awt::PaintEvent& rEvent)
{
    // Deactivated views must not be painted.
    if ( ! mbIsPresenterViewActive)
        return;

    awt::Rectangle aViewWindowBox (mxViewWindow->getPosSize());
    if (aViewWindowBox.Width <= 0 || aViewWindowBox.Height <= 0)
        return;

    if (rEvent.Source == mxWindow)
        PaintOuterWindow(rEvent.UpdateRect);
    else if (mbIsEndSlideVisible)
        PaintEndSlide(rEvent.UpdateRect);
    else
        PaintInnerWindow(rEvent);
}

//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterSlideShowView::mousePressed (const awt::MouseEvent& rEvent)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(cppu::UnoType<awt::XMouseListener>::get());
    if (pIterator != nullptr)
    {
        pIterator->notifyEach(&awt::XMouseListener::mousePressed, aEvent);
    }

    // Only when the end slide is displayed we forward the mouse event to
    // the PresenterController so that it switches to the next slide and
    // ends the presentation.
    if (mbIsEndSlideVisible)
        if (mpPresenterController.get() != nullptr)
            mpPresenterController->HandleMouseClick(rEvent);
}

void SAL_CALL PresenterSlideShowView::mouseReleased (const awt::MouseEvent& rEvent)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(cppu::UnoType<awt::XMouseListener>::get());
    if (pIterator != nullptr)
    {
        pIterator->notifyEach(&awt::XMouseListener::mouseReleased, aEvent);
    }
}

void SAL_CALL PresenterSlideShowView::mouseEntered (const awt::MouseEvent& rEvent)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(cppu::UnoType<awt::XMouseListener>::get());
    if (pIterator != nullptr)
    {
        pIterator->notifyEach(&awt::XMouseListener::mouseEntered, aEvent);
    }
}

void SAL_CALL PresenterSlideShowView::mouseExited (const awt::MouseEvent& rEvent)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(cppu::UnoType<awt::XMouseListener>::get());
    if (pIterator != nullptr)
    {
        pIterator->notifyEach(&awt::XMouseListener::mouseExited, aEvent);
    }
}

//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterSlideShowView::mouseDragged (const awt::MouseEvent& rEvent)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(cppu::UnoType<awt::XMouseMotionListener>::get());
    if (pIterator != nullptr)
    {
        pIterator->notifyEach(&awt::XMouseMotionListener::mouseDragged, aEvent);
    }
}

void SAL_CALL PresenterSlideShowView::mouseMoved (const awt::MouseEvent& rEvent)
{
    awt::MouseEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(cppu::UnoType<awt::XMouseMotionListener>::get());
    if (pIterator != nullptr)
    {
        pIterator->notifyEach(&awt::XMouseMotionListener::mouseMoved, aEvent);
    }
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterSlideShowView::windowResized (const awt::WindowEvent&)
{
    ThrowIfDisposed();
    ::osl::MutexGuard aGuard (::osl::Mutex::getGlobalMutex());

    Resize();
}

void SAL_CALL PresenterSlideShowView::windowMoved (const awt::WindowEvent&)
{
    if ( ! mbIsPaintPending)
        mbIsForcedPaintPending = true;
}

void SAL_CALL PresenterSlideShowView::windowShown (const lang::EventObject&)
{
    Resize();
}

void SAL_CALL PresenterSlideShowView::windowHidden (const lang::EventObject&) {}

//----- XView -----------------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterSlideShowView::getResourceId()
{
    return mxViewId;
}

sal_Bool SAL_CALL PresenterSlideShowView::isAnchorOnly()
{
    return false;
}

//----- CachablePresenterView -------------------------------------------------

void PresenterSlideShowView::ActivatePresenterView()
{
    if (mxSlideShow.is() && ! mbIsViewAdded)
    {
        impl_addAndConfigureView();
        mbIsViewAdded = true;
    }
}

void PresenterSlideShowView::DeactivatePresenterView()
{
    if (mxSlideShow.is() && mbIsViewAdded)
    {
        mxSlideShow->removeView(this);
        mbIsViewAdded = false;
    }
}


void PresenterSlideShowView::PaintOuterWindow (const awt::Rectangle& rRepaintBox)
{
    if ( ! mxCanvas.is())
        return;

    if (mpBackground.get() == nullptr)
        return;

    const rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rRepaintBox, mxCanvas->getDevice()));

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        nullptr,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    Reference<rendering::XBitmap> xBackgroundBitmap (mpBackground->GetNormalBitmap());
    if (xBackgroundBitmap.is())
    {
        Sequence<rendering::Texture> aTextures (1);
        const geometry::IntegerSize2D aBitmapSize(xBackgroundBitmap->getSize());
        aTextures[0] = rendering::Texture (
            geometry::AffineMatrix2D(
                aBitmapSize.Width,0,0,
                0,aBitmapSize.Height,0),
            1,
            0,
            xBackgroundBitmap,
            nullptr,
            nullptr,
            rendering::StrokeAttributes(),
            rendering::TexturingMode::REPEAT,
            rendering::TexturingMode::REPEAT);

        if (mxBackgroundPolygon1.is())
            mxCanvas->fillTexturedPolyPolygon(
                mxBackgroundPolygon1,
                aViewState,
                aRenderState,
                aTextures);
        if (mxBackgroundPolygon2.is())
            mxCanvas->fillTexturedPolyPolygon(
                mxBackgroundPolygon2,
                aViewState,
                aRenderState,
                aTextures);
    }
    else
    {
        PresenterCanvasHelper::SetDeviceColor(aRenderState, mpBackground->maReplacementColor);

        if (mxBackgroundPolygon1.is())
            mxCanvas->fillPolyPolygon(mxBackgroundPolygon1, aViewState, aRenderState);
        if (mxBackgroundPolygon2.is())
            mxCanvas->fillPolyPolygon(mxBackgroundPolygon2, aViewState, aRenderState);
    }
}

void PresenterSlideShowView::PaintEndSlide (const awt::Rectangle& rRepaintBox)
{
    if ( ! mxCanvas.is())
        return;

    const rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rRepaintBox, mxCanvas->getDevice()));

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        nullptr,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);
    PresenterCanvasHelper::SetDeviceColor(aRenderState, util::Color(0x00000000));
    mxCanvas->fillPolyPolygon(
        PresenterGeometryHelper::CreatePolygon(mxViewWindow->getPosSize(), mxCanvas->getDevice()),
        aViewState,
        aRenderState);

    do
    {
        if (mpPresenterController.get() == nullptr)
            break;
        std::shared_ptr<PresenterTheme> pTheme (mpPresenterController->GetTheme());
        if (pTheme == nullptr)
            break;

        const OUString sViewStyle (pTheme->GetStyleName(mxViewId->getResourceURL()));
        PresenterTheme::SharedFontDescriptor pFont (pTheme->GetFont(sViewStyle));
        if (pFont.get() == nullptr)
            break;

        /// this is responsible of the " presentation exit " text inside the slide windows
        PresenterCanvasHelper::SetDeviceColor(aRenderState, util::Color(0x00ffffff));
        aRenderState.AffineTransform.m02 = 20;
        aRenderState.AffineTransform.m12 = 40;
        const rendering::StringContext aContext (
            msClickToExitPresentationText, 0, msClickToExitPresentationText.getLength());
        pFont->PrepareFont(mxCanvas);
        const Reference<rendering::XTextLayout> xLayout (
            pFont->mxFont->createTextLayout(aContext,rendering::TextDirection::WEAK_LEFT_TO_RIGHT,0));
        mxCanvas->drawTextLayout(
            xLayout,
            aViewState,
            aRenderState);
    }
    while (false);

    // Finally, in double buffered environments, request the changes to be
    // made visible.
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(true);
}

void PresenterSlideShowView::PaintInnerWindow (const awt::PaintEvent& rEvent)
{
    // Forward window paint to listeners.
    awt::PaintEvent aEvent (rEvent);
    aEvent.Source = static_cast<XWeak*>(this);
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(cppu::UnoType<awt::XPaintListener>::get());
    if (pIterator != nullptr)
    {
        pIterator->notifyEach(&awt::XPaintListener::windowPaint, aEvent);
    }

    /** The slide show relies on the back buffer of the canvas not being
        modified.  With a shared canvas there are times when that can not be
        guaranteed.
    */
    if (mbIsForcedPaintPending && mxSlideShow.is() && mbIsViewAdded)
    {
        mxSlideShow->removeView(this);
        impl_addAndConfigureView();
    }

    // Finally, in double buffered environments, request the changes to be
    // made visible.
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(true);
}

Reference<awt::XWindow> PresenterSlideShowView::CreateViewWindow (
    const Reference<awt::XWindow>& rxParentWindow) const
{
    Reference<awt::XWindow> xViewWindow;
    try
    {
        Reference<lang::XMultiComponentFactory> xFactory (mxComponentContext->getServiceManager());
        if ( ! xFactory.is())
            return xViewWindow;

        Reference<awt::XToolkit2> xToolkit = awt::Toolkit::create(mxComponentContext);
        awt::WindowDescriptor aWindowDescriptor (
            awt::WindowClass_CONTAINER,
            OUString(),
            Reference<awt::XWindowPeer>(rxParentWindow,UNO_QUERY_THROW),
            -1, // parent index not available
            awt::Rectangle(0,0,10,10),
            awt::WindowAttribute::SIZEABLE
                | awt::WindowAttribute::MOVEABLE
                | awt::WindowAttribute::NODECORATION);
        xViewWindow.set( xToolkit->createWindow(aWindowDescriptor),UNO_QUERY_THROW);

        // Make the background transparent.  The slide show paints its own background.
        Reference<awt::XWindowPeer> xPeer (xViewWindow, UNO_QUERY_THROW);
        xPeer->setBackground(0xff000000);

        xViewWindow->setVisible(true);
    }
    catch (RuntimeException&)
    {
    }
    return xViewWindow;
}

Reference<rendering::XCanvas> PresenterSlideShowView::CreateViewCanvas (
    const Reference<awt::XWindow>& rxViewWindow) const
{
    // Create a canvas for the view window.
    return mxPresenterHelper->createSharedCanvas(
        Reference<rendering::XSpriteCanvas>(mxTopPane->getCanvas(), UNO_QUERY),
        mxTopPane->getWindow(),
        mxTopPane->getCanvas(),
        mxTopPane->getWindow(),
        rxViewWindow);
}

void PresenterSlideShowView::Resize()
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
    CreateBackgroundPolygons();

    // Notify listeners that the transformation that maps the view into the
    // window has changed.
    lang::EventObject aEvent (static_cast<XWeak*>(this));
    ::cppu::OInterfaceContainerHelper* pIterator
        = maBroadcaster.getContainer(cppu::UnoType<util::XModifyListener>::get());
    if (pIterator != nullptr)
    {
        pIterator->notifyEach(&util::XModifyListener::modified, aEvent);
    }

    // Due to constant aspect ratio resizing may lead a preview that changes
    // its position but not its size.  This invalidates the back buffer and
    // we have to enforce a complete repaint.
    if ( ! mbIsPaintPending)
        mbIsForcedPaintPending = true;
}

void PresenterSlideShowView::CreateBackgroundPolygons()
{
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    const awt::Rectangle aViewWindowBox (mxViewWindow->getPosSize());
    if (aWindowBox.Height == aViewWindowBox.Height && aWindowBox.Width == aViewWindowBox.Width)
    {
        mxBackgroundPolygon1 = nullptr;
        mxBackgroundPolygon2 = nullptr;
    }
    else if (aWindowBox.Height == aViewWindowBox.Height)
    {
        // Paint two boxes to the left and right of the view window.
        mxBackgroundPolygon1 = PresenterGeometryHelper::CreatePolygon(
            awt::Rectangle(
                0,
                0,
                aViewWindowBox.X,
                aWindowBox.Height),
            mxCanvas->getDevice());
        mxBackgroundPolygon2 = PresenterGeometryHelper::CreatePolygon(
            awt::Rectangle(
                aViewWindowBox.X + aViewWindowBox.Width,
                0,
                aWindowBox.Width - aViewWindowBox.X - aViewWindowBox.Width,
                aWindowBox.Height),
            mxCanvas->getDevice());
    }
    else
    {
        // Paint two boxes above and below the view window.
        mxBackgroundPolygon1 = PresenterGeometryHelper::CreatePolygon(
            awt::Rectangle(
                0,
                0,
                aWindowBox.Width,
                aViewWindowBox.Y),
            mxCanvas->getDevice());
        mxBackgroundPolygon2 = PresenterGeometryHelper::CreatePolygon(
            awt::Rectangle(
                0,
                aViewWindowBox.Y + aViewWindowBox.Height,
                aWindowBox.Width,
                aWindowBox.Height - aViewWindowBox.Y - aViewWindowBox.Height),
            mxCanvas->getDevice());
    }
}

void PresenterSlideShowView::ThrowIfDisposed()
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            "PresenterSlideShowView object has already been disposed",
            static_cast<uno::XWeak*>(this));
    }
}

void PresenterSlideShowView::impl_addAndConfigureView()
{
    Reference<presentation::XSlideShowView> xView (this);
    mxSlideShow->addView(xView);
    // Prevent embedded sounds being played twice at the same time by
    // disabling sound for the new slide show view.
    beans::PropertyValue aProperty;
    aProperty.Name = "IsSoundEnabled";
    Sequence<Any> aValues (2);
    aValues[0] <<= xView;
    aValues[1] <<= false;
    aProperty.Value <<= aValues;
    mxSlideShow->setProperty(aProperty);
}

} } // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
