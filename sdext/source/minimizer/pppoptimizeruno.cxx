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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sdext.hxx"

#include <stdio.h>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <pppoptimizer.hxx>
#include <pppoptimizerdialog.hxx>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    // -------------------------------------------------------------------------

    sal_Bool SAL_CALL component_writeInfo( void* /*pServiceManager*/, void* pRegistryKey )
    {
        if (pRegistryKey)
        {
            try
            {
                Reference< XRegistryKey >   xNewKey;
                sal_Int32                   nPos;

                xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( PPPOptimizer_getImplementationName() );
                xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
                const Sequence< OUString > & rSNL1 = PPPOptimizer_getSupportedServiceNames();
                const OUString * pArray1 = rSNL1.getConstArray();
                for ( nPos = rSNL1.getLength(); nPos--; )
                    xNewKey->createKey( pArray1[nPos] );

                xNewKey = reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey( PPPOptimizerDialog_getImplementationName() );
                xNewKey = xNewKey->createKey( OUString::createFromAscii( "/UNO/SERVICES" ) );
                const Sequence< OUString > & rSNL2 = PPPOptimizerDialog_getSupportedServiceNames();
                const OUString * pArray2 = rSNL2.getConstArray();
                for ( nPos = rSNL2.getLength(); nPos--; )
                    xNewKey->createKey( pArray2[nPos] );

                return sal_True;
            }
            catch (InvalidRegistryException &)
            {
                OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
            }
        }
        return sal_False;
    }

    // -------------------------------------------------------------------------

    void* SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
    {
        OUString    aImplName( OUString::createFromAscii( pImplName ) );
        void*       pRet = 0;

        if( pServiceManager )
        {
            Reference< XSingleComponentFactory > xFactory;
            if( aImplName.equals( PPPOptimizer_getImplementationName() ) )
            {
                xFactory = createSingleComponentFactory(
                        PPPOptimizer_createInstance,
                        OUString::createFromAscii( pImplName ),
                        PPPOptimizer_getSupportedServiceNames() );

            }
            else if( aImplName.equals( PPPOptimizerDialog_getImplementationName() ) )
            {
                xFactory = createSingleComponentFactory(
                        PPPOptimizerDialog_createInstance,
                        OUString::createFromAscii( pImplName ),
                        PPPOptimizerDialog_getSupportedServiceNames() );
            }
            if( xFactory.is() )
            {
                xFactory->acquire();
                pRet = xFactory.get();
            }
        }
        return pRet;
    }
}
