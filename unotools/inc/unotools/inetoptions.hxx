/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
