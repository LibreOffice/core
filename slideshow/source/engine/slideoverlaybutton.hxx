/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#pragma once

#include <sal/config.h>

#include <eventmultiplexer.hxx>
#include <mouseeventhandler.hxx>
#include <screenupdater.hxx>
#include <unoview.hxx>
#include <vieweventhandler.hxx>

#include <basegfx/point/b2dpoint.hxx>
#include <cppcanvas/sprite.hxx>

#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/awt/Point.hpp>

#include <functional>
#include <memory>
#include <vector>

namespace slideshow::internal
{
class EventMultiplexer;
typedef std::shared_ptr<class SlideOverlayButton> SlideOverlayButtonSharedPtr;

class SlideOverlayButton : public ViewEventHandler, public MouseEventHandler
{
public:
    SlideOverlayButton(const SlideOverlayButton&) = delete;
    SlideOverlayButton& operator=(const SlideOverlayButton&) = delete;

    static SlideOverlayButtonSharedPtr
    create(const css::uno::Reference<css::rendering::XBitmap>& xIconBitmap,
           const css::awt::Point& rPosition,
           const std::function<void(basegfx::B2DPoint)>& clickHandler,
           ScreenUpdater& rScreenUpdater, EventMultiplexer& rEventMultiplexer,
           const UnoViewContainer& rViewContainer);

    /** Shows button icon.
     */
    void show() { setVisible(true); }

    /** Hides button icon.
     */
    void hide() { setVisible(false); }

    css::geometry::IntegerSize2D getSize() const;
    basegfx::B2DPoint calcSpritePos(UnoViewSharedPtr const& rView) const;

private:
    SlideOverlayButton(css::uno::Reference<css::rendering::XBitmap> xIconBitmap,
                       css::awt::Point pPosition,
                       std::function<void(basegfx::B2DPoint)> clickHandler,
                       ScreenUpdater& rScreenUpdater, EventMultiplexer& rEventMultiplexer,
                       const UnoViewContainer& rViewContainer);

    // ViewEventHandler
    virtual void viewAdded(const UnoViewSharedPtr& rView) override;
    virtual void viewRemoved(const UnoViewSharedPtr& rView) override;
    virtual void viewChanged(const UnoViewSharedPtr& rView) override;
    virtual void viewsChanged() override;

    // MouseEventHandler
    virtual bool handleMousePressed(const css::awt::MouseEvent& e) override;
    virtual bool handleMouseReleased(const css::awt::MouseEvent& e) override;
    virtual bool handleMouseDragged(const css::awt::MouseEvent& e) override;
    virtual bool handleMouseMoved(const css::awt::MouseEvent& e) override;

    void setVisible(const bool bVisible);

    typedef std::vector<std::pair<UnoViewSharedPtr, cppcanvas::CustomSpriteSharedPtr>> ViewsVecT;

    css::uno::Reference<css::rendering::XBitmap> mxIconBitmap;
    EventMultiplexer& mrEventMultiplexer;
    css::awt::Point mpPosition;
    std::function<void(basegfx::B2DPoint)> mClickHandler;

    ViewsVecT maViews;
    ScreenUpdater& mrScreenUpdater;
    bool mbVisible = false;
    sal_Int32 mnIgnoreClicksCnt = 0;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
