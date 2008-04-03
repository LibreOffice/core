/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterToolBar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:06:28 $
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

#include "PresenterToolBar.hxx"

#include "PresenterBitmapContainer.hxx"
#include "PresenterGeometryHelper.hxx"
//#include "PresenterHelper.hxx"

#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/deployment/XPackageInformationProvider.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/RenderState.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/ViewState.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/util/Color.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <map>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))

namespace {
const sal_Int32 gnButtonSize = 64;
}

namespace sdext { namespace presenter {

//===== PresenterToolBar::Element =============================================

class PresenterToolBar::Element
{
public:
    explicit Element (
        const Reference<awt::XWindowPeer>& rxWindowPeer);
    explicit Element (
        const Reference<awt::XWindowPeer>& rxWindowPeer,
        const awt::Rectangle& rBoundingBox);
    virtual ~Element (void);

    virtual void SetCurrentSlide (
        const Reference<drawing::XDrawPage>& rxCurrentSlide,
        const sal_Int32 nSlideCount);
    virtual void SetPosSize (const awt::Rectangle& rBox);
    virtual awt::Rectangle GetBoundingBox (void) const;
    virtual void Paint (
        const Reference<rendering::XCanvas>& rxCanvas,
        const rendering::ViewState& rViewState) = 0;
    virtual bool SetState (const bool bIsOver, const bool bIsPressed);
    virtual void Invalidate (void);
    virtual bool IsOutside (const awt::Rectangle& rBox);

protected:
    Reference<awt::XWindowPeer> mxWindowPeer;
    awt::Rectangle maBoundingBox;
};




//===== PresenterToolBar::Button ==============================================

class Button : public PresenterToolBar::Element
{
public:
    typedef ::boost::function<void()> Callback;

    /**
        @param rxWindowPeer
            The window peer is used to invalidate the region of the button
            to enforce its repaint.
    */
    Button (
        const Reference<awt::XWindowPeer>& rxWindowPeer,
        const OUString& rsText,
        const PresenterBitmapContainer::BitmapSet& rIcons,
        const Callback& rCallback);

    virtual void Paint (
        const Reference<rendering::XCanvas>& rxCanvas,
        const rendering::ViewState& rViewState);
    virtual bool SetState (const bool bIsOver, const bool bIsPressed);

private:
    OUString msText;
    bool mbIsOver;
    bool mbIsPressed;
    PresenterBitmapContainer::BitmapSet maIcons;
    bool mbIconLoadingFailed;
    Callback maCallback;
};




//===== PresenterToolBar::Label ===============================================

class Label : public PresenterToolBar::Element
{
public:
    Label (
        const Reference<awt::XWindowPeer>& rxPeer,
        const util::Color& rFontColor,
        const rendering::FontRequest& rRequestedFontDescriptor);

    virtual void SetCurrentSlide (
        const Reference<drawing::XDrawPage>& rxCurrentSlide,
        const sal_Int32 nSlideCount);
    void SetText (const OUString& rsText);
    virtual void Paint (
        const Reference<rendering::XCanvas>& rxCanvas,
        const rendering::ViewState& rViewState);

private:
    OUString msText;
    Reference<rendering::XCanvasFont> mxFont;
    util::Color maFontColor;
    rendering::FontRequest maRequestedFontDescriptor;
};




//===== PresenterToolBar ======================================================

PresenterToolBar::PresenterToolBar (
    const Reference<XComponentContext>& rxContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterToolBarInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mxPane(),
      mxViewId(rxViewId),
      mxCanvas(),
      maElementContainer(),
      mxWindow(),
      mxControl(),
      mxCurrentSlide(),
      mxSlideShowController(),
      mpPresenterController(rpPresenterController),
      msPreviousButtonBitmapURL(),
      msNextButtonBitmapURL(),
      mpIconContainer()
{
    try
    {
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        Reference<XConfigurationController> xCC(xCM->getConfigurationController(),UNO_QUERY_THROW);
        mxPane = Reference<XPane>(xCC->getResource(rxViewId->getAnchor()), UNO_QUERY_THROW);

        mxWindow = mxPane->getWindow();
        mxCanvas = mxPane->getCanvas();

        mpIconContainer.reset(new PresenterBitmapContainer(rxContext, mxCanvas,
                A2S("PresenterScreenSettings/ToolBar/Bitmaps")));
        CreateControls();

        // Set background of tool bar.
        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
        if (xPeer.is())
        {
            xPeer->setBackground(
                mpPresenterController->GetViewBackgroundColor(mxViewId->getResourceURL()));
        }

        if (mxWindow.is())
        {
            mxWindow->addWindowListener(this);
            mxWindow->addPaintListener(this);
            mxWindow->addMouseListener(this);
            mxWindow->addMouseMotionListener(this);

            xPeer = Reference<awt::XWindowPeer>(mxWindow, UNO_QUERY);
            if (xPeer.is())
                xPeer->setBackground(util::Color(0xff000000));

            mxWindow->setVisible(sal_True);
        }


        mxSlideShowController = rpPresenterController->GetSlideShowController();
        UpdateSlideNumber();
        Resize();
    }
    catch (RuntimeException&)
    {
        mxViewId = NULL;
        maElementContainer.clear();
        mxControl = NULL;
        throw;
    }
}




PresenterToolBar::~PresenterToolBar (void)
{
}




void SAL_CALL PresenterToolBar::disposing (void)
{
    if (mxWindow.is())
    {
        mxWindow->removeWindowListener(this);
        mxWindow->removePaintListener(this);
        mxWindow->removeMouseListener(this);
        mxWindow->removeMouseMotionListener(this);
        mxWindow = NULL;
    }
    Reference<lang::XComponent> xComponent (mxControl, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
    mxControl = NULL;
    maElementContainer.clear();
    mxViewId = NULL;
}




//-----  lang::XEventListener -------------------------------------------------

void SAL_CALL PresenterToolBar::disposing (const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (rEventObject.Source == mxWindow)
        mxWindow = NULL;
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterToolBar::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    Resize();
}




void SAL_CALL PresenterToolBar::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterToolBar::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterToolBar::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterToolBar::windowPaint (const css::awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    if ( ! mxCanvas.is())
        return;

    const rendering::ViewState aViewState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rEvent.UpdateRect, mxCanvas->getDevice()));

    Clear(rEvent.UpdateRect, aViewState);
    Paint(rEvent.UpdateRect, aViewState);

    // Make the back buffer visible.
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}




//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterToolBar::mousePressed (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    CheckMouseOver(rEvent, true, true);
}




void SAL_CALL PresenterToolBar::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    CheckMouseOver(rEvent, true);
}




void SAL_CALL PresenterToolBar::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    CheckMouseOver(rEvent, true);
}




void SAL_CALL PresenterToolBar::mouseExited (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    CheckMouseOver(rEvent, false);
}




//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterToolBar::mouseMoved (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();

    CheckMouseOver(rEvent, true);
}




void SAL_CALL PresenterToolBar::mouseDragged (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    (void)rEvent;
}




//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterToolBar::getResourceId (void)
    throw (RuntimeException)
{
    return mxViewId;
}




sal_Bool SAL_CALL PresenterToolBar::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}




//----- XDrawView -------------------------------------------------------------

void SAL_CALL PresenterToolBar::setCurrentPage (const Reference<drawing::XDrawPage>& rxSlide)
    throw (RuntimeException)
{
    if (mxCurrentSlide != rxSlide)
    {
        mxCurrentSlide = rxSlide;
        UpdateSlideNumber();
    }
}




Reference<drawing::XDrawPage> SAL_CALL PresenterToolBar::getCurrentPage (void)
    throw (RuntimeException)
{
    return mxCurrentSlide;
}




//-----------------------------------------------------------------------------

void PresenterToolBar::GotoPreviousSlide (void)
{
    if (mxSlideShowController.is())
    {
        mxSlideShowController->gotoPreviousSlide();
        // Going to the previous slide does not trigger any event that would
        // cause an update of the presenter view.  Therefore request an
        // update explicitly.
        if (mpPresenterController.is())
            mpPresenterController->UpdateCurrentSlide(0);
    }
}




void PresenterToolBar::GotoNextEffect (void)
{
    if (mxSlideShowController.is())
    {
        mxSlideShowController->gotoNextEffect();
        // When playing the next effect causes a change to the next slide
        // then events are sent that cause an update of the presenter view.
    }
}




void PresenterToolBar::GotoNextSlide (void)
{
    if (mxSlideShowController.is())
    {
        mxSlideShowController->gotoNextSlide();
        // Going to the next slide does not trigger any event that would
        // cause an update of the presenter view.  Therefore request an
        // update explicitly.
        if (mpPresenterController.is())
            mpPresenterController->UpdateCurrentSlide(0);
    }
}




void PresenterToolBar::CreateControls (void)
{
    if ( ! mxWindow.is())
        return;

    // Expand the macro in the bitmap file names.
    Reference<deployment::XPackageInformationProvider> xInformationProvider (
        mxComponentContext->getValueByName(
            A2S("/singletons/com.sun.star.deployment.PackageInformationProvider")),
        UNO_QUERY);
    Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);

    maElementContainer.push_back(boost::shared_ptr<Element>(new Button(
        xPeer,
        A2S("Back"),
        mpIconContainer->GetButtons(A2S("PreviousSlide")),
        ::boost::bind(&PresenterToolBar::GotoPreviousSlide,this))));

    maElementContainer.push_back(boost::shared_ptr<Element>(new Label(
        xPeer,
        mpPresenterController->GetViewFontColor(mxViewId->getResourceURL()),
        mpPresenterController->GetViewFontRequest(mxViewId->getResourceURL()))));

    maElementContainer.push_back(boost::shared_ptr<Element>(new Button(
        xPeer,
        A2S("Next\nEffect"),
        mpIconContainer->GetButtons(A2S("NextEffect")),
        ::boost::bind(&PresenterToolBar::GotoNextEffect,this))));

    maElementContainer.push_back(boost::shared_ptr<Element>(new Button(
        xPeer,
        A2S("Next\nSlide"),
        mpIconContainer->GetButtons(A2S("NextSlide")),
        ::boost::bind(&PresenterToolBar::GotoNextSlide,this))));
}




void PresenterToolBar::Resize (void)
{
    if (mxWindow.is())
    {
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        if (mxControl.is())
            mxControl->setPosSize(0,0, aWindowBox.Width, aWindowBox.Height, awt::PosSize::POSSIZE);

        // Calculate the summed width of all elements.
        sal_Int32 nTotalControlWidth (0);
        sal_Int32 nMaximalHeight (0);
        sal_Int32 nElementCount (0);
        ElementContainer::iterator iElement;
        for (iElement=maElementContainer.begin(); iElement!=maElementContainer.end(); ++iElement)
        {
            if (iElement->get() != NULL)
            {
                ++nElementCount;
                nTotalControlWidth += (*iElement)->GetBoundingBox().Width;
                const sal_Int32 nHeight ((*iElement)->GetBoundingBox().Height);
                if (nHeight > nMaximalHeight)
                    nMaximalHeight = nHeight;
            }
        }

        // Calculate the gaps between elements.
        sal_Int32 nGapSize = (aWindowBox.Width - nTotalControlWidth) / 3;
        if (nGapSize > 10)
            nGapSize = 10;

        sal_Int32 nX = (aWindowBox.Width - (nTotalControlWidth + (nElementCount-1)*nGapSize)) / 2;
        sal_Int32 nY = (aWindowBox.Height - nMaximalHeight) / 2;

        // Place the elements.
        for (iElement=maElementContainer.begin(); iElement!=maElementContainer.end(); ++iElement)
        {
            if (iElement->get() != NULL)
            {
                const awt::Rectangle aBox ((*iElement)->GetBoundingBox());
                (*iElement)->SetPosSize(awt::Rectangle(nX,nY, aBox.Width, aBox.Height));

                nX += aBox.Width + nGapSize;
            }
        }

        // The whole window has to be repainted.
        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->invalidate(awt::InvalidateStyle::CHILDREN);
    }
}




void PresenterToolBar::Clear (
    const awt::Rectangle& rUpdateBox,
    const rendering::ViewState& rViewState)
{
    OSL_ASSERT(mxCanvas.is());

    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(3),
        rendering::CompositeOperation::SOURCE);
    const sal_Int32 nColor (
        mpPresenterController->GetViewBackgroundColor(mxViewId->getResourceURL()));
    aRenderState.DeviceColor[0] = ((nColor&0x00ff0000) >> 16) / 255.0;
    aRenderState.DeviceColor[1] = ((nColor&0x0000ff00) >>  8) / 255.0;
    aRenderState.DeviceColor[2] = ((nColor&0x000000ff) >>  0) / 255.0;

    Reference<rendering::XPolyPolygon2D> xRectangle (
        PresenterGeometryHelper::CreatePolygon(rUpdateBox, mxCanvas->getDevice()));
    if (xRectangle.is())
        mxCanvas->fillPolyPolygon(xRectangle, rViewState, aRenderState);
}




void PresenterToolBar::Paint (
    const awt::Rectangle& rUpdateBox,
    const rendering::ViewState& rViewState)
{
    OSL_ASSERT(mxCanvas.is());

    ElementContainer::iterator iElement;
    for (iElement=maElementContainer.begin(); iElement!=maElementContainer.end(); ++iElement)
    {
        if (iElement->get() != NULL)
        {
            if ( ! (*iElement)->IsOutside(rUpdateBox))
                (*iElement)->Paint(mxCanvas, rViewState);
        }
    }
}




void PresenterToolBar::UpdateSlideNumber (void)
{
    if( mxSlideShowController.is() )
    {
        ElementContainer::iterator iElement;
        for (iElement=maElementContainer.begin();
             iElement!=maElementContainer.end();
             ++iElement)
        {
            if (iElement->get() != NULL)
                (*iElement)->SetCurrentSlide(mxCurrentSlide, mxSlideShowController->getSlideCount());
        }
    }

    // All elements may have to be repainted.
    Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
    if (xPeer.is())
        xPeer->invalidate(awt::InvalidateStyle::CHILDREN);
}




void PresenterToolBar::CheckMouseOver (
    const css::awt::MouseEvent& rEvent,
    const bool bOverWindow,
    const bool bMouseDown)
{
    ElementContainer::iterator iElement;
    for (iElement=maElementContainer.begin();
         iElement!=maElementContainer.end();
         ++iElement)
    {
        if (iElement->get() == NULL)
            continue;

        awt::Rectangle aBox ((*iElement)->GetBoundingBox());
        const bool bIsOver = bOverWindow
            && aBox.X <= rEvent.X
            && aBox.Width+aBox.X-1 >= rEvent.X
            && aBox.Y <= rEvent.Y
            && aBox.Height+aBox.Y-1 >= rEvent.Y;
        (*iElement)->SetState(
            bIsOver,
            bIsOver && rEvent.Buttons!=0 && bMouseDown && rEvent.ClickCount>0);
    }
}




void PresenterToolBar::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterToolBar has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




//===== PresenterToolBar::Element =============================================

PresenterToolBar::Element::Element (const Reference<awt::XWindowPeer>& rxWindowPeer)
    : mxWindowPeer(rxWindowPeer),
      maBoundingBox()
{
}




PresenterToolBar::Element::Element (
    const Reference<awt::XWindowPeer>& rxWindowPeer,
    const awt::Rectangle& rBoundingBox)
    : mxWindowPeer(rxWindowPeer),
      maBoundingBox(rBoundingBox)
{
}




PresenterToolBar::Element::~Element (void)
{
}




void PresenterToolBar::Element::SetCurrentSlide (
    const Reference<drawing::XDrawPage>& rxCurrentSlide,
    const sal_Int32 nSlideCount)
{
    (void)rxCurrentSlide;
    (void)nSlideCount;
}




void PresenterToolBar::Element::SetPosSize (const awt::Rectangle& rBox)
{
    maBoundingBox = rBox;
}




awt::Rectangle PresenterToolBar::Element::GetBoundingBox (void) const
{
    return maBoundingBox;
}




bool PresenterToolBar::Element::SetState (
    const bool bIsOver,
    const bool bIsPressed)
{
    (void)bIsOver;
    (void)bIsPressed;
    return false;
}




void PresenterToolBar::Element::Invalidate (void)
{
    if (mxWindowPeer.is())
        mxWindowPeer->invalidateRect(maBoundingBox, awt::InvalidateStyle::UPDATE);
}




bool PresenterToolBar::Element::IsOutside (const awt::Rectangle& rBox)
{
    if (rBox.X >= maBoundingBox.X+maBoundingBox.Width)
        return true;
    else if (rBox.Y >= maBoundingBox.Y+maBoundingBox.Height)
        return true;
    else if (maBoundingBox.X >= rBox.X+rBox.Width)
        return true;
    else if (maBoundingBox.Y >= rBox.Y+rBox.Height)
        return true;
    else
        return false;
}




//===== PresenterToolBar::Button ==============================================

Button::Button (
    const Reference<awt::XWindowPeer>& rxWindowPeer,
    const OUString& rsText,
    const PresenterBitmapContainer::BitmapSet& rIcons,
    const Callback& rCallback)
    : Element(rxWindowPeer, awt::Rectangle(0,0,gnButtonSize,gnButtonSize)),
      msText(rsText),
      mbIsOver(false),
      mbIsPressed(false),
      maIcons(rIcons),
      mbIconLoadingFailed(false),
      maCallback(rCallback)
{
}




void Button::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState)
{
    OSL_ASSERT(rxCanvas.is());

    Reference<rendering::XBitmap> xBitmap;
    if (mbIsPressed)
        xBitmap = maIcons.mxButtonDownIcon;
    else if (mbIsOver)
        xBitmap = maIcons.mxMouseOverIcon;
    else
        xBitmap = maIcons.mxNormalIcon;

    if (xBitmap.is())
    {
        const rendering::RenderState aRenderState(
            geometry::AffineMatrix2D(1,0,maBoundingBox.X, 0,1,maBoundingBox.Y),
            NULL,
            Sequence<double>(3),
            rendering::CompositeOperation::OVER);
        rxCanvas->drawBitmap(xBitmap, rViewState, aRenderState);
    }
}




bool Button::SetState (
    const bool bIsOver,
    const bool bIsPressed)
{
    bool bModified (mbIsOver != bIsOver || mbIsPressed != bIsPressed);
    bool bClicked (mbIsPressed && bIsOver && ! bIsPressed);

    mbIsOver = bIsOver;
    mbIsPressed = bIsPressed;

    if (bModified)
        Invalidate();

    if (bClicked)
        maCallback();

    return bModified;
}




//===== PresenterToolBar::Label ===============================================

Label::Label (
    const Reference<awt::XWindowPeer>& rxWindowPeer,
    const util::Color& rFontColor,
    const rendering::FontRequest& rFontDescriptor)
    : Element(rxWindowPeer, awt::Rectangle(0,0,100,gnButtonSize)),
      msText(),
      mxFont(),
      maFontColor(rFontColor),
      maRequestedFontDescriptor(rFontDescriptor)
{
    maRequestedFontDescriptor.CellSize = gnButtonSize / 2;
}




void Label::SetCurrentSlide (
    const Reference<drawing::XDrawPage>& rxCurrentSlide,
    const sal_Int32 nSlideCount)
{
    try
    {
        Reference<beans::XPropertySet> xSlideProperties (rxCurrentSlide, UNO_QUERY_THROW);
        const Any aSlideNumber (xSlideProperties->getPropertyValue(
            OUString::createFromAscii("Number")));
        sal_Int32 nSlideNumber (0);
        if (aSlideNumber >>= nSlideNumber)
        {
            SetText(
                OUString::valueOf(nSlideNumber)
                    + OUString::createFromAscii(" / ")
                        + OUString::valueOf(nSlideCount));
        }
    }
    catch (RuntimeException&)
    {
    }
}




void Label::SetText (const OUString& rsText)
{
    msText = rsText;
    Invalidate();
}




void Label::Paint (
    const Reference<rendering::XCanvas>& rxCanvas,
    const rendering::ViewState& rViewState)
{
    OSL_ASSERT(rxCanvas.is());

    if ( ! mxFont.is())
        mxFont = rxCanvas->createFont(
            maRequestedFontDescriptor,
            Sequence<beans::PropertyValue>(),
            geometry::Matrix2D(1,0,0,1));

    if (mxFont.is())
    {
        rendering::StringContext aContext (msText, 0, msText.getLength());

        Reference<rendering::XTextLayout> xLayout (
            mxFont->createTextLayout(aContext, rendering::TextDirection::WEAK_LEFT_TO_RIGHT, 0));

        geometry::RealRectangle2D aBox (xLayout->queryTextBounds());
        const double nTextWidth = aBox.X2 - aBox.X1;
        const double nTextHeight = aBox.Y2 - aBox.Y1;
        const double nY = maBoundingBox.Y + (maBoundingBox.Height - nTextHeight)/2
            + 3*nTextHeight/4;
        const double nX = maBoundingBox.X + (maBoundingBox.Width - nTextWidth)/2;

        rendering::RenderState aRenderState(
            geometry::AffineMatrix2D(1,0,nX, 0,1,nY),
            NULL,
            Sequence<double>(3),
            rendering::CompositeOperation::SOURCE);
        aRenderState.DeviceColor[0] = ((maFontColor&0x00ff0000)>>16) / 255.0;
        aRenderState.DeviceColor[1] = ((maFontColor&0x0000ff00)>>8) / 255.0;
        aRenderState.DeviceColor[2] = (maFontColor&0x000000ff) / 255.0;

        rxCanvas->drawText(
            aContext,
            mxFont,
            rViewState,
            aRenderState,
            rendering::TextDirection::WEAK_LEFT_TO_RIGHT);
    }
}




} } // end of namespace ::sdext::presenter
