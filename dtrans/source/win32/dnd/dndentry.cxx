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
#include "precompiled_dtrans.hxx"
#include <cppuhelper/factory.hxx>
#include <com/sun/star/container/XSet.hpp>
#include <osl/diagnose.h>

#include "source.hxx"
#include "target.hxx"

using namespace ::rtl                       ;
using namespace ::com::sun::star::uno       ;
using namespace ::com::sun::star::registry  ;
using namespace ::cppu                      ;
using namespace ::com::sun::star::lang;

rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

Reference< XInterface > SAL_CALL createDragSource( const Reference< XMultiServiceFactory >& rServiceManager )
{
    DragSource* pSource= new DragSource( rServiceManager );
    return Reference<XInterface>( static_cast<XInitialization*>(pSource), UNO_QUERY);
}

Reference< XInterface > SAL_CALL createDropTarget( const Reference< XMultiServiceFactory >& rServiceManager )
{
    DropTarget* pTarget= new DropTarget( rServiceManager );
    return Reference<XInterface>( static_cast<XInitialization*>(pTarget), UNO_QUERY);
}


extern "C"
{
SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//----------------------------------------------------------------------
// component_getImplementationEnvironment
//----------------------------------------------------------------------

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//----------------------------------------------------------------------
// component_getFactory
// returns a factory to create XFilePicker-Services
//----------------------------------------------------------------------

SAL_DLLPUBLIC_EXPORT void* SAL_CALL component_getFactory( const sal_Char* pImplName, uno_Interface* pSrvManager, uno_Interface* /*pRegistryKey*/ )
{
    void* pRet = 0;
    Reference< XSingleServiceFactory > xFactory;

    if ( pSrvManager && ( 0 == rtl_str_compare( pImplName, DNDSOURCE_IMPL_NAME ) ) )
    {
        Sequence< OUString > aSNS( 1 );
        aSNS.getArray( )[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( DNDSOURCE_SERVICE_NAME ) );

        xFactory= createSingleFactory(
            reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
            OUString::createFromAscii( pImplName ),
            createDragSource,
            aSNS,
            &g_moduleCount.modCnt);

    }
    else if( pSrvManager && ( 0 == rtl_str_compare( pImplName, DNDTARGET_IMPL_NAME ) ) )
    {
        Sequence< OUString > aSNS( 1 );
        aSNS.getArray( )[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( DNDTARGET_SERVICE_NAME ) );

        xFactory= createSingleFactory(
            reinterpret_cast< XMultiServiceFactory* > ( pSrvManager ),
            OUString::createFromAscii( pImplName ),
            createDropTarget,
            aSNS);

    }

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }

    return pRet;
}

} // extern "C"
