/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inetoptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:23:24 $
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

#ifndef _SVTOOLS_INETOPTIONS_HXX_
#define _SVTOOLS_INETOPTIONS_HXX_

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <svtools/options.hxx>
#endif

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
class SVL_DLLPUBLIC SvtInetOptions: public svt::detail::Options
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

#endif // _SVTOOLS_INETOPTIONS_HXX_
