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
#include "precompiled_xmlscript.hxx"

#include <cppuhelper/implementationentry.hxx>

using namespace ::rtl;
using namespace ::com::sun::star::uno;


// =============================================================================

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

    // -----------------------------------------------------------------------------

    static struct ::cppu::ImplementationEntry s_entries [] =
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

// =============================================================================

extern "C"
{
    void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    // -----------------------------------------------------------------------------

    void * SAL_CALL component_getFactory(
        const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, ::xmlscript::s_entries );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
