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

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppu/macros.hxx>
#include <cppuhelper/factory.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <uno/environment.h>

#include "interactionhandler.hxx"
#include "requeststringresolver.hxx"
#include "passwordcontainer.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

using ::rtl::OUString;

//============================================================================
//
//  component_getImplementationEnvironment
//
//============================================================================

extern "C" void SAL_CALL
component_getImplementationEnvironment(sal_Char const ** pEnvTypeName,
                       uno_Environment **)
{
    *pEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//============================================================================
//
//  component_getFactory
//
//============================================================================

extern "C" void * SAL_CALL component_getFactory(sal_Char const * pImplName,
                        void * pServiceManager,
                        void *)
{
    if (!pImplName)
        return 0;

    void * pRet = 0;

    Reference< XMultiServiceFactory > xSMgr(
        reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );
    Reference< XSingleServiceFactory > xFactory;

    //////////////////////////////////////////////////////////////////////
    // UUI Interaction Handler.
    //////////////////////////////////////////////////////////////////////

    if ( rtl_str_compare(pImplName,
                         UUIInteractionHandler::m_aImplementationName)
         == 0)
    {
        xFactory =
            cppu::createSingleFactory(
                static_cast< XMultiServiceFactory * >(pServiceManager),
                OUString::createFromAscii(
                    UUIInteractionHandler::m_aImplementationName),
                &UUIInteractionHandler::createInstance,
                UUIInteractionHandler::getSupportedServiceNames_static());
    }

    //////////////////////////////////////////////////////////////////////
    // UUI Interaction Request String Resolver.
    //////////////////////////////////////////////////////////////////////

    else if ( rtl_str_compare(pImplName,
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

    //////////////////////////////////////////////////////////////////////
    // UUI Password Container Interaction Handler.
    //////////////////////////////////////////////////////////////////////

    else if ( uui::PasswordContainerInteractionHandler::getImplementationName_Static().
                compareToAscii( pImplName ) == 0 )
    {
        xFactory =
            uui::PasswordContainerInteractionHandler::createServiceFactory( xSMgr );
    }

    //////////////////////////////////////////////////////////////////////

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
