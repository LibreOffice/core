/*************************************************************************
 *
 *  $RCSfile: confapifactory.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: jb $ $Date: 2002-12-06 13:06:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    }
    namespace lang
    {
        class XMultiServiceFactory;
    }
} } }

namespace configmgr
{
    namespace css = ::com::sun::star;
    namespace uno = css::uno;
    namespace lang = css::lang;
    struct ServiceRegistrationInfo;
    class ConnectionSettings;

    typedef uno::Reference< lang::XMultiServiceFactory > CreationContext;

    typedef uno::Reference< uno::XInterface > (SAL_CALL * ProviderInstantiation)
            (
                CreationContext const& rServiceManager,
                ConnectionSettings const& _rConnectionSettings
            );

// provider instantiation
    uno::Reference< uno::XInterface > SAL_CALL instantiateConfigProvider
        (
            CreationContext const& rServiceManager,
            ConnectionSettings const& _rConnectionSettings
        );

    uno::Reference< uno::XInterface > SAL_CALL instantiateAdminProvider
        (
            CreationContext const& rServiceManager,
            ConnectionSettings const& _rConnectionSettings
        );

    uno::Reference< uno::XInterface > SAL_CALL instantiateLocalAdminProvider
        (
            CreationContext const& rServiceManager,
            ConnectionSettings const& _rConnectionSettings
        );


// provider service info
    const ServiceRegistrationInfo* getConfigurationProviderServices();
    const ServiceRegistrationInfo* getAdminProviderServices();
    const ServiceRegistrationInfo* getLocalAdminProviderServices();

// other services - instantiation and info
    uno::Reference< uno::XInterface > SAL_CALL instantiateConfigRegistry
        ( CreationContext const& rServiceManager );

    const ServiceRegistrationInfo* getConfigurationRegistryServiceInfo();

    namespace xml
    {
        uno::Reference< uno::XInterface > SAL_CALL instantiateSchemaParser
        ( CreationContext const& rServiceManager );
        uno::Reference< uno::XInterface > SAL_CALL instantiateLayerParser
        ( CreationContext const& rServiceManager );
        uno::Reference< uno::XInterface > SAL_CALL instantiateLayerWriter
        ( CreationContext const& rServiceManager );

        const ServiceRegistrationInfo* getSchemaParserServiceInfo();
        const ServiceRegistrationInfo* getLayerParserServiceInfo();
        const ServiceRegistrationInfo* getLayerWriterServiceInfo();
    }
    namespace backend
    {
        uno::Reference< uno::XInterface > SAL_CALL
            instantiateUpdateMerger( CreationContext const& rServiceManager );

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateSingleBackendAdapter(const CreationContext& aServiceManager) ;

        uno::Reference< uno::XInterface > SAL_CALL
            instantiateMergeImporter( CreationContext const& rServiceManager );

        uno::Reference< uno::XInterface > SAL_CALL
            instantiateCopyImporter( CreationContext const& rServiceManager );

        const ServiceRegistrationInfo * getUpdateMergerServiceInfo();
        const ServiceRegistrationInfo * getSingleBackendAdapterServiceInfo() ;
        const ServiceRegistrationInfo * getMergeImportServiceInfo();
        const ServiceRegistrationInfo * getCopyImportServiceInfo();
    }
    namespace localbe {
        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalBackend(const CreationContext& aServiceManager) ;

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalDataImporter(const CreationContext& aServiceManager) ;

        uno::Reference<uno::XInterface> SAL_CALL
            instantiateLocalHierarchyBrowser(const CreationContext& aServiceManager) ;

        const ServiceRegistrationInfo * getLocalBackendServiceInfo() ;
        const ServiceRegistrationInfo * getLocalDataImportServiceInfo() ;
        const ServiceRegistrationInfo * getLocalHierarchyBrowserServiceInfo() ;
    } // localbe
} //  namespace configmgr

#endif // CONFIGMGR_API_FACTORY_HXX_


