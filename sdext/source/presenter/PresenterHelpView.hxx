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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERHELPVIEW_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERHELPVIEW_HXX

#include "PresenterController.hxx"
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/awt/XPaintListener.hpp>
#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <memory>

namespace {
    typedef cppu::WeakComponentImplHelper<
        css::drawing::framework::XView,
        css::awt::XWindowListener,
        css::awt::XPaintListener
        > PresenterHelpViewInterfaceBase;
}

namespace sdext { namespace presenter {

class PresenterButton;

/** Show help text that describes the defined keys.
*/
class PresenterHelpView
    : private ::cppu::BaseMutex,
      public PresenterHelpViewInterfaceBase
{
public:
    explicit PresenterHelpView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterHelpView();

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // lang::XEventListener

    virtual void SAL_CALL
        disposing (const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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

    // XResourceId

    virtual css::uno::Reference<css::drawing::framework::XResourceId> SAL_CALL getResourceId()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL isAnchorOnly()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    class TextContainer;

    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewId;
    css::uno::Reference<css::drawing::framework::XPane> mxPane;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;
    ::rtl::Reference<PresenterController> mpPresenterController;
    PresenterTheme::SharedFontDescriptor mpFont;
    std::unique_ptr<TextContainer> mpTextContainer;
    ::rtl::Reference<PresenterButton> mpCloseButton;
    sal_Int32 mnSeparatorY;
    sal_Int32 mnMaximalWidth;

    void ProvideCanvas();
    void Resize();
    void Paint (const css::awt::Rectangle& rRedrawArea);
    void ReadHelpStrings();
    void ProcessString (
        const css::uno::Reference<css::beans::XPropertySet>& rsProperties);

    /** Find a font size, so that all text can be displayed at the same
        time.
    */
    void CheckFontSize();

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed()
        throw (css::lang::DisposedException);
};

} } // end of namespace ::sdext::presenter

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
