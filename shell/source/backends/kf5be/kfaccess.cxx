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

#include "kfaccess.hxx"

#include <QtGui/QFont>
#include <QtCore/QString>
#include <QtGui/QFontDatabase>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QUrl>

#include <kprotocolmanager.h>

#include <kemailsettings.h>
// #include <kglobalsettings.h>

#include <com/sun/star/uno/Any.hxx>
#include <osl/diagnose.h>
#include <osl/file.h>
#include <rtl/ustring.hxx>

namespace kfaccess
{
namespace
{
namespace uno = css::uno;
}

namespace
{
OUString fromQStringToOUString(QString const& s)
{
    // Conversion from QString size()'s int to OUString's sal_Int32 should be non-narrowing:
    return { reinterpret_cast<char16_t const*>(s.utf16()), s.size() };
}
}

css::beans::Optional<css::uno::Any> getValue(std::u16string_view id)
{
    if (id == u"ExternalMailer")
    {
        KEMailSettings aEmailSettings;
        QString aClientProgram;
        OUString sClientProgram;

        aClientProgram = aEmailSettings.getSetting(KEMailSettings::ClientProgram);
        if (aClientProgram.isEmpty())
            aClientProgram = QStringLiteral("kmail");
        else
            aClientProgram = aClientProgram.section(QLatin1Char(' '), 0, 0);
        sClientProgram = fromQStringToOUString(aClientProgram);
        return css::beans::Optional<css::uno::Any>(true, uno::Any(sClientProgram));
    }
    else if (id == u"SourceViewFontHeight")
    {
        const QFont aFixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        const short nFontHeight = aFixedFont.pointSize();
        return css::beans::Optional<css::uno::Any>(true, uno::Any(nFontHeight));
    }
    else if (id == u"SourceViewFontName")
    {
        const QFont aFixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        const QString aFontName = aFixedFont.family();
        const OUString sFontName = fromQStringToOUString(aFontName);
        return css::beans::Optional<css::uno::Any>(true, uno::Any(sFontName));
    }
    else if (id == u"EnableATToolSupport")
    {
        /* does not make much sense without an accessibility bridge */
        bool ATToolSupport = false;
        return css::beans::Optional<css::uno::Any>(true,
                                                   uno::Any(OUString::boolean(ATToolSupport)));
    }
    else if (id == u"WorkPathVariable")
    {
        QString aDocumentsDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        if (aDocumentsDir.isEmpty())
            aDocumentsDir = QDir::homePath();
        OUString sDocumentsDir;
        OUString sDocumentsURL;
        if (aDocumentsDir.endsWith(QLatin1Char('/')))
            aDocumentsDir.truncate(aDocumentsDir.length() - 1);
        sDocumentsDir = fromQStringToOUString(aDocumentsDir);
        osl_getFileURLFromSystemPath(sDocumentsDir.pData, &sDocumentsURL.pData);
        return css::beans::Optional<css::uno::Any>(true, uno::Any(sDocumentsURL));
    }
    else if (id == u"ooInetHTTPProxyName")
    {
        QString aHTTPProxy;
        switch (KProtocolManager::proxyType())
        {
            case KProtocolManager::ManualProxy: // Proxies are manually configured
                aHTTPProxy = KProtocolManager::proxyFor(QStringLiteral("HTTP"));
                break;
            case KProtocolManager::PACProxy: // A proxy configuration URL has been given
            case KProtocolManager::WPADProxy: // A proxy should be automatically discovered
            case KProtocolManager::EnvVarProxy: // Proxy values set through environment variables
                // In such cases, the proxy address is not stored in KDE, but determined dynamically.
                // The proxy address may depend on the requested address, on the time of the day, on the speed of the wind...
                // The best we can do here is to ask the current value for a given address.
                aHTTPProxy = KProtocolManager::proxyForUrl(
                    QUrl(QStringLiteral("http://www.libreoffice.org")));
                break;
            default: // No proxy is used
                break;
        }
        if (!aHTTPProxy.isEmpty())
        {
            QUrl aProxy(aHTTPProxy);
            OUString sProxy = fromQStringToOUString(aProxy.host());
            return css::beans::Optional<css::uno::Any>(true, uno::Any(sProxy));
        }
    }
    else if (id == u"ooInetHTTPProxyPort")
    {
        QString aHTTPProxy;
        switch (KProtocolManager::proxyType())
        {
            case KProtocolManager::ManualProxy: // Proxies are manually configured
                aHTTPProxy = KProtocolManager::proxyFor(QStringLiteral("HTTP"));
                break;
            case KProtocolManager::PACProxy: // A proxy configuration URL has been given
            case KProtocolManager::WPADProxy: // A proxy should be automatically discovered
            case KProtocolManager::EnvVarProxy: // Proxy values set through environment variables
                // In such cases, the proxy address is not stored in KDE, but determined dynamically.
                // The proxy address may depend on the requested address, on the time of the day, on the speed of the wind...
                // The best we can do here is to ask the current value for a given address.
                aHTTPProxy = KProtocolManager::proxyForUrl(
                    QUrl(QStringLiteral("http://www.libreoffice.org")));
                break;
            default: // No proxy is used
                break;
        }
        if (!aHTTPProxy.isEmpty())
        {
            QUrl aProxy(aHTTPProxy);
            sal_Int32 nPort = aProxy.port();
            return css::beans::Optional<css::uno::Any>(true, uno::Any(nPort));
        }
    }
    else if (id == u"ooInetHTTPSProxyName")
    {
        QString aHTTPSProxy;
        switch (KProtocolManager::proxyType())
        {
            case KProtocolManager::ManualProxy: // Proxies are manually configured
                aHTTPSProxy = KProtocolManager::proxyFor(QStringLiteral("HTTPS"));
                break;
            case KProtocolManager::PACProxy: // A proxy configuration URL has been given
            case KProtocolManager::WPADProxy: // A proxy should be automatically discovered
            case KProtocolManager::EnvVarProxy: // Proxy values set through environment variables
                // In such cases, the proxy address is not stored in KDE, but determined dynamically.
                // The proxy address may depend on the requested address, on the time of the day, on the speed of the wind...
                // The best we can do here is to ask the current value for a given address.
                aHTTPSProxy = KProtocolManager::proxyForUrl(
                    QUrl(QStringLiteral("https://www.libreoffice.org")));
                break;
            default: // No proxy is used
                break;
        }
        if (!aHTTPSProxy.isEmpty())
        {
            QUrl aProxy(aHTTPSProxy);
            OUString sProxy = fromQStringToOUString(aProxy.host());
            return css::beans::Optional<css::uno::Any>(true, uno::Any(sProxy));
        }
    }
    else if (id == u"ooInetHTTPSProxyPort")
    {
        QString aHTTPSProxy;
        switch (KProtocolManager::proxyType())
        {
            case KProtocolManager::ManualProxy: // Proxies are manually configured
                aHTTPSProxy = KProtocolManager::proxyFor(QStringLiteral("HTTPS"));
                break;
            case KProtocolManager::PACProxy: // A proxy configuration URL has been given
            case KProtocolManager::WPADProxy: // A proxy should be automatically discovered
            case KProtocolManager::EnvVarProxy: // Proxy values set through environment variables
                // In such cases, the proxy address is not stored in KDE, but determined dynamically.
                // The proxy address may depend on the requested address, on the time of the day, on the speed of the wind...
                // The best we can do here is to ask the current value for a given address.
                aHTTPSProxy = KProtocolManager::proxyForUrl(
                    QUrl(QStringLiteral("https://www.libreoffice.org")));
                break;
            default: // No proxy is used
                break;
        }
        if (!aHTTPSProxy.isEmpty())
        {
            QUrl aProxy(aHTTPSProxy);
            sal_Int32 nPort = aProxy.port();
            return css::beans::Optional<css::uno::Any>(true, uno::Any(nPort));
        }
    }
    else if (id == u"ooInetNoProxy")
    {
        QString aNoProxyFor;
        switch (KProtocolManager::proxyType())
        {
            case KProtocolManager::ManualProxy: // Proxies are manually configured
            case KProtocolManager::PACProxy: // A proxy configuration URL has been given
            case KProtocolManager::WPADProxy: // A proxy should be automatically discovered
            case KProtocolManager::EnvVarProxy: // Proxy values set through environment variables
                aNoProxyFor = KProtocolManager::noProxyFor();
                break;
            default: // No proxy is used
                break;
        }
        if (!aNoProxyFor.isEmpty())
        {
            OUString sNoProxyFor;

            aNoProxyFor = aNoProxyFor.replace(QLatin1Char(','), QLatin1Char(';'));
            sNoProxyFor = fromQStringToOUString(aNoProxyFor);
            return css::beans::Optional<css::uno::Any>(true, uno::Any(sNoProxyFor));
        }
    }
    else if (id == u"ooInetProxyType")
    {
        sal_Int32 nProxyType;
        switch (KProtocolManager::proxyType())
        {
            case KProtocolManager::ManualProxy: // Proxies are manually configured
            case KProtocolManager::PACProxy: // A proxy configuration URL has been given
            case KProtocolManager::WPADProxy: // A proxy should be automatically discovered
            case KProtocolManager::EnvVarProxy: // Proxy values set through environment variables
                nProxyType = 1;
                break;
            default: // No proxy is used
                nProxyType = 0;
        }
        return css::beans::Optional<css::uno::Any>(true, uno::Any(nProxyType));
    }
    else
    {
        OSL_ASSERT(false); // this cannot happen
    }
    return css::beans::Optional<css::uno::Any>();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
