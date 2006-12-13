/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updateprotocoltest.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:17:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
