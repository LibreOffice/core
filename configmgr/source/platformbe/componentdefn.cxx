/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: componentdefn.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:28:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKEND_SYSTEMINTEGRATIONMANAGER_HXX_
#include "systemintegrationmanager.hxx"
#endif //CONFIGMGR_BACKEND_SYSTEMINTEGRATIONMANAGER_HXX_
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif // _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_


using namespace configmgr::backend ;

//==============================================================================

static uno::Reference<uno::XInterface> SAL_CALL createSystemIntegrationManager(
                   const uno::Reference<uno::XComponentContext>& aContext) {
    return * new SystemIntegrationManager(aContext) ;
}
//==============================================================================

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createSystemIntegrationManager,
        SystemIntegrationManager::getSystemIntegrationManagerName,
        SystemIntegrationManager::getServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;
//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment ** /* ppEnv */
            )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL component_writeInfo(void *aServiceManager,
                                                 void *aRegistryKey) {
    return cppu::component_writeInfoHelper(aServiceManager,
                                           aRegistryKey,
                                           kImplementations_entries) ;
}
//------------------------------------------------------------------------------

extern "C" void *component_getFactory(const sal_Char *aImplementationName,
                                      void *aServiceManager,
                                      void *aRegistryKey) {
    return cppu::component_getFactoryHelper(aImplementationName,
                                            aServiceManager,
                                            aRegistryKey,
                                            kImplementations_entries) ;
}
//------------------------------------------------------------------------------

