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
#include "precompiled_package.hxx"
#include <ManifestReader.hxx>
#include <ManifestWriter.hxx>
#include <cppuhelper/factory.hxx>
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#include <vos/diagnose.hxx>
#include <ZipPackage.hxx>

#include <zipfileaccess.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::packages;
using namespace ::com::sun::star::packages::manifest;

// C functions to implement this as a component

extern "C" void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** /*ppEnv*/ )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/**
 * This function is called to get service factories for an implementation.
 * @param pImplName name of implementation
 * @param pServiceManager generic uno interface providing a service manager to instantiate components
 * @param pRegistryKey registry data key to read and write component persistent data
 * @return a component factory (generic uno interface)
 */
extern "C" void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;
    uno::Reference< XMultiServiceFactory > xSMgr(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );
    uno::Reference< XSingleServiceFactory > xFactory;

    if (ManifestReader::static_getImplementationName().compareToAscii( pImplName ) == 0)
        xFactory = ManifestReader::createServiceFactory ( xSMgr );
    else if (ManifestWriter::static_getImplementationName().compareToAscii( pImplName ) == 0)
        xFactory = ManifestWriter::createServiceFactory ( xSMgr );
    else if (ZipPackage::static_getImplementationName().compareToAscii( pImplName ) == 0)
        xFactory = ZipPackage::createServiceFactory ( xSMgr );
    else if ( OZipFileAccess::impl_staticGetImplementationName().compareToAscii( pImplName ) == 0 )
        xFactory = ::cppu::createSingleFactory( xSMgr,
                                           OZipFileAccess::impl_staticGetImplementationName(),
                                           OZipFileAccess::impl_staticCreateSelfInstance,
                                           OZipFileAccess::impl_staticGetSupportedServiceNames() );

    if ( xFactory.is() )
    {
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}

