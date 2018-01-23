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

#include <sal/config.h>

#include <QtWidgets/QApplication>

#include <boost/noncopyable.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/string.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <uno/current_context.hxx>

#include <osl/process.h>
#include <osl/thread.h>

#include "kde5access.hxx"

namespace
{
OUString getServiceImplementationName()
{
    return OUString("com.sun.star.comp.configuration.backend.KDE5Backend");
}

css::uno::Sequence<OUString> getServiceSupportedServiceNames()
{
    OUString name("com.sun.star.configuration.backend.KDE5Backend");
    return css::uno::Sequence<OUString>(&name, 1);
}

class Service : public cppu::WeakImplHelper<css::lang::XServiceInfo, css::beans::XPropertySet>,
                private boost::noncopyable
{
public:
    Service();

private:
    virtual ~Service() override {}

    virtual OUString SAL_CALL getImplementationName() override
    {
        return getServiceImplementationName();
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override
    {
        return ServiceName == getSupportedServiceNames()[0];
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return getServiceSupportedServiceNames();
    }

    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override
    {
        return css::uno::Reference<css::beans::XPropertySetInfo>();
    }

    virtual void SAL_CALL setPropertyValue(OUString const&, css::uno::Any const&) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(OUString const& PropertyName) override;

    virtual void SAL_CALL addPropertyChangeListener(
        OUString const&, css::uno::Reference<css::beans::XPropertyChangeListener> const&) override
    {
    }

    virtual void SAL_CALL removePropertyChangeListener(
        OUString const&, css::uno::Reference<css::beans::XPropertyChangeListener> const&) override
    {
    }

    virtual void SAL_CALL addVetoableChangeListener(
        OUString const&, css::uno::Reference<css::beans::XVetoableChangeListener> const&) override
    {
    }

    virtual void SAL_CALL removeVetoableChangeListener(
        OUString const&, css::uno::Reference<css::beans::XVetoableChangeListener> const&) override
    {
    }

    bool enabled_;
};

OString getDisplayArg()
{
    OUString aParam;
    const sal_uInt32 nParams = osl_getCommandArgCount();
    for (sal_uInt32 nIdx = 0; nIdx < nParams; ++nIdx)
    {
        osl_getCommandArg(nIdx, &aParam.pData);
        if (aParam != "-display")
            continue;

        ++nIdx;
        osl_getCommandArg(nIdx, &aParam.pData);
        return OUStringToOString(aParam, osl_getThreadTextEncoding());
    }
    return {};
}

OString getExecutable()
{
    OUString aParam, aBin;
    osl_getExecutableFile(&aParam.pData);
    osl_getSystemPathFromFileURL(aParam.pData, &aBin.pData);
    return OUStringToOString(aBin, osl_getThreadTextEncoding());
}

// init the QApplication when we load the kde5backend into a non-Qt vclplug (e.g. Gtk3KDE5)
// TODO: use a helper process to read these values without linking to Qt directly?
// TODO: share this code somehow with Qt5Instance.cxx?
void initQApp()
{
    const auto aDisplay = getDisplayArg();
    int nFakeArgc = aDisplay.isEmpty() ? 2 : 3;
    char** pFakeArgv = new char*[nFakeArgc];

    pFakeArgv[0] = strdup(getExecutable().getStr());
    pFakeArgv[1] = strdup("--nocrashhandler");
    if (aDisplay.isEmpty())
        pFakeArgv[2] = strdup(aDisplay.getStr());

    char* session_manager = nullptr;
    if (auto* session_manager_env = getenv("SESSION_MANAGER"))
    {
        session_manager = strdup(session_manager_env);
        unsetenv("SESSION_MANAGER");
    }

    auto app = new QApplication(nFakeArgc, pFakeArgv);
    QObject::connect(app, &QObject::destroyed, app, [nFakeArgc, pFakeArgv]() {
        for (int i = 0; i < nFakeArgc; ++i)
            delete pFakeArgv[i];
        delete[] pFakeArgv;
    });

    if (session_manager != nullptr)
    {
        // coverity[tainted_string] - trusted source for setenv
        setenv("SESSION_MANAGER", session_manager, 1);
        free(session_manager);
    }

    QApplication::setQuitOnLastWindowClosed(false);
}

Service::Service()
    : enabled_(false)
{
    css::uno::Reference<css::uno::XCurrentContext> context(css::uno::getCurrentContext());
    if (context.is())
    {
        if (!qApp)
        {
            initQApp();
        }
        OUString desktop;
        context->getValueByName("system.desktop-environment") >>= desktop;
        enabled_ = desktop == "KDE5" && qApp != nullptr;
    }
}

void Service::setPropertyValue(OUString const&, css::uno::Any const&)
{
    throw css::lang::IllegalArgumentException("setPropertyValue not supported",
                                              static_cast<cppu::OWeakObject*>(this), -1);
}

css::uno::Any Service::getPropertyValue(OUString const& PropertyName)
{
    if (PropertyName == "EnableATToolSupport" || PropertyName == "ExternalMailer"
        || PropertyName == "SourceViewFontHeight" || PropertyName == "SourceViewFontName"
        || PropertyName == "WorkPathVariable" || PropertyName == "ooInetFTPProxyName"
        || PropertyName == "ooInetFTPProxyPort" || PropertyName == "ooInetHTTPProxyName"
        || PropertyName == "ooInetHTTPProxyPort" || PropertyName == "ooInetHTTPSProxyName"
        || PropertyName == "ooInetHTTPSProxyPort" || PropertyName == "ooInetNoProxy"
        || PropertyName == "ooInetProxyType")
    {
        return css::uno::makeAny(enabled_ ? kde5access::getValue(PropertyName)
                                          : css::beans::Optional<css::uno::Any>());
    }
    else if (PropertyName == "givenname" || PropertyName == "sn"
             || PropertyName == "TemplatePathVariable")
    {
        return css::uno::makeAny(css::beans::Optional<css::uno::Any>());
        //TODO: obtain values from KDE?
    }
    throw css::beans::UnknownPropertyException(PropertyName, static_cast<cppu::OWeakObject*>(this));
}

css::uno::Reference<css::uno::XInterface>
createInstance(css::uno::Reference<css::uno::XComponentContext> const&)
{
    return static_cast<cppu::OWeakObject*>(new Service);
}

static cppu::ImplementationEntry const services[]
    = { { &createInstance, &getServiceImplementationName, &getServiceSupportedServiceNames,
          &cppu::createSingleComponentFactory, nullptr, 0 },
        { nullptr, nullptr, nullptr, nullptr, nullptr, 0 } };
}

extern "C" SAL_DLLPUBLIC_EXPORT void*
kde5be1_component_getFactory(char const* pImplName, void* pServiceManager, void* pRegistryKey)
{
    return cppu::component_getFactoryHelper(pImplName, pServiceManager, pRegistryKey, services);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
