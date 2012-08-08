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
