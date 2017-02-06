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

#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
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

// ContentProvider Implementation.

FTPContentProvider::FTPContentProvider( const Reference< XComponentContext >& rxContext)
    : ::ucbhelper::ContentProviderImplHelper(rxContext)
    , m_ftpLoaderThread(nullptr)
    , m_pProxyDecider(nullptr)
{
}


// virtual
FTPContentProvider::~FTPContentProvider()
{
    delete m_ftpLoaderThread;
    delete m_pProxyDecider;
}

// XInterface methods.
void SAL_CALL FTPContentProvider::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL FTPContentProvider::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL FTPContentProvider::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               (static_cast< XTypeProvider* >(this)),
                                               (static_cast< XServiceInfo* >(this)),
                                               (static_cast< XContentProvider* >(this))
        );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.
css::uno::Sequence< sal_Int8 > SAL_CALL FTPContentProvider::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

css::uno::Sequence< css::uno::Type > SAL_CALL FTPContentProvider::getTypes()
{
    static cppu::OTypeCollection* pCollection = nullptr;
    if ( !pCollection )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                    cppu::UnoType<XTypeProvider>::get(),
                    cppu::UnoType<XServiceInfo>::get(),
                    cppu::UnoType<XContentProvider>::get()
                );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}


// XServiceInfo methods.

OUString SAL_CALL FTPContentProvider::getImplementationName()
{
    return getImplementationName_Static();
}

OUString FTPContentProvider::getImplementationName_Static()
{
    return OUString("com.sun.star.comp.FTPContentProvider");
}

sal_Bool SAL_CALL FTPContentProvider::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL FTPContentProvider::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

/// @throws css::uno::Exception
static css::uno::Reference< css::uno::XInterface > SAL_CALL
FTPContentProvider_CreateInstance( const css::uno::Reference<
                                   css::lang::XMultiServiceFactory> & rSMgr )
{
    css::lang::XServiceInfo* pX = static_cast<css::lang::XServiceInfo*>(
        new FTPContentProvider( ucbhelper::getComponentContext(rSMgr) ));
    return css::uno::Reference< css::uno::XInterface >::query( pX );
}

css::uno::Sequence< OUString > FTPContentProvider::getSupportedServiceNames_Static()
{
    css::uno::Sequence<OUString> aSNS { FTP_CONTENT_PROVIDER_SERVICE_NAME };
    return aSNS;
}

// Service factory implementation.

css::uno::Reference< css::lang::XSingleServiceFactory >
FTPContentProvider::createServiceFactory( const css::uno::Reference<
            css::lang::XMultiServiceFactory >& rxServiceMgr )
{
    return css::uno::Reference<
        css::lang::XSingleServiceFactory >(
            cppu::createOneInstanceFactory(
                rxServiceMgr,
                FTPContentProvider::getImplementationName_Static(),
                FTPContentProvider_CreateInstance,
                FTPContentProvider::getSupportedServiceNames_Static() ) );
}

// XContentProvider methods.

// virtual
Reference<XContent> SAL_CALL FTPContentProvider::queryContent(
        const Reference< XContentIdentifier >& xCanonicId)
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
            "ftp",
            aURL.host(),
            aURL.port().toInt32()))
        {
            xContent = new FTPContent( m_xContext, this,xCanonicId,aURL);
            registerNewContent(xContent);
        }
        else {
            Reference<XContentProvider> xProvider(UniversalContentBroker::create( m_xContext )->queryContentProvider("http:"));
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

void FTPContentProvider::init()
{
    m_ftpLoaderThread = new FTPLoaderThread();
    m_pProxyDecider = new ucbhelper::InternetProxyDecider( m_xContext );
}

CURL* FTPContentProvider::handle()
{
    // Cannot be zero if called from here;
    return m_ftpLoaderThread->handle();
}


bool FTPContentProvider::forHost( const OUString& host,
                                  const OUString& port,
                                  const OUString& username,
                                  OUString& password,
                                  OUString& account)
{
    osl::MutexGuard aGuard(m_aMutex);
    for(ServerInfo & i : m_ServerInfo)
        if(host == i.host &&
           port == i.port &&
           username == i.username )
        {
            password = i.password;
            account = i.account;
            return true;
        }

    return false;
}

bool  FTPContentProvider::setHost( const OUString& host,
                                   const OUString& port,
                                   const OUString& username,
                                   const OUString& password,
                                   const OUString& account)
{
    ServerInfo inf;
    inf.host = host;
    inf.port = port;
    inf.username = username;
    inf.password = password;
    inf.account = account;

    bool present(false);
    osl::MutexGuard aGuard(m_aMutex);
    for(ServerInfo & i : m_ServerInfo)
        if(host == i.host &&
           port == i.port &&
           username == i.username)
        {
            present = true;
            i.password = password;
            i.account = account;
        }

    if(!present)
        m_ServerInfo.push_back(inf);

    return !present;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
