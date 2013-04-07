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

#ifndef _FIXEDVALUEBACKEND_HXX_
#define _FIXEDVALUEBACKEND_HXX_

#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase2.hxx>
#include <rtl/string.hxx>

namespace uno = css::uno ;
namespace lang = css::lang ;

class WinInetBackend : public ::cppu::WeakImplHelper2 <
        css::beans::XPropertySet,
        lang::XServiceInfo > {

    public :

        static WinInetBackend* createInstance();

        // XServiceInfo
        virtual OUString SAL_CALL
            getImplementationName(  )
                throw (uno::RuntimeException) ;

        virtual sal_Bool SAL_CALL
            supportsService( const OUString& aServiceName )
                throw (uno::RuntimeException) ;

        virtual uno::Sequence<OUString> SAL_CALL
            getSupportedServiceNames(  )
                throw (uno::RuntimeException) ;

        /**
          Provides the implementation name.

          @return   implementation name
          */
        static OUString SAL_CALL getBackendName(void) ;
        /**
          Provides the supported services names

          @return   service names
          */
        static uno::Sequence<OUString> SAL_CALL getBackendServiceNames(void) ;

        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo() throw (css::uno::RuntimeException)
        { return css::uno::Reference< css::beans::XPropertySetInfo >(); }

        virtual void SAL_CALL setPropertyValue(
            OUString const &, css::uno::Any const &)
            throw (
                css::beans::UnknownPropertyException,
                css::beans::PropertyVetoException,
                css::lang::IllegalArgumentException,
                css::lang::WrappedTargetException, css::uno::RuntimeException);

        virtual css::uno::Any SAL_CALL getPropertyValue(
            OUString const & PropertyName)
            throw (
                css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException, css::uno::RuntimeException);

        virtual void SAL_CALL addPropertyChangeListener(
            OUString const &,
            css::uno::Reference< css::beans::XPropertyChangeListener > const &)
            throw (
                css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException, css::uno::RuntimeException)
        {}

        virtual void SAL_CALL removePropertyChangeListener(
            OUString const &,
            css::uno::Reference< css::beans::XPropertyChangeListener > const &)
            throw (
                css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException, css::uno::RuntimeException)
        {}

        virtual void SAL_CALL addVetoableChangeListener(
            OUString const &,
            css::uno::Reference< css::beans::XVetoableChangeListener > const &)
            throw (
                css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException, css::uno::RuntimeException)
        {}

        virtual void SAL_CALL removeVetoableChangeListener(
            OUString const &,
            css::uno::Reference< css::beans::XVetoableChangeListener > const &)
            throw (
                css::beans::UnknownPropertyException,
                css::lang::WrappedTargetException, css::uno::RuntimeException)
        {}

    protected:
        /**
          Service constructor from a service factory.

          @param xContext   component context
          */
        WinInetBackend();

        /** Destructor */
        ~WinInetBackend(void) ;

    private:
        com::sun::star::beans::Optional< com::sun::star::uno::Any >
            valueProxyType_;
        com::sun::star::beans::Optional< com::sun::star::uno::Any >
            valueNoProxy_;
        com::sun::star::beans::Optional< com::sun::star::uno::Any >
            valueHttpProxyName_;
        com::sun::star::beans::Optional< com::sun::star::uno::Any >
            valueHttpProxyPort_;
        com::sun::star::beans::Optional< com::sun::star::uno::Any >
            valueHttpsProxyName_;
        com::sun::star::beans::Optional< com::sun::star::uno::Any >
            valueHttpsProxyPort_;
        com::sun::star::beans::Optional< com::sun::star::uno::Any >
            valueFtpProxyName_;
        com::sun::star::beans::Optional< com::sun::star::uno::Any >
            valueFtpProxyPort_;
} ;


#endif // _FIXEDVALUEBACKEND_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
