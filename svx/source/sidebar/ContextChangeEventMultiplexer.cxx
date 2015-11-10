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

#include "svx/sidebar/ContextChangeEventMultiplexer.hxx"

#include <com/sun/star/ui/ContextChangeEventObject.hpp>
#include <com/sun/star/ui/XContextChangeEventMultiplexer.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/viewsh.hxx>
#include <tools/diagnose_ex.h>

using namespace css;
using namespace css::uno;


void ContextChangeEventMultiplexer::NotifyContextChange (
    const css::uno::Reference<css::frame::XController>& rxController,
    const ::sfx2::sidebar::EnumContext::Context eContext)
{
    if (rxController.is() && rxController->getFrame().is())
    {
        const css::ui::ContextChangeEventObject aEvent(
            rxController,
            GetModuleName(rxController->getFrame()),
            ::sfx2::sidebar::EnumContext::GetContextName(eContext));

        css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
            css::ui::ContextChangeEventMultiplexer::get(
                ::comphelper::getProcessComponentContext()));
        if (xMultiplexer.is())
            xMultiplexer->broadcastContextChangeEvent(aEvent, rxController);
    }
}




void ContextChangeEventMultiplexer::NotifyContextChange (
    SfxViewShell* pViewShell,
    const ::sfx2::sidebar::EnumContext::Context eContext)
{
    if (pViewShell != nullptr)
        NotifyContextChange(pViewShell->GetController(), eContext);
}




::rtl::OUString ContextChangeEventMultiplexer::GetModuleName (
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    try
    {
        const Reference<frame::XModuleManager> xModuleManager =
            frame::ModuleManager::create( comphelper::getProcessComponentContext() );
        return xModuleManager->identify(rxFrame);
    }
    catch (const Exception&)
    {
        // An exception typically means that a context change is notified
        // during initialization or destruction of a view.
        // Ignore it.
    }
    return ::sfx2::sidebar::EnumContext::GetApplicationName(
        ::sfx2::sidebar::EnumContext::Application_None);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
