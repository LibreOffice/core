/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inetoptions.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 08:49:43 $
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
#include <inetoptions.hxx>
#endif

#ifndef INCLUDED_RTL_INSTANCE_HXX
#include "rtl/instance.hxx"
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _WILDCARD_HXX
#include <tools/wldcrd.hxx>
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
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SYSTEM_XPROXYSETTINGS_HPP_
#include <com/sun/star/system/XProxySettings.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _RTL_USTRING_H_
#include <rtl/ustring.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _SALHELPER_REFOBJ_HXX_
#include <salhelper/refobj.hxx>
#endif
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
        OSL_ENSURE(false,
                   "SvtInetOptions::Impl::Impl(): Bad EnableNotifications()");
}

//============================================================================
star::uno::Any SvtInetOptions::Impl::getProperty(Index nIndex)
{
    for (int nTryCount = 0; nTryCount < 10; ++nTryCount)
    {
        {
            osl::MutexGuard aGuard(m_aMutex);
            if (m_aEntries[nIndex].m_eState != Entry::UNKNOWN)
                return m_aEntries[nIndex].m_aValue;
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
    OSL_ENSURE(false,
               "SvtInetOptions::Impl::getProperty(): Possible life lock");
    {
        osl::MutexGuard aGuard(m_aMutex);
        return m_aEntries[nIndex].m_aValue;
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
        RTL_LOGFILE_CONTEXT(aLog, "svtools (???) ::SvtInetOptions_Impl::ctor()");
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
