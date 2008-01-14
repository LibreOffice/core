/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ldapuserprofilelayer.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:42:01 $
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

#ifndef EXTENSIONS_CONFIG_LDAP_LADPUSERPROFILELAYER_HXX_
#define EXTENSIONS_CONFIG_LDAP_LADPUSERPROFILELAYER_HXX_

#ifndef EXTENSIONS_CONFIG_LDAP_LDAPUSERPROF_HXX_
#include "ldapuserprof.hxx"
#endif
#ifndef EXTENSIONS_CONFIG_LDAP_LDAPACCESS_HXX_
#include "ldapaccess.hxx"
#endif // EXTENSIONS_CONFIG_LDAP_LDAPACCESS_HXX_

#include "propertysethelper.hxx"

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <com/sun/star/configuration/backend/XLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYERCONTENTDESCIBER_HPP_
#include <com/sun/star/configuration/backend/XLayerContentDescriber.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XTIMESTAMPED_HPP_
#include <com/sun/star/util/XTimeStamped.hpp>
#endif // _COM_SUN_STAR_UTIL_XTIMESTAMPED_HPP_

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif // _CPPUHELPER_IMPLBASE2_HXX_

#ifndef _SALHELPER_SIMPLEREFERENCEOBJECT_HXX_
#include <salhelper/simplereferenceobject.hxx>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif


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
