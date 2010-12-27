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

#include "precompiled_desktop.hxx"
#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/frame/DispatchDescriptor.hpp"
#include "com/sun/star/frame/XDispatch.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XStatusListener.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/URL.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase3.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "filter/msfilter/countryid.hxx"
#include "osl/diagnose.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/lbnames.h"
#include "vcl/svapp.hxx"

namespace {

namespace css = com::sun::star;

class Service:
    public cppu::WeakImplHelper3<
        css::lang::XServiceInfo, css::frame::XDispatchProvider,
        css::frame::XDispatch >,
    private boost::noncopyable
{
public:
    static css::uno::Reference< css::uno::XInterface > SAL_CALL static_create(
        css::uno::Reference< css::uno::XComponentContext > const &)
        SAL_THROW((css::uno::Exception))
    { return static_cast< cppu::OWeakObject * >(new Service); }

    static rtl::OUString SAL_CALL static_getImplementationName();

    static css::uno::Sequence< rtl::OUString > SAL_CALL
    static_getSupportedServiceNames();

private:
    Service() {}

    virtual ~Service() {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    { return static_getImplementationName(); }

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    { return ServiceName == getSupportedServiceNames()[0]; } //TODO

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException)
    { return static_getSupportedServiceNames(); }

    virtual css::uno::Reference< css::frame::XDispatch > SAL_CALL queryDispatch(
        css::util::URL const &, rtl::OUString const &, sal_Int32)
        throw (css::uno::RuntimeException)
    { return this; }

    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > >
    SAL_CALL queryDispatches(
        css::uno::Sequence< css::frame::DispatchDescriptor > const & Requests)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL dispatch(
        css::util::URL const &,
        css::uno::Sequence< css::beans::PropertyValue > const &)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL addStatusListener(
        css::uno::Reference< css::frame::XStatusListener > const &,
        css::util::URL const &)
        throw (css::uno::RuntimeException)
    {}

    virtual void SAL_CALL removeStatusListener(
        css::uno::Reference< css::frame::XStatusListener > const &,
        css::util::URL const &)
        throw (css::uno::RuntimeException)
    {}
};

rtl::OUString Service::static_getImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.test.deployment.boxt"));
}

css::uno::Sequence< rtl::OUString > Service::static_getSupportedServiceNames() {
    rtl::OUString name(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.test.deployment.boxt"));
    return css::uno::Sequence< rtl::OUString >(&name, 1);
}

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > >
Service::queryDispatches(
    css::uno::Sequence< css::frame::DispatchDescriptor > const & Requests)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > > s(
        Requests.getLength());
    for (sal_Int32 i = 0; i < s.getLength(); ++i) {
        s[i] = queryDispatch(
            Requests[i].FeatureURL, Requests[i].FrameName,
            Requests[i].SearchFlags);
    }
    return s;
}

void Service::dispatch(
    css::util::URL const &,
    css::uno::Sequence< css::beans::PropertyValue > const &)
    throw (css::uno::RuntimeException)
{
    msfilter::ConvertCountryToLanguage(msfilter::COUNTRY_DONTKNOW);
        // link against some obscure library that is unlikely already loaded
    Application::ShowNativeErrorBox(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("boxt")),
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test")));
}

static cppu::ImplementationEntry const services[] = {
    { &Service::static_create, &Service::static_getImplementationName,
      &Service::static_getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" void * SAL_CALL component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

extern "C" void SAL_CALL component_getImplementationEnvironment(
    char const ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey)
{
    return component_writeInfoHelper(pServiceManager, pRegistryKey, services);
}
