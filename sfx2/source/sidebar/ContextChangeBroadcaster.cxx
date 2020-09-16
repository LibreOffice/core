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
#include <sidebar/ContextChangeBroadcaster.hxx>
#include <vcl/EnumContext.hxx>
#include <com/sun/star/ui/ContextChangeEventObject.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <osl/diagnose.h>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/viewsh.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

ContextChangeBroadcaster::ContextChangeBroadcaster()
    : msContextName(),
      mbIsBroadcasterEnabled(true)
{
}

ContextChangeBroadcaster::~ContextChangeBroadcaster()
{
}

void ContextChangeBroadcaster::Initialize (const OUString& rsContextName)
{
    msContextName = rsContextName;
}

void ContextChangeBroadcaster::Activate (const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (msContextName.getLength() > 0)
        BroadcastContextChange(rxFrame, GetModuleName(rxFrame), msContextName);
}

void ContextChangeBroadcaster::Deactivate (const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if (msContextName.getLength() > 0)
    {
        BroadcastContextChange(
            rxFrame,
            GetModuleName(rxFrame),
            (comphelper::LibreOfficeKit::isActive() ? msContextName:
             vcl::EnumContext::GetContextName(vcl::EnumContext::Context::Default)));
    }
}

bool ContextChangeBroadcaster::SetBroadcasterEnabled (const bool bIsEnabled)
{
    const bool bWasEnabled (mbIsBroadcasterEnabled);
    mbIsBroadcasterEnabled = bIsEnabled;
    return bWasEnabled;
}

void ContextChangeBroadcaster::BroadcastContextChange (
    const css::uno::Reference<css::frame::XFrame>& rxFrame,
    const OUString& rsModuleName,
    const OUString& rsContextName)
{
    if ( ! mbIsBroadcasterEnabled)
        return;

    if (rsContextName.getLength() == 0)
        return;

    if ( ! rxFrame.is() || ! rxFrame->getController().is())
    {
        // Frame is (probably) being deleted.  Broadcasting context
        // changes is not necessary anymore.
        return;
    }

    // notify the LOK too
    if (comphelper::LibreOfficeKit::isActive())
    {
        if (SfxViewShell* pViewShell = SfxViewShell::Get(rxFrame->getController()))
            SfxLokHelper::notifyContextChange(pViewShell, rsModuleName, rsContextName);
    }

    const css::ui::ContextChangeEventObject aEvent(
        rxFrame->getController(),
        rsModuleName,
        rsContextName);

    css::uno::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
        css::ui::ContextChangeEventMultiplexer::get(
            ::comphelper::getProcessComponentContext()));
    if (xMultiplexer.is())
        xMultiplexer->broadcastContextChangeEvent(aEvent, rxFrame->getController());
}

OUString ContextChangeBroadcaster::GetModuleName (const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    if ( ! rxFrame.is() || ! rxFrame->getController().is())
        return OUString();
    try
    {
        const Reference<XComponentContext> xContext (::comphelper::getProcessComponentContext() );
        const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( xContext );
        return xModuleManager->identify(rxFrame);
    }
    catch (const Exception&)
    {
        OSL_ENSURE(false, "can not determine module name");
    }
    return OUString();
}

} // end of namespace ::sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
