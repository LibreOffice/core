/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterScrollBar.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:04:09 $
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

#ifndef SDEXT_PRESENTER_PRESENTER_SCROLL_BAR_HXX
#define SDEXT_PRESENTER_PRESENTER_SCROLL_BAR_HXX

#include "PresenterBitmapContainer.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper3 <
        css::awt::XWindowListener,
        css::awt::XMouseListener,
        css::awt::XMouseMotionListener
    > PresenterScrollBarInterfaceBase;
}

/** Base class of horizontal and vertical scroll bars.
*/
class PresenterScrollBar
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterScrollBarInterfaceBase
{
public:
    typedef ::boost::function<void(double)> ThumbMotionListener;
    virtual ~PresenterScrollBar (void);

    virtual void SAL_CALL disposing (void);

    /** Set the bounding box of the scroll bar.
    */
    void SetPosSize (const css::geometry::RealRectangle2D& rBox);

    /** Set the position of the movable thumb.
        @param nPosition
            A value between 0 and the last value given to SetTotalSize()
            minus the last value given to SetThumbSize().
    */
    void SetThumbPosition (const double nPosition);

    /** Set the upper border of the slider range.
    */
    void SetTotalSize (const double nTotalSize);

    /** Set the size of the movable thumb.
        @param nThumbSize
            A value not larger than the last value given to SetTotalSize().
    */
    void SetThumbSize (const double nThumbSize);

    /** Set the canvas that is used for painting the scroll bar.
    */
    void SetCanvas (const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);

    /** On some occasions it is necessary to trigger the painting of a
        scrollbar from the outside.
    */
    virtual void Paint (
        const css::awt::Rectangle& rUpdateBox,
            bool bNoClip = false);

    virtual sal_Int32 GetSize (void) const = 0;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


    // XMouseListener

    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException);

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException);


    // XMouseMotionListener

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException);


    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);


protected:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;
    double mnThumbPosition;
    double mnTotalSize;
    double mnThumbSize;
    css::awt::Point maDragPosition;
    ::boost::function<void(double)> maThumbMotionListener;
    enum Area { None, Total, Pager, Thumb, PagerUp, PagerDown, PrevButton, NextButton };
    Area meButtonDownArea;
    Area meMouseMoveArea;
    enum Border {
        LeftOrTopOfPrevButton,
        RightOrBottomOfPrevButton,
        LeftOrTopOfThumb,
        RightOrBottomOfThumb,
        LeftOrTopOfNextButton,
        RightOrBottomOfNextButton,
        _Count
    };
    double maBorders[_Count];
    bool mbIsNotificationActive;
    static boost::weak_ptr<PresenterBitmapContainer> mpSharedBitmaps;
    boost::shared_ptr<PresenterBitmapContainer> mpBitmaps;
    PresenterBitmapContainer::BitmapSet maPrevButtonSet;
    PresenterBitmapContainer::BitmapSet maNextButtonSet;
    PresenterBitmapContainer::BitmapSet maPagerStartSet;
    PresenterBitmapContainer::BitmapSet maPagerCenterSet;
    PresenterBitmapContainer::BitmapSet maPagerEndSet;
    PresenterBitmapContainer::BitmapSet maThumbStartSet;
    PresenterBitmapContainer::BitmapSet maThumbCenterSet;
    PresenterBitmapContainer::BitmapSet maThumbEndSet;

    virtual css::geometry::RealRectangle2D GetRectangle (const Area eArea) const;
    virtual css::geometry::RealRectangle2D GetBorderRectangle (
        const sal_Int32 nLeftOrTopBorder,
        const sal_Int32 nRightOrBottomBorder) const = 0;
    virtual double GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const = 0;
    virtual css::geometry::RealPoint2D GetPoint (const double nMajor, const double nMinor) const = 0;
    virtual double GetMajor (const double nX, const double nY) const = 0;
    virtual double GetMinor (const double nX, const double nY) const = 0;
    virtual void UpdateBorders (void) = 0;
    virtual void UpdateBitmaps (void) = 0;

    PresenterScrollBar (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const ::boost::function<void(double)>& rThumbMotionListener);

    void Repaint (const css::geometry::RealRectangle2D aBox);
    void Paint(
        const Area eArea,
        const PresenterBitmapContainer::BitmapSet& rBitmaps,
        const double nRed,
        const double nGreen,
        const double nBlue);
    void PaintBitmap(
        const Area eArea,
        const PresenterBitmapContainer::BitmapSet& rBitmaps);
    void PaintBox (
        const Area eArea,
        const double nRed,
        const double nGreen,
        const double nBlue);
    void Paint(
        const Area eArea,
        const PresenterBitmapContainer::BitmapSet& rStartBitmaps,
        const PresenterBitmapContainer::BitmapSet& rCenterBitmaps,
        const PresenterBitmapContainer::BitmapSet& rEndBitmaps);
    void NotifyThumbPositionChange (void);
    void UpdateWidthOrHeight (sal_Int32& rSize,
        const PresenterBitmapContainer::BitmapSet& rSet);
    css::uno::Reference<css::rendering::XBitmap> GetBitmap (
        const Area eArea,
        const PresenterBitmapContainer::BitmapSet& rBitmaps) const;

private:
    Area GetArea (const double nX, const double nY) const;
};




/** A vertical scroll bar.
*/
class PresenterVerticalScrollBar : public PresenterScrollBar
{
public:
    PresenterVerticalScrollBar (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const ::boost::function<void(double)>& rThumbMotionListener);
    virtual ~PresenterVerticalScrollBar (void);
    virtual sal_Int32 GetSize (void) const;

protected:
    virtual css::geometry::RealRectangle2D GetBorderRectangle (
        const sal_Int32 nLeftOrTopBorder,
        const sal_Int32 nRightOrBottomBorder) const;
    virtual double GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const;
    virtual css::geometry::RealPoint2D GetPoint (const double nMajor, const double nMinor) const;
    virtual double GetMinor (const double nX, const double nY) const;
    virtual double GetMajor (const double nX, const double nY) const;
    virtual void UpdateBorders (void);
    virtual void UpdateBitmaps (void);

private:
    sal_Int32 mnScrollBarWidth;
};




/** A horizontal scroll bar.
*/
class PresenterHorizontalScrollBar : public PresenterScrollBar
{
public:
    PresenterHorizontalScrollBar (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const ::boost::function<void(double)>& rThumbMotionListener);
    virtual ~PresenterHorizontalScrollBar (void);
    virtual sal_Int32 GetSize (void) const;

protected:
    virtual css::geometry::RealRectangle2D GetBorderRectangle (
        const sal_Int32 nLeftOrTopBorder,
        const sal_Int32 nRightOrBottomBorder) const;
    virtual double GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const;
    virtual css::geometry::RealPoint2D GetPoint (const double nMajor, const double nMinor) const;
    virtual double GetMinor (const double nX, const double nY) const;
    virtual double GetMajor (const double nX, const double nY) const;
    virtual void UpdateBorders (void);
    virtual void UpdateBitmaps (void);

private:
    sal_Int32 mnScrollBarHeight;
};




} } // end of namespace ::sdext::presenter

#endif
