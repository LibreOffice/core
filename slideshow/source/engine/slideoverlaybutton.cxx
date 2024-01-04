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

#include <sal/config.h>

#include <eventmultiplexer.hxx>
#include "slideoverlaybutton.hxx"

#include <canvas/canvastools.hxx>
#include <cppcanvas/customsprite.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>

#include <algorithm>
#include <utility>

namespace slideshow::internal
{
SlideOverlayButtonSharedPtr
SlideOverlayButton::create(const css::uno::Reference<css::rendering::XBitmap>& xIconBitmap,
                           const css::awt::Point& rPosition,
                           const std::function<void(basegfx::B2DPoint)>& clickHandler,
                           ScreenUpdater& rScreenUpdater, EventMultiplexer& rEventMultiplexer,
                           const UnoViewContainer& rViewContainer)
{
    SlideOverlayButtonSharedPtr pRet(new SlideOverlayButton(
        xIconBitmap, rPosition, clickHandler, rScreenUpdater, rEventMultiplexer, rViewContainer));

    rEventMultiplexer.addViewHandler(pRet);
    // Set priority to 1000 so that the handler fires before the click handler on the slide
    rEventMultiplexer.addClickHandler(pRet, 1000);
    return pRet;
}

SlideOverlayButton::SlideOverlayButton(css::uno::Reference<css::rendering::XBitmap> xIconBitmap,
                                       css::awt::Point pPosition,
                                       std::function<void(basegfx::B2DPoint)> clickHandler,
                                       ScreenUpdater& rScreenUpdater,
                                       EventMultiplexer& rEventMultiplexer,
                                       const UnoViewContainer& rViewContainer)
    : mxIconBitmap(std::move(xIconBitmap))
    , mrEventMultiplexer(rEventMultiplexer)
    , mpPosition(std::move(pPosition))
    , mClickHandler(std::move(clickHandler))
    , mrScreenUpdater(rScreenUpdater)
{
    for (const auto& pView : rViewContainer)
        viewAdded(pView);
}

void SlideOverlayButton::setVisible(const bool bVisible)
{
    if (mbVisible == bVisible)
        return;

    mbVisible = bVisible;

    for (const auto& rView : maViews)
        if (rView.second)
            rView.second->show();

    // sprites changed, need a screen update for this frame.
    mrScreenUpdater.requestImmediateUpdate();
}

css::geometry::IntegerSize2D SlideOverlayButton::getSize() const { return mxIconBitmap->getSize(); }

basegfx::B2DPoint SlideOverlayButton::calcSpritePos(UnoViewSharedPtr const& rView) const
{
    const css::awt::Rectangle aViewArea(rView->getUnoView()->getCanvasArea());
    return basegfx::B2DPoint(
        aViewArea.X + std::min(aViewArea.Width, mpPosition.X),
        aViewArea.Y
            + std::max(sal_Int32(0),
                       aViewArea.Height - mxIconBitmap->getSize().Height - mpPosition.Y));
}

void SlideOverlayButton::viewAdded(const UnoViewSharedPtr& rView)
{
    cppcanvas::CustomSpriteSharedPtr sprite;

    try
    {
        const css::geometry::IntegerSize2D spriteSize(mxIconBitmap->getSize());
        sprite = rView->createSprite(basegfx::B2DSize(spriteSize.Width, spriteSize.Height),
                                     1000.0); // sprite should be in front of all
        // other sprites
        css::rendering::ViewState viewState;
        canvas::tools::initViewState(viewState);
        css::rendering::RenderState renderState;
        canvas::tools::initRenderState(renderState);
        sprite->getContentCanvas()->getUNOCanvas()->drawBitmap(mxIconBitmap, viewState,
                                                               renderState);

        sprite->setAlpha(0.9);
        sprite->movePixel(calcSpritePos(rView));
        sprite->show();
    }
    catch (css::uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("slideshow", "");
    }

    maViews.emplace_back(rView, sprite);
}

void SlideOverlayButton::viewRemoved(const UnoViewSharedPtr& rView)
{
    std::erase_if(maViews, [&rView](const ViewsVecT::value_type& cp) { return rView == cp.first; });
}

void SlideOverlayButton::viewChanged(const UnoViewSharedPtr& rView)
{
    // find entry corresponding to modified view
    ViewsVecT::iterator aModifiedEntry(
        std::find_if(maViews.begin(), maViews.end(),
                     [&rView](const ViewsVecT::value_type& cp) { return rView == cp.first; }));

    assert(aModifiedEntry != maViews.end());
    if (aModifiedEntry == maViews.end())
        return;

    if (aModifiedEntry->second)
        aModifiedEntry->second->movePixel(calcSpritePos(aModifiedEntry->first));
}

void SlideOverlayButton::viewsChanged()
{
    // reposition sprites on all views
    for (const auto& rView : maViews)
    {
        if (rView.second)
            rView.second->movePixel(calcSpritePos(rView.first));
    }
}

bool SlideOverlayButton::handleMousePressed(const css::awt::MouseEvent& /*e*/) { return false; }

bool SlideOverlayButton::handleMouseReleased(const css::awt::MouseEvent& e)
{
    css::uno::Reference<css::presentation::XSlideShowView> view(e.Source,
                                                                css::uno::UNO_QUERY_THROW);
    const basegfx::B2DPoint btnPnt(
        view->getCanvasArea().X + std::min(view->getCanvasArea().Width, mpPosition.X),
        view->getCanvasArea().Y
            + std::max(sal_Int32(0), view->getCanvasArea().Height - mxIconBitmap->getSize().Height
                                         - mpPosition.Y));
    const basegfx::B2DPoint clickPnt
        = mrEventMultiplexer.toNormalPoint(e.Source, basegfx::B2DPoint(e.X, e.Y));
    if (clickPnt.getX() > btnPnt.getX()
        && clickPnt.getX() < btnPnt.getX() + mxIconBitmap->getSize().Width
        && clickPnt.getY() > btnPnt.getY()
        && clickPnt.getY() < btnPnt.getY() + mxIconBitmap->getSize().Height)
    {
        if (mnIgnoreClicksCnt == 0)
        {
            mnIgnoreClicksCnt = e.ClickCount - 1;
            mClickHandler(clickPnt);
        }
        else
        {
            mnIgnoreClicksCnt--;
        }
        return true;
    }
    return false;
}

bool SlideOverlayButton::handleMouseDragged(const css::awt::MouseEvent& /*e*/) { return false; }

bool SlideOverlayButton::handleMouseMoved(const css::awt::MouseEvent& /*e*/) { return false; }

} // namespace slideshow::internal

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
