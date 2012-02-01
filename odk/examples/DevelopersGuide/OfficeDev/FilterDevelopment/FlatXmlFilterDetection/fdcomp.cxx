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

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "filterdetect.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

// This method not longer necessary since OOo 3.4 where the component registration was
// was changed to passive component registration. For more details see
// http://wiki.services.openoffice.org/wiki/Passive_Component_Registration
//==================================================================================================
// SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(
//  void * pServiceManager, void * pRegistryKey )
// {
//  if (pRegistryKey)
//  {
//      try
//      {
//          Reference< XRegistryKey > xNewKey(
//              reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( FilterDetect_getImplementationName() ) );
//          xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );

//          const Sequence< OUString > & rSNL = FilterDetect_getSupportedServiceNames();
//          const OUString * pArray = rSNL.getConstArray();
//          for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
//              xNewKey->createKey( pArray[nPos] );

//          return sal_True;
//      }
//      catch (InvalidRegistryException &)
//      {
//          OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
//      }
//  }
//  return sal_False;
// }

//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    OUString implName = OUString::createFromAscii( pImplName );
    if ( pServiceManager && implName.equals(FilterDetect_getImplementationName()) )
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            FilterDetect_createInstance, FilterDetect_getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

}
