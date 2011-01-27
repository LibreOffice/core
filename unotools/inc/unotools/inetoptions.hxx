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

#ifndef _unotools_INETOPTIONS_HXX_
#define _unotools_INETOPTIONS_HXX_

#include "unotools/unotoolsdllapi.h"
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <sal/types.h>
#include <unotools/options.hxx>

namespace com { namespace sun { namespace star { namespace beans {
    class XPropertiesChangeListener;
} } } }
namespace rtl { class OUString; }

//============================================================================
/** The names of all the properties (options) accessible through
    SvtInetOptions.

    @descr  These names are used in the methods
    SvtInetOptions::addPropertiesChangeListener() and
    SvtInetOptions::removePropertiesChangeListener().
 */
#define SVT_INET_OPTION_PROXY_NO_PROXY "Inet/Proxy/NoProxy"
#define SVT_INET_OPTION_PROXY_TYPE "Inet/Proxy/Type"
#define SVT_INET_OPTION_PROXY_FTP_NAME "Inet/Proxy/FTP/Name"
#define SVT_INET_OPTION_PROXY_FTP_PORT "Inet/Proxy/FTP/Port"
#define SVT_INET_OPTION_PROXY_HTTP_NAME "Inet/Proxy/HTTP/Name"
#define SVT_INET_OPTION_PROXY_HTTP_PORT "Inet/Proxy/HTTP/Port"

//============================================================================
/** Interface to access those configuration database entries that are related
    to the various Internet services.

    @descr  The Set...() methods all have a second parameter bFlush.  If false
    is passed (the default), the new value is not written back directly, but
    only cached within this class.  If the value in the configuration database
    is also changed, the cached change will get lost.  If, on the other hand,
    true is passed, the new value is instantly written back to the
    configuration database.
 */
class UNOTOOLS_DLLPUBLIC SvtInetOptions: public utl::detail::Options
{
public:
    SvtInetOptions();

    virtual ~SvtInetOptions();

    enum ProxyType { NONE, AUTOMATIC, MANUAL };

    rtl::OUString GetProxyNoProxy() const;

    sal_Int32 GetProxyType() const;

    rtl::OUString GetProxyFtpName() const;

    sal_Int32 GetProxyFtpPort() const;

    rtl::OUString GetProxyHttpName() const;

    sal_Int32 GetProxyHttpPort() const;

    void SetProxyNoProxy(rtl::OUString const & rValue, bool bFlush = false);

    void SetProxyType(ProxyType eValue, bool bFlush = false);

    void SetProxyFtpName(rtl::OUString const & rValue, bool bFlush = false);

    void SetProxyFtpPort(sal_Int32 nValue, bool bFlush = false);

    void SetProxyHttpName(rtl::OUString const & rValue, bool bFlush = false);

    void SetProxyHttpPort(sal_Int32 nValue, bool bFlush = false);

    void flush();

    /** Add a listener on changes of certain properties (options).

        @param rPropertyNames  The names of the properties (options).  If an
        empty sequence is used, nothing is done.

        @param rListener  A listener.  If the listener is already registered
        on other properties, it continues to also get notifications about
        changes of those properties.  The
        com::sun::star::beans::PropertyChangeEvents supplied to the listener
        will have void OldValue and NewValue slots.
     */
    void
    addPropertiesChangeListener(
        com::sun::star::uno::Sequence< rtl::OUString > const & rPropertyNames,
        com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertiesChangeListener > const &
            rListener);

    /** Remove a listener on changes of certain properties (options).

        @param rPropertyNames  The names of the properties (options).  If an
        empty sequence is used, nothing is done.

        @param rListener  A listener.  If the listener is still registered on
        other properties, it continues to get notifications about changes of
        those properties.
     */
    void
    removePropertiesChangeListener(
        com::sun::star::uno::Sequence< rtl::OUString > const & rPropertyNames,
        com::sun::star::uno::Reference<
                com::sun::star::beans::XPropertiesChangeListener > const &
            rListener);

private:
    class Impl;

    static Impl * m_pImpl;
};

#endif // _unotools_INETOPTIONS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
