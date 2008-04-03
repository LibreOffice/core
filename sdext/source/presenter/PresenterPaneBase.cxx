/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterPaneBase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:00:56 $
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

#include "PresenterPaneBase.hxx"
#include "PresenterGeometryHelper.hxx"
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/drawing/CanvasFeature.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

namespace sdext { namespace presenter {

//===== PresenterPaneBase =====================================================

PresenterPaneBase::PresenterPaneBase (const Reference<XComponentContext>& rxContext)
    : PresenterPaneBaseInterfaceBase(m_aMutex),
      mxBorderWindow(),
      mxBorderCanvas(),
      mxContentWindow(),
      mxContentCanvas(),
      mxPaneId(),
      mxBorderPainter(),
      mxPresenterHelper(),
      msTitle(),
      mxComponentContext(rxContext),
      maViewBackgroundColor(0x00ffffff),
      mxViewBackgroundBitmap()
{
    Reference<lang::XMultiComponentFactory> xFactory (
        mxComponentContext->getServiceManager(), UNO_QUERY_THROW);
    mxPresenterHelper = Reference<drawing::XPresenterHelper>(
        xFactory->createInstanceWithContext(
            OUString::createFromAscii("com.sun.star.comp.Draw.PresenterHelper"),
            mxComponentContext),
        UNO_QUERY_THROW);
}




PresenterPaneBase::~PresenterPaneBase (void)
{
}




void PresenterPaneBase::disposing (void)
{
    {
        Reference<lang::XComponent> xComponent (mxPresenterHelper, UNO_QUERY);
        mxPresenterHelper = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    if (mxBorderWindow.is())
    {
        mxBorderWindow->removeWindowListener(this);
        mxBorderWindow->removePaintListener(this);
    }

    {
        Reference<XComponent> xComponent (mxContentCanvas, UNO_QUERY);
        mxContentCanvas = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<XComponent> xComponent (mxContentWindow, UNO_QUERY);
        mxContentWindow = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<XComponent> xComponent (mxBorderCanvas, UNO_QUERY);
        mxBorderCanvas = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    {
        Reference<XComponent> xComponent (mxBorderWindow, UNO_QUERY);
        mxBorderWindow = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    mxComponentContext = NULL;
}




void PresenterPaneBase::SetTitle (const OUString& rsTitle)
{
    msTitle = rsTitle;
}




//----- XInitialization -------------------------------------------------------

void SAL_CALL PresenterPaneBase::initialize (const Sequence<Any>& rArguments)
    throw (Exception, RuntimeException)
{
    ThrowIfDisposed();

    if ( ! mxComponentContext.is())
    {
        throw RuntimeException(
            OUString::createFromAscii("PresenterSpritePane: missing component context"),
            static_cast<XWeak*>(this));
    }

    if (rArguments.getLength() == 5 || rArguments.getLength() == 6)
    {
        try
        {
            // Get the resource id from the first argument.
            if ( ! (rArguments[0] >>= mxPaneId))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterPane: invalid pane id"),
                    static_cast<XWeak*>(this),
                    0);
            }

            Reference<awt::XWindow> xParentWindow;
            if ( ! (rArguments[1] >>= xParentWindow))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterPane: invalid parent window"),
                    static_cast<XWeak*>(this),
                    1);
            }

            Reference<rendering::XSpriteCanvas> xParentCanvas;
            if ( ! (rArguments[2] >>= xParentCanvas))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterPane: invalid parent canvas"),
                    static_cast<XWeak*>(this),
                    2);
            }

            if ( ! (rArguments[3] >>= msTitle))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterPane: invalid title"),
                    static_cast<XWeak*>(this),
                    3);
            }

            if ( ! (rArguments[4] >>= mxBorderPainter))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterPane: invalid border painter"),
                    static_cast<XWeak*>(this),
                    4);
            }

            bool bIsWindowVisibleOnCreation (true);
            if (rArguments.getLength()>5 && ! (rArguments[5] >>= bIsWindowVisibleOnCreation))
            {
                throw lang::IllegalArgumentException(
                    OUString::createFromAscii("PresenterPane: invalid window visibility flag"),
                    static_cast<XWeak*>(this),
                    5);
            }

            CreateWindows(xParentWindow, bIsWindowVisibleOnCreation);

            if (mxBorderWindow.is())
            {
                mxBorderWindow->addWindowListener(this);
                mxBorderWindow->addPaintListener(this);
            }

            CreateCanvases(xParentWindow, xParentCanvas);

            // Raise new windows.
            ToTop();
        }
        catch (Exception&)
        {
            mxContentWindow = NULL;
            mxComponentContext = NULL;
            throw;
        }
    }
    else
    {
        throw RuntimeException(
            OUString::createFromAscii("PresenterSpritePane: invalid number of arguments"),
                static_cast<XWeak*>(this));
    }
}




//----- XResourceId -----------------------------------------------------------

Reference<XResourceId> SAL_CALL PresenterPaneBase::getResourceId (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    return mxPaneId;
}




sal_Bool SAL_CALL PresenterPaneBase::isAnchorOnly (void)
    throw (RuntimeException)
{
    return true;
}




//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterPaneBase::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxBorderWindow)
    {
        mxBorderWindow = NULL;
    }
}




//-----------------------------------------------------------------------------


void PresenterPaneBase::CreateWindows (
    const Reference<awt::XWindow>& rxParentWindow,
    const bool bIsWindowVisibleOnCreation)
{
    if (mxPresenterHelper.is() && rxParentWindow.is())
    {

        mxBorderWindow = mxPresenterHelper->createWindow(
            rxParentWindow,
            sal_False,
            bIsWindowVisibleOnCreation,
            sal_False,
            sal_False);
        mxContentWindow = mxPresenterHelper->createWindow(
            mxBorderWindow,
            sal_False,
            bIsWindowVisibleOnCreation,
            sal_False,
            sal_False);
    }
}




Reference<awt::XWindow> PresenterPaneBase::GetBorderWindow (void) const
{
    return mxBorderWindow;
}




void PresenterPaneBase::ToTop (void)
{
    if (mxPresenterHelper.is())
        mxPresenterHelper->toTop(mxContentWindow);
}




void PresenterPaneBase::SetBackground (
    const css::util::Color aViewBackgroundColor,
    const css::uno::Reference<css::rendering::XBitmap>& rxViewBackgroundBitmap)
{
    maViewBackgroundColor = aViewBackgroundColor;
    mxViewBackgroundBitmap = rxViewBackgroundBitmap;
}




void PresenterPaneBase::PaintBorderBackground (
    const awt::Rectangle& rBorderBox,
    const awt::Rectangle& rUpdateBox)
{
    if ( ! mxBorderCanvas.is())
        return;

    rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rUpdateBox, mxBorderCanvas->getDevice()));

    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        NULL,
        Sequence<double>(3),
        rendering::CompositeOperation::SOURCE);
    aRenderState.DeviceColor[0] = ((maViewBackgroundColor >> 16) & 0x0ff) / 255.0;
    aRenderState.DeviceColor[1] = ((maViewBackgroundColor >> 8) & 0x0ff) / 255.0;
    aRenderState.DeviceColor[2] = ((maViewBackgroundColor >> 0) & 0x0ff) / 255.0;

    // The outer box of the border is given.  We need the center and inner
    // box as well.
    awt::Rectangle aCenterBox (
        mxBorderPainter->removeBorder(
            mxPaneId->getResourceURL(),
            rBorderBox,
            drawing::framework::BorderType_OUTER_BORDER));
    awt::Rectangle aInnerBox (
        mxBorderPainter->removeBorder(
            mxPaneId->getResourceURL(),
            rBorderBox,
            drawing::framework::BorderType_TOTAL_BORDER));

    // Create a clip polypolygon that has the inner box as hole.
    ::std::vector<awt::Rectangle> aRectangles;
    aRectangles.reserve(2);
    aRectangles.push_back(aCenterBox);
    aRectangles.push_back(aInnerBox);
    Reference<rendering::XPolyPolygon2D> xPolyPolygon (
        PresenterGeometryHelper::CreatePolygon(
            aRectangles,
            mxBorderCanvas->getDevice()));
    if (xPolyPolygon.is())
    {
        xPolyPolygon->setFillRule(rendering::FillRule_EVEN_ODD);
        aRenderState.Clip = xPolyPolygon;
    }

    Reference<rendering::XPolyPolygon2D> xPolygon(PresenterGeometryHelper::CreatePolygon(
        aCenterBox,
        mxBorderCanvas->getDevice()));
    mxBorderCanvas->fillPolyPolygon(
        xPolygon,
        aViewState,
        aRenderState);
}




void PresenterPaneBase::PaintBorder (const awt::Rectangle& rUpdateBox)
{
    OSL_ASSERT(mxPaneId.is());

    if (mxBorderPainter.is() && mxBorderWindow.is() && mxBorderCanvas.is())
    {
        awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
        awt::Rectangle aLocalBorderBox (0,0, aBorderBox.Width, aBorderBox.Height);

        PaintBorderBackground(aLocalBorderBox, rUpdateBox);

        mxBorderPainter->paintBorder(
            mxPaneId->getResourceURL(),
            mxBorderCanvas,
            aLocalBorderBox,
            rUpdateBox,
            msTitle);
    }
}




void PresenterPaneBase::LayoutContextWindow (void)
{
    OSL_ASSERT(mxPaneId.is());
    OSL_ASSERT(mxBorderWindow.is());
    OSL_ASSERT(mxContentWindow.is());
    if (mxBorderPainter.is() && mxPaneId.is() && mxBorderWindow.is() && mxContentWindow.is())
    {
        const awt::Rectangle aBorderBox (mxBorderWindow->getPosSize());
        const awt::Rectangle aInnerBox (mxBorderPainter->removeBorder(
            mxPaneId->getResourceURL(),
            aBorderBox,
            drawing::framework::BorderType_TOTAL_BORDER));
        mxContentWindow->setPosSize(
            aInnerBox.X - aBorderBox.X,
            aInnerBox.Y - aBorderBox.Y,
            aInnerBox.Width,
            aInnerBox.Height,
            awt::PosSize::POSSIZE);
    }
}




void PresenterPaneBase::ThrowIfDisposed (void)
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterPane object has already been disposed")),
            static_cast<uno::XWeak*>(this));
    }
}




} } // end of namespace ::sd::presenter
