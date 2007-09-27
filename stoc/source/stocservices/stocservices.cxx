/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stocservices.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 13:04:09 $
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

#include "stocservices.hxx"

#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "sal/types.h"
#include "uno/environment.h"
#include "uno/lbnames.h"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace stoc_services;

rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

static struct ImplementationEntry g_entries[] =
{
    // typeconv
    {
    TypeConverter_Impl_CreateInstance, tcv_getImplementationName,
    tcv_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    // uriproc
    {
    ExternalUriReferenceTranslator::create,
    ExternalUriReferenceTranslator::getImplementationName,
    ExternalUriReferenceTranslator::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    UriReferenceFactory::create,
    UriReferenceFactory::getImplementationName,
    UriReferenceFactory::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    UriSchemeParser_vndDOTsunDOTstarDOTexpand::create,
    UriSchemeParser_vndDOTsunDOTstarDOTexpand::getImplementationName,
    UriSchemeParser_vndDOTsunDOTstarDOTexpand::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    UriSchemeParser_vndDOTsunDOTstarDOTscript::create,
    UriSchemeParser_vndDOTsunDOTstarDOTscript::getImplementationName,
    UriSchemeParser_vndDOTsunDOTstarDOTscript::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    VndSunStarPkgUrlReferenceFactory::create,
    VndSunStarPkgUrlReferenceFactory::getImplementationName,
    VndSunStarPkgUrlReferenceFactory::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
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
