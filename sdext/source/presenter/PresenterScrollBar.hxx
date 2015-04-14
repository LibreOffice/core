/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERSCROLLBAR_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERSCROLLBAR_HXX

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
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

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
    virtual ~PresenterScrollBar();

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

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

    double GetThumbPosition() const { return mnThumbPosition;}

    /** Set the upper border of the slider range.
    */
    void SetTotalSize (const double nTotalSize);

    /** Set the size of the movable thumb.
        @param nThumbSize
            A value not larger than the last value given to SetTotalSize().
    */
    void SetThumbSize (const double nThumbSize);
    double GetThumbSize() const { return mnThumbSize;}

    void SetLineHeight (const double nLineHeight);
    double GetLineHeight() const { return mnLineHeight;}

    /** Set the canvas that is used for painting the scroll bar.
    */
    void SetCanvas (const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);

    void SetBackground (const SharedBitmapDescriptor& rpBackgroundBitmap);

    /** Call this after changing total size or thumb position or size to
        move the thumb to a valid position.
    */
    void CheckValues();

    /** On some occasions it is necessary to trigger the painting of a
        scrollbar from the outside.
    */
    void Paint (
        const css::awt::Rectangle& rUpdateBox,
        bool bNoClip = false);

    virtual sal_Int32 GetSize() const = 0;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XMouseListener

    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XMouseMotionListener

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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

    css::geometry::RealRectangle2D GetRectangle (const Area eArea) const;
    virtual double GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const = 0;
    virtual void UpdateDragAnchor (const double nDragDistance) = 0;
    virtual css::geometry::RealPoint2D GetPoint (const double nMajor, const double nMinor) const = 0;
    virtual double GetMinor (const double nX, const double nY) const = 0;
    virtual void UpdateBorders() = 0;
    virtual void UpdateBitmaps() = 0;
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
        const css::geometry::RealRectangle2D& rBox,
        const bool bAsynchronous);
    void PaintBackground (
        const css::awt::Rectangle& rRepaintBox);
    void PaintBitmap(
        const css::awt::Rectangle& rRepaintBox,
        const Area eArea,
        const SharedBitmapDescriptor& rpBitmaps);
    void NotifyThumbPositionChange();
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
    virtual ~PresenterVerticalScrollBar();
    virtual sal_Int32 GetSize() const SAL_OVERRIDE;

protected:
    virtual double GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const SAL_OVERRIDE;
    virtual void UpdateDragAnchor (const double nDragDistance) SAL_OVERRIDE;
    virtual css::geometry::RealPoint2D GetPoint (const double nMajor, const double nMinor) const SAL_OVERRIDE;
    virtual double GetMinor (const double nX, const double nY) const SAL_OVERRIDE;
    virtual void UpdateBorders() SAL_OVERRIDE;
    virtual void UpdateBitmaps() SAL_OVERRIDE;
    virtual void PaintComposite(
        const css::awt::Rectangle& rRepaintBox,
        const Area eArea,
        const SharedBitmapDescriptor& rpStartBitmaps,
        const SharedBitmapDescriptor& rpCenterBitmaps,
        const SharedBitmapDescriptor& rpEndBitmaps) SAL_OVERRIDE;

private:
    sal_Int32 mnScrollBarWidth;
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
