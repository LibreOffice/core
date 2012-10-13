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


#include <localebackend.hxx>

#include <cppuhelper/implementationentry.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace uno = com::sun::star::uno ;

//------------------------------------------------------------------------------

static uno::Reference<uno::XInterface> SAL_CALL createLocaleBackend(
    SAL_UNUSED_PARAMETER const uno::Reference<uno::XComponentContext>&){

    return * LocaleBackend::createInstance();
}

//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createLocaleBackend,
        LocaleBackend::getBackendName,
        LocaleBackend::getBackendServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;

//------------------------------------------------------------------------------

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL localebe1_component_getFactory(
    const sal_Char *aImplementationName,
    void *aServiceManager,
    void *aRegistryKey) {

    return cppu::component_getFactoryHelper(
        aImplementationName,
        aServiceManager,
        aRegistryKey,
        kImplementations_entries) ;
}

//------------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
