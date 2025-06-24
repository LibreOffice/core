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

#pragma once

#include "PresenterPaneBase.hxx"
#include "PresenterSprite.hxx"
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <rtl/ref.hxx>
#include <memory>

namespace sdext::presenter
{
/** Use a sprite to display the contents and the border of a pane.  Windows
    are still used to define the locations and sizes of both the border and
    the pane content.  Note that every resize results in a disposed canvas.
    Therefore call getCanvas in every repaint or at least after every resize.
*/
class PresenterSpritePane : public PresenterPaneBase
{
public:
    PresenterSpritePane(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
                        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterSpritePane() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    using PresenterPaneBase::disposing;

    // AbstractPane

    virtual css::uno::Reference<css::awt::XWindow> getWindow() override;

    virtual css::uno::Reference<css::rendering::XCanvas> getCanvas() override;

    // XWindowListener

    virtual void SAL_CALL windowResized(const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowMoved(const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowShown(const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL windowHidden(const css::lang::EventObject& rEvent) override;

    // XPaintListener

    virtual void SAL_CALL windowPaint(const css::awt::PaintEvent& rEvent) override;

private:
    css::uno::Reference<css::rendering::XSpriteCanvas> mxParentCanvas;
    std::shared_ptr<PresenterSprite> mpSprite;

    virtual void CreateCanvases(
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxParentCanvas) override;
    void UpdateCanvases();
};

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
