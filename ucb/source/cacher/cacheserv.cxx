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
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cachedcontentresultset.hxx>
#include <cachedcontentresultsetstub.hxx>
#include <cacheddynamicresultset.hxx>
#include <cacheddynamicresultsetstub.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;



extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL cached1_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * )
{
    void * pRet = nullptr;

    Reference< XMultiServiceFactory > xSMgr(
            static_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;


    // CachedContentResultSetFactory.


    if ( CachedContentResultSetFactory::getImplementationName_Static().
                equalsAscii( pImplName ) )
    {
        xFactory = CachedContentResultSetFactory::createServiceFactory( xSMgr );
    }


    // CachedContentResultSetStubFactory.


    else if ( CachedContentResultSetStubFactory::getImplementationName_Static().
                equalsAscii( pImplName ) )
    {
        xFactory = CachedContentResultSetStubFactory::createServiceFactory( xSMgr );
    }


    // CachedDynamicResultSetFactory.


    else if ( CachedDynamicResultSetFactory::getImplementationName_Static().
                equalsAscii( pImplName ) )
    {
        xFactory = CachedDynamicResultSetFactory::createServiceFactory( xSMgr );
    }


    // CachedDynamicResultSetStubFactory.


    else if ( CachedDynamicResultSetStubFactory::getImplementationName_Static().
                equalsAscii( pImplName ) )
    {
        xFactory = CachedDynamicResultSetStubFactory::createServiceFactory( xSMgr );
    }



    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
