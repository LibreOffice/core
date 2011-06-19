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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
