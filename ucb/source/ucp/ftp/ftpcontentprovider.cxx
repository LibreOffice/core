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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/socket.hxx>
#include "ftpcontentprovider.hxx"
#include "ftpcontent.hxx"
#include "ftploaderthread.hxx"


using namespace ftp;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;
using namespace com::sun::star::beans;



//=========================================================================
//=========================================================================
//
// ContentProvider Implementation.
//
//=========================================================================
//=========================================================================

FTPContentProvider::FTPContentProvider(
    const Reference< XMultiServiceFactory >& rSMgr)
: ::ucbhelper::ContentProviderImplHelper(rSMgr),
  m_ftpLoaderThread(0),
  m_pProxyDecider(0)
{
}

//=========================================================================
// virtual
FTPContentProvider::~FTPContentProvider()
{
    delete m_ftpLoaderThread;
    delete m_pProxyDecider;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_3(FTPContentProvider,
                  XTypeProvider,
                  XServiceInfo,
                  XContentProvider)

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3(FTPContentProvider,
                     XTypeProvider,
                     XServiceInfo,
                     XContentProvider)

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_IMPL_1(
    FTPContentProvider,
    rtl::OUString("com.sun.star.comp.FTPContentProvider"),
    rtl::OUString(FTP_CONTENT_PROVIDER_SERVICE_NAME));

//=========================================================================
//
// Service factory implementation.
//
//=========================================================================

ONE_INSTANCE_SERVICE_FACTORY_IMPL(FTPContentProvider);


//=========================================================================
//
// XContentProvider methods.
//
//=========================================================================

// virtual
Reference<XContent> SAL_CALL
FTPContentProvider::queryContent(
    const Reference< XContentIdentifier >& xCanonicId
)
    throw(
        IllegalIdentifierException,
        RuntimeException
    )
{
    // Check, if a content with given id already exists...
    Reference<XContent> xContent = queryExistingContent(xCanonicId).get();
    if(xContent.is())
        return xContent;

    // A new content has to be returned:
    {
        // Initialize
        osl::MutexGuard aGuard( m_aMutex );
        if(!m_ftpLoaderThread || !m_pProxyDecider)
        {
            try {
                init();
            } catch( ... ) {
                throw RuntimeException();
            }

            if(!m_ftpLoaderThread || !m_pProxyDecider)
                throw RuntimeException();
        }
    }

    try {
        FTPURL aURL(xCanonicId->getContentIdentifier(),
                    this);

        if(!m_pProxyDecider->shouldUseProxy(
            rtl::OUString("ftp"),
            aURL.host(),
            aURL.port().toInt32()))
        {
            xContent = new FTPContent(m_xSMgr,this,xCanonicId,aURL);
            registerNewContent(xContent);
        }
        else {
            Reference<XContentProvider>
                xProvider(getHttpProvider());
            if(xProvider.is())
                return xProvider->queryContent(xCanonicId);
            else
                throw RuntimeException();
        }
    } catch(const malformed_exception&) {
        throw IllegalIdentifierException();
    }

    // may throw IllegalIdentifierException
    return xContent;
}




void FTPContentProvider::init() {
    m_ftpLoaderThread = new FTPLoaderThread();
    m_pProxyDecider = new ucbhelper::InternetProxyDecider(comphelper::getComponentContext(m_xSMgr));
}



CURL* FTPContentProvider::handle() {
    // Cannot be zero if called from here;
    return m_ftpLoaderThread->handle();
}


bool FTPContentProvider::forHost(
    const rtl::OUString& host,
    const rtl::OUString& port,
    const rtl::OUString& username,
    rtl::OUString& password,
    rtl::OUString& account)
{
    osl::MutexGuard aGuard(m_aMutex);
    for(unsigned int i = 0; i < m_ServerInfo.size(); ++i)
        if(host == m_ServerInfo[i].host &&
           port == m_ServerInfo[i].port &&
           username == m_ServerInfo[i].username )
        {
            password = m_ServerInfo[i].password;
            account = m_ServerInfo[i].account;
            return true;
        }

    return false;
}


bool  FTPContentProvider::setHost(
    const rtl::OUString& host,
    const rtl::OUString& port,
    const rtl::OUString& username,
    const rtl::OUString& password,
    const rtl::OUString& account)
{
    ServerInfo inf;
    inf.host = host;
    inf.port = port;
    inf.username = username;
    inf.password = password;
    inf.account = account;

    bool present(false);
    osl::MutexGuard aGuard(m_aMutex);
    for(unsigned int i = 0; i < m_ServerInfo.size(); ++i)
        if(host == m_ServerInfo[i].host &&
           port == m_ServerInfo[i].port &&
           username == m_ServerInfo[i].username)
        {
            present = true;
            m_ServerInfo[i].password = password;
            m_ServerInfo[i].account = account;
        }

    if(!present)
        m_ServerInfo.push_back(inf);

    return !present;
}



Reference<XContentProvider>
FTPContentProvider::getHttpProvider()
    throw(RuntimeException)
{
    // used for access to ftp-proxy
    return
        UniversalContentBroker::create(
            comphelper::getComponentContext(m_xSMgr))->
        queryContentProvider("http:");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
