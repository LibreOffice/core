/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SDEXT_PRESENTER_PRESENTER_SCROLL_BAR_HXX
#define SDEXT_PRESENTER_PRESENTER_SCROLL_BAR_HXX

#include "PresenterBitmapContainer.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase4.hxx>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterCanvasHelper;
class PresenterPaintManager;

namespace {
    typedef ::cppu::WeakComponentImplHelper4 <
        css::awt::XWindowListener,
        css::awt::XPaintListener,
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

    void SetVisible (const bool bIsVisible);

    /** Set the bounding box of the scroll bar.
    */
    void SetPosSize (const css::geometry::RealRectangle2D& rBox);

    /** Set the position of the movable thumb.
        @param nPosition
            A value between 0 and the last value given to SetTotalSize()
            minus the last value given to SetThumbSize().
    */
    void SetThumbPosition (
        double nPosition,
        const bool bAsynchronousRepaint);

    double GetThumbPosition (void) const;

    /** Set the upper border of the slider range.
    */
    void SetTotalSize (const double nTotalSize);

    double GetTotalSize (void) const;

    /** Set the size of the movable thumb.
        @param nThumbSize
            A value not larger than the last value given to SetTotalSize().
    */
    void SetThumbSize (const double nThumbSize);
    double GetThumbSize (void) const;

    void SetLineHeight (const double nLineHeight);
    double GetLineHeight (void) const;

    /** Set the canvas that is used for painting the scroll bar.
    */
    void SetCanvas (const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);

    void SetBackground (const SharedBitmapDescriptor& rpBackgroundBitmap);

    /** Call this after changing total size or thumb position or size to
        move the thumb to a valid position.
    */
    void CheckValues (void);

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


    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent)
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


    enum Area { Total, Pager, Thumb, PagerUp, PagerDown, PrevButton, NextButton, None,
                __AreaCount__ = None };

protected:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;
    ::boost::shared_ptr<PresenterPaintManager> mpPaintManager;
    double mnThumbPosition;
    double mnTotalSize;
    double mnThumbSize;
    double mnLineHeight;
    css::geometry::RealPoint2D maDragAnchor;
    ::boost::function<void(double)> maThumbMotionListener;
    Area meButtonDownArea;
    Area meMouseMoveArea;
    css::geometry::RealRectangle2D maBox[__AreaCount__];
    bool mbIsNotificationActive;
    static boost::weak_ptr<PresenterBitmapContainer> mpSharedBitmaps;
    boost::shared_ptr<PresenterBitmapContainer> mpBitmaps;
    SharedBitmapDescriptor mpPrevButtonDescriptor;
    SharedBitmapDescriptor mpNextButtonDescriptor;
    SharedBitmapDescriptor mpPagerStartDescriptor;
    SharedBitmapDescriptor mpPagerCenterDescriptor;
    SharedBitmapDescriptor mpPagerEndDescriptor;
    SharedBitmapDescriptor mpThumbStartDescriptor;
    SharedBitmapDescriptor mpThumbCenterDescriptor;
    SharedBitmapDescriptor mpThumbEndDescriptor;
    bool maEnabledState[__AreaCount__];

    virtual css::geometry::RealRectangle2D GetRectangle (const Area eArea) const;
    virtual double GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const = 0;
    virtual void UpdateDragAnchor (const double nDragDistance) = 0;
    virtual css::geometry::RealPoint2D GetPoint (const double nMajor, const double nMinor) const = 0;
    virtual double GetMajor (const double nX, const double nY) const = 0;
    virtual double GetMinor (const double nX, const double nY) const = 0;
    virtual void UpdateBorders (void) = 0;
    virtual void UpdateBitmaps (void) = 0;
    virtual void PaintComposite(
        const css::awt::Rectangle& rRepaintBox,
        const Area eArea,
        const SharedBitmapDescriptor& rpStartBitmaps,
        const SharedBitmapDescriptor& rpCenterBitmaps,
        const SharedBitmapDescriptor& rpEndBitmaps) = 0;

    PresenterScrollBar (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const ::boost::shared_ptr<PresenterPaintManager>& rpPaintManager,
        const ::boost::function<void(double)>& rThumbMotionListener);

    void Repaint (
        const css::geometry::RealRectangle2D aBox,
        const bool bAsynchronous);
    void PaintBackground (
        const css::awt::Rectangle& rRepaintBox);
    void PaintBitmap(
        const css::awt::Rectangle& rRepaintBox,
        const Area eArea,
        const SharedBitmapDescriptor& rpBitmaps);
    void NotifyThumbPositionChange (void);
    void UpdateWidthOrHeight (sal_Int32& rSize,
        const SharedBitmapDescriptor& rpDescriptor);
    css::uno::Reference<css::rendering::XBitmap> GetBitmap (
        const Area eArea,
        const SharedBitmapDescriptor& rpBitmaps) const;
    PresenterBitmapContainer::BitmapDescriptor::Mode GetBitmapMode (
        const Area eArea) const;
    bool IsDisabled (const Area eArea) const;
    double ValidateThumbPosition (double nPosition);
    void SetThumbPosition (
        double nPosition,
        const bool bAsynchronousRepaint,
        const bool bValidate,
        const bool bNotify);

private:
    class MousePressRepeater;
    ::boost::shared_ptr<MousePressRepeater> mpMousePressRepeater;
    SharedBitmapDescriptor mpBackgroundBitmap;
    ::boost::scoped_ptr<PresenterCanvasHelper> mpCanvasHelper;

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
        const ::boost::shared_ptr<PresenterPaintManager>& rpPaintManager,
        const ::boost::function<void(double)>& rThumbMotionListener);
    virtual ~PresenterVerticalScrollBar (void);
    virtual sal_Int32 GetSize (void) const;

protected:
    virtual double GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const;
    virtual void UpdateDragAnchor (const double nDragDistance);
    virtual css::geometry::RealPoint2D GetPoint (const double nMajor, const double nMinor) const;
    virtual double GetMinor (const double nX, const double nY) const;
    virtual double GetMajor (const double nX, const double nY) const;
    virtual void UpdateBorders (void);
    virtual void UpdateBitmaps (void);
    virtual void PaintComposite(
        const css::awt::Rectangle& rRepaintBox,
        const Area eArea,
        const SharedBitmapDescriptor& rpStartBitmaps,
        const SharedBitmapDescriptor& rpCenterBitmaps,
        const SharedBitmapDescriptor& rpEndBitmaps);

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
        const ::boost::shared_ptr<PresenterPaintManager>& rpPaintManager,
        const ::boost::function<void(double)>& rThumbMotionListener);
    virtual ~PresenterHorizontalScrollBar (void);
    virtual sal_Int32 GetSize (void) const;

protected:
    virtual double GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const;
    virtual void UpdateDragAnchor (const double nDragDistance);
    virtual css::geometry::RealPoint2D GetPoint (const double nMajor, const double nMinor) const;
    virtual double GetMinor (const double nX, const double nY) const;
    virtual double GetMajor (const double nX, const double nY) const;
    virtual void UpdateBorders (void);
    virtual void UpdateBitmaps (void);
    virtual void PaintComposite(
        const css::awt::Rectangle& rRepaintBox,
        const Area eArea,
        const SharedBitmapDescriptor& rpStartBitmaps,
        const SharedBitmapDescriptor& rpCenterBitmaps,
        const SharedBitmapDescriptor& rpEndBitmaps);

private:
    sal_Int32 mnScrollBarHeight;
};




} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
