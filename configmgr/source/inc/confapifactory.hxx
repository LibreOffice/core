/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: confapifactory.hxx,v $
 * $Revision: 1.19 $
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

#ifndef CONFIGMGR_API_FACTORY_HXX_
#define CONFIGMGR_API_FACTORY_HXX_

#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace uno
    {
        class XInterface;
        template <class> class Reference;
        class XComponentContext;
    }
    namespace lang
    {
        class XSingleComponentFactory;
    }
} } }
namespace rtl { class OUString; }

namespace configmgr
{
    namespace uno  = ::com::sun::star::uno;
    namespace lang = ::com::sun::star::lang;

    extern
    uno::Reference< lang::XSingleComponentFactory > SAL_CALL
        createProviderFactory(
            ::rtl::OUString const & aImplementationName,
            bool bAdmin
        )
        SAL_THROW( () );

    struct ServiceRegistrationInfo;
    struct SingletonRegistrationInfo;

// provider instantiation
    uno::Reference<uno::XInterface> SAL_CALL
        getDefaultConfigProviderSingleton( uno::Reference< uno::XComponentContext > const& xContext );

    uno::Reference<uno::XInterface> SAL_CALL
        instantiateDefaultProvider( uno::Reference< uno::XComponentContext > const& xContext );

// provider service info
    const SingletonRegistrationInfo * getDefaultProviderSingletonInfo();
    const ServiceRegistrationInfo   * getConfigurationProviderServiceInfo();
    const ServiceRegistrationInfo   * getDefaultProviderServiceInfo();
    const ServiceRegistrationInfo   * getAdminProviderServiceInfo();

// other services - instantiation and info
    uno::Reference< uno::XInterface > SAL_CALL
        instantiateConfigRegistry( uno::Reference< uno::XComponentContext > const& xContext );

    const ServiceRegistrationInfo* getConfigurationRegistryServiceInfo();

// bootstrap context support
    uno::Reference<uno::XInterface> SAL_CALL
        instantiateBootstrapContext( uno::Reference< uno::XComponentContext > const& xContext );

    const SingletonRegistrationInfo * getBootstrapContextSingletonInfo();
    const ServiceRegistrationInfo   * getBootstrapContextServiceInfo();

    namespace xml
    {
        uno::Reference< uno::XInterface > SAL_CALL instantiateSchemaParser
        ( uno::Reference< uno::XComponentContext > const& xContext );
        uno::Reference< uno::XInterface > SAL_CALL instantiateLayerParser
        ( uno::Reference< uno::XComponentContext > const& xContext );
        uno::Reference< uno::XInterface > SAL_CALL instantiateLayerWriter
        ( uno::Reference< uno::XComponentContext > const& xContext );

        const ServiceRegistrationInfo* getSchemaParserServiceInfo();
        const ServiceRegistrationInfo* getLayerParserServiceInfo();
        const ServiceRegistrationInfo* getLayerWriterServiceInfo();
    }
    namespace backend
    {
        uno::Reference<uno::XInterface> SAL_CALL
            getDefaultBackendSingleton( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateDefaultBackend( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference< uno::XInterface > SAL_CALL
            instantiateUpdateMerger( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateSingleBackendAdapter( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference< uno::XInterface > SAL_CALL
            instantiateMergeImporter( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference< uno::XInterface > SAL_CALL
            instantiateCopyImporter( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateMultiStratumBackend( uno::Reference< uno::XComponentContext > const& xContext );

        const SingletonRegistrationInfo * getDefaultBackendSingletonInfo();
        const ServiceRegistrationInfo   * getDefaultBackendServiceInfo();

        const ServiceRegistrationInfo * getUpdateMergerServiceInfo();
        const ServiceRegistrationInfo * getSingleBackendAdapterServiceInfo();
        const ServiceRegistrationInfo * getMergeImportServiceInfo();
        const ServiceRegistrationInfo * getCopyImportServiceInfo();
        const ServiceRegistrationInfo * getMultiStratumBackendServiceInfo();
    }
    namespace localbe
    {
        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalBackend( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalDataImporter( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalHierarchyBrowser( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalSchemaSupplier( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalLegacyStratum( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalDataStratum( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalReadonlyStratum( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalResourceStratum( uno::Reference< uno::XComponentContext > const& xContext );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalMultiStratum( uno::Reference< uno::XComponentContext > const& xContext );

        const ServiceRegistrationInfo * getLocalBackendServiceInfo();
        const ServiceRegistrationInfo * getLocalDataImportServiceInfo();
        const ServiceRegistrationInfo * getLocalHierarchyBrowserServiceInfo();
        const ServiceRegistrationInfo * getLocalSchemaSupplierServiceInfo();
        const ServiceRegistrationInfo * getLocalLegacyStratumServiceInfo();
        const ServiceRegistrationInfo * getLocalDataStratumServiceInfo();
        const ServiceRegistrationInfo * getLocalReadonlyStratumServiceInfo();
        const ServiceRegistrationInfo * getLocalResourceStratumServiceInfo();
        const ServiceRegistrationInfo * getLocalMultiStratumServiceInfo();
    } // localbe

} //  namespace configmgr

#endif // CONFIGMGR_API_FACTORY_HXX_


