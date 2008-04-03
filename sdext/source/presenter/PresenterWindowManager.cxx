/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterWindowManager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:07:10 $
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

#include "PresenterWindowManager.hxx"
#include "PresenterAnimation.hxx"
#include "PresenterAnimator.hxx"
#include "PresenterController.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterHelper.hxx"
#include "PresenterPaneBase.hxx"
#include "PresenterPaneBorderPainter.hxx"
#include "PresenterPaneContainer.hxx"
#include "PresenterPaneFactory.hxx"
#include "PresenterTheme.hxx"
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/framework/ResourceId.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/FillRule.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/Texture.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;
using ::rtl::OUString;

#undef VERBOSE
//#define VERBOSE

namespace sdext { namespace presenter {

PresenterWindowManager::PresenterWindowManager (
    const Reference<XComponentContext>& rxContext,
    const ::rtl::Reference<PresenterPaneContainer>& rpPaneContainer,
    const ::rtl::Reference<PresenterController>& rpPresenterController)
    : PresenterWindowManagerInterfaceBase(m_aMutex),
      mxComponentContext(rxContext),
      mpPresenterController(rpPresenterController),
      mxParentWindow(),
      mxParentCanvas(),
      mxPaneBorderManager(),
      mpPaneBorderPainter(),
      mpPaneContainer(rpPaneContainer),
      mbIsLayoutPending(true),
      mbIsLayouting(false),
      mpTheme(),
      mxBackgroundBitmap(),
      maBackgroundColor(),
      maPaneBackgroundColor(),
      mxClipPolygon()
{
    UpdateWindowList();
}




PresenterWindowManager::~PresenterWindowManager (void)
{
}




void SAL_CALL PresenterWindowManager::disposing (void)
{
    SetParentPane(NULL);

    Reference<lang::XComponent> xComponent (mxPaneBorderManager, UNO_QUERY);
    if (xComponent.is())
        xComponent->dispose();
    mxPaneBorderManager = NULL;

    PresenterPaneContainer::PaneList::const_iterator iPane;
    PresenterPaneContainer::PaneList::const_iterator iEnd (mpPaneContainer->maPanes.end());
    for (iPane=mpPaneContainer->maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        if ((*iPane)->mxBorderWindow.is())
        {
            (*iPane)->mxBorderWindow->removeWindowListener(this);
            (*iPane)->mxBorderWindow->removeFocusListener(this);
        }
    }
}




void PresenterWindowManager::SetParentPane (
    const Reference<drawing::framework::XPane>& rxPane)
{
    if (mxParentWindow.is())
    {
        mxParentWindow->removeWindowListener(this);
        mxParentWindow->removePaintListener(this);
        mxParentWindow->removeMouseListener(this);
        mxParentWindow->removeFocusListener(this);
    }
    mxParentWindow = NULL;
    mxParentCanvas = NULL;

    if (rxPane.is())
    {
        mxParentWindow = rxPane->getWindow();
        mxParentCanvas = rxPane->getCanvas();
    }
    else
    {
        mxParentWindow = NULL;
    }

    if (mxParentWindow.is())
    {
        mxParentWindow->addWindowListener(this);
        mxParentWindow->addPaintListener(this);
        mxParentWindow->addMouseListener(this);
        mxParentWindow->addFocusListener(this);

        // We paint our own background, make that of the parent window transparent.
        Reference<awt::XWindowPeer> xPeer (mxParentWindow, UNO_QUERY);
        if (xPeer.is())
            xPeer->setBackground(util::Color(0xff000000));
    }
}




void PresenterWindowManager::SetTheme (const ::boost::shared_ptr<PresenterTheme>& rpTheme)
{
    mpTheme = rpTheme;

    // Get background bitmap or background color from the theme.
    mxBackgroundBitmap = NULL;
    maBackgroundColor = util::Color(0x00ffffff);

    try
    {
        if (mpTheme.get() != NULL)
            mxBackgroundBitmap = Reference<rendering::XBitmap>(
                mpTheme->getPropertyValue(OUString::createFromAscii("Background_Bitmap")),
                UNO_QUERY);
    }
    catch (beans::UnknownPropertyException&)
    {
        OSL_TRACE("no Background_Bitmap property found in PresenterWindowManager::PaintBackground");
    }

    if ( ! mxBackgroundBitmap.is())
    {
        try
        {
            if (mpTheme.get() != NULL)
                mpTheme->getPropertyValue(OUString::createFromAscii("Background_Color"))
                    >>= maBackgroundColor;
        }
        catch (beans::UnknownPropertyException&)
        {
            OSL_TRACE("no Background_Color property found in PresenterWindowManager::PaintBackground");
        }
    }
}




void PresenterWindowManager::NotifyPaneCreation (
    const PresenterPaneContainer::SharedPaneDescriptor& rpDescriptor)
{
    if (rpDescriptor.get()==NULL)
    {
        OSL_ASSERT(rpDescriptor.get()!=NULL);
        return;
    }
    if ( ! rpDescriptor->mxContentWindow.is())
    {
        OSL_ASSERT(rpDescriptor->mxContentWindow.is());
        return;
    }

    mbIsLayoutPending = true;

    Reference<awt::XWindow> xBorderWindow (rpDescriptor->mxBorderWindow);
    OSL_ASSERT(xBorderWindow.is());
    if (xBorderWindow.is() && ! rpDescriptor->mbIsSprite)
    {
        Invalidate();

        xBorderWindow->addWindowListener(this);
        xBorderWindow->addFocusListener(this);
    }

    UpdateWindowList();
    Layout();
}




void PresenterWindowManager::NotifyViewCreation (const Reference<XView>& rxView)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneId(rxView->getResourceId()->getAnchor()));
    OSL_ASSERT(pDescriptor.get() != NULL);
    if (pDescriptor.get() != NULL)
    {
        Reference<awt::XWindowPeer> xPeer (pDescriptor->mxContentWindow, UNO_QUERY);
        if (xPeer.is())
        {
            xPeer->invalidate(awt::InvalidateStyle::NOERASE | awt::InvalidateStyle::UPDATE);
        }
    }
}




void PresenterWindowManager::SetPanePosSizeRelative (
    const Reference<XResourceId>& rxPaneId,
    const double nRelativeX,
    const double nRelativeY,
    const double nRelativeWidth,
    const double nRelativeHeight)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneId(rxPaneId));
    if (pDescriptor.get() != NULL)
    {
        pDescriptor->mnLeft = nRelativeX;
        pDescriptor->mnTop = nRelativeY;
        pDescriptor->mnRight = nRelativeX + nRelativeWidth;
        pDescriptor->mnBottom = nRelativeY + nRelativeHeight;

        mpPaneContainer->ToTop(pDescriptor);
    }
}




void PresenterWindowManager::SetPanePosSizeAbsolute (
    const Reference<XResourceId>& rxPaneId,
    const double nX,
    const double nY,
    const double nWidth,
    const double nHeight)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindPaneId(rxPaneId));
    if (pDescriptor.get() != NULL)
    {
        awt::Rectangle aParentBox = mxParentWindow->getPosSize();
        if (aParentBox.Width > 0 && aParentBox.Height > 0)
        {
            pDescriptor->mnLeft = nX / aParentBox.Width;
            pDescriptor->mnTop = nY / aParentBox.Height;
            pDescriptor->mnRight = (nX + nWidth) / aParentBox.Width;
            pDescriptor->mnBottom = (nY + nHeight) / aParentBox.Height;
        }
        if (pDescriptor->mxBorderWindow.is())
            pDescriptor->mxBorderWindow->setPosSize(
                ::sal::static_int_cast<sal_Int32>(nX),
                ::sal::static_int_cast<sal_Int32>(nY),
                ::sal::static_int_cast<sal_Int32>(nWidth),
                ::sal::static_int_cast<sal_Int32>(nHeight),
                awt::PosSize::POSSIZE);
    }
}




void PresenterWindowManager::SetPaneBorderPainter (
    const ::rtl::Reference<PresenterPaneBorderPainter>& rPainter)
{
    mpPaneBorderPainter = rPainter;
}




//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterWindowManager::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (rEvent.Source == mxParentWindow)
    {
        Layout();
    }
    else
    {
        Reference<awt::XWindow> xWindow (rEvent.Source,UNO_QUERY);
        UpdateWindowSize(xWindow);
    }
}




void SAL_CALL PresenterWindowManager::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    if (rEvent.Source != mxParentWindow)
    {
        Reference<awt::XWindow> xWindow (rEvent.Source,UNO_QUERY);
        UpdateWindowSize(xWindow);
    }
}




void SAL_CALL PresenterWindowManager::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterWindowManager::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




//----- XPaintListener --------------------------------------------------------

void SAL_CALL PresenterWindowManager::windowPaint (const awt::PaintEvent& rEvent)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    if ( ! mxParentWindow.is())
        return;
    if ( ! mxParentCanvas.is())
        return;

    if (mpTheme.get()!=NULL)
    {
        try
        {
            if (mbIsLayoutPending)
                Layout();
            PaintBackground(rEvent.UpdateRect);
            PaintChildren(rEvent);
        }
        catch (RuntimeException&)
        {
            OSL_ASSERT(FALSE);
        }
    }
}




//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterWindowManager::mousePressed (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterWindowManager::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterWindowManager::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterWindowManager::mouseExited (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
}




//----- XFocusListener --------------------------------------------------------

void SAL_CALL PresenterWindowManager::focusGained (const css::awt::FocusEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    (void)rEvent;
}




void SAL_CALL PresenterWindowManager::focusLost (const css::awt::FocusEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();
    (void)rEvent;
}




//----- XEventListener --------------------------------------------------------

void SAL_CALL PresenterWindowManager::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxParentWindow)
        mxParentWindow = NULL;
    else
    {
        Reference<awt::XWindow> xWindow (rEvent.Source, UNO_QUERY);
    }
}




//-----------------------------------------------------------------------------

void PresenterWindowManager::PaintChildren (const awt::PaintEvent& rEvent) const
{
    // Call windowPaint on all children that lie in or touch the
    // update rectangle.
    PresenterPaneContainer::PaneList::const_iterator iPane;
    PresenterPaneContainer::PaneList::const_iterator iEnd (mpPaneContainer->maPanes.end());
    for (iPane=mpPaneContainer->maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        try
        {
            // Make sure that the pane shall and can be painted.
            if ( ! (*iPane)->mbIsActive)
                continue;
            if ((*iPane)->mbIsSprite)
                continue;
            if ( ! (*iPane)->mxPane.is())
                continue;
            if ( ! (*iPane)->mxBorderWindow.is())
                continue;
            Reference<awt::XWindow> xBorderWindow ((*iPane)->mxBorderWindow);
            if ( ! xBorderWindow.is())
                continue;

            // Get the area in which the border of the pane has to be painted.
            const awt::Rectangle aBorderBox (xBorderWindow->getPosSize());
            const awt::Rectangle aBorderUpdateBox(
                PresenterGeometryHelper::Intersection(
                    rEvent.UpdateRect,
                    aBorderBox));
            if (aBorderUpdateBox.Width<=0 || aBorderUpdateBox.Height<=0)
                continue;

            const awt::Rectangle aLocalBorderUpdateBox(
                PresenterGeometryHelper::TranslateRectangle(
                    aBorderUpdateBox,
                    -aBorderBox.X,
                    -aBorderBox.Y));

            // Invalidate the area of the content window.
            Reference<awt::XWindowPeer> xPeer (xBorderWindow, UNO_QUERY);
            if (xPeer.is())
            {
                xPeer->invalidateRect(
                    aLocalBorderUpdateBox,
                    awt::InvalidateStyle::CHILDREN | awt::InvalidateStyle::NOTRANSPARENT);
            }
        }
        catch (RuntimeException&)
        {
            OSL_ASSERT(FALSE);
        }
    }
}




void PresenterWindowManager::Layout (void)
{
    if (mxParentWindow.is() && ! mbIsLayouting)
    {
        mbIsLayoutPending = false;
        mbIsLayouting = true;

        try
        {
            awt::Rectangle aBox = mxParentWindow->getPosSize();

            PresenterPaneContainer::PaneList::const_iterator iPane;
            PresenterPaneContainer::PaneList::const_iterator iEnd (mpPaneContainer->maPanes.end());
            for (iPane=mpPaneContainer->maPanes.begin(); iPane!=iEnd; ++iPane)
            {
                const PresenterPaneContainer::SharedPaneDescriptor& pDescriptor (*iPane);
                if ( ! pDescriptor->mxBorderWindow.is())
                    continue;

                // Layout the border window.
                const sal_Int32 nX = (sal_Int32)(pDescriptor->mnLeft * aBox.Width);
                const sal_Int32 nY = (sal_Int32)(pDescriptor->mnTop * aBox.Height);
                const sal_Int32 nWidth = (sal_Int32)(pDescriptor->mnRight * aBox.Width) - nX;
                const sal_Int32 nHeight = (sal_Int32)(pDescriptor->mnBottom * aBox.Height) - nY;

                pDescriptor->mxBorderWindow->setPosSize(
                    nX,nY,nWidth,nHeight,
                    awt::PosSize::POSSIZE);

            }
        }
        catch (Exception&)
        {
            mbIsLayouting = false;
            throw;
        }

        mbIsLayouting = false;
    }
}




void PresenterWindowManager::UpdateWindowSize (const Reference<awt::XWindow>& rxBorderWindow)
{
    PresenterPaneContainer::SharedPaneDescriptor pDescriptor (
        mpPaneContainer->FindBorderWindow(rxBorderWindow));
    if (pDescriptor.get() != NULL)
    {
        mxClipPolygon = NULL;

        awt::Rectangle aParentBox = mxParentWindow->getPosSize();
        awt::Rectangle aBorderBox (pDescriptor->mxBorderWindow->getPosSize());

        if ( ! mbIsLayouting)
        {
            const double nWidth (aParentBox.Width);
            const double nHeight (aParentBox.Height);
            pDescriptor->mnLeft = double(aBorderBox.X) / nWidth;
            pDescriptor->mnTop = double(aBorderBox.Y) / nHeight;
            pDescriptor->mnRight = double(aBorderBox.X + aBorderBox.Width) / nWidth;
            pDescriptor->mnBottom = double(aBorderBox.Y + aBorderBox.Height) / nHeight;
        }
        else
        {
            // This update of the window size was initiated by
            // Layout(). Therefore the window size does not have to be
            // updated.
        }

        // ToTop is called last because it may invalidate the iterator.
        if ( ! mbIsLayouting)
            mpPaneContainer->ToTop(pDescriptor);
    }
}




void PresenterWindowManager::PaintBackground (const awt::Rectangle& rUpdateBox)
{
    (void)rUpdateBox;
    if ( ! mxParentWindow.is())
        return;

    Reference<rendering::XGraphicDevice> xDevice (mxParentCanvas->getDevice());
    if ( ! xDevice.is())
        return;

    // Create a polygon for the background and for clipping.
    Reference<rendering::XPolyPolygon2D> xBackgroundPolygon (
        PresenterGeometryHelper::CreatePolygon(mxParentWindow->getPosSize(), xDevice));
    if ( ! mxClipPolygon.is())
        mxClipPolygon = CreateClipPolyPolygon();

    // Create View- and RenderState structs.
    const rendering::ViewState aViewState(
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        PresenterGeometryHelper::CreatePolygon(rUpdateBox, xDevice));
    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(1,0,0, 0,1,0),
        mxClipPolygon,
        Sequence<double>(3),
        rendering::CompositeOperation::SOURCE);

    // Paint the background.
    if (mxBackgroundBitmap.is())
    {
        Sequence<rendering::Texture> aTextures (1);
        geometry::IntegerSize2D aBitmapSize (mxBackgroundBitmap->getSize());
        aTextures[0] = rendering::Texture (
            geometry::AffineMatrix2D(
                aBitmapSize.Width,0,0,
                0,aBitmapSize.Height,0),
            1,
            0,
            mxBackgroundBitmap,
            NULL,
            NULL,
            rendering::StrokeAttributes(),
            rendering::TexturingMode::REPEAT,
            rendering::TexturingMode::REPEAT);

        mxParentCanvas->fillTexturedPolyPolygon(
            xBackgroundPolygon,
            aViewState,
            aRenderState,
            aTextures);
    }
    else
    {
        aRenderState.DeviceColor[0] = ((maBackgroundColor >> 16) & 0x0ff) / 255.0;
        aRenderState.DeviceColor[1] = ((maBackgroundColor >> 8) & 0x0ff) / 255.0;
        aRenderState.DeviceColor[2] = ((maBackgroundColor >> 0) & 0x0ff) / 255.0;
        mxParentCanvas->fillPolyPolygon(
            xBackgroundPolygon,
            aViewState,
            aRenderState);
    }
}




Reference<rendering::XPolyPolygon2D> PresenterWindowManager::CreateClipPolyPolygon (void) const
{
    // Create a clip polygon that includes the whole update area but has the
    // content windows as holes.
    const sal_Int32 nPaneCount (mpPaneContainer->maPanes.size());
    ::std::vector<awt::Rectangle> aRectangles;
    aRectangles.reserve(1+nPaneCount);
    aRectangles.push_back(mxParentWindow->getPosSize());
    PresenterPaneContainer::PaneList::const_iterator iPane;
    PresenterPaneContainer::PaneList::const_iterator iEnd (mpPaneContainer->maPanes.end());
    for (iPane=mpPaneContainer->maPanes.begin(); iPane!=iEnd; ++iPane)
    {
        PresenterPaneContainer::SharedPaneDescriptor pDescriptor (*iPane);
        if ( ! pDescriptor->mbIsActive)
            continue;
        if ( ! pDescriptor->mbNeedsClipping)
            continue;
        if ( ! pDescriptor->mxBorderWindow.is() || ! pDescriptor->mxContentWindow.is())
            continue;
        Reference<awt::XWindow2> xWindow (pDescriptor->mxBorderWindow, UNO_QUERY);
        if (xWindow.is() && ! xWindow->isVisible())
            continue;

        const awt::Rectangle aOuterBorderBox (pDescriptor->mxBorderWindow->getPosSize());
        awt::Rectangle aInnerBorderBox (pDescriptor->mxContentWindow->getPosSize());
        aInnerBorderBox.X += aOuterBorderBox.X;
        aInnerBorderBox.Y += aOuterBorderBox.Y;
        aRectangles.push_back(aInnerBorderBox);
    }
    Reference<rendering::XPolyPolygon2D> xPolyPolygon (
        PresenterGeometryHelper::CreatePolygon(
            aRectangles,
            mxParentCanvas->getDevice()));
    if (xPolyPolygon.is())
        xPolyPolygon->setFillRule(rendering::FillRule_EVEN_ODD);
    return xPolyPolygon;
}




void PresenterWindowManager::UpdateWindowList (void)
{
    try
    {
        OSL_ASSERT(mxComponentContext.is());

        Reference<lang::XComponent> xComponent (mxPaneBorderManager, UNO_QUERY);
        if (xComponent.is())
            xComponent->dispose();

        Reference<lang::XMultiComponentFactory> xFactory (mxComponentContext->getServiceManager());
        if (xFactory.is())
        {
            Sequence<Any> aArguments (1 + mpPaneContainer->maPanes.size()*2);
            sal_Int32 nIndex (0);
            aArguments[nIndex++] = Any(mxParentWindow);
            for (sal_uInt32 nPaneIndex=0; nPaneIndex<mpPaneContainer->maPanes.size(); ++nPaneIndex)
            {
                if ( ! mpPaneContainer->maPanes[nPaneIndex]->mbIsActive)
                    continue;

                const Reference<awt::XWindow> xBorderWindow (
                    mpPaneContainer->maPanes[nPaneIndex]->mxBorderWindow);
                const Reference<awt::XWindow> xContentWindow (
                    mpPaneContainer->maPanes[nPaneIndex]->mxContentWindow);
                const Reference<awt::XWindow2> xBorderWindow2(xBorderWindow, UNO_QUERY);
                if (xBorderWindow.is()
                    && xContentWindow.is()
                    && ( ! xBorderWindow2.is() || xBorderWindow2->isVisible()))
                {
                    aArguments[nIndex++] = Any(xBorderWindow);
                    aArguments[nIndex++] = Any(xContentWindow);
                }
            }

            aArguments.realloc(nIndex);
            mxPaneBorderManager = xFactory->createInstanceWithArgumentsAndContext(
                OUString::createFromAscii("com.sun.star.comp.Draw.PresenterPaneBorderManager"),
                aArguments,
                mxComponentContext);
        }
    }
    catch (RuntimeException&)
    {
    }
}




void PresenterWindowManager::Invalidate (void)
{
    Reference<awt::XWindowPeer> xPeer (mxParentWindow, UNO_QUERY);
    if (xPeer.is())
        xPeer->invalidate(
            awt::InvalidateStyle::CHILDREN|awt::InvalidateStyle::NOTRANSPARENT);
}




Reference<awt::XWindow> PresenterWindowManager::GetParentWindow (void) const
{
    return mxParentWindow;
}




Reference<rendering::XCanvas> PresenterWindowManager::GetParentCanvas (void) const
{
    return mxParentCanvas;
}




void PresenterWindowManager::Update (void)
{
    mxClipPolygon = NULL;
    mbIsLayoutPending = true;

    UpdateWindowList();
    Invalidate();
}




void PresenterWindowManager::ThrowIfDisposed (void) const
    throw (::com::sun::star::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterWindowManager has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}





} } // end of namespace ::sdext::presenter
