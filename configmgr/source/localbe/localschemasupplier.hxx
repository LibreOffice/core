/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localschemasupplier.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:06:02 $
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

#ifndef CONFIGMGR_LOCALBE_LOCALSCHEMASUPPLIER_HXX_
#define CONFIGMGR_LOCALBE_LOCALSCHEMASUPPLIER_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XVERSIONEDSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XVersionedSchemaSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif // _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif // _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif // _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_INVALIDBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_CANNOTCONNECTEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif // _CPPUHELPER_COMPBASE3_HXX_

namespace configmgr { namespace localbe {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

typedef cppu::WeakComponentImplHelper3<backend::XVersionedSchemaSupplier,
                                       lang::XInitialization,
                                       lang::XServiceInfo> SingleBackendBase ;

/**
  Implements the SchemaSupplier service for local schema file access.
  */
class LocalSchemaSupplier : public SingleBackendBase {
    public :
        /**
          Service constructor from a service factory.

          @param xConxtext   Component Context
          */
        LocalSchemaSupplier(const uno::Reference<uno::XComponentContext>& xContext) ;

        /** Destructor */
        ~LocalSchemaSupplier(void) ;


        // XInitialize
        virtual void SAL_CALL
            initialize( const uno::Sequence<uno::Any>& aParameters)
                throw (uno::RuntimeException, uno::Exception,
                       css::configuration::InvalidBootstrapFileException,
                       backend::CannotConnectException,
                       backend::BackendSetupException);

        // XVersionedSchemaSupplier
        virtual rtl::OUString SAL_CALL
            getSchemaVersion( const rtl::OUString& aComponent )
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

        // XSchemaSupplier
        virtual uno::Reference<backend::XSchema> SAL_CALL
            getComponentSchema( const rtl::OUString& aComponent )
                throw (backend::BackendAccessException,
                        lang::IllegalArgumentException,
                        uno::RuntimeException) ;

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

        /**
          Provides the implementation name.

          @return   implementation name
          */
        static rtl::OUString SAL_CALL getName(void) ;
        /**
          Provides the supported services names

          @return   service names
          */
        static uno::Sequence<rtl::OUString> SAL_CALL getServices(void) ;

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
        /** Version of the schema repository */
        // TODO: Add support for repository-specific versions
        rtl::OUString mSchemaVersion;
} ;

} } // configmgr.localschemasupplirt

#endif // CONFIGMGR_LOCALBE_LOCALSCHEMASUPPLIER_HXX_
