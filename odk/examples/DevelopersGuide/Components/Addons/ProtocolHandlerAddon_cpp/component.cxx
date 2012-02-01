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



#include <stdio.h>
#include <rtl/ustring.hxx>
#include <cppuhelper/queryinterface.hxx> // helper for queryInterface() impl
#include <cppuhelper/factory.hxx> // helper for component factory
// generated c++ interfaces
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

// include our specific addon header to get access to functions and definitions
#include <addon.hxx>

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//##################################################################################################
//#### EXPORTED ####################################################################################
//##################################################################################################


/**
 * Gives the environment this component belongs to.
 */
extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/**
 * This function creates an implementation section in the registry and another subkey
 *
 * for each supported service.
 * @param pServiceManager   the service manager
 * @param pRegistryKey      the registry key
 */
// This method not longer necessary since OOo 3.4 where the component registration was
// was changed to passive component registration. For more details see
// http://wiki.services.openoffice.org/wiki/Passive_Component_Registration
//
// extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(void * pServiceManager, void * pRegistryKey)
// {
//  sal_Bool result = sal_False;

//  if (pRegistryKey)
//  {
//      try
//      {
//          Reference< XRegistryKey > xNewKey(
//              reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
//                  OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLEMENTATION_NAME "/UNO/SERVICES") ) ) );

//          const Sequence< OUString > & rSNL =
//              Addon_getSupportedServiceNames();
//          const OUString * pArray = rSNL.getConstArray();
//          for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
//              xNewKey->createKey( pArray[nPos] );

//          return sal_True;
//      }
//      catch (InvalidRegistryException &)
//      {
//          // we should not ignore exceptions
//      }
//  }
//  return result;
// }

/**
 * This function is called to get service factories for an implementation.
 *
 * @param pImplName       name of implementation
 * @param pServiceManager a service manager, need for component creation
 * @param pRegistryKey    the registry key for this component, need for persistent data
 * @return a component factory
 */
extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey)
{
    void * pRet = 0;

    if (rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) ),
            Addon_createInstance,
            Addon_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
