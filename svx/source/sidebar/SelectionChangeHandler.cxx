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
#include "svx/sidebar/SelectionChangeHandler.hxx"
#include "svx/sidebar/SelectionAnalyzer.hxx"
#include "svx/sidebar/ContextChangeEventMultiplexer.hxx"
#include "svx/svdmrkv.hxx"

#include <vcl/EnumContext.hxx>
#include <sfx2/shell.hxx>


using namespace css;
using namespace css::uno;

namespace svx { namespace sidebar {

SelectionChangeHandler::SelectionChangeHandler (
    const std::function<rtl::OUString()>& rSelectionChangeCallback,
    const Reference<css::frame::XController>& rxController,
    const vcl::EnumContext::Context eDefaultContext)
    : SelectionChangeHandlerInterfaceBase(m_aMutex),
      maSelectionChangeCallback(rSelectionChangeCallback),
      mxController(rxController),
      meDefaultContext(eDefaultContext),
      mbIsConnected(false)
{
}


SelectionChangeHandler::~SelectionChangeHandler()
{
}


void SAL_CALL SelectionChangeHandler::selectionChanged (const lang::EventObject&)
{
    if (maSelectionChangeCallback)
    {
        const vcl::EnumContext::Context eContext (
            vcl::EnumContext::GetContextEnum(maSelectionChangeCallback()));
        ContextChangeEventMultiplexer::NotifyContextChange(
            mxController,
            eContext==vcl::EnumContext::Context::Unknown
                ? meDefaultContext
                : eContext);
    }
}


void SAL_CALL SelectionChangeHandler::disposing (const lang::EventObject&)
{
}


void SAL_CALL SelectionChangeHandler::disposing()
{
    if (mbIsConnected)
        Disconnect();
}


void SelectionChangeHandler::Connect()
{
    uno::Reference<view::XSelectionSupplier> xSupplier (mxController, uno::UNO_QUERY);
    if (xSupplier.is())
    {
        mbIsConnected = true;
        xSupplier->addSelectionChangeListener(this);
    }
}


void SelectionChangeHandler::Disconnect()
{
    uno::Reference<view::XSelectionSupplier> xSupplier (mxController, uno::UNO_QUERY);
    if (xSupplier.is())
    {
        mbIsConnected = false;
        xSupplier->removeSelectionChangeListener(this);
    }
}


} } // end of namespace svx::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
