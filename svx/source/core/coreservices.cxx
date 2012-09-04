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



/** this file contains the uno service registrations for all services in the svxcore lib */

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "sal/types.h"
#include "osl/diagnose.h"
#include "cppuhelper/factory.hxx"
#include "uno/lbnames.h"

using rtl::OUString;

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

namespace svx
{
extern OUString SAL_CALL ExtrusionDepthController_getImplementationName();
extern Reference< XInterface > SAL_CALL ExtrusionDepthController_createInstance(const Reference< XMultiServiceFactory > &)  throw( RuntimeException );
extern Sequence< OUString > SAL_CALL ExtrusionDepthController_getSupportedServiceNames() throw( RuntimeException );

extern OUString SAL_CALL ExtrusionDirectionControl_getImplementationName();
extern Reference< XInterface > SAL_CALL ExtrusionDirectionControl_createInstance(const Reference< XMultiServiceFactory > &)  throw( RuntimeException );
extern Sequence< OUString > SAL_CALL ExtrusionDirectionControl_getSupportedServiceNames() throw( RuntimeException );

extern OUString SAL_CALL ExtrusionLightingControl_getImplementationName();
extern Reference< XInterface > SAL_CALL ExtrusionLightingControl_createInstance(const Reference< XMultiServiceFactory > &)  throw( RuntimeException );
extern Sequence< OUString > SAL_CALL ExtrusionLightingControl_getSupportedServiceNames() throw( RuntimeException );

extern OUString SAL_CALL ExtrusionSurfaceControl_getImplementationName();
extern Reference< XInterface > SAL_CALL ExtrusionSurfaceControl_createInstance(const Reference< XMultiServiceFactory > &)  throw( RuntimeException );
extern Sequence< OUString > SAL_CALL ExtrusionSurfaceControl_getSupportedServiceNames() throw( RuntimeException );

extern OUString SAL_CALL FontworkAlignmentControl_getImplementationName();
extern Reference< XInterface > SAL_CALL FontworkAlignmentControl_createInstance(const Reference< XMultiServiceFactory > &)  throw( RuntimeException );
extern Sequence< OUString > SAL_CALL FontworkAlignmentControl_getSupportedServiceNames() throw( RuntimeException );

extern OUString SAL_CALL FontworkCharacterSpacingControl_getImplementationName();
extern Reference< XInterface > SAL_CALL FontworkCharacterSpacingControl_createInstance(const Reference< XMultiServiceFactory > &)  throw( RuntimeException );
extern Sequence< OUString > SAL_CALL FontworkCharacterSpacingControl_getSupportedServiceNames() throw( RuntimeException );
}

extern "C"
{

SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment (
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory (
    const sal_Char * pImplName, void * pServiceManager, void *  )
{
    void * pRet = 0;
    if( pServiceManager  )
    {
        Reference< XSingleServiceFactory > xFactory;

        if( ::svx::ExtrusionDepthController_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                ::svx::ExtrusionDepthController_getImplementationName(),
                ::svx::ExtrusionDepthController_createInstance,
                ::svx::ExtrusionDepthController_getSupportedServiceNames() );
        }
        else if( ::svx::ExtrusionDirectionControl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                ::svx::ExtrusionDirectionControl_getImplementationName(),
                ::svx::ExtrusionDirectionControl_createInstance,
                ::svx::ExtrusionDirectionControl_getSupportedServiceNames() );
        }
        else if( ::svx::ExtrusionLightingControl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                ::svx::ExtrusionLightingControl_getImplementationName(),
                ::svx::ExtrusionLightingControl_createInstance,
                ::svx::ExtrusionLightingControl_getSupportedServiceNames() );
        }
        else if( ::svx::ExtrusionSurfaceControl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                ::svx::ExtrusionSurfaceControl_getImplementationName(),
                ::svx::ExtrusionSurfaceControl_createInstance,
                ::svx::ExtrusionSurfaceControl_getSupportedServiceNames() );
        }
        else if( ::svx::FontworkAlignmentControl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                ::svx::FontworkAlignmentControl_getImplementationName(),
                ::svx::FontworkAlignmentControl_createInstance,
                ::svx::FontworkAlignmentControl_getSupportedServiceNames() );
        }
        else if( ::svx::FontworkCharacterSpacingControl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                ::svx::FontworkCharacterSpacingControl_getImplementationName(),
                ::svx::FontworkCharacterSpacingControl_createInstance,
                ::svx::FontworkCharacterSpacingControl_getSupportedServiceNames() );
        }       if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}
