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
#include <comphelper/processfactory.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XInitialization.hpp>


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
    uno::Sequence< uno::Any > theArguments(2);
    theArguments[0] = uno::makeAny( UNISTRING( "Local") );
    theArguments[1] = uno::makeAny( UNISTRING( "Office") );

    uno::Reference< uno::XInterface > xUCB =
        rComponentContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            UNISTRING( "com.sun.star.ucb.UniversalContentBroker" ),
            theArguments,
            rComponentContext );

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
