/*************************************************************************
 *
 *  $RCSfile: inetoptions.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2000-11-03 17:47:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVTOOLS_INETOPTIONS_HXX_
#include <inetoptions.hxx>
#endif

#if defined SOLARIS // must include <string> before <algorithm>...
#include <string>
#endif // SOLARIS

#include <algorithm>

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif

namespace unnamed_svt_inetoptions {} using namespace unnamed_svt_inetoptions;
    // unnamed namespaces don't work well yet...

using namespace com::sun::star;
using namespace svt;

//============================================================================
//
//  takeAny
//
//============================================================================

namespace unnamed_svt_inetoptions {

template< typename T > inline T takeAny(uno::Any const & rAny)
{
    T aValue;
    rAny >>= aValue;
    return aValue;
}

}

//============================================================================
//
//  InetOptions::Impl
//
//============================================================================

class InetOptions::Impl: public vos::OReference, public utl::ConfigItem
{
public:
    enum Index
    {
        INDEX_DNS_IP_ADDRESS,
        INDEX_SMTP_SERVER_NAME,
        INDEX_PROTOCOL_REVEAL_MAIL_ADDRESS,
        INDEX_PROTOCOL_USER_AGENT,
        INDEX_PROXY_NO_PROXY,
        INDEX_PROXY_TYPE,
        INDEX_PROXY_FTP_NAME,
        INDEX_PROXY_FTP_PORT,
        INDEX_PROXY_HTTP_NAME,
        INDEX_PROXY_HTTP_PORT,
        INDEX_PROXY_SOCKS_NAME,
        INDEX_PROXY_SOCKS_PORT
    };

    Impl();

    uno::Any getProperty(Index nIndex);

    void setProperty(Index nIndex, uno::Any const & rValue, bool bFlush);

private:
    enum { ENTRY_COUNT = INDEX_PROXY_SOCKS_PORT + 1 };

    struct Entry
    {
        enum State { UNKNOWN, KNOWN, MODIFIED };

        inline Entry(): m_eState(UNKNOWN) {}

        rtl::OUString m_aName;
        uno::Any m_aValue;
        State m_eState;
    };

    Entry m_aEntries[ENTRY_COUNT];
    vos::OMutex m_aMutex;

    virtual inline ~Impl() { Commit(); }

    virtual void Notify(uno::Sequence< rtl::OUString > const & rKeys);

    virtual void Commit();
};

//============================================================================
// virtual
void InetOptions::Impl::Notify(uno::Sequence< rtl::OUString > const & rKeys)
{
    vos::OGuard aGuard(m_aMutex);
    for (sal_Int32 i = 0; i < rKeys.getLength(); ++i)
        for (sal_Int32 j = 0; j < ENTRY_COUNT; ++j)
            if (rKeys[i] == m_aEntries[j].m_aName)
            {
                m_aEntries[j].m_eState = Entry::UNKNOWN;
                break;
            }
}

//============================================================================
// virtual
void InetOptions::Impl::Commit()
{
    uno::Sequence< rtl::OUString > aKeys(ENTRY_COUNT);
    uno::Sequence< uno::Any > aValues(ENTRY_COUNT);
    sal_Int32 nCount = 0;
    {
        vos::OGuard aGuard(m_aMutex);
        for (sal_Int32 i = 0; i < ENTRY_COUNT; ++i)
            if (m_aEntries[i].m_eState == Entry::MODIFIED)
            {
                aKeys[nCount] = m_aEntries[i].m_aName;
                aValues[nCount] = m_aEntries[i].m_aValue;
                ++nCount;
                m_aEntries[i].m_eState = Entry::KNOWN;
            }
    }
    if (nCount > 0)
    {
        aKeys.realloc(nCount);
        aValues.realloc(nCount);
        PutProperties(aKeys, aValues);
    }
}

//============================================================================
InetOptions::Impl::Impl():
    ConfigItem(rtl::OUString::createFromAscii("Inet"))
{
    m_aEntries[INDEX_DNS_IP_ADDRESS].m_aName
        = rtl::OUString::createFromAscii("DNS/IP_Address");
    m_aEntries[INDEX_SMTP_SERVER_NAME].m_aName
        = rtl::OUString::createFromAscii("SMTP/ServerName");
    m_aEntries[INDEX_PROTOCOL_REVEAL_MAIL_ADDRESS].m_aName
        = rtl::OUString::createFromAscii("Protocol/RevealMailAddress");
    m_aEntries[INDEX_PROTOCOL_USER_AGENT].m_aName
        = rtl::OUString::createFromAscii("Protocol/UserAgent");
    m_aEntries[INDEX_PROXY_NO_PROXY].m_aName
        = rtl::OUString::createFromAscii("Proxy/NoProxy");
    m_aEntries[INDEX_PROXY_TYPE].m_aName
        = rtl::OUString::createFromAscii("Proxy/Type");
    m_aEntries[INDEX_PROXY_FTP_NAME].m_aName
        = rtl::OUString::createFromAscii("Proxy/FTP/Name");
    m_aEntries[INDEX_PROXY_FTP_PORT].m_aName
        = rtl::OUString::createFromAscii("Proxy/FTP/Port");
    m_aEntries[INDEX_PROXY_HTTP_NAME].m_aName
        = rtl::OUString::createFromAscii("Proxy/HTTP/Name");
    m_aEntries[INDEX_PROXY_HTTP_PORT].m_aName
        = rtl::OUString::createFromAscii("Proxy/HTTP/Port");
    m_aEntries[INDEX_PROXY_SOCKS_NAME].m_aName
        = rtl::OUString::createFromAscii("Proxy/SOCKS/Name");
    m_aEntries[INDEX_PROXY_SOCKS_PORT].m_aName
        = rtl::OUString::createFromAscii("Proxy/SOCKS/Port");
    uno::Sequence< rtl::OUString > aKeys(ENTRY_COUNT);
    for (sal_Int32 i = 0; i < ENTRY_COUNT; ++i)
        aKeys[i] = m_aEntries[i].m_aName;
    if (!EnableNotification(aKeys))
        VOS_ENSURE(false,
                   "InetOptions::Impl::Impl(): Bad EnableNotifications()");
}

//============================================================================
uno::Any InetOptions::Impl::getProperty(Index nIndex)
{
    for (int nTryCount = 0; nTryCount < 10; ++nTryCount)
    {
        {
            vos::OGuard aGuard(m_aMutex);
            if (m_aEntries[nIndex].m_eState != Entry::UNKNOWN)
                return m_aEntries[nIndex].m_aValue;
        }
        uno::Sequence< rtl::OUString > aKeys(ENTRY_COUNT);
        int nIndices[ENTRY_COUNT];
        sal_Int32 nCount = 0;
        {
            vos::OGuard aGuard(m_aMutex);
            for (int i = 0; i < ENTRY_COUNT; ++i)
                if (m_aEntries[i].m_eState == Entry::UNKNOWN)
                {
                    aKeys[nCount] = m_aEntries[i].m_aName;
                    nIndices[nCount] = i;
                    ++nCount;
                }
        }
        if (nCount > 0)
        {
            aKeys.realloc(nCount);
            uno::Sequence< uno::Any > aValues(GetProperties(aKeys));
            VOS_ENSURE(aValues.getLength() == nCount,
                       "InetOptions::Impl::getProperty():"
                           " Bad GetProperties() result");
            nCount = std::min(nCount, aValues.getLength());
            {
                vos::OGuard aGuard(m_aMutex);
                for (sal_Int32 i = 0; i < nCount; ++i)
                {
                    int nIndex = nIndices[i];
                    if (m_aEntries[nIndex].m_eState == Entry::UNKNOWN)
                    {
                        m_aEntries[nIndices[i]].m_aValue = aValues[i];
                        m_aEntries[nIndices[i]].m_eState = Entry::KNOWN;
                    }
                }
            }
        }
    }
    VOS_ENSURE(false, "InetOptions::Impl::getProperty(): Possible life lock");
    {
        vos::OGuard aGuard(m_aMutex);
        return m_aEntries[nIndex].m_aValue;
    }
}

//============================================================================
void InetOptions::Impl::setProperty(Index nIndex, uno::Any const & rValue,
                                    bool bFlush)
{
    SetModified();
    {
        vos::OGuard aGuard(m_aMutex);
        m_aEntries[nIndex].m_aValue = rValue;
        m_aEntries[nIndex].m_eState = bFlush ? Entry::KNOWN : Entry::MODIFIED;
    }
    if (bFlush)
    {
        uno::Sequence< rtl::OUString > aKeys(1);
        aKeys[0] = m_aEntries[nIndex].m_aName;
        uno::Sequence< uno::Any > aValues(1);
        aValues[0] = rValue;
        PutProperties(aKeys, aValues);
    }
}

//============================================================================
//
//  InetOptions
//
//============================================================================

InetOptions::InetOptions()
{
    static Impl * pSingleImpl = 0;
    if (!pSingleImpl)
    {
        static vos::OMutex aMutex;
        vos::OGuard aGuard(aMutex);
        if (!pSingleImpl)
            pSingleImpl = new Impl;
    }
    m_xImpl = pSingleImpl;
}

//============================================================================
InetOptions::~InetOptions()
{}

//============================================================================
rtl::OUString InetOptions::getDnsIpAddress() const
{
    return takeAny< rtl::OUString >(m_xImpl->
                                        getProperty(
                                            Impl::INDEX_DNS_IP_ADDRESS));
}

//============================================================================
rtl::OUString InetOptions::getSmtpServerName() const
{
    return takeAny< rtl::OUString >(m_xImpl->
                                        getProperty(
                                            Impl::INDEX_SMTP_SERVER_NAME));
}

//============================================================================
bool InetOptions::getProtocolRevealMailAddress() const
{
    return
        takeAny< sal_Bool >(m_xImpl->
                                getProperty(
                                    Impl::INDEX_PROTOCOL_REVEAL_MAIL_ADDRESS))
            != false;
}

//============================================================================
rtl::OUString InetOptions::getProtocolUserAgent() const
{
    return takeAny< rtl::OUString >(m_xImpl->
                                        getProperty(
                                            Impl::INDEX_PROTOCOL_USER_AGENT));
}

//============================================================================
rtl::OUString InetOptions::getProxyNoProxy() const
{
    return takeAny< rtl::OUString >(m_xImpl->
                                        getProperty(
                                            Impl::INDEX_PROXY_NO_PROXY));
}

//============================================================================
sal_Int32 InetOptions::getProxyType() const
{
    return takeAny< sal_Int32 >(m_xImpl->
                                    getProperty(Impl::INDEX_PROXY_TYPE));
}

//============================================================================
rtl::OUString InetOptions::getProxyFtpName() const
{
    return takeAny< rtl::OUString >(m_xImpl->
                                        getProperty(
                                            Impl::INDEX_PROXY_FTP_NAME));
}

//============================================================================
sal_Int32 InetOptions::getProxyFtpPort() const
{
    return takeAny< sal_Int32 >(m_xImpl->
                                    getProperty(Impl::INDEX_PROXY_FTP_PORT));
}

//============================================================================
rtl::OUString InetOptions::getProxyHttpName() const
{
    return takeAny< rtl::OUString >(m_xImpl->
                                        getProperty(
                                            Impl::INDEX_PROXY_HTTP_NAME));
}

//============================================================================
sal_Int32 InetOptions::getProxyHttpPort() const
{
    return takeAny< sal_Int32 >(m_xImpl->
                                    getProperty(Impl::INDEX_PROXY_HTTP_PORT));
}

//============================================================================
rtl::OUString InetOptions::getProxySocksName() const
{
    return takeAny< rtl::OUString >(m_xImpl->
                                        getProperty(
                                            Impl::INDEX_PROXY_SOCKS_NAME));
}

//============================================================================
sal_Int32 InetOptions::getProxySocksPort() const
{
    return takeAny< sal_Int32 >(m_xImpl->
                                    getProperty(
                                        Impl::INDEX_PROXY_SOCKS_PORT));
}

//============================================================================
void InetOptions::setDnsIpAddress(rtl::OUString const & rValue, bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_DNS_IP_ADDRESS,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void InetOptions::setProtocolRevealMailAddress(bool bValue, bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_PROTOCOL_REVEAL_MAIL_ADDRESS,
                         uno::makeAny(sal_Bool(bValue)),
                         bFlush);
}

//============================================================================
void InetOptions::setProtocolUserAgent(rtl::OUString const & rValue,
                                       bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_PROTOCOL_USER_AGENT,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void InetOptions::setProxyNoProxy(rtl::OUString const & rValue, bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_PROXY_NO_PROXY,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void InetOptions::setProxyType(ProxyType eValue, bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_PROXY_TYPE,
                         uno::makeAny(sal_Int32(eValue)),
                         bFlush);
}

//============================================================================
void InetOptions::setProxyFtpName(rtl::OUString const & rValue, bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_PROXY_FTP_NAME,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void InetOptions::setProxyFtpPort(sal_Int32 nValue, bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_PROXY_FTP_PORT,
                         uno::makeAny(nValue),
                         bFlush);
}

//============================================================================
void InetOptions::setProxyHttpName(rtl::OUString const & rValue, bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_PROXY_HTTP_NAME,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void InetOptions::setProxyHttpPort(sal_Int32 nValue, bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_PROXY_HTTP_PORT,
                         uno::makeAny(nValue),
                         bFlush);
}

//============================================================================
void InetOptions::setProxySocksName(rtl::OUString const & rValue, bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_PROXY_SOCKS_NAME,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void InetOptions::setProxySocksPort(sal_Int32 nValue, bool bFlush)
{
    m_xImpl->setProperty(Impl::INDEX_PROXY_SOCKS_PORT,
                         uno::makeAny(nValue),
                         bFlush);
}
