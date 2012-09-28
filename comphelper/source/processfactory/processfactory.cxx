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

#include <osl/mutex.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/uno/DeploymentException.hpp"

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace osl;

namespace comphelper
{

/*
    This function preserves only that the xProcessFactory variable will not be create when
    the library is loaded.
*/
Reference< XMultiServiceFactory > localProcessFactory( const Reference< XMultiServiceFactory >& xSMgr, sal_Bool bSet )
{
    Guard< Mutex > aGuard( Mutex::getGlobalMutex() );

    static Reference< XMultiServiceFactory > xProcessFactory;
    if ( bSet )
    {
        xProcessFactory = xSMgr;
    }

    return xProcessFactory;
}


void setProcessServiceFactory(const Reference< XMultiServiceFactory >& xSMgr)
{
    localProcessFactory( xSMgr, sal_True );
}

Reference< XMultiServiceFactory > getProcessServiceFactory()
{
    Reference< XMultiServiceFactory> xReturn;
    xReturn = localProcessFactory( xReturn, sal_False );
    if ( !xReturn.is() )
    {
        throw DeploymentException(
            "null process service factory", Reference< XInterface >() );
    }
    return xReturn;
}

Reference< XComponentContext > getComponentContext(
    Reference< XMultiServiceFactory > const & factory)
{
    Reference< XComponentContext > xRet;
    uno::Reference<beans::XPropertySet> const xProps( factory, uno::UNO_QUERY );
    if (xProps.is()) {
        try {
            xRet.set( xProps->getPropertyValue( rtl::OUString(
                              RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ),
                      uno::UNO_QUERY );
        }
        catch (beans::UnknownPropertyException & e) {
            throw DeploymentException(
                "unknown service factory DefaultContext property: " + e.Message,
                Reference< XInterface >( factory, UNO_QUERY ) );
        }
    }
    if ( !xRet.is() )
    {
        throw DeploymentException(
            "no service factory DefaultContext",
            Reference< XInterface >( factory, UNO_QUERY ) );
    }
    return xRet;
}

Reference< XComponentContext > getProcessComponentContext()
{
    return getComponentContext( getProcessServiceFactory() );
}

} // namespace comphelper

extern "C" {
uno::XComponentContext * comphelper_getProcessComponentContext()
{
    uno::Reference<uno::XComponentContext> xRet;
    xRet = ::comphelper::getProcessComponentContext();
    if (xRet.is())
        xRet->acquire();
    return xRet.get();
}
} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
