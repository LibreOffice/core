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

#include "cppuhelper/implementationentry.hxx"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"

// =============================================================================
// component exports
// =============================================================================
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace evtlstner
{
    // =============================================================================
    // component operations
    // =============================================================================

    uno::Reference< XInterface > SAL_CALL create(
        Reference< XComponentContext > const & xContext )
        SAL_THROW(());

    // -----------------------------------------------------------------------------

    ::rtl::OUString SAL_CALL getImplementationName();

    Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();

    Reference<XInterface> SAL_CALL create(
        Sequence<Any> const &, Reference<XComponentContext> const & );
} // end evtlstner

namespace ooevtdescgen
{
    // =============================================================================
    // component operations
    // =============================================================================

    uno::Reference< XInterface > SAL_CALL create(
        Reference< XComponentContext > const & xContext )
        SAL_THROW(());

    // -----------------------------------------------------------------------------

    ::rtl::OUString SAL_CALL getImplementationName();

    Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames();

    Reference<XInterface> SAL_CALL create(
        Sequence<Any> const &, Reference<XComponentContext> const & );
} // end ooevtdescgen



    // =============================================================================

    const ::cppu::ImplementationEntry s_component_entries [] =
    {
        {
            ::evtlstner::create, ::evtlstner::getImplementationName,
            ::evtlstner::getSupportedServiceNames,
            ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            ::ooevtdescgen::create, ::ooevtdescgen::getImplementationName,
            ::ooevtdescgen::getSupportedServiceNames,
            ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };

extern "C"
{
    SAL_DLLPUBLIC_EXPORT  void * SAL_CALL vbaevents_component_getFactory(
        const sal_Char * pImplName, lang::XMultiServiceFactory * pServiceManager,
        registry::XRegistryKey * pRegistryKey )
    {
        OSL_TRACE("In component_getFactory");
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, s_component_entries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
