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
#if 1

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <cppuhelper/compbase2.hxx>

#include "ldapaccess.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class XComponentContext;
} } } }

namespace extensions { namespace config { namespace ldap {

namespace uno = css::uno ;
namespace lang = css::lang ;
namespace container = css::container;

struct LdapDefinition;

typedef cppu::WeakComponentImplHelper2<css::beans::XPropertySet,
                                       lang::XServiceInfo> BackendBase ;

struct LdapProfileMutexHolder { osl::Mutex mMutex; };
/**
  Implements the PlatformBackend service, a specialization of the
  XPropertySet service for retreiving LDAP user profile
  configuration settings from a LDAP repsoitory.
  */
class LdapUserProfileBe : private LdapProfileMutexHolder, public BackendBase
{
    public :

        LdapUserProfileBe(const uno::Reference<uno::XComponentContext>& xContext);
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

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() throw (css::uno::RuntimeException)
        { return css::uno::Reference< css::beans::XPropertySetInfo >(); }

        virtual void SAL_CALL setPropertyValue(
            rtl::OUString const &, css::uno::Any const &)
            throw (
                css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException, css::uno::RuntimeException);

        virtual css::uno::Any SAL_CALL getPropertyValue(
            rtl::OUString const & PropertyName)
            throw (
                css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException, css::uno::RuntimeException);

        virtual void SAL_CALL addPropertyChangeListener(
            rtl::OUString const &,
            css::uno::Reference< css::beans::XPropertyChangeListener > const &)
            throw (
                css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException, css::uno::RuntimeException)
        {}

        virtual void SAL_CALL removePropertyChangeListener(
            rtl::OUString const &,
            css::uno::Reference< css::beans::XPropertyChangeListener > const &)
            throw (
                css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException, css::uno::RuntimeException)
        {}

        virtual void SAL_CALL addVetoableChangeListener(
            rtl::OUString const &,
            css::uno::Reference< css::beans::XVetoableChangeListener > const &)
            throw (
                css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException, css::uno::RuntimeException)
        {}

        virtual void SAL_CALL removeVetoableChangeListener(
            rtl::OUString const &,
            css::uno::Reference< css::beans::XVetoableChangeListener > const &)
            throw (
                css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException, css::uno::RuntimeException)
        {}

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
        /** Check if LDAP is configured */
        bool readLdapConfiguration(
            uno::Reference<uno::XComponentContext> const & context,
            LdapDefinition * definition, rtl::OUString * loggedOnUser);

        bool getLdapStringParam(uno::Reference<container::XNameAccess>& xAccess,
                                const rtl::OUString& aLdapSetting,
                                rtl::OUString& aServerParameter);

    LdapData data_;
} ;
//------------------------------------------------------------------------------
}}}

#endif // EXTENSIONS_CONFIG_LDAP_LDAPUSERPROFILE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
