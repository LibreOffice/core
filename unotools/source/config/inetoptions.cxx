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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"
#include <unotools/inetoptions.hxx>
#include "rtl/instance.hxx"
#include <tools/urlobj.hxx>
#include <tools/wldcrd.hxx>

#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <utility>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/XPropertiesChangeListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/system/XProxySettings.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <osl/mutex.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <unotools/configitem.hxx>
#include <unotools/processfactory.hxx>
#include <osl/diagnose.h>
#include <salhelper/refobj.hxx>
#include <rtl/logfile.hxx>
#include "itemholder1.hxx"

using namespace com::sun;

//============================================================================
//
//  takeAny
//
//============================================================================

namespace {

template< typename T > inline T takeAny(star::uno::Any const & rAny)
{
    T aValue = T();
    rAny >>= aValue;
    return aValue;
}

}

//============================================================================
//
//  SvtInetOptions::Impl
//
//============================================================================

class SvtInetOptions::Impl: public salhelper::ReferenceObject,
                            public utl::ConfigItem
{
public:
    enum Index
    {
        INDEX_NO_PROXY,
        INDEX_PROXY_TYPE,
        INDEX_FTP_PROXY_NAME,
        INDEX_FTP_PROXY_PORT,
        INDEX_HTTP_PROXY_NAME,
        INDEX_HTTP_PROXY_PORT
    };

    Impl();

    star::uno::Any getProperty(Index nIndex);

    void
    setProperty(Index nIndex, star::uno::Any const & rValue, bool bFlush);

    inline void flush() { Commit(); }

    void
    addPropertiesChangeListener(
        star::uno::Sequence< rtl::OUString > const & rPropertyNames,
        star::uno::Reference< star::beans::XPropertiesChangeListener > const &
            rListener);

    void
    removePropertiesChangeListener(
        star::uno::Sequence< rtl::OUString > const & rPropertyNames,
        star::uno::Reference< star::beans::XPropertiesChangeListener > const &
            rListener);

private:
    enum { ENTRY_COUNT = INDEX_HTTP_PROXY_PORT + 1 };

    struct Entry
    {
        enum State { UNKNOWN, KNOWN, MODIFIED };

        inline Entry(): m_eState(UNKNOWN) {}

        rtl::OUString m_aName;
        star::uno::Any m_aValue;
        State m_eState;
    };

    // MSVC has problems with the below Map type when
    // star::uno::Reference< star::beans::XPropertiesChangeListener > is not
    // wrapped in class Listener:
    class Listener:
        public star::uno::Reference< star::beans::XPropertiesChangeListener >
    {
    public:
        Listener(star::uno::Reference<
                         star::beans::XPropertiesChangeListener > const &
                     rListener):
            star::uno::Reference< star::beans::XPropertiesChangeListener >(
                rListener)
        {}
    };

    typedef std::map< Listener, std::set< rtl::OUString > > Map;

    osl::Mutex m_aMutex;
    Entry m_aEntries[ENTRY_COUNT];
    Map m_aListeners;

    virtual inline ~Impl() { Commit(); }

    virtual void Notify(star::uno::Sequence< rtl::OUString > const & rKeys);

    virtual void Commit();

    void notifyListeners(star::uno::Sequence< rtl::OUString > const & rKeys);
};

//============================================================================
// virtual
void
SvtInetOptions::Impl::Notify(star::uno::Sequence< rtl::OUString > const &
                                 rKeys)
{
    {
        osl::MutexGuard aGuard(m_aMutex);
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
    star::uno::Sequence< rtl::OUString > aKeys(ENTRY_COUNT);
    star::uno::Sequence< star::uno::Any > aValues(ENTRY_COUNT);
    sal_Int32 nCount = 0;
    {
        osl::MutexGuard aGuard(m_aMutex);
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
SvtInetOptions::Impl::notifyListeners(
    star::uno::Sequence< rtl::OUString > const & rKeys)
{
    typedef
        std::vector< std::pair< star::uno::Reference<
                                    star::beans::XPropertiesChangeListener >,
                                star::uno::Sequence<
                                    star::beans::PropertyChangeEvent > > >
    List;
    List aNotifications;
    {
        osl::MutexGuard aGuard(m_aMutex);
        aNotifications.reserve(m_aListeners.size());
        Map::const_iterator aMapEnd(m_aListeners.end());
        for (Map::const_iterator aIt(m_aListeners.begin()); aIt != aMapEnd;
             ++aIt)
        {
            const Map::mapped_type &rSet = aIt->second;
            Map::mapped_type::const_iterator aSetEnd(rSet.end());
            star::uno::Sequence< star::beans::PropertyChangeEvent >
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
                    push_back(std::pair< List::value_type::first_type,
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
    ConfigItem(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Inet/Settings")))
{
    m_aEntries[INDEX_NO_PROXY].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetNoProxy"));
    m_aEntries[INDEX_PROXY_TYPE].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetProxyType"));
    m_aEntries[INDEX_FTP_PROXY_NAME].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetFTPProxyName"));
    m_aEntries[INDEX_FTP_PROXY_PORT].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetFTPProxyPort"));
    m_aEntries[INDEX_HTTP_PROXY_NAME].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetHTTPProxyName"));
    m_aEntries[INDEX_HTTP_PROXY_PORT].m_aName
        = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ooInetHTTPProxyPort"));

    star::uno::Sequence< rtl::OUString > aKeys(ENTRY_COUNT);
    for (sal_Int32 i = 0; i < ENTRY_COUNT; ++i)
        aKeys[i] = m_aEntries[i].m_aName;
    if (!EnableNotification(aKeys))
        OSL_FAIL("SvtInetOptions::Impl::Impl(): Bad EnableNotifications()");
}

//============================================================================
star::uno::Any SvtInetOptions::Impl::getProperty(Index nPropIndex)
{
    for (int nTryCount = 0; nTryCount < 10; ++nTryCount)
    {
        {
            osl::MutexGuard aGuard(m_aMutex);
            if (m_aEntries[nPropIndex].m_eState != Entry::UNKNOWN)
                return m_aEntries[nPropIndex].m_aValue;
        }
        star::uno::Sequence< rtl::OUString > aKeys(ENTRY_COUNT);
        int nIndices[ENTRY_COUNT];
        sal_Int32 nCount = 0;
        {
            osl::MutexGuard aGuard(m_aMutex);
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
            star::uno::Sequence< star::uno::Any >
                aValues(GetProperties(aKeys));
            OSL_ENSURE(aValues.getLength() == nCount,
                       "SvtInetOptions::Impl::getProperty():"
                           " Bad GetProperties() result");
            nCount = std::min(nCount, aValues.getLength());
            {
                osl::MutexGuard aGuard(m_aMutex);
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
    OSL_FAIL("SvtInetOptions::Impl::getProperty(): Possible life lock");
    {
        osl::MutexGuard aGuard(m_aMutex);
        return m_aEntries[nPropIndex].m_aValue;
    }
}

//============================================================================
void SvtInetOptions::Impl::setProperty(Index nIndex,
                                       star::uno::Any const & rValue,
                                       bool bFlush)
{
    SetModified();
    {
        osl::MutexGuard aGuard(m_aMutex);
        m_aEntries[nIndex].m_aValue = rValue;
        m_aEntries[nIndex].m_eState = bFlush ? Entry::KNOWN : Entry::MODIFIED;
    }

    star::uno::Sequence< rtl::OUString > aKeys(1);
    aKeys[0] = m_aEntries[nIndex].m_aName;
    if (bFlush)
    {
        star::uno::Sequence< star::uno::Any > aValues(1);
        aValues[0] = rValue;
        PutProperties(aKeys, aValues);
    }
    else
        notifyListeners(aKeys);
}

//============================================================================
void
SvtInetOptions::Impl::addPropertiesChangeListener(
    star::uno::Sequence< rtl::OUString > const & rPropertyNames,
    star::uno::Reference< star::beans::XPropertiesChangeListener > const &
        rListener)
{
    osl::MutexGuard aGuard(m_aMutex);
    Map::mapped_type & rEntry = m_aListeners[rListener];
    for (sal_Int32 i = 0; i < rPropertyNames.getLength(); ++i)
        rEntry.insert(rPropertyNames[i]);
}

//============================================================================
void
SvtInetOptions::Impl::removePropertiesChangeListener(
    star::uno::Sequence< rtl::OUString > const & rPropertyNames,
    star::uno::Reference< star::beans::XPropertiesChangeListener > const &
        rListener)
{
    osl::MutexGuard aGuard(m_aMutex);
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

namespace
{
    class LocalSingleton : public rtl::Static< osl::Mutex, LocalSingleton >
    {
    };
}

// static
SvtInetOptions::Impl * SvtInetOptions::m_pImpl = 0;

//============================================================================
SvtInetOptions::SvtInetOptions()
{
    osl::MutexGuard aGuard(LocalSingleton::get());
    if (!m_pImpl)
    {
        RTL_LOGFILE_CONTEXT(aLog, "unotools ( ??? ) ::SvtInetOptions_Impl::ctor()");
        m_pImpl = new Impl;

        ItemHolder1::holdConfigItem(E_INETOPTIONS);
    }
    m_pImpl->acquire();
}

//============================================================================
SvtInetOptions::~SvtInetOptions()
{
    osl::MutexGuard aGuard(LocalSingleton::get());
    if (m_pImpl->release() == 0)
        m_pImpl = 0;
}

//============================================================================
rtl::OUString SvtInetOptions::GetProxyNoProxy() const
{
    return takeAny< rtl::OUString >(m_pImpl->
                                        getProperty(Impl::INDEX_NO_PROXY));
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
                                            Impl::INDEX_FTP_PROXY_NAME));
}

//============================================================================
sal_Int32 SvtInetOptions::GetProxyFtpPort() const
{
    return takeAny< sal_Int32 >(m_pImpl->
                                    getProperty(Impl::INDEX_FTP_PROXY_PORT));
}

//============================================================================
rtl::OUString SvtInetOptions::GetProxyHttpName() const
{
    return takeAny< rtl::OUString >(m_pImpl->
                                        getProperty(
                                            Impl::INDEX_HTTP_PROXY_NAME));
}

//============================================================================
sal_Int32 SvtInetOptions::GetProxyHttpPort() const
{
    return takeAny< sal_Int32 >(m_pImpl->
                                    getProperty(Impl::INDEX_HTTP_PROXY_PORT));
}

//============================================================================
void SvtInetOptions::SetProxyNoProxy(rtl::OUString const & rValue,
                                     bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_NO_PROXY,
                         star::uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyType(ProxyType eValue, bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_PROXY_TYPE,
                         star::uno::makeAny(sal_Int32(eValue)),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyFtpName(rtl::OUString const & rValue,
                                     bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_FTP_PROXY_NAME,
                         star::uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyFtpPort(sal_Int32 nValue, bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_FTP_PROXY_PORT,
                         star::uno::makeAny(nValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyHttpName(rtl::OUString const & rValue,
                                      bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_HTTP_PROXY_NAME,
                         star::uno::makeAny(rValue),
                         bFlush);
}

//============================================================================
void SvtInetOptions::SetProxyHttpPort(sal_Int32 nValue, bool bFlush)
{
    m_pImpl->setProperty(Impl::INDEX_HTTP_PROXY_PORT,
                         star::uno::makeAny(nValue),
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
    star::uno::Sequence< rtl::OUString > const & rPropertyNames,
    star::uno::Reference< star::beans::XPropertiesChangeListener > const &
        rListener)
{
    m_pImpl->addPropertiesChangeListener(rPropertyNames, rListener);
}

//============================================================================
void
SvtInetOptions::removePropertiesChangeListener(
    star::uno::Sequence< rtl::OUString > const & rPropertyNames,
    star::uno::Reference< star::beans::XPropertiesChangeListener > const &
        rListener)
{
    m_pImpl->removePropertiesChangeListener(rPropertyNames, rListener);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
