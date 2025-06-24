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

#include <sddllapi.h>
#include <framework/AbstractPane.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>

namespace sd::framework {

/** A pane is a wrapper for a window and possibly for a tab bar (for view
    switching).  Panes are unique resources.

    This class has two responsibilities:
    1. It implements the AbstractPane interface.  This is the most important
    interface of this class for API based views (of which there not that
    many yet) because it gives access to the XWindow.
    2. It gives access to the underlying VCL Window.
    This is necessary at the moment and in the
    foreseeable future because many parts of the Draw and Impress views rely
    on direct access on the Window class.
*/
class SD_DLLPUBLIC Pane
    : public sd::framework::AbstractPane
{
public:
    /** Create a new Pane object that wraps the given window.
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
        noexcept;
    virtual ~Pane() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    /** This method is typically used to obtain
        a Window pointer from an AbstractPane object.
    */
    virtual vcl::Window* GetWindow();

    //----- AbstractPane -------------------------------------------------------------

    /** For a UNO API based implementation of a view this may the most
        important method of this class because the view is only interested
        in the window of the pane.
    */
    virtual css::uno::Reference<css::awt::XWindow> getWindow() override;

    virtual css::uno::Reference<css::rendering::XCanvas> getCanvas() override;

    /** Return whether all windows that are used to implement the pane are
        visible.
        @return `TRUE` when all windows of the pane are visible.
    */
    virtual bool isVisible();

    /** Hide or show the pane.  If there is more than one window used to
        implement the pane then it is left to the implementation if one,
        some, or all windows are hidden or shown as long as the pane becomes
        hidden or visible.
        @param bIsVisible
            When `TRUE` then show the pane.  Hide it otherwise.
    */
    virtual void setVisible (bool bIsVisible);

    //----- XResource ---------------------------------------------------------

    virtual css::uno::Reference<css::drawing::framework::XResourceId>
        SAL_CALL getResourceId() override;

    /** For the typical pane it makes no sense to be displayed without a
        view.  Therefore this default implementation returns always <TRUE/>.
    */
    virtual sal_Bool SAL_CALL isAnchorOnly() override;

protected:
    css::uno::Reference<css::drawing::framework::XResourceId> mxPaneId;
    VclPtr<vcl::Window> mpWindow;
    css::uno::Reference<css::awt::XWindow> mxWindow;
    css::uno::Reference<css::rendering::XCanvas> mxCanvas;

    /** Override this method, not getCanvas(), when you want to provide a
        different canvas.

        @throws css::uno::RuntimeException
    */
    virtual css::uno::Reference<css::rendering::XCanvas>
        CreateCanvas();

};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
