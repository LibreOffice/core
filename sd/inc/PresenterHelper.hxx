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
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <comphelper/compbase.hxx>

namespace com::sun::star::uno { class XComponentContext; }

namespace sd::presenter {

class SD_DLLPUBLIC PresenterHelper final
    : public comphelper::WeakComponentImplHelper<css::lang::XInitialization>
{
public:
    explicit PresenterHelper (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterHelper() override;
    PresenterHelper(const PresenterHelper&) = delete;
    PresenterHelper& operator=(const PresenterHelper&) = delete;

    // XInitialize

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments) override;

    /** Create a new window as child window of the given parent window.
        @param xParentWindow
            The parent window of the new window.
        @param bCreateSystemChildWindow
            When `true` then the new window will be a system window that,
            in the context of the presenter screen, can not be painted over
            by other windows that lie behind it.
        @param bInitiallyVisible
            When `true` the new window will be visible from the start,
            i.e. a window listener will not receive a windowShown signal.
        @param bEnableChildTransparentMode
            When `true` the parent window is painted behind its child
            windows. This is one half of allowing child windows to be
            transparent.
        @param bEnableParentClip
            When `true` then the parent window is not clipped where its
            child windows are painted.  This is the other half of allowing
            child windows to be transparent.
    */
    static css::uno::Reference<css::awt::XWindow> createWindow (
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow,
        bool bCreateSystemChildWindow,
        bool bInitiallyVisible,
        bool bEnableChildTransparentMode,
        bool bEnableParentClip);

    /** Create a new canvas for the given window.  The new canvas is a
        wrapper around the given shared canvas.  The wrapper only modifies
        the origin in all output and clipping methods.
        @param xUpdateCanvas
            This canvas is used to call updateScreen() on.  May be `NULL`
        @param xUpdateWindow
            The window that belongs to the update canvas.  May also be
            `NULL` (is expected to b `NULL` whenever xUpdateCanvas is.)
        @param xSharedCanvas
            The canvas that is shared by the wrapper.
        @param xSharedWindow
            The window of the shared canvas.  This is used to determine the
            proper offset.
        @param xWindow
            The canvas is created for this window.  Must not be `NULL`
    */
    static css::uno::Reference<css::rendering::XCanvas> createSharedCanvas (
        const css::uno::Reference<css::rendering::XSpriteCanvas>& rxUpdateCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxUpdateWindow,
        const css::uno::Reference<css::rendering::XCanvas>& rxSharedCanvas,
        const css::uno::Reference<css::awt::XWindow>& rxSharedWindow,
        const css::uno::Reference<css::awt::XWindow>& rxWindow);

    /** Create a new canvas for the given window.
        @param xWindow
            The canvas is created for this window.  Must not be `NULL`
        @param nRequestedCanvasFeatureList
            List of requested features that the new canvas should (has to)
            provide.  Use only values from the CanvasFeature
            constants group.
        @param sOptionalCanvasServiceName
            When an explicit service name is given then a new object of this
            service is created.  This service name lets the caller select a
            specific canvas implementation, e.g. with or without hardware
            acceleration.
    */
    virtual css::uno::Reference<css::rendering::XCanvas> createCanvas (
        const css::uno::Reference<css::awt::XWindow>& rxWindow,
        sal_Int16 nRequestedCanvasFeatures,
        const OUString& rsOptionalCanvasServiceName);

    /** Move the specified window to the top of its stacking order.  As a
        result the window will be painted over all its overlapping
        siblings.
        @param xWindow
            This window will be moved to the top of its stacking order.
    */
    static void toTop(const css::uno::Reference<css::awt::XWindow>& rxWindow);

    /** Load a bitmap with a given ID.
        @param sId
            The ID of the bitmap.
        @param xCanvas
            The bitmap is created to be compatible, and possibly optimized,
            for this canvas.
    */
    virtual css::uno::Reference<css::rendering::XBitmap> loadBitmap (
        std::u16string_view sId,
        const css::uno::Reference<css::rendering::XCanvas>& rxCanvas);

    /** Capture the mouse so that no other window will receive mouse events.
        Note that this is a potentially dangerous method.  Not calling
        releaseMouse eventually can lead to an unresponsive application.
        @param xWindow
            The window for which mouse events will be notified even when the
            mouse pointer moves outside the window or over other windows.
    */
    virtual void captureMouse (const css::uno::Reference<css::awt::XWindow>& rxWindow);

    /** Release a previously captured mouse.
        @param xWindow
            The window from which the mouse will be released.
    */
    virtual void releaseMouse (const css::uno::Reference<css::awt::XWindow>& rxWindow);

    /** Return the bounding box of the given child window relative to the
        direct or indirect parent window.
    */
    virtual css::awt::Rectangle getWindowExtentsRelative (
        const css::uno::Reference<css::awt::XWindow>& rxChildWindow,
        const css::uno::Reference<css::awt::XWindow>& rxParentWindow);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
};

} // end of namespace ::sd::presenter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
