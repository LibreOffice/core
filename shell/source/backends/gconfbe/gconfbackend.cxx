/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
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

#include "sal/config.h"

#include <cstddef>

#include "boost/noncopyable.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/beans/PropertyVetoException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/beans/XVetoableChangeListener.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XCurrentContext.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/current_context.hxx"
#include "uno/lbnames.h"

#include "gconfaccess.hxx"

namespace {

namespace css = com::sun::star;

rtl::OUString SAL_CALL getServiceImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.comp.configuration.backend.GconfBackend"));
}

css::uno::Sequence< rtl::OUString > SAL_CALL getServiceSupportedServiceNames() {
    rtl::OUString name(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.configuration.backend.GconfBackend"));
    return css::uno::Sequence< rtl::OUString >(&name, 1);
}

class Service:
    public cppu::WeakImplHelper2<
        css::lang::XServiceInfo, css::beans::XPropertySet >,
    private boost::noncopyable
{
public:
    Service();

private:
    virtual ~Service() {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    { return getServiceImplementationName(); }

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    { return ServiceName == getSupportedServiceNames()[0]; }

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException)
    { return getServiceSupportedServiceNames(); }

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

    bool enabled_;
};

Service::Service(): enabled_(false) {
    css::uno::Reference< css::uno::XCurrentContext > context(
        css::uno::getCurrentContext());
    if (context.is()) {
        rtl::OUString desktop;
        context->getValueByName(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("system.desktop-environment"))) >>=
            desktop;
        enabled_ = desktop == "GNOME";
    }
}

void Service::setPropertyValue(rtl::OUString const &, css::uno::Any const &)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    throw css::lang::IllegalArgumentException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("setPropertyValue not supported")),
        static_cast< cppu::OWeakObject * >(this), -1);
}

css::uno::Any Service::getPropertyValue(rtl::OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    for (std::size_t i = 0; i < gconfaccess::nConfigurationValues; ++i)
    {
        if (PropertyName.equalsAsciiL(
                gconfaccess::ConfigurationValues[i].OOoConfItem,
                gconfaccess::ConfigurationValues[i].nOOoConfItemLen))
        {
            return css::uno::makeAny(
                enabled_
                ? gconfaccess::getValue(gconfaccess::ConfigurationValues[i])
                : css::beans::Optional< css::uno::Any >());
        }
    }
    throw css::beans::UnknownPropertyException(
        PropertyName, static_cast< cppu::OWeakObject * >(this));
}

css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
    SAL_UNUSED_PARAMETER css::uno::Reference< css::uno::XComponentContext >
        const &)
{
    return static_cast< cppu::OWeakObject * >(new Service);
}

static cppu::ImplementationEntry const services[] = {
    { &createInstance, &getServiceImplementationName,
      &getServiceSupportedServiceNames, &cppu::createSingleComponentFactory, 0,
      0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL gconfbe1_component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
