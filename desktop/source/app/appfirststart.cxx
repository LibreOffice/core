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

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <unotools/configmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#include "app.hxx"

using namespace ::desktop;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;

static const char aAccessSrvc[] = "com.sun.star.configuration.ConfigurationUpdateAccess";

/* Local function - get access to the configuration */
static Reference< XPropertySet > impl_getConfigurationAccess( const OUString& rPath )
{
    Reference< XMultiServiceFactory > xConfigProvider(
        configuration::theDefaultProvider::get(
            comphelper::getProcessComponentContext() ) );
    Sequence< Any > aArgs( 1 );
    NamedValue aValue( OUString( "nodepath"  ), makeAny( rPath ) );
    aArgs[0] <<= aValue;
    return Reference< XPropertySet >(
            xConfigProvider->createInstanceWithArguments( OUString(aAccessSrvc), aArgs ), UNO_QUERY_THROW );
}

void Desktop::DoRestartActionsIfNecessary( sal_Bool bQuickStart )
{
    if ( bQuickStart )
    {
        try
        {
            Reference< XPropertySet > xPSet = impl_getConfigurationAccess( OUString( "org.openoffice.Setup/Office"  ) );

            OUString sPropName( "OfficeRestartInProgress"  );
            Any aRestart = xPSet->getPropertyValue( sPropName );
            sal_Bool bRestart = sal_False;
            if ( ( aRestart >>= bRestart ) && bRestart )
            {
                xPSet->setPropertyValue( sPropName, makeAny( sal_False ) );
                Reference< util::XChangesBatch >( xPSet, UNO_QUERY_THROW )->commitChanges();

                Sequence< Any > aSeq( 1 );
                sal_Bool bQuickstart = shouldLaunchQuickstart();
                aSeq[0] <<= bQuickstart;

                css::uno::Reference< css::uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
                Reference < XInitialization > xQuickstart(
                    xContext->getServiceManager()->createInstanceWithContext("com.sun.star.office.Quickstart", xContext),
                    UNO_QUERY_THROW );
                xQuickstart->initialize( aSeq );
            }
        }
        catch( const uno::Exception& )
        {
            // this is no critical operation so it should not prevent office from starting
        }
    }
}

void Desktop::SetRestartState()
{
    try
    {
        Reference< XPropertySet > xPSet = impl_getConfigurationAccess( OUString( "org.openoffice.Setup/Office"  ) );
        OUString sPropName( "OfficeRestartInProgress"  );
        xPSet->setPropertyValue( sPropName, makeAny( sal_True ) );
        Reference< util::XChangesBatch >( xPSet, UNO_QUERY_THROW )->commitChanges();
    }
    catch( const uno::Exception& )
    {
        // this is no critical operation, ignore the exception
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
