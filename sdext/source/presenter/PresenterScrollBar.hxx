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
#include <cppuhelper/compbase.hxx>

#include <functional>
#include <memory>

namespace sdext { namespace presenter {

class PresenterCanvasHelper;
class PresenterPaintManager;

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::awt::XWindowListener,
        css::awt::XPaintListener,
        css::awt::XMouseListener,
        css::awt::XMouseMotionListener
    > PresenterScrollBarInterfaceBase;
}

/** Base class of horizontal and vertical scroll bars.
*/
class PresenterScrollBar
    : private ::cppu::BaseMutex,
      public PresenterScrollBarInterfaceBase
{
public:
    typedef ::std::function<void (double)> ThumbMotionListener;
    virtual ~PresenterScrollBar();
    PresenterScrollBar(const PresenterScrollBar&) = delete;
    PresenterScrollBar& operator=(const PresenterScrollBar&) = delete;

    virtual void SAL_CALL disposing() override;

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
        const css::awt::Rectangle& rUpdateBox);

    virtual sal_Int32 GetSize() const = 0;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    // XPaintListener

    virtual void SAL_CALL windowPaint (const css::awt::PaintEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    // XMouseListener

    virtual void SAL_CALL mousePressed (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL mouseReleased (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL mouseEntered (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL mouseExited (const css::awt::MouseEvent& rEvent)
        throw(css::uno::RuntimeException, std::exception) override;

    // XMouseMotionListener

    virtual void SAL_CALL mouseMoved (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL mouseDragged (const css::awt::MouseEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    // lang::XEventListener
    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    enum Area { Total, Pager, Thumb, PagerUp, PagerDown, PrevButton, NextButton, None,
                AreaCount = None };

protected:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;
    std::shared_ptr<PresenterPaintManager> mpPaintManager;
    double mnThumbPosition;
    double mnTotalSize;
    double mnThumbSize;
    double mnLineHeight;
    css::geometry::RealPoint2D maDragAnchor;
    ::std::function<void (double)> maThumbMotionListener;
    Area meButtonDownArea;
    Area meMouseMoveArea;
    css::geometry::RealRectangle2D maBox[AreaCount];
    bool mbIsNotificationActive;
    static std::weak_ptr<PresenterBitmapContainer> mpSharedBitmaps;
    std::shared_ptr<PresenterBitmapContainer> mpBitmaps;
    SharedBitmapDescriptor mpPrevButtonDescriptor;
    SharedBitmapDescriptor mpNextButtonDescriptor;
    SharedBitmapDescriptor mpPagerStartDescriptor;
    SharedBitmapDescriptor mpPagerCenterDescriptor;
    SharedBitmapDescriptor mpPagerEndDescriptor;
    SharedBitmapDescriptor mpThumbStartDescriptor;
    SharedBitmapDescriptor mpThumbCenterDescriptor;
    SharedBitmapDescriptor mpThumbEndDescriptor;
    bool maEnabledState[AreaCount];

    css::geometry::RealRectangle2D const & GetRectangle (const Area eArea) const;
    virtual double GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const = 0;
    virtual void UpdateDragAnchor (const double nDragDistance) = 0;
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
        const std::shared_ptr<PresenterPaintManager>& rpPaintManager,
        const ::std::function<void (double)>& rThumbMotionListener);

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

private:
    class MousePressRepeater;
    std::shared_ptr<MousePressRepeater> mpMousePressRepeater;
    SharedBitmapDescriptor mpBackgroundBitmap;
    std::unique_ptr<PresenterCanvasHelper> mpCanvasHelper;

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
        const std::shared_ptr<PresenterPaintManager>& rpPaintManager,
        const ::std::function<void (double)>& rThumbMotionListener);
    virtual ~PresenterVerticalScrollBar();
    virtual sal_Int32 GetSize() const override;

protected:
    virtual double GetDragDistance (const sal_Int32 nX, const sal_Int32 nY) const override;
    virtual void UpdateDragAnchor (const double nDragDistance) override;
    virtual double GetMinor (const double nX, const double nY) const override;
    virtual void UpdateBorders() override;
    virtual void UpdateBitmaps() override;
    virtual void PaintComposite(
        const css::awt::Rectangle& rRepaintBox,
        const Area eArea,
        const SharedBitmapDescriptor& rpStartBitmaps,
        const SharedBitmapDescriptor& rpCenterBitmaps,
        const SharedBitmapDescriptor& rpEndBitmaps) override;

private:
    sal_Int32 mnScrollBarWidth;
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
