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
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <rtl/instance.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <comphelper/scopeguard.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/security.hxx>


namespace extensions::config::ldap {

LdapUserProfileBe::LdapUserProfileBe( const uno::Reference<uno::XComponentContext>& xContext)
: BackendBase(m_aMutex)
{
    LdapDefinition aDefinition;
    OUString loggedOnUser;
    // true initially to handle reentrant call; will become false if readLdapConfiguration fails
    bool bHaveLdapConfiguration = true;

    // This whole rigmarole is to prevent an infinite recursion where reading
    // the configuration for the backend would create another instance of the
    // backend, which would try and read the configuration which would...
    {
        static osl::Mutex aInitMutex;
        osl::MutexGuard aInitGuard(aInitMutex);

        static bool bReentrantCall; // = false
        OSL_ENSURE(!bReentrantCall, "configuration: Ldap Backend constructor called reentrantly - probably a registration error.");

        if (!bReentrantCall)
        {
            bReentrantCall = true ;
            comphelper::ScopeGuard aReentrantCallGuard([]() { bReentrantCall = false; });
            // Don't throw on fail: this will crash if LDAP is misconfigured, and user opens
            // Expert Configuration dialog. Instead, just don't fill data_, which will make the
            // backend return empty values. This happens in SvtUserOptions::Impl::GetValue_Impl
            // anyway even in throwing scenario, but doing it here also improves performance
            // because of avoiding repeated attempts to create the backend.
            bHaveLdapConfiguration = readLdapConfiguration(
                xContext, &aDefinition, &loggedOnUser);
            if (!bHaveLdapConfiguration)
                SAL_WARN("extensions.config", "LdapUserProfileBackend: LDAP not configured");
        }
    }

    if (bHaveLdapConfiguration)
    {
        LdapConnection connection;
        connection.connectSimple(aDefinition);
        connection.getUserProfile(loggedOnUser, &data_);
    }
}

LdapUserProfileBe::~LdapUserProfileBe()
{
}


bool LdapUserProfileBe::readLdapConfiguration(
    css::uno::Reference< css::uno::XComponentContext > const & context,
    LdapDefinition * definition, OUString * loggedOnUser)
{
    OSL_ASSERT(context.is() && definition != nullptr && loggedOnUser != nullptr);

    uno::Reference< XInterface > xIface;
    try
    {
        uno::Reference< lang::XMultiServiceFactory > xCfgProvider(
            css::configuration::theDefaultProvider::get(context));

        css::beans::NamedValue aPath(u"nodepath"_ustr, uno::Any(u"org.openoffice.LDAP/UserDirectory"_ustr) );

        uno::Sequence< uno::Any > aArgs{ uno::Any(aPath) };

        xIface = xCfgProvider->createInstanceWithArguments(u"com.sun.star.configuration.ConfigurationAccess"_ustr, aArgs);

        uno::Reference<container::XNameAccess > xAccess(xIface, uno::UNO_QUERY_THROW);
        xAccess->getByName(u"ServerDefinition"_ustr) >>= xIface;

        uno::Reference<container::XNameAccess > xChildAccess(xIface, uno::UNO_QUERY_THROW);

        if (!getLdapStringParam(xChildAccess, u"Server"_ustr, definition->mServer))
            return false;
        if (!getLdapStringParam(xChildAccess, u"BaseDN"_ustr, definition->mBaseDN))
            return false;

        definition->mPort=0;
        xChildAccess->getByName(u"Port"_ustr) >>= definition->mPort ;
        if (definition->mPort == 0)
            return false;

        if (!getLdapStringParam(xAccess, u"UserObjectClass"_ustr, definition->mUserObjectClass))
            return false;
        if (!getLdapStringParam(xAccess, u"UserUniqueAttribute"_ustr, definition->mUserUniqueAttr))
            return false;

        getLdapStringParam(xAccess, u"SearchUser"_ustr, definition->mAnonUser);
        getLdapStringParam(xAccess, u"SearchPassword"_ustr, definition->mAnonCredentials);
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("extensions.config", "LdapUserProfileBackend: access to configuration data failed");
        return false;
    }

    osl::Security aSecurityContext;
    if (!aSecurityContext.getUserName(*loggedOnUser))
        SAL_WARN("extensions.config", "LdapUserProfileBackend - could not get Logged on user from system");

    sal_Int32 nIndex = loggedOnUser->indexOf('/');
    if (nIndex > 0)
        *loggedOnUser = loggedOnUser->copy(nIndex+1);

    return true;
}


bool LdapUserProfileBe::getLdapStringParam(
    uno::Reference<container::XNameAccess> const & xAccess,
    const OUString& aLdapSetting,
    OUString& aServerParameter)
{
    xAccess->getByName(aLdapSetting) >>= aServerParameter;

    return !aServerParameter.isEmpty();
}

void LdapUserProfileBe::setPropertyValue(
    OUString const &, css::uno::Any const &)
{
    throw css::lang::IllegalArgumentException(
        u"setPropertyValue not supported"_ustr,
        static_cast< cppu::OWeakObject * >(this), -1);
}

css::uno::Any LdapUserProfileBe::getPropertyValue(
    OUString const & PropertyName)
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
            return css::uno::Any(
                css::beans::Optional< css::uno::Any >(
                    true, css::uno::Any(k->second)));
        }
        if (j == PropertyName.getLength()) {
            break;
        }
        i = j + 1;
    }
    return css::uno::Any(css::beans::Optional< css::uno::Any >());
}


OUString SAL_CALL LdapUserProfileBe::getImplementationName()
{
    return u"com.sun.star.comp.configuration.backend.LdapUserProfileBe"_ustr;
}

sal_Bool SAL_CALL LdapUserProfileBe::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence<OUString>
SAL_CALL LdapUserProfileBe::getSupportedServiceNames()
{
    return { u"com.sun.star.configuration.backend.LdapUserProfileBe"_ustr };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
extensions_ldp_LdapUserProfileBe_get_implementation(
    css::uno::XComponentContext* context , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new extensions::config::ldap::LdapUserProfileBe(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
