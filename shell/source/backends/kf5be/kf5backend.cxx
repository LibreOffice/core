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
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XCurrentContext.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <uno/current_context.hxx>

#include <osl/process.h>
#include <osl/thread.h>

#include "kf5access.hxx"

namespace
{
class Service : public cppu::WeakImplHelper<css::lang::XServiceInfo, css::beans::XPropertySet>,
                private boost::noncopyable
{
public:
    Service();

private:
    virtual ~Service() override {}

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.configuration.backend.KF5Backend";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const& ServiceName) override
    {
        return ServiceName == getSupportedServiceNames()[0];
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { "com.sun.star.configuration.backend.KF5Backend" };
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

    std::map<OUString, css::beans::Optional<css::uno::Any>> m_KDESettings;
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

void readKDESettings(std::map<OUString, css::beans::Optional<css::uno::Any>>& rSettings)
{
    const std::vector<OUString> aKeys
        = { "EnableATToolSupport",  "ExternalMailer",       "SourceViewFontHeight",
            "SourceViewFontName",   "WorkPathVariable",     "ooInetFTPProxyName",
            "ooInetFTPProxyPort",   "ooInetHTTPProxyName",  "ooInetHTTPProxyPort",
            "ooInetHTTPSProxyName", "ooInetHTTPSProxyPort", "ooInetNoProxy",
            "ooInetProxyType" };

    for (const OUString& aKey : aKeys)
    {
        css::beans::Optional<css::uno::Any> aValue = kf5access::getValue(aKey);
        std::pair<OUString, css::beans::Optional<css::uno::Any>> elem
            = std::make_pair(aKey, aValue);
        rSettings.insert(elem);
    }
}

// init the QApplication when we load the kf5backend into a non-Qt vclplug (e.g. gtk3_kde5)
// TODO: use a helper process to read these values without linking to Qt directly?
// TODO: share this code somehow with Qt5Instance.cxx?
void initQApp(std::map<OUString, css::beans::Optional<css::uno::Any>>& rSettings)
{
    const auto aDisplay = getDisplayArg();
    int nFakeArgc = aDisplay.isEmpty() ? 2 : 3;
    char** pFakeArgv = new char*[nFakeArgc];

    pFakeArgv[0] = strdup(getExecutable().getStr());
    pFakeArgv[1] = strdup("--nocrashhandler");
    if (!aDisplay.isEmpty())
        pFakeArgv[2] = strdup(aDisplay.getStr());

    char* session_manager = nullptr;
    if (auto* session_manager_env = getenv("SESSION_MANAGER"))
    {
        session_manager = strdup(session_manager_env);
        unsetenv("SESSION_MANAGER");
    }

    std::unique_ptr<QApplication> app(new QApplication(nFakeArgc, pFakeArgv));
    QObject::connect(app.get(), &QObject::destroyed, app.get(), [nFakeArgc, pFakeArgv]() {
        for (int i = 0; i < nFakeArgc; ++i)
            free(pFakeArgv[i]);
        delete[] pFakeArgv;
    });

    readKDESettings(rSettings);

    if (session_manager != nullptr)
    {
        // coverity[tainted_string] - trusted source for setenv
        setenv("SESSION_MANAGER", session_manager, 1);
        free(session_manager);
    }
}

Service::Service()
{
    css::uno::Reference<css::uno::XCurrentContext> context(css::uno::getCurrentContext());
    if (!context.is())
        return;

    OUString desktop;
    context->getValueByName("system.desktop-environment") >>= desktop;

    if (desktop == "PLASMA5")
    {
        if (!qApp) // no qt event loop yet
        {
            // so we start one and read KDE settings
            initQApp(m_KDESettings);
        }
        else // someone else (most likely kde/qt vclplug) has started qt event loop
            // all that is left to do is to read KDE settings
            readKDESettings(m_KDESettings);
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
        std::map<OUString, css::beans::Optional<css::uno::Any>>::iterator it
            = m_KDESettings.find(PropertyName);
        if (it != m_KDESettings.end())
            return css::uno::makeAny(it->second);
        else
            return css::uno::makeAny(css::beans::Optional<css::uno::Any>());
    }
    else if (PropertyName == "givenname" || PropertyName == "sn"
             || PropertyName == "TemplatePathVariable")
    {
        return css::uno::makeAny(css::beans::Optional<css::uno::Any>());
        //TODO: obtain values from KDE?
    }
    throw css::beans::UnknownPropertyException(PropertyName, static_cast<cppu::OWeakObject*>(this));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
shell_kf5desktop_get_implementation(css::uno::XComponentContext*,
                                    css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new Service());
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
