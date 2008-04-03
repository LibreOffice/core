/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterHelpView.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 15:58:56 $
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

#include "PresenterHelpView.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterHelper.hxx"
#include "PresenterWindowManager.hxx"
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/util/Color.hpp>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

PresenterHelpView::PresenterHelpView (
    const Reference<uno::XComponentContext>& rxContext,
    const Reference<XResourceId>& rxViewId,
    const Reference<frame::XController>& rxController,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterHelpViewInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mxViewId(rxViewId),
      mxPane(),
      mxWindow(),
      mpPresenterController(rpPresenterController),
      mxCanvas(),
      mxFont()
{
    try
    {
        // Get the content window via the pane anchor.
        Reference<XControllerManager> xCM (rxController, UNO_QUERY_THROW);
        Reference<XConfigurationController> xCC (
            xCM->getConfigurationController(), UNO_QUERY_THROW);
        mxPane = Reference<XPane>(xCC->getResource(rxViewId->getAnchor()), UNO_QUERY_THROW);

        mxWindow = mxPane->getWindow();

        mxWindow->addWindowListener(this);
        mxWindow->addPaintListener(this);
        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->setBackground(util::Color(0xff000000));
        mxWindow->setVisible(sal_True);

        if (mpPresenterController.is())
        {
            ::rtl::Reference<PresenterWindowManager> pWindowManager(
                mpPresenterController->GetWindowManager());
            if (pWindowManager.is())
                pWindowManager->SetPanePosSizeRelative(rxViewId->getAnchor(),
                    0.1,0.2, 0.8,0.6);
        }

        Resize();
    }
    catch (RuntimeException&)
    {
        mxViewId = NULL;
        mxWindow = NULL;
        throw;
    }
}




PresenterHelpView::~PresenterHelpView (void)
{
}




void SAL_CALL PresenterHelpView::disposing (void)
{
    mxViewId = NULL;

    if (mxWindow.is())
    {
        mxWindow->removeWindowListener(this);
        mxWindow->removePaintListener(this);
    }
}




//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterHelpView::disposing (const lang::EventObject& rEventObject)
    throw (RuntimeException)
{
    if (rEventObject.Source == mxCanvas)
    {
        mxCanvas = NULL;
    }
    else if (rEventObject.Source == mxWindow)
    {
        mxWindow = NULL;
        dispose();
    }
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterHelpView::windowResized (const awt::WindowEvent& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    Resize();
}




void SAL_CALL PresenterHelpView::windowMoved (const awt::WindowEvent& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}




void SAL_CALL PresenterHelpView::windowShown (const lang::EventObject& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    Resize();
}




void SAL_CALL PresenterHelpView::windowHidden (const lang::EventObject& rEvent)
    throw (uno::RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterHelpView::windowPaint (const css::awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    Paint(rEvent.UpdateRect);
}




void PresenterHelpView::Paint (const awt::Rectangle& rUpdateBox)
{
    ProvideCanvas();

    Sequence<double> aBackgroundColor(4);
    aBackgroundColor[0] = 0.9;
    aBackgroundColor[1] = 0.6;
    aBackgroundColor[2] = 0.4;
    aBackgroundColor[3] = 0;

    rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rUpdateBox, mxCanvas->getDevice()));

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        aBackgroundColor,
        rendering::CompositeOperation::ADD);

    const awt::Rectangle aWindowBox (mxWindow->getPosSize());

    mxCanvas->fillPolyPolygon(
        PresenterGeometryHelper::CreatePolygon(
            awt::Rectangle(0,0,aWindowBox.Width,aWindowBox.Height),
            mxCanvas->getDevice()),
        aViewState,
        aRenderState);

    aRenderState.DeviceColor[0] = 0.4;
    aRenderState.DeviceColor[1] = 0.4;
    aRenderState.DeviceColor[2] = 0.4;
    aRenderState.DeviceColor[3] = 0.8;

    // Create a polygon for a rectangle with rounded corners.
    Sequence<Sequence<geometry::RealBezierSegment2D> > aPolygon(1);
    aPolygon[0] = Sequence<geometry::RealBezierSegment2D>(8);
    const double nLeft = 0.2 * aWindowBox.Width;
    const double nTop = 0.2 * aWindowBox.Height;
    const double nRight = 0.8 * aWindowBox.Width;
    const double nBottom = 0.8 * aWindowBox.Height;
    const double nRadius = ::std::min(30.0,::std::min(aWindowBox.Width/2.0,aWindowBox.Height/2.0));

#define SetCurve(aPolygon, x,y, dx1,dy1, dx2,dy2)                \
    aPolygon.Px=(x); aPolygon.Py=(y);                            \
    aPolygon.C1x=(x)+(dx1); aPolygon.C1y=(y)+(dy1);              \
    aPolygon.C2x=(x)+(dx1)+(dx2); aPolygon.C2y=(y)+(dy1)+(dy2);
#define SetLine(aPolygon, x1,y1, x2,y2)                                 \
    aPolygon.Px=(x1); aPolygon.Py=(y1);                                 \
    aPolygon.C1x=0.666*(x1)+0.334*(x2); aPolygon.C1y=0.666*(y1)+0.334*(y2); \
    aPolygon.C2x=0.333*(x1)+0.667*(x2); aPolygon.C2y=0.333*(y1)+0.667*(y2);

    SetCurve(aPolygon[0][0], nLeft + nRadius, nTop, -nRadius, 0, 0,0);
    SetLine(aPolygon[0][1], nLeft, nTop+nRadius, nLeft, nBottom-nRadius);

    SetCurve(aPolygon[0][2], nLeft, nBottom-nRadius, 0,nRadius, 0,0);
    SetLine(aPolygon[0][3], nLeft+nRadius, nBottom, nRight-nRadius, nBottom);

    SetCurve(aPolygon[0][4], nRight-nRadius, nBottom, nRadius,0, 0,0);
    SetLine(aPolygon[0][5], nRight, nBottom-nRadius, nRight, nTop+nRadius);

    SetCurve(aPolygon[0][6], nRight, nTop+nRadius, 0,-nRadius, 0,0);
    SetLine(aPolygon[0][7], nRight-nRadius, nTop, nLeft+nRadius, nTop);

    Reference<rendering::XPolyPolygon2D> xPolygon (
        mxCanvas->getDevice()->createCompatibleBezierPolyPolygon(aPolygon),
        UNO_QUERY_THROW);
    if (xPolygon.is())
    {
        xPolygon->setClosed(0, sal_True);
        mxCanvas->fillPolyPolygon(
            xPolygon,
            aViewState,
            aRenderState);
    }

    if ( ! mxFont.is())
        mxFont = mxCanvas->createFont(
            mpPresenterController->GetViewFontRequest(mxViewId->getResourceURL()),
            Sequence<beans::PropertyValue>(),
            geometry::Matrix2D(1,0,0,1));

    if (mxFont.is())
    {
        OUString sText (OUString::createFromAscii("Help View\nSecond Line of Text"));
        rendering::StringContext aContext (sText, 0, sText.getLength());

        Reference<rendering::XTextLayout> xLayout (
            mxFont->createTextLayout(aContext, rendering::TextDirection::WEAK_LEFT_TO_RIGHT, 0));

        geometry::RealRectangle2D aBox (xLayout->queryTextBounds());
        //        const double nTextWidth = aBox.X2 - aBox.X1;
        //        const double nTextHeight = aBox.Y2 - aBox.Y1;
        const double nX = nTop + 2*nRadius;
        const double nY = nLeft + 2*nRadius;

        rendering::RenderState aFontRenderState(
            geometry::AffineMatrix2D(1,0,nX, 0,1,nY),
            NULL,
            Sequence<double>(3),
            rendering::CompositeOperation::SOURCE);
        const util::Color aFontColor (
            mpPresenterController->GetViewFontColor(mxViewId->getResourceURL()));
        aRenderState.DeviceColor[0] = ((aFontColor & 0x00ff0000)>>16) / 255.0;
        aRenderState.DeviceColor[1] = ((aFontColor & 0x0000ff00)>>8) / 255.0;
        aRenderState.DeviceColor[2] = (aFontColor & 0x000000ff) / 255.0;

        mxCanvas->drawText(
            aContext,
            mxFont,
            aViewState,
            aFontRenderState,
            rendering::TextDirection::WEAK_LEFT_TO_RIGHT);
    }

    mxCanvas->updateScreen(sal_False);
}




//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterHelpView::getResourceId (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return mxViewId;
}




sal_Bool SAL_CALL PresenterHelpView::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}




//-----------------------------------------------------------------------------

void PresenterHelpView::ProvideCanvas (void)
{
    if ( ! mxCanvas.is() && mxPane.is())
    {
        mxCanvas = Reference<rendering::XSpriteCanvas>(mxPane->getCanvas(), UNO_QUERY);
        if ( ! mxCanvas.is())
            return;
        Reference<lang::XComponent> xComponent (mxCanvas, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(static_cast<awt::XPaintListener*>(this));
    }
}




void PresenterHelpView::Resize (void)
{
    // The whole window has to be repainted.
    Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY);
    if (xPeer.is())
        xPeer->invalidate(awt::InvalidateStyle::CHILDREN);
}




void PresenterHelpView::ThrowIfDisposed (void)
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterHelpView has been already disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}

} } // end of namespace ::sdext::presenter
