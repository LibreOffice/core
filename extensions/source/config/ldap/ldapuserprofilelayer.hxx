#ifndef EXTENSIONS_CONFIG_LDAP_LADPUSERPROFILELAYER_HXX_
#define EXTENSIONS_CONFIG_LDAP_LADPUSERPROFILELAYER_HXX_

#ifndef EXTENSIONS_CONFIG_LDAP_LDAPUSERPROF_HXX_
#include "ldapuserprof.hxx"
#endif
#ifndef EXTENSIONS_CONFIG_LDAP_LDAPACCESS_HXX_
#include "ldapaccess.hxx"
#endif // EXTENSIONS_CONFIG_LDAP_LDAPACCESS_HXX_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_
#include <com/sun/star/configuration/backend/XLayer.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XLAYER_HPP_

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_
#include <com/sun/star/configuration/backend/BackendAccessException.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_BACKENDACCESSEXCEPTION_HPP_

#ifndef _COM_SUN_STAR_UTIL_XTIMESTAMPED_HPP_
#include <com/sun/star/util/XTimeStamped.hpp>
#endif // _COM_SUN_STAR_UTIL_XTIMESTAMPED_HPP_

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif // _CPPUHELPER_IMPLBASE2_HXX_


namespace extensions { namespace config { namespace ldap {

namespace css = com::sun::star ;
namespace uno = css::uno ;
namespace lang = css::lang ;
namespace backend = css::configuration::backend ;
namespace util = css::util ;

/**
  Implementation of the XLayer interfaces  for LdapUserProfileBe.
  Class reads UserProfile setting form LDAP.
  The timestamp indicates the last modification time
  */
 class LdapUserProfileLayer : public cppu::WeakImplHelper2<backend::XLayer,
                                                           util::XTimeStamped>
{
    public :
        /** Constructor */
        LdapUserProfileLayer(
            const uno::Reference<lang::XMultiServiceFactory>& xFactory,
            const rtl::OUString& aUser,
            const LdapUserProfileMap& aUserProfileMap,
            LdapConnection& aConnection,
            const rtl::OUString& aTimeStamp);

        /** Destructor */
        ~LdapUserProfileLayer(void) {}

        // XLayer
        virtual void SAL_CALL readData(
                const uno::Reference<backend::XLayerHandler>& xHandler)
                throw ( backend::MalformedDataException,
                        lang::NullPointerException,
                        lang::WrappedTargetException,
                        uno::RuntimeException) ;

        // XTimeStamped
        virtual rtl::OUString SAL_CALL getTimestamp(void)
            throw (uno::RuntimeException){
                return mTimeStamp;}
    private :
        uno::Reference<lang::XMultiServiceFactory> mFactory;
        LdapConnection& mConnection;
        const LdapUserProfileMap& mUserProfileMap;
        rtl::OUString  mUserName;
        rtl::OUString  mTimeStamp;
  } ;
}}}
#endif // EXTENSIONS_CONFIG_LDAP_LADPUSERPROFILELAYER_HXX_
