/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include "PresenterButton.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterController.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterPaintManager.hxx"
#include "PresenterUIPainter.hxx"
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace sdext { namespace presenter {

const static double gnHorizontalBorder (15);
const static double gnVerticalBorder (5);



::rtl::Reference<PresenterButton> PresenterButton::Create (
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const ::boost::shared_ptr<PresenterTheme>& rpTheme,
    const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
    const css::uno::Reference<css::rendering::XCanvas>& rxParentCanvas,
    const OUString& rsConfigurationName)
{
    Reference<beans::XPropertySet> xProperties (GetConfigurationProperties(
        rxComponentContext,
        rsConfigurationName));
    if (xProperties.is())
    {
        OUString sText;
        OUString sAction;
        PresenterConfigurationAccess::GetProperty(xProperties, A2S("Text")) >>= sText;
        PresenterConfigurationAccess::GetProperty(xProperties, A2S("Action")) >>= sAction;

        PresenterTheme::SharedFontDescriptor pFont;
        if (rpTheme.get() != NULL)
            pFont = rpTheme->GetFont(A2S("ButtonFont"));

        PresenterTheme::SharedFontDescriptor pMouseOverFont;
        if (rpTheme.get() != NULL)
            pMouseOverFont = rpTheme->GetFont(A2S("ButtonMouseOverFont"));

        rtl::Reference<PresenterButton> pButton (
            new PresenterButton(
                rxComponentContext,
                rpPresenterController,
                rpTheme,
                rxParentWindow,
                pFont,
                pMouseOverFont,
                sText,
                sAction));
        pButton->SetCanvas(rxParentCanvas, rxParentWindow);
        return pButton;
    }
    else
        return NULL;
}




PresenterButton::PresenterButton (
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const ::boost::shared_ptr<PresenterTheme>& rpTheme,
    const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
    const PresenterTheme::SharedFontDescriptor& rpFont,
    const PresenterTheme::SharedFontDescriptor& rpMouseOverFont,
    const OUString& rsText,
    const OUString& rsAction)
    : PresenterButtonInterfaceBase(m_aMutex),
      mpPresenterController(rpPresenterController),
      mpTheme(rpTheme),
      mxWindow(),
      mxCanvas(),
      mxPresenterHelper(),
      msText(rsText),
      mpFont(rpFont),
      mpMouseOverFont(rpMouseOverFont),
      msAction(rsAction),
      maCenter(),
      maButtonSize(-1,-1),
      meState(PresenterBitmapDescriptor::Normal),
      mxNormalBitmap(),
      mxMouseOverBitmap()
{
    try
    {
        Reference<lang::XMultiComponentFactory> xFactory (rxComponentContext->getServiceManager());
        if ( ! xFactory.is())
            throw RuntimeException();

        mxPresenterHelper = Reference<drawing::XPresenterHelper>(
            xFactory->createInstanceWithContext(
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Draw.PresenterHelper")),
                rxComponentContext),
            UNO_QUERY_THROW);

        if (mxPresenterHelper.is())
            mxWindow = mxPresenterHelper->createWindow(rxParentWindow,
                sal_False,
                sal_False,
                sal_False,
                sal_False);

        // Make the background transparent.
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




PresenterButton::~PresenterButton (void)
{
}




void SAL_CALL PresenterButton::disposing (void)
{
    if (mxCanvas.is())
    {
        Reference<lang::XComponent> xComponent (mxCanvas, UNO_QUERY);
        mxCanvas = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

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
}




void PresenterButton::SetCenter (const css::geometry::RealPoint2D& rLocation)
{
    if (mxCanvas.is())
    {
        Invalidate();

        maCenter = rLocation;
        mxWindow->setPosSize(
            sal_Int32(0.5 + maCenter.X - maButtonSize.Width/2),
            sal_Int32(0.5 + maCenter.Y - maButtonSize.Height/2),
            maButtonSize.Width,
            maButtonSize.Height,
            awt::PosSize::POSSIZE);

        Invalidate();
    }
    else
    {
        // The button can not be painted but we can at least store the new center.
        maCenter = rLocation;
    }
}




void PresenterButton::SetCanvas (
    const css::uno::Reference<css::rendering::XCanvas>& rxParentCanvas,
    const css::uno::Reference<css::awt::XWindow>& rxParentWindow)
{
    if (mxCanvas.is())
    {
        Reference<lang::XComponent> xComponent (mxCanvas, UNO_QUERY);
        mxCanvas = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    if (mxPresenterHelper.is() && rxParentCanvas.is() && rxParentWindow.is())
    {
        mxCanvas = mxPresenterHelper->createSharedCanvas (
            Reference<rendering::XSpriteCanvas>(rxParentCanvas, UNO_QUERY),
            rxParentWindow,
            rxParentCanvas,
            rxParentWindow,
            mxWindow);
        if (mxCanvas.is())
        {
            SetupButtonBitmaps();
            SetCenter(maCenter);
        }
    }
}




css::geometry::IntegerSize2D PresenterButton::GetSize (void)
{
    if (maButtonSize.Width < 0)
        CalculateButtonSize();
    return maButtonSize;
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterButton::windowResized (const css::awt::WindowEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}





void SAL_CALL PresenterButton::windowMoved (const css::awt::WindowEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}




void SAL_CALL PresenterButton::windowShown (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}




void SAL_CALL PresenterButton::windowHidden (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterButton::windowPaint (const css::awt::PaintEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    if (mxWindow.is() && mxCanvas.is())
    {
        Reference<rendering::XBitmap> xBitmap;
        if (meState == PresenterBitmapDescriptor::MouseOver)
            xBitmap = mxMouseOverBitmap;
        else
            xBitmap = mxNormalBitmap;
        if ( ! xBitmap.is())
            return;

        rendering::ViewState aViewState(
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL);
        rendering::RenderState aRenderState(
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            PresenterGeometryHelper::CreatePolygon(rEvent.UpdateRect, mxCanvas->getDevice()),
            Sequence<double>(4),
            rendering::CompositeOperation::SOURCE);

        mxCanvas->drawBitmap(xBitmap, aViewState, aRenderState);

        Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
        if (xSpriteCanvas.is())
            xSpriteCanvas->updateScreen(sal_False);
    }
}




//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterButton::mousePressed (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();

    meState = PresenterBitmapDescriptor::ButtonDown;
}




void SAL_CALL PresenterButton::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();

    if (meState == PresenterBitmapDescriptor::ButtonDown)
    {
        OSL_ASSERT(mpPresenterController.get()!=NULL);
        mpPresenterController->DispatchUnoCommand(msAction);

        meState = PresenterBitmapDescriptor::Normal;
        Invalidate();
    }
}




void SAL_CALL PresenterButton::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    meState = PresenterBitmapDescriptor::MouseOver;
    Invalidate();
}




void SAL_CALL PresenterButton::mouseExited (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    meState = PresenterBitmapDescriptor::Normal;
    Invalidate();
}





//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterButton::mouseMoved (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}




void SAL_CALL PresenterButton::mouseDragged (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}




//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterButton::disposing (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    if (rEvent.Source == mxWindow)
        mxWindow = NULL;
}




//-----------------------------------------------------------------------------

css::geometry::IntegerSize2D PresenterButton::CalculateButtonSize (void)
{
    if (mpFont.get()!=NULL && !mpFont->mxFont.is() && mxCanvas.is())
        mpFont->PrepareFont(mxCanvas);
    if (mpFont.get()==NULL || !mpFont->mxFont.is())
        return geometry::IntegerSize2D(-1,-1);

    geometry::RealSize2D aTextSize (PresenterCanvasHelper::GetTextSize(mpFont->mxFont,msText));

    return geometry::IntegerSize2D (
        sal_Int32(0.5 + aTextSize.Width + 2*gnHorizontalBorder),
        sal_Int32(0.5 + aTextSize.Height + 2*gnVerticalBorder));
}




void PresenterButton::RenderButton (
    const Reference<rendering::XCanvas>& rxCanvas,
    const geometry::IntegerSize2D& rSize,
    const PresenterTheme::SharedFontDescriptor& rpFont,
    const PresenterBitmapDescriptor::Mode eMode,
    const SharedBitmapDescriptor& rpLeft,
    const SharedBitmapDescriptor& rpCenter,
    const SharedBitmapDescriptor& rpRight)
{
    if ( ! rxCanvas.is())
        return;

    const awt::Rectangle aBox(0,0, rSize.Width, rSize.Height);

    PresenterUIPainter::PaintHorizontalBitmapComposite (
        rxCanvas,
        aBox,
        aBox,
        GetBitmap(rpLeft, eMode),
        GetBitmap(rpCenter, eMode),
        GetBitmap(rpRight, eMode));

    if (rpFont.get()==NULL || ! rpFont->mxFont.is())
        return;

    const rendering::StringContext aContext (msText, 0, msText.getLength());
    const Reference<rendering::XTextLayout> xLayout (
        rpFont->mxFont->createTextLayout(aContext,rendering::TextDirection::WEAK_LEFT_TO_RIGHT,0));
    const geometry::RealRectangle2D aTextBBox (xLayout->queryTextBounds());

    rendering::RenderState aRenderState (geometry::AffineMatrix2D(1,0,0, 0,1,0), NULL,
        Sequence<double>(4), rendering::CompositeOperation::SOURCE);
    PresenterCanvasHelper::SetDeviceColor(aRenderState, rpFont->mnColor);
    aRenderState.AffineTransform.m02 = (rSize.Width - aTextBBox.X2 + aTextBBox.X1)/2;
    aRenderState.AffineTransform.m12 = (rSize.Height - aTextBBox.Y2 + aTextBBox.Y1)/2 - aTextBBox.Y1;

    rxCanvas->drawText(
        aContext,
        rpFont->mxFont,
        rendering::ViewState(geometry::AffineMatrix2D(1,0,0, 0,1,0), NULL),
        aRenderState,
        rendering::TextDirection::WEAK_LEFT_TO_RIGHT);
}




void PresenterButton::Invalidate (void)
{
    mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
}




Reference<rendering::XBitmap> PresenterButton::GetBitmap (
    const SharedBitmapDescriptor& mpIcon,
    const PresenterBitmapDescriptor::Mode eMode)
{
    if (mpIcon.get() != NULL)
        return mpIcon->GetBitmap(eMode);
    else
    {
        OSL_ASSERT(mpIcon.get()!=NULL);
        return NULL;
    }
}




void PresenterButton::SetupButtonBitmaps (void)
{
    if ( ! mxCanvas.is())
        return;
    if ( ! mxCanvas->getDevice().is())
        return;

    // Get the bitmaps for the button border.
    SharedBitmapDescriptor pLeftBitmap (mpTheme->GetBitmap(A2S("ButtonFrameLeft")));
    SharedBitmapDescriptor pCenterBitmap(mpTheme->GetBitmap(A2S("ButtonFrameCenter")));
    SharedBitmapDescriptor pRightBitmap(mpTheme->GetBitmap(A2S("ButtonFrameRight")));

    maButtonSize = CalculateButtonSize();

    if (maButtonSize.Height<=0 && maButtonSize.Width<= 0)
        return;

    mxNormalBitmap = mxCanvas->getDevice()->createCompatibleAlphaBitmap(maButtonSize);
    Reference<rendering::XCanvas> xCanvas (mxNormalBitmap, UNO_QUERY);
    if (xCanvas.is())
        RenderButton(
            xCanvas,
            maButtonSize,
            mpFont,
            PresenterBitmapDescriptor::Normal,
            pLeftBitmap,
            pCenterBitmap,
            pRightBitmap);

    mxMouseOverBitmap = mxCanvas->getDevice()->createCompatibleAlphaBitmap(maButtonSize);
    xCanvas = Reference<rendering::XCanvas>(mxMouseOverBitmap, UNO_QUERY);
    if (mpMouseOverFont.get()!=NULL && !mpMouseOverFont->mxFont.is() && mxCanvas.is())
        mpMouseOverFont->PrepareFont(mxCanvas);
    if (xCanvas.is())
        RenderButton(
            xCanvas,
            maButtonSize,
            mpMouseOverFont,
            PresenterBitmapDescriptor::MouseOver,
            pLeftBitmap,
            pCenterBitmap,
            pRightBitmap);
}




Reference<beans::XPropertySet> PresenterButton::GetConfigurationProperties (
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const OUString& rsConfgurationName)
{
    PresenterConfigurationAccess aConfiguration (
        rxComponentContext,
        PresenterConfigurationAccess::msPresenterScreenRootName,
        PresenterConfigurationAccess::READ_ONLY);
    return Reference<beans::XPropertySet>(
        PresenterConfigurationAccess::Find (
            Reference<container::XNameAccess>(
                aConfiguration.GetConfigurationNode(A2S("PresenterScreenSettings/Buttons")),
                UNO_QUERY),
            ::boost::bind(&PresenterConfigurationAccess::IsStringPropertyEqual,
                rsConfgurationName,
                A2S("Name"),
                _2)),
        UNO_QUERY);
}




void PresenterButton::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterButton object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}


} } // end of namespace sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
