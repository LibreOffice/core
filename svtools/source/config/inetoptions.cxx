/*************************************************************************
 *
 *  $RCSfile: inetoptions.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: sb $ $Date: 2001-02-12 15:54:52 $
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

#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <utility>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
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
#ifndef _VOS_DIAGNOSE_HXX_
#include <vos/diagnose.hxx>
#endif


namespace unnamed_svt_inetoptions {} using namespace unnamed_svt_inetoptions;
    // unnamed namespaces don't work well yet...

using namespace com::sun::star;

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
//  SvtInetOptions::Impl
//
//============================================================================

class SvtInetOptions::Impl: public vos::OReference, public utl::ConfigItem
{
public:
    enum Index
    {
        INDEX_DNS_IP_ADDRESS,
        INDEX_SMTP_SERVER_NAME,
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

    inline void flush() { Commit(); }

    void
    addPropertiesChangeListener(
        uno::Sequence< rtl::OUString > const & rPropertyNames,
        uno::Reference< beans::XPropertiesChangeListener > const & rListener);

    void
    removePropertiesChangeListener(
        uno::Sequence< rtl::OUString > const & rPropertyNames,
        uno::Reference< beans::XPropertiesChangeListener > const & rListener);

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

    // MSVC has problems with the below Map type when
    // uno::Reference< beans::XPropertiesChangeListener > is not wrapped in
    // class Listener:
    class Listener: public uno::Reference< beans::XPropertiesChangeListener >
    {
    public:
        Listener(uno::Reference< beans::XPropertiesChangeListener > const &
                     rListener):
            uno::Reference< beans::XPropertiesChangeListener >(rListener) {}
    };

    typedef std::map< Listener, std::set< rtl::OUString > > Map;

    vos::OMutex m_aMutex;
    Entry m_aEntries[ENTRY_COUNT];
    Map m_aListeners;

    virtual inline ~Impl() { Commit(); }

    virtual void Notify(uno::Sequence< rtl::OUString > const & rKeys);

    virtual void Commit();

    void notifyListeners(uno::Sequence< rtl::OUString > const & rKeys);
};

//============================================================================
// virtual
void
SvtInetOptions::Impl::Notify(uno::Sequence< rtl::OUString > const & rKeys)
{
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
    notifyListeners(rKeys);
}

//============================================================================
// virtual
void SvtInetOptions::Impl::Commit()
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
void
SvtInetOptions::Impl::notifyListeners(uno::Sequence< rtl::OUString > const &
                                          rKeys)
{
    typedef std::vector< std::pair< uno::Reference<
                                        beans::XPropertiesChangeListener >,
                                    uno::Sequence<
                                        beans::PropertyChangeEvent > > >
        List;
    List aNotifications;
    {
        vos::OGuard aGuard(m_aMutex);
        aNotifications.reserve(m_aListeners.size());
        Map::const_iterator aMapEnd(m_aListeners.end());
        for (Map::const_iterator aIt(m_aListeners.begin()); aIt != aMapEnd;
             ++aIt)
        {
            Map::data_type const & rSet = aIt->second;
            Map::data_type::const_iterator aSetEnd(rSet.end());
            uno::Sequence< beans::PropertyChangeEvent >
                aEvents(rKeys.getLength());
            sal_Int32 nCount = 0;
            for (sal_Int32 i = 0; i < rKeys.getLength(); ++i)
            {
                rtl::OUString
                    aTheKey(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                              "Inet/")));
                aTheKey += rKeys[i];
                if (rSet.find(aTheKey) != aSetEnd)
                {
                    aEvents[nCount].PropertyName = aTheKey;
                    aEvents[nCount].PropertyHandle = -1;
                    ++nCount;
                }
            }
            if (nCount > 0)
            {
                aEvents.realloc(nCount);
                aNotifications.
                    push_back(std::make_pair< List::value_type::first_type,
                                              List::value_type::second_type >(
                                  aIt->first, aEvents));
            }
        }
    }
    for (List::size_type i = 0; i < aNotifications.size(); ++i)
        if (aNotifications[i].first.is())
            aNotifications[i].first->
                propertiesChange(aNotifications[i].second);
}

//============================================================================
SvtInetOptions::Impl::Impl():
    ConfigItem(rtl::OUString::createFromAscii("Inet"))
{
    m_aEntries[INDEX_DNS_IP_ADDRESS].m_aName
        = rtl::OUString::createFromAscii("DNS/IP_Address");
    m_aEntries[INDEX_SMTP_SERVER_NAME].m_aName
        = rtl::OUString::createFromAscii("SMTP/ServerName");
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
                   "SvtInetOptions::Impl::Impl(): Bad EnableNotifications()");
}

//============================================================================
uno::Any SvtInetOptions::Impl::getProperty(Index nIndex)
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
                       "SvtInetOptions::Impl::getProperty():"
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
    VOS_ENSURE(false,
               "SvtInetOptions::Impl::getProperty(): Possible life lock");
    {
        vos::OGuard aGuard(m_aMutex);
        return m_aEntries[nIndex].m_aValue;
    }
}

//============================================================================
void SvtInetOptions::Impl::setProperty(Index nIndex, uno::Any const & rValue,
                                       bool bFlush)
{
    SetModified();
    {
        vos::OGuard aGuard(m_aMutex);
        m_aEntries[nIndex].m_aValue = rValue;
        m_aEntries[nIndex].m_eState = bFlush ? Entry::KNOWN : Entry::MODIFIED;
    }

    //SB: Remove the following hack once #83237# is fixed.
    /* KSO: Interim fix for #83237#. Otherwise changes will be written */
    /*      and notified to listeners listening directly at the config */
    /*      server only at application end.                            */
    bFlush = true;

    uno::Sequence< rtl::OUString > aKeys(1);
    aKeys[0] = m_aEntries[nIndex].m_aName;
    if (bFlush)
    {
        uno::Sequence< uno::Any > aValues(1);
        aValues[0] = rValue;
        PutProperties(aKeys, aValues);
    }
    else
        notifyListeners(aKeys);
}

//============================================================================
void
SvtInetOptions::Impl::addPropertiesChangeListener(
    uno::Sequence< rtl::OUString > const & rPropertyNames,
    uno::Reference< beans::XPropertiesChangeListener > const & rListener)
{
    vos::OGuard aGuard(m_aMutex);
    Map::data_type & rEntry = m_aListeners[rListener];
    for (sal_Int32 i = 0; i < rPropertyNames.getLength(); ++i)
        rEntry.insert(rPropertyNames[i]);
}

//============================================================================
void
SvtInetOptions::Impl::removePropertiesChangeListener(
    uno::Sequence< rtl::OUString > const & rPropertyNames,
    uno::Reference< beans::XPropertiesChangeListener > const & rListener)
{
    vos::OGuard aGuard(m_aMutex);
    Map::iterator aIt(m_aListeners.find(rListener));
    if (aIt != m_aListeners.end())
    {
        for (sal_Int32 i = 0; i < rPropertyNames.getLength(); ++i)
            aIt->second.erase(rPropertyNames[i]);
        if (aIt->second.empty())
            m_aListeners.erase(aIt);
    }
}

//============================================================================
//
//  SvtInetOptions
//
//============================================================================

// static
SvtInetOptions::Impl * SvtInetOptions::m_pImpl = 0;

//============================================================================
SvtInetOptions::SvtInetOptions()
{
    osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());
    if (!m_pImpl)
        m_pImpl = new Impl;
    m_pImpl->acquire();
}

//============================================================================
SvtInetOptions::~SvtInetOptions()
{
    osl::MutexGuard aGuard(osl::Mutex::getGlobalMutex());
    if (m_pImpl->release() == 0)
        m_pImpl = 0;
}

//============================================================================
rtl::OUString SvtInetOptions::GetDnsIpAddress() const
{
    return takeAny< rtl::OUString >(m_pImpl->
                                        getProperty(
                                            Impl::INDEX_DNS_IP_ADDRESS));
}

//============================================================================
rtl::OUString SvtInetOptions::GetSmtpServerName() const
{
    return takeAny< rtl::OUString >(m_pImpl->
                                        getProperty(
                                            Impl::INDEX_SMTP_SERVER_NAME));
}

//============================================================================
rtl::OUString SvtInetOptions::GetProxyNoProxy() const
{
    return takeAny< rtl::OUString >(m_pImpl->
                                        getProperty(
                                            Impl::INDEX_PROXY_NO_PROXY));
}

//============================================================================
sal_Int32 SvtInetOptions::GetProxyType() const
{
    return takeAny< sal_Int32 >(m_pImpl->
                                    getProperty(Impl::INDEX_PROXY_TYPE));
}

//============================================================================
rtl::OUString SvtInetOptions::GetProxyFtpName() const
{
    return takeAny< rtl::OUString >(m_pImpl->
                                        getProperty(
                                            Impl::INDEX_PROXY_FTP_NAME));
}

//============================================================================
sal_Int32 SvtInetOptions::GetProxyFtpPort() const
{
    return takeAny< sal_Int32 >(m_pImpl->
                                    getProperty(Impl::INDEX_PROXY_FTP_PORT));
}

//============================================================================
rtl::OUString SvtInetOptions::GetProxyHttpName() const
{
    return takeAny< rtl::OUString >(m_pImpl->
                                        getProperty(
                                            Impl::INDEX_PROXY_HTTP_NAME));
}

//============================================================================
sal_Int32 SvtInetOptions::GetProxyHttpPort() const
{
    return takeAny< sal_Int32 >(m_pImpl->
                                    getProperty(Impl::INDEX_PROXY_HTTP_PORT));
}

//============================================================================
rtl::OUString SvtInetOptions::GetProxySocksName() const
{
    return takeAny< rtl::OUString >(m_pImpl->
                                        getProperty(
                                            Impl::INDEX_PROXY_SOCKS_NAME));
}

//============================================================================
sal_Int32 SvtInetOptions::GetProxySocksPort() const
{
    return takeAny< sal_Int32 >(m_pImpl->
                                    getProperty(
                                        Impl::INDEX_PROXY_SOCKS_PORT));
}

//============================================================================
void SvtInetOptions::SetDnsIpAddress(rtl::OUString const & rValue,
                                     bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_DNS_IP_ADDRESS,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetSmtpServerName(rtl::OUString const & rValue,
                                       bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_SMTP_SERVER_NAME,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyNoProxy(rtl::OUString const & rValue,
                                     bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_PROXY_NO_PROXY,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyType(ProxyType eValue, bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_PROXY_TYPE,
                         uno::makeAny(sal_Int32(eValue)),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyFtpName(rtl::OUString const & rValue,
                                     bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_PROXY_FTP_NAME,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyFtpPort(sal_Int32 nValue, bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_PROXY_FTP_PORT,
                         uno::makeAny(nValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyHttpName(rtl::OUString const & rValue,
                                      bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_PROXY_HTTP_NAME,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyHttpPort(sal_Int32 nValue, bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_PROXY_HTTP_PORT,
                         uno::makeAny(nValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxySocksName(rtl::OUString const & rValue,
                                       bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_PROXY_SOCKS_NAME,
                         uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxySocksPort(sal_Int32 nValue, bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_PROXY_SOCKS_PORT,
                         uno::makeAny(nValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::flush()
{
    m_pImpl->flush();
}

//============================================================================
void
SvtInetOptions::addPropertiesChangeListener(
    uno::Sequence< rtl::OUString > const & rPropertyNames,
    uno::Reference< beans::XPropertiesChangeListener > const & rListener)
{
    m_pImpl->addPropertiesChangeListener(rPropertyNames, rListener);
}

//============================================================================
void
SvtInetOptions::removePropertiesChangeListener(
    uno::Sequence< rtl::OUString > const & rPropertyNames,
    uno::Reference< beans::XPropertiesChangeListener > const & rListener)
{
    m_pImpl->removePropertiesChangeListener(rPropertyNames, rListener);
}
