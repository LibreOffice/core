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
 * $Revision: 1.8 $
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
#include <set>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

static const sal_Int32 gnSpaceBelowSeparator (10);
static const sal_Int32 gnSpaceAboveSeparator (10);
static const sal_Int32 gnPartHeight (128);
/** Maximal size of memory used for bitmaps which show the notes text.
*/
static const sal_Int32 gnMaximalCacheSize (8*1024*1024);
static const double gnLineScrollFactor (1.2);

namespace sdext { namespace presenter {

//===== PresenterNotesView::BitmapContainer ===================================

namespace {
    class NotesBitmapDescriptor
    {
    public:
        NotesBitmapDescriptor (const sal_Int32 nTop, const sal_Int32 nBottom,
            const Reference<rendering::XBitmap> xBitmap)
            : Top(nTop), Bottom(nBottom), Bitmap(xBitmap)
        { }
        sal_Int32 Top;
        sal_Int32 Bottom;
        Reference<rendering::XBitmap> Bitmap;
    };
    typedef ::boost::shared_ptr<NotesBitmapDescriptor> SharedNotesBitmapDescriptor;
}
typedef ::std::vector<SharedNotesBitmapDescriptor> BitmapDescriptorSet;


/** Container of bitmaps that show parts of the notes text.  If the bitmaps
    use more than a specified amount of memory then some bitmaps that are
    currently not on the screen are discarded.
*/
class PresenterNotesView::BitmapContainer
    : public BitmapDescriptorSet
{
public:
    BitmapContainer (const ::boost::shared_ptr<BitmapFactory>& rpFactory);

    /** Call this when for instance the font size has changed and all
        bitmaps have to be created anew or the slide has changed.
    */
    void Clear (void);
    void Prune (const sal_Int32 nTop, const sal_Int32 nBottom);
    sal_Int32 GetMemorySize (const Reference<rendering::XBitmap>& rxBitmap) const;
    sal_Int32 GetMemorySize (void) const;
    const_iterator GetBegin (const double nTop, const double nBottom);
    const_iterator GetEnd (const double nTop, const double nBottom);

private:
    ::boost::shared_ptr<BitmapFactory> mpFactory;
    sal_Int32 mnMaximalCacheSize;
    const sal_Int32 mnPartHeight;
    sal_Int32 mnTotalHeight;

    sal_Int32 GetTopIndex (const double nValue) const;
    sal_Int32 GetBottomIndex (const double nValue) const;
    void ProvideBitmaps (
        const sal_Int32 nTopIndex,
        const sal_Int32 nBottomIndex);
};




//===== PresenterNotesView::BitmapFactory =====================================

class PresenterNotesView::BitmapFactory
{
public:
    BitmapFactory (
        const Reference<XComponentContext>& rxComponentContext,
        const PresenterTheme::SharedFontDescriptor& rpFont,
        const Reference<rendering::XCanvas>& rxCanvas,
        const SharedBitmapDescriptor& rpBackground);
    ~BitmapFactory (void);
    void SetText (const OUString& rsText);
    void SetWidth (const sal_Int32 nWidth);
    void SetFontHeight (const sal_Int32 nHeight);
    sal_Int32 GetHeightForWidth (const sal_Int32 nWidth);
    sal_Int32 GetTotalHeight (void);
    Reference<rendering::XBitmap> CreateBitmap (const sal_Int32 nTop, const sal_Int32 nBottom);

private:
    Reference<rendering::XCanvas> mxCanvas;
    OUString msText;
    PresenterTheme::SharedFontDescriptor mpFont;
    css::awt::FontDescriptor maFontDescriptor;
    sal_Int32 mnWidth;
    sal_Int32 mnTotalHeight;
    css::uno::Reference<css::beans::XPropertySet> mxTextView;
    SharedBitmapDescriptor mpBackground;
};




//===== PresenterNotesView ====================================================

PresenterNotesView::PresenterNotesView (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterNotesViewInterfaceBase(m_aMutex),
      mxViewId(rxViewId),
      mpPresenterController(rpPresenterController),
      mxCanvas(),
      mpBitmapContainer(),
      mpBitmapFactory(),
      mxCurrentNotesPage(),
      mpScrollBar(),
      mxToolBarWindow(),
      mxToolBarCanvas(),
      mpToolBar(),
      mpCloseButton(),
      maSeparatorColor(0xffffff),
      mnSeparatorYLocation(0),
      maTextBoundingBox(),
      mpBackground(),
      mnTop(0),
      mnFontSize(12)
{
    try
    {
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        Reference<XConfigurationController> xCC (xCM->getConfigurationController(), UNO_QUERY_THROW);
        Reference<XPane> xPane (xCC->getResource(rxViewId->getAnchor()), UNO_QUERY_THROW);

        mxParentWindow = xPane->getWindow();
        mxCanvas = xPane->getCanvas();

        const OUString sResourceURL (mxViewId->getResourceURL());
        PresenterTheme::SharedFontDescriptor pFont(
            rpPresenterController->GetViewFont(sResourceURL));
        mpBitmapFactory.reset(new BitmapFactory(
            rxComponentContext,
            pFont,
            mxCanvas,
            mpPresenterController->GetViewBackground(mxViewId->getResourceURL())));
        mpBitmapContainer.reset(new BitmapContainer(mpBitmapFactory));

        maSeparatorColor = pFont->mnColor;
        mnFontSize = pFont->mnSize;

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
        mpScrollBar->SetBackground(
            mpPresenterController->GetViewBackground(mxViewId->getResourceURL()));

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
    if (xIndexAccess.is())
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

        mpBitmapFactory->SetText(sText);

        Layout();
        mpBitmapContainer->Clear();

        if (mpScrollBar.get() != NULL)
        {
            mpScrollBar->SetThumbPosition(0, false);
            UpdateScrollBar();
        }

        Invalidate();
    }
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
            Scroll(-gnLineScrollFactor * mnFontSize);
            break;

        case awt::Key::Y:
        case awt::Key::Z:
            Scroll(+gnLineScrollFactor * mnFontSize);
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
            const sal_Int32 nHeight (mpBitmapFactory->GetHeightForWidth(
                sal_Int32(aNewTextBoundingBox.X2 - aNewTextBoundingBox.X1)));
            if (nHeight > nTextBoxHeight)
            {
                bShowVerticalScrollbar = true;
                aNewTextBoundingBox.X2 -= mpScrollBar->GetSize();
            }
            mpScrollBar->SetTotalSize(nHeight);
        }
        catch(beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }

        mpScrollBar->SetVisible(bShowVerticalScrollbar);
        mpBitmapFactory->SetWidth(sal_Int32(aNewTextBoundingBox.X2 - aNewTextBoundingBox.X1));
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
            mpBitmapContainer->Clear();
        }
    }
}




void PresenterNotesView::Paint (const awt::Rectangle& rUpdateBox)
{
    if ( ! mxParentWindow.is())
        return;
    if ( ! mxCanvas.is())
        return;

    if (mpBackground.get() == NULL)
        mpBackground = mpPresenterController->GetViewBackground(mxViewId->getResourceURL());

    if (rUpdateBox.Y < maTextBoundingBox.Y2
        && rUpdateBox.X < maTextBoundingBox.X2)
    {
        PaintText(rUpdateBox);
    }

    if (rUpdateBox.Y + rUpdateBox.Height > maTextBoundingBox.Y2)
    {
        PaintToolBar(rUpdateBox);
    }
}




void PresenterNotesView::PaintToolBar (const awt::Rectangle& rUpdateBox)
{
    awt::Rectangle aWindowBox (mxParentWindow->getPosSize());

    rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);
    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    if (mpBackground.get() != NULL)
    {
        // Paint the background.
        mpPresenterController->GetCanvasHelper()->Paint(
            mpBackground,
            mxCanvas,
            rUpdateBox,
            awt::Rectangle(0,sal_Int32(maTextBoundingBox.Y2),aWindowBox.Width,aWindowBox.Height),
            awt::Rectangle());
    }

    // Paint the horizontal separator.
    OSL_ASSERT(mxViewId.is());
    PresenterCanvasHelper::SetDeviceColor(aRenderState, maSeparatorColor);

    mxCanvas->drawLine(
        geometry::RealPoint2D(0,mnSeparatorYLocation),
        geometry::RealPoint2D(aWindowBox.Width,mnSeparatorYLocation),
        aViewState,
        aRenderState);
}




void PresenterNotesView::PaintText (const awt::Rectangle& rUpdateBox)
{
    const awt::Rectangle aBox (PresenterGeometryHelper::Intersection(rUpdateBox,
            PresenterGeometryHelper::ConvertRectangle(maTextBoundingBox)));

    if (aBox.Width <= 0 || aBox.Height <= 0)
        return;

    rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(aBox, mxCanvas->getDevice()));
    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(3),
        rendering::CompositeOperation::SOURCE);

    if (mpBackground.get() != NULL)
    {
        // Paint the background.
        mpPresenterController->GetCanvasHelper()->Paint(
            mpBackground,
            mxCanvas,
            rUpdateBox,
            aBox,
            awt::Rectangle());
    }

    // Iterator over all bitmaps that are (partially) visible and paint
    // them.
    const double nBottom (mnTop + maTextBoundingBox.Y2 - maTextBoundingBox.Y1);
    BitmapContainer::const_iterator iBitmap (mpBitmapContainer->GetBegin(mnTop, nBottom));
    BitmapContainer::const_iterator iEnd (mpBitmapContainer->GetEnd(mnTop, nBottom));
    for ( ; iBitmap!=iEnd; ++iBitmap)
    {
        if (iBitmap->get() != NULL && (*iBitmap)->Bitmap.is())
        {
            aRenderState.AffineTransform.m02 = maTextBoundingBox.X1;
            aRenderState.AffineTransform.m12 = (*iBitmap)->Top + maTextBoundingBox.Y1 - mnTop;
            mxCanvas->drawBitmap((*iBitmap)->Bitmap, aViewState, aRenderState);
        }
    }

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}




void PresenterNotesView::Invalidate (void)
{
    mpPresenterController->GetPaintManager()->Invalidate(
        mxParentWindow,
        PresenterGeometryHelper::ConvertRectangle(maTextBoundingBox));
}




void PresenterNotesView::Scroll (const double rnDistance)
{
    try
    {
        mnTop += rnDistance;

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
        mnTop = nTop;

        UpdateScrollBar();
        Invalidate();
    }
    catch (beans::UnknownPropertyException&)
    {}
}




void PresenterNotesView::ChangeFontSize (const sal_Int32 nSizeChange)
{
    const sal_Int32 nNewSize (mnFontSize + nSizeChange);
    if (nNewSize > 5)
    {
        mnFontSize = nNewSize;
        mpBitmapFactory->SetFontHeight(mnFontSize);
        mpBitmapContainer->Clear();

        Layout();
        UpdateScrollBar();
        Invalidate();

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
            double nHeight = mpBitmapFactory->GetTotalHeight();
            mpScrollBar->SetTotalSize(nHeight);
        }
        catch(beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }

        mpScrollBar->SetLineHeight(mnFontSize*1.2);
        mpScrollBar->SetThumbPosition(mnTop, false);

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




//===== PresenterNotesView::BitmapContainer ===================================

PresenterNotesView::BitmapContainer::BitmapContainer (
    const ::boost::shared_ptr<BitmapFactory>& rpFactory)
    : mpFactory(rpFactory),
      mnMaximalCacheSize(gnMaximalCacheSize),
      mnPartHeight(gnPartHeight),
      mnTotalHeight(0)
{
}




void PresenterNotesView::BitmapContainer::Clear (void)
{
    clear();
    mnTotalHeight = mpFactory->GetTotalHeight();
    const sal_Int32 nSize ((mnTotalHeight+mnPartHeight-1) / mnPartHeight);
    if (nSize > 0)
        resize(nSize, SharedNotesBitmapDescriptor());
}




void PresenterNotesView::BitmapContainer::Prune (
    const sal_Int32 nTopIndex,
    const sal_Int32 nBottomIndex)
{
    sal_Int32 nDistance (::std::max(nTopIndex-0, sal_Int32(size())-nBottomIndex));
    sal_Int32 nIndex;
    sal_Int32 nTotalSize (GetMemorySize());
    iterator iPart;

    while (nDistance > 0
        && nTotalSize > mnMaximalCacheSize)
    {
        // Remove bitmap that is nDistance places before nTopIndex.
        nIndex = nTopIndex - nDistance;
        if (nIndex >= 0)
        {
            iPart = begin() + nIndex;
            nTotalSize -= GetMemorySize((*iPart)->Bitmap);
            (*iPart)->Bitmap = NULL;
        }

        if (nTotalSize <= mnMaximalCacheSize)
            break;

        // Remove bitmap that is nDistance places behind nBottomIndex.
        nIndex = nBottomIndex + nDistance;
        if (nIndex < sal_Int32(size()))
        {
            iPart = begin() + nIndex;
            nTotalSize -= GetMemorySize((*iPart)->Bitmap);
            (*iPart)->Bitmap = NULL;
        }
    }
}




sal_Int32 PresenterNotesView::BitmapContainer::GetMemorySize (
    const Reference<rendering::XBitmap>& rxBitmap) const
{
    if (rxBitmap.is())
    {
        const geometry::IntegerSize2D aSize (rxBitmap->getSize());
        return aSize.Width * aSize.Height * 3;
    }
    return 0;
}




sal_Int32 PresenterNotesView::BitmapContainer::GetMemorySize (void) const
{
    sal_Int32 nSize (0);
    for (const_iterator iBitmap=begin(); iBitmap!=end(); ++iBitmap)
        if (iBitmap->get() != NULL)
            if ((*iBitmap)->Bitmap.is())
                nSize += GetMemorySize((*iBitmap)->Bitmap);
    return nSize;
}




sal_Int32 PresenterNotesView::BitmapContainer::GetTopIndex (const double nValue) const
{
    const sal_Int32 nIndex (sal::static_int_cast<sal_Int32>(nValue) / mnPartHeight);
    if (nIndex < 0)
        return 0;
    else if (nIndex >= sal_Int32(size()))
        return sal_Int32(size())-1;
    else
        return nIndex;
}




sal_Int32 PresenterNotesView::BitmapContainer::GetBottomIndex (const double nValue) const
{
    const sal_Int32 nIndex ((sal::static_int_cast<sal_Int32>(nValue)+mnPartHeight-1) / mnPartHeight);
    if (nIndex >= sal_Int32(size()))
        return size()-1;
    else if (nIndex < 0)
        return -1;
    else return nIndex;
}




PresenterNotesView::BitmapContainer::const_iterator PresenterNotesView::BitmapContainer::GetBegin (
    const double nTop,
    const double nBottom)
{
    const sal_Int32 nTopIndex (GetTopIndex(nTop));
    const sal_Int32 nBottomIndex(GetBottomIndex(nBottom));
    ProvideBitmaps(nTopIndex, nBottomIndex);
    if (nTopIndex >= 0)
        return begin()+nTopIndex;
    else
        return end();
}




PresenterNotesView::BitmapContainer::const_iterator PresenterNotesView::BitmapContainer::GetEnd (
    const double nTop,
    const double nBottom)
{
    (void)nTop;
    const sal_Int32 nIndex (GetBottomIndex(nBottom));
    if (nIndex >= 0)
        return (begin() + nIndex)+1;
    else
        return end();
}




void PresenterNotesView::BitmapContainer::ProvideBitmaps (
    const sal_Int32 nTopIndex,
    const sal_Int32 nBottomIndex)
{
    BitmapDescriptorSet aNewBitmaps;

    if (nTopIndex < 0 || nBottomIndex<0)
        return;

    if (nTopIndex > nBottomIndex)
        return;

    for (sal_Int32 nIndex=nTopIndex; nIndex<=nBottomIndex; ++nIndex)
    {
        iterator iPart (begin() + nIndex);
        const sal_Int32 nTop (nIndex * mnPartHeight);
        const sal_Int32 nBottom (nTop + mnPartHeight - 1);
        if (iPart->get() == NULL)
        {
            iPart->reset(new NotesBitmapDescriptor(nTop, nBottom, NULL));
        }
        if ( ! (*iPart)->Bitmap.is())
        {
            (*iPart)->Bitmap = mpFactory->CreateBitmap(nTop, nBottom);
            (*iPart)->Top = nTop;
            (*iPart)->Bottom = nTop + (*iPart)->Bitmap->getSize().Height;
        }
    }

    // Calculate memory size used by all bitmaps.
    if (GetMemorySize() > mnMaximalCacheSize)
        Prune(nTopIndex,nBottomIndex);
}




//===== PresenterNotesView::BitmapFactory =====================================

PresenterNotesView::BitmapFactory::BitmapFactory (
    const Reference<XComponentContext>& rxComponentContext,
    const PresenterTheme::SharedFontDescriptor& rpFont,
    const Reference<rendering::XCanvas>& rxCanvas,
    const SharedBitmapDescriptor& rpBackground)
    : mxCanvas(rxCanvas),
      msText(),
      mpFont(rpFont),
      maFontDescriptor(),
      mnWidth(100),
      mnTotalHeight(0),
      mxTextView(),
      mpBackground(rpBackground)
{
    Reference<lang::XMultiComponentFactory> xFactory (
        rxComponentContext->getServiceManager(), UNO_QUERY_THROW);
    Sequence<Any> aArguments(1);
    aArguments[0] <<= rxCanvas;
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
}




PresenterNotesView::BitmapFactory::~BitmapFactory (void)
{
    {
        Reference<XComponent> xComponent (mxTextView, UNO_QUERY);
        mxTextView = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }
}




void PresenterNotesView::BitmapFactory::SetText (const OUString& rsText)
{
    if (mxTextView.is())
        mxTextView->setPropertyValue(A2S("Text"), Any(rsText));
    mnTotalHeight = 0;
}




void PresenterNotesView::BitmapFactory::SetWidth (const sal_Int32 nWidth)
{
    mnWidth = nWidth;
}




void PresenterNotesView::BitmapFactory::SetFontHeight (const sal_Int32 nHeight)
{
    maFontDescriptor.Height = sal::static_int_cast<sal_Int16>(nHeight);
    mxTextView->setPropertyValue(A2S("FontDescriptor"), Any(maFontDescriptor));
    if (mpFont.get() != NULL)
        mpFont->mnSize = nHeight;
    mnTotalHeight = 0;
}




sal_Int32 PresenterNotesView::BitmapFactory::GetHeightForWidth (const sal_Int32 nWidth)
{
    mxTextView->setPropertyValue(
        A2S("Size"),
        Any(awt::Size(nWidth, 100)));
    sal_Int32 nHeight (0);
    if (mxTextView->getPropertyValue(A2S("TotalHeight")) >>= nHeight)
        return nHeight;
    else
        return -1;
}




sal_Int32 PresenterNotesView::BitmapFactory::GetTotalHeight (void)
{
    if (mnTotalHeight == 0)
    {
        sal_Int32 nHeight (0);
        if (mxTextView->getPropertyValue(A2S("TotalHeight")) >>= nHeight)
            mnTotalHeight = nHeight;
    }
    return mnTotalHeight;
}




Reference<rendering::XBitmap> PresenterNotesView::BitmapFactory::CreateBitmap (
    const sal_Int32 nTop,
    const sal_Int32 nBottom)
{
    // Get text bitmap.
    sal_Int32 nHeight;
    if (nBottom > GetTotalHeight())
        nHeight = GetTotalHeight() - nTop + 1;
    else
        nHeight = nBottom - nTop + 1;
    mxTextView->setPropertyValue(A2S("Size"), Any(awt::Size(mnWidth,nHeight)));
    mxTextView->setPropertyValue(A2S("Top"), Any(sal_Int32(nTop)));
    Reference<rendering::XBitmap> xTextBitmap (
        mxTextView->getPropertyValue(A2S("Bitmap")), UNO_QUERY);

    return xTextBitmap;
}




} } // end of namespace ::sdext::presenter
