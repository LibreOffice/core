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


#include "cacheupdatelistener.hxx"
#include "constant.hxx"

#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <salhelper/singletonref.hxx>
#include <unotools/configpaths.hxx>
#include <rtl/ustring.hxx>

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions



CacheUpdateListener::CacheUpdateListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR,
                                         FilterCache &rFilterCache,
                                         const css::uno::Reference< css::uno::XInterface >& xConfigAccess,
                                         FilterCache::EItemType eConfigType)
    : BaseLock()
    , m_xSMGR(xSMGR)
    , m_rCache(rFilterCache)
    , m_xConfig(xConfigAccess)
    , m_eConfigType(eConfigType)
{
}

CacheUpdateListener::~CacheUpdateListener()
{
}

void CacheUpdateListener::startListening()
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    css::uno::Reference< css::util::XChangesNotifier > xNotifier(m_xConfig, css::uno::UNO_QUERY);
    aLock.clear();
    // <- SAFE

    if (!xNotifier.is())
        return;

    css::uno::Reference< css::util::XChangesListener > xThis(static_cast< css::util::XChangesListener* >(this), css::uno::UNO_QUERY_THROW);
    xNotifier->addChangesListener(xThis);
}



void CacheUpdateListener::stopListening()
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    css::uno::Reference< css::util::XChangesNotifier > xNotifier(m_xConfig, css::uno::UNO_QUERY);
    aLock.clear();
    // <- SAFE

    if (!xNotifier.is())
        return;

    css::uno::Reference< css::util::XChangesListener > xThis(static_cast< css::util::XChangesListener* >(this), css::uno::UNO_QUERY);
    xNotifier->removeChangesListener(xThis);
}



void SAL_CALL  CacheUpdateListener::changesOccurred(const css::util::ChangesEvent& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // disposed ?
    if ( ! m_xConfig.is())
        return;

    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    FilterCache::EItemType                                 eType = m_eConfigType;

    aLock.clear();
    // <- SAFE

    OUStringList lChangedItems;
    sal_Int32    c = aEvent.Changes.getLength();
    sal_Int32    i = 0;

    for (i=0; i<c; ++i)
    {
        const css::util::ElementChange& aChange = aEvent.Changes[i];

        ::rtl::OUString sOrgPath ;
        ::rtl::OUString sTempPath;

        ::rtl::OUString sProperty;
        ::rtl::OUString sNode    ;
        ::rtl::OUString sLocale  ;

        /*  at least we must be able to retrieve 2 path elements
            But sometimes the original path can contain 3 of them ... in case
            a localized value was changed.
            =>
            1) Filters/Filter["filtername"]/Property
            2) Filters/Filter["filtername"]/LocalizedProperty/Locale
        */

        aChange.Accessor >>= sOrgPath;
        if ( ! ::utl::splitLastFromConfigurationPath(sOrgPath, sTempPath, sLocale))
            continue;
        sOrgPath = sTempPath;
        if ( ! ::utl::splitLastFromConfigurationPath(sOrgPath, sTempPath, sProperty))
        {
            sNode     = sLocale;
            sProperty = ::rtl::OUString();
            sLocale   = ::rtl::OUString();
        }
        else
        {
            sOrgPath = sTempPath;
            if ( ! ::utl::splitLastFromConfigurationPath(sOrgPath, sTempPath, sNode))
            {
                sNode     = sProperty;
                sProperty = sLocale;
                sLocale   = ::rtl::OUString();
            }
        }

        if ( sNode.isEmpty() )
            continue;

        OUStringList::const_iterator pIt = ::std::find(lChangedItems.begin(), lChangedItems.end(), sNode);
        if (pIt == lChangedItems.end())
            lChangedItems.push_back(sNode);
    }

    sal_Bool                     bNotifyRefresh = sal_False;
    OUStringList::const_iterator pIt;
    for (  pIt  = lChangedItems.begin();
           pIt != lChangedItems.end()  ;
         ++pIt                         )
    {
        const ::rtl::OUString& sItem = *pIt;
        try
        {
            m_rCache.refreshItem(eType, sItem);
        }
        catch(const css::container::NoSuchElementException&)
            {
                // can be ignored! Because we must be aware that
                // sItem was removed from the configuration and we forced an update of the cache.
                // But we know, that the cache is up-to-date know and has thrown this exception afterwards .-)
            }
        // NO FLUSH! Otherwise we start a never ending story here .-)
        bNotifyRefresh = sal_True;
    }

    // notify sfx cache about the changed filter cache .-)
    if (bNotifyRefresh)
    {
        css::uno::Reference< css::util::XRefreshable > xRefreshBroadcaster(
            xSMGR->createInstance(SERVICE_FILTERCONFIGREFRESH),
            css::uno::UNO_QUERY);
        if (xRefreshBroadcaster.is())
            xRefreshBroadcaster->refresh();
    }
}



void SAL_CALL CacheUpdateListener::disposing(const css::lang::EventObject& aEvent)
    throw(css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (aEvent.Source == m_xConfig)
        m_xConfig.clear();
    aLock.clear();
    // <- SAFE
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
