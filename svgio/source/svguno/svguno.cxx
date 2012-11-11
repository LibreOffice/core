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

#include <svgio/svgiodllapi.h>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <uno/environment.h>
#include <cppuhelper/factory.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace svgio
{
    namespace svgreader
    {
        extern uno::Sequence< rtl::OUString > SAL_CALL XSvgParser_getSupportedServiceNames();
        extern rtl::OUString SAL_CALL XSvgParser_getImplementationName();
        extern uno::Reference< uno::XInterface > SAL_CALL XSvgParser_createInstance( const uno::Reference< lang::XMultiServiceFactory > & );
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// component_getImplementationEnvironment

extern "C"
{
    SVGIO_DLLPUBLIC void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** /* ppEnv */ )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }
}

//////////////////////////////////////////////////////////////////////////////
// component_getFactory

extern "C"
{
    SVGIO_DLLPUBLIC void* SAL_CALL component_getFactory( const sal_Char* pImplName, void* pServiceManager, void* /* pRegistryKey */ )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory;
        void* pRet = 0;

        if(svgio::svgreader::XSvgParser_getImplementationName().equalsAscii(pImplName))
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< lang::XMultiServiceFactory * >(pServiceManager),
                svgio::svgreader::XSvgParser_getImplementationName(),
                svgio::svgreader::XSvgParser_createInstance,
                svgio::svgreader::XSvgParser_getSupportedServiceNames());
        }

        if(xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }

        return pRet;
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
