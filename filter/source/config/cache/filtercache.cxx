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


#include "filtercache.hxx"
#include "macros.hxx"
#include "constant.hxx"
#include "cacheupdatelistener.hxx"

/*TODO see using below ... */
#define AS_ENABLE_FILTER_UINAMES
#define WORKAROUND_EXCEPTION_PROBLEM

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/CorruptedFilterConfigurationException.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>

#include <unotools/configpaths.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/uri.hxx>
#include <tools/urlobj.hxx>
#include <tools/wldcrd.hxx>
#include <i18nlangtag/languagetag.hxx>

#include <officecfg/Setup.hxx>


namespace filter{
    namespace config{

FilterCache::FilterCache()
    : BaseLock    (                                        )
    , m_eFillState(E_CONTAINS_NOTHING                      )
{
    int i = 0;
    OUString sStandardProps[9];

    sStandardProps[i++] = PROPNAME_USERDATA;
    sStandardProps[i++] = PROPNAME_TEMPLATENAME;
    // E_READ_UPDATE only above
    sStandardProps[i++] = PROPNAME_TYPE;
    sStandardProps[i++] = PROPNAME_FILEFORMATVERSION;
    sStandardProps[i++] = PROPNAME_UICOMPONENT;
    sStandardProps[i++] = PROPNAME_FILTERSERVICE;
    sStandardProps[i++] = PROPNAME_DOCUMENTSERVICE;
    sStandardProps[i++] = PROPNAME_EXPORTEXTENSION;
    sStandardProps[i++] = PROPNAME_FLAGS; // must be last.
    assert(i == SAL_N_ELEMENTS(sStandardProps));

    // E_READ_NOTHING -> creative nothingness.
    m_aStandardProps[E_READ_STANDARD] =
        css::uno::Sequence< OUString >(sStandardProps + 2, 7);
    m_aStandardProps[E_READ_UPDATE] =
        css::uno::Sequence< OUString >(sStandardProps, 2);
    m_aStandardProps[E_READ_ALL] =
        css::uno::Sequence< OUString >(sStandardProps,
                                       SAL_N_ELEMENTS(sStandardProps));

    i = 0;
    OUString sTypeProps[7];
    sTypeProps[i++] = PROPNAME_MEDIATYPE;
    // E_READ_UPDATE only above
    sTypeProps[i++] = PROPNAME_PREFERREDFILTER;
    sTypeProps[i++] = PROPNAME_DETECTSERVICE;
    sTypeProps[i++] = PROPNAME_URLPATTERN;
    sTypeProps[i++] = PROPNAME_EXTENSIONS;
    sTypeProps[i++] = PROPNAME_PREFERRED;
    sTypeProps[i++] = PROPNAME_CLIPBOARDFORMAT;
    assert(i == SAL_N_ELEMENTS(sTypeProps));

    // E_READ_NOTHING -> more creative nothingness.
    m_aTypeProps[E_READ_STANDARD] =
        css::uno::Sequence< OUString >(sTypeProps + 1, 6);
    m_aTypeProps[E_READ_UPDATE] =
        css::uno::Sequence< OUString >(sTypeProps, 1);
    m_aTypeProps[E_READ_ALL] =
        css::uno::Sequence< OUString >(sTypeProps,
                                       SAL_N_ELEMENTS(sTypeProps));
}


FilterCache::~FilterCache()
{
    if (m_xTypesChglisteners.is())
        m_xTypesChglisteners->stopListening();
    if (m_xFiltersChgListener.is())
        m_xFiltersChgListener->stopListening();
}


FilterCache* FilterCache::clone() const
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    FilterCache* pClone = new FilterCache();

    // Don't copy the configuration access points here.
    // They will be created on demand inside the cloned instance,
    // if they are needed.

    pClone->m_lTypes                     = m_lTypes;
    pClone->m_lFilters                   = m_lFilters;
    pClone->m_lFrameLoaders              = m_lFrameLoaders;
    pClone->m_lContentHandlers           = m_lContentHandlers;
    pClone->m_lExtensions2Types          = m_lExtensions2Types;
    pClone->m_lURLPattern2Types          = m_lURLPattern2Types;

    pClone->m_sActLocale                 = m_sActLocale;

    pClone->m_eFillState                 = m_eFillState;

    pClone->m_lChangedTypes              = m_lChangedTypes;
    pClone->m_lChangedFilters            = m_lChangedFilters;
    pClone->m_lChangedFrameLoaders       = m_lChangedFrameLoaders;
    pClone->m_lChangedContentHandlers    = m_lChangedContentHandlers;

    return pClone;
    // <- SAFE ----------------------------------
}


void FilterCache::takeOver(const FilterCache& rClone)
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // a)
    // Don't copy the configuration access points here!
    // We must use our own ones...

    // b)
    // Further we can ignore the uno service manager.
    // We should already have a valid instance.

    // c)
    // Take over only changed items!
    // Otherwise we risk the following scenario:
    // c1) clone_1 contains changed filters
    // c2) clone_2 container changed types
    // c3) clone_1 take over changed filters and unchanged types
    // c4) clone_2 take over unchanged filters(!) and changed types(!)
    // c5) c4 overwrites c3!

    if (!rClone.m_lChangedTypes.empty())
        m_lTypes = rClone.m_lTypes;
    if (!rClone.m_lChangedFilters.empty())
        m_lFilters = rClone.m_lFilters;
    if (!rClone.m_lChangedFrameLoaders.empty())
        m_lFrameLoaders = rClone.m_lFrameLoaders;
    if (!rClone.m_lChangedContentHandlers.empty())
        m_lContentHandlers = rClone.m_lContentHandlers;

    m_lChangedTypes.clear();
    m_lChangedFilters.clear();
    m_lChangedFrameLoaders.clear();
    m_lChangedContentHandlers.clear();

    m_sActLocale     = rClone.m_sActLocale;

    m_eFillState     = rClone.m_eFillState;

    // renew all dependencies and optimizations
    // Because we can't be sure, that changed filters on one clone
    // and changed types of another clone work together.
    // But here we can check against the later changes...
    impl_validateAndOptimize();
    // <- SAFE ----------------------------------
}


void FilterCache::load(EFillState eRequired)
    throw(css::uno::Exception)
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // check if required fill state is already reached ...
    // There is nothing to do then.
    if ((m_eFillState & eRequired) == eRequired)
        return;

    // Otherwise load the missing items.


    // a) load some const values from configuration.
    //    These values are needed there for loading
    //    config items ...
    //    Further we load some std items from the
    //    configuration so we can try to load the first
    //    office document with a minimal set of values.
    if (m_eFillState == E_CONTAINS_NOTHING)
    {
        impl_getDirectCFGValue(CFGDIRECTKEY_OFFICELOCALE) >>= m_sActLocale;
        if (m_sActLocale.isEmpty())
        {
            _FILTER_CONFIG_LOG_1_("FilterCache::ctor() ... could not specify office locale => use default \"%s\"\n", _FILTER_CONFIG_TO_ASCII_(DEFAULT_OFFICELOCALE));
            m_sActLocale = DEFAULT_OFFICELOCALE;
        }

        // Support the old configuration support. Read it only one times during office runtime!
        impl_readOldFormat();
    }


    // b) If the required fill state was not reached
    //    but std values was already loaded ...
    //    we must load some further missing items.
    impl_load(eRequired);
    // <- SAFE
}


bool FilterCache::isFillState(FilterCache::EFillState eState) const
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return ((m_eFillState & eState) == eState);
    // <- SAFE
}


OUStringList FilterCache::getMatchingItemsByProps(      EItemType  eType  ,
                                                  const CacheItem& lIProps,
                                                  const CacheItem& lEProps) const
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // search for right list
    // An exception is thrown - "eType" is unknown.
    // => rList will be valid everytimes next line is reached.
    const CacheItemList& rList = impl_getItemList(eType);

    OUStringList lKeys;

    // search items, which provides all needed properties of set "lIProps"
    // but not of set "lEProps"!
    for (CacheItemList::const_iterator pIt  = rList.begin();
                                       pIt != rList.end()  ;
                                     ++pIt                 )
    {
        _FILTER_CONFIG_LOG_1_("getMatchingProps for \"%s\"  ...\n",
                              _FILTER_CONFIG_TO_ASCII_(pIt->first))
        if (
            (pIt->second.haveProps(lIProps)    ) &&
            (pIt->second.dontHaveProps(lEProps))
           )
        {
            lKeys.push_back(pIt->first);
        }
    }

    return lKeys;
    // <- SAFE
}


bool FilterCache::hasItems(EItemType eType) const
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // search for right list
    // An exception is thrown - "eType" is unknown.
    // => rList will be valid everytimes next line is reached.
    const CacheItemList& rList = impl_getItemList(eType);

    return !rList.empty();
    // <- SAFE
}


OUStringList FilterCache::getItemNames(EItemType eType) const
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // search for right list
    // An exception is thrown - "eType" is unknown.
    // => rList will be valid everytimes next line is reached.
    const CacheItemList& rList = impl_getItemList(eType);

    OUStringList lKeys;
    for (CacheItemList::const_iterator pIt  = rList.begin();
                                       pIt != rList.end()  ;
                                     ++pIt                 )
    {
        lKeys.push_back(pIt->first);
    }
    return lKeys;
    // <- SAFE
}


bool FilterCache::hasItem(      EItemType        eType,
                              const OUString& sItem)
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // search for right list
    // An exception is thrown - "eType" is unknown.
    // => rList will be valid everytimes next line is reached.
    const CacheItemList& rList = impl_getItemList(eType);

    // if item could not be found - check if it can be loaded
    // from the underlying configuration layer. Might it was not already
    // loaded into this FilterCache object before.
    CacheItemList::const_iterator pIt = rList.find(sItem);
    if (pIt != rList.end())
        return true;

    try
    {
        impl_loadItemOnDemand(eType, sItem);
        // no exception => item could be loaded!
        return true;
    }
    catch(const css::container::NoSuchElementException&)
    {}

    return false;
    // <- SAFE
}


CacheItem FilterCache::getItem(      EItemType        eType,
                               const OUString& sItem)
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // search for right list
    // An exception is thrown if "eType" is unknown.
    // => rList will be valid everytimes next line is reached.
    CacheItemList& rList = impl_getItemList(eType);

    // check if item exists ...
    CacheItemList::iterator pIt = rList.find(sItem);
    if (pIt == rList.end())
    {
        // ... or load it on demand from the
        // underlying configuration layer.
        // Note: NoSuchElementException is thrown automatically here if
        // item could not be loaded!
        pIt = impl_loadItemOnDemand(eType, sItem);
    }

    /* Workaround for #137955#
       Draw types and filters are installed ... but draw was disabled during setup.
       We must suppress accessing these filters. Otherwise the office can crash.
       Solution for the next major release: do not install those filters !
     */
    if (eType == E_FILTER)
    {
        CacheItem& rFilter = pIt->second;
        OUString sDocService;
        rFilter[PROPNAME_DOCUMENTSERVICE] >>= sDocService;

        // In Standalone-Impress the module WriterWeb is not installed
        // but it is there to load help pages
        bool bIsHelpFilter = sItem == "writer_web_HTML_help";

        if ( !bIsHelpFilter && !impl_isModuleInstalled(sDocService) )
        {
            OUString sMsg("The requested filter '" + sItem +
                          "' exists ... but it should not; because the corresponding LibreOffice module was not installed.");
            throw css::container::NoSuchElementException(sMsg, css::uno::Reference< css::uno::XInterface >());
        }
    }

    return pIt->second;
    // <- SAFE
}


void FilterCache::removeItem(      EItemType        eType,
                             const OUString& sItem)
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // search for right list
    // An exception is thrown - "eType" is unknown.
    // => rList will be valid everytimes next line is reached.
    CacheItemList& rList = impl_getItemList(eType);

    CacheItemList::iterator pItem = rList.find(sItem);
    if (pItem == rList.end())
        pItem = impl_loadItemOnDemand(eType, sItem); // throws NoSuchELementException!
    rList.erase(pItem);

    impl_addItem2FlushList(eType, sItem);
}


void FilterCache::setItem(      EItemType        eType ,
                          const OUString& sItem ,
                          const CacheItem&       aValue)
    throw(css::uno::Exception, std::exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // search for right list
    // An exception is thrown - "eType" is unknown.
    // => rList will be valid everytimes next line is reached.
    CacheItemList& rList = impl_getItemList(eType);

    // name must be part of the property set too ... otherwise our
    // container query can't work correctly
    CacheItem aItem = aValue;
    aItem[PROPNAME_NAME] <<= sItem;
    aItem.validateUINames(m_sActLocale);

    // remove implicit properties as e.g. FINALIZED or MANDATORY
    // They can't be saved here and must be readed on demand later, if they are needed.
    removeStatePropsFromItem(aItem);

    rList[sItem] = aItem;

    impl_addItem2FlushList(eType, sItem);
}


void FilterCache::refreshItem(      EItemType        eType,
                              const OUString& sItem)
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    impl_loadItemOnDemand(eType, sItem);
}


void FilterCache::addStatePropsToItem(      EItemType        eType,
                                      const OUString& sItem,
                                            CacheItem&       rItem)
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // Note: Opening of the configuration layer throws some exceptions
    // if it failed. So we mustn't check any reference here...
    css::uno::Reference< css::container::XNameAccess > xPackage;
    css::uno::Reference< css::container::XNameAccess > xSet;
    switch(eType)
    {
        case E_TYPE :
            {
                xPackage.set(impl_openConfig(E_PROVIDER_TYPES), css::uno::UNO_QUERY_THROW);
                xPackage->getByName(CFGSET_TYPES) >>= xSet;
            }
            break;

        case E_FILTER :
            {
                xPackage.set(impl_openConfig(E_PROVIDER_FILTERS), css::uno::UNO_QUERY_THROW);
                xPackage->getByName(CFGSET_FILTERS) >>= xSet;
            }
            break;

        case E_FRAMELOADER :
            {
                /* TODO
                    Hack -->
                        The default frame loader can't be located inside the normal set of frame loaders.
                        Its an atomic property inside the misc cfg package. So we can't retrieve the information
                        about FINALIZED and MANDATORY very easy ... :-(
                        => set it to readonly/required everytimes :-)
                */
                css::uno::Any   aDirectValue       = impl_getDirectCFGValue(CFGDIRECTKEY_DEFAULTFRAMELOADER);
                OUString sDefaultFrameLoader;
                if (
                    (aDirectValue >>= sDefaultFrameLoader) &&
                    (!sDefaultFrameLoader.isEmpty()      ) &&
                    (sItem.equals(sDefaultFrameLoader)   )
                   )
                {
                    rItem[PROPNAME_FINALIZED] <<= sal_True;
                    rItem[PROPNAME_MANDATORY] <<= sal_True;
                    return;
                }
                /* <-- HACK */

                xPackage.set(impl_openConfig(E_PROVIDER_OTHERS), css::uno::UNO_QUERY_THROW);
                xPackage->getByName(CFGSET_FRAMELOADERS) >>= xSet;
            }
            break;

        case E_CONTENTHANDLER :
            {
                xPackage.set(impl_openConfig(E_PROVIDER_OTHERS), css::uno::UNO_QUERY_THROW);
                xPackage->getByName(CFGSET_CONTENTHANDLERS) >>= xSet;
            }
            break;
        default: break;
    }

    try
    {
        css::uno::Reference< css::beans::XProperty > xItem;
        xSet->getByName(sItem) >>= xItem;
        css::beans::Property aDescription = xItem->getAsProperty();

        bool bFinalized = ((aDescription.Attributes & css::beans::PropertyAttribute::READONLY  ) == css::beans::PropertyAttribute::READONLY  );
        bool bMandatory = ((aDescription.Attributes & css::beans::PropertyAttribute::REMOVABLE) != css::beans::PropertyAttribute::REMOVABLE);

        rItem[PROPNAME_FINALIZED] <<= bFinalized;
        rItem[PROPNAME_MANDATORY] <<= bMandatory;
    }
    catch(const css::container::NoSuchElementException&)
    {
        /*  Ignore exceptions for missing elements inside configuration.
            May by the following reason exists:
                -   The item does not exists inside the new configuration package org.openoffice.TypeDetection - but
                    we got it from the old package org.openoffice.Office/TypeDetection. We don't migrate such items
                    automatically to the new format. Because it will disturb e.g. the deinstallation of an external filter
                    package. Because such external filter can remove the old file - but not the automatically created new one ...

            => mark item as FINALIZED / MANDATORY, we don't support writing to the old format
        */
        rItem[PROPNAME_FINALIZED] <<= sal_True;
        rItem[PROPNAME_MANDATORY] <<= sal_True;
    }

    // <- SAFE
}


void FilterCache::removeStatePropsFromItem(CacheItem& rItem)
    throw(css::uno::Exception)
{
    CacheItem::iterator pIt;
    pIt = rItem.find(PROPNAME_FINALIZED);
    if (pIt != rItem.end())
        rItem.erase(pIt);
    pIt = rItem.find(PROPNAME_MANDATORY);
    if (pIt != rItem.end())
        rItem.erase(pIt);
}


void FilterCache::flush()
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // renew all dependencies and optimizations
    impl_validateAndOptimize();

    if (m_lChangedTypes.size() > 0)
    {
        css::uno::Reference< css::container::XNameAccess > xConfig(impl_openConfig(E_PROVIDER_TYPES), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameAccess > xSet   ;

        xConfig->getByName(CFGSET_TYPES) >>= xSet;
        impl_flushByList(xSet, E_TYPE, m_lTypes, m_lChangedTypes);

        css::uno::Reference< css::util::XChangesBatch > xFlush(xConfig, css::uno::UNO_QUERY);
        xFlush->commitChanges();
    }

    if (m_lChangedFilters.size() > 0)
    {
        css::uno::Reference< css::container::XNameAccess > xConfig(impl_openConfig(E_PROVIDER_FILTERS), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameAccess > xSet   ;

        xConfig->getByName(CFGSET_FILTERS) >>= xSet;
        impl_flushByList(xSet, E_FILTER, m_lFilters, m_lChangedFilters);

        css::uno::Reference< css::util::XChangesBatch > xFlush(xConfig, css::uno::UNO_QUERY);
        xFlush->commitChanges();
    }

    /*TODO FrameLoader/ContentHandler must be flushed here too ... */
}


void FilterCache::impl_flushByList(const css::uno::Reference< css::container::XNameAccess >& xSet  ,
                                         EItemType                                           eType ,
                                   const CacheItemList&                                      rCache,
                                   const OUStringList&                                       lItems)
    throw(css::uno::Exception)
{
    css::uno::Reference< css::container::XNameContainer >   xAddRemoveSet(xSet, css::uno::UNO_QUERY);
    css::uno::Reference< css::container::XNameReplace >     xReplaceeSet(xSet, css::uno::UNO_QUERY);
    css::uno::Reference< css::lang::XSingleServiceFactory > xFactory(xSet, css::uno::UNO_QUERY);

    for (OUStringList::const_iterator pIt  = lItems.begin();
                                      pIt != lItems.end()  ;
                                    ++pIt                  )
    {
        const OUString& sItem  = *pIt;
              EItemFlushState  eState = impl_specifyFlushOperation(xSet, rCache, sItem);
        switch(eState)
        {
            case E_ITEM_REMOVED :
            {
                xAddRemoveSet->removeByName(sItem);
            }
            break;

            case E_ITEM_ADDED :
            {
                css::uno::Reference< css::container::XNameReplace > xItem (xFactory->createInstance(), css::uno::UNO_QUERY);

                // special case. no exception - but not a valid item => set must be finalized or mandatory!
                // Reject flush operation by throwing an exception. At least one item couldnt be flushed.
                if (!xItem.is())
                    throw css::uno::Exception("Can not add item. Set is finalized or mandatory!",
                                              css::uno::Reference< css::uno::XInterface >());

                CacheItemList::const_iterator pItem = rCache.find(sItem);
                impl_saveItem(xItem, eType, pItem->second);
                xAddRemoveSet->insertByName(sItem, css::uno::makeAny(xItem));
            }
            break;

            case E_ITEM_CHANGED :
            {
                css::uno::Reference< css::container::XNameReplace > xItem;
                xSet->getByName(sItem) >>= xItem;

                // special case. no exception - but not a valid item => it must be finalized or mandatory!
                // Reject flush operation by throwing an exception. At least one item couldnt be flushed.
                if (!xItem.is())
                    throw css::uno::Exception("Can not change item. Its finalized or mandatory!",
                                              css::uno::Reference< css::uno::XInterface >());

                CacheItemList::const_iterator pItem = rCache.find(sItem);
                impl_saveItem(xItem, eType, pItem->second);
            }
            break;
            default: break;
        }
    }
}


void FilterCache::detectFlatForURL(const css::util::URL& aURL      ,
                                         FlatDetection&  rFlatTypes) const
    throw(css::uno::Exception)
{
    // extract extension from URL, so it can be used directly as key into our hash map!
    // Note further: It must be converted to lower case, because the optimize hash
    // (which maps extensions to types) work with lower case key strings!
    INetURLObject   aParser    (aURL.Main);
    OUString sExtension = aParser.getExtension(INetURLObject::LAST_SEGMENT       ,
                                                      true                          ,
                                                      INetURLObject::DECODE_WITH_CHARSET);
    sExtension = sExtension.toAsciiLowerCase();

    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);


    // i) Step over all well known URL pattern
    //    and add registered types to the return list too
    //    Do it as first one - because: if a type match by a
    //    pattern a following deep detection can be suppressed!
    //    Further we can stop after first match ...
    for (CacheItemRegistration::const_iterator pPattReg  = m_lURLPattern2Types.begin();
                                               pPattReg != m_lURLPattern2Types.end()  ;
                                             ++pPattReg                               )
    {
        WildCard aPatternCheck(pPattReg->first);
        if (aPatternCheck.Matches(aURL.Main))
        {
            const OUStringList& rTypesForPattern = pPattReg->second;

            FlatDetectionInfo aInfo;
            aInfo.sType = *(rTypesForPattern.begin());
            aInfo.bMatchByPattern = true;

            rFlatTypes.push_back(aInfo);
//          return;
        }
    }


    // ii) search types matching to the given extension.
    //     Copy every matching type without changing its order!
    //     Because preferred types was added as first one during
    //     loading configuration.
    CacheItemRegistration::const_iterator pExtReg = m_lExtensions2Types.find(sExtension);
    if (pExtReg != m_lExtensions2Types.end())
    {
        const OUStringList& rTypesForExtension = pExtReg->second;
        for (OUStringList::const_iterator pIt  = rTypesForExtension.begin();
                                          pIt != rTypesForExtension.end()  ;
                                        ++pIt                              )
        {
            FlatDetectionInfo aInfo;
            aInfo.sType             = *pIt;
            aInfo.bMatchByExtension = true;

            rFlatTypes.push_back(aInfo);
        }
    }

    aLock.clear();
    // <- SAFE ----------------------------------
}

const CacheItemList& FilterCache::impl_getItemList(EItemType eType) const
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    switch(eType)
    {
        case E_TYPE           : return m_lTypes          ;
        case E_FILTER         : return m_lFilters        ;
        case E_FRAMELOADER    : return m_lFrameLoaders   ;
        case E_CONTENTHANDLER : return m_lContentHandlers;

    }

    throw css::uno::RuntimeException("unknown sub container requested.",
                                            css::uno::Reference< css::uno::XInterface >());
    // <- SAFE ----------------------------------
}

CacheItemList& FilterCache::impl_getItemList(EItemType eType)
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    switch(eType)
    {
        case E_TYPE           : return m_lTypes          ;
        case E_FILTER         : return m_lFilters        ;
        case E_FRAMELOADER    : return m_lFrameLoaders   ;
        case E_CONTENTHANDLER : return m_lContentHandlers;

    }

    throw css::uno::RuntimeException("unknown sub container requested.",
                                            css::uno::Reference< css::uno::XInterface >());
    // <- SAFE ----------------------------------
}

css::uno::Reference< css::uno::XInterface > FilterCache::impl_openConfig(EConfigProvider eProvider)
    throw(css::uno::Exception)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    OUString                              sPath      ;
    css::uno::Reference< css::uno::XInterface >* pConfig = nullptr;
    css::uno::Reference< css::uno::XInterface >  xOld       ;
    OString                               sRtlLog    ;

    switch(eProvider)
    {
        case E_PROVIDER_TYPES :
        {
            if (m_xConfigTypes.is())
                return m_xConfigTypes;
            sPath           = CFGPACKAGE_TD_TYPES;
            pConfig         = &m_xConfigTypes;
            sRtlLog         = "impl_openconfig(E_PROVIDER_TYPES)";
        }
        break;

        case E_PROVIDER_FILTERS :
        {
            if (m_xConfigFilters.is())
                return m_xConfigFilters;
            sPath           = CFGPACKAGE_TD_FILTERS;
            pConfig         = &m_xConfigFilters;
            sRtlLog         = "impl_openconfig(E_PROVIDER_FILTERS)";
        }
        break;

        case E_PROVIDER_OTHERS :
        {
            if (m_xConfigOthers.is())
                return m_xConfigOthers;
            sPath   = CFGPACKAGE_TD_OTHERS;
            pConfig = &m_xConfigOthers;
            sRtlLog = "impl_openconfig(E_PROVIDER_OTHERS)";
        }
        break;

        case E_PROVIDER_OLD :
        {
            // This special provider is used to work with
            // the old configuration format only. Its not cached!
            sPath   = CFGPACKAGE_TD_OLD;
            pConfig = &xOld;
            sRtlLog = "impl_openconfig(E_PROVIDER_OLD)";
        }
        break;

        default : throw css::uno::RuntimeException("These configuration node is not supported here for open!", nullptr);
    }

    {
        SAL_INFO( "filter.config", "" << sRtlLog.getStr());
        *pConfig = impl_createConfigAccess(sPath    ,
                                           false,   // bReadOnly
                                           true );  // bLocalesMode
    }


    // Start listening for changes on that configuration access.
    switch(eProvider)
    {
        case E_PROVIDER_TYPES:
        {
            m_xTypesChglisteners.set(new CacheUpdateListener(*this, *pConfig, FilterCache::E_TYPE));
            m_xTypesChglisteners->startListening();
        }
        break;
        case E_PROVIDER_FILTERS:
        {
            m_xFiltersChgListener.set(new CacheUpdateListener(*this, *pConfig, FilterCache::E_FILTER));
            m_xFiltersChgListener->startListening();
        }
        break;
        default:
        break;
    }

    return *pConfig;
}

css::uno::Any FilterCache::impl_getDirectCFGValue(const OUString& sDirectKey)
{
    OUString sRoot;
    OUString sKey ;

    if (
        (!::utl::splitLastFromConfigurationPath(sDirectKey, sRoot, sKey)) ||
        (sRoot.isEmpty()                                             ) ||
        (sKey.isEmpty()                                              )
       )
        return css::uno::Any();

    css::uno::Reference< css::uno::XInterface > xCfg = impl_createConfigAccess(sRoot    ,
                                                                               true ,  // bReadOnly
                                                                               false); // bLocalesMode
    if (!xCfg.is())
        return css::uno::Any();

    css::uno::Reference< css::container::XNameAccess > xAccess(xCfg, css::uno::UNO_QUERY);
    if (!xAccess.is())
        return css::uno::Any();

    css::uno::Any aValue;
    try
    {
        aValue = xAccess->getByName(sKey);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    #if OSL_DEBUG_LEVEL > 0
    catch(const css::uno::Exception& ex)
    #else
    catch(const css::uno::Exception&)
    #endif
        {
            #if OSL_DEBUG_LEVEL > 0
            OSL_FAIL(OUStringToOString(ex.Message, RTL_TEXTENCODING_UTF8).getStr());
            #endif
            aValue.clear();
        }

    return aValue;
}


css::uno::Reference< css::uno::XInterface > FilterCache::impl_createConfigAccess(const OUString& sRoot       ,
                                                                                       bool         bReadOnly   ,
                                                                                       bool         bLocalesMode)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    css::uno::Reference< css::uno::XInterface > xCfg;

    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
            css::configuration::theDefaultProvider::get( comphelper::getProcessComponentContext() ) );

        ::std::vector< css::uno::Any > lParams;
        css::beans::NamedValue aParam;

        // set root path
        aParam.Name = "nodepath";
        aParam.Value <<= sRoot;
        lParams.push_back(css::uno::makeAny(aParam));

        // enable "all locales mode" ... if required
        if (bLocalesMode)
        {
            aParam.Name = "locale";
            aParam.Value <<= OUString("*");
            lParams.push_back(css::uno::makeAny(aParam));
        }

        // open it
        if (bReadOnly)
            xCfg = xConfigProvider->createInstanceWithArguments(SERVICE_CONFIGURATIONACCESS,
                    comphelper::containerToSequence(lParams));
        else
            xCfg = xConfigProvider->createInstanceWithArguments(SERVICE_CONFIGURATIONUPDATEACCESS,
                    comphelper::containerToSequence(lParams));

        // If configuration could not be opened ... but factory method does not throwed an exception
        // trigger throwing of our own CorruptedFilterConfigurationException.
        // Let message empty. The normal exception text show enough information to the user.
        if (! xCfg.is())
            throw css::uno::Exception(
                    "Got NULL reference on opening configuration file ... but no exception.",
                    css::uno::Reference< css::uno::XInterface >());
    }
    catch(const css::uno::Exception& ex)
    {
        throw css::document::CorruptedFilterConfigurationException(
                "filter configuration, caught: " + ex.Message,
                css::uno::Reference< css::uno::XInterface >(),
                ex.Message);
    }

    return xCfg;
    // <- SAFE
}


void FilterCache::impl_validateAndOptimize()
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // First check if any filter or type could be readed
    // from the underlying configuration!
    bool bSomeTypesShouldExist   = ((m_eFillState & E_CONTAINS_STANDARD       ) == E_CONTAINS_STANDARD       );
    bool bAllFiltersShouldExist  = ((m_eFillState & E_CONTAINS_FILTERS        ) == E_CONTAINS_FILTERS        );

#if OSL_DEBUG_LEVEL > 0

    sal_Int32             nWarnings = 0;

//  sal_Bool bAllTypesShouldExist    = ((m_eFillState & E_CONTAINS_TYPES          ) == E_CONTAINS_TYPES          );
    bool bAllLoadersShouldExist  = ((m_eFillState & E_CONTAINS_FRAMELOADERS   ) == E_CONTAINS_FRAMELOADERS   );
    bool bAllHandlersShouldExist = ((m_eFillState & E_CONTAINS_CONTENTHANDLERS) == E_CONTAINS_CONTENTHANDLERS);
#endif

    if (
        (
            (bSomeTypesShouldExist) &&
            (m_lTypes.empty())
        ) ||
        (
            (bAllFiltersShouldExist) &&
            (m_lFilters.empty())
        )
       )
    {
        throw css::document::CorruptedFilterConfigurationException(
                "filter configuration: the list of types or filters is empty",
                css::uno::Reference< css::uno::XInterface >(),
                "The list of types or filters is empty." );
    }

    // Create a log for all detected problems, which
    // occur in the next few lines.
    // If there are some real errors throw a RuntimException!
    // If there are some warnings only, show an assertion.
    sal_Int32             nErrors   = 0;
    OUStringBuffer sLog(256);

    CacheItemList::iterator pIt;

    for (pIt = m_lTypes.begin(); pIt != m_lTypes.end(); ++pIt)
    {
        OUString sType = pIt->first;
        CacheItem       aType = pIt->second;

        // get its registration for file Extensions AND(!) URLPattern ...
        // It doesn't matter if these items exists or if our
        // used index access create some default ones ...
        // only in case there is no filled set of Extensions AND
        // no filled set of URLPattern -> we must try to remove this invalid item
        // from this cache!
        css::uno::Sequence< OUString > lExtensions;
        css::uno::Sequence< OUString > lURLPattern;
        aType[PROPNAME_EXTENSIONS] >>= lExtensions;
        aType[PROPNAME_URLPATTERN] >>= lURLPattern;
        sal_Int32 ce = lExtensions.getLength();
        sal_Int32 cu = lURLPattern.getLength();

#if OSL_DEBUG_LEVEL > 0

        OUString sInternalTypeNameCheck;
        aType[PROPNAME_NAME] >>= sInternalTypeNameCheck;
        if (!sInternalTypeNameCheck.equals(sType))
        {
            sLog.append("Warning\t:\t" "The type \"" + sType + "\" does support the property \"Name\" correctly.\n");
            ++nWarnings;
        }

        if (!ce && !cu)
        {
            sLog.append("Warning\t:\t" "The type \"" + sType + "\" does not contain any URL pattern nor any extensions.\n");
            ++nWarnings;
        }
#endif

        // create an optimized registration for this type to
        // its set list of extensions/url pattern. If it's a "normal" type
        // set it at the end of this optimized list. But if its
        // a "Preferred" one - set it to the front of this list.
        // Of course multiple "Preferred" registrations can occur
        // (they shouldn't - but they can!) ... Ignore it. The last
        // preferred type is useable in the same manner then every
        // other type!
        bool bPreferred = false;
        aType[PROPNAME_PREFERRED] >>= bPreferred;

        const OUString* pExtensions = lExtensions.getConstArray();
        for (sal_Int32 e=0; e<ce; ++e)
        {
            // Note: We must be sure that address the right hash entry
            // does not depend from any upper/lower case problems ...
            OUString sNormalizedExtension = pExtensions[e].toAsciiLowerCase();

            OUStringList& lTypesForExtension = m_lExtensions2Types[sNormalizedExtension];
            if (::std::find(lTypesForExtension.begin(), lTypesForExtension.end(), sType) != lTypesForExtension.end())
                continue;

            if (bPreferred)
                lTypesForExtension.insert(lTypesForExtension.begin(), sType);
            else
                lTypesForExtension.push_back(sType);
        }

        const OUString* pURLPattern = lURLPattern.getConstArray();
        for (sal_Int32 u=0; u<cu; ++u)
        {
            OUStringList& lTypesForURLPattern = m_lURLPattern2Types[pURLPattern[u]];
            if (::std::find(lTypesForURLPattern.begin(), lTypesForURLPattern.end(), sType) != lTypesForURLPattern.end())
                continue;

            if (bPreferred)
                lTypesForURLPattern.insert(lTypesForURLPattern.begin(), sType);
            else
                lTypesForURLPattern.push_back(sType);
        }

#if OSL_DEBUG_LEVEL > 0

        // Don't check cross references between types and filters, if
        // not all filters read from disk!
        // OK - this cache can read single filters on demand too ...
        // but then the fill state of this cache should not be set to E_CONTAINS_FILTERS!
        if (!bAllFiltersShouldExist)
            continue;

        OUString sPrefFilter;
        aType[PROPNAME_PREFERREDFILTER] >>= sPrefFilter;
        if (sPrefFilter.isEmpty())
        {
            // OK - there is no filter for this type. But thats not an error.
            // May be it can be handled by a ContentHandler ...
            // But at this time its not guaranteed that there is any ContentHandler
            // or FrameLoader inside this cache ... but on disk ...
            bool bReferencedByLoader  = true;
            bool bReferencedByHandler = true;
            if (bAllLoadersShouldExist)
                bReferencedByLoader = !impl_searchFrameLoaderForType(sType).isEmpty();

            if (bAllHandlersShouldExist)
                bReferencedByHandler = !impl_searchContentHandlerForType(sType).isEmpty();

            if (
                (!bReferencedByLoader ) &&
                (!bReferencedByHandler)
               )
            {
                sLog.append("Warning\t:\t" "The type \"" + sType + "\" is not used by any filter, loader or content handler.\n");
                ++nWarnings;
            }
        }

        if (!sPrefFilter.isEmpty())
        {
            CacheItemList::const_iterator pIt2 = m_lFilters.find(sPrefFilter);
            if (pIt2 == m_lFilters.end())
            {
                if (bAllFiltersShouldExist)
                {
                    ++nWarnings; // preferred filters can point to a non-installed office module ! no error ... it's a warning only .-(
                    sLog.append("error\t:\t");
                }
                else
                {
                    ++nWarnings;
                    sLog.append("warning\t:\t");
                }

                sLog.append("The type \"" + sType + "\" points to an invalid filter \"" + sPrefFilter + "\".\n");
                continue;
            }

            CacheItem       aPrefFilter   = pIt2->second;
            OUString sFilterTypeReg;
            aPrefFilter[PROPNAME_TYPE] >>= sFilterTypeReg;
            if (sFilterTypeReg != sType)
            {
                sLog.append("error\t:\t" "The preferred filter \""
                    + sPrefFilter + "\" of type \"" + sType +
                    "\" is registered for another type \"" + sFilterTypeReg +
                    "\".\n");
                ++nErrors;
            }

            sal_Int32 nFlags = 0;
            aPrefFilter[PROPNAME_FLAGS] >>= nFlags;
            if (!(static_cast<SfxFilterFlags>(nFlags) & SfxFilterFlags::IMPORT))
            {
                sLog.append("error\t:\t" "The preferred filter \"" + sPrefFilter + "\" of type \"" +
                            sType + "\" is not an IMPORT filter!\n");
                ++nErrors;
            }

            OUString sInternalFilterNameCheck;
            aPrefFilter[PROPNAME_NAME] >>= sInternalFilterNameCheck;
            if (!sInternalFilterNameCheck.equals(sPrefFilter))
            {
                sLog.append("Warning\t:\t" "The filter \"" + sPrefFilter +
                            "\" does support the property \"Name\" correctly.\n");
                ++nWarnings;
            }
        }
#endif
    }

    // create dependencies between the global default frame loader
    // and all types (and of course if registered filters), which
    // does not registered for any other loader.
    css::uno::Any   aDirectValue       = impl_getDirectCFGValue(CFGDIRECTKEY_DEFAULTFRAMELOADER);
    OUString sDefaultFrameLoader;

    if (
        (!(aDirectValue >>= sDefaultFrameLoader)) ||
        (sDefaultFrameLoader.isEmpty()       )
       )
    {
        sLog.append("error\t:\t" "There is no valid default frame loader!?\n");
        ++nErrors;
    }

    // a) get list of all well known types
    // b) step over all well known frame loader services
    //    and remove all types from list a), which already
    //    referenced by a loader b)
    OUStringList lTypes = getItemNames(E_TYPE);
    for (  pIt  = m_lFrameLoaders.begin();
           pIt != m_lFrameLoaders.end()  ;
         ++pIt                           )
    {
        // Note: of course the default loader must be ignored here.
        // Because we replace its registration later completely with all
        // types, which are not referenced by any other loader.
        // So we can avoid our code against the complexity of a diff!
        OUString sLoader = pIt->first;
        if (sLoader.equals(sDefaultFrameLoader))
            continue;

        CacheItem&     rLoader   = pIt->second;
        css::uno::Any& rTypesReg = rLoader[PROPNAME_TYPES];
        OUStringList   lTypesReg (comphelper::sequenceToContainer<OUStringList>(rTypesReg.get<css::uno::Sequence<OUString> >()));

        for (OUStringList::const_iterator pTypesReg  = lTypesReg.begin();
                                          pTypesReg != lTypesReg.end()  ;
                                        ++pTypesReg                     )
        {
            OUStringList::iterator pTypeCheck = ::std::find(lTypes.begin(), lTypes.end(), *pTypesReg);
            if (pTypeCheck != lTypes.end())
                lTypes.erase(pTypeCheck);
        }
    }

    CacheItem& rDefaultLoader = m_lFrameLoaders[sDefaultFrameLoader];
    rDefaultLoader[PROPNAME_NAME ] <<= sDefaultFrameLoader;
    rDefaultLoader[PROPNAME_TYPES] <<= comphelper::containerToSequence(lTypes);

    OUString sLogOut = sLog.makeStringAndClear();
    OSL_ENSURE(!nErrors, OUStringToOString(sLogOut,RTL_TEXTENCODING_UTF8).getStr());
    if (nErrors>0)
        throw css::document::CorruptedFilterConfigurationException(
                "filter configuration: " + sLogOut,
                css::uno::Reference< css::uno::XInterface >(),
                sLogOut);
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE(!nWarnings, OUStringToOString(sLogOut,RTL_TEXTENCODING_UTF8).getStr());
#endif

    // <- SAFE
}

void FilterCache::impl_addItem2FlushList(      EItemType        eType,
                                         const OUString& sItem)
    throw(css::uno::Exception)
{
    OUStringList* pList = nullptr;
    switch(eType)
    {
        case E_TYPE :
                pList = &m_lChangedTypes;
                break;

        case E_FILTER :
                pList = &m_lChangedFilters;
                break;

        case E_FRAMELOADER :
                pList = &m_lChangedFrameLoaders;
                break;

        case E_CONTENTHANDLER :
                pList = &m_lChangedContentHandlers;
                break;

        default : throw css::uno::RuntimeException("unsupported item type", nullptr);
    }

    OUStringList::const_iterator pItem = ::std::find(pList->begin(), pList->end(), sItem);
    if (pItem == pList->end())
        pList->push_back(sItem);
}

FilterCache::EItemFlushState FilterCache::impl_specifyFlushOperation(const css::uno::Reference< css::container::XNameAccess >& xSet ,
                                                                     const CacheItemList&                                      rList,
                                                                     const OUString&                                    sItem)
    throw(css::uno::Exception)
{
    bool bExistsInConfigLayer = xSet->hasByName(sItem);
    bool bExistsInMemory      = (rList.find(sItem) != rList.end());

    EItemFlushState eState( E_ITEM_UNCHANGED );

    // !? ... such situation can occur, if an item was added and(!) removed before it was flushed :-)
    if (!bExistsInConfigLayer && !bExistsInMemory)
        eState = E_ITEM_UNCHANGED;
    else if (!bExistsInConfigLayer && bExistsInMemory)
        eState = E_ITEM_ADDED;
    else if (bExistsInConfigLayer && bExistsInMemory)
        eState = E_ITEM_CHANGED;
    else if (bExistsInConfigLayer && !bExistsInMemory)
        eState = E_ITEM_REMOVED;

    return eState;
}

void FilterCache::impl_load(EFillState eRequiredState)
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // Attention: Detect services are part of the standard set!
    // So there is no need to handle it separately.


    // a) The standard set of config value is needed.
    if (
        ((eRequiredState & E_CONTAINS_STANDARD) == E_CONTAINS_STANDARD) &&
        ((m_eFillState   & E_CONTAINS_STANDARD) != E_CONTAINS_STANDARD)
       )
    {
        // Attention! If config couldnt be opened successfully
        // and exception os thrown automatically and must be forwarded
        // to our calli ...
        css::uno::Reference< css::container::XNameAccess > xTypes(impl_openConfig(E_PROVIDER_TYPES), css::uno::UNO_QUERY_THROW);
        {
            SAL_INFO( "filter.config", "FilterCache::load std");
            impl_loadSet(xTypes, E_TYPE, E_READ_STANDARD, &m_lTypes);
        }
    }


    // b) We need all type information ...
    if (
        ((eRequiredState & E_CONTAINS_TYPES) == E_CONTAINS_TYPES) &&
        ((m_eFillState   & E_CONTAINS_TYPES) != E_CONTAINS_TYPES)
       )
    {
        // Attention! If config couldnt be opened successfully
        // and exception os thrown automatically and must be forwarded
        // to our calli ...
        css::uno::Reference< css::container::XNameAccess > xTypes(impl_openConfig(E_PROVIDER_TYPES), css::uno::UNO_QUERY_THROW);
        {
            SAL_INFO( "filter.config", "FilterCache::load all types");
            impl_loadSet(xTypes, E_TYPE, E_READ_UPDATE, &m_lTypes);
        }
    }


    // c) We need all filter information ...
    if (
        ((eRequiredState & E_CONTAINS_FILTERS) == E_CONTAINS_FILTERS) &&
        ((m_eFillState   & E_CONTAINS_FILTERS) != E_CONTAINS_FILTERS)
       )
    {
        // Attention! If config couldnt be opened successfully
        // and exception os thrown automatically and must be forwarded
        // to our calli ...
        css::uno::Reference< css::container::XNameAccess > xFilters(impl_openConfig(E_PROVIDER_FILTERS), css::uno::UNO_QUERY_THROW);
        {
            SAL_INFO( "filter.config", "FilterCache::load all filters");
            impl_loadSet(xFilters, E_FILTER, E_READ_ALL, &m_lFilters);
        }
    }


    // c) We need all frame loader information ...
    if (
        ((eRequiredState & E_CONTAINS_FRAMELOADERS) == E_CONTAINS_FRAMELOADERS) &&
        ((m_eFillState   & E_CONTAINS_FRAMELOADERS) != E_CONTAINS_FRAMELOADERS)
       )
    {
        // Attention! If config couldnt be opened successfully
        // and exception os thrown automatically and must be forwarded
        // to our calli ...
        css::uno::Reference< css::container::XNameAccess > xLoaders(impl_openConfig(E_PROVIDER_OTHERS), css::uno::UNO_QUERY_THROW);
        {
            SAL_INFO( "filter.config", "FilterCache::load all frame loader");
            impl_loadSet(xLoaders, E_FRAMELOADER, E_READ_ALL, &m_lFrameLoaders);
        }
    }


    // d) We need all content handler information ...
    if (
        ((eRequiredState & E_CONTAINS_CONTENTHANDLERS) == E_CONTAINS_CONTENTHANDLERS) &&
        ((m_eFillState   & E_CONTAINS_CONTENTHANDLERS) != E_CONTAINS_CONTENTHANDLERS)
       )
    {
        // Attention! If config couldnt be opened successfully
        // and exception os thrown automatically and must be forwarded
        // to our calli ...
        css::uno::Reference< css::container::XNameAccess > xHandlers(impl_openConfig(E_PROVIDER_OTHERS), css::uno::UNO_QUERY_THROW);
        {
            SAL_INFO( "filter.config", "FilterCache::load all content handler");
            impl_loadSet(xHandlers, E_CONTENTHANDLER, E_READ_ALL, &m_lContentHandlers);
        }
    }

    // update fill state. Note: it's a bit field, which combines different parts.
    m_eFillState = (EFillState) ((sal_Int32)m_eFillState | (sal_Int32)eRequiredState);

    // any data readed?
    // yes! => validate it and update optimized structures.
    impl_validateAndOptimize();

    // <- SAFE
}


void FilterCache::impl_loadSet(const css::uno::Reference< css::container::XNameAccess >& xConfig,
                                     EItemType                                           eType  ,
                                     EReadOption                                         eOption,
                                     CacheItemList*                                      pCache )
    throw(css::uno::Exception)
{
    // get access to the right configuration set
    OUString sSetName;
    switch(eType)
    {
        case E_TYPE :
            sSetName = CFGSET_TYPES;
            break;

        case E_FILTER :
            sSetName = CFGSET_FILTERS;
            break;

        case E_FRAMELOADER :
            sSetName = CFGSET_FRAMELOADERS;
            break;

        case E_CONTENTHANDLER :
            sSetName = CFGSET_CONTENTHANDLERS;
            break;
        default: break;
    }

    css::uno::Reference< css::container::XNameAccess > xSet;
    css::uno::Sequence< OUString >              lItems;

    try
    {
        css::uno::Any aVal = xConfig->getByName(sSetName);
        if (!(aVal >>= xSet) || !xSet.is())
        {
            OUString sMsg("Could not open configuration set \"" + sSetName + "\".");
            throw css::uno::Exception(sMsg, css::uno::Reference< css::uno::XInterface >());
        }
        lItems = xSet->getElementNames();
    }
    catch(const css::uno::Exception& ex)
    {
        throw css::document::CorruptedFilterConfigurationException(
                "filter configuration, caught: " + ex.Message,
                css::uno::Reference< css::uno::XInterface >(),
                ex.Message);
    }

    // get names of all existing sub items of this set
    // step over it and fill internal cache structures.

    // But don't update optimized structures like e.g. hash
    // for mapping extensions to its types!

    const OUString* pItems = lItems.getConstArray();
          sal_Int32        c      = lItems.getLength();
    for (sal_Int32 i=0; i<c; ++i)
    {
        CacheItemList::iterator pItem = pCache->find(pItems[i]);
        switch(eOption)
        {
            // a) read a standard set of properties only or read all
            case E_READ_STANDARD :
            case E_READ_ALL      :
            {
                try
                {
                    (*pCache)[pItems[i]] = impl_loadItem(xSet, eType, pItems[i], eOption);
                }
                catch(const css::uno::Exception& ex)
                {
                    throw css::document::CorruptedFilterConfigurationException(
                            "filter configuration, caught: " + ex.Message,
                            css::uno::Reference< css::uno::XInterface >(),
                            ex.Message);
                }
            }
            break;

            // b) read optional properties only!
            //    All items must already exist inside our cache.
            //    But they must be updated.
            case E_READ_UPDATE :
            {
                if (pItem == pCache->end())
                {
                    OUString sMsg("item \"" + pItems[i] + "\" not found for update!");
                    throw css::uno::Exception(sMsg, css::uno::Reference< css::uno::XInterface >());
                }
                try
                {
                    CacheItem aItem = impl_loadItem(xSet, eType, pItems[i], eOption);
                    pItem->second.update(aItem);
                }
                catch(const css::uno::Exception& ex)
                {
                    throw css::document::CorruptedFilterConfigurationException(
                            "filter configuration, caught: " + ex.Message,
                            css::uno::Reference< css::uno::XInterface >(),
                            ex.Message);
                }
            }
            break;
            default: break;
        }
    }
}


void FilterCache::impl_readPatchUINames(const css::uno::Reference< css::container::XNameAccess >& xNode,
                                              CacheItem&                                          rItem)
    throw(css::uno::Exception)
{

    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);
    OUString sActLocale     = m_sActLocale    ;
    aLock.clear();
    // <- SAFE ----------------------------------

    css::uno::Any aVal = xNode->getByName(PROPNAME_UINAME);
    css::uno::Reference< css::container::XNameAccess > xUIName;
    if (!(aVal >>= xUIName) && !xUIName.is())
        return;

    const ::std::vector< OUString >                 lLocales(comphelper::sequenceToContainer< ::std::vector< OUString >>(
                                                                xUIName->getElementNames()));
          ::std::vector< OUString >::const_iterator pLocale ;
          ::comphelper::SequenceAsHashMap                                   lUINames;

    for (  pLocale  = lLocales.begin();
           pLocale != lLocales.end()  ;
         ++pLocale                    )
    {
        const OUString& sLocale = *pLocale;

        OUString sValue;
        xUIName->getByName(sLocale) >>= sValue;

        lUINames[sLocale] <<= sValue;
    }

    aVal <<= lUINames.getAsConstPropertyValueList();
    rItem[PROPNAME_UINAMES] = aVal;

    // find right UIName for current office locale
    // Use fallbacks too!
    pLocale = LanguageTag::getFallback(lLocales, sActLocale);
    if (pLocale == lLocales.end())
    {
#if OSL_DEBUG_LEVEL > 0
        if ( sActLocale == "en-US" )
            return;
        OUString sName = rItem.getUnpackedValueOrDefault(PROPNAME_NAME, OUString());

        OUString sMsg("Fallback scenario for filter or type '" + sName + "' and locale '" +
                      sActLocale + "' failed. Please check your filter configuration.");

        OSL_FAIL(_FILTER_CONFIG_TO_ASCII_(sMsg));
#endif
        return;
    }

    const OUString& sLocale = *pLocale;
    ::comphelper::SequenceAsHashMap::const_iterator pUIName = lUINames.find(sLocale);
    if (pUIName != lUINames.end())
        rItem[PROPNAME_UINAME] = pUIName->second;
}


void FilterCache::impl_savePatchUINames(const css::uno::Reference< css::container::XNameReplace >& xNode,
                                        const CacheItem&                                           rItem)
    throw(css::uno::Exception)
{
    css::uno::Reference< css::container::XNameContainer > xAdd  (xNode, css::uno::UNO_QUERY);
    css::uno::Reference< css::container::XNameAccess >    xCheck(xNode, css::uno::UNO_QUERY);

    css::uno::Sequence< css::beans::PropertyValue > lUINames = rItem.getUnpackedValueOrDefault(PROPNAME_UINAMES, css::uno::Sequence< css::beans::PropertyValue >());
    sal_Int32                                       c        = lUINames.getLength();
    const css::beans::PropertyValue*                pUINames = lUINames.getConstArray();

    for (sal_Int32 i=0; i<c; ++i)
    {
        if (xCheck->hasByName(pUINames[i].Name))
            xNode->replaceByName(pUINames[i].Name, pUINames[i].Value);
        else
            xAdd->insertByName(pUINames[i].Name, pUINames[i].Value);
    }
}

/*-----------------------------------------------
    TODO
        clarify, how the real problem behind the
        wrong constructed CacheItem instance (which
        will force a crash during destruction)
        can be solved ...
-----------------------------------------------*/
CacheItem FilterCache::impl_loadItem(const css::uno::Reference< css::container::XNameAccess >& xSet   ,
                                           EItemType                                           eType  ,
                                     const OUString&                                    sItem  ,
                                           EReadOption                                         eOption)
    throw(css::uno::Exception)
{
    // try to get an API object, which points directly to the
    // requested item. If it fail an exception should occur and
    // break this operation. Of course returned API object must be
    // checked too.
    css::uno::Reference< css::container::XNameAccess > xItem;
    #ifdef WORKAROUND_EXCEPTION_PROBLEM
    try
    {
    #endif
        css::uno::Any aVal = xSet->getByName(sItem);
        if (!(aVal >>= xItem) || !xItem.is())
        {
            OUString sMsg("found corrupted item \"" + sItem + "\".");
            throw css::uno::RuntimeException(sMsg, css::uno::Reference< css::uno::XInterface >());
        }
    #ifdef WORKAROUND_EXCEPTION_PROBLEM
    }
    catch(const css::container::NoSuchElementException&)
    {
        throw;
    }
    #endif

    // set too. Of course its already used as key into the e.g. outside
    // used hash map ... but some of our API methods provide
    // this property set as result only. But the user of this CacheItem
    // should know, which value the key names has :-) ITS IMPORTANT!
    CacheItem aItem;
    aItem[PROPNAME_NAME] = css::uno::makeAny(sItem);
    switch(eType)
    {
        case E_TYPE :
        {
            assert(eOption >= 0 && eOption <= E_READ_ALL);
            css::uno::Sequence< OUString > &rNames = m_aTypeProps[eOption];

            // read standard properties of a filter
            if (rNames.getLength() > 0)
            {
                css::uno::Reference< css::beans::XMultiPropertySet >
                    xPropSet( xItem, css::uno::UNO_QUERY_THROW);
                css::uno::Sequence< css::uno::Any > aValues;
                aValues = xPropSet->getPropertyValues(rNames);

                for (sal_Int32 i = 0; i < aValues.getLength(); i++)
                    aItem[rNames[i]] = aValues[i];
            }

            // read optional properties of a type
            // no else here! Is an additional switch ...
            if (eOption == E_READ_UPDATE || eOption == E_READ_ALL)
                impl_readPatchUINames(xItem, aItem);
        }
        break;


        case E_FILTER :
        {
            assert(eOption >= 0 && eOption <= E_READ_ALL);
            css::uno::Sequence< OUString > &rNames = m_aStandardProps[eOption];

            // read standard properties of a filter
            if (rNames.getLength() > 0)
            {
                css::uno::Reference< css::beans::XMultiPropertySet >
                    xPropSet( xItem, css::uno::UNO_QUERY_THROW);
                css::uno::Sequence< css::uno::Any > aValues;
                aValues = xPropSet->getPropertyValues(rNames);

                for (sal_Int32 i = 0; i < rNames.getLength(); i++)
                {
                    OUString &rPropName = rNames[i];
                    if (i != rNames.getLength() - 1 || rPropName != PROPNAME_FLAGS)
                        aItem[rPropName] = aValues[i];
                    else
                    {
                        assert(rPropName == PROPNAME_FLAGS);
                        // special handling for flags! Convert it from a list of names to its
                        // int representation ...
                        css::uno::Sequence< OUString > lFlagNames;
                        if (aValues[i] >>= lFlagNames)
                            aItem[rPropName] <<= (sal_Int32) FilterCache::impl_convertFlagNames2FlagField(lFlagNames);
                    }
                }
            }
//TODO remove it if moving of filter uinames to type uinames
//       will be finished really
#ifdef AS_ENABLE_FILTER_UINAMES
            if (eOption == E_READ_UPDATE || eOption == E_READ_ALL)
                impl_readPatchUINames(xItem, aItem);
#endif // AS_ENABLE_FILTER_UINAMES
        }
        break;

        case E_FRAMELOADER :
        case E_CONTENTHANDLER :
            aItem[PROPNAME_TYPES] = xItem->getByName(PROPNAME_TYPES);
            break;
        default: break;
    }

    return aItem;
}


CacheItemList::iterator FilterCache::impl_loadItemOnDemand(      EItemType        eType,
                                                           const OUString& sItem)
    throw(css::uno::Exception)
{
    CacheItemList*                              pList   = nullptr;
    css::uno::Reference< css::uno::XInterface > xConfig    ;
    OUString                             sSet       ;

    switch(eType)
    {
        case E_TYPE :
        {
            pList   = &m_lTypes;
            xConfig = impl_openConfig(E_PROVIDER_TYPES);
            sSet    = CFGSET_TYPES;
        }
        break;

        case E_FILTER :
        {
            pList   = &m_lFilters;
            xConfig = impl_openConfig(E_PROVIDER_FILTERS);
            sSet    = CFGSET_FILTERS;
        }
        break;

        case E_FRAMELOADER :
        {
            pList   = &m_lFrameLoaders;
            xConfig = impl_openConfig(E_PROVIDER_OTHERS);
            sSet    = CFGSET_FRAMELOADERS;
        }
        break;

        case E_CONTENTHANDLER :
        {
            pList   = &m_lContentHandlers;
            xConfig = impl_openConfig(E_PROVIDER_OTHERS);
            sSet    = CFGSET_CONTENTHANDLERS;
        }
        break;
    }

    if (!pList)
        throw css::container::NoSuchElementException();

    css::uno::Reference< css::container::XNameAccess > xRoot(xConfig, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNameAccess > xSet ;
    xRoot->getByName(sSet) >>= xSet;

    CacheItemList::iterator pItemInCache  = pList->find(sItem);
    bool                bItemInConfig = xSet->hasByName(sItem);

    if (bItemInConfig)
    {
        (*pList)[sItem] = impl_loadItem(xSet, eType, sItem, E_READ_ALL);
        _FILTER_CONFIG_LOG_2_("impl_loadItemOnDemand(%d, \"%s\") ... OK", (int)eType, _FILTER_CONFIG_TO_ASCII_(sItem).getStr())
    }
    else
    {
        if (pItemInCache != pList->end())
            pList->erase(pItemInCache);
        // OK - this item does not exists inside configuration.
        // And we already updated our internal cache.
        // But the outside code needs this NoSuchElementException
        // to know, that this item does notexists.
        // Nobody checks the iterator!
        throw css::container::NoSuchElementException();
    }

    return pList->find(sItem);
}


void FilterCache::impl_saveItem(const css::uno::Reference< css::container::XNameReplace >& xItem,
                                      EItemType                                            eType,
                                const CacheItem                                            aItem)
    throw(css::uno::Exception)
{
    CacheItem::const_iterator pIt;
    switch(eType)
    {

        case E_TYPE :
        {
            pIt = aItem.find(PROPNAME_PREFERREDFILTER);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_PREFERREDFILTER, pIt->second);
            pIt = aItem.find(PROPNAME_DETECTSERVICE);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_DETECTSERVICE, pIt->second);
            pIt = aItem.find(PROPNAME_URLPATTERN);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_URLPATTERN, pIt->second);
            pIt = aItem.find(PROPNAME_EXTENSIONS);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_EXTENSIONS, pIt->second);
            pIt = aItem.find(PROPNAME_PREFERRED);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_PREFERRED, pIt->second);
            pIt = aItem.find(PROPNAME_MEDIATYPE);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_MEDIATYPE, pIt->second);
            pIt = aItem.find(PROPNAME_CLIPBOARDFORMAT);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_CLIPBOARDFORMAT, pIt->second);

            css::uno::Reference< css::container::XNameReplace > xUIName;
            xItem->getByName(PROPNAME_UINAME) >>= xUIName;
            impl_savePatchUINames(xUIName, aItem);
        }
        break;


        case E_FILTER :
        {
            pIt = aItem.find(PROPNAME_TYPE);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_TYPE, pIt->second);
            pIt = aItem.find(PROPNAME_FILEFORMATVERSION);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_FILEFORMATVERSION, pIt->second);
            pIt = aItem.find(PROPNAME_UICOMPONENT);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_UICOMPONENT, pIt->second);
            pIt = aItem.find(PROPNAME_FILTERSERVICE);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_FILTERSERVICE, pIt->second);
            pIt = aItem.find(PROPNAME_DOCUMENTSERVICE);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_DOCUMENTSERVICE, pIt->second);
            pIt = aItem.find(PROPNAME_USERDATA);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_USERDATA, pIt->second);
            pIt = aItem.find(PROPNAME_TEMPLATENAME);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_TEMPLATENAME, pIt->second);

            // special handling for flags! Convert it from an integer flag field back
            // to a list of names ...
            // But note: because we work directly on a reference to the cache item,
            // its not allowed to change the value here. We must work on a copy!
            pIt = aItem.find(PROPNAME_FLAGS);
            if (pIt != aItem.end())
            {
                sal_Int32 nFlags = 0;
                pIt->second >>= nFlags;
                css::uno::Any aFlagNameList;
                aFlagNameList <<= FilterCache::impl_convertFlagField2FlagNames(static_cast<SfxFilterFlags>(nFlags));
                xItem->replaceByName(PROPNAME_FLAGS, aFlagNameList);
            }

//TODO remove it if moving of filter uinames to type uinames
//       will be finished really
#ifdef AS_ENABLE_FILTER_UINAMES
            css::uno::Reference< css::container::XNameReplace > xUIName;
            xItem->getByName(PROPNAME_UINAME) >>= xUIName;
            impl_savePatchUINames(xUIName, aItem);
#endif //  AS_ENABLE_FILTER_UINAMES
        }
        break;


        case E_FRAMELOADER :
        case E_CONTENTHANDLER :
        {
            pIt = aItem.find(PROPNAME_TYPES);
            if (pIt != aItem.end())
                xItem->replaceByName(PROPNAME_TYPES, pIt->second);
        }
        break;
        default: break;
    }
}

/*-----------------------------------------------
    static! => no locks necessary
-----------------------------------------------*/
css::uno::Sequence< OUString > FilterCache::impl_convertFlagField2FlagNames(SfxFilterFlags nFlags)
{
    OUStringList lFlagNames;

    if (nFlags & SfxFilterFlags::STARONEFILTER    ) lFlagNames.push_back(FLAGNAME_3RDPARTYFILTER   );
    if (nFlags & SfxFilterFlags::ALIEN            ) lFlagNames.push_back(FLAGNAME_ALIEN            );
    if (nFlags & SfxFilterFlags::CONSULTSERVICE   ) lFlagNames.push_back(FLAGNAME_CONSULTSERVICE   );
    if (nFlags & SfxFilterFlags::DEFAULT          ) lFlagNames.push_back(FLAGNAME_DEFAULT          );
    if (nFlags & SfxFilterFlags::ENCRYPTION       ) lFlagNames.push_back(FLAGNAME_ENCRYPTION       );
    if (nFlags & SfxFilterFlags::EXPORT           ) lFlagNames.push_back(FLAGNAME_EXPORT           );
    if (nFlags & SfxFilterFlags::IMPORT           ) lFlagNames.push_back(FLAGNAME_IMPORT           );
    if (nFlags & SfxFilterFlags::INTERNAL         ) lFlagNames.push_back(FLAGNAME_INTERNAL         );
    if (nFlags & SfxFilterFlags::NOTINFILEDLG     ) lFlagNames.push_back(FLAGNAME_NOTINFILEDIALOG  );
    if (nFlags & SfxFilterFlags::MUSTINSTALL      ) lFlagNames.push_back(FLAGNAME_NOTINSTALLED     );
    if (nFlags & SfxFilterFlags::OWN              ) lFlagNames.push_back(FLAGNAME_OWN              );
    if (nFlags & SfxFilterFlags::PACKED           ) lFlagNames.push_back(FLAGNAME_PACKED           );
    if (nFlags & SfxFilterFlags::PASSWORDTOMODIFY ) lFlagNames.push_back(FLAGNAME_PASSWORDTOMODIFY );
    if (nFlags & SfxFilterFlags::PREFERED         ) lFlagNames.push_back(FLAGNAME_PREFERRED        );
    if (nFlags & SfxFilterFlags::STARTPRESENTATION) lFlagNames.push_back(FLAGNAME_STARTPRESENTATION);
    if (nFlags & SfxFilterFlags::OPENREADONLY     ) lFlagNames.push_back(FLAGNAME_READONLY         );
    if (nFlags & SfxFilterFlags::SUPPORTSSELECTION) lFlagNames.push_back(FLAGNAME_SUPPORTSSELECTION);
    if (nFlags & SfxFilterFlags::TEMPLATE         ) lFlagNames.push_back(FLAGNAME_TEMPLATE         );
    if (nFlags & SfxFilterFlags::TEMPLATEPATH     ) lFlagNames.push_back(FLAGNAME_TEMPLATEPATH     );
    if (nFlags & SfxFilterFlags::COMBINED         ) lFlagNames.push_back(FLAGNAME_COMBINED         );
    if (nFlags & SfxFilterFlags::SUPPORTSSIGNING) lFlagNames.push_back(FLAGNAME_SUPPORTSSIGNING);

    return comphelper::containerToSequence(lFlagNames);
}

/*-----------------------------------------------
    static! => no locks necessary
-----------------------------------------------*/
SfxFilterFlags FilterCache::impl_convertFlagNames2FlagField(const css::uno::Sequence< OUString >& lNames)
{
    SfxFilterFlags nField = SfxFilterFlags::NONE;

    const OUString* pNames = lNames.getConstArray();
    sal_Int32       c      = lNames.getLength();
    for (sal_Int32 i=0; i<c; ++i)
    {
        if (pNames[i] == FLAGNAME_3RDPARTYFILTER)
        {
            nField |= SfxFilterFlags::STARONEFILTER;
            continue;
        }
        if (pNames[i] == FLAGNAME_ALIEN)
        {
            nField |= SfxFilterFlags::ALIEN;
            continue;
        }
        if (pNames[i] == FLAGNAME_CONSULTSERVICE)
        {
            nField |= SfxFilterFlags::CONSULTSERVICE;
            continue;
        }
        if (pNames[i] == FLAGNAME_DEFAULT)
        {
            nField |= SfxFilterFlags::DEFAULT;
            continue;
        }
        if (pNames[i] == FLAGNAME_ENCRYPTION)
        {
            nField |= SfxFilterFlags::ENCRYPTION;
            continue;
        }
        if (pNames[i] == FLAGNAME_EXPORT)
        {
            nField |= SfxFilterFlags::EXPORT;
            continue;
        }
        if (pNames[i] == FLAGNAME_IMPORT)
        {
            nField |= SfxFilterFlags::IMPORT;
            continue;
        }
        if (pNames[i] == FLAGNAME_INTERNAL)
        {
            nField |= SfxFilterFlags::INTERNAL;
            continue;
        }
        if (pNames[i] == FLAGNAME_NOTINFILEDIALOG)
        {
            nField |= SfxFilterFlags::NOTINFILEDLG;
            continue;
        }
        if (pNames[i] == FLAGNAME_NOTINSTALLED)
        {
            nField |= SfxFilterFlags::MUSTINSTALL;
            continue;
        }
        if (pNames[i] == FLAGNAME_OWN)
        {
            nField |= SfxFilterFlags::OWN;
            continue;
        }
        if (pNames[i] == FLAGNAME_PACKED)
        {
            nField |= SfxFilterFlags::PACKED;
            continue;
        }
        if (pNames[i] == FLAGNAME_PASSWORDTOMODIFY)
        {
            nField |= SfxFilterFlags::PASSWORDTOMODIFY;
            continue;
        }
        if (pNames[i] == FLAGNAME_PREFERRED)
        {
            nField |= SfxFilterFlags::PREFERED;
            continue;
        }
        if (pNames[i] == FLAGNAME_STARTPRESENTATION)
        {
            nField |= SfxFilterFlags::STARTPRESENTATION;
            continue;
        }
        if (pNames[i] == FLAGNAME_SUPPORTSSIGNING)
        {
            nField |= SfxFilterFlags::SUPPORTSSIGNING;
            continue;
        }
        if (pNames[i] == FLAGNAME_READONLY)
        {
            nField |= SfxFilterFlags::OPENREADONLY;
            continue;
        }
        if (pNames[i] == FLAGNAME_SUPPORTSSELECTION)
        {
            nField |= SfxFilterFlags::SUPPORTSSELECTION;
            continue;
        }
        if (pNames[i] == FLAGNAME_TEMPLATE)
        {
            nField |= SfxFilterFlags::TEMPLATE;
            continue;
        }
        if (pNames[i] == FLAGNAME_TEMPLATEPATH)
        {
            nField |= SfxFilterFlags::TEMPLATEPATH;
            continue;
        }
        if (pNames[i] == FLAGNAME_COMBINED)
        {
            nField |= SfxFilterFlags::COMBINED;
            continue;
        }
    }

    return nField;
}


void FilterCache::impl_interpretDataVal4Type(const OUString& sValue,
                                                   sal_Int32        nProp ,
                                                   CacheItem&       rItem )
{
    switch(nProp)
    {
        // Preferred
        case 0:     {
                        if (sValue.toInt32() == 1)
                            rItem[PROPNAME_PREFERRED] = css::uno::makeAny(sal_True);
                        else
                            rItem[PROPNAME_PREFERRED] = css::uno::makeAny(sal_False);
                    }
                    break;
        // MediaType
        case 1:     rItem[PROPNAME_MEDIATYPE] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        // ClipboardFormat
        case 2:     rItem[PROPNAME_CLIPBOARDFORMAT] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        // URLPattern
        case 3:     rItem[PROPNAME_URLPATTERN] <<= comphelper::containerToSequence(impl_tokenizeString(sValue, (sal_Unicode)';'));
                    break;
        // Extensions
        case 4:     rItem[PROPNAME_EXTENSIONS] <<= comphelper::containerToSequence(impl_tokenizeString(sValue, (sal_Unicode)';'));
                    break;
    }
}


void FilterCache::impl_interpretDataVal4Filter(const OUString& sValue,
                                                     sal_Int32        nProp ,
                                                     CacheItem&       rItem )
{
    switch(nProp)
    {
        // Order
        case 0:     {
                        sal_Int32 nOrder = sValue.toInt32();
                        if (nOrder > 0)
                        {
                            SAL_WARN( "filter.config", "FilterCache::impl_interpretDataVal4Filter()\nCan not move Order value from filter to type on demand!");
                            _FILTER_CONFIG_LOG_2_("impl_interpretDataVal4Filter(%d, \"%s\") ... OK", (int)eType, _FILTER_CONFIG_TO_ASCII_(rItem).getStr())
                        }
                    }
                    break;
        // Type
        case 1:     rItem[PROPNAME_TYPE] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        // DocumentService
        case 2:     rItem[PROPNAME_DOCUMENTSERVICE] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        // FilterService
        case 3:     rItem[PROPNAME_FILTERSERVICE] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        // Flags
        case 4:     rItem[PROPNAME_FLAGS] <<= sValue.toInt32();
                    break;
        // UserData
        case 5:     rItem[PROPNAME_USERDATA] <<= comphelper::containerToSequence(impl_tokenizeString(sValue, (sal_Unicode)';'));
                    break;
        // FileFormatVersion
        case 6:     rItem[PROPNAME_FILEFORMATVERSION] <<= sValue.toInt32();
                    break;
        // TemplateName
        case 7:     rItem[PROPNAME_TEMPLATENAME] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        // [optional!] UIComponent
        case 8:     rItem[PROPNAME_UICOMPONENT] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
    }
}

/*-----------------------------------------------
    TODO work on a cache copy first, which can be flushed afterwards
         That would be useful to guarantee a consistent cache.
-----------------------------------------------*/
void FilterCache::impl_readOldFormat()
{
    // Attention: Opening/Reading of this old configuration format has to be handled gracefully.
    // Its optional and should not disturb our normal work!
    // E.g. we must check, if the package exists ...
    try
    {
        css::uno::Reference< css::uno::XInterface > xInt = impl_openConfig(E_PROVIDER_OLD);
        css::uno::Reference< css::container::XNameAccess > xCfg =
            css::uno::Reference< css::container::XNameAccess >(xInt, css::uno::UNO_QUERY_THROW);

        OUString TYPES_SET("Types");

        // May be there is no type set ...
        if (xCfg->hasByName(TYPES_SET))
        {
            css::uno::Reference< css::container::XNameAccess > xSet;
            xCfg->getByName(TYPES_SET) >>= xSet;
            const css::uno::Sequence< OUString > lItems = xSet->getElementNames();
            const OUString*                      pItems = lItems.getConstArray();
            for (sal_Int32 i=0; i<lItems.getLength(); ++i)
                m_lTypes[pItems[i]] = impl_readOldItem(xSet, E_TYPE, pItems[i]);
        }

        OUString FILTER_SET("Filters");
        // May be there is no filter set ...
        if (xCfg->hasByName(FILTER_SET))
        {
            css::uno::Reference< css::container::XNameAccess > xSet;
            xCfg->getByName(FILTER_SET) >>= xSet;
            const css::uno::Sequence< OUString > lItems = xSet->getElementNames();
            const OUString*                      pItems = lItems.getConstArray();
            for (sal_Int32 i=0; i<lItems.getLength(); ++i)
                m_lFilters[pItems[i]] = impl_readOldItem(xSet, E_FILTER, pItems[i]);
        }
    }
    /* corrupt filter addon? Because it's external (optional) code.. we can ignore it. Addon won't work then...
       but that seems to be acceptable.
       see #139088# for further information
    */
    catch(const css::uno::Exception&)
    {
    }
}

CacheItem FilterCache::impl_readOldItem(const css::uno::Reference< css::container::XNameAccess >& xSet ,
                                              EItemType                                           eType,
                                        const OUString&                                    sItem)
    throw(css::uno::Exception)
{
    css::uno::Reference< css::container::XNameAccess > xItem;
    xSet->getByName(sItem) >>= xItem;
    if (!xItem.is())
        throw css::uno::Exception("Can not read old item.", css::uno::Reference< css::uno::XInterface >());

    CacheItem aItem;
    aItem[PROPNAME_NAME] <<= sItem;

    // Installed flag ...
    // Isn't used any longer!

    // UIName
    impl_readPatchUINames(xItem, aItem);

    // Data
    OUString sData;
    OUStringList    lData;
    xItem->getByName( "Data" ) >>= sData;
    lData = impl_tokenizeString(sData, (sal_Unicode)',');
    if (
        (sData.isEmpty()) ||
        (lData.size()<1    )
       )
    {
        throw css::uno::Exception( "Can not read old item property DATA.", css::uno::Reference< css::uno::XInterface >());
    }

    sal_Int32 nProp = 0;
    for (OUStringList::const_iterator pProp  = lData.begin();
                                      pProp != lData.end()  ;
                                    ++pProp                 )
    {
        const OUString& sProp = *pProp;
        switch(eType)
        {
            case E_TYPE :
                impl_interpretDataVal4Type(sProp, nProp, aItem);
                break;

            case E_FILTER :
                impl_interpretDataVal4Filter(sProp, nProp, aItem);
                break;
            default: break;
        }
        ++nProp;
    }

    return aItem;
}


OUStringList FilterCache::impl_tokenizeString(const OUString& sData     ,
                                                    sal_Unicode      cSeparator)
{
    OUStringList lData  ;
    sal_Int32    nToken = 0;
    do
    {
        OUString sToken = sData.getToken(0, cSeparator, nToken);
        lData.push_back(sToken);
    }
    while(nToken >= 0);
    return lData;
}

#if OSL_DEBUG_LEVEL > 0


OUString FilterCache::impl_searchFrameLoaderForType(const OUString& sType) const
{
    CacheItemList::const_iterator pIt;
    for (  pIt  = m_lFrameLoaders.begin();
           pIt != m_lFrameLoaders.end()  ;
         ++pIt                           )
    {
        const OUString& sItem = pIt->first;
        ::comphelper::SequenceAsHashMap lProps(pIt->second);
        OUStringList                    lTypes(
                comphelper::sequenceToContainer<OUStringList>(lProps[PROPNAME_TYPES].get<css::uno::Sequence<OUString> >()));

        if (::std::find(lTypes.begin(), lTypes.end(), sType) != lTypes.end())
            return sItem;
    }

    return OUString();
}


OUString FilterCache::impl_searchContentHandlerForType(const OUString& sType) const
{
    CacheItemList::const_iterator pIt;
    for (  pIt  = m_lContentHandlers.begin();
           pIt != m_lContentHandlers.end()  ;
         ++pIt                              )
    {
        const OUString& sItem = pIt->first;
        ::comphelper::SequenceAsHashMap lProps(pIt->second);
        OUStringList                    lTypes(
                comphelper::sequenceToContainer<OUStringList>( lProps[PROPNAME_TYPES].get<css::uno::Sequence<OUString> >() ));
        if (::std::find(lTypes.begin(), lTypes.end(), sType) != lTypes.end())
            return sItem;
    }

    return OUString();
}
#endif


bool FilterCache::impl_isModuleInstalled(const OUString& sModule)
{
    css::uno::Reference< css::container::XNameAccess > xCfg;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (! m_xModuleCfg.is())
    {
        m_xModuleCfg = officecfg::Setup::Office::Factories::get();
    }

    xCfg = m_xModuleCfg;
    aLock.clear();
    // <- SAFE

    if (xCfg.is())
        return xCfg->hasByName(sModule);

    return false;
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
