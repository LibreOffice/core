/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: backendaccess.hxx,v $
 * $Revision: 1.14 $
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

#ifndef CONFIGMGR_BACKEND_BACKENDACCESS_HXX_
#define CONFIGMGR_BACKEND_BACKENDACCESS_HXX_

#include "backendnotifier.hxx"
#include "mergeddataprovider.hxx"
#include "mergedcomponentdata.hxx"
#include "matchlocale.hxx"
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/configuration/backend/XSchema.hpp>
#include <com/sun/star/configuration/backend/XBackend.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "binarycache.hxx"
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>


namespace configmgr { namespace backend {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backenduno = css::configuration::backend ;

/**
  Implementation of IMergedDataProvider handling the access
  to the configuration data.
  */
class BackendAccess : public IMergedDataProvider
{
    public :
        /**
          Constructor using an XBackend implementation and a
          service factory.

          @param xBackend   backend used for access to data
          @param xContext   uno context for instantiation of services
          */
        BackendAccess(  const uno::Reference<backenduno::XBackend>& xBackend,
                        const uno::Reference<uno::XComponentContext>& xContext) ;
        /** Destructor */
        ~BackendAccess(void) ;

        // IMergedDataProvider
        virtual ResultHolder< ComponentInstance > getNodeData(const ComponentRequest& aRequest,
                                            ITemplateDataProvider* aTemplateProvider,
                                            INodeDataListener *aListener = NULL)
            SAL_THROW((com::sun::star::uno::Exception)) ;
        virtual void removeRequestListener(INodeDataListener *aListener,
                                           const ComponentRequest& aRequest)
            SAL_THROW(());
        virtual void updateNodeData(const UpdateRequest& aUpdate)
            SAL_THROW((com::sun::star::uno::Exception)) ;
        virtual ResultHolder< NodeInstance > getDefaultData(const NodeRequest& aRequest)
            SAL_THROW((com::sun::star::uno::Exception)) ;
        virtual ResultHolder< TemplateInstance > getTemplateData(const TemplateRequest& aRequest)
            SAL_THROW((com::sun::star::uno::Exception)) ;

    private :

        /**
          Retrieves the schema of a component.
          */
        uno::Reference< backenduno::XSchema > getSchema(const rtl::OUString& aComponent) ;

        /**
          Retrieves the schema version of a component.
          */
        rtl::OUString getSchemaVersion(const rtl::OUString& aComponent) ;

        /**
          Retrieves the layers for a request.
          */
        uno::Sequence< uno::Reference<backenduno::XLayer> > getLayers(const rtl::OUString& aComponent,const RequestOptions& aOptions) ;

        /**
          Reads merged default data with a given number of layers.
          */
        bool readDefaultData( MergedComponentData & aComponentData,
                                rtl::OUString const & aComponent,
                                RequestOptions const & aOptions,
                                bool bIncludeTemplates,
                                const uno::Reference<backenduno::XLayer> * pLayers,
                                sal_Int32 nNumLayers,
                                ITemplateDataProvider *aTemplateProvider,
                                sal_Int32 * pLayersMerged = 0)
            SAL_THROW((com::sun::star::uno::Exception));
        /**
          Merges layers onto component data.
          */
        void merge(
                MergedComponentData& aData,
                const uno::Reference<backenduno::XLayer> * pLayers,
                sal_Int32 aNumLayers,
                com::sun::star::lang::Locale const & aRequestedLocale,
                std::vector< com::sun::star::lang::Locale > & inoutMergedLocales,
                ITemplateDataProvider *aTemplateProvider,
                sal_Int32 * pLayersMerged = 0)
            SAL_THROW((com::sun::star::uno::Exception));
    private :
        /**
          Decides if merging should be retried after an exception.

          @throws com::sun::star::uno::Exception
            if not approved
          */
        bool approveRecovery(
                const uno::Any & aMergeException,
                const uno::Reference<backenduno::XLayer>  & aBrokenLayer,
                bool bUserLayerData)
            SAL_THROW((com::sun::star::uno::Exception));

    private :
        /** Get the factory used for service invocation */
        uno::Reference<lang::XMultiServiceFactory> getServiceFactory() const;
        /** UNO context to which this backend belongs */
        uno::Reference<uno::XComponentContext> mContext ;
        /** Backend being accessed */
        uno::Reference<backenduno::XBackend> mBackend ;
        /** Binary cache of default data */
        BinaryCache mBinaryCache;
        /** Manages Nofification from the Backends */
        uno::Reference<backenduno::XBackendChangesListener>  mXNotifier;

        BackendChangeNotifier * mNotifier;

} ;

} } // configmgr.backend

#endif // CONFIGMGR_BACKEND_BACKENDACCESS_HXX_
