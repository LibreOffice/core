/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <comphelper/asyncquithandler.hxx>
#include <comphelper/processfactory.hxx>

AsyncQuitHandler::AsyncQuitHandler()
    : mbForceQuit(false)
{
}

AsyncQuitHandler& AsyncQuitHandler::instance()
{
    static AsyncQuitHandler aInst;
    return aInst;
}

void AsyncQuitHandler::QuitApplication()
{
    css::uno::Reference<css::frame::XDesktop2> xDesktop
        = css::frame::Desktop::create(comphelper::getProcessComponentContext());
    xDesktop->terminate();
}

void AsyncQuitHandler::SetForceQuit() { mbForceQuit = true; }

bool AsyncQuitHandler::IsForceQuit() { return mbForceQuit; }

IMPL_STATIC_LINK_NOARG(AsyncQuitHandler, OnAsyncQuit, void*, void) { QuitApplication(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
