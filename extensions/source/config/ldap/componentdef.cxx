/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: componentdef.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:40:47 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"

#ifndef EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILE_HXX_
#include "ldapuserprofilebe.hxx"
#endif //CONFIGMGR_BACKEND_LDAPUSERPROFILE_HXX_
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_
#include <cppuhelper/implementationentry.hxx>
#endif // _CPPUHELPER_IMPLEMENTATIONENTRY_HXX_

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif // _RTL_USTRBUF_HXX_

using namespace extensions::config::ldap ;

//==============================================================================

static uno::Reference<uno::XInterface> SAL_CALL createLdapUserProfileBe(
                   const uno::Reference<uno::XComponentContext>& aContext) {
    return * new LdapUserProfileBe(aContext) ;
}
//------------------------------------------------------------------------------

static const cppu::ImplementationEntry kImplementations_entries[] =
{
    {
        createLdapUserProfileBe,
        LdapUserProfileBe::getLdapUserProfileBeName,
        LdapUserProfileBe::getLdapUserProfileBeServiceNames,
        cppu::createSingleComponentFactory,
        NULL,
        0
    },
    { NULL, NULL, NULL, NULL, NULL, 0 }
} ;
//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                                            const sal_Char **aEnvTypeName,
                                            uno_Environment** /*aEnvironment*/) {
    *aEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}
//------------------------------------------------------------------------------

extern "C" sal_Bool SAL_CALL component_writeInfo(void *aServiceManager,
                                                 void *aRegistryKey) {
    using namespace ::com::sun::star::registry;
    if (aRegistryKey)
    {

        /** Service factory */
        uno::Reference<lang::XMultiServiceFactory> xFactory
            (reinterpret_cast<lang::XMultiServiceFactory*> (aServiceManager),
             uno::UNO_QUERY);

        rtl::OUStringBuffer aImplKeyName;
        aImplKeyName.appendAscii("/");
        aImplKeyName.append(LdapUserProfileBe::getLdapUserProfileBeName());

        rtl::OUString aMainKeyName(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));

        uno::Reference<XRegistryKey> xNewImplKey(
            reinterpret_cast<XRegistryKey*>
            (aRegistryKey)->createKey(aImplKeyName.makeStringAndClear()));

        uno::Reference<XRegistryKey> xNewKey(
            xNewImplKey->createKey(aMainKeyName));

        //Now register associated service names
        uno::Sequence<rtl::OUString> sServiceNames =
            LdapUserProfileBe::getLdapUserProfileBeServiceNames();
        for (sal_Int32 i = 0 ; i < sServiceNames.getLength() ; ++ i)
        {
            xNewKey->createKey(sServiceNames[i]);
        }
        //Now register associated org.openoffice.UserProfile component
        //that this backend supports under xNewImplKey
        uno::Reference<XRegistryKey> xComponentKey(
            xNewImplKey->createKey(rtl::OUString::createFromAscii
            ("/DATA/SupportedComponents")));

        uno::Sequence<rtl::OUString> aComponentList(1);
        aComponentList[0]= rtl::OUString::createFromAscii
            ("org.openoffice.UserProfile");

        xComponentKey->setAsciiListValue(aComponentList);
        return sal_True;
    }
    return sal_False;
}
//------------------------------------------------------------------------------

extern "C" void *component_getFactory(const sal_Char *aImplementationName,
                                      void *aServiceManager,
                                      void *aRegistryKey) {
    return cppu::component_getFactoryHelper(aImplementationName,
                                            aServiceManager,
                                            aRegistryKey,
                                            kImplementations_entries) ;
}
//------------------------------------------------------------------------------
