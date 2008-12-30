/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: componentdf.cxx,v $
 * $Revision: 1.6 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include "backendlayerhelper.hxx"
#include <com/sun/star/registry/XRegistryKey.hpp>
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif // _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_

//==============================================================================

static com::sun::star::uno::Reference<com::sun::star::uno::XInterface> SAL_CALL createBackendLayerHelper(
                   const com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext>& aContext) {
    return * new configmgr::backendhelper::BackendLayerHelper(aContext) ;
}
//==============================================================================

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createBackendLayerHelper,
        configmgr::backendhelper::BackendLayerHelper::getBackendLayerHelperName,
        configmgr::backendhelper::BackendLayerHelper::getBackendLayerHelperServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;
//------------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment ** /* ppEnv */
            )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}
//------------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_writeInfo(
    void *aServiceManager, void *aRegistryKey)
{
    return cppu::component_writeInfoHelper(aServiceManager,
                                           aRegistryKey,
                                           kImplementations_entries) ;
}
//------------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void *component_getFactory(
    const sal_Char *aImplementationName, void *aServiceManager,
    void *aRegistryKey)
{
    return cppu::component_getFactoryHelper(aImplementationName,
                                            aServiceManager,
                                            aRegistryKey,
                                            kImplementations_entries) ;
}
//------------------------------------------------------------------------------
