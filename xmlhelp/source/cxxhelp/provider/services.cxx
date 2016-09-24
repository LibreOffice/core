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

#include "provider.hxx"
#include "tvfactory.hxx"

using namespace com::sun::star;

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL ucpchelp_component_getFactory(
    const sal_Char * pImplName,
    void * pServiceManager,
    SAL_UNUSED_PARAMETER void * /*pRegistryKey*/ )
{
    void * pRet = nullptr;

    uno::Reference< lang::XMultiServiceFactory > xSMgr(
        static_cast< lang::XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< lang::XSingleServiceFactory > xFactory;

    if ( ::chelp::ContentProvider::getImplementationName_Static().
             equalsAscii( pImplName ) )
    {
        xFactory = ::chelp::ContentProvider::createServiceFactory( xSMgr );
    }
    else if ( treeview::TVFactory::getImplementationName_static().equalsAscii( pImplName ) )
    {
        xFactory = treeview::TVFactory::createServiceFactory( xSMgr );
    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
