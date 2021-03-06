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

#pragma once

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/compbase.hxx>

#include "ldapaccess.hxx"

namespace com::sun::star::uno {
    class XComponentContext;
}

namespace extensions::config::ldap {

namespace uno = css::uno ;
namespace lang = css::lang ;
namespace container = css::container;

struct LdapDefinition;

typedef cppu::WeakComponentImplHelper<css::beans::XPropertySet,
                                       lang::XServiceInfo> BackendBase ;

struct LdapProfileMutexHolder { osl::Mutex mMutex; };
/**
  Implements the PlatformBackend service, a specialization of the
  XPropertySet service for retrieving LDAP user profile
  configuration settings from an LDAP repository.
  */
class LdapUserProfileBe : private LdapProfileMutexHolder, public BackendBase
{
    public:

        explicit LdapUserProfileBe(const uno::Reference<uno::XComponentContext>& xContext);
        virtual ~LdapUserProfileBe() override ;

        // XServiceInfo
        virtual OUString SAL_CALL
            getImplementationName(  ) override ;

        virtual sal_Bool SAL_CALL
            supportsService( const OUString& aServiceName ) override ;

        virtual uno::Sequence<OUString> SAL_CALL
            getSupportedServiceNames(  ) override ;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() override
        { return css::uno::Reference< css::beans::XPropertySetInfo >(); }

        virtual void SAL_CALL setPropertyValue(
            OUString const &, css::uno::Any const &) override;

        virtual css::uno::Any SAL_CALL getPropertyValue(
            OUString const & PropertyName) override;

        virtual void SAL_CALL addPropertyChangeListener(
            OUString const &,
            css::uno::Reference< css::beans::XPropertyChangeListener > const &) override
        {}

        virtual void SAL_CALL removePropertyChangeListener(
            OUString const &,
            css::uno::Reference< css::beans::XPropertyChangeListener > const &) override
        {}

        virtual void SAL_CALL addVetoableChangeListener(
            OUString const &,
            css::uno::Reference< css::beans::XVetoableChangeListener > const &) override
        {}

        virtual void SAL_CALL removeVetoableChangeListener(
            OUString const &,
            css::uno::Reference< css::beans::XVetoableChangeListener > const &) override
        {}

    private:
        /** Check if LDAP is configured */
        static bool readLdapConfiguration(
            uno::Reference<uno::XComponentContext> const & context,
            LdapDefinition * definition, OUString * loggedOnUser);

        static bool getLdapStringParam(uno::Reference<container::XNameAccess> const & xAccess,
                                const OUString& aLdapSetting,
                                OUString& aServerParameter);

    LdapData data_;
} ;

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
