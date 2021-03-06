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

#pragma once

#include <vector>
#include <ucbhelper/proxydecider.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include "curl.hxx"

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

        virtual ~FTPContentProvider() override;

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
        virtual void SAL_CALL acquire()
            throw() override;
        virtual void SAL_CALL release()
            throw() override;

        // XTypeProvider
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() override;
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        // XContentProvider
        virtual css::uno::Reference< css::ucb::XContent > SAL_CALL
        queryContent( const css::uno::Reference< css::ucb::XContentIdentifier >& Identifier ) override;

        CURL* handle();

        /** host is in the form host:port.
         */

        void forHost(std::u16string_view host,
                             std::u16string_view port,
                             std::u16string_view username,
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
        std::unique_ptr<FTPLoaderThread> m_ftpLoaderThread;
        std::unique_ptr<ucbhelper::InternetProxyDecider> m_pProxyDecider;
        std::vector<ServerInfo> m_ServerInfo;

        void init();
    };  // end class FTPContentProvider

}       // end namespace ftp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
