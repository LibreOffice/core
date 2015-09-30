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


#include <cppuhelper/bootstrap.hxx>

#include "updateprotocol.hxx"
#include <com/sun/star/ucb/UniversalContentBroker.hpp>

#include <sal/main.h>
#include <osl/process.h>
#include <stdio.h>
#include "sal/log.hxx"

namespace task = ::com::sun::star::task;
namespace uno  = ::com::sun::star::uno;



SAL_IMPLEMENT_MAIN()
{
    (void) argv;
    (void) argc;

    if( osl_getCommandArgCount() != 0 )
    {
        fprintf(stderr, "Usage: updateprotocoltest\n");
        return -1;
    }

    // create the initial component context
    uno::Reference< uno::XComponentContext > rComponentContext = cppu::defaultBootstrap_InitialComponentContext();

    // initialize UCB (for backwards compatibility, in case some code still uses
    // plain createInstance w/o args directly to obtain an instance):
    css::ucb::UniversalContentBroker::create(rComponentContext);


    OUString aURL;
    OUString aVersion;

    try
    {
        if( checkForUpdates(rComponentContext, uno::Reference< task::XInteractionHandler > (), aURL, aVersion) )
        {
            SAL_INFO("extensions.update", "Update found: " << aVersion << " on " << aURL);
        }
        else
        {
            SAL_INFO("extensions.update", "no updates found" );
        }
    }
    catch( ... )
    {
        SAL_INFO("extensions.update", "unhandled exception caught" );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
