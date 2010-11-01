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

#ifndef _UCBHELPER_PROXYDECIDER_HXX
#define _UCBHELPER_PROXYDECIDER_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include "ucbhelper/ucbhelperdllapi.h"

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace ucbhelper
{

/**
 * This struct describes a proxy server.
 */
struct InternetProxyServer
{
    /**
      * The name of the proxy server.
      */
    ::rtl::OUString aName;

    /**
      * The port of the proxy server.
      */
    sal_Int32 nPort;

    /**
      * Constructor.
      */
    InternetProxyServer() : nPort( -1 ) {}
};

namespace proxydecider_impl { class InternetProxyDecider_Impl; }

/**
 * This class is able to decide whether and which internet proxy server is to
 * be used to access a given URI.
 *
 * The implementation reads the internet proxy settings from Office
 * configuration. It listens for configuration changes and adapts itself
 * accordingly. Because configuration data can change during runtime clients
 * should not cache results obtained from InternetProxyDecider instances. One
 * instance should be kept to be queried multiple times instead.
 */
class UCBHELPER_DLLPUBLIC InternetProxyDecider
{
public:
    /**
      * Constructor.
      *
      * Note: Every instance should be held alive as long as possible because
      *       because construction is quite expensive.
      *
      * @param rxSMgr is a Service Manager.
      */
    InternetProxyDecider( const ::com::sun::star::uno::Reference<
                    ::com::sun::star::lang::XMultiServiceFactory >& rxSMgr );

    /**
      * Destructor.
      */
    ~InternetProxyDecider();

    /**
      * Informs whether a proxy server should be used.
      *
      * @param rProtocol contains the internet protocol to be used to
      *         access the server (i.e. "ftp", "http"). The protocol string
      *         is handled case-insensitive and must not be empty.
      * @param rHost contains the name of the server that should be accessed.
      *         This parameter might be left empty. In this case the
      *         implementation will return whether a proxy is configured
      *         for the given protocol.
      * @param nPort contains the port of the server that should be accessed.
      *         If host is not empty this parameter must always contain a valid
      *         port number, for instance the default port for the requested
      *         protocol(i.e. 80 or http).
      * @return true if a proxy server should be used, false otherwise.
      */
    bool
    shouldUseProxy( const rtl::OUString & rProtocol,
                    const rtl::OUString & rHost,
                    sal_Int32 nPort ) const;

    /**
      * Returns the proxy server to be used.
      *
      * @param rProtocol contains the internet protocol to be used to
      *         access the server (i.e. "ftp", "http"). The protocol string
      *         is handled case-insensitive and must not be empty.
      * @param rHost contains the name of the server that should be accessed.
      *         This parameter might be left empty. In this case the
      *         implementation will return the proxy that is configured
      *         for the given protocol.
      * @param nPort contains the port of the server that should be accessed.
      *         If host is not empty this parameter must always contain a valid
      *         port number, for instance the default port for the requested
      *         protocol(i.e. 80 or http).
      * @return a InternetProxyServer reference. If member aName of the
      *         InternetProxyServer is empty no proxy server is to be used.
      */
    const InternetProxyServer &
    getProxy( const rtl::OUString & rProtocol,
              const rtl::OUString & rHost,
              sal_Int32 nPort ) const;

private:
    proxydecider_impl::InternetProxyDecider_Impl * m_pImpl;
};

} // namespace ucbhelper

#endif /* !_UCBHELPER_PROXYDECIDER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
