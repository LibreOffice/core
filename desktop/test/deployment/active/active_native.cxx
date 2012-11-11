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

#include "boost/noncopyable.hpp"
#include "com/sun/star/awt/MessageBoxButtons.hpp"
#include "com/sun/star/awt/Rectangle.hpp"
#include "com/sun/star/awt/Toolkit.hpp"
#include "com/sun/star/awt/XMessageBox.hpp"
#include "com/sun/star/awt/XMessageBoxFactory.hpp"
#include "com/sun/star/awt/XWindowPeer.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/frame/DispatchDescriptor.hpp"
#include "com/sun/star/frame/XDesktop.hpp"
#include "com/sun/star/frame/XDispatch.hpp"
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XFrame.hpp"
#include "com/sun/star/frame/XStatusListener.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/uno/DeploymentException.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/URL.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/diagnose.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/lbnames.h"

namespace {

namespace css = com::sun::star;

class Provider:
    public cppu::WeakImplHelper2<
        css::lang::XServiceInfo, css::frame::XDispatchProvider >,
    private boost::noncopyable
{
public:
    static css::uno::Reference< css::uno::XInterface > SAL_CALL static_create(
        css::uno::Reference< css::uno::XComponentContext > const & xContext)
        SAL_THROW((css::uno::Exception))
    { return static_cast< cppu::OWeakObject * >(new Provider(xContext)); }

    static rtl::OUString SAL_CALL static_getImplementationName();

    static css::uno::Sequence< rtl::OUString > SAL_CALL
    static_getSupportedServiceNames();

private:
    Provider(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        context_(context) { OSL_ASSERT(context.is()); }

    virtual ~Provider() {}

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
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > >
    SAL_CALL queryDispatches(
        css::uno::Sequence< css::frame::DispatchDescriptor > const & Requests)
        throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context_;
};

rtl::OUString Provider::static_getImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.comp.test.deployment.active_native"));
}

css::uno::Sequence< rtl::OUString > Provider::static_getSupportedServiceNames()
{
    rtl::OUString name(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.test.deployment.active_native"));
    return css::uno::Sequence< rtl::OUString >(&name, 1);
}

css::uno::Reference< css::frame::XDispatch > Provider::queryDispatch(
    css::util::URL const &, rtl::OUString const &, sal_Int32)
    throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::frame::XDispatch > dispatch;
    if (!(context_->getValueByName(
              rtl::OUString(
                  RTL_CONSTASCII_USTRINGPARAM(
                      "/singletons/com.sun.star.test.deployment."
                      "active_native_singleton"))) >>=
          dispatch) ||
        !dispatch.is())
    {
        throw css::uno::DeploymentException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "component context fails to supply singleton"
                    " com.sun.star.test.deployment.active_native_singleton of"
                    " type com.sun.star.frame.XDispatch")),
            context_);
    }
    return dispatch;
}

css::uno::Sequence< css::uno::Reference< css::frame::XDispatch > >
Provider::queryDispatches(
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

class Dispatch:
    public cppu::WeakImplHelper2<
        css::lang::XServiceInfo, css::frame::XDispatch >,
    private boost::noncopyable
{
public:
    static css::uno::Reference< css::uno::XInterface > SAL_CALL static_create(
        css::uno::Reference< css::uno::XComponentContext > const & xContext)
        SAL_THROW((css::uno::Exception))
    { return static_cast< cppu::OWeakObject * >(new Dispatch(xContext)); }

    static rtl::OUString SAL_CALL static_getImplementationName();

    static css::uno::Sequence< rtl::OUString > SAL_CALL
    static_getSupportedServiceNames()
    { return css::uno::Sequence< rtl::OUString >(); }

private:
    Dispatch(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        context_(context) { OSL_ASSERT(context.is()); }

    virtual ~Dispatch() {}

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    { return static_getImplementationName(); }

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const &)
        throw (css::uno::RuntimeException)
    { return false; } //TODO

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (css::uno::RuntimeException)
    { return static_getSupportedServiceNames(); }

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

    css::uno::Reference< css::uno::XComponentContext > context_;
};

rtl::OUString Dispatch::static_getImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.comp.test.deployment.active_native_singleton"));
}

void Dispatch::dispatch(
    css::util::URL const &,
    css::uno::Sequence< css::beans::PropertyValue > const &)
    throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::lang::XMultiComponentFactory > smgr( context_->getServiceManager(), css::uno::UNO_SET_THROW);
    css::uno::Reference< css::awt::XToolkit > toolkit( css::awt::Toolkit::create(context_), css::uno::UNO_SET_THROW);

    css::uno::Reference< css::awt::XMessageBox > box(
        css::uno::Reference< css::awt::XMessageBoxFactory >(
            toolkit,
            css::uno::UNO_QUERY_THROW)->createMessageBox(
                css::uno::Reference< css::awt::XWindowPeer >(
                    css::uno::Reference< css::frame::XFrame >(
                        css::uno::Reference< css::frame::XDesktop >(
                            smgr->createInstanceWithContext(
                                rtl::OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "com.sun.star.frame.Desktop")),
                                context_),
                            css::uno::UNO_QUERY_THROW)->getCurrentFrame(),
                        css::uno::UNO_SET_THROW)->getComponentWindow(),
                    css::uno::UNO_QUERY_THROW),
                css::awt::Rectangle(),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("infobox")),
                css::awt::MessageBoxButtons::BUTTONS_OK,
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("active")),
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("native"))),
        css::uno::UNO_SET_THROW);
    box->execute();
    css::uno::Reference< css::lang::XComponent >(
        box, css::uno::UNO_QUERY_THROW)->dispose();
}

static cppu::ImplementationEntry const services[] = {
    { &Provider::static_create, &Provider::static_getImplementationName,
      &Provider::static_getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { &Dispatch::static_create, &Dispatch::static_getImplementationName,
      &Dispatch::static_getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey)
{
    if (!component_writeInfoHelper(pServiceManager, pRegistryKey, services)) {
        return false;
    }
    try {
        css::uno::Reference< css::registry::XRegistryKey >(
            (css::uno::Reference< css::registry::XRegistryKey >(
                static_cast< css::registry::XRegistryKey * >(pRegistryKey))->
             createKey(
                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) +
                 Dispatch::static_getImplementationName() +
                 rtl::OUString(
                     RTL_CONSTASCII_USTRINGPARAM(
                         "/UNO/SINGLETONS/com.sun.star.test.deployment."
                         "active_native_singleton")))),
            css::uno::UNO_SET_THROW)->
            setStringValue(Dispatch::static_getImplementationName());
    } catch (const css::uno::Exception & e) {
        (void) e;
        OSL_TRACE(
            "active_native component_writeInfo exception: %s",
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
