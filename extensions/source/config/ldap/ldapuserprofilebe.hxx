/*************************************************************************
 *
 *  $RCSfile: ldapuserprofilebe.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 14:38:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards OOurce License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free OOftware; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free OOftware Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free OOftware
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards OOurce License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  OOurce License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  OOftware provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE OOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the OOftware.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILEBE_HXX_
#define EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILEBE_HXX_
#ifndef EXTENSIONS_CONFIG_LDAP_LDAPUSERPROF_HXX_
#include "ldapuserprof.hxx"
#endif
#ifndef EXTENSIONS_CONFIG_LDAP_LDAPACCESS_HXX_
#include "ldapaccess.hxx"
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XSINGLELAYERSTRATUM_HPP_
#include <com/sun/star/configuration/backend/XSingleLayerStratum.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif // _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif // _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif // _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_CANNOTCONNECTEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/CannotConnectException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_INSUFFICIENTACCESSRIGHTSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/InsufficientAccessRightsException.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_INSUFFICIENTACCESSRIGHTSEXCEPTION_HPP_
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_CONNECTIONLOSTEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/ConnectionLostException.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_CONNECTIONLOSTEXCEPTION_HPP_
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif // _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_

#ifndef _COM_SUN_STAR_LDAP_LDAPCONNECTIONEXCEPTION_HPP_
#include <com/sun/star/ldap/LdapConnectionException.hpp>
#endif // _COM_SUN_STAR_LDAP_LDAPCONNECTIONEXCEPTION_HPP_

#ifndef _COM_SUN_STAR_LDAP_LDAPGENERICEXCEPTION_HPP_
#include <com/sun/star/ldap/LdapGenericException.hpp>
#endif // _COM_SUN_STAR_LDAP_LDAPGENERICEXCEPTION_HPP_

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif // _CPPUHELPER_COMPBASE3_HXX_

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif


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

/**
  Implements the PlatformBackend service, a specialization of the
  SingleLayerStratum service for retreiving LDAP user profile
  configuration settings from a LDAP repsoitory.
  */
class LdapUserProfileBe : public BackendBase
{
    public :

        LdapUserProfileBe(const uno::Reference<uno::XComponentContext>& xContext)
            throw(backend::BackendAccessException,
                  backend::BackendSetupException);
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
                   lang::IllegalArgumentException) ;

        virtual uno::Reference<backend::XUpdatableLayer> SAL_CALL
        getUpdatableLayer( const rtl::OUString& aLayerId )
            throw (backend::BackendAccessException,
                   lang::NoSupportException,
                   lang::IllegalArgumentException) ;
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
        bool isLdapConfigured(LdapDefinition& aDefinition);

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
        void getMappingFileUrl(rtl::OUString& aFileUrl,
                               const rtl::OUString& aFileMapName)const;

        /** Service Factory object */
        uno::Reference<lang::XMultiServiceFactory> mFactory;
        /** Component Context */
        uno::Reference<uno::XComponentContext> mContext ;
        /** Mutex for resource protection */
        osl::Mutex mMutex ;
        /** Maping LDAP->00o */
        LdapUserProfileMap mUserProfileMap;
        /** Object for LDAP functionality */
        LdapConnection mLdapConnection;
        /**Currently logged in user */
        rtl::OUString mLoggedOnUser ;
        /** DN of currently logged in user */
        rtl::OString mUserDN;
} ;
//------------------------------------------------------------------------------
}}}

#endif // EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILE_HXX_
