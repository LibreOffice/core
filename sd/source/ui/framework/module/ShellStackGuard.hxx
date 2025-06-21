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

#include <framework/ConfigurationController.hxx>

#include <framework/ConfigurationChangeListener.hxx>

#include <rtl/ref.hxx>
#include <vcl/idle.hxx>
#include <comphelper/compbase.hxx>
#include <memory>

namespace sd
{
class DrawController;
class ViewShellBase;
}

namespace sd::framework
{
class ConfigurationController;

/** This module locks updates of the current configuration in situations
    when the shell stack must not be modified.

    On every start of a configuration update the ShellStackGuard checks the
    printer.  If it is printing the configuration update is locked.  It then
    polls the printer and unlocks updates when printing finishes.

    When in the future there are no resources left that use shells then this
    module can be removed.
*/
class ShellStackGuard : public sd::framework::ConfigurationChangeListener
{
public:
    explicit ShellStackGuard(rtl::Reference<sd::DrawController> const& rxController);
    virtual ~ShellStackGuard() override;

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // ConfigurationChangeListener

    virtual void
    notifyConfigurationChange(const sd::framework::ConfigurationChangeEvent& rEvent) override;

    // XEventListener

    virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent) override;

private:
    rtl::Reference<ConfigurationController> mxConfigurationController;
    ViewShellBase* mpBase;
    std::unique_ptr<ConfigurationController::Lock> mpUpdateLock;
    Idle maPrinterPollingIdle;

    DECL_LINK(TimeoutHandler, Timer*, void);

    /** Return <TRUE/> when the printer is printing.  Return <FALSE/> when
        the printer is not printing, or there is no printer, or something
        else went wrong.
    */
    bool IsPrinting() const;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
