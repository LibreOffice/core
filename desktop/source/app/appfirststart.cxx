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
#include "precompiled_desktop.hxx"

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <tools/datetime.hxx>
#include <unotools/configmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#include "app.hxx"

using ::rtl::OUString;
using namespace ::desktop;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;

static const OUString sConfigSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ) );
static const OUString sAccessSrvc( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationUpdateAccess" ) );

/* Local function - get access to the configuration */
static Reference< XPropertySet > impl_getConfigurationAccess( const OUString& rPath )
{
    Reference < XMultiServiceFactory > xFactory = ::comphelper::getProcessServiceFactory();

    // get configuration provider
    Reference< XMultiServiceFactory > xConfigProvider = Reference< XMultiServiceFactory >(
            xFactory->createInstance( sConfigSrvc ), UNO_QUERY_THROW );

    Sequence< Any > aArgs( 1 );
    NamedValue aValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "NodePath" ) ), makeAny( rPath ) );
    aArgs[0] <<= aValue;
    return Reference< XPropertySet >(
            xConfigProvider->createInstanceWithArguments( sAccessSrvc, aArgs ), UNO_QUERY_THROW );
}

void Desktop::DoRestartActionsIfNecessary( sal_Bool bQuickStart )
{
    if ( bQuickStart )
    {
        try
        {
            Reference< XPropertySet > xPSet = impl_getConfigurationAccess( OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Setup/Office" ) ) );

            OUString sPropName( RTL_CONSTASCII_USTRINGPARAM( "OfficeRestartInProgress" ) );
            Any aRestart = xPSet->getPropertyValue( sPropName );
            sal_Bool bRestart = sal_False;
            if ( ( aRestart >>= bRestart ) && bRestart )
            {
                xPSet->setPropertyValue( sPropName, makeAny( sal_False ) );
                Reference< util::XChangesBatch >( xPSet, UNO_QUERY_THROW )->commitChanges();

                Sequence< Any > aSeq( 1 );
                sal_Bool bQuickstart = shouldLaunchQuickstart();
                aSeq[0] <<= bQuickstart;

                Reference < XInitialization > xQuickstart( ::comphelper::getProcessServiceFactory()->createInstance(
                    OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.Quickstart" ) ) ),UNO_QUERY_THROW );
                xQuickstart->initialize( aSeq );
            }
        }
        catch( uno::Exception& )
        {
            // this is no critical operation so it should not prevent office from starting
        }
    }
}

void Desktop::SetRestartState()
{
    try
    {
        Reference< XPropertySet > xPSet = impl_getConfigurationAccess( OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Setup/Office" ) ) );
        OUString sPropName( RTL_CONSTASCII_USTRINGPARAM( "OfficeRestartInProgress" ) );
        xPSet->setPropertyValue( sPropName, makeAny( sal_True ) );
        Reference< util::XChangesBatch >( xPSet, UNO_QUERY_THROW )->commitChanges();
    }
    catch( uno::Exception& )
    {
        // this is no critical operation, ignore the exception
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
