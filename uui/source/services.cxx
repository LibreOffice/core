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

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppu/macros.hxx>
#include <cppuhelper/factory.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <uno/environment.h>

#include "requeststringresolver.hxx"
#include "passwordcontainer.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;



extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL uui_component_getFactory(sal_Char const * pImplName,
                        void * pServiceManager,
                        void *)
{
    if (!pImplName)
        return nullptr;

    void * pRet = nullptr;

    Reference< XMultiServiceFactory > xSMgr(
        static_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;


    // UUI Interaction Request String Resolver.


    if ( rtl_str_compare(pImplName,
                  UUIInteractionRequestStringResolver::m_aImplementationName)
          == 0)
    {
        xFactory =
            cppu::createSingleFactory(
                static_cast< XMultiServiceFactory * >(pServiceManager),
                OUString::createFromAscii(
                    UUIInteractionRequestStringResolver::m_aImplementationName),
                &UUIInteractionRequestStringResolver::createInstance,
                UUIInteractionRequestStringResolver::getSupportedServiceNames_static());
    }


    // UUI Password Container Interaction Handler.


    else if ( uui::PasswordContainerInteractionHandler::getImplementationName_Static().
                equalsAscii( pImplName ) )
    {
        xFactory =
            uui::PasswordContainerInteractionHandler::createServiceFactory( xSMgr );
    }



    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
