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
#include <cppuhelper/compbase3.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class Window;

namespace cssu = ::com::sun::star::uno;

namespace {

typedef ::cppu::WeakComponentImplHelper3 <
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
        ::Window* pWindow)
        throw ();
    virtual ~Pane (void);

    virtual void SAL_CALL disposing (void);

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId (void);

    /** This method is typically used together with the XUnoTunnel to obtain
        a Window pointer from an XPane object.
    */
    virtual ::Window* GetWindow (void);

    void SetWindow (::Window* pWindow);

    //----- XPane -------------------------------------------------------------

    /** For a UNO API based implementation of a view this may the most
        important method of this class because the view is only interested
        in the window of the pane.
    */
    virtual cssu::Reference<css::awt::XWindow>
        SAL_CALL getWindow (void)
        throw (cssu::RuntimeException, std::exception);

    virtual cssu::Reference<css::rendering::XCanvas>
        SAL_CALL getCanvas (void)
        throw (cssu::RuntimeException, std::exception);


    //----- XPane2 -------------------------------------------------------------

    virtual sal_Bool SAL_CALL isVisible (void)
        throw (cssu::RuntimeException, std::exception);

    virtual void SAL_CALL setVisible (sal_Bool bIsVisible)
        throw (cssu::RuntimeException, std::exception);

    virtual cssu::Reference<css::accessibility::XAccessible> SAL_CALL getAccessible (void)
        throw (cssu::RuntimeException, std::exception);

    virtual void SAL_CALL setAccessible (
        const cssu::Reference<css::accessibility::XAccessible>& rxAccessible)
        throw (cssu::RuntimeException, std::exception);


    //----- XResource ---------------------------------------------------------

    virtual ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId>
        SAL_CALL getResourceId (void)
        throw (::com::sun::star::uno::RuntimeException, std::exception);

    /** For the typical pane it makes no sense to be dislayed without a
        view.  Therefore this default implementation returns always <TRUE/>.
    */
    virtual sal_Bool SAL_CALL isAnchorOnly (void)
        throw (com::sun::star::uno::RuntimeException, std::exception);


    //----- XUnoTunnel --------------------------------------------------------

    virtual sal_Int64 SAL_CALL getSomething (const com::sun::star::uno::Sequence<sal_Int8>& rId)
        throw (com::sun::star::uno::RuntimeException, std::exception);


protected:
    ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResourceId> mxPaneId;
    ::Window* mpWindow;
    ::com::sun::star::uno::Reference<com::sun::star::awt::XWindow> mxWindow;
    ::com::sun::star::uno::Reference<com::sun::star::rendering::XCanvas> mxCanvas;

    /** Overload this method, not getCanvas(), when you want to provide a
        different canvas.
    */
    virtual ::com::sun::star::uno::Reference<com::sun::star::rendering::XCanvas>
        CreateCanvas (void)
        throw (::com::sun::star::uno::RuntimeException);

    /** Throw DisposedException when the object has already been disposed or
        is currently being disposed.  Otherwise this method returns
        normally.
    */
    void ThrowIfDisposed (void) const
        throw (::com::sun::star::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
