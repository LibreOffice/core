/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterNotesView.cxx,v $
 *
 * $Revision: 1.5 $
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

#include "PresenterNotesView.hxx"
#include "PresenterButton.hxx"
#include "PresenterCanvasHelper.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterPaintManager.hxx"
#include "PresenterScrollBar.hxx"
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

static const sal_Int32 gnSpaceBelowSeparator (10);
static const sal_Int32 gnSpaceAboveSeparator (10);

namespace sdext { namespace presenter {

PresenterNotesView::PresenterNotesView (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterNotesViewInterfaceBase(m_aMutex),
      mxViewId(rxViewId),
      mpPresenterController(rpPresenterController),
      mxTextView(),
      mxCanvas(),
      mxBitmap(),
      mxCurrentNotesPage(),
      mpFont(),
      maFontDescriptor(),
      mpScrollBar(),
      mxToolBarWindow(),
      mxToolBarCanvas(),
      mpToolBar(),
      mpCloseButton(),
      mnSeparatorYLocation(0),
      maTextBoundingBox(),
      mpBackground()
{
    const OUString sResourceURL (mxViewId->getResourceURL());
    mpFont = rpPresenterController->GetViewFont(sResourceURL);

    try
    {
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        Reference<XConfigurationController> xCC (xCM->getConfigurationController(), UNO_QUERY_THROW);
        Reference<XPane> xPane (xCC->getResource(rxViewId->getAnchor()), UNO_QUERY_THROW);

        mxParentWindow = xPane->getWindow();
        mxCanvas = xPane->getCanvas();

        Reference<lang::XMultiComponentFactory> xFactory (
            rxComponentContext->getServiceManager(), UNO_QUERY_THROW);
        Sequence<Any> aArguments(1);
        aArguments[0] <<= mxCanvas;
        mxTextView = Reference<beans::XPropertySet>(
            xFactory->createInstanceWithArgumentsAndContext(
                A2S("com.sun.star.drawing.PresenterTextView"),
                aArguments,
                rxComponentContext),
            UNO_QUERY_THROW);
        mxTextView->setPropertyValue(A2S("BackgroundColor"), Any(sal_uInt32(0x00ff0000)));
        if (mpFont.get() != NULL)
        {
            maFontDescriptor.Name = mpFont->msFamilyName;
            maFontDescriptor.Height = sal::static_int_cast<sal_Int16>(mpFont->mnSize);
            maFontDescriptor.StyleName = mpFont->msStyleName;
            mxTextView->setPropertyValue(A2S("FontDescriptor"), Any(maFontDescriptor));
            mxTextView->setPropertyValue(A2S("TextColor"), Any(mpFont->mnColor));
        }

        CreateToolBar(rxComponentContext, rpPresenterController);

        mpCloseButton = PresenterButton::Create(
            rxComponentContext,
            mpPresenterController,
            mpPresenterController->GetTheme(),
            mxParentWindow,
            mxCanvas,
            A2S("NotesViewCloser"));

        if (mxParentWindow.is())
        {
            mxParentWindow->addWindowListener(this);
            mxParentWindow->addPaintListener(this);
            mxParentWindow->addKeyListener(this);
            mxParentWindow->setVisible(sal_True);
        }

        mpScrollBar = new PresenterVerticalScrollBar(
            rxComponentContext,
            mxParentWindow,
            mpPresenterController->GetPaintManager(),
            ::boost::bind(&PresenterNotesView::SetTop, this, _1));
        mpScrollBar->SetCanvas(mxCanvas);

        Layout();
    }
    catch (RuntimeException&)
    {
        PresenterNotesView::disposing();
        throw;
    }
}




PresenterNotesView::~PresenterNotesView (void)
{
}




void SAL_CALL PresenterNotesView::disposing (void)
{
    if (mxParentWindow.is())
    {
        mxParentWindow->removeWindowListener(this);
        mxParentWindow->removePaintListener(this);
        mxParentWindow->removeKeyListener(this);
        mxParentWindow = NULL;
    }

    {
        Reference<XComponent> xComponent (mxTextView, UNO_QUERY);
        mxTextView = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    // Dispose tool bar.
    {
        Reference<XComponent> xComponent (static_cast<XWeak*>(mpToolBar.get()), UNO_QUERY);
        mpToolBar = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }
    {
        Reference<XComponent> xComponent (mxToolBarCanvas, UNO_QUERY);
        mxToolBarCanvas = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }
    {
        Reference<XComponent> xComponent (mxToolBarWindow, UNO_QUERY);
        mxToolBarWindow = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    // Dispose close button
    {
        Reference<XComponent> xComponent (static_cast<XWeak*>(mpCloseButton.get()), UNO_QUERY);
        mpCloseButton = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    // Create the tool bar.

    mpScrollBar = NULL;

    mxViewId = NULL;
}




void PresenterNotesView::CreateToolBar (
    const css::uno::Reference<css::uno::XComponentContext>& rxContext,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
{
    if (rpPresenterController.get() == NULL)
        return;

    Reference<drawing::XPresenterHelper> xPresenterHelper (
        rpPresenterController->GetPresenterHelper());
    if ( ! xPresenterHelper.is())
        return;

    // Create a new window as container of the tool bar.
    mxToolBarWindow = xPresenterHelper->createWindow(
        mxParentWindow,
        sal_False,
        sal_True,
        sal_False,
        sal_False);
    mxToolBarCanvas = xPresenterHelper->createSharedCanvas (
        Reference<rendering::XSpriteCanvas>(mxCanvas, UNO_QUERY),
        mxParentWindow,
        mxCanvas,
        mxParentWindow,
        mxToolBarWindow);

    // Create the tool bar.
    mpToolBar = new PresenterToolBar(
        rxContext,
        mxToolBarWindow,
        mxToolBarCanvas,
        rpPresenterController,
        PresenterToolBar::Left);
    mpToolBar->Initialize(
        A2S("PresenterScreenSettings/ToolBars/NotesToolBar"));
}




void PresenterNotesView::SetSlide (const Reference<drawing::XDrawPage>& rxNotesPage)
{
    static const ::rtl::OUString sNotesShapeName (
        A2S("com.sun.star.presentation.NotesShape"));
    static const ::rtl::OUString sTextShapeName (
        A2S("com.sun.star.drawing.TextShape"));

    Reference<container::XIndexAccess> xIndexAccess (rxNotesPage, UNO_QUERY);
    if (xIndexAccess.is()
        && mxTextView.is())
    {
        ::rtl::OUString sText;

        // Iterate over all shapes and find the one that holds the text.
        sal_Int32 nCount (xIndexAccess->getCount());
        for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
        {

            Reference<lang::XServiceName> xServiceName (
                xIndexAccess->getByIndex(nIndex), UNO_QUERY);
            if (xServiceName.is()
                && xServiceName->getServiceName().equals(sNotesShapeName))
            {
                Reference<text::XTextRange> xText (xServiceName, UNO_QUERY);
                if (xText.is())
                {
                    sText += xText->getString();
                }
            }
            else
            {
                Reference<drawing::XShapeDescriptor> xShapeDescriptor (
                    xIndexAccess->getByIndex(nIndex), UNO_QUERY);
                if (xShapeDescriptor.is())
                {
                    ::rtl::OUString sType (xShapeDescriptor->getShapeType());
                    if (sType.equals(sNotesShapeName) || sType.equals(sTextShapeName))
                    {
                        Reference<text::XTextRange> xText (
                            xIndexAccess->getByIndex(nIndex), UNO_QUERY);
                        if (xText.is())
                        {
                            sText += xText->getString();
                        }
                    }
                }
            }
        }

        mxBitmap = NULL;
        mxTextView->setPropertyValue(A2S("Text"), Any(sText));

        Layout();

        if (mpScrollBar.get() != NULL)
        {
            mpScrollBar->SetThumbPosition(0, false);
            UpdateScrollBar();
        }

        Invalidate();
    }
}




Reference<awt::XWindow> PresenterNotesView::GetWindow (void) const
{
    return mxParentWindow;
}




//-----  lang::XEventListener -------------------------------------------------

void SAL_CALL PresenterNotesView::disposing (const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (rEventObject.Source == mxParentWindow)
        mxParentWindow = NULL;
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterNotesView::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    Layout();
}




void SAL_CALL PresenterNotesView::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterNotesView::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterNotesView::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterNotesView::windowPaint (const awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    if ( ! mbIsPresenterViewActive)
        return;

    ::osl::MutexGuard aSolarGuard (::osl::Mutex::getGlobalMutex());
    Paint(rEvent.UpdateRect);
}




//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterNotesView::getResourceId (void)
    throw (RuntimeException)
{
    return mxViewId;
}




sal_Bool SAL_CALL PresenterNotesView::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}




//----- XDrawView -------------------------------------------------------------

void SAL_CALL PresenterNotesView::setCurrentPage (const Reference<drawing::XDrawPage>& rxSlide)
    throw (RuntimeException)
{
    // Get the associated notes page.
    mxCurrentNotesPage = NULL;
    try
    {
        Reference<presentation::XPresentationPage> xPresentationPage(rxSlide, UNO_QUERY);
        if (xPresentationPage.is())
            mxCurrentNotesPage = xPresentationPage->getNotesPage();
    }
    catch (RuntimeException&)
    {
    }

    SetSlide(mxCurrentNotesPage);
}




Reference<drawing::XDrawPage> SAL_CALL PresenterNotesView::getCurrentPage (void)
    throw (RuntimeException)
{
    return NULL;
}




//----- XKeyListener ----------------------------------------------------------

void SAL_CALL PresenterNotesView::keyPressed (const awt::KeyEvent& rEvent)
    throw (RuntimeException)
{
    switch (rEvent.KeyCode)
    {
        case awt::Key::A:
            Scroll(A2S("-1l"));
            break;

        case awt::Key::Y:
        case awt::Key::Z:
            Scroll(A2S("+1l"));
            break;

        case awt::Key::S:
            ChangeFontSize(-1);
            break;

        case awt::Key::G:
            ChangeFontSize(+1);
            break;
    }
}




void SAL_CALL PresenterNotesView::keyReleased (const awt::KeyEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




//-----------------------------------------------------------------------------

void PresenterNotesView::Layout (void)
{
    if ( ! mxParentWindow.is())
        return;
    if ( ! mxTextView.is())
        return;

    awt::Rectangle aWindowBox (mxParentWindow->getPosSize());
    geometry::RealRectangle2D aNewTextBoundingBox (0,0,aWindowBox.Width, aWindowBox.Height);

    // Size the tool bar and the horizontal separator above it.
    if (mxToolBarWindow.is())
    {
        const geometry::RealSize2D aToolBarSize (mpToolBar->GetMinimalSize());
        const sal_Int32 nToolBarHeight = sal_Int32(aToolBarSize.Height + 0.5);
        mxToolBarWindow->setPosSize(0, aWindowBox.Height - nToolBarHeight,
            sal_Int32(aToolBarSize.Width + 0.5), nToolBarHeight,
            awt::PosSize::POSSIZE);
        aNewTextBoundingBox.Y2 -= nToolBarHeight;

        mnSeparatorYLocation = aWindowBox.Height - nToolBarHeight - gnSpaceBelowSeparator;
        aNewTextBoundingBox.Y2 = mnSeparatorYLocation - gnSpaceAboveSeparator;

        // Place the close button.
        if (mpCloseButton.get() != NULL)
            mpCloseButton->SetCenter(geometry::RealPoint2D(
                (aWindowBox.Width +  aToolBarSize.Width) / 2,
                aWindowBox.Height - aToolBarSize.Height/2));
    }

    // Check whether the vertical scroll bar is necessary.
    if (mpScrollBar.get() != NULL)
    {
        bool bShowVerticalScrollbar (false);
        try
        {
            const double nTextBoxHeight (aNewTextBoundingBox.Y2 - aNewTextBoundingBox.Y1);
            mxTextView->setPropertyValue(
                A2S("Size"),
                Any(awt::Size(
                    sal_Int32(aNewTextBoundingBox.X2 - aNewTextBoundingBox.X1),
                    sal_Int32(nTextBoxHeight))));
            double nHeight (0);
            if (mxTextView->getPropertyValue(A2S("TotalHeight")) >>= nHeight)
            {
                if (nHeight > nTextBoxHeight)
                {
                    bShowVerticalScrollbar = true;
                    aNewTextBoundingBox.X2 -= mpScrollBar->GetSize();
                }
                mpScrollBar->SetTotalSize(nHeight);
            }
        }
        catch(beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }

        mpScrollBar->SetVisible(bShowVerticalScrollbar);
        mxTextView->setPropertyValue(
            A2S("Size"),
            Any(awt::Size(
                sal_Int32(aNewTextBoundingBox.X2 - aNewTextBoundingBox.X1),
                sal_Int32(aNewTextBoundingBox.Y2 - aNewTextBoundingBox.Y1))));
        mpScrollBar->SetPosSize(
            geometry::RealRectangle2D(
                aNewTextBoundingBox.X2,
                aNewTextBoundingBox.X1,
                aNewTextBoundingBox.X2 + mpScrollBar->GetSize(),
                aNewTextBoundingBox.Y2));
        if ( ! bShowVerticalScrollbar)
            mpScrollBar->SetThumbPosition(0, false);

        UpdateScrollBar();
    }

    // Has the text area has changed it position or size?
    if (aNewTextBoundingBox.X1 != maTextBoundingBox.X1
        || aNewTextBoundingBox.Y1 != maTextBoundingBox.Y1
        || aNewTextBoundingBox.X2 != maTextBoundingBox.X2
        || aNewTextBoundingBox.Y2 != maTextBoundingBox.Y2)
    {
        maTextBoundingBox = aNewTextBoundingBox;

        // When the size has changed then we need a new text bitmap.
        if (aNewTextBoundingBox.X2-aNewTextBoundingBox.X1
            != maTextBoundingBox.X2-maTextBoundingBox.X1
            || aNewTextBoundingBox.Y2-aNewTextBoundingBox.Y1
            != maTextBoundingBox.Y2-maTextBoundingBox.Y1)
        {
            mxBitmap = NULL;
        }
    }
}




void PresenterNotesView::Paint (const awt::Rectangle& rUpdateBox)
{
    if ( ! mxParentWindow.is())
        return;
    if ( ! mxCanvas.is())
        return;

    awt::Rectangle aWindowBox (mxParentWindow->getPosSize());

    const rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);
    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(3),
        rendering::CompositeOperation::SOURCE);

    if (mpBackground.get() == NULL)
        mpBackground = mpPresenterController->GetViewBackground(mxViewId->getResourceURL());
    // Paint the background.
    mpPresenterController->GetCanvasHelper()->Paint(
        mpBackground,
        mxCanvas,
        rUpdateBox,
        awt::Rectangle(0,0,aWindowBox.Width,aWindowBox.Height),
        awt::Rectangle());

    // Paint the horizontal separator.
    OSL_ASSERT(mxViewId.is());
    if (mpFont.get() != NULL)
        PresenterCanvasHelper::SetDeviceColor(
            aRenderState,
            mpFont->mnColor);
    mxCanvas->drawLine(
        geometry::RealPoint2D(0,mnSeparatorYLocation),
        geometry::RealPoint2D(aWindowBox.Width,mnSeparatorYLocation),
        aViewState,
        aRenderState);

    if ( ! PresenterGeometryHelper::AreRectanglesDisjoint(
        rUpdateBox,
        PresenterGeometryHelper::ConvertRectangle(maTextBoundingBox)))
    {
        // Create a new text bitmap if necessary.
        if ( ! mxBitmap.is())
        {
            mxBitmap = Reference<rendering::XBitmap>(
                mxTextView->getPropertyValue(A2S("Bitmap")),
                UNO_QUERY);
        }

        aRenderState.AffineTransform.m02 = maTextBoundingBox.X1;
        aRenderState.AffineTransform.m12 = maTextBoundingBox.Y1;

        // Paint the text bitmap.
        if (mxBitmap.is())
        {
            mxCanvas->drawBitmap(
                mxBitmap,
                aViewState,
                aRenderState);

            Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
            if (xSpriteCanvas.is())
                xSpriteCanvas->updateScreen(sal_False);
        }
    }
}




void PresenterNotesView::Invalidate (void)
{
    mxBitmap = NULL;
    mpPresenterController->GetPaintManager()->Invalidate(mxParentWindow);
}




void PresenterNotesView::Scroll (const OUString& rsDistance)
{
    try
    {
        mxTextView->setPropertyValue(A2S("RelativeTop"), Any(rsDistance));

        UpdateScrollBar();
        Invalidate();
    }
    catch (beans::UnknownPropertyException&)
    {}
}




void PresenterNotesView::SetTop (const double nTop)
{
    try
    {
        mxTextView->setPropertyValue(A2S("Top"), Any(sal_Int32(nTop)));

        UpdateScrollBar();
        Invalidate();
    }
    catch (beans::UnknownPropertyException&)
    {}
}




void PresenterNotesView::ChangeFontSize (const double nSizeChange)
{
    const double nNewSize (maFontDescriptor.Height + nSizeChange);
    if (nNewSize > 5)
    {
        maFontDescriptor.Height = sal::static_int_cast<sal_Int16>(0.5 + nNewSize);
        mxTextView->setPropertyValue(A2S("FontDescriptor"), Any(maFontDescriptor));

        Layout();
        UpdateScrollBar();
        Invalidate();

        // Make the new font height persistent.
        if (mpFont.get() != NULL)
            mpFont->mnSize = maFontDescriptor.Height;

        // Write the new font size to the configuration to make it persistent.
        try
        {
            const OUString sStyleName (mpPresenterController->GetTheme()->GetStyleName(
                mxViewId->getResourceURL()));
            ::boost::shared_ptr<PresenterConfigurationAccess> pConfiguration (
                mpPresenterController->GetTheme()->GetNodeForViewStyle(
                    sStyleName,
                    PresenterConfigurationAccess::READ_WRITE));
            if (pConfiguration.get()==NULL || ! pConfiguration->IsValid())
                return;

            pConfiguration->GoToChild(A2S("Font"));
            pConfiguration->SetProperty(A2S("Size"), Any((sal_Int32)(nNewSize+0.5)));
            pConfiguration->CommitChanges();
        }
        catch (Exception&)
        {
            OSL_ASSERT(false);
        }
    }
}




void PresenterNotesView::UpdateScrollBar (void)
{
    if (mpScrollBar.get() != NULL)
    {
        try
        {
            double nHeight = 0;
            if (mxTextView->getPropertyValue(A2S("TotalHeight")) >>= nHeight)
                mpScrollBar->SetTotalSize(nHeight);
        }
        catch(beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }

        try
        {
            double nTop = 0;
            if (mxTextView->getPropertyValue(A2S("Top")) >>= nTop)
                mpScrollBar->SetThumbPosition(nTop, false);
        }
        catch(beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }

        mpScrollBar->SetThumbSize(maTextBoundingBox.Y2 - maTextBoundingBox.Y1);
        mpScrollBar->CheckValues();
    }
}




void PresenterNotesView::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            A2S("PresenterNotesView object has already been disposed"),
            static_cast<uno::XWeak*>(this));
    }
}


} } // end of namespace ::sdext::presenter
