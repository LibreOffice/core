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

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

namespace task = ::com::sun::star::task;
namespace uno  = ::com::sun::star::uno;

// -----------------------------------------------------------------------

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

    // initialize UCB
    uno::Reference< uno::XUniversalContentBroker > xUCB =
        UniversalContentBroker::createWithKeys(rComponentContext, theArguments, "Local", "Office");


    rtl::OUString aURL;
    rtl::OUString aVersion;

    try
    {
        if( checkForUpdates(rComponentContext, uno::Reference< task::XInteractionHandler > (), aURL, aVersion) )
        {
            OSL_TRACE( "Update found: %s on %s",
                rtl::OUStringToOString( aVersion, RTL_TEXTENCODING_UTF8).getStr(),
                rtl::OUStringToOString( aURL, RTL_TEXTENCODING_UTF8).getStr());
        }
        else
        {
            OSL_TRACE( "no updates found" );
        }
    }
    catch( ... )
    {
        OSL_TRACE( "unhandled exception caught" );
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
