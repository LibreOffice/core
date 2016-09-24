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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPAINTMANAGER_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPAINTMANAGER_HXX

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <rtl/ref.hxx>

#include <functional>

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

    ::std::function<void (const css::awt::Rectangle& rRepaintBox)>
        GetInvalidator (
            const css::uno::Reference<css::awt::XWindow>& rxWindow);

    /** Request a repaint of the whole window.
        @param rxWindow
            May be the parent window or one of its descendents.
    */
    void Invalidate (
        const css::uno::Reference<css::awt::XWindow>& rxWindow);
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
