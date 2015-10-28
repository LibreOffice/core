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

#ifndef INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_PANE_HXX
#define INCLUDED_SD_SOURCE_UI_INC_FRAMEWORK_PANE_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XPane2.hpp>
#include <com/sun/star/drawing/framework/TabBarButton.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/compbase.hxx>
#include <vcl/vclptr.hxx>
#include <memory>

namespace vcl { class Window; }


namespace {

typedef ::cppu::WeakComponentImplHelper <
      css::drawing::framework::XPane,
      css::drawing::framework::XPane2,
      css::lang::XUnoTunnel
    > PaneInterfaceBase;

} // end of anonymous namespace.

namespace sd { namespace framework {

/** A pane is a wrapper for a window and possibly for a tab bar (for view
    switching).  Panes are unique resources.

    This class has two responsibilities:
    1. It implements the XPane interface.  This is the most important
    interface of this class for API based views (of which there not that
    many yet) because it gives access to the XWindow.
    2. It gives access to the underlying VCL Window by implementing the
    XUnoTunnel interface.  This is necessary at the moment and in the
    foreseeable future because many parts of the Draw and Impress views rely
    on direct access on the Window class.
*/
class Pane
    : protected MutexOwner,
      public PaneInterfaceBase
{
public:
    /** Create a new Pane object that wrapps the given window.
        @param rsPaneURL
            The URL that is used by the configuration to identify the pane.
            The given URL has to be valid.
        @param pWindow
            The VCL Window (usually this really is an sd::Window) that is
            wrapped by the new Pane object.  The given pointer must not be
            NULL.
    */
    Pane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        vcl::Window* pWindow)
        throw ();
    virtual ~Pane();

    virtual void SAL_CALL disposing() override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();

    /** This method is typically used together with the XUnoTunnel to obtain
        a Window pointer from an XPane object.
    */
    virtual vcl::Window* GetWindow();

    //----- XPane -------------------------------------------------------------

    /** For a UNO API based implementation of a view this may the most
        important method of this class because the view is only interested
        in the window of the pane.
    */
    virtual css::uno::Reference<css::awt::XWindow>
        SAL_CALL getWindow()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::rendering::XCanvas>
        SAL_CALL getCanvas()
        throw (css::uno::RuntimeException, std::exception) override;

    //----- XPane2 -------------------------------------------------------------

    virtual sal_Bool SAL_CALL isVisible()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setVisible (sal_Bool bIsVisible)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::accessibility::XAccessible> SAL_CALL getAccessible()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setAccessible (
        const css::uno::Reference<css::accessibility::XAccessible>& rxAccessible)
        throw (css::uno::RuntimeException, std::exception) override;

    //----- XResource ---------------------------------------------------------

    virtual css::uno::Reference<css::drawing::framework::XResourceId>
        SAL_CALL getResourceId()
        throw (css::uno::RuntimeException, std::exception) override;

    /** For the typical pane it makes no sense to be displayed without a
        view.  Therefore this default implementation returns always <TRUE/>.
    */
    virtual sal_Bool SAL_CALL isAnchorOnly()
        throw (css::uno::RuntimeException, std::exception) override;

    //----- XUnoTunnel --------------------------------------------------------

    virtual sal_Int64 SAL_CALL getSomething (const css::uno::Sequence<sal_Int8>& rId)
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    css::uno::Reference<css::drawing::framework::XResourceId> mxPaneId;
    VclPtr<vcl::Window> mpWindow;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;

    /** Override this method, not getCanvas(), when you want to provide a
        different canvas.
    */
    virtual css::uno::Reference<css::rendering::XCanvas>
        CreateCanvas()
        throw (css::uno::RuntimeException);

    /** Throw DisposedException when the object has already been disposed or
        is currently being disposed.  Otherwise this method returns
        normally.
    */
    void ThrowIfDisposed() const
        throw (css::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
