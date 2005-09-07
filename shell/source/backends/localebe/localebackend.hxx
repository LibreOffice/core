/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localebackend.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:48:28 $
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

#ifndef _FIXEDVALUEBACKEND_HXX_
#define _FIXEDVALUEBACKEND_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSINGLELAYERSTRATUM_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDCHANGESNOTIFIER_HPP_
#include <com/sun/star/configuration/backend/XBackendChangesNotifier.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif


namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;


/**
  Implements the SingleLayerStratum service.
  */
class LocaleBackend : public ::cppu::WeakImplHelper2 <
        backend::XSingleLayerStratum,
        lang::XServiceInfo > {

    public :

        static LocaleBackend* createInstance(const uno::Reference<uno::XComponentContext>& xContext);

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
        static rtl::OUString SAL_CALL getBackendName(void) ;
        /**
          Provides the supported services names

          @return   service names
          */
        static uno::Sequence<rtl::OUString> SAL_CALL getBackendServiceNames(void) ;
        /**
          Provides the supported component nodes

          @return supported component nodes
        */
        static uno::Sequence<rtl::OUString> SAL_CALL getSupportedComponents(void) ;

        //XSingleLayerStratum
        virtual uno::Reference<backend::XLayer> SAL_CALL
        getLayer( const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp )
            throw (backend::BackendAccessException,
                   lang::IllegalArgumentException) ;

        virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL
        getUpdatableLayer( const rtl::OUString& aLayerId )
            throw (backend::BackendAccessException,
                   lang::NoSupportException,
                   lang::IllegalArgumentException) ;
    protected:
        /**
          Service constructor from a service factory.

          @param xContext   component context
          */
        LocaleBackend(const uno::Reference<uno::XComponentContext>& xContext)
            throw (backend::BackendAccessException);

        /** Destructor */
        ~LocaleBackend(void) ;

    private:

        uno::Reference<uno::XComponentContext> m_xContext ;
        uno::Reference<backend::XLayer> m_xSystemLayer ;

        // Returns a time stamp in the appropriate format
        // for configuration layers.
        static rtl::OUString createTimeStamp(void);

        // Returns the system locale
        static rtl::OUString getLocale(void);

        // Returns the system UI locale
        static rtl::OUString getUILocale(void);
} ;


#endif // _FIXEDVALUEBACKEND_HXX_
