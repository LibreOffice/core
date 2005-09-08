/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: singlebackendadapter.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:34:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef CONFIGMGR_BACKEND_SINGLEBACKENDADAPTER_HXX_
#define CONFIGMGR_BACKEND_SINGLEBACKENDADAPTER_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_
#include <com/sun/star/configuration/backend/XBackend.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_
#include <com/sun/star/configuration/backend/XBackendEntities.hpp>

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XSchemaSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif // _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif // _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_

#ifndef _CPPUHELPER_COMPBASE5_HXX_
#include <cppuhelper/compbase5.hxx>
#endif // _CPPUHELPER_COMPBASE4_HXX_

namespace configmgr { namespace backend {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backenduno = css::configuration::backend ;

typedef cppu::WeakComponentImplHelper5< backenduno::XBackend,
                                        backenduno::XBackendEntities,
                                        backenduno::XSchemaSupplier,
                                        lang::XInitialization,
                                        lang::XServiceInfo> BackendBase ;
/**
  Class implementing the Backend service for remote access.
  It just transfers calls to a SingleBackend implementation.
  */
class SingleBackendAdapter : public BackendBase {
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

        /**
          Provides the implementation name.

          @return   implementation name
          */
        static rtl::OUString SAL_CALL getName(void) ;
        /**
          Provides the list of supported services.

          @return   list of service names
          */
        static uno::Sequence<rtl::OUString> SAL_CALL getServices(void) ;
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
