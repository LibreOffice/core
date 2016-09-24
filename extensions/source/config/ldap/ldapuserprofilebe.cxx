/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "ldapaccess.hxx"
#include "ldapuserprofilebe.hxx"
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <osl/process.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/byteseq.h>

#include <rtl/instance.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/security.hxx>


namespace extensions { namespace config { namespace ldap {

LdapUserProfileBe::LdapUserProfileBe( const uno::Reference<uno::XComponentContext>& xContext)
: LdapProfileMutexHolder(),
  BackendBase(mMutex)
{
    LdapDefinition aDefinition;
    OUString loggedOnUser;

    // This whole rigmarole is to prevent an infinite recursion where reading
    // the configuration for the backend would create another instance of the
    // backend, which would try and read the configuration which would...
    {
        osl::Mutex & aInitMutex = rtl::Static< osl::Mutex, LdapUserProfileBe >::get();
        osl::MutexGuard aInitGuard(aInitMutex);

        static bool bReentrantCall; // = false
        OSL_ENSURE(!bReentrantCall, "configuration: Ldap Backend constructor called reentrantly - probably a registration error.");

        if (!bReentrantCall)
        {
            try
            {
                bReentrantCall = true ;
                if (!readLdapConfiguration(
                        xContext, &aDefinition, &loggedOnUser))
                {
                    throw css::uno::RuntimeException(
                        OUString("LdapUserProfileBe- LDAP not configured"),
                        nullptr);
                }

                bReentrantCall = false ;
            }
            catch (...)
            {
                bReentrantCall = false;
                throw;
            }
        }
    }

    LdapConnection connection;
    connection.connectSimple(aDefinition);
    connection.getUserProfile(loggedOnUser, &data_);
}

LdapUserProfileBe::~LdapUserProfileBe()
{
}


bool LdapUserProfileBe::readLdapConfiguration(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    LdapDefinition * definition, OUString * loggedOnUser)
{
    OSL_ASSERT(context.is() && definition != nullptr && loggedOnUser != nullptr);
    const OUString kReadOnlyViewService("com.sun.star.configuration.ConfigurationAccess") ;
    const OUString kComponent("org.openoffice.LDAP/UserDirectory");
    const OUString kServerDefiniton("ServerDefinition");
    const OUString kServer("Server");
    const OUString kPort("Port");
    const OUString kBaseDN("BaseDN");
    const OUString kUser("SearchUser");
    const OUString kPassword("SearchPassword");
    const OUString kUserObjectClass("UserObjectClass");
    const OUString kUserUniqueAttr("UserUniqueAttribute");

    uno::Reference< XInterface > xIface;
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xCfgProvider(
            css::configuration::theDefaultProvider::get(context));

        css::beans::NamedValue aPath(OUString("nodepath"), uno::makeAny(kComponent) );

        uno::Sequence< uno::Any > aArgs(1);
        aArgs[0] <<=  aPath;

        xIface = xCfgProvider->createInstanceWithArguments(kReadOnlyViewService, aArgs);

        uno::Reference<container::XNameAccess > xAccess(xIface, uno::UNO_QUERY_THROW);
        xAccess->getByName(kServerDefiniton) >>= xIface;

        uno::Reference<container::XNameAccess > xChildAccess(xIface, uno::UNO_QUERY_THROW);

        if (!getLdapStringParam(xChildAccess, kServer, definition->mServer))
            return false;
        if (!getLdapStringParam(xChildAccess, kBaseDN, definition->mBaseDN))
            return false;

        definition->mPort=0;
        xChildAccess->getByName(kPort) >>= definition->mPort ;
        if (definition->mPort == 0)
            return false;

        if (!getLdapStringParam(xAccess, kUserObjectClass, definition->mUserObjectClass))
            return false;
        if (!getLdapStringParam(xAccess, kUserUniqueAttr, definition->mUserUniqueAttr))
            return false;

        getLdapStringParam(xAccess, kUser, definition->mAnonUser);
        getLdapStringParam(xAccess, kPassword, definition->mAnonCredentials);
    }
    catch (const uno::Exception & e)
    {
        OSL_TRACE("LdapUserProfileBackend: access to configuration data failed: %s",
                OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
        return false;
    }

    osl::Security aSecurityContext;
    if (!aSecurityContext.getUserName(*loggedOnUser))
        OSL_TRACE("LdapUserProfileBackend - could not get Logged on user from system");

    sal_Int32 nIndex = loggedOnUser->indexOf('/');
    if (nIndex > 0)
        *loggedOnUser = loggedOnUser->copy(nIndex+1);

    //Remember to remove
    OSL_TRACE("Logged on user is %s", OUStringToOString(*loggedOnUser,RTL_TEXTENCODING_ASCII_US).getStr());

    return true;
}


bool LdapUserProfileBe::getLdapStringParam(
    uno::Reference<container::XNameAccess>& xAccess,
    const OUString& aLdapSetting,
    OUString& aServerParameter)
{
    xAccess->getByName(aLdapSetting) >>= aServerParameter;

    return !aServerParameter.isEmpty();
}

void LdapUserProfileBe::setPropertyValue(
    OUString const &, css::uno::Any const &)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    throw css::lang::IllegalArgumentException(
        OUString("setPropertyValue not supported"),
        static_cast< cppu::OWeakObject * >(this), -1);
}

css::uno::Any LdapUserProfileBe::getPropertyValue(
    OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    for (sal_Int32 i = 0;;) {
        sal_Int32 j = PropertyName.indexOf(',', i);
        if (j == -1) {
            j = PropertyName.getLength();
        }
        if (j == i) {
            throw css::beans::UnknownPropertyException(
                PropertyName, static_cast< cppu::OWeakObject * >(this));
        }
        LdapData::iterator k(data_.find(PropertyName.copy(i, j - i)));
        if (k != data_.end()) {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true, css::uno::makeAny(k->second)));
        }
        if (j == PropertyName.getLength()) {
            break;
        }
        i = j + 1;
    }
    return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
}


OUString SAL_CALL LdapUserProfileBe::getLdapUserProfileBeName() {
    return OUString("com.sun.star.comp.configuration.backend.LdapUserProfileBe");
}


OUString SAL_CALL LdapUserProfileBe::getImplementationName()
    throw (uno::RuntimeException, std::exception)
{
    return getLdapUserProfileBeName() ;
}


uno::Sequence<OUString> SAL_CALL LdapUserProfileBe::getLdapUserProfileBeServiceNames()
{
    uno::Sequence<OUString> aServices { "com.sun.star.configuration.backend.LdapUserProfileBe" };
    return aServices ;
}

sal_Bool SAL_CALL LdapUserProfileBe::supportsService(const OUString& aServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence<OUString>
SAL_CALL LdapUserProfileBe::getSupportedServiceNames()
    throw (uno::RuntimeException, std::exception)
{
    return getLdapUserProfileBeServiceNames() ;
}

}}}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
