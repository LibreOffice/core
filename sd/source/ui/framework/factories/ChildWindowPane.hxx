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

#include <framework/Pane.hxx>

#include <com/sun/star/lang/XEventListener.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <memory>

class SfxShell;

namespace sd { class ViewShellBase; }
namespace com::sun::star::awt { class XWindow; }
namespace com::sun::star::drawing::framework { class XResourceId; }

namespace sd::framework {

typedef ::cppu::ImplInheritanceHelper <
    ::sd::framework::Pane,
    css::lang::XEventListener
    > ChildWindowPaneInterfaceBase;

/** The ChildWindowPane listens to the child window and disposes itself when
    the child window becomes inaccessible.  This happens for instance when a
    document is made read-only and the task pane is turned off.
*/
class ChildWindowPane
    : public ChildWindowPaneInterfaceBase
{
public:
    ChildWindowPane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        sal_uInt16 nChildWindowId,
        ViewShellBase& rViewShellBase,
        ::std::unique_ptr<SfxShell> && pShell);
    virtual ~ChildWindowPane() override;

    /** Hide the pane.  To make the pane visible again, call GetWindow().
    */
    void Hide();

    virtual void SAL_CALL disposing() override;

    /** This returns the content window when the child window is already
        visible.  Otherwise <NULL/> is returned.  In that case a later call
        may return the requested window (making a child window visible is an
        asynchronous process.)
        Note that GetWindow() may return different Window pointers when
        Hide() is called in between.
    */
    virtual vcl::Window* GetWindow() override;

    /** The local getWindow() first calls GetWindow() to provide a valid
        window pointer before forwarding the call to the base class.
    */
    virtual css::uno::Reference<css::awt::XWindow>
        SAL_CALL getWindow() override;

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XEventListener

    virtual void SAL_CALL disposing(
        const css::lang::EventObject& rEvent) override;

private:
    sal_uInt16 mnChildWindowId;
    ViewShellBase& mrViewShellBase;
    ::std::unique_ptr<SfxShell> mpShell;

    /** This flag is set when the pane shell has been activated at least
        once.  It is used to optimize the start-up performance (by not
        showing the window too early) and by not delaying its creation at
        later times.
    */
    bool mbHasBeenActivated;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
