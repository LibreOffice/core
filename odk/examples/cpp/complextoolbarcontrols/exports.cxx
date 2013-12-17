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

#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>

#include "MyProtocolHandler.h"
#include "MyListener.h"

extern "C"
{

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory(const sal_Char* pImplName      ,
                                                                void*     pServiceManager,
                                                                void*     pRegistryKey   )
{
    if ( !pServiceManager || !pImplName )
        return 0;

    css::uno::Reference< css::uno::XInterface > xFactory  ;
    css::uno::Reference< css::lang::XMultiServiceFactory >  xSMGR     (reinterpret_cast< css::lang::XMultiServiceFactory* >(pServiceManager), css::uno::UNO_QUERY);
    ::rtl::OUString                                         sImplName = ::rtl::OUString::createFromAscii(pImplName);

    if (sImplName == MYLISTENER_IMPLEMENTATIONNAME)
    {
        css::uno::Sequence< ::rtl::OUString > lNames(1);
        lNames[0] = MYLISTENER_IMPLEMENTATIONNAME;
        xFactory = ::cppu::createSingleFactory(xSMGR, sImplName, MyListener::st_createInstance, lNames);
    }
    else if (sImplName == MYPROTOCOLHANDLER_IMPLEMENTATIONNAME)
    {
        css::uno::Sequence< ::rtl::OUString > lNames(1);
        lNames[0] = MYPROTOCOLHANDLER_SERVICENAME;
        xFactory = ::cppu::createSingleComponentFactory(MyProtocolHandler_createInstance, sImplName, lNames);
    }

    if (!xFactory.is())
        return 0;

    xFactory->acquire();
    return xFactory.get();
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    char const ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

} // extern C

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
