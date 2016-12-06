/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppu/macros.hxx>
#include <cppuhelper/factory.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <uno/environment.h>

#include "interactionhandler.hxx"
#include "requeststringresolver.hxx"
#include "passwordcontainer.hxx"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

//============================================================================
//
//  component_getImplementationEnvironment
//
//============================================================================

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL
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

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL
component_getFactory(sal_Char const * pImplName,
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
