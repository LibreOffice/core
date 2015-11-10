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

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <cppuhelper/factory.hxx>

#include <xprimitive2drenderer.hxx>

using namespace ::com::sun::star;

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void* SAL_CALL drawinglayer_component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /* pRegistryKey */ )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory;
        void* pRet = nullptr;

        if(drawinglayer::unorenderer::XPrimitive2DRenderer_getImplementationName().equalsAscii(pImplName))
        {
            xFactory = ::cppu::createSingleFactory(
                static_cast< lang::XMultiServiceFactory * >(pServiceManager),
                drawinglayer::unorenderer::XPrimitive2DRenderer_getImplementationName(),
                drawinglayer::unorenderer::XPrimitive2DRenderer_createInstance,
                drawinglayer::unorenderer::XPrimitive2DRenderer_getSupportedServiceNames());
        }

        if(xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }

        return pRet;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
