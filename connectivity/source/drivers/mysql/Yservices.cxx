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

#include "mysql/YDriver.hxx"
#include <cppuhelper/factory.hxx>

using namespace connectivity::mysql;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::lang::XSingleServiceFactory;
using ::com::sun::star::lang::XMultiServiceFactory;

typedef Reference< XSingleServiceFactory > (SAL_CALL *createFactoryFunc)
        (
            const Reference< XMultiServiceFactory > & rServiceManager,
            const OUString & rComponentName,
            ::cppu::ComponentInstantiation pCreateFunction,
            const Sequence< OUString > & rServiceNames,
            rtl_ModuleCount*
        );

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL mysql_component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* /*pRegistryKey*/)
{
    if (!pServiceManager)
    {
        return nullptr;
    }

    Reference< XSingleServiceFactory > xRet;
    const Reference< XMultiServiceFactory > xServiceManager( static_cast<XMultiServiceFactory*>(pServiceManager) );
    const OUString sImplementationName( OUString::createFromAscii(pImplementationName) );

    if( ODriverDelegator::getImplementationName_Static() == sImplementationName )
        try
        {
            xRet = ::cppu::createSingleFactory( xServiceManager, sImplementationName, ODriverDelegator_CreateInstance,
                       ODriverDelegator::getSupportedServiceNames_Static());
        }
        catch(...)
        {
        }

    if(xRet.is())
        xRet->acquire();

    return xRet.get();
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
