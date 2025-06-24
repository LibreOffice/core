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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANEBASE_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANEBASE_HXX

#include "PresenterPaneBorderPainter.hxx"

#include <PresenterHelper.hxx>

#include <cppuhelper/compbase.hxx>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <framework/AbstractPane.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <rtl/ref.hxx>


namespace sdext::presenter {

class PresenterController;

typedef ::cppu::ImplInheritanceHelper<sd::framework::AbstractPane, css::awt::XWindowListener,
                                        css::awt::XPaintListener>
    PresenterPaneBaseInterfaceBase;

/** Base class of the panes used by the presenter screen.  Pane objects are
    stored in the PresenterPaneContainer.  Sizes and positions are
    controlled by the PresenterWindowManager.  Interactive positioning and
    resizing is managed by the PresenterPaneBorderManager.  Borders around
    panes are painted by the PresenterPaneBorderPainter.
*/
class PresenterPaneBase
    : public PresenterPaneBaseInterfaceBase
{
public:
    PresenterPaneBase (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        ::rtl::Reference<PresenterController> xPresenterController);
    virtual ~PresenterPaneBase() override;
    PresenterPaneBase(const PresenterPaneBase&) = delete;
    PresenterPaneBase& operator=(const PresenterPaneBase&) = delete;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    const css::uno::Reference<css::awt::XWindow>& GetBorderWindow() const;
    void SetTitle (const OUString& rsTitle);
    const rtl::Reference<PresenterPaneBorderPainter>& GetPaneBorderPainter() const;

    void initialize(const rtl::Reference<sd::framework::ResourceId>& rxPaneId,
                    const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
                    const css::uno::Reference<css::rendering::XCanvas>& rxParentCanvas,
                    const rtl::Reference<PresenterPaneBorderPainter>& rxBorderPainter,
                    bool bIsWindowVisibleOnCreation);

    // AbstractResourceI

    virtual rtl::Reference<sd::framework::ResourceId> getResourceId() override;

    virtual bool isAnchorOnly() override;

    // XWindowListener

    virtual void SAL_CALL windowResized (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowMoved (const css::awt::WindowEvent& rEvent) override;

    virtual void SAL_CALL windowShown (const css::lang::EventObject& rEvent) override;

    virtual void SAL_CALL windowHidden (const css::lang::EventObject& rEvent) override;

    // lang::XEventListener

    virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent) override;

protected:
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    css::uno::Reference<css::awt::XWindow> mxBorderWindow;
    css::uno::Reference<css::rendering::XCanvas> mxBorderCanvas;
    css::uno::Reference<css::awt::XWindow> mxContentWindow;
    css::uno::Reference<css::rendering::XCanvas> mxContentCanvas;
    rtl::Reference<sd::framework::ResourceId> mxPaneId;
    rtl::Reference<PresenterPaneBorderPainter> mxBorderPainter;
    OUString msTitle;
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;

    virtual void CreateCanvases (
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxParentCanvas) = 0;

    void CreateWindows (
        const bool bIsWindowVisibleOnCreation);
    void PaintBorder (const css::awt::Rectangle& rUpdateRectangle);
    void ToTop();
    void LayoutContextWindow();
};

} // end of namespace ::sd::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
