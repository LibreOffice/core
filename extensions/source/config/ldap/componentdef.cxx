/*************************************************************************
 *
 *  $RCSfile: componentdef.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 14:37:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
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
    { NULL }
} ;
//------------------------------------------------------------------------------

extern "C" void SAL_CALL component_getImplementationEnvironment(
                                            const sal_Char **aEnvTypeName,
                                            uno_Environment **aEnvironment) {
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
