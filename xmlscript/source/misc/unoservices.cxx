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
#include "precompiled_xmlscript.hxx"

#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif

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
    SAL_DLLPUBLIC_EXPORT void SAL_CALL component_getImplementationEnvironment(
        const sal_Char ** ppEnvTypeName, uno_Environment ** )
    {
        *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
    }

    // -----------------------------------------------------------------------------

    SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
        const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
    {
        return ::cppu::component_getFactoryHelper(
            pImplName, pServiceManager, pRegistryKey, ::xmlscript::s_entries );
    }
}
