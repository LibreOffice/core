/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterNotesView.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:59:39 $
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

#include "PresenterNotesView.hxx"
#include "PresenterScrollBar.hxx"
#include <com/sun/star/awt/InvalidateStyle.hpp>
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
#include <boost/bind.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

PresenterNotesView::PresenterNotesView (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterNotesViewInterfaceBase(m_aMutex),
      mxViewId(rxViewId),
      mxTextView(),
      mxCanvas(),
      mxBitmap(),
      mxCurrentNotesPage(),
      maFontDescriptor(),
      mpScrollBar()
{
    const OUString sResourceURL (mxViewId->getResourceURL());
    maFontDescriptor = rpPresenterController->GetViewFontDescriptor(sResourceURL);

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
                OUString::createFromAscii("com.sun.star.drawing.PresenterTextView"),
                aArguments,
                rxComponentContext),
            UNO_QUERY_THROW);
        mxTextView->setPropertyValue(
            OUString::createFromAscii("BackgroundColor"),
            Any(rpPresenterController->GetViewBackgroundColor(sResourceURL)));
        mxTextView->setPropertyValue(
            OUString::createFromAscii("FontDescriptor"),
            Any(maFontDescriptor));
        mxTextView->setPropertyValue(
            OUString::createFromAscii("TextColor"),
            Any(rpPresenterController->GetViewFontColor(sResourceURL)));

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
            ::boost::bind(&PresenterNotesView::SetTop, this, _1));
        mpScrollBar->SetCanvas(mxCanvas);

        Resize();
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

    mpScrollBar = NULL;

    mxViewId = NULL;
}




void PresenterNotesView::SetSlide (const Reference<drawing::XDrawPage>& rxNotesPage)
{
    static const ::rtl::OUString sNotesShapeName (
        OUString::createFromAscii("com.sun.star.presentation.NotesShape"));
    static const ::rtl::OUString sTextShapeName (
        OUString::createFromAscii("com.sun.star.drawing.TextShape"));

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
        mxTextView->setPropertyValue(OUString::createFromAscii("Text"), Any(sText));

        if (mpScrollBar.get() != NULL)
        {
            mpScrollBar->SetThumbPosition(0);
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
    Resize();
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
    (void)rEvent;
    ThrowIfDisposed();
    ::osl::MutexGuard aSolarGuard (::osl::Mutex::getGlobalMutex());
    Paint();
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
    Invalidate();
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
        case awt::Key::UP:
            Scroll(OUString::createFromAscii("-1l"));
            break;

        case awt::Key::DOWN:
            Scroll(OUString::createFromAscii("+1l"));
            break;
    }
}




void SAL_CALL PresenterNotesView::keyReleased (const awt::KeyEvent& rEvent)
    throw (RuntimeException)
{
    switch (rEvent.KeyCode)
    {
        case awt::Key::ADD:
        {
            maFontDescriptor.Height += 1;
            mxTextView->setPropertyValue(
                OUString::createFromAscii("FontDescriptor"),
                Any(maFontDescriptor));
            UpdateScrollBar();
            Invalidate();
        }
        break;

        case awt::Key::SUBTRACT:
        {
            if (maFontDescriptor.Height > 1)
                maFontDescriptor.Height -= 1;
            mxTextView->setPropertyValue(
                OUString::createFromAscii("FontDescriptor"),
                Any(maFontDescriptor));
            UpdateScrollBar();
            Invalidate();
        }
        break;
    }
}




//-----------------------------------------------------------------------------

void PresenterNotesView::Resize (void)
{
    if (mxParentWindow.is() && mpScrollBar.get()!=NULL)
    {
        const awt::Rectangle aWindowBox (mxParentWindow->getPosSize());
        mxTextView->setPropertyValue(
            OUString::createFromAscii("Size"),
            Any(awt::Size(aWindowBox.Width - mpScrollBar->GetSize(),aWindowBox.Height)));
        mpScrollBar->SetPosSize(
            geometry::RealRectangle2D(
                aWindowBox.Width - mpScrollBar->GetSize(),
                0,
                aWindowBox.Width,
                aWindowBox.Height));
        UpdateScrollBar();
    }
    mxBitmap = NULL;
}




void PresenterNotesView::Paint (void)
{
    if (mxParentWindow.is() && mpScrollBar.get() != NULL)
        mpScrollBar->Paint(mxParentWindow->getPosSize());

    if ( ! mxBitmap.is())
    {
        if (mxParentWindow.is())
        {
            awt::Rectangle aWindowBox (mxParentWindow->getPosSize());
            if (mpScrollBar.get() != NULL)
                aWindowBox.Width -= mpScrollBar->GetSize();
            mxBitmap = Reference<rendering::XBitmap>(
                mxTextView->getPropertyValue(OUString::createFromAscii("Bitmap")),
                UNO_QUERY);
        }
    }

    if (mxBitmap.is() && mxCanvas.is())
    {
        const rendering::ViewState aViewState (
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL);
        rendering::RenderState aRenderState(
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL,
            Sequence<double>(3),
            rendering::CompositeOperation::SOURCE);
        mxCanvas->drawBitmap(
            mxBitmap,
            aViewState,
            aRenderState);

        Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
        if (xSpriteCanvas.is())
            xSpriteCanvas->updateScreen(sal_False);
    }
}




void PresenterNotesView::Invalidate (void)
{
    mxBitmap = NULL;
    Reference<awt::XWindowPeer> xPeer (mxParentWindow, UNO_QUERY);
    if (xPeer.is())
        xPeer->invalidate(awt::InvalidateStyle::NOERASE);
}




void PresenterNotesView::Scroll (const OUString& rsDistance)
{
    try
    {
        mxTextView->setPropertyValue(
            OUString::createFromAscii("RelativeTop"),
            Any(rsDistance));

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
        mxTextView->setPropertyValue(
            OUString::createFromAscii("Top"),
            Any(sal_Int32(nTop)));

        UpdateScrollBar();
        Invalidate();
    }
    catch (beans::UnknownPropertyException&)
    {}
}




void PresenterNotesView::UpdateScrollBar (void)
{
    if (mpScrollBar.get() != NULL)
    {
        try
        {
            double nHeight;
            if (mxTextView->getPropertyValue(OUString::createFromAscii("TotalHeight")) >>= nHeight)
                mpScrollBar->SetTotalSize(nHeight);
        }
        catch(beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }

        try
        {

            double nTop;
            if (mxTextView->getPropertyValue(OUString::createFromAscii("Top")) >>= nTop)
                mpScrollBar->SetThumbPosition(nTop);
        }
        catch(beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }

        if (mxParentWindow.is())
            mpScrollBar->SetThumbSize(mxParentWindow->getPosSize().Height);
    }
}




void PresenterNotesView::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterNotesView object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}


} } // end of namespace ::sdext::presenter
