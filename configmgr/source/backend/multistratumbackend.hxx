/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: multistratumbackend.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_BACKEND_MULTISTRATUMBACKEND_HXX_
#define CONFIGMGR_BACKEND_MULTISTRATUMBACKEND_HXX_

#include <com/sun/star/configuration/backend/XBackend.hpp>
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>
#include <com/sun/star/configuration/backend/XVersionedSchemaSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/backend/BackendSetupException.hpp>
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#include <com/sun/star/configuration/backend/XBackendChangesListener.hpp>
#include <cppuhelper/compbase7.hxx>

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr { namespace backend {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backenduno = css::configuration::backend ;

/**
  Class implementing the Backend service for multibackend access.
  It creates the required backends and coordinates access to them.
  */
class MultiStratumBackend : public cppu::WeakComponentImplHelper7< backenduno::XBackend, backenduno::XBackendEntities, backenduno::XVersionedSchemaSupplier, backenduno::XBackendChangesNotifier, backenduno::XBackendChangesListener, lang::XInitialization, lang::XServiceInfo > {
    public :
        /**
          Service constructor from a service factory.

          @param xContext   component context
          */
        explicit
        MultiStratumBackend(
                const uno::Reference<uno::XComponentContext>& xContext) ;
        /** Destructor  */
        ~MultiStratumBackend() ;

        // XInitialize
        virtual void SAL_CALL initialize(
                                const uno::Sequence<uno::Any>& aParameters)
            throw (uno::RuntimeException, uno::Exception,
                   css::configuration::InvalidBootstrapFileException,
                   backenduno::BackendSetupException) ;

        // XVersionedSchemaSupplier
        virtual rtl::OUString
            SAL_CALL getSchemaVersion(const rtl::OUString& aComponent)
                throw (backenduno::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        // XSchemaSupplier
        virtual uno::Reference<backenduno::XSchema>
            SAL_CALL getComponentSchema(const rtl::OUString& aComponent)
                throw (backenduno::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        // XBackend
        virtual uno::Sequence<uno::Reference<backenduno::XLayer> >
            SAL_CALL listOwnLayers(const rtl::OUString& aComponent)
                throw (backenduno::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        virtual uno::Reference<backenduno::XUpdateHandler>
            SAL_CALL getOwnUpdateHandler(const rtl::OUString& aComponent)
            throw (backenduno::BackendAccessException,
                    lang::IllegalArgumentException,
                    lang::NoSupportException,
                    uno::RuntimeException) ;
        virtual uno::Sequence<uno::Reference<backenduno::XLayer> > SAL_CALL
            listLayers(const rtl::OUString& aComponent,
                       const rtl::OUString& aEntity)
            throw (backenduno::BackendAccessException,
                    lang::IllegalArgumentException,
                    uno::RuntimeException) ;
        virtual uno::Reference<backenduno::XUpdateHandler> SAL_CALL
            getUpdateHandler(const rtl::OUString& aComponent,
                             const rtl::OUString& aEntity)
            throw (backenduno::BackendAccessException,
                    lang::IllegalArgumentException,
                    lang::NoSupportException,
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
                throw (backenduno::BackendAccessException, uno::RuntimeException);

        virtual sal_Bool SAL_CALL
            isEqualEntity( const rtl::OUString& aEntity, const rtl::OUString& aOtherEntity )
                throw ( backenduno::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName()
            throw (uno::RuntimeException) ;
        virtual sal_Bool SAL_CALL supportsService(
                                            const rtl::OUString& aServiceName)
            throw (uno::RuntimeException) ;
        virtual uno::Sequence<rtl::OUString> SAL_CALL
            getSupportedServiceNames(void) throw (uno::RuntimeException) ;

        // XBackendChangesNotifier
        virtual void SAL_CALL addChangesListener( const uno::Reference<backenduno::XBackendChangesListener>& xListner,
                                                  const rtl::OUString& aComponent)
            throw (::com::sun::star::uno::RuntimeException);


        virtual void SAL_CALL removeChangesListener( const uno::Reference<backenduno::XBackendChangesListener>& xListner,
                                                     const rtl::OUString& aComponent)
            throw (::com::sun::star::uno::RuntimeException);

       // XBackendChangesListener
        virtual void SAL_CALL componentDataChanged(const backenduno::ComponentChangeEvent& aEvent)
            throw (::com::sun::star::uno::RuntimeException);

        virtual void SAL_CALL disposing( lang::EventObject const & rSource )
           throw (uno::RuntimeException);

        void notifyListeners(const backenduno::ComponentChangeEvent& aEvent) const;
  protected:
    // ComponentHelper
        virtual void SAL_CALL disposing();
   private :
        /** Initialize the schema supplier backend

        */
        void initializeSchemaSupplier(const uno::Reference<uno::XComponentContext>& aContext);

        /** Initialize strata(SingleLayer or MultiLayer) backend
        */
        void initializeBackendStrata(const uno::Reference<uno::XComponentContext>& aContext);

        /** Get Layers from Backend Strata
        */
        uno::Sequence<uno::Reference<backenduno::XLayer> >
            searchSupportingStrata(sal_Int32 nNumLayer,
                                   rtl::OUString aEntity,
                                   const rtl::OUString& aComponent)  ;


        /** Find the Stratum that supports the specified Entity
        *   @return Number of Supported Strata
        */
        sal_Int32 findSupportingStratum(const rtl::OUString& aEntity) ;


        /**
            Check state of MultiStratumBackend  -
            @return    true if not disposed/uninitialized
        */
        bool checkOkState();
        /** Service factory */
        uno::Reference<lang::XMultiServiceFactory> mFactory ;
        /** Mutex for resource protection */
        osl::Mutex mMutex ;

        uno::Reference<backenduno::XSchemaSupplier> mSchemaSupplier ;
        /** list of all backends */
        std::vector< uno::Reference <uno::XInterface> > mBackendStrata;

        rtl::OUString mOwnerEntity;

        /** Helper object that listens to the Strata Backends */
        uno::Reference<backenduno::XBackendChangesListener> mStrataListener;
        /** List of higher level listeners */
        typedef std::multimap< rtl::OUString, uno::Reference<backenduno::XBackendChangesListener> > ListenerList;
        ListenerList mListenerList;
} ;

} } // configmgr.backend

#endif // CONFIGMGR_BACKEND_MULTISTRATUMBACKEND_HXX_
