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

#include "sddllapi.h"

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <comphelper/compbase.hxx>

namespace com::sun::star::uno { class XComponentContext; }

namespace sd::presenter {

class SD_DLLPUBLIC PresenterHelper final
{
public:
    PresenterHelper() = delete;
    PresenterHelper(const PresenterHelper&) = delete;
    PresenterHelper& operator=(const PresenterHelper&) = delete;

    /** Create a new window as child window of the given parent window.
        @param rxParentWindow
            The parent window of the new window.
        @param bInitiallyVisible
            When `true` the new window will be visible from the start,
            i.e. a window listener will not receive a windowShown signal.
    */
    static css::uno::Reference<css::awt::XWindow> createWindow (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        bool bInitiallyVisible);

    /** Create a new canvas for the given window.  The new canvas is a
        wrapper around the given shared canvas.  The wrapper only modifies
        the origin in all output and clipping methods.
        @param rxUpdateCanvas
            This canvas is used to call updateScreen() on.  May be `NULL`
        @param rxUpdateWindow
            The window that belongs to the update canvas.  May also be
            `NULL` (is expected to b `NULL` whenever xUpdateCanvas is.)
        @param rxSharedCanvas
            The canvas that is shared by the wrapper.
        @param rxSharedWindow
            The window of the shared canvas.  This is used to determine the
            proper offset.
        @param rxWindow
            The canvas is created for this window.  Must not be `NULL`
    */
    static css::uno::Reference<css::rendering::XCanvas> createSharedCanvas (
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxUpdateCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxUpdateWindow,
        const css::uno::Reference<css::rendering::XCanvas>& rxSharedCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxSharedWindow,
        const css::uno::Reference<css::awt::XWindow>& rxWindow);

    /** Move the specified window to the top of its stacking order.  As a
        result the window will be painted over all its overlapping
        siblings.
        @param rxWindow
            This window will be moved to the top of its stacking order.
    */
    static void toTop(const css::uno::Reference<css::awt::XWindow>& rxWindow);

    /** Load a bitmap with a given ID.
        @param sId
            The ID of the bitmap.
        @param rxCanvas
            The bitmap is created to be compatible, and possibly optimized,
            for this canvas.
    */
    static css::uno::Reference<css::rendering::XBitmap> loadBitmap(
        std::u16string_view sId,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);

    /** Capture the mouse so that no other window will receive mouse events.
        Note that this is a potentially dangerous method.  Not calling
        releaseMouse eventually can lead to an unresponsive application.
        @param xWindow
            The window for which mouse events will be notified even when the
            mouse pointer moves outside the window or over other windows.
    */
    static void captureMouse(const css::uno::Reference<css::awt::XWindow>& rxWindow);

    /** Release a previously captured mouse.
        @param rxWindow
            The window from which the mouse will be released.
    */
    static void releaseMouse(const css::uno::Reference<css::awt::XWindow>& rxWindow);

    /** Return the bounding box of the given child window relative to the
        direct or indirect parent window.
    */
    static css::awt::Rectangle getWindowExtentsRelative(
        const css::uno::Reference<css::awt::XWindow>& rxChildWindow,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow);
};

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
