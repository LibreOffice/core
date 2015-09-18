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
    ::com::sun::star::drawing::framework::XPane,
    ::com::sun::star::drawing::framework::XPane2,
      ::com::sun::star::lang::XUnoTunnel
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
        const ::com::sun::star::uno::Reference<
            com::sun::star::drawing::framework::XResourceId>& rxPaneId,
        vcl::Window* pWindow)
        throw ();
    virtual ~Pane();

    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();

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
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::rendering::XCanvas>
        SAL_CALL getCanvas()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //----- XPane2 -------------------------------------------------------------

    virtual sal_Bool SAL_CALL isVisible()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setVisible (sal_Bool bIsVisible)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual css::uno::Reference<css::accessibility::XAccessible> SAL_CALL getAccessible()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL setAccessible (
        const css::uno::Reference<css::accessibility::XAccessible>& rxAccessible)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //----- XResource ---------------------------------------------------------

    virtual ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>
        SAL_CALL getResourceId()
        throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    /** For the typical pane it makes no sense to be dislayed without a
        view.  Therefore this default implementation returns always <TRUE/>.
    */
    virtual sal_Bool SAL_CALL isAnchorOnly()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    //----- XUnoTunnel --------------------------------------------------------

    virtual sal_Int64 SAL_CALL getSomething (const com::sun::star::uno::Sequence<sal_Int8>& rId)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId> mxPaneId;
    VclPtr<vcl::Window> mpWindow;
    ::com::sun::star::uno::Reference<com::sun::star::awt::XWindow> mxWindow;
    ::com::sun::star::uno::Reference<com::sun::star::rendering::XCanvas> mxCanvas;

    /** Override this method, not getCanvas(), when you want to provide a
        different canvas.
    */
    virtual ::com::sun::star::uno::Reference<com::sun::star::rendering::XCanvas>
        CreateCanvas()
        throw (::com::sun::star::uno::RuntimeException);

    /** Throw DisposedException when the object has already been disposed or
        is currently being disposed.  Otherwise this method returns
        normally.
    */
    void ThrowIfDisposed() const
        throw (::com::sun::star::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
