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

#ifndef INCLUDED_COMPHELPER_ASYNCQUITHANDLER_HXX
#define INCLUDED_COMPHELPER_ASYNCQUITHANDLER_HXX

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include <comphelper/processfactory.hxx>
#include <tools/link.hxx>

// Use: Application::PostUserEvent( LINK( &AsyncQuitHandler::instance(), AsyncQuitHandler, OnAsyncQuit ) );

class AsyncQuitHandler
{
    AsyncQuitHandler()
        : mbForceQuit(false)
    {
    }

    bool mbForceQuit;

public:
    AsyncQuitHandler(const AsyncQuitHandler&) = delete;
    const AsyncQuitHandler& operator=(const AsyncQuitHandler&) = delete;

    static AsyncQuitHandler& instance()
    {
        static AsyncQuitHandler aInst;
        return aInst;
    }

    static void QuitApplication()
    {
        css::uno::Reference<css::frame::XDesktop2> xDesktop
            = css::frame::Desktop::create(comphelper::getProcessComponentContext());
        xDesktop->terminate();
    }

    // Hack for the TerminationVetoer in extensions/source/ole/unoobjw.cxx. When it is an Automation
    // client itself that explicitly requests a quit (see VbaApplicationBase::Quit()), we do quit.
    // The flag can only be set to true, not back to false.
    void SetForceQuit() { mbForceQuit = true; }

    bool IsForceQuit() { return mbForceQuit; }

    DECL_STATIC_LINK(AsyncQuitHandler, OnAsyncQuit, void*, void);
};

IMPL_STATIC_LINK_NOARG(AsyncQuitHandler, OnAsyncQuit, void*, void) { QuitApplication(); }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
