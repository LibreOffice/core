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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANE_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANE_HXX

#include "PresenterPaneBase.hxx"
#include <com/sun/star/awt/XMouseListener.hpp>
#include <com/sun/star/awt/XMouseMotionListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XPaneBorderPainter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <rtl/ref.hxx>

namespace sdext { namespace presenter {

/** Pane used by the presenter screen.  Pane objects are stored in the
    PresenterPaneContainer.  Sizes and positions are controlled
    by the PresenterWindowManager.  Interactive positioning and resizing is
    managed by the PresenterPaneBorderManager.  Borders around panes are
    painted by the PresenterPaneBorderPainter.
*/
class PresenterPane : public PresenterPaneBase
{
public:
    PresenterPane (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterPane();

    // XPane

    css::uno::Reference<css::awt::XWindow> SAL_CALL getWindow()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Reference<css::rendering::XCanvas> SAL_CALL getCanvas()
        throw (css::uno::RuntimeException, std::exception) override;

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

private:
    /** Store the bounding box so that when the window is resized or moved
        we still know the old position and size.
    */
    css::awt::Rectangle maBoundingBox;

    virtual void CreateCanvases (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxParentCanvas) override;

    void Invalidate (
        const css::awt::Rectangle& rRepaintBox);
    void UpdateBoundingBox();
};

} } // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
