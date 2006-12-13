/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: updatefeedtest.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:19:19 $
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

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>

#include <com/sun/star/lang/XInitialization.hpp>


#ifndef _COM_SUN_STAR_DEPLOYMENT_UPDATEINFORMATIONPROVIDER_HPP_
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>
#endif

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
