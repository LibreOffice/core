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

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XInitialization.hpp>


#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>

#include <sal/main.h>
#include <osl/process.h>
#include <sal/log.hxx>
#include <stdio.h>

namespace deployment = ::com::sun::star::deployment;
namespace lang = ::com::sun::star::lang;
namespace uno = ::com::sun::star::uno;
namespace xml = ::com::sun::star::xml;


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

    // initialize UCB (for backwards compatibility, in case some code still uses
    // plain createInstance w/o args directly to obtain an instance):
    ucb::UniversalContentBroker::create(rComponentContext);

    // retrieve the update information provider
    uno::Reference< deployment::XUpdateInformationProvider > rUpdateInformationProvider =
        deployment::UpdateInformationProvider::create( rComponentContext );

    uno::Sequence< OUString > theURLs(1);
    osl_getCommandArg( 0, &theURLs[0].pData );
    // theURLs[0] = "http://localhost/~olli/atomfeed.xml";

    OUString aExtension = "MyExtension";

    try
    {
        uno::Sequence< uno::Reference< xml::dom::XElement > > theUpdateInfo =
            rUpdateInformationProvider->getUpdateInformation( theURLs, aExtension );
    }
    catch( const uno::Exception & )
    {
        SAL_WARN("extensions.update", "exception caught: " << exceptionToString(ex));
    }
    catch( ... )
    {
        SAL_WARN("extensions.update", "exception of undetermined type caught" );
    }


    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
