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

#include <vcl/svapp.hxx>
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

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sdext { namespace presenter {

const static double gnHorizontalBorder (15);
const static double gnVerticalBorder (5);

::rtl::Reference<PresenterButton> PresenterButton::Create (
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const std::shared_ptr<PresenterTheme>& rpTheme,
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
        PresenterConfigurationAccess::GetProperty(xProperties, "Text") >>= sText;
        PresenterConfigurationAccess::GetProperty(xProperties, "Action") >>= sAction;

        PresenterTheme::SharedFontDescriptor pFont;
        if (rpTheme != nullptr)
            pFont = rpTheme->GetFont("ButtonFont");

        PresenterTheme::SharedFontDescriptor pMouseOverFont;
        if (rpTheme != nullptr)
            pMouseOverFont = rpTheme->GetFont("ButtonMouseOverFont");

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
        return nullptr;
}

PresenterButton::PresenterButton (
    const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
    const ::rtl::Reference<PresenterController>& rpPresenterController,
    const std::shared_ptr<PresenterTheme>& rpTheme,
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

        // Make the background transparent.
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

PresenterButton::~PresenterButton()
{
}

void SAL_CALL PresenterButton::disposing()
{
    if (mxCanvas.is())
    {
        Reference<lang::XComponent> xComponent (mxCanvas, UNO_QUERY);
        mxCanvas = nullptr;
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
        mxWindow = nullptr;
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
        mxCanvas = nullptr;
        if (xComponent.is())
            xComponent->dispose();
    }

    if (!(mxPresenterHelper.is() && rxParentCanvas.is() && rxParentWindow.is()))
        return;

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

css::geometry::IntegerSize2D const & PresenterButton::GetSize()
{
    if (maButtonSize.Width < 0)
        CalculateButtonSize();
    return maButtonSize;
}

//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterButton::windowResized (const css::awt::WindowEvent&)
{
    ThrowIfDisposed();
}

void SAL_CALL PresenterButton::windowMoved (const css::awt::WindowEvent&)
{
    ThrowIfDisposed();
}

void SAL_CALL PresenterButton::windowShown (const css::lang::EventObject&)
{
    ThrowIfDisposed();
}

void SAL_CALL PresenterButton::windowHidden (const css::lang::EventObject&)
{
    ThrowIfDisposed();
}

//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterButton::windowPaint (const css::awt::PaintEvent& rEvent)
{
    ThrowIfDisposed();
    if (!(mxWindow.is() && mxCanvas.is()))
        return;

    Reference<rendering::XBitmap> xBitmap;
    if (meState == PresenterBitmapDescriptor::MouseOver)
        xBitmap = mxMouseOverBitmap;
    else
        xBitmap = mxNormalBitmap;
    if ( ! xBitmap.is())
        return;

    rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        nullptr);
    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rEvent.UpdateRect, mxCanvas->getDevice()),
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    mxCanvas->drawBitmap(xBitmap, aViewState, aRenderState);

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(false);
}

//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterButton::mousePressed (const css::awt::MouseEvent&)
{
    ThrowIfDisposed();
    meState = PresenterBitmapDescriptor::ButtonDown;
}

void SAL_CALL PresenterButton::mouseReleased (const css::awt::MouseEvent&)
{
    ThrowIfDisposed();

    if (meState == PresenterBitmapDescriptor::ButtonDown)
    {
        OSL_ASSERT(mpPresenterController.get()!=nullptr);
        mpPresenterController->DispatchUnoCommand(msAction);

        meState = PresenterBitmapDescriptor::Normal;
        Invalidate();
    }
}

void SAL_CALL PresenterButton::mouseEntered (const css::awt::MouseEvent&)
{
    ThrowIfDisposed();
    meState = PresenterBitmapDescriptor::MouseOver;
    Invalidate();
}

void SAL_CALL PresenterButton::mouseExited (const css::awt::MouseEvent&)
{
    ThrowIfDisposed();
    meState = PresenterBitmapDescriptor::Normal;
    Invalidate();
}

//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterButton::mouseMoved (const css::awt::MouseEvent&)
{
    ThrowIfDisposed();
}

void SAL_CALL PresenterButton::mouseDragged (const css::awt::MouseEvent&)
{
    ThrowIfDisposed();
}

//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterButton::disposing (const css::lang::EventObject& rEvent)
{
    if (rEvent.Source == mxWindow)
        mxWindow = nullptr;
}


css::geometry::IntegerSize2D PresenterButton::CalculateButtonSize()
{
    if (mpFont.get()!=nullptr && !mpFont->mxFont.is() && mxCanvas.is())
        mpFont->PrepareFont(mxCanvas);
    if (mpFont.get()==nullptr || !mpFont->mxFont.is())
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

    if (rpFont.get()==nullptr || ! rpFont->mxFont.is())
        return;

    const rendering::StringContext aContext (msText, 0, msText.getLength());
    const Reference<rendering::XTextLayout> xLayout (
        rpFont->mxFont->createTextLayout(aContext,rendering::TextDirection::WEAK_LEFT_TO_RIGHT,0));
    const geometry::RealRectangle2D aTextBBox (xLayout->queryTextBounds());

    rendering::RenderState aRenderState (geometry::AffineMatrix2D(1,0,0, 0,1,0), nullptr,
        Sequence<double>(4), rendering::CompositeOperation::SOURCE);
    PresenterCanvasHelper::SetDeviceColor(aRenderState, rpFont->mnColor);

    aRenderState.AffineTransform.m02 = (rSize.Width - aTextBBox.X2 + aTextBBox.X1)/2;
    aRenderState.AffineTransform.m12 = (rSize.Height - aTextBBox.Y2 + aTextBBox.Y1)/2 - aTextBBox.Y1;

    /// this is responsible of the close button
    rxCanvas->drawTextLayout(
        xLayout,
        rendering::ViewState(geometry::AffineMatrix2D(1,0,0, 0,1,0), nullptr),
        aRenderState);
}

void PresenterButton::Invalidate()
{
    mpPresenterController->GetPaintManager()->Invalidate(mxWindow);
}

Reference<rendering::XBitmap> PresenterButton::GetBitmap (
    const SharedBitmapDescriptor& mpIcon,
    const PresenterBitmapDescriptor::Mode eMode)
{
    if (mpIcon.get() != nullptr)
        return mpIcon->GetBitmap(eMode);
    else
    {
        OSL_ASSERT(mpIcon.get()!=nullptr);
        return nullptr;
    }
}

void PresenterButton::SetupButtonBitmaps()
{
    if ( ! mxCanvas.is())
        return;
    if ( ! mxCanvas->getDevice().is())
        return;

    // Get the bitmaps for the button border.
    SharedBitmapDescriptor pLeftBitmap (mpTheme->GetBitmap("ButtonFrameLeft"));
    SharedBitmapDescriptor pCenterBitmap(mpTheme->GetBitmap("ButtonFrameCenter"));
    SharedBitmapDescriptor pRightBitmap(mpTheme->GetBitmap("ButtonFrameRight"));

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
    xCanvas.set(mxMouseOverBitmap, UNO_QUERY);
    if (mpMouseOverFont.get()!=nullptr && !mpMouseOverFont->mxFont.is() && mxCanvas.is())
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
                aConfiguration.GetConfigurationNode("PresenterScreenSettings/Buttons"),
                UNO_QUERY),
            [&rsConfgurationName](OUString const&, uno::Reference<beans::XPropertySet> const& xProps) -> bool
            {
                return PresenterConfigurationAccess::IsStringPropertyEqual(
                        rsConfgurationName, "Name", xProps);
            }),
        UNO_QUERY);
}

void PresenterButton::ThrowIfDisposed() const
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            "PresenterButton object has already been disposed",
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} } // end of namespace sdext::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
