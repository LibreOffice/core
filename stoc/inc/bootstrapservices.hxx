/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


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
    ::com::sun::star::uno::Sequence< OUString > smgr_wrapper_getSupportedServiceNames();
    OUString smgr_wrapper_getImplementationName();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL OServiceManagerWrapper_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext )
    throw (::com::sun::star::uno::Exception);
    ::com::sun::star::uno::Sequence< OUString > smgr_getSupportedServiceNames();
    OUString smgr_getImplementationName();
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ORegistryServiceManager_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > & xContext )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< OUString > regsmgr_getSupportedServiceNames();
    OUString regsmgr_getImplementationName();

    //security
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ac_create(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xComponentContext )
    SAL_THROW( (::com::sun::star::uno::Exception) );
    ::com::sun::star::uno::Sequence< OUString > ac_getSupportedServiceNames() SAL_THROW(());
    OUString ac_getImplementationName() SAL_THROW(());

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL filepolicy_create(
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xComponentContext )
    SAL_THROW( (com::sun::star::uno::Exception) );
    ::com::sun::star::uno::Sequence< OUString > filepolicy_getSupportedServiceNames() SAL_THROW(());
    OUString filepolicy_getImplementationName() SAL_THROW(());

    //defaultregistry
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL NestedRegistry_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& )
    throw( ::com::sun::star::uno::Exception);
    ::com::sun::star::uno::Sequence< OUString > defreg_getSupportedServiceNames();
    OUString defreg_getImplementationName();

    //simpleregistry
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SimpleRegistry_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );
    ::com::sun::star::uno::Sequence< OUString > simreg_getSupportedServiceNames();
    OUString simreg_getImplementationName();

    //implementationregistry
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL ImplementationRegistration_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& ); // throw(Exception)
    ::com::sun::star::uno::Sequence< OUString > impreg_getSupportedServiceNames();
    OUString impreg_getImplementationName();

    //loader
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL DllComponentLoader_CreateInstance(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& )
    throw(::com::sun::star::uno::Exception);
    ::com::sun::star::uno::Sequence< OUString > loader_getSupportedServiceNames();
    OUString loader_getImplementationName();
} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
