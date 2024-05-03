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

#include <writer/WConnection.hxx>
#include <writer/WDatabaseMetaData.hxx>
#include <writer/WCatalog.hxx>
#include <writer/WDriver.hxx>
#include <resource/sharedresources.hxx>
#include <strings.hrc>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <tools/urlobj.hxx>
#include <sal/log.hxx>
#include <component/CPreparedStatement.hxx>
#include <component/CStatement.hxx>
#include <unotools/pathoptions.hxx>
#include <connectivity/dbexception.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;

using OConnection_BASE = connectivity::file::OConnection;

namespace connectivity::writer
{
OWriterConnection::OWriterConnection(ODriver* _pDriver)
    : OConnection(_pDriver)
{
}

OWriterConnection::~OWriterConnection() = default;

void OWriterConnection::construct(const OUString& rURL,
                                  const uno::Sequence<beans::PropertyValue>& rInfo)
{
    //  open file

    sal_Int32 nLen = rURL.indexOf(':');
    nLen = rURL.indexOf(':', nLen + 1);

    m_aFileName = rURL.copy(nLen + 1); // DSN

    INetURLObject aURL;
    aURL.SetSmartProtocol(INetProtocol::File);
    {
        SvtPathOptions aPathOptions;
        m_aFileName = aPathOptions.SubstituteVariable(m_aFileName);
    }
    aURL.SetSmartURL(m_aFileName);
    if (aURL.GetProtocol() == INetProtocol::NotValid)
    {
        //  don't pass invalid URL to loadComponentFromURL
        throw sdbc::SQLException();
    }
    m_aFileName = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);

    m_sPassword.clear();
    const char pPwd[] = "password";

    const beans::PropertyValue* pIter = rInfo.getConstArray();
    const beans::PropertyValue* pEnd = pIter + rInfo.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        if (pIter->Name == pPwd)
        {
            pIter->Value >>= m_sPassword;
            break;
        }
    } // for(;pIter != pEnd;++pIter)
    ODocHolder aDocHolder(this); // just to test that the doc can be loaded
    acquireDoc();
}

uno::Reference<text::XTextDocument> const& OWriterConnection::acquireDoc()
{
    if (m_xDoc.is())
    {
        osl_atomic_increment(&m_nDocCount);
        return m_xDoc;
    }
    //  open read-only as long as updating isn't implemented
    uno::Sequence<beans::PropertyValue> aArgs(m_sPassword.isEmpty() ? 2 : 3);
    auto pArgs = aArgs.getArray();
    pArgs[0].Name = "Hidden";
    pArgs[0].Value <<= true;
    pArgs[1].Name = "ReadOnly";
    pArgs[1].Value <<= true;

    if (!m_sPassword.isEmpty())
    {
        pArgs[2].Name = "Password";
        pArgs[2].Value <<= m_sPassword;
    }

    uno::Reference<frame::XDesktop2> xDesktop
        = frame::Desktop::create(getDriver()->getComponentContext());
    uno::Reference<lang::XComponent> xComponent;
    uno::Any aLoaderException;
    try
    {
        xComponent = xDesktop->loadComponentFromURL(m_aFileName, "_blank", 0, aArgs);
    }
    catch (const uno::Exception&)
    {
        aLoaderException = ::cppu::getCaughtException();
    }

    m_xDoc.set(xComponent, uno::UNO_QUERY);

    //  if the URL is not a text document, throw the exception here
    //  instead of at the first access to it
    if (!m_xDoc.is())
    {
        if (aLoaderException.hasValue())
        {
            uno::Exception aLoaderError;
            OSL_VERIFY(aLoaderException >>= aLoaderError);

            SAL_WARN("connectivity.writer",
                     "empty m_xDoc, " << exceptionToString(aLoaderException));
        }

        const OUString sError(m_aResources.getResourceStringWithSubstitution(
            STR_COULD_NOT_LOAD_FILE, "$filename$", m_aFileName));
        ::dbtools::throwGenericSQLException(sError, *this);
    }
    osl_atomic_increment(&m_nDocCount);
    m_xCloseVetoButTerminateListener.set(new CloseVetoButTerminateListener);
    m_xCloseVetoButTerminateListener->start(m_xDoc, xDesktop);
    return m_xDoc;
}

void OWriterConnection::releaseDoc()
{
    if (osl_atomic_decrement(&m_nDocCount) == 0)
    {
        if (m_xCloseVetoButTerminateListener.is())
        {
            m_xCloseVetoButTerminateListener->stop(); // dispose m_xDoc
            m_xCloseVetoButTerminateListener.clear();
        }
        m_xDoc.clear();
    }
}

void OWriterConnection::disposing()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    m_nDocCount = 0;
    if (m_xCloseVetoButTerminateListener.is())
    {
        m_xCloseVetoButTerminateListener->stop(); // dispose m_xDoc
        m_xCloseVetoButTerminateListener.clear();
    }
    m_xDoc.clear();

    OConnection::disposing();
}

// XServiceInfo

IMPLEMENT_SERVICE_INFO(OWriterConnection, "com.sun.star.sdbc.drivers.writer.Connection",
                       "com.sun.star.sdbc.Connection")

uno::Reference<sdbc::XDatabaseMetaData> SAL_CALL OWriterConnection::getMetaData()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    uno::Reference<sdbc::XDatabaseMetaData> xMetaData = m_xMetaData;
    if (!xMetaData.is())
    {
        xMetaData = new OWriterDatabaseMetaData(this);
        m_xMetaData = xMetaData;
    }

    return xMetaData;
}

css::uno::Reference<css::sdbcx::XTablesSupplier> OWriterConnection::createCatalog()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    uno::Reference<css::sdbcx::XTablesSupplier> xTab = m_xCatalog;
    if (!xTab.is())
    {
        xTab = new OWriterCatalog(this);
        m_xCatalog = xTab;
    }
    return xTab;
}

uno::Reference<sdbc::XStatement> SAL_CALL OWriterConnection::createStatement()
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    uno::Reference<sdbc::XStatement> xReturn = new component::OComponentStatement(this);
    m_aStatements.emplace_back(xReturn);
    return xReturn;
}

uno::Reference<sdbc::XPreparedStatement>
    SAL_CALL OWriterConnection::prepareStatement(const OUString& sql)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    rtl::Reference<component::OComponentPreparedStatement> pStmt
        = new component::OComponentPreparedStatement(this);
    pStmt->construct(sql);
    m_aStatements.emplace_back(*pStmt);
    return pStmt;
}

uno::Reference<sdbc::XPreparedStatement>
    SAL_CALL OWriterConnection::prepareCall(const OUString& /*sql*/)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OConnection_BASE::rBHelper.bDisposed);

    ::dbtools::throwFeatureNotImplementedSQLException("XConnection::prepareCall", *this);
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
