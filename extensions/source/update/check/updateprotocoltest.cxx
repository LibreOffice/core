/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#include <cppuhelper/bootstrap.hxx>

#include "updateprotocol.hxx"

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
    uno::Sequence< uno::Any > theArguments(2);
    theArguments[0] = uno::makeAny( UNISTRING( "Local") );
    theArguments[1] = uno::makeAny( UNISTRING( "Office") );

    uno::Reference< uno::XInterface > xUCB =
        rComponentContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            UNISTRING( "com.sun.star.ucb.UniversalContentBroker" ),
            theArguments,
            rComponentContext );


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
