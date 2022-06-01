/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "cacheupdatelistener.hxx"
#include "configflush.hxx"

#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <unotools/configpaths.hxx>
#include <rtl/ustring.hxx>
#include <comphelper/processfactory.hxx>
#include <utility>


namespace filter::config{

CacheUpdateListener::CacheUpdateListener(FilterCache &rFilterCache,
                                         css::uno::Reference< css::uno::XInterface > xConfigAccess,
                                         FilterCache::EItemType eConfigType)
    : m_rCache(rFilterCache)
    , m_xConfig(std::move(xConfigAccess))
    , m_eConfigType(eConfigType)
{
}

CacheUpdateListener::~CacheUpdateListener()
{
}

void CacheUpdateListener::startListening()
{
    // SAFE ->
    std::unique_lock aLock(m_aMutex);
    css::uno::Reference< css::util::XChangesNotifier > xNotifier(m_xConfig, css::uno::UNO_QUERY);
    aLock.unlock();
    // <- SAFE

    if (!xNotifier.is())
        return;

    css::uno::Reference< css::util::XChangesListener > xThis(this);
    xNotifier->addChangesListener(xThis);
}


void CacheUpdateListener::stopListening()
{
    // SAFE ->
    std::unique_lock aLock(m_aMutex);
    css::uno::Reference< css::util::XChangesNotifier > xNotifier(m_xConfig, css::uno::UNO_QUERY);
    aLock.unlock();
    // <- SAFE

    if (!xNotifier.is())
        return;

    css::uno::Reference< css::util::XChangesListener > xThis(this);
    xNotifier->removeChangesListener(xThis);
}


void SAL_CALL  CacheUpdateListener::changesOccurred(const css::util::ChangesEvent& aEvent)
{
    // SAFE ->
    std::unique_lock aLock(m_aMutex);

    // disposed ?
    if ( ! m_xConfig.is())
        return;

    FilterCache::EItemType                             eType = m_eConfigType;

    aLock.unlock();
    // <- SAFE

    std::vector<OUString> lChangedItems;
    sal_Int32    c = aEvent.Changes.getLength();
    sal_Int32    i = 0;

    for (i=0; i<c; ++i)
    {
        const css::util::ElementChange& aChange = aEvent.Changes[i];

        OUString sOrgPath ;
        OUString sTempPath;

        OUString sProperty;
        OUString sNode    ;
        OUString sLocale  ;

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
            sProperty.clear();
            sLocale.clear();
        }
        else
        {
            sOrgPath = sTempPath;
            if ( ! ::utl::splitLastFromConfigurationPath(sOrgPath, sTempPath, sNode))
            {
                sNode     = sProperty;
                sProperty = sLocale;
                sLocale.clear();
            }
        }

        if ( sNode.isEmpty() )
            continue;

        auto pIt = ::std::find(lChangedItems.cbegin(), lChangedItems.cend(), sNode);
        if (pIt == lChangedItems.cend())
            lChangedItems.push_back(sNode);
    }

    bool                     bNotifyRefresh = false;
    for (auto const& changedItem : lChangedItems)
    {
        try
        {
            m_rCache.refreshItem(eType, changedItem);
        }
        catch(const css::container::NoSuchElementException&)
            {
                // can be ignored! Because we must be aware that
                // sItem was removed from the configuration and we forced an update of the cache.
                // But we know, that the cache is up-to-date know and has thrown this exception afterwards .-)
            }
        // NO FLUSH! Otherwise we start a never ending story here .-)
        bNotifyRefresh = true;
    }

    // notify sfx cache about the changed filter cache .-)
    if (bNotifyRefresh)
    {
        rtl::Reference< ConfigFlush > xRefreshBroadcaster = new ConfigFlush();
        xRefreshBroadcaster->refresh();
    }
}


void SAL_CALL CacheUpdateListener::disposing(const css::lang::EventObject& aEvent)
{
    // SAFE ->
    std::unique_lock aLock(m_aMutex);
    if (aEvent.Source == m_xConfig)
        m_xConfig.clear();
    // <- SAFE
}

} // namespace filter::config

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
