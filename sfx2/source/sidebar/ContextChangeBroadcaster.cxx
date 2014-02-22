/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include <sfx2/sidebar/ContextChangeBroadcaster.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <com/sun/star/ui/ContextChangeEventObject.hpp>
#include <com/sun/star/ui/ContextChangeEventMultiplexer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <comphelper/processfactory.hxx>


using ::rtl::OUString;
using namespace css;
using namespace cssu;

namespace sfx2 { namespace sidebar {


ContextChangeBroadcaster::ContextChangeBroadcaster (void)
    : msContextName(),
      mbIsBroadcasterEnabled(true)
{
}



ContextChangeBroadcaster::~ContextChangeBroadcaster (void)
{
}




void ContextChangeBroadcaster::Initialize (const ::rtl::OUString& rsContextName)
{
    msContextName = rsContextName;
}




void ContextChangeBroadcaster::Activate (const cssu::Reference<css::frame::XFrame>& rxFrame)
{
    if (msContextName.getLength() > 0)
        BroadcastContextChange(rxFrame, GetModuleName(rxFrame), msContextName);
}




void ContextChangeBroadcaster::Deactivate (const cssu::Reference<css::frame::XFrame>& rxFrame)
{
    if (msContextName.getLength() > 0)
    {
        BroadcastContextChange(
            rxFrame,
            GetModuleName(rxFrame),
            EnumContext::GetContextName(EnumContext::Context_Default));
    }
}




bool ContextChangeBroadcaster::SetBroadcasterEnabled (const bool bIsEnabled)
{
    const bool bWasEnabled (mbIsBroadcasterEnabled);
    mbIsBroadcasterEnabled = bIsEnabled;
    return bWasEnabled;
}




void ContextChangeBroadcaster::BroadcastContextChange (
    const cssu::Reference<css::frame::XFrame>& rxFrame,
    const ::rtl::OUString& rsModuleName,
    const ::rtl::OUString& rsContextName)
{
    if ( ! mbIsBroadcasterEnabled)
        return;

    if (rsContextName.getLength() == 0)
        return;

    if ( ! rxFrame.is() || ! rxFrame->getController().is())
    {
        
        
        return;
    }

    const css::ui::ContextChangeEventObject aEvent(
        rxFrame->getController(),
        rsModuleName,
        rsContextName);

    cssu::Reference<css::ui::XContextChangeEventMultiplexer> xMultiplexer (
        css::ui::ContextChangeEventMultiplexer::get(
            ::comphelper::getProcessComponentContext()));
    if (xMultiplexer.is())
        xMultiplexer->broadcastContextChangeEvent(aEvent, rxFrame->getController());
}




OUString ContextChangeBroadcaster::GetModuleName (const cssu::Reference<css::frame::XFrame>& rxFrame)
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



} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
