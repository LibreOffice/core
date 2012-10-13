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
#include <rtl/instance.hxx>

#include "typedetection.hxx"
#include "filterfactory.hxx"
#include "contenthandlerfactory.hxx"
#include "frameloaderfactory.hxx"
#include "configflush.hxx"


namespace filter{
    namespace config{


extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL
    filterconfig1_component_getFactory( const sal_Char* pImplementationName,
                                        void* pServiceManager,
                                        void* /* pRegistryKey */ )
{
    if ((!pImplementationName) || (!pServiceManager ))
        return NULL;

    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
        xSMGR = reinterpret_cast< com::sun::star::lang::XMultiServiceFactory* >(pServiceManager);
    com::sun::star::uno::Reference< com::sun::star::lang::XSingleServiceFactory > xFactory;
    rtl::OUString sImplName = rtl::OUString::createFromAscii(pImplementationName);

    if (TypeDetection::impl_getImplementationName() == sImplName)
        xFactory = cppu::createSingleFactory( xSMGR,
                                              TypeDetection::impl_getImplementationName(),
                                              TypeDetection::impl_createInstance,
                                              TypeDetection::impl_getSupportedServiceNames() );

    if (FilterFactory::impl_getImplementationName() == sImplName)
        xFactory = cppu::createSingleFactory( xSMGR,
                                              FilterFactory::impl_getImplementationName(),
                                              FilterFactory::impl_createInstance,
                                              FilterFactory::impl_getSupportedServiceNames() );

    if (ContentHandlerFactory::impl_getImplementationName() == sImplName)
        xFactory = cppu::createSingleFactory( xSMGR,
                                              ContentHandlerFactory::impl_getImplementationName(),
                                              ContentHandlerFactory::impl_createInstance,
                                              ContentHandlerFactory::impl_getSupportedServiceNames() );

    if (FrameLoaderFactory::impl_getImplementationName() == sImplName)
        xFactory = cppu::createSingleFactory( xSMGR,
                                              FrameLoaderFactory::impl_getImplementationName(),
                                              FrameLoaderFactory::impl_createInstance,
                                              FrameLoaderFactory::impl_getSupportedServiceNames() );

    if (ConfigFlush::impl_getImplementationName() == sImplName)
        xFactory = cppu::createOneInstanceFactory( xSMGR,
                                                   ConfigFlush::impl_getImplementationName(),
                                                   ConfigFlush::impl_createInstance,
                                                   ConfigFlush::impl_getSupportedServiceNames() );

    /* And if one of these checks was successful => xFactory was set! */
    if (xFactory.is())
    {
        xFactory->acquire();
        return xFactory.get();
    }

    return NULL;
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
