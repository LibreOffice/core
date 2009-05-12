/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: kdebackend.hxx,v $
 * $Revision: 1.4 $
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

#ifndef KDEBACKEND_HXX_
#define KDEBACKEND_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#include <cppuhelper/compbase2.hxx>

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;


//------------------------------------------------------------------------------
typedef cppu::WeakComponentImplHelper2<backend::XSingleLayerStratum,
                       lang::XServiceInfo> BackendBase ;

/**
  Implements the SingleLayerStratum service for KDE access.
  */
class KDEBackend : public BackendBase {
    public :

    static KDEBackend* createInstance(const uno::Reference<uno::XComponentContext>& xContext);

        // XServiceInfo
        virtual rtl::OUString SAL_CALL getImplementationName(  )
            throw (uno::RuntimeException) ;

        virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& aServiceName )
            throw (uno::RuntimeException) ;

        virtual uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames(  )
            throw (uno::RuntimeException) ;

        /**
          Provides the implementation name.

          @return   implementation name
          */
        static rtl::OUString SAL_CALL getBackendName(void) ;

        /**
          Provides the supported services names

          @return   service names
          */
        static uno::Sequence<rtl::OUString> SAL_CALL getBackendServiceNames(void) ;

        //XSingleLayerStratum
        virtual uno::Reference<backend::XLayer> SAL_CALL
            getLayer( const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp )
                throw (backend::BackendAccessException, lang::IllegalArgumentException) ;

        virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL
            getUpdatableLayer( const rtl::OUString& aLayerId )
                throw (backend::BackendAccessException, lang::NoSupportException,
                       lang::IllegalArgumentException) ;

    protected:
        /**
          Service constructor from a service factory.

          @param xContext   component context
          */
         KDEBackend(const uno::Reference<uno::XComponentContext>& xContext)
            throw (backend::BackendAccessException);

        /** Destructor */
        ~KDEBackend(void) ;

    private:

        /** Build KDE/OO mapping table */
        void initializeMappingTable ();


        /** The component context */
        uno::Reference<uno::XComponentContext> m_xContext;

        /** Mutex for reOOurces protection */
        osl::Mutex mMutex ;

        static KDEBackend* mInstance;
};

#endif // CONFIGMGR_LOCALBE_LOCALSINGLESTRATUM_HXX_
