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

#include <comphelper/processfactory.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XInitialization.hpp>


#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>

#include <sal/main.h>
#include <osl/process.h>
#include <stdio.h>

namespace deployment = ::com::sun::star::deployment;
namespace lang = ::com::sun::star::lang;
namespace uno = ::com::sun::star::uno;
namespace xml = ::com::sun::star::xml;

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

// -----------------------------------------------------------------------

SAL_IMPLEMENT_MAIN()
{
    (void) argv;
    (void) argc;

    if( osl_getCommandArgCount() != 1 )
    {
        fprintf(stderr, "Usage: updatefeedtest <url>\n");
        return -1;
    }

    // create the initial component context
    uno::Reference< uno::XComponentContext > rComponentContext = cppu::defaultBootstrap_InitialComponentContext();

    // initialize UCB
    uno::Reference< uno::XUniversalContentBroker > xUCB =
        ucb::UniversalContentBroker::createWithKeys(rComponentContext, "Local", "Office");

    // retrieve the update information provider
    uno::Reference< deployment::XUpdateInformationProvider > rUpdateInformationProvider =
        deployment::UpdateInformationProvider::create( rComponentContext );

    uno::Sequence< rtl::OUString > theURLs(1);
    osl_getCommandArg( 0, &theURLs[0].pData );
    // theURLs[0] = UNISTRING( "http://localhost/~olli/atomfeed.xml" );

    rtl::OUString aExtension = UNISTRING( "MyExtension" );

    try
    {
        uno::Sequence< uno::Reference< xml::dom::XElement > > theUpdateInfo =
            rUpdateInformationProvider->getUpdateInformation( theURLs, aExtension );

        OSL_TRACE( "getUpdateInformation returns %d element(s)", theUpdateInfo.getLength() );
    }
    catch( const uno::Exception & e )
    {
        OSL_TRACE( "exception caught: %s", rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_UTF8 ).getStr());
    }
    catch( ... )
    {
        OSL_TRACE( "exception of undetermined type caught" );
    }


    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
