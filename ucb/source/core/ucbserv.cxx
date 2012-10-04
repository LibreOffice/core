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

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include "ucb.hxx"
#include "ucbstore.hxx"
#include "ucbprops.hxx"
#include "provprox.hxx"
#include "cmdenv.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using ::rtl::OUString;


//=========================================================================
extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucb_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * )
{
    void * pRet = 0;

    Reference< XMultiServiceFactory > xSMgr(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;

    //////////////////////////////////////////////////////////////////////
    // Universal Content Broker.
    //////////////////////////////////////////////////////////////////////

    if ( UniversalContentBroker::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UniversalContentBroker::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCB Store.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbStore::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UcbStore::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCB PropertiesManager.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbPropertiesManager::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory = UcbPropertiesManager::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // UCP Proxy Factory.
    //////////////////////////////////////////////////////////////////////

    else if ( UcbContentProviderProxyFactory::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory
            = UcbContentProviderProxyFactory::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////
    // Command Environment.
    //////////////////////////////////////////////////////////////////////

    else if ( ucb_cmdenv::UcbCommandEnvironment::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory
            = ucb_cmdenv::UcbCommandEnvironment::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
