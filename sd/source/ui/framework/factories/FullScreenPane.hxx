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

#include "FrameWindowPane.hxx"
#include <tools/link.hxx>
#include <vcl/wrkwin.hxx>

class VclWindowEvent;

namespace vcl { class Window; }
namespace com::sun::star::uno { class XComponentContext; }

namespace sd::framework {

/** The full screen pane creates a pane that covers the complete application
    window, i.e. that hides menu bar, tool bars, status bars.
*/
class FullScreenPane
    : public FrameWindowPane
{
public:
    /** Create a new full screen pane.
        @param rxComponentContext
            Used for creating a new canvas.
        @param rxPaneId
            The resource id of the new pane.
        @param pViewShellWindow
            The top-level parent of this window is used to obtain title and
            icon for the new top-level window.
    */
    FullScreenPane (
        const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const vcl::Window* pViewShellWindow);
    virtual ~FullScreenPane() noexcept override;

    virtual void SAL_CALL disposing() override;

    //----- XPane -------------------------------------------------------------

    virtual sal_Bool SAL_CALL isVisible() override;

    virtual void SAL_CALL setVisible (sal_Bool bIsVisible) override;

    virtual css::uno::Reference<css::accessibility::XAccessible> SAL_CALL getAccessible() override;

    virtual void SAL_CALL setAccessible (
        const css::uno::Reference<css::accessibility::XAccessible>& rxAccessible) override;

    DECL_LINK(WindowEventHandler, VclWindowEvent&, void);

protected:
    virtual css::uno::Reference<css::rendering::XCanvas>
        CreateCanvas() override;

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    VclPtr<WorkWindow> mpWorkWindow;

    static void ExtractArguments (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        sal_Int32& rnScreenNumberReturnValue);
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
