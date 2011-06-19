/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FTP_FTPCONTENTPROVIDER_HXX_
#define _FTP_FTPCONTENTPROVIDER_HXX_

#include <vector>
#include <osl/mutex.hxx>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/proxydecider.hxx>
#include <ucbhelper/providerhelper.hxx>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include "ftphandleprovider.hxx"
#include "ftpurl.hxx"

// UNO service name for the provider. This name will be used by the UCB to
// create instances of the provider.

#define FTP_CONTENT_PROVIDER_SERVICE_NAME "com.sun.star.ucb.FTPContentProvider"
#define FTP_CONTENT_TYPE "application/ftp-content"


/**
 *  Definition of ftpcontentprovider
 */



namespace ftp {


    class FTPLoaderThread;


    class FTPContentProvider:
        public ::ucbhelper::ContentProviderImplHelper,
        public FTPHandleProvider
    {
    public:

        FTPContentProvider(
            const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory>& xMSF );

        ~FTPContentProvider();

        // XInterface
        XINTERFACE_DECL()

        // XTypeProvider
        XTYPEPROVIDER_DECL()

        // XServiceInfo
        XSERVICEINFO_DECL()

        // XContentProvider
        virtual com::sun::star::uno::Reference< com::sun::star::ucb::XContent > SAL_CALL
        queryContent(
            const com::sun::star::uno::Reference< com::sun::star::ucb::XContentIdentifier >& Identifier )
            throw( com::sun::star::ucb::IllegalIdentifierException,
                   com::sun::star::uno::RuntimeException );

        // FTPHandleProvider.

        virtual CURL* handle();

        virtual bool forHost(const rtl::OUString& host,
                             const rtl::OUString& port,
                             const rtl::OUString& username,
                             rtl::OUString& password,
                             rtl::OUString& account);

        virtual bool setHost(const rtl::OUString& host,
                             const rtl::OUString& port,
                             const rtl::OUString& username,
                             const rtl::OUString& password,
                             const rtl::OUString& account);


        struct ServerInfo {
            rtl::OUString host;
            rtl::OUString port;
            rtl::OUString username;
            rtl::OUString password;
            rtl::OUString account;
        };

    private:

        osl::Mutex m_aMutex;
        FTPLoaderThread *m_ftpLoaderThread;
        ucbhelper::InternetProxyDecider *m_pProxyDecider;
        std::vector<ServerInfo> m_ServerInfo;

        void init();

        com::sun::star::uno::Reference<com::sun::star::ucb::XContentProvider>
        getHttpProvider()
            throw(com::sun::star::uno::RuntimeException);

    };  // end class FTPContentProvider

}       // end namespace ftp

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
