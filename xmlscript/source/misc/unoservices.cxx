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

#include <cppuhelper/implementationentry.hxx>

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace xmlscript
{
    Sequence< OUString > SAL_CALL getSupportedServiceNames_DocumentHandlerImpl();
    OUString SAL_CALL getImplementationName_DocumentHandlerImpl();
    Reference< XInterface > SAL_CALL create_DocumentHandlerImpl(
        Reference< XComponentContext > const & xContext )
            SAL_THROW( (Exception) );

    Sequence< OUString > SAL_CALL getSupportedServiceNames_XMLBasicExporter();
    OUString SAL_CALL getImplementationName_XMLBasicExporter();
    Reference< XInterface > SAL_CALL create_XMLBasicExporter(
        Reference< XComponentContext > const & xContext )
            SAL_THROW( (Exception) );

    Sequence< OUString > SAL_CALL getSupportedServiceNames_XMLOasisBasicExporter();
    OUString SAL_CALL getImplementationName_XMLOasisBasicExporter();
    Reference< XInterface > SAL_CALL create_XMLOasisBasicExporter(
        Reference< XComponentContext > const & xContext )
            SAL_THROW( (Exception) );

    Sequence< OUString > SAL_CALL getSupportedServiceNames_XMLBasicImporter();
    OUString SAL_CALL getImplementationName_XMLBasicImporter();
    Reference< XInterface > SAL_CALL create_XMLBasicImporter(
        Reference< XComponentContext > const & xContext )
            SAL_THROW( (Exception) );

    Sequence< OUString > SAL_CALL getSupportedServiceNames_XMLOasisBasicImporter();
    OUString SAL_CALL getImplementationName_XMLOasisBasicImporter();
    Reference< XInterface > SAL_CALL create_XMLOasisBasicImporter(
        Reference< XComponentContext > const & xContext )
            SAL_THROW( (Exception) );

    static const struct ::cppu::ImplementationEntry s_entries [] =
    {
        {
            create_DocumentHandlerImpl, getImplementationName_DocumentHandlerImpl,
            getSupportedServiceNames_DocumentHandlerImpl, ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            create_XMLBasicExporter, getImplementationName_XMLBasicExporter,
            getSupportedServiceNames_XMLBasicExporter, ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            create_XMLOasisBasicExporter, getImplementationName_XMLOasisBasicExporter,
            getSupportedServiceNames_XMLOasisBasicExporter, ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            create_XMLBasicImporter, getImplementationName_XMLBasicImporter,
            getSupportedServiceNames_XMLBasicImporter, ::cppu::createSingleComponentFactory,
            0, 0
        },
        {
            create_XMLOasisBasicImporter, getImplementationName_XMLOasisBasicImporter,
            getSupportedServiceNames_XMLOasisBasicImporter, ::cppu::createSingleComponentFactory,
            0, 0
        },
        { 0, 0, 0, 0, 0, 0 }
    };
}

extern "C"
{
    SAL_DLLPUBLIC_EXPORT void * SAL_CALL xmlscript_component_getFactory(
        const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, ::xmlscript::s_entries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
