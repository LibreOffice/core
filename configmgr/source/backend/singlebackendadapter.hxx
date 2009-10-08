/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: singlebackendadapter.hxx,v $
 * $Revision: 1.9 $
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

#ifndef CONFIGMGR_BACKEND_SINGLEBACKENDADAPTER_HXX_
#define CONFIGMGR_BACKEND_SINGLEBACKENDADAPTER_HXX_

#include <com/sun/star/configuration/backend/XBackend.hpp>
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>
#include <com/sun/star/configuration/backend/XSchemaSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/compbase5.hxx>

namespace configmgr { namespace backend {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backenduno = css::configuration::backend ;

/**
  Class implementing the Backend service for remote access.
  It just transfers calls to a SingleBackend implementation.
  */
class SingleBackendAdapter : public cppu::WeakComponentImplHelper5< backenduno::XBackend, backenduno::XBackendEntities, backenduno::XSchemaSupplier, lang::XInitialization, lang::XServiceInfo > {
    public :
        /**
          Service constructor from a service factory.

          @param aFactory   service factory
          */
        SingleBackendAdapter(
                const uno::Reference<uno::XComponentContext>& xContext) ;
        /** Destructor  */
        ~SingleBackendAdapter(void) ;

        // XInitialize
        virtual void SAL_CALL initialize(
                                const uno::Sequence<uno::Any>& aParameters)
            throw (uno::RuntimeException, uno::Exception) ;

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
        virtual rtl::OUString SAL_CALL getImplementationName(void)
            throw (uno::RuntimeException) ;
        virtual sal_Bool SAL_CALL supportsService(
                                            const rtl::OUString& aServiceName)
            throw (uno::RuntimeException) ;
        virtual uno::Sequence<rtl::OUString> SAL_CALL
            getSupportedServiceNames(void) throw (uno::RuntimeException) ;
    protected :
    private :
        /** Service factory */
        uno::Reference<lang::XMultiServiceFactory> mFactory ;
        /** Mutex for resource protection */
        osl::Mutex mMutex ;
        /** Remote backend that the offline cache is handling */
        uno::Reference<backenduno::XSchemaSupplier> mBackend ;
        /* checks if object is in initialized and undisposed state */
        bool checkOkState();
} ;

} } // configmgr.backend

#endif // CONFIGMGR_BACKEND_SINGLEBACKENDADAPTER_HXX_
