/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
