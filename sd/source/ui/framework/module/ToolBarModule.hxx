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

#include <ToolBarManager.hxx>
#include <tools/link.hxx>
#include <framework/ConfigurationChangeListener.hxx>
#include <com/sun/star/ui/XContextChangeEventListener.hpp>
#include <comphelper/compbase.hxx>
#include <o3tl/deleter.hxx>
#include <rtl/ref.hxx>
#include <memory>

namespace sd {
class DrawController;
class ViewShellBase;
}

namespace sd::tools
{
class EventMultiplexerEvent;
}

namespace sd::framework
{
class ConfigurationController;
class ResourceId;

/** This module is responsible for locking the ToolBarManager during
    configuration updates and for triggering ToolBarManager updates.
*/
class ToolBarModule final
    : public sd::framework::ConfigurationChangeListener
{
public:
    /** Create a new module.
        @param rxController
            This is the access point to the drawing framework.
    */
    explicit ToolBarModule (
        const rtl::Reference<sd::DrawController>& rxController);
    virtual ~ToolBarModule() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // ConfigurationChangeListener

    virtual void notifyConfigurationChange (
        const sd::framework::ConfigurationChangeEvent& rEvent) override;

    // XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent) override;

private:
    rtl::Reference<ConfigurationController> mxConfigurationController;
    ViewShellBase* mpBase;
    std::unique_ptr<ToolBarManager::UpdateLock, o3tl::default_delete<ToolBarManager::UpdateLock>> mpToolBarManagerLock;
    bool mbMainViewSwitchUpdatePending;
    bool mbListeningEventMultiplexer = false;

    /** Update toolbars via ToolbarManager

        @param pViewShell may be nullptr
    */
    void UpdateToolbars(const ViewShell* pViewShell);

    void HandleUpdateStart();
    void HandleUpdateEnd();
    void HandlePaneViewShellFocused(
        const rtl::Reference<sd::framework::ResourceId>& rxResourceId);

    DECL_LINK(EventMultiplexerListener, ::sd::tools::EventMultiplexerEvent&, void);
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
