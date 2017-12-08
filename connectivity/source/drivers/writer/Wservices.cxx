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

#include <writer/WDriver.hxx>
#include <cppuhelper/factory.hxx>

using namespace com::sun::star;

typedef uno::Reference< lang::XSingleServiceFactory > (* createFactoryFunc)
(
    const uno::Reference< lang::XMultiServiceFactory >& rServiceManager,
    const OUString& rComponentName,
    ::cppu::ComponentInstantiation pCreateFunction,
    const uno::Sequence< OUString >& rServiceNames,
    rtl_ModuleCount*
);


struct ProviderRequest
{
    uno::Reference< lang::XSingleServiceFactory > xRet;
    uno::Reference< lang::XMultiServiceFactory > const xServiceManager;
    OUString const sImplementationName;

    ProviderRequest(
        void* pServiceManager,
        sal_Char const* pImplementationName
    )
        : xServiceManager(static_cast<lang::XMultiServiceFactory*>(pServiceManager))
        , sImplementationName(OUString::createFromAscii(pImplementationName))
    {
    }

    bool CREATE_PROVIDER(
        const OUString& Implname,
        const uno::Sequence< OUString >& Services,
        ::cppu::ComponentInstantiation Factory,
        createFactoryFunc creator
    )
    {
        if (!xRet.is() && (Implname == sImplementationName))
        {
            try
            {
                xRet = creator(xServiceManager, sImplementationName,Factory, Services,nullptr);
            }
            catch (...)
            {
            }
        }
        return xRet.is();
    }

    void* getProvider() const
    {
        return xRet.get();
    }
};


extern "C" SAL_DLLPUBLIC_EXPORT void* connectivity_writer_component_getFactory(
    const sal_Char* pImplementationName,
    void* pServiceManager,
    void* /*pRegistryKey*/)
{
    void* pRet = nullptr;
    if (pServiceManager)
    {
        ProviderRequest aReq(pServiceManager,pImplementationName);

        aReq.CREATE_PROVIDER(
            connectivity::writer::ODriver::getImplementationName_Static(),
            connectivity::writer::ODriver::getSupportedServiceNames_Static(),
            connectivity::writer::ODriver_CreateInstance, ::cppu::createSingleFactory)
        ;

        if (aReq.xRet.is())
            aReq.xRet->acquire();

        pRet = aReq.getProvider();
    }

    return pRet;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
