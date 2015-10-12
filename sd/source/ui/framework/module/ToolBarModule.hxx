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

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_MODULE_TOOLBARMODULE_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_MODULE_TOOLBARMODULE_HXX

#include "ToolBarManager.hxx"
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XTabBar.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <memory>

namespace sd {
class ViewShellBase;
}

namespace sd { namespace framework {

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::drawing::framework::XConfigurationChangeListener
        > ToolBarModuleInterfaceBase;
}

/** This module is responsible for locking the ToolBarManager during
    configuration updates and for triggering ToolBarManager updates.
*/
class ToolBarModule
    : private ::cppu::BaseMutex,
      public ToolBarModuleInterfaceBase
{
public:
    /** Create a new module.
        @param rxController
            This is the access point to the drawing framework.
    */
    ToolBarModule (
        const css::uno::Reference<css::frame::XController>& rxController);
    virtual ~ToolBarModule();

    virtual void SAL_CALL disposing() override;

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference<
        css::drawing::framework::XConfigurationController> mxConfigurationController;
    ViewShellBase* mpBase;
    std::unique_ptr<ToolBarManager::UpdateLock> mpToolBarManagerLock;
    bool mbMainViewSwitchUpdatePending;

    void HandleUpdateStart();
    void HandleUpdateEnd();
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
