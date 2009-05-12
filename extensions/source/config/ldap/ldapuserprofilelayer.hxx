/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ldapuserprofilelayer.hxx,v $
 * $Revision: 1.7 $
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

#ifndef EXTENSIONS_CONFIG_LDAP_LADPUSERPROFILELAYER_HXX_
#define EXTENSIONS_CONFIG_LDAP_LADPUSERPROFILELAYER_HXX_

#include "ldapuserprof.hxx"
#include "ldapaccess.hxx"

#include "propertysethelper.hxx"
#include <com/sun/star/configuration/backend/XLayer.hpp>
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif
#include <com/sun/star/util/XTimeStamped.hpp>
#include <cppuhelper/implbase2.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/ref.hxx>


namespace extensions { namespace config { namespace ldap {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
namespace util = css::util ;

//------------------------------------------------------------------------------
struct LdapUserProfileSource : public salhelper::SimpleReferenceObject
{
    LdapConnection      mConnection;
    LdapUserProfileMap  mProfileMap;

    rtl::OUString getComponentName() const;
    rtl::OUString getConfigurationBasePath() const;
    void getUserProfile(rtl::OUString const & aUser, LdapUserProfile & aProfile);
};
typedef rtl::Reference< LdapUserProfileSource > LdapUserProfileSourceRef;
//------------------------------------------------------------------------------

/**
  Implementation of the XLayer interfaces  for LdapUserProfileBe.
  Class reads UserProfile setting form LDAP.
  The timestamp indicates the last modification time
  */
 class LdapUserProfileLayer : public cppu::ImplInheritanceHelper2<
                                                apihelper::ReadOnlyPropertySetHelper,
                                                backend::XLayer,
                                                util::XTimeStamped>
{
    public :
        /** Constructor */
        LdapUserProfileLayer(
            const uno::Reference<lang::XMultiServiceFactory>& xFactory,
            const rtl::OUString& aUser,
            const LdapUserProfileSourceRef & aUserProfileSource,
            const rtl::OUString& aTimestamp);
        // throw (uno::RuntimeException

        /** Destructor */
        ~LdapUserProfileLayer();

        // XLayer
        virtual void SAL_CALL readData(
                const uno::Reference<backend::XLayerHandler>& xHandler)
                throw ( backend::MalformedDataException,
                        lang::NullPointerException,
                        lang::WrappedTargetException,
                        uno::RuntimeException) ;

        // XTimeStamped
        virtual rtl::OUString SAL_CALL getTimestamp(void)
            throw (uno::RuntimeException)
        { return mTimestamp; }

    protected:
        // PropertySetHelper
        virtual cppu::IPropertyArrayHelper * SAL_CALL newInfoHelper();

        virtual void SAL_CALL getFastPropertyValue( uno::Any& rValue, sal_Int32 nHandle ) const;
    private :
        struct ProfileData;
        bool readProfile();

        uno::Reference<backend::XLayerContentDescriber> mLayerDescriber;
        LdapUserProfileSourceRef mSource;
        rtl::OUString mUser;
        rtl::OUString mTimestamp;
        ProfileData * mProfile;
    private:
        using cppu::OPropertySetHelper::getFastPropertyValue;
  } ;
}}}
#endif // EXTENSIONS_CONFIG_LDAP_LADPUSERPROFILELAYER_HXX_
