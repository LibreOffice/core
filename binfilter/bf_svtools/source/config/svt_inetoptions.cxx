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


#ifndef _SVTOOLS_INETOPTIONS_HXX_
#include <bf_svtools/inetoptions.hxx>
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
    T aValue = T();
    rAny >>= aValue;
    return aValue;
}

}

namespace binfilter
{

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

    inline void flush() { Commit(); }

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
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtInetOptions_Impl::ctor()");
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

}
