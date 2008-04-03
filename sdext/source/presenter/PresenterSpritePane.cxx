/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterSpritePane.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:05:44 $
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

#include "PresenterSpritePane.hxx"
#include "PresenterGeometryHelper.hxx"
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//----- Service ---------------------------------------------------------------

OUString PresenterSpritePane::getImplementationName_static (void)
{
    return OUString::createFromAscii("com.sun.star.comp.Draw.PresenterSpritePane");
}




Sequence<OUString> PresenterSpritePane::getSupportedServiceNames_static (void)
{
    static const ::rtl::OUString sServiceName(
        ::rtl::OUString::createFromAscii("com.sun.star.drawing.PresenterSpritePane"));
    return Sequence<rtl::OUString>(&sServiceName, 1);
}




Reference<XInterface> PresenterSpritePane::Create (const Reference<uno::XComponentContext>& rxContext)
    SAL_THROW((css::uno::Exception))
{
    return Reference<XInterface>(static_cast<XWeak*>(new PresenterSpritePane(rxContext)));
}




//===== TransparentBorderPainter ==============================================

namespace {
    typedef ::cppu::WeakComponentImplHelper1<
        css::drawing::framework::XPaneBorderPainter
    > TransparentBorderPainterInterfaceBase;
}

class TransparentBorderPainter
    : private ::boost::noncopyable,
      protected ::cppu::BaseMutex,
      public TransparentBorderPainterInterfaceBase
{
public:
    TransparentBorderPainter (void);
    virtual ~TransparentBorderPainter (void);

    // XPaneBorderPainter

    virtual css::awt::Rectangle SAL_CALL addBorder (
        const rtl::OUString& rsPaneBorderStyleName,
        const css::awt::Rectangle& rRectangle,
        css::drawing::framework::BorderType eBorderType)
        throw(css::uno::RuntimeException);

    virtual css::awt::Rectangle SAL_CALL removeBorder (
        const rtl::OUString& rsPaneBorderStyleName,
        const css::awt::Rectangle& rRectangle,
        css::drawing::framework::BorderType eBorderType)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL paint (
        const rtl::OUString& rsPaneBorderStyleName,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
        const css::awt::Rectangle& rOuterBorderRectangle,
        const css::awt::Rectangle& rRepaintArea,
        const rtl::OUString& rsTitle)
        throw(css::uno::RuntimeException);

private:
    double mnLeft;
    double mnTop;
    double mnRight;
    double mnBottom;

    void ThrowIfDisposed (void) const
        throw (::com::sun::star::lang::DisposedException);
};




//===== PresenterSpritePane =========================================================

PresenterSpritePane::PresenterSpritePane (const Reference<XComponentContext>& rxContext)
    : PresenterPaneBase(rxContext),
      mxParentWindow(),
      mxParentCanvas(),
      mpSprite(new PresenterSprite())
{
    Reference<lang::XMultiComponentFactory> xFactory (
        mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
    mxPresenterHelper = Reference<drawing::XPresenterHelper>(
        xFactory->createInstanceWithContext(
            OUString::createFromAscii("com.sun.star.comp.Draw.PresenterHelper"),
            mxComponentContext),
        UNO_QUERY_THROW);
}




PresenterSpritePane::~PresenterSpritePane (void)
{
}




void PresenterSpritePane::disposing (void)
{
    mpSprite->SetFactory(NULL);
    mxParentWindow = NULL;
    mxParentCanvas = NULL;
    PresenterPaneBase::disposing();
}




//----- XPane -----------------------------------------------------------------

Reference<awt::XWindow> SAL_CALL PresenterSpritePane::getWindow (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return mxContentWindow;
}




Reference<rendering::XCanvas> SAL_CALL PresenterSpritePane::getCanvas (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    if ( ! mxContentCanvas.is())
        UpdateCanvases();

    return mxContentCanvas;
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterSpritePane::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    mpSprite->Resize(geometry::RealSize2D(rEvent.Width, rEvent.Height));
    LayoutContextWindow();
    UpdateCanvases();
}





void SAL_CALL PresenterSpritePane::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();
    awt::Rectangle aBox (
        mxPresenterHelper->getWindowExtentsRelative(mxBorderWindow, mxParentWindow));
    mpSprite->MoveTo(geometry::RealPoint2D(aBox.X, aBox.Y));
    mpSprite->Update();
}




void SAL_CALL PresenterSpritePane::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();

    mpSprite->Show();
    ToTop();

    if (mxContentWindow.is())
    {
        LayoutContextWindow();
        mxContentWindow->setVisible(sal_True);
    }
}




void SAL_CALL PresenterSpritePane::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();

    mpSprite->Hide();
    if (mxContentWindow.is())
        mxContentWindow->setVisible(sal_False);
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterSpritePane::windowPaint (const awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ThrowIfDisposed();

    /*
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxParentCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
    */
}




//-----------------------------------------------------------------------------


::boost::shared_ptr<PresenterSprite> PresenterSpritePane::GetSprite (void)
{
    return mpSprite;
}




void PresenterSpritePane::ShowTransparentBorder (void)
{
    //    mxBorderPainter = new TransparentBorderPainter();
}




void PresenterSpritePane::UpdateCanvases (void)
{
    Reference<XComponent> xContentCanvasComponent (mxContentCanvas, UNO_QUERY);
    if (xContentCanvasComponent.is())
    {
        if (xContentCanvasComponent.is())
            xContentCanvasComponent->dispose();
    }

    // The border canvas is the content canvas of the sprite.
    mxBorderCanvas = mpSprite->GetCanvas();

    // The content canvas is a wrapper of the border canvas.
    if (mxBorderCanvas.is())
        mxContentCanvas = mxPresenterHelper->createSharedCanvas(
            mxParentCanvas,
            mxParentWindow,
            mxBorderCanvas,
            mxBorderWindow,
            mxContentWindow);

    const awt::Rectangle aWindowBox (mxBorderWindow->getPosSize());
    PaintBorder(awt::Rectangle(0,0,aWindowBox.Width,aWindowBox.Height));
}




void PresenterSpritePane::CreateCanvases (
    const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
    const css::uno::Reference<css::rendering::XSpriteCanvas>& rxParentCanvas)
{
    OSL_ASSERT(!mxParentWindow.is() || mxParentWindow==rxParentWindow);
    OSL_ASSERT(!mxParentCanvas.is() || mxParentCanvas==rxParentCanvas);
    mxParentWindow = rxParentWindow;
    mxParentCanvas = rxParentCanvas;

    mpSprite->SetFactory(mxParentCanvas);
    if (mxBorderWindow.is())
    {
        const awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
        mpSprite->Resize(geometry::RealSize2D(aBorderBox.Width, aBorderBox.Height));
    }

    UpdateCanvases();
}




//===== TransparentBorderPainter ==============================================

TransparentBorderPainter::TransparentBorderPainter (void)
    : TransparentBorderPainterInterfaceBase(m_aMutex),
      mnLeft(50),
      mnTop(50),
      mnRight(50),
      mnBottom(50)
{
}




TransparentBorderPainter::~TransparentBorderPainter (void)
{
}




// XPaneBorderPainter

css::awt::Rectangle SAL_CALL TransparentBorderPainter::addBorder (
    const rtl::OUString& rsPaneBorderStyleName,
    const css::awt::Rectangle& rRectangle,
    css::drawing::framework::BorderType eBorderType)
    throw(css::uno::RuntimeException)
{
    (void)rsPaneBorderStyleName;

    switch (eBorderType)
    {
        case drawing::framework::BorderType_INNER_BORDER:
        default:
            return rRectangle;

        case drawing::framework::BorderType_OUTER_BORDER:
        case drawing::framework::BorderType_TOTAL_BORDER:
            return awt::Rectangle(
                sal::static_int_cast<sal_Int32>(rRectangle.X - mnLeft),
                sal::static_int_cast<sal_Int32>(rRectangle.Y - mnTop),
                sal::static_int_cast<sal_Int32>(rRectangle.Width + (mnLeft + mnRight)),
                sal::static_int_cast<sal_Int32>(rRectangle.Height + (mnTop + mnBottom)));
    }
}




css::awt::Rectangle SAL_CALL TransparentBorderPainter::removeBorder (
    const rtl::OUString& rsPaneBorderStyleName,
    const css::awt::Rectangle& rRectangle,
    css::drawing::framework::BorderType eBorderType)
    throw(css::uno::RuntimeException)
{
    (void)rsPaneBorderStyleName;

    switch (eBorderType)
    {
        case drawing::framework::BorderType_INNER_BORDER:
        default:
            return rRectangle;

        case drawing::framework::BorderType_OUTER_BORDER:
        case drawing::framework::BorderType_TOTAL_BORDER:
            return awt::Rectangle(
                sal::static_int_cast<sal_Int32>(rRectangle.X + mnLeft),
                sal::static_int_cast<sal_Int32>(rRectangle.Y + mnTop),
                sal::static_int_cast<sal_Int32>(rRectangle.Width - (mnLeft + mnRight)),
                sal::static_int_cast<sal_Int32>(rRectangle.Height - (mnTop + mnBottom)));
    }
}




void SAL_CALL TransparentBorderPainter::paint (
    const rtl::OUString& rsPaneBorderStyleName,
    const css::uno::Reference<css::rendering::XCanvas>& rxCanvas,
    const css::awt::Rectangle& rOuterBorderRectangle,
    const css::awt::Rectangle& rRepaintArea,
    const rtl::OUString& rsTitle)
    throw(css::uno::RuntimeException)
{
    (void)rsPaneBorderStyleName;
    (void)rRepaintArea;
    (void)rsTitle;

    rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL);

    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(4),
        rendering::CompositeOperation::SOURCE);

    aRenderState.DeviceColor[0] = 0.5;
    aRenderState.DeviceColor[1] = 0.5;
    aRenderState.DeviceColor[2] = 0.5;
    aRenderState.DeviceColor[3] = 0.5;

    Reference<rendering::XPolyPolygon2D> xPolygon (
        PresenterGeometryHelper::CreatePolygon(rOuterBorderRectangle, rxCanvas->getDevice()));
    if (xPolygon.is())
        rxCanvas->fillPolyPolygon(
            xPolygon,
            aViewState,
            aRenderState);
}




void TransparentBorderPainter::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "TransparentBorderPainter object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}



} } // end of namespace ::sd::presenter
