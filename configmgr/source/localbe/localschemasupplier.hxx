/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: localschemasupplier.hxx,v $
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

#ifndef CONFIGMGR_LOCALBE_LOCALSCHEMASUPPLIER_HXX_
#define CONFIGMGR_LOCALBE_LOCALSCHEMASUPPLIER_HXX_

#include <com/sun/star/configuration/backend/XVersionedSchemaSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#include <cppuhelper/compbase3.hxx>

namespace configmgr { namespace localbe {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;

/**
  Implements the SchemaSupplier service for local schema file access.
  */
class LocalSchemaSupplier : public cppu::WeakComponentImplHelper3<backend::XVersionedSchemaSupplier, lang::XInitialization, lang::XServiceInfo> {
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
