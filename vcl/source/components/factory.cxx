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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <vcl/dllapi.h>

#include <factory.hxx>

extern "C" {

    VCL_DLLPUBLIC void* vcl_component_getFactory(
        const char* pImplementationName,
        void* pXUnoSMgr,
        void* /*pXUnoKey*/)
    {
        if (!pXUnoSMgr)
            return nullptr;

        css::uno::Reference<css::lang::XMultiServiceFactory> xMgr(
            static_cast<css::lang::XMultiServiceFactory*>(pXUnoSMgr));

        css::uno::Reference<css::lang::XSingleServiceFactory> xFactory;

        if( vcl::DragSource_getImplementationName().equalsAscii( pImplementationName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                xMgr, vcl::DragSource_getImplementationName(), vcl::DragSource_createInstance,
                vcl::DragSource_getSupportedServiceNames() );
        }

        if( xFactory.is() )
        {
            xFactory->acquire();
            return xFactory.get();
        }

        return nullptr;
    }

} /* extern "C" */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
