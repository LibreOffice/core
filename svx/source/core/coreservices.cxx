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

extern OUString SAL_CALL FontWorkAlignmentControl_getImplementationName();
extern Reference< XInterface > SAL_CALL FontWorkAlignmentControl_createInstance(const Reference< XMultiServiceFactory > &)  throw( RuntimeException );
extern Sequence< OUString > SAL_CALL FontWorkAlignmentControl_getSupportedServiceNames() throw( RuntimeException );

extern OUString SAL_CALL FontWorkCharacterSpacingControl_getImplementationName();
extern Reference< XInterface > SAL_CALL FontWorkCharacterSpacingControl_createInstance(const Reference< XMultiServiceFactory > &)  throw( RuntimeException );
extern Sequence< OUString > SAL_CALL FontWorkCharacterSpacingControl_getSupportedServiceNames() throw( RuntimeException );
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
        else if( ::svx::FontWorkAlignmentControl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                ::svx::FontWorkAlignmentControl_getImplementationName(),
                ::svx::FontWorkAlignmentControl_createInstance,
                ::svx::FontWorkAlignmentControl_getSupportedServiceNames() );
        }
        else if( ::svx::FontWorkCharacterSpacingControl_getImplementationName().equalsAscii( pImplName ) )
        {
            xFactory = ::cppu::createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                ::svx::FontWorkCharacterSpacingControl_getImplementationName(),
                ::svx::FontWorkCharacterSpacingControl_createInstance,
                ::svx::FontWorkCharacterSpacingControl_getSupportedServiceNames() );
        }       if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
