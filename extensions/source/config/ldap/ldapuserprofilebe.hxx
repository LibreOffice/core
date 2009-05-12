/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ldapuserprofilebe.hxx,v $
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

#ifndef EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILEBE_HXX_
#define EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILEBE_HXX_

#include "ldapuserprof.hxx"
#include "ldapaccess.hxx"
#include "ldapuserprofilelayer.hxx"
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#include <com/sun/star/configuration/backend/ConnectionLostException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/compbase2.hxx>


#define CONTEXT_ITEM_PREFIX_                "/modules/com.sun.star.configuration/bootstrap/"
namespace extensions { namespace config { namespace ldap {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace ldap= css::ldap ;
namespace backend = css::configuration::backend ;
namespace container = css::container;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
typedef cppu::WeakComponentImplHelper2<backend::XSingleLayerStratum,
                                       lang::XServiceInfo> BackendBase ;

struct LdapProfileMutexHolder { osl::Mutex mMutex; };
/**
  Implements the PlatformBackend service, a specialization of the
  SingleLayerStratum service for retreiving LDAP user profile
  configuration settings from a LDAP repsoitory.
  */
class LdapUserProfileBe : private LdapProfileMutexHolder, public BackendBase
{
    public :

        LdapUserProfileBe(const uno::Reference<uno::XComponentContext>& xContext);
        // throw(backend::BackendAccessException, backend::BackendSetupException, RuntimeException);
        ~LdapUserProfileBe(void) ;

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

        //XSingleLayerStratum
        virtual uno::Reference<backend::XLayer> SAL_CALL
        getLayer( const rtl::OUString& aLayerId, const rtl::OUString& aTimestamp )
            throw (backend::BackendAccessException,
                   lang::IllegalArgumentException,
                   uno::RuntimeException) ;

        virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL
        getUpdatableLayer( const rtl::OUString& aLayerId )
            throw (backend::BackendAccessException,
                   lang::NoSupportException,
                   lang::IllegalArgumentException,
                   uno::RuntimeException) ;
         /**
          Provides the implementation name.
          @return   implementation name
          */
        static rtl::OUString SAL_CALL getLdapUserProfileBeName(void) ;
        /**
          Provides the supported services names
          @return   service names
          */
        static uno::Sequence<rtl::OUString> SAL_CALL
            getLdapUserProfileBeServiceNames(void) ;

    private:
        /** Build OO/LDAP attribute mapping table */
        void  initializeMappingTable (const rtl::OUString& aFileMapName);
        /** Check if LDAP is configured */
        bool readLdapConfiguration(LdapDefinition& aDefinition);

        bool getLdapStringParam(uno::Reference<container::XNameAccess>& xAccess,
                                const rtl::OUString& aLdapSetting,
                                rtl::OString& aServerParameter);

        void mapGenericException(ldap::LdapGenericException& aException)
            throw (backend::InsufficientAccessRightsException,
                   backend::ConnectionLostException,
                   backend::BackendAccessException);
        /** Connect to LDAP server */
        void connectToLdapServer(const LdapDefinition& aDefinition );
        /** Get URL of OO-to-LDAP Mapping File */
        rtl::OUString getMappingFileUrl(const rtl::OUString& aFileMapName) const;

        /** Service Factory object */
        uno::Reference<lang::XMultiServiceFactory> mFactory;
        /** Component Context */
        uno::Reference<uno::XComponentContext> mContext ;
        /** Object for LDAP functionality */
        LdapUserProfileSourceRef mLdapSource;
        /**Currently logged in user */
        rtl::OUString mLoggedOnUser ;
        /** DN of currently logged in user */
        rtl::OString mUserDN;

} ;
//------------------------------------------------------------------------------
}}}

#endif // EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILE_HXX_
