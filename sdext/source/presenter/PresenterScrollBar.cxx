/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterScrollBar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:03:57 $
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

#include "PresenterScrollBar.hxx"
#include "PresenterBitmapContainer.hxx"
#include "PresenterGeometryHelper.hxx"
#include <com/sun/star/awt/InvalidateStyle.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/TexturingMode.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <boost/weak_ptr.hpp>
#include <cmath>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

#define A2S(pString) (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(pString)))


namespace sdext { namespace presenter {

namespace {
    double clip (const double nMin, const double nMax, const double nValue)
    {
        if (nValue < nMin)
            return nMin;
        else if (nValue > nMax)
            return nMax;
        else
            return nValue;
    }

} // end of anonymous namespace



//===== PresenterScrollBar ====================================================

boost::weak_ptr<PresenterBitmapContainer> PresenterScrollBar::mpSharedBitmaps;

PresenterScrollBar::PresenterScrollBar (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<awt::XWindow>& rxParentWindow,
    const ::boost::function<void(double)>& rThumbMotionListener)
    : PresenterScrollBarInterfaceBase(m_aMutex),
      mxComponentContext(rxComponentContext),
      mxParentWindow(rxParentWindow),
      mxWindow(),
      mxCanvas(),
      mxPresenterHelper(),
      mnThumbPosition(0),
      mnTotalSize(0),
      mnThumbSize(0),
      maDragPosition(-1,-1),
      maThumbMotionListener(rThumbMotionListener),
      meButtonDownArea(None),
      meMouseMoveArea(None),
      //maBorders(),
      mbIsNotificationActive(false),
      mpBitmaps(),
      maPrevButtonSet(),
      maNextButtonSet(),
      maPagerStartSet(),
      maPagerCenterSet(),
      maPagerEndSet()
{
    try
    {
        Reference<lang::XMultiComponentFactory> xFactory (rxComponentContext->getServiceManager());
        if ( ! xFactory.is())
            throw RuntimeException();

        mxPresenterHelper = Reference<drawing::XPresenterHelper>(
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.comp.Draw.PresenterHelper"),
                rxComponentContext),
            UNO_QUERY_THROW);

        // Create a window that contains the scroll bar.
        Reference<awt::XToolkit> xToolkit (
            xFactory->createInstanceWithContext(
                OUString::createFromAscii("com.sun.star.awt.Toolkit"),
                rxComponentContext),
            UNO_QUERY_THROW);
        awt::WindowDescriptor aWindowDescriptor (
            awt::WindowClass_CONTAINER,
            OUString(),
            Reference<awt::XWindowPeer>(rxParentWindow,UNO_QUERY_THROW),
            -1, // parent index not available
            awt::Rectangle(0,0,10,10),
            awt::WindowAttribute::SIZEABLE
                | awt::WindowAttribute::MOVEABLE
                | awt::WindowAttribute::NODECORATION);
        mxWindow = Reference<awt::XWindow>(
            xToolkit->createWindow(aWindowDescriptor),UNO_QUERY_THROW);

        // Make the background transparent.  The slide show paints its own background.
        Reference<awt::XWindowPeer> xPeer (mxWindow, UNO_QUERY_THROW);
        if (xPeer.is())
        {
            xPeer->setBackground(0xff000000);
        }

        mxWindow->setVisible(sal_True);
        mxWindow->addWindowListener(this);
        mxWindow->addMouseListener(this);
        mxWindow->addMouseMotionListener(this);
    }
    catch (RuntimeException&)
    {
    }
}




PresenterScrollBar::~PresenterScrollBar (void)
{
}




void SAL_CALL PresenterScrollBar::disposing (void)
{
    if (mxWindow.is())
    {
        mxWindow->removeWindowListener(this);
        mxWindow->removeMouseListener(this);
        mxWindow->removeMouseMotionListener(this);

        Reference<lang::XComponent> xComponent (mxWindow, UNO_QUERY);
        mxWindow = NULL;
        if (xComponent.is())
            xComponent->dispose();
    }

    mpBitmaps.reset();
}




void PresenterScrollBar::SetPosSize (const css::geometry::RealRectangle2D& rBox)
{
    if (mxWindow.is())
    {
        mxWindow->setPosSize(
            sal_Int32(floor(rBox.X1)),
            sal_Int32(floor(rBox.Y1)),
            sal_Int32(ceil(rBox.X2-rBox.X1)),
            sal_Int32(ceil(rBox.Y2-rBox.Y1)),
            awt::PosSize::POSSIZE);
        UpdateBorders();
    }
}




void PresenterScrollBar::SetThumbPosition (const double nPosition)
{
    if (nPosition != mnThumbPosition && ! mbIsNotificationActive)
    {
        if (nPosition < 0)
            mnThumbPosition = 0;
        else if ((nPosition + mnThumbSize) > mnTotalSize)
            mnThumbPosition = mnTotalSize - mnThumbSize;
        else
            mnThumbPosition = nPosition;

        UpdateBorders();

        Repaint(GetRectangle(Total));

        NotifyThumbPositionChange();
    }
}




void PresenterScrollBar::SetTotalSize (const double nTotalSize)
{
    if (mnTotalSize != nTotalSize)
    {
        mnTotalSize = nTotalSize;
        UpdateBorders();
        Repaint(GetRectangle(Total));
    }
}




void PresenterScrollBar::SetThumbSize (const double nThumbSize)
{
    OSL_ASSERT(nThumbSize>=0);
    if (mnThumbSize != nThumbSize)
    {
        mnThumbSize = nThumbSize;
        UpdateBorders();
        Repaint(GetRectangle(Total));
    }
}



void PresenterScrollBar::SetCanvas (const Reference<css::rendering::XCanvas>& rxCanvas)
{
    if (mxCanvas != rxCanvas)
    {
        mxCanvas = rxCanvas;
        if (mxCanvas.is())
        {
            if (mpBitmaps.get()==NULL)
            {
                if (mpSharedBitmaps.expired())
                {
                    try
                    {
                        mpBitmaps.reset(new PresenterBitmapContainer(
                            mxComponentContext,
                            mxCanvas,
                            OUString::createFromAscii("PresenterScreenSettings/ScrollBar/Bitmaps")));
                        mpSharedBitmaps = mpBitmaps;
                    }
                    catch(Exception&)
                    {
                        OSL_ASSERT(false);
                    }
                }
                else
                    mpBitmaps = ::boost::shared_ptr<PresenterBitmapContainer>(mpSharedBitmaps);
                UpdateBitmaps();
                UpdateBorders();
            }

            Repaint(GetRectangle(Total));
        }
    }
}




void PresenterScrollBar::Paint (
    const awt::Rectangle& rUpdateBox,
    const bool bNoClip)
{
    if ( ! mxCanvas.is() || ! mxWindow.is())
    {
        OSL_ASSERT(mxCanvas.is());
        OSL_ASSERT(mxWindow.is());
        return;
    }

    if ( ! bNoClip)
        if (PresenterGeometryHelper::AreRectanglesDisjoint (rUpdateBox, mxWindow->getPosSize()))
            return;

    Paint(PagerUp, maPagerStartSet, maPagerCenterSet, PresenterBitmapContainer::BitmapSet());
    Paint(PagerDown, PresenterBitmapContainer::BitmapSet(), maPagerCenterSet, maPagerEndSet);
    Paint(Thumb, maThumbStartSet, maThumbCenterSet, maThumbEndSet);
    Paint(PrevButton, maPrevButtonSet, 1,0,0);
    Paint(NextButton, maNextButtonSet, 0,1,0);

    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}






//----- XWindowListener -------------------------------------------------------

void SAL_CALL PresenterScrollBar::windowResized (const css::awt::WindowEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}





void SAL_CALL PresenterScrollBar::windowMoved (const css::awt::WindowEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterScrollBar::windowShown (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterScrollBar::windowHidden (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    (void)rEvent;
}





//----- XMouseListener --------------------------------------------------------

void SAL_CALL PresenterScrollBar::mousePressed (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    maDragPosition.X = rEvent.X;
    maDragPosition.Y = rEvent.Y;
    meButtonDownArea = GetArea(rEvent.X, rEvent.Y);
}




void SAL_CALL PresenterScrollBar::mouseReleased (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    if (mxPresenterHelper.is())
        mxPresenterHelper->releaseMouse(mxWindow);

    if (meButtonDownArea != None && meButtonDownArea == GetArea(rEvent.X, rEvent.Y))
    {
        switch (meButtonDownArea)
        {
            case PrevButton:
                SetThumbPosition(mnThumbPosition - mnTotalSize / 100.0);
                break;

            case NextButton:
                SetThumbPosition(mnThumbPosition + mnTotalSize / 100.0);
                break;

            case PagerUp:
                SetThumbPosition(mnThumbPosition - mnTotalSize / 20.0);
                break;

            case PagerDown:
                SetThumbPosition(mnThumbPosition + mnTotalSize / 20.0);
                break;

            default:
                break;
        }
    }
}




void SAL_CALL PresenterScrollBar::mouseEntered (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL PresenterScrollBar::mouseExited (const css::awt::MouseEvent& rEvent)
    throw(css::uno::RuntimeException)
{
    (void)rEvent;
    if (meMouseMoveArea != None)
    {
        const Area eOldMouseMoveArea (meMouseMoveArea);
        meMouseMoveArea = None;
        Repaint(GetRectangle(eOldMouseMoveArea));
    }
    meButtonDownArea = None;
    meMouseMoveArea = None;
}





//----- XMouseMotionListener --------------------------------------------------

void SAL_CALL PresenterScrollBar::mouseMoved (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    const Area eArea (GetArea(rEvent.X, rEvent.Y));
    if (eArea != meMouseMoveArea)
    {
        const Area eOldMouseMoveArea (meMouseMoveArea);
        meMouseMoveArea = eArea;
        if (eOldMouseMoveArea != None)
            Repaint(GetRectangle(eOldMouseMoveArea));
        if (meMouseMoveArea != None)
            Repaint(GetRectangle(meMouseMoveArea));
    }
}




void SAL_CALL PresenterScrollBar::mouseDragged (const css::awt::MouseEvent& rEvent)
    throw (css::uno::RuntimeException)
{
    if (meButtonDownArea != Thumb)
        return;
    if (mxPresenterHelper.is())
        mxPresenterHelper->captureMouse(mxWindow);

    const double nDragDistance (GetDragDistance(rEvent.X,rEvent.Y));
    maDragPosition.X = rEvent.X;
    maDragPosition.Y = rEvent.Y;
    if (nDragDistance != 0)
    {
        SetThumbPosition(mnThumbPosition + nDragDistance);
    }
}




//----- lang::XEventListener --------------------------------------------------

void SAL_CALL PresenterScrollBar::disposing (const css::lang::EventObject& rEvent)
    throw (css::uno::RuntimeException)
{
    if (rEvent.Source == mxWindow)
        mxWindow = NULL;
}




//-----------------------------------------------------------------------------

geometry::RealRectangle2D PresenterScrollBar::GetRectangle (const Area eArea) const
{
    if ( ! mxWindow.is())
        return geometry::RealRectangle2D(0,0,0,0);

    switch (eArea)
    {
        case Total:
        default:
            return GetBorderRectangle(LeftOrTopOfPrevButton, RightOrBottomOfNextButton);

        case PrevButton:
            return GetBorderRectangle(LeftOrTopOfPrevButton, RightOrBottomOfPrevButton);

        case NextButton:
            return GetBorderRectangle(LeftOrTopOfNextButton, RightOrBottomOfNextButton);

        case Pager:
            return GetBorderRectangle(RightOrBottomOfPrevButton, LeftOrTopOfNextButton);

        case PagerUp:
            return GetBorderRectangle(RightOrBottomOfPrevButton, LeftOrTopOfThumb);

        case PagerDown:
            return GetBorderRectangle(RightOrBottomOfThumb, LeftOrTopOfNextButton);

        case Thumb:
            return GetBorderRectangle(LeftOrTopOfThumb, RightOrBottomOfThumb);
    }
}




void PresenterScrollBar::Repaint (const geometry::RealRectangle2D aBox)
{
#if 1
    Paint(
        awt::Rectangle(
            sal_Int32(aBox.X1),
            sal_Int32(aBox.Y1),
            sal_Int32(aBox.X2 - aBox.X1),
            sal_Int32(aBox.Y2 - aBox.Y1)),
        true);
#else
    Reference<awt::XWindowPeer> xPeer (mxParentWindow, UNO_QUERY);
    if (xPeer.is() && mxWindow.is())
    {
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        const sal_Int32 nX (sal_Int32(floor(aBox.X1)) + aWindowBox.X);
        const sal_Int32 nY (sal_Int32(floor(aBox.Y1)) + aWindowBox.Y);
        const sal_Int32 nWidth (sal_Int32(ceil(aBox.X2) - nX));
        const sal_Int32 nHeight (sal_Int32(ceil(aBox.Y2) - nY));
        xPeer->invalidateRect(awt::Rectangle(nX,nY,nWidth,nHeight), awt::InvalidateStyle::UPDATE);
    }
#endif
}




void PresenterScrollBar::Paint(
    const Area eArea,
    const PresenterBitmapContainer::BitmapSet& rBitmaps,
    const double nRed,
    const double nGreen,
    const double nBlue)
{
    if (rBitmaps.mxNormalIcon.is())
        PaintBitmap(eArea, rBitmaps);
    else
        PaintBox(eArea, nRed, nGreen, nBlue);
}




void PresenterScrollBar::Paint(
    const Area eArea,
    const PresenterBitmapContainer::BitmapSet& rStartBitmaps,
    const PresenterBitmapContainer::BitmapSet& rCenterBitmaps,
    const PresenterBitmapContainer::BitmapSet& rEndBitmaps)
{
    geometry::RealRectangle2D aBox (GetRectangle(eArea));
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    aBox.X1 += aWindowBox.X;
    aBox.Y1 += aWindowBox.Y;
    aBox.X2 += aWindowBox.X;
    aBox.Y2 += aWindowBox.Y;
    Reference<rendering::XPolyPolygon2D> xPolygon (
        PresenterGeometryHelper::CreatePolygon(aBox, mxCanvas->getDevice()));

    if (xPolygon.is())
    {

        const rendering::ViewState aViewState (
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            xPolygon);

        rendering::RenderState aRenderState (
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL,
            Sequence<double>(3),
            rendering::CompositeOperation::SOURCE);

        Reference<rendering::XBitmap> xStartBitmap (GetBitmap(eArea, rStartBitmaps));
        Reference<rendering::XBitmap> xCenterBitmap (GetBitmap(eArea, rCenterBitmaps));
        Reference<rendering::XBitmap> xEndBitmap (GetBitmap(eArea, rEndBitmaps));

        if (xCenterBitmap.is())
        {
            const geometry::IntegerSize2D aSize (xCenterBitmap->getSize());
            geometry::RealPoint2D aFilteredMajorSize (GetPoint(
                GetMajor(aSize.Width, aSize.Height),
                0));
            geometry::RealPoint2D aOffset (aBox.X1, aBox.Y1);
            while(aOffset.X < aBox.X2 && aOffset.Y < aBox.Y2)
            {
                aRenderState.AffineTransform.m02 = aOffset.X;
                aRenderState.AffineTransform.m12 = aOffset.Y;
                mxCanvas->drawBitmap(
                    xCenterBitmap,
                    aViewState,
                    aRenderState);
                aOffset.X += aFilteredMajorSize.X;
                aOffset.Y += aFilteredMajorSize.Y;
            }
        }

        if (xStartBitmap.is())
        {
            aRenderState.AffineTransform.m02 = aBox.X1;
            aRenderState.AffineTransform.m12 = aBox.Y1;
            mxCanvas->drawBitmap(
                xStartBitmap,
                aViewState,
                aRenderState);
        }

        if (xEndBitmap.is())
        {
            const geometry::IntegerSize2D aSize (xEndBitmap->getSize());
            geometry::RealPoint2D aFilteredSize (GetPoint(
                GetMajor(aSize.Width, aSize.Height),
                GetMinor(aSize.Width, aSize.Height)));
            aRenderState.AffineTransform.m02 = aBox.X2 - aFilteredSize.X;
            aRenderState.AffineTransform.m12 = aBox.Y2 - aFilteredSize.Y;
            mxCanvas->drawBitmap(
                xEndBitmap,
                aViewState,
                aRenderState);
        }
    }
}




void PresenterScrollBar::PaintBitmap(
    const Area eArea,
    const PresenterBitmapContainer::BitmapSet& rBitmaps)
{
    const geometry::RealRectangle2D aBox (GetRectangle(eArea));
    Reference<rendering::XPolyPolygon2D> xPolygon (
        PresenterGeometryHelper::CreatePolygon(aBox, mxCanvas->getDevice()));
    Reference<rendering::XBitmap> xBitmap (GetBitmap(eArea,rBitmaps));

    if (xPolygon.is() && xBitmap.is())
    {
        const rendering::ViewState aViewState (
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL);

        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        rendering::RenderState aRenderState (
            geometry::AffineMatrix2D(
                1,0,aWindowBox.X+aBox.X1,
                0,1,aWindowBox.Y+aBox.Y1),
            NULL,
            Sequence<double>(3),
            rendering::CompositeOperation::SOURCE);

        mxCanvas->drawBitmap(
            xBitmap,
            aViewState,
            aRenderState);
    }
}




void PresenterScrollBar::PaintBox (
    const Area eArea,
    const double nRed,
    const double nGreen,
    const double nBlue)
{
    Reference<rendering::XPolyPolygon2D> xPolygon (
        PresenterGeometryHelper::CreatePolygon(GetRectangle(eArea),
            mxCanvas->getDevice()));

    if (xPolygon.is())
    {
        const rendering::ViewState aViewState (
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL);

        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        rendering::RenderState aRenderState (
            geometry::AffineMatrix2D(1,0,aWindowBox.X, 0,1,aWindowBox.Y),
            NULL,
            Sequence<double>(3),
            rendering::CompositeOperation::SOURCE);

        if (meMouseMoveArea == eArea)
        {
            aRenderState.DeviceColor[0] = (1+nRed)/2;
            aRenderState.DeviceColor[1] = (1+nGreen)/2;
            aRenderState.DeviceColor[2] = (1+nBlue)/2;
        }
        else
        {
            aRenderState.DeviceColor[0] = nRed;
            aRenderState.DeviceColor[1] = nGreen;
            aRenderState.DeviceColor[2] = nBlue;
        }
        mxCanvas->fillPolyPolygon(
            xPolygon,
            aViewState,
            aRenderState);
    }
}




void PresenterScrollBar::NotifyThumbPositionChange (void)
{
    if ( ! mbIsNotificationActive)
    {
        mbIsNotificationActive = true;

        try
        {
            maThumbMotionListener(mnThumbPosition);
        }
        catch (Exception&)
        {
        }

        mbIsNotificationActive = false;
    }
}




PresenterScrollBar::Area PresenterScrollBar::GetArea (const double nX, const double nY) const
{
    const double nValue (GetMajor(nX,nY));

    if (nValue < maBorders[LeftOrTopOfThumb])
        if (nValue < maBorders[RightOrBottomOfPrevButton])
            if (nValue < maBorders[LeftOrTopOfPrevButton])
                return None;
            else
                return PrevButton;
        else
            return PagerUp;
    else if (nValue > maBorders[RightOrBottomOfThumb])
        if (nValue > maBorders[LeftOrTopOfNextButton])
            if (nValue > maBorders[RightOrBottomOfNextButton])
                return None;
            else
                return NextButton;
        else
            return PagerDown;
    else
        return Thumb;
}




void PresenterScrollBar::UpdateWidthOrHeight (
    sal_Int32& rSize,
    const PresenterBitmapContainer::BitmapSet& rSet)
{
    if (rSet.mxNormalIcon.is())
    {
        const geometry::IntegerSize2D aBitmapSize (rSet.mxNormalIcon->getSize());
        const sal_Int32 nBitmapSize = (sal_Int32)GetMinor(aBitmapSize.Width, aBitmapSize.Height);
        if (nBitmapSize > rSize)
            rSize = nBitmapSize;
    }
}




css::uno::Reference<css::rendering::XBitmap> PresenterScrollBar::GetBitmap (
    const Area eArea,
    const PresenterBitmapContainer::BitmapSet& rBitmaps) const
{
    if (eArea==meMouseMoveArea && rBitmaps.mxMouseOverIcon.is())
        return rBitmaps.mxMouseOverIcon;
    return rBitmaps.mxNormalIcon;
}




//===== PresenterVerticalScrollBar ============================================

PresenterVerticalScrollBar::PresenterVerticalScrollBar (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<awt::XWindow>& rxParentWindow,
    const ::boost::function<void(double)>& rThumbMotionListener)
    : PresenterScrollBar(rxComponentContext, rxParentWindow, rThumbMotionListener),
      mnScrollBarWidth(0)
{
}




PresenterVerticalScrollBar::~PresenterVerticalScrollBar (void)
{
}




geometry::RealRectangle2D PresenterVerticalScrollBar::GetBorderRectangle (
    const sal_Int32 nLeftOrTopBorder,
    const sal_Int32 nRightOrBottomBorder) const
{
    OSL_ASSERT(nLeftOrTopBorder < nRightOrBottomBorder);
    return geometry::RealRectangle2D(
        0, maBorders[nLeftOrTopBorder],
        mnScrollBarWidth, maBorders[nRightOrBottomBorder]);
}




double PresenterVerticalScrollBar::GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const
{
    (void)nX;
    const sal_Int32 nDistance (nY - maDragPosition.Y);
    if (nDistance == 0)
        return 0;
    else
    {
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        const double nBarWidth (aWindowBox.Width);
        const double nPagerHeight (aWindowBox.Height - 2*nBarWidth);
        return mnTotalSize / nPagerHeight * nDistance;
    }
}




sal_Int32 PresenterVerticalScrollBar::GetSize (void) const
{
    return mnScrollBarWidth;
}




geometry::RealPoint2D PresenterVerticalScrollBar::GetPoint (
    const double nMajor, const double nMinor) const
{
    return geometry::RealPoint2D(nMinor, nMajor);
}




double PresenterVerticalScrollBar::GetMajor (const double nX, const double nY) const
{
    (void)nX;
    return nY;
}




double PresenterVerticalScrollBar::GetMinor (const double nX, const double nY) const
{
    (void)nY;
    return nX;
}




void PresenterVerticalScrollBar::UpdateBorders (void)
{
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    const double nButtonHeight = aWindowBox.Height>=2*mnScrollBarWidth
        ? mnScrollBarWidth : aWindowBox.Height/2.0;
    const double nPagerHeight (aWindowBox.Height - 2*nButtonHeight);

    maBorders[LeftOrTopOfPrevButton] = 0;
    maBorders[RightOrBottomOfPrevButton] = nButtonHeight;
    maBorders[LeftOrTopOfNextButton] = nPagerHeight + nButtonHeight;
    maBorders[RightOrBottomOfNextButton] = aWindowBox.Height;
    if (mnTotalSize <= 0)
    {
        maBorders[LeftOrTopOfThumb] = maBorders[RightOrBottomOfPrevButton];
        maBorders[RightOrBottomOfThumb] = maBorders[LeftOrTopOfNextButton];
    }
    else
    {
        maBorders[LeftOrTopOfThumb] = clip(
            maBorders[RightOrBottomOfPrevButton],
            maBorders[LeftOrTopOfNextButton],
            nButtonHeight + (mnThumbPosition) / mnTotalSize * nPagerHeight);
        maBorders[RightOrBottomOfThumb] = clip(
            maBorders[RightOrBottomOfPrevButton],
            maBorders[LeftOrTopOfNextButton],
            nButtonHeight + (mnThumbPosition+mnThumbSize) / mnTotalSize * nPagerHeight);
    }

    OSL_ASSERT(0 <= maBorders[LeftOrTopOfPrevButton]);
    OSL_ASSERT(maBorders[LeftOrTopOfPrevButton] <= maBorders[RightOrBottomOfPrevButton]);
    OSL_ASSERT(maBorders[RightOrBottomOfPrevButton] <= maBorders[LeftOrTopOfThumb]);
    OSL_ASSERT(maBorders[LeftOrTopOfThumb] <= maBorders[RightOrBottomOfThumb]);
    OSL_ASSERT(maBorders[RightOrBottomOfThumb] <= maBorders[LeftOrTopOfNextButton]);
    OSL_ASSERT(maBorders[LeftOrTopOfNextButton] <= maBorders[RightOrBottomOfNextButton]);
    OSL_ASSERT(maBorders[RightOrBottomOfNextButton] <= aWindowBox.Height);
}




void PresenterVerticalScrollBar::UpdateBitmaps (void)
{
    if (mpBitmaps.get() != NULL)
    {
        maPrevButtonSet = mpBitmaps->GetButtons(A2S("Up"));
        maNextButtonSet = mpBitmaps->GetButtons(A2S("Down"));
        maPagerStartSet = mpBitmaps->GetButtons(A2S("PagerTop"));
        maPagerCenterSet = mpBitmaps->GetButtons(A2S("PagerVertical"));
        maPagerEndSet = mpBitmaps->GetButtons(A2S("PagerBottom"));
        maThumbStartSet = mpBitmaps->GetButtons(A2S("ThumbTop"));
        maThumbCenterSet = mpBitmaps->GetButtons(A2S("ThumbVertical"));
        maThumbEndSet = mpBitmaps->GetButtons(A2S("ThumbBottom"));

        mnScrollBarWidth = 0;
        UpdateWidthOrHeight(mnScrollBarWidth, maPrevButtonSet);
        UpdateWidthOrHeight(mnScrollBarWidth, maNextButtonSet);
        UpdateWidthOrHeight(mnScrollBarWidth, maPagerStartSet);
        UpdateWidthOrHeight(mnScrollBarWidth, maPagerCenterSet);
        UpdateWidthOrHeight(mnScrollBarWidth, maPagerEndSet);
        UpdateWidthOrHeight(mnScrollBarWidth, maThumbStartSet);
        UpdateWidthOrHeight(mnScrollBarWidth, maThumbCenterSet);
        UpdateWidthOrHeight(mnScrollBarWidth, maThumbEndSet);
        if (mnScrollBarWidth == 0)
            mnScrollBarWidth = 20;
    }
}




//===== PresenterHorizontalScrollBar ============================================

PresenterHorizontalScrollBar::PresenterHorizontalScrollBar (
    const Reference<XComponentContext>& rxComponentContext,
    const Reference<awt::XWindow>& rxParentWindow,
    const ::boost::function<void(double)>& rThumbMotionListener)
    : PresenterScrollBar(rxComponentContext, rxParentWindow, rThumbMotionListener),
      mnScrollBarHeight(0)
{
}




PresenterHorizontalScrollBar::~PresenterHorizontalScrollBar (void)
{
}




geometry::RealRectangle2D PresenterHorizontalScrollBar::GetBorderRectangle (
    const sal_Int32 nLeftOrTopBorder,
    const sal_Int32 nRightOrBottomBorder) const
{
    OSL_ASSERT(nLeftOrTopBorder < nRightOrBottomBorder);
    return geometry::RealRectangle2D(
        maBorders[nLeftOrTopBorder], 0,
        maBorders[nRightOrBottomBorder], mnScrollBarHeight);
}




double PresenterHorizontalScrollBar::GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const
{
    (void)nY;
    const sal_Int32 nDistance (nX - maDragPosition.X);
    if (nDistance == 0)
        return 0;
    else
    {
        const awt::Rectangle aWindowBox (mxWindow->getPosSize());
        const double nBarHeight (aWindowBox.Height);
        const double nPagerWidth (aWindowBox.Width - 2*nBarHeight);
        return mnTotalSize / nPagerWidth * nDistance;
    }
}




sal_Int32 PresenterHorizontalScrollBar::GetSize (void) const
{
    return mnScrollBarHeight;
}





geometry::RealPoint2D PresenterHorizontalScrollBar::GetPoint (
    const double nMajor, const double nMinor) const
{
    return geometry::RealPoint2D(nMajor, nMinor);
}




double PresenterHorizontalScrollBar::GetMajor (const double nX, const double nY) const
{
    (void)nY;
    return nX;
}




double PresenterHorizontalScrollBar::GetMinor (const double nX, const double nY) const
{
    (void)nX;
    return nY;
}




void PresenterHorizontalScrollBar::UpdateBorders (void)
{
    const awt::Rectangle aWindowBox (mxWindow->getPosSize());
    const double nButtonWidth = (aWindowBox.Width>2*mnScrollBarHeight
        ? mnScrollBarHeight : aWindowBox.Width/2.0);
    const double nPagerWidth (aWindowBox.Width - 2*nButtonWidth);

    maBorders[LeftOrTopOfPrevButton] = 0;
    maBorders[RightOrBottomOfPrevButton] = nButtonWidth;
    maBorders[LeftOrTopOfNextButton] = nPagerWidth + nButtonWidth;
    maBorders[RightOrBottomOfNextButton] = aWindowBox.Width;
    if (mnTotalSize <= 0)
    {
        maBorders[LeftOrTopOfThumb] = maBorders[RightOrBottomOfPrevButton];
        maBorders[RightOrBottomOfThumb] = maBorders[LeftOrTopOfNextButton];
    }
    else
    {
        maBorders[LeftOrTopOfThumb] = clip (
            maBorders[RightOrBottomOfPrevButton],
            maBorders[LeftOrTopOfNextButton],
            nButtonWidth + (mnThumbPosition) / mnTotalSize * nPagerWidth);
        maBorders[RightOrBottomOfThumb] = clip (
            maBorders[RightOrBottomOfPrevButton],
            maBorders[LeftOrTopOfNextButton],
            nButtonWidth + (mnThumbPosition+mnThumbSize) / mnTotalSize * nPagerWidth);
    }


    OSL_ASSERT(0 <= maBorders[LeftOrTopOfPrevButton]);
    OSL_ASSERT(maBorders[LeftOrTopOfPrevButton] <= maBorders[RightOrBottomOfPrevButton]);
    OSL_ASSERT(maBorders[RightOrBottomOfPrevButton] <= maBorders[LeftOrTopOfThumb]);
    OSL_ASSERT(maBorders[LeftOrTopOfThumb] <= maBorders[RightOrBottomOfThumb]);
    OSL_ASSERT(maBorders[RightOrBottomOfThumb] <= maBorders[LeftOrTopOfNextButton]);
    OSL_ASSERT(maBorders[LeftOrTopOfNextButton] <= maBorders[RightOrBottomOfNextButton]);
    OSL_ASSERT(maBorders[RightOrBottomOfNextButton] <= aWindowBox.Width);
}




void PresenterHorizontalScrollBar::UpdateBitmaps (void)
{
    if (mpBitmaps.get() != NULL)
    {
        maPrevButtonSet = mpBitmaps->GetButtons(A2S("Left"));
        maNextButtonSet = mpBitmaps->GetButtons(A2S("Right"));
        maPagerStartSet = mpBitmaps->GetButtons(A2S("PagerLeft"));
        maPagerCenterSet = mpBitmaps->GetButtons(A2S("PagerHorizontal"));
        maPagerEndSet = mpBitmaps->GetButtons(A2S("PagerRight"));
        maThumbStartSet = mpBitmaps->GetButtons(A2S("ThumbLeft"));
        maThumbCenterSet = mpBitmaps->GetButtons(A2S("ThumbHorizontal"));
        maThumbEndSet = mpBitmaps->GetButtons(A2S("ThumbRight"));

        mnScrollBarHeight = 0;
        UpdateWidthOrHeight(mnScrollBarHeight, maPrevButtonSet);
        UpdateWidthOrHeight(mnScrollBarHeight, maNextButtonSet);
        UpdateWidthOrHeight(mnScrollBarHeight, maPagerStartSet);
        UpdateWidthOrHeight(mnScrollBarHeight, maPagerCenterSet);
        UpdateWidthOrHeight(mnScrollBarHeight, maPagerEndSet);
        UpdateWidthOrHeight(mnScrollBarHeight, maThumbStartSet);
        UpdateWidthOrHeight(mnScrollBarHeight, maThumbCenterSet);
        UpdateWidthOrHeight(mnScrollBarHeight, maThumbEndSet);
        if (mnScrollBarHeight == 0)
            mnScrollBarHeight = 20;
    }
}



} } // end of namespace ::sdext::presenter
