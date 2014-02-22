/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

/** this file contains the uno service registrations for all services in the svxcore lib */


#include "sal/types.h"
#include "osl/diagnose.h"
#include "cppuhelper/factory.hxx"

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

SAL_DLLPUBLIC_EXPORT void * SAL_CALL svxcore_component_getFactory (
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
