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

#ifndef INCLUDED_UCB_SOURCE_UCP_FTP_FTPCONTENTPROVIDER_HXX
#define INCLUDED_UCB_SOURCE_UCP_FTP_FTPCONTENTPROVIDER_HXX

#include <vector>
#include <osl/mutex.hxx>
#include <ucbhelper/proxydecider.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include "ftpurl.hxx"

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.

#define FTP_CONTENT_PROVIDER_SERVICE_NAME "com.sun.star.ucb.FTPContentProvider"
#define FTP_CONTENT_TYPE "application/ftp-content"

/**
 *  Definition of ftpcontentprovider
 */
namespace ftp
{
    class FTPLoaderThread;

    class FTPContentProvider:
        public ::ucbhelper::ContentProviderImplHelper
    {
    public:

        explicit FTPContentProvider( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

        virtual ~FTPContentProvider();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType )
            throw( css::uno::RuntimeException,
                   std::exception ) override;
        virtual void SAL_CALL acquire()
            throw() override;
        virtual void SAL_CALL release()
            throw() override;

        // XTypeProvider
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId()
            throw( css::uno::RuntimeException,
                   std::exception ) override;
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
            throw( css::uno::RuntimeException,
                   std::exception ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName()
            throw( css::uno::RuntimeException,
                   std::exception ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
            throw( css::uno::RuntimeException,
                   std::exception ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
            throw( css::uno::RuntimeException,
                   std::exception ) override;

        static OUString getImplementationName_Static();
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

        static css::uno::Reference< css::lang::XSingleServiceFactory >
        createServiceFactory( const css::uno::Reference<
                              css::lang::XMultiServiceFactory >& rxServiceMgr );

        // XContentProvider
        virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
        queryContent( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier )
            throw( css::ucb::IllegalIdentifierException,
                   css::uno::RuntimeException,
                   std::exception ) override;

        CURL* handle();

        /** host is in the form host:port.
         */

        bool forHost(const OUString& host,
                             const OUString& port,
                             const OUString& username,
                             OUString& password,
                             OUString& account);

        bool setHost(const OUString& host,
                             const OUString& port,
                             const OUString& username,
                             const OUString& password,
                             const OUString& account);

        struct ServerInfo
        {
            OUString host;
            OUString port;
            OUString username;
            OUString password;
            OUString account;
        };

    private:

        osl::Mutex m_aMutex;
        FTPLoaderThread *m_ftpLoaderThread;
        ucbhelper::InternetProxyDecider *m_pProxyDecider;
        std::vector<ServerInfo> m_ServerInfo;

        void init();

        css::uno::Reference<css::ucb::XContentProvider> getHttpProvider()
            throw(css::uno::RuntimeException);

    };  // end class FTPContentProvider

}       // end namespace ftp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
