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

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/ucb/IllegalIdentifierException.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/weak.hxx>
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
{
}


// virtual
FTPContentProvider::~FTPContentProvider()
{
    m_ftpLoaderThread.reset();
    m_pProxyDecider.reset();
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
                                               static_cast< XTypeProvider* >(this),
                                               static_cast< XServiceInfo* >(this),
                                               static_cast< XContentProvider* >(this)
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
    static cppu::OTypeCollection s_aCollection(
                    cppu::UnoType<XTypeProvider>::get(),
                    cppu::UnoType<XServiceInfo>::get(),
                    cppu::UnoType<XContentProvider>::get()
                );

    return s_aCollection.getTypes();
}


// XServiceInfo methods.

OUString SAL_CALL FTPContentProvider::getImplementationName()
{
    return "com.sun.star.comp.FTPContentProvider";
}

sal_Bool SAL_CALL FTPContentProvider::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL FTPContentProvider::getSupportedServiceNames()
{
    return { FTP_CONTENT_PROVIDER_SERVICE_NAME };
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
            } catch (css::uno::Exception const & ex) {
                css::uno::Any anyEx = cppu::getCaughtException();
                throw css::lang::WrappedTargetRuntimeException( ex.Message,
                                css::uno::Reference< css::uno::XInterface >(),
                                anyEx );
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
            if(!xProvider.is())
                throw RuntimeException();
            return xProvider->queryContent(xCanonicId);
        }
    } catch(const malformed_exception&) {
        throw IllegalIdentifierException();
    }

    // may throw IllegalIdentifierException
    return xContent;
}

void FTPContentProvider::init()
{
    m_ftpLoaderThread.reset( new FTPLoaderThread() );
    m_pProxyDecider.reset( new ucbhelper::InternetProxyDecider( m_xContext ) );
}

CURL* FTPContentProvider::handle()
{
    // Cannot be zero if called from here;
    return m_ftpLoaderThread->handle();
}


void FTPContentProvider::forHost( const OUString& host,
                                  const OUString& port,
                                  const OUString& username,
                                  OUString& password,
                                  OUString& account)
{
    osl::MutexGuard aGuard(m_aMutex);
    for(const ServerInfo & i : m_ServerInfo)
        if(host == i.host &&
           port == i.port &&
           username == i.username )
        {
            password = i.password;
            account = i.account;
            return;
        }
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

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_ftp_FTPContentProvider_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new FTPContentProvider(context)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
