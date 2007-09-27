/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: services.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:56:37 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_stoc.hxx"

#include "bootstrapservices.hxx"

#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "sal/types.h"
#include "uno/environment.h"
#include "uno/lbnames.h"

#include <stdio.h>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;

extern rtl_StandardModuleCount g_moduleCount;

using namespace stoc_bootstrap;

static struct ImplementationEntry g_entries[] =
{
    //servicemanager
    {
    OServiceManager_CreateInstance, smgr_getImplementationName,
    smgr_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    {
    ORegistryServiceManager_CreateInstance, regsmgr_getImplementationName,
    regsmgr_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    {
    OServiceManagerWrapper_CreateInstance, smgr_wrapper_getImplementationName,
    smgr_wrapper_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //security
    {
    ac_create, ac_getImplementationName,
    ac_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt, 0
    },
    {
    filepolicy_create, filepolicy_getImplementationName,
    filepolicy_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt, 0
    },
    //simpleregistry
    {
    SimpleRegistry_CreateInstance, simreg_getImplementationName,
    simreg_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //defaultregistry
    {
    NestedRegistry_CreateInstance, defreg_getImplementationName,
    defreg_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //implementationregistry
    {
    ImplementationRegistration_CreateInstance, impreg_getImplementationName,
    impreg_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //loader
    {
    DllComponentLoader_CreateInstance, loader_getImplementationName,
    loader_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //registry_tdprovider
    {
    ProviderImpl_create, rdbtdp_getImplementationName,
    rdbtdp_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //tdmanager
    {
    ManagerImpl_create, tdmgr_getImplementationName,
    tdmgr_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    //end
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{

sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    return component_writeInfoHelper( pServiceManager, pRegistryKey, g_entries );
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}
