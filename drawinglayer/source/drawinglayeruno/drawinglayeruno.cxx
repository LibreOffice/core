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

#include <drawinglayer/drawinglayerdllapi.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <uno/environment.h>
#include <cppuhelper/factory.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer
{
    namespace unorenderer
    {
        extern uno::Sequence< OUString > SAL_CALL XPrimitive2DRenderer_getSupportedServiceNames();
        extern OUString SAL_CALL XPrimitive2DRenderer_getImplementationName();
        extern uno::Reference< uno::XInterface > SAL_CALL XPrimitive2DRenderer_createInstance( const uno::Reference< lang::XMultiServiceFactory > & );
    } // end of namespace unorenderer
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// component_getImplementationEnvironment

extern "C"
{
    DRAWINGLAYER_DLLPUBLIC void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }
}

//////////////////////////////////////////////////////////////////////////////
// component_getFactory

extern "C"
{
    DRAWINGLAYER_DLLPUBLIC void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /* pRegistryKey */ )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory;
        void* pRet = 0;

        if(drawinglayer::unorenderer::XPrimitive2DRenderer_getImplementationName().equalsAscii(pImplName))
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< lang::XMultiServiceFactory * >(pServiceManager),
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
