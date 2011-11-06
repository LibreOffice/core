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
#include "precompiled_stoc.hxx"

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>

namespace com { namespace sun { namespace star { namespace uno {
    class XInterface;
    class XComponentContext;
} } } }

namespace stoc_bootstrap
{
    //servicemanager
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL OServiceManager_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > smgr_wrapper_getSupportedServiceNames();
    ::rtl::OUString smgr_wrapper_getImplementationName();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL OServiceManagerWrapper_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext )
    throw (::com::sun::star::uno::Exception);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > smgr_getSupportedServiceNames();
    ::rtl::OUString smgr_getImplementationName();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ORegistryServiceManager_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > regsmgr_getSupportedServiceNames();
    ::rtl::OUString regsmgr_getImplementationName();

    //security
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ac_create(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xComponentContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > ac_getSupportedServiceNames() SAL_THROW( () );
    ::rtl::OUString ac_getImplementationName() SAL_THROW( () );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL filepolicy_create(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xComponentContext )
    SAL_THROW( (com::sun::star::uno::Exception) );
    ::com::sun::star::uno::Sequence< rtl::OUString > filepolicy_getSupportedServiceNames() SAL_THROW( () );
    ::rtl::OUString filepolicy_getImplementationName() SAL_THROW( () );

    //defaultregistry
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL NestedRegistry_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& )
    throw( ::com::sun::star::uno::Exception);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > defreg_getSupportedServiceNames();
    ::rtl::OUString defreg_getImplementationName();

    //simpleregistry
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SimpleRegistry_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > simreg_getSupportedServiceNames();
    ::rtl::OUString simreg_getImplementationName();

    //implementationregistry
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplementationRegistration_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& ); // throw(Exception)
    ::com::sun::star::uno::Sequence< ::rtl::OUString > impreg_getSupportedServiceNames();
    ::rtl::OUString impreg_getImplementationName();

    //loader
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL DllComponentLoader_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& )
    throw(::com::sun::star::uno::Exception);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > loader_getSupportedServiceNames();
    ::rtl::OUString loader_getImplementationName();

    //registry_tdprovider
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ProviderImpl_create(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& )
    throw(::com::sun::star::uno::Exception);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > rdbtdp_getSupportedServiceNames();
    ::rtl::OUString rdbtdp_getImplementationName();

    //tdmanager
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ManagerImpl_create(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& )
    SAL_THROW( (::com::sun::star::uno::Exception) );
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL tdmgr_getSupportedServiceNames();
    ::rtl::OUString SAL_CALL tdmgr_getImplementationName();
} // namespace
