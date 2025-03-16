/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <cstddef>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/util/URL.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustring.hxx>
#include <uno/lbnames.h>

namespace
{
class Provider : public cppu::WeakImplHelper3<css::lang::XServiceInfo,
                                              css::frame::XDispatchProvider, css::frame::XDispatch>
{
public:
    Provider(const Provider&) = delete;
    const Provider& operator=(const Provider&) = delete;

    static css::uno::Reference<css::uno::XInterface>
        SAL_CALL static_create(css::uno::Reference<css::uno::XComponentContext> const& xContext)
    {
        return static_cast<cppu::OWeakObject*>(new Provider(xContext));
    }

    static rtl::OUString SAL_CALL static_getImplementationName();

    static css::uno::Sequence<rtl::OUString> SAL_CALL static_getSupportedServiceNames();

private:
    explicit Provider(css::uno::Reference<css::uno::XComponentContext> const& context)
        : context_(context)
    {
        assert(context.is());
    }

    virtual ~Provider() {}

    virtual rtl::OUString SAL_CALL getImplementationName() override
    {
        return static_getImplementationName();
    }

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const& ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames() override
    {
        return static_getSupportedServiceNames();
    }

    virtual css::uno::Reference<css::frame::XDispatch>
        SAL_CALL queryDispatch(css::util::URL const&, rtl::OUString const&, sal_Int32) override;

    virtual css::uno::Sequence<css::uno::Reference<css::frame::XDispatch>> SAL_CALL
    queryDispatches(css::uno::Sequence<css::frame::DispatchDescriptor> const& Requests) override;

    virtual void SAL_CALL dispatch(css::util::URL const&,
                                   css::uno::Sequence<css::beans::PropertyValue> const&) override;

    virtual void SAL_CALL addStatusListener(css::uno::Reference<css::frame::XStatusListener> const&,
                                            css::util::URL const&) override
    {
    }

    virtual void SAL_CALL removeStatusListener(
        css::uno::Reference<css::frame::XStatusListener> const&, css::util::URL const&) override
    {
    }

    css::uno::Reference<css::uno::XComponentContext> context_;
};

rtl::OUString Provider::static_getImplementationName()
{
    return rtl::OUString("org.libreoffice.test.desktop.deployment.crashextension.impl");
}

css::uno::Sequence<rtl::OUString> Provider::static_getSupportedServiceNames()
{
    rtl::OUString name("org.libreoffice.test.desktop.deployment.crashextension");
    return css::uno::Sequence<rtl::OUString>(&name, 1);
}

css::uno::Reference<css::frame::XDispatch> Provider::queryDispatch(css::util::URL const&,
                                                                   rtl::OUString const&, sal_Int32)
{
    return this;
}

css::uno::Sequence<css::uno::Reference<css::frame::XDispatch>>
Provider::queryDispatches(css::uno::Sequence<css::frame::DispatchDescriptor> const& Requests)
{
    css::uno::Sequence<css::uno::Reference<css::frame::XDispatch>> s(Requests.getLength());
    for (sal_Int32 i = 0; i < s.getLength(); ++i)
    {
        s[i]
            = queryDispatch(Requests[i].FeatureURL, Requests[i].FrameName, Requests[i].SearchFlags);
    }
    return s;
}

void Provider::dispatch(css::util::URL const&, css::uno::Sequence<css::beans::PropertyValue> const&)
{
    // Crash LibreOffice
    char volatile* p = nullptr;
    *p = 0;
}

cppu::ImplementationEntry const services[]
    = { { &Provider::static_create, &Provider::static_getImplementationName,
          &Provider::static_getSupportedServiceNames, &cppu::createSingleComponentFactory, nullptr,
          0 },
        { nullptr, nullptr, nullptr, nullptr, nullptr, 0 } };
}

extern "C" SAL_DLLPUBLIC_EXPORT void*
component_getFactory(char const* pImplName, void* pServiceManager, void* pRegistryKey)
{
    return cppu::component_getFactoryHelper(pImplName, pServiceManager, pRegistryKey, services);
}

extern "C" SAL_DLLPUBLIC_EXPORT void
component_getImplementationEnvironment(char const** ppEnvTypeName, uno_Environment**)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
