/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SDEXT_PRESENTER_PRESENTER_PAINT_MANAGER_HXX
#define SDEXT_PRESENTER_PRESENTER_PAINT_MANAGER_HXX

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <rtl/ref.hxx>
#include <boost/function.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterPaneContainer;

/** Synchronize painting of windows and canvases.  At the moment there is
    just some processing of invalidate calls.
    This could be extended to process incoming windowPaint() calls.
*/
class PresenterPaintManager
{
public:
    /** Create paint manager with the window that is the top node in the
        local window hierarchy.
    */
    PresenterPaintManager (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        const css::uno::Reference<css::drawing::XPresenterHelper>& rxPresenterHelper,
        const rtl::Reference<PresenterPaneContainer>& rpPaneContainer);

    ::boost::function<void(const css::awt::Rectangle& rRepaintBox)>
        GetInvalidator (
            const css::uno::Reference<css::awt::XWindow>& rxWindow,
            const bool bSynchronous = false);

    /** Request a repaint of the whole window.
        @param rxWindow
            May be the parent window or one of its descendents.
    */
    void Invalidate (
        const css::uno::Reference<css::awt::XWindow>& rxWindow,
        const bool bSynchronous = false);
    void Invalidate (
        const css::uno::Reference<css::awt::XWindow>& rxWindow,
        const sal_Int16 nInvalidateFlags);

    /** Request a repaint of a part of a window.
        @param rxWindow
            May be the parent window or one of its descendents.
    */
    void Invalidate (
        const css::uno::Reference<css::awt::XWindow>& rxWindow,
        const css::awt::Rectangle& rRepaintBox,
        const bool bSynchronous = false);
    void Invalidate (
        const css::uno::Reference<css::awt::XWindow>& rxWindow,
        const css::awt::Rectangle& rRepaintBox,
        const sal_Int16 nInvalidateFlags);

private:
    css::uno::Reference<css::awt::XWindow> mxParentWindow;
    css::uno::Reference<css::awt::XWindowPeer> mxParentWindowPeer;
    css::uno::Reference<css::drawing::XPresenterHelper> mxPresenterHelper;
    ::rtl::Reference<PresenterPaneContainer> mpPaneContainer;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
