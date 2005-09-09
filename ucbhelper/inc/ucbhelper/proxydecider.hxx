/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: proxydecider.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:30:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _UCBHELPER_PROXYDECIDER_HXX
#define _UCBHELPER_PROXYDECIDER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hXX>
#endif
#ifndef INCLUDED_UCBHELPERDLLAPI_H
#include "ucbhelper/ucbhelperdllapi.h"
#endif

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
