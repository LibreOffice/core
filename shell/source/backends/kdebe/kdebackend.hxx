/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: kdebackend.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2006-08-04 12:30:26 $
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

#ifndef KDEBACKEND_HXX_
#define KDEBACKEND_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSCHEMASUPPLIER_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_INVALIDBOOTSTRAPFILEEXCEPTION_HPP_
#include <com/sun/star/configuration/InvalidBootstrapFileException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_CANNOTCONNECTEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif

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
