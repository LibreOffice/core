/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localsinglebackend.hxx,v $
 * $Revision: 1.12 $
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

#ifndef CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_
#define CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_

#include <com/sun/star/configuration/backend/XSchemaSupplier.hpp>
#include <com/sun/star/configuration/backend/XMultiLayerStratum.hpp>
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#include <cppuhelper/compbase5.hxx>

namespace configmgr { namespace localbe {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

/**
  Implements the SingleBackend service for local file access.
  Layer identifiers in that backend are file URLs.
  */
class LocalSingleBackend : public cppu::WeakComponentImplHelper5<backend::XSchemaSupplier, backend::XMultiLayerStratum, backend::XBackendEntities, lang::XInitialization, lang::XServiceInfo> {
    public :
        /**
          Service constructor from a service factory.

          @param xFactory   service factory
          */
        LocalSingleBackend(const uno::Reference<uno::XComponentContext>& xContext) ;

        /** Destructor */
        ~LocalSingleBackend(void) ;


        // XInitialize
        virtual void SAL_CALL
            initialize( const uno::Sequence<uno::Any>& aParameters)
                throw (uno::RuntimeException, uno::Exception,
                       css::configuration::InvalidBootstrapFileException,
                       backend::CannotConnectException,
                       backend::BackendSetupException);

        // XSchemaSupplier
        virtual uno::Reference<backend::XSchema> SAL_CALL
            getComponentSchema( const rtl::OUString& aComponent )
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        // XMultiLayerStratum
        virtual uno::Sequence<rtl::OUString> SAL_CALL
            listLayerIds( const rtl::OUString& aComponent, const rtl::OUString& aEntity )
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        virtual rtl::OUString SAL_CALL
            getUpdateLayerId( const rtl::OUString& aComponent, const rtl::OUString& aEntity )
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        virtual uno::Reference<backend::XLayer> SAL_CALL
            getLayer( const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp )
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        virtual uno::Sequence<uno::Reference<backend::XLayer> > SAL_CALL
            getLayers(const uno::Sequence<rtl::OUString>& aLayerIds,
                      const rtl::OUString& aTimestamp)
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        virtual uno::Sequence<uno::Reference<backend::XLayer> > SAL_CALL
            getMultipleLayers(const uno::Sequence<rtl::OUString>& aLayerIds,
                              const uno::Sequence<rtl::OUString>& aTimestamps)
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL
            getUpdatableLayer( const rtl::OUString& aLayerId )
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        // XBackendEntities
        virtual rtl::OUString SAL_CALL
            getOwnerEntity(  )
                throw (uno::RuntimeException);

        virtual rtl::OUString SAL_CALL
            getAdminEntity(  )
                throw (uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            supportsEntity( const rtl::OUString& aEntity )
                throw (backend::BackendAccessException, uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            isEqualEntity( const rtl::OUString& aEntity, const rtl::OUString& aOtherEntity )
                throw (backend::BackendAccessException, lang::IllegalArgumentException, uno::RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL
            getImplementationName(  )
                throw (uno::RuntimeException) ;

        virtual sal_Bool SAL_CALL
            supportsService( const rtl::OUString& aServiceName )
                throw (uno::RuntimeException) ;

        virtual uno::Sequence<rtl::OUString> SAL_CALL
            getSupportedServiceNames(  )
                throw (uno::RuntimeException) ;

    public: // helpers for other implementation that need to use the same data
        /**
          Locates the main layer data and localized data directories in a layer directory hierarchy
          */
        static bool getLayerSubDirectories( rtl::OUString const & aLayerBaseUrl,
                                             rtl::OUString& aMainLayerUrl,
                                             rtl::OUString& aSubLayerUrl);
        /**
          Creates a simple readonly non-composite layer for a component in a base directory
          */
        static uno::Reference<backend::XLayer>
            createSimpleLayer(const uno::Reference<lang::XMultiServiceFactory>& xFactory,
                              rtl::OUString const & aLayerBaseUrl,
                              rtl::OUString const & aComponent);

        /**
          Creates a simple readonly non-composite layer for a component in a given file
          */
        static uno::Reference<backend::XLayer>
            createSimpleLayer(const uno::Reference<lang::XMultiServiceFactory>& xFactory,
                              rtl::OUString const & aComponentUrl);
    private :
        /** Service factory */
        uno::Reference<lang::XMultiServiceFactory> mFactory ;
        /** Mutex for resources protection */
        osl::Mutex mMutex ;
        /**
          Base of the schema data. Is a list to allow
          for multiple schema directories.
          */
        uno::Sequence<rtl::OUString> mSchemaDataUrls ;
        /**
          Base of the default data. Is a list to allow
          for multiple layers of default data.
          */
        uno::Sequence<rtl::OUString> mDefaultDataUrls ;
        /** Base of the user data */
        rtl::OUString mUserDataUrl ;

        /** special index for entity */
        sal_Int32 findEntity(rtl::OUString const & _aEntity);
        /** parse and translate layer-id */
        sal_Int32 resolveLayerId(rtl::OUString const & _aLayerId, rtl::OUString & _aFile);

        /**
          Builds a LocalFileLayer object given a layer id.
          Since the LocalFileLayer implements the various
          interfaces a layer can be accessed as, a few methods
          need one. This method handles the layer id mapping
          and the existence or not of sublayers.

          @param aLayerId   layer id
          @return   local file layer
          @throws com::sun::star::lang::IllegalArgumentException
                  if the layer id is invalid.
          */
        uno::Reference<backend::XUpdatableLayer> getFileLayer(const rtl::OUString& aLayerId)
            throw (lang::IllegalArgumentException) ;
        /**
          Same as above, but using a component URL and layer index
          combination instead of a layer id (which encodes both).

          @param aComponent     component URL
          @param aLayerIndex    layer index
          @return   local file layer
          */
        uno::Reference<backend::XUpdatableLayer> getFileLayer(const rtl::OUString& aComponent,
                                                                sal_Int32 aLayerIndex) ;
        /**
          Maps a layer index (-1 for user layer, 0-x for defaults)
          to the appropriate layer and sublayers base directories.

          @param aLayerIndex    layer index
          @param aLayerUrl      layer base URL, filled on return
          @param aSubLayerUrl   sublayer base URL, filled on return
          */
        bool getLayerDirectories(sal_Int32 aLayerIndex,
                                 rtl::OUString& aLayerUrl,
                                 rtl::OUString& aSubLayerUrl) ;
        /**
          Tells if a file is more recent than a given date.
          The date is formatted YYYYMMDDhhmmssZ.

          @param aComponent     URL of the component to check
          @param aLayerIndex    index of the layer involved (-1 = user)
          @param aTimestamp     timestamp to check against
          @return   sal_True if the file is more recent, sal_False otherwise
          */
        sal_Bool isMoreRecent(const rtl::OUString& aComponent,
                              sal_Int32 aLayerId,
                              const rtl::OUString& aTimestamp) ;
    } ;

} } // configmgr.localbe

#endif // CONFIGMGR_LOCALBE_LOCALSINGLEBACKEND_HXX_
