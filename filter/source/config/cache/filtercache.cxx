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


#include "filtercache.hxx"
#include "lateinitlistener.hxx"
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
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/document/CorruptedFilterConfigurationException.hpp>
#include <comphelper/sequenceasvector.hxx>
#include <comphelper/locale.hxx>
#include <comphelper/processfactory.hxx>

#include <unotools/configpaths.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/logfile.hxx>
#include <rtl/uri.hxx>
#include <tools/urlobj.hxx>
#include <tools/wldcrd.hxx>

#include <comphelper/configurationhelper.hxx>


namespace filter{
    namespace config{

namespace css = ::com::sun::star;

FilterCache::FilterCache()
    : BaseLock    (                                        )
    , m_xSMGR     (::comphelper::getProcessServiceFactory())
    , m_eFillState(E_CONTAINS_NOTHING                      )
{
    RTL_LOGFILE_TRACE("{ (as96863) FilterCache lifetime");
}



FilterCache::~FilterCache()
{
    RTL_LOGFILE_TRACE("} (as96863) FilterCache lifetime");
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

    // Dont copy the configuration access points here.
    // They will be created on demand inside the cloned instance,
    // if they are needed.

    pClone->m_xSMGR                      = m_xSMGR;

    pClone->m_lTypes                     = m_lTypes;
    pClone->m_lDetectServices            = m_lDetectServices;
    pClone->m_lFilters                   = m_lFilters;
    pClone->m_lFrameLoaders              = m_lFrameLoaders;
    pClone->m_lContentHandlers           = m_lContentHandlers;
    pClone->m_lExtensions2Types          = m_lExtensions2Types;
    pClone->m_lURLPattern2Types          = m_lURLPattern2Types;

    pClone->m_sActLocale                 = m_sActLocale;
    pClone->m_sFormatName                = m_sFormatName;
    pClone->m_sFormatVersion             = m_sFormatVersion;

    pClone->m_eFillState                 = m_eFillState;

    pClone->m_lChangedTypes              = m_lChangedTypes;
    pClone->m_lChangedFilters            = m_lChangedFilters;
    pClone->m_lChangedDetectServices     = m_lChangedDetectServices;
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
    // Dont copy the configuration access points here!
    // We must use our own ones ...

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

    if (rClone.m_lChangedTypes.size()>0)
        m_lTypes = rClone.m_lTypes;
    if (rClone.m_lChangedDetectServices.size()>0)
        m_lDetectServices = rClone.m_lDetectServices;
    if (rClone.m_lChangedFilters.size()>0)
        m_lFilters = rClone.m_lFilters;
    if (rClone.m_lChangedFrameLoaders.size()>0)
        m_lFrameLoaders = rClone.m_lFrameLoaders;
    if (rClone.m_lChangedContentHandlers.size()>0)
        m_lContentHandlers = rClone.m_lContentHandlers;

    m_lChangedTypes.clear();
    m_lChangedDetectServices.clear();
    m_lChangedFilters.clear();
    m_lChangedFrameLoaders.clear();
    m_lChangedContentHandlers.clear();

    m_sActLocale     = rClone.m_sActLocale;
    m_sFormatName    = rClone.m_sFormatName;
    m_sFormatVersion = rClone.m_sFormatVersion;

    m_eFillState     = rClone.m_eFillState;

    // renew all dependencies and optimizations
    // Because we cant be shure, that changed filters on one clone
    // and changed types of another clone work together.
    // But here we can check against the lates changes ...
    impl_validateAndOptimize();
    // <- SAFE ----------------------------------
}



void FilterCache::load(EFillState eRequired,
#if OSL_DEBUG_LEVEL > 1
    sal_Bool bByThread
#else
    sal_Bool
#endif
)
    throw(css::uno::Exception)
{
    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // check if required fill state is already reached ...
    // There is nothing to do then.
    if ((m_eFillState & eRequired) == eRequired)
        return;

#if OSL_DEBUG_LEVEL > 1
    if (!bByThread && ((eRequired & E_CONTAINS_ALL) == E_CONTAINS_ALL))
    {
        OSL_FAIL("Who disturb our \"fill cache on demand\" feature and force loading of ALL data during office startup? Please optimize your code, so a full filled filter cache is not realy needed here!");
    }
#endif

    // Otherwise load the missing items.

    // ------------------------------------------
    // a) load some const values from configration.
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

        impl_getDirectCFGValue(CFGDIRECTKEY_FORMATNAME) >>= m_sFormatName;
        if (m_sFormatName.isEmpty())
            impl_getDirectCFGValue(CFGDIRECTKEY_PRODUCTNAME) >>= m_sFormatName;

        if (m_sFormatName.isEmpty())
        {
            _FILTER_CONFIG_LOG_1_("FilterCache::ctor() ... could not specify format name => use default \"%s\"\n", _FILTER_CONFIG_TO_ASCII_(DEFAULT_FORMATNAME));
            m_sFormatName = DEFAULT_FORMATNAME;
        }

        impl_getDirectCFGValue(CFGDIRECTKEY_FORMATVERSION) >>= m_sFormatVersion;
        if (m_sFormatVersion.isEmpty())
        {
            _FILTER_CONFIG_LOG_1_("FilterCache::ctor() ... could not specify format version => use default \"%s\"\n", _FILTER_CONFIG_TO_ASCII_(DEFAULT_FORMATVERSION));
            m_sFormatVersion = DEFAULT_FORMATVERSION;
        }

        // Support the old configuration support. Read it only one times during office runtime!
        impl_readOldFormat();

        // enable "loadOnDemand" feature ...
        // Create uno listener, which waits for finishing the office startup
        // and starts a thread, which calls loadAll() at this filter cache.
        // Note: Its not a leak to create this listener with new here.
        // It kills itself after working!
        /* LateInitListener* pLateInit = */ new LateInitListener(comphelper::getComponentContext(m_xSMGR));
    }

    // ------------------------------------------
    // b) If the required fill state was not reached
    //    but std values was already loaded ...
    //    we must load some further missing items.
    impl_load(eRequired);
    // <- SAFE
}



sal_Bool FilterCache::isFillState(FilterCache::EFillState eState) const
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



sal_Bool FilterCache::hasItems(EItemType eType) const
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



sal_Bool FilterCache::hasItem(      EItemType        eType,
                              const ::rtl::OUString& sItem)
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
        return sal_True;

    try
    {
        impl_loadItemOnDemand(eType, sItem);
        // no exception => item could be loaded!
        return sal_True;
    }
    catch(const css::container::NoSuchElementException&)
    {}

    return sal_False;
    // <- SAFE
}



CacheItem FilterCache::getItem(      EItemType        eType,
                               const ::rtl::OUString& sItem)
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
        // Note: NoSuchElementException is thrown automaticly here if
        // item could not be loaded!
        pIt = impl_loadItemOnDemand(eType, sItem);
    }

    /* Workaround for #137955#
       Draw types and filters are installed ... but draw was disabled during setup.
       We must supress accessing these filters. Otherwise the office can crash.
       Solution for the next major release: do not install those filters !
     */
    if (eType == E_FILTER)
    {
        CacheItem& rFilter = pIt->second;
        ::rtl::OUString sDocService;
        rFilter[PROPNAME_DOCUMENTSERVICE] >>= sDocService;

        // In Standalone-Impress the module WriterWeb is not installed
        // but it is there to load help pages
        bool bIsHelpFilter = sItem == "writer_web_HTML_help";

        if ( !bIsHelpFilter && !impl_isModuleInstalled(sDocService) )
        {
            ::rtl::OUStringBuffer sMsg(256);
            sMsg.appendAscii("The requested filter '"                                                               );
            sMsg.append     (sItem                                                                                  );
            sMsg.appendAscii("' exists ... but it shouldnt; because the corresponding OOo module was not installed.");
            throw css::container::NoSuchElementException(sMsg.makeStringAndClear(), css::uno::Reference< css::uno::XInterface >());
        }
    }

    return pIt->second;
    // <- SAFE
}



void FilterCache::removeItem(      EItemType        eType,
                             const ::rtl::OUString& sItem)
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
                          const ::rtl::OUString& sItem ,
                          const CacheItem&       aValue)
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // search for right list
    // An exception is thrown - "eType" is unknown.
    // => rList will be valid everytimes next line is reached.
    CacheItemList& rList = impl_getItemList(eType);

    // name must be part of the property set too ... otherwhise our
    // container query cant work correctly
    CacheItem aItem = aValue;
    aItem[PROPNAME_NAME] <<= sItem;
    aItem.validateUINames(m_sActLocale);

    // remove implicit properties as e.g. FINALIZED or MANDATORY
    // They cant be saved here and must be readed on demand later, if they are needed.
    removeStatePropsFromItem(aItem);

    rList[sItem] = aItem;

    impl_addItem2FlushList(eType, sItem);
}

//-----------------------------------------------
void FilterCache::refreshItem(      EItemType        eType,
                              const ::rtl::OUString& sItem)
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    impl_loadItemOnDemand(eType, sItem);
}



void FilterCache::addStatePropsToItem(      EItemType        eType,
                                      const ::rtl::OUString& sItem,
                                            CacheItem&       rItem)
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // Note: Opening of the configuration layer throws some exceptions
    // if it failed. So we dont must check any reference here ...
    css::uno::Reference< css::container::XNameAccess > xPackage;
    css::uno::Reference< css::container::XNameAccess > xSet;
    switch(eType)
    {
        case E_TYPE :
            {
                xPackage = css::uno::Reference< css::container::XNameAccess >(impl_openConfig(E_PROVIDER_TYPES), css::uno::UNO_QUERY_THROW);
                xPackage->getByName(CFGSET_TYPES) >>= xSet;
            }
            break;

        case E_FILTER :
            {
                xPackage = css::uno::Reference< css::container::XNameAccess >(impl_openConfig(E_PROVIDER_FILTERS), css::uno::UNO_QUERY_THROW);
                xPackage->getByName(CFGSET_FILTERS) >>= xSet;
            }
            break;

        case E_FRAMELOADER :
            {
                /* TODO
                    Hack -->
                        The default frame loader cant be located inside te normal set of frame loaders.
                        Its an atomic property inside the misc cfg package. So we cant retrieve the information
                        about FINALIZED and MANDATORY very easy ... :-(
                        => set it to readonly/required everytimes :-)
                */
                css::uno::Any   aDirectValue       = impl_getDirectCFGValue(CFGDIRECTKEY_DEFAULTFRAMELOADER);
                ::rtl::OUString sDefaultFrameLoader;
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

                xPackage = css::uno::Reference< css::container::XNameAccess >(impl_openConfig(E_PROVIDER_OTHERS), css::uno::UNO_QUERY_THROW);
                xPackage->getByName(CFGSET_FRAMELOADERS) >>= xSet;
            }
            break;

        case E_CONTENTHANDLER :
            {
                xPackage = css::uno::Reference< css::container::XNameAccess >(impl_openConfig(E_PROVIDER_OTHERS), css::uno::UNO_QUERY_THROW);
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

        sal_Bool bFinalized = ((aDescription.Attributes & css::beans::PropertyAttribute::READONLY  ) == css::beans::PropertyAttribute::READONLY  );
        sal_Bool bMandatory = ((aDescription.Attributes & css::beans::PropertyAttribute::REMOVEABLE) != css::beans::PropertyAttribute::REMOVEABLE);

        rItem[PROPNAME_FINALIZED] <<= bFinalized;
        rItem[PROPNAME_MANDATORY] <<= bMandatory;
    }
    catch(const css::container::NoSuchElementException&)
    {
        /*  Ignore exceptions for missing elements inside configuration.
            May by the following reason exists:
                -   The item does not exists inside the new configuration package org.openoffice.TypeDetection - but
                    we got it from the old package org.openoffice.Office/TypeDetection. We dont migrate such items
                    automaticly to the new format. Because it will disturb e.g. the deinstallation of an external filter
                    package. Because such external filter can remove the old file - but not the automaticly created new one ...

            => mark item as FINALIZED / MANDATORY, we dont support writing to the old format
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
    css::uno::Reference< css::container::XNameContainer >   xAddRemoveSet = css::uno::Reference< css::container::XNameContainer >  (xSet, css::uno::UNO_QUERY);
    css::uno::Reference< css::container::XNameReplace >     xReplaceeSet  = css::uno::Reference< css::container::XNameReplace >    (xSet, css::uno::UNO_QUERY);
    css::uno::Reference< css::lang::XSingleServiceFactory > xFactory      = css::uno::Reference< css::lang::XSingleServiceFactory >(xSet, css::uno::UNO_QUERY);

    for (OUStringList::const_iterator pIt  = lItems.begin();
                                      pIt != lItems.end()  ;
                                    ++pIt                  )
    {
        const ::rtl::OUString& sItem  = *pIt;
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
                    throw css::uno::Exception(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Cant add item. Set is finalized or mandatory!" )),
                                              css::uno::Reference< css::uno::XInterface >()                                   );

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
                    throw css::uno::Exception(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Cant change item. Its finalized or mandatory!" )),
                                              css::uno::Reference< css::uno::XInterface >()                                    );

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
    ::rtl::OUString sExtension = aParser.getExtension(INetURLObject::LAST_SEGMENT       ,
                                                      sal_True                          ,
                                                      INetURLObject::DECODE_WITH_CHARSET);
    sExtension = sExtension.toAsciiLowerCase();

    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    //*******************************************
    // i) Step over all well known URL pattern
    //    and add registered types to the return list too
    //    Do it as first one - because: if a type match by a
    //    pattern a following deep detection can be supressed!
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
            aInfo.bMatchByPattern = sal_True;

            rFlatTypes.push_back(aInfo);
//          return;
        }
    }

    //*******************************************
    // ii) search types matching to the given extension.
    //     Copy every macthing type without changing its order!
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
            aInfo.bMatchByExtension = sal_True;

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
        case E_DETECTSERVICE  : return m_lDetectServices ;

    }

    throw css::uno::Exception(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "unknown sub container requested." )),
                                            css::uno::Reference< css::uno::XInterface >()                      );
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
        case E_DETECTSERVICE  : return m_lDetectServices ;

    }

    throw css::uno::Exception(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "unknown sub container requested." )),
                                            css::uno::Reference< css::uno::XInterface >()                      );
    // <- SAFE ----------------------------------
}

css::uno::Reference< css::uno::XInterface > FilterCache::impl_openConfig(EConfigProvider eProvider)
    throw(css::uno::Exception)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    ::rtl::OUString                              sPath      ;
    css::uno::Reference< css::uno::XInterface >* pConfig = 0;
    css::uno::Reference< css::uno::XInterface >  xOld       ;
    ::rtl::OString                               sRtlLog    ;

    switch(eProvider)
    {
        case E_PROVIDER_TYPES :
        {
            if (m_xConfigTypes.is())
                return m_xConfigTypes;
            sPath           = CFGPACKAGE_TD_TYPES;
            pConfig         = &m_xConfigTypes;
            sRtlLog         = ::rtl::OString("framework (as96863) ::FilterCache::impl_openconfig(E_PROVIDER_TYPES)");
        }
        break;

        case E_PROVIDER_FILTERS :
        {
            if (m_xConfigFilters.is())
                return m_xConfigFilters;
            sPath           = CFGPACKAGE_TD_FILTERS;
            pConfig         = &m_xConfigFilters;
            sRtlLog         = ::rtl::OString("framework (as96863) ::FilterCache::impl_openconfig(E_PROVIDER_FILTERS)");
        }
        break;

        case E_PROVIDER_OTHERS :
        {
            if (m_xConfigOthers.is())
                return m_xConfigOthers;
            sPath   = CFGPACKAGE_TD_OTHERS;
            pConfig = &m_xConfigOthers;
            sRtlLog = ::rtl::OString("framework (as96863) ::FilterCache::impl_openconfig(E_PROVIDER_OTHERS)");
        }
        break;

        case E_PROVIDER_OLD :
        {
            // This special provider is used to work with
            // the old configuration format only. Its not cached!
            sPath   = CFGPACKAGE_TD_OLD;
            pConfig = &xOld;
            sRtlLog = ::rtl::OString("framework (as96863) ::FilterCache::impl_openconfig(E_PROVIDER_OLD)");
        }
        break;

        default : throw css::uno::Exception(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "These configuration node isnt supported here for open!" )), 0);
    }

    {
        RTL_LOGFILE_CONTEXT(aLog, sRtlLog.getStr());
        *pConfig = impl_createConfigAccess(sPath    ,
                                           sal_False,   // bReadOnly
                                           sal_True );  // bLocalesMode
    }


    // Start listening for changes on that configuration access.
    switch(eProvider)
    {
        case E_PROVIDER_TYPES:
        {
            m_xTypesChglisteners.set(new CacheUpdateListener(m_xSMGR, *this, *pConfig, FilterCache::E_TYPE));
            m_xTypesChglisteners->startListening();
        }
        break;
        case E_PROVIDER_FILTERS:
        {
            m_xFiltersChgListener.set(new CacheUpdateListener(m_xSMGR, *this, *pConfig, FilterCache::E_FILTER));
            m_xFiltersChgListener->startListening();
        }
        break;
        default:
        break;
    }

    return *pConfig;
}



css::uno::Any FilterCache::impl_getDirectCFGValue(const ::rtl::OUString& sDirectKey)
{
    ::rtl::OUString sRoot;
    ::rtl::OUString sKey ;

    if (
        (!::utl::splitLastFromConfigurationPath(sDirectKey, sRoot, sKey)) ||
        (sRoot.isEmpty()                                             ) ||
        (sKey.isEmpty()                                              )
       )
        return css::uno::Any();

    css::uno::Reference< css::uno::XInterface > xCfg = impl_createConfigAccess(sRoot    ,
                                                                               sal_True ,  // bReadOnly
                                                                               sal_False); // bLocalesMode
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
            OSL_FAIL(::rtl::OUStringToOString(ex.Message, RTL_TEXTENCODING_UTF8).getStr());
            #endif
            aValue.clear();
        }

    return aValue;
}



css::uno::Reference< css::uno::XInterface > FilterCache::impl_createConfigAccess(const ::rtl::OUString& sRoot       ,
                                                                                       sal_Bool         bReadOnly   ,
                                                                                       sal_Bool         bLocalesMode)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    css::uno::Reference< css::uno::XInterface > xCfg;

    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
            css::configuration::theDefaultProvider::get(
                comphelper::getComponentContext(m_xSMGR)));

        ::comphelper::SequenceAsVector< css::uno::Any > lParams;
        css::beans::NamedValue aParam;

        // set root path
        aParam.Name    = _FILTER_CONFIG_FROM_ASCII_("nodepath");
        aParam.Value <<= sRoot;
        lParams.push_back(css::uno::makeAny(aParam));

        // enable "all locales mode" ... if required
        if (bLocalesMode)
        {
            aParam.Name    = _FILTER_CONFIG_FROM_ASCII_("locale");
            aParam.Value <<= _FILTER_CONFIG_FROM_ASCII_("*"     );
            lParams.push_back(css::uno::makeAny(aParam));
        }

        // open it
        if (bReadOnly)
            xCfg = xConfigProvider->createInstanceWithArguments(SERVICE_CONFIGURATIONACCESS, lParams.getAsConstList());
        else
            xCfg = xConfigProvider->createInstanceWithArguments(SERVICE_CONFIGURATIONUPDATEACCESS, lParams.getAsConstList());

        // If configuration could not be opened ... but factory method does not throwed an exception
        // trigger throwing of our own CorruptedFilterConfigurationException.
        // Let message empty. The normal exception text show enough informations to the user.
        if (! xCfg.is())
            throw css::uno::Exception(
                    _FILTER_CONFIG_FROM_ASCII_("Got NULL reference on opening configuration file ... but no exception."),
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

    RTL_LOGFILE_CONTEXT( aLog, "framework (as96863) ::FilterCache::impl_validateAndOptimize");

    // First check if any filter or type could be readed
    // from the underlying configuration!
    sal_Bool bSomeTypesShouldExist   = ((m_eFillState & E_CONTAINS_STANDARD       ) == E_CONTAINS_STANDARD       );
    sal_Bool bAllFiltersShouldExist  = ((m_eFillState & E_CONTAINS_FILTERS        ) == E_CONTAINS_FILTERS        );

#if OSL_DEBUG_LEVEL > 0

    sal_Int32             nWarnings = 0;

//  sal_Bool bAllTypesShouldExist    = ((m_eFillState & E_CONTAINS_TYPES          ) == E_CONTAINS_TYPES          );
    sal_Bool bAllLoadersShouldExist  = ((m_eFillState & E_CONTAINS_FRAMELOADERS   ) == E_CONTAINS_FRAMELOADERS   );
    sal_Bool bAllHandlersShouldExist = ((m_eFillState & E_CONTAINS_CONTENTHANDLERS) == E_CONTAINS_CONTENTHANDLERS);
#endif

    if (
        (
            (bSomeTypesShouldExist) &&
            (m_lTypes.size() < 1  )
        ) ||
        (
            (bAllFiltersShouldExist) &&
            (m_lFilters.size() < 1 )
        )
       )
    {
        throw css::document::CorruptedFilterConfigurationException(
                "filter configuration: the list of types or filters is empty",
                css::uno::Reference< css::uno::XInterface >(),
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "The list of types or filters is empty." )));
    }

    // Create a log for all detected problems, which
    // occur in the next few lines.
    // If there are some real errors throw a RuntimException!
    // If there are some warnings only, show an assertion.
    sal_Int32             nErrors   = 0;
    ::rtl::OUStringBuffer sLog(256);

    CacheItemList::iterator pIt;

    for (pIt = m_lTypes.begin(); pIt != m_lTypes.end(); ++pIt)
    {
        ::rtl::OUString sType = pIt->first;
        CacheItem       aType = pIt->second;

        // create list of all known detect services / frame loader / content handler on demand
        // Because these informations are available as type properties!
        ::rtl::OUString sDetectService;
        aType[PROPNAME_DETECTSERVICE ] >>= sDetectService;
        if (!sDetectService.isEmpty())
            impl_resolveItem4TypeRegistration(&m_lDetectServices, sDetectService, sType);

        // get its registration for file Extensions AND(!) URLPattern ...
        // It doesnt matter if these items exists or if our
        // used index access create some default ones ...
        // only in case there is no filled set of Extensions AND
        // no filled set of URLPattern -> we must try to remove this invalid item
        // from this cache!
        css::uno::Sequence< ::rtl::OUString > lExtensions;
        css::uno::Sequence< ::rtl::OUString > lURLPattern;
        aType[PROPNAME_EXTENSIONS] >>= lExtensions;
        aType[PROPNAME_URLPATTERN] >>= lURLPattern;
        sal_Int32 ce = lExtensions.getLength();
        sal_Int32 cu = lURLPattern.getLength();

#if OSL_DEBUG_LEVEL > 0

        ::rtl::OUString sInternalTypeNameCheck;
        aType[PROPNAME_NAME] >>= sInternalTypeNameCheck;
        if (!sInternalTypeNameCheck.equals(sType))
        {
            sLog.appendAscii("Warning\t:\t");
            sLog.appendAscii("The type \"" );
            sLog.append     (sType         );
            sLog.appendAscii("\" does support the property \"Name\" correctly.\n");
            ++nWarnings;
        }

        if (!ce && !cu)
        {
            sLog.appendAscii("Warning\t:\t");
            sLog.appendAscii("The type \"" );
            sLog.append     (sType         );
            sLog.appendAscii("\" does not contain any URL pattern nor any extensions.\n");
            ++nWarnings;
        }
#endif

        // create an optimized registration for this type to
        // its set list of extensions/url pattern. If its a "normal" type
        // set it at the end of this optimized list. But if its
        // a "Preferred" one - set it to the front of this list.
        // Of course multiple "Preferred" registrations can occur
        // (they shouldn't - but they can!) ... Ignore it. The last
        // preferred type is useable in the same manner then every
        // other type!
        sal_Bool bPreferred = sal_False;
        aType[PROPNAME_PREFERRED] >>= bPreferred;

        const ::rtl::OUString* pExtensions = lExtensions.getConstArray();
        for (sal_Int32 e=0; e<ce; ++e)
        {
            // Note: We must be sure that address the right hash entry
            // does not depend from any upper/lower case problems ...
            ::rtl::OUString sNormalizedExtension = pExtensions[e].toAsciiLowerCase();

            OUStringList& lTypesForExtension = m_lExtensions2Types[sNormalizedExtension];
            if (::std::find(lTypesForExtension.begin(), lTypesForExtension.end(), sType) != lTypesForExtension.end())
                continue;

            if (bPreferred)
                lTypesForExtension.insert(lTypesForExtension.begin(), sType);
            else
                lTypesForExtension.push_back(sType);
        }

        const ::rtl::OUString* pURLPattern = lURLPattern.getConstArray();
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

        // Dont check cross references between types and filters, if
        // not all filters read from disk!
        // OK - this cache can read single filters on demand too ...
        // but then the fill state of this cache shouldnt be set to E_CONTAINS_FILTERS!
        if (!bAllFiltersShouldExist)
            continue;

        ::rtl::OUString sPrefFilter;
        aType[PROPNAME_PREFERREDFILTER] >>= sPrefFilter;
        if (sPrefFilter.isEmpty())
        {
            // OK - there is no filter for this type. But thats not an error.
            // May be it can be handled by a ContentHandler ...
            // But at this time its not guaranteed that there is any ContentHandler
            // or FrameLoader inside this cache ... but on disk ...
            sal_Bool bReferencedByLoader  = sal_True;
            sal_Bool bReferencedByHandler = sal_True;
            if (bAllLoadersShouldExist)
                bReferencedByLoader = !impl_searchFrameLoaderForType(sType).isEmpty();

            if (bAllHandlersShouldExist)
                bReferencedByHandler = !impl_searchContentHandlerForType(sType).isEmpty();

            if (
                (!bReferencedByLoader ) &&
                (!bReferencedByHandler)
               )
            {
                sLog.appendAscii("Warning\t:\t"                                            );
                sLog.appendAscii("The type \""                                             );
                sLog.append     (sType                                                     );
                sLog.appendAscii("\" isnt used by any filter, loader or content handler.\n");
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
                    sLog.appendAscii("error\t:\t");
                }
                else
                {
                    ++nWarnings;
                    sLog.appendAscii("warning\t:\t");
                }

                sLog.appendAscii("The type \""                      );
                sLog.append     (sType                              );
                sLog.appendAscii("\" points to an invalid filter \"");
                sLog.append     (sPrefFilter                        );
                sLog.appendAscii("\".\n"                            );

                continue;
            }

            CacheItem       aPrefFilter   = pIt2->second;
            ::rtl::OUString sFilterTypeReg;
            aPrefFilter[PROPNAME_TYPE] >>= sFilterTypeReg;
            if (sFilterTypeReg != sType)
            {
                sLog.appendAscii("error\t:\t"                       );
                sLog.appendAscii("The preferred filter \""          );
                sLog.append     (sPrefFilter                        );
                sLog.appendAscii("\" of type \""                    );
                sLog.append     (sType                              );
                sLog.appendAscii("is registered for another type \"");
                sLog.append     (sFilterTypeReg                     );
                sLog.appendAscii("\".\n"                            );
                ++nErrors;
            }

            sal_Int32 nFlags = 0;
            aPrefFilter[PROPNAME_FLAGS] >>= nFlags;
            if ((nFlags & FLAGVAL_IMPORT) != FLAGVAL_IMPORT)
            {
                sLog.appendAscii("error\t:\t"                   );
                sLog.appendAscii("The preferred filter \""      );
                sLog.append     (sPrefFilter                    );
                sLog.appendAscii("\" of type \""                );
                sLog.append     (sType                          );
                sLog.appendAscii("\" is not an IMPORT filter!\n");
                ++nErrors;
            }

            ::rtl::OUString sInternalFilterNameCheck;
            aPrefFilter[PROPNAME_NAME] >>= sInternalFilterNameCheck;
            if (!sInternalFilterNameCheck.equals(sPrefFilter))
            {
                sLog.appendAscii("Warning\t:\t"  );
                sLog.appendAscii("The filter \"" );
                sLog.append     (sPrefFilter     );
                sLog.appendAscii("\" does support the property \"Name\" correctly.\n");
                ++nWarnings;
            }
        }
#endif
    }

    // create dependencies between the global default frame loader
    // and all types (and of course if registered filters), which
    // does not registered for any other loader.
    css::uno::Any   aDirectValue       = impl_getDirectCFGValue(CFGDIRECTKEY_DEFAULTFRAMELOADER);
    ::rtl::OUString sDefaultFrameLoader;

    if (
        (!(aDirectValue >>= sDefaultFrameLoader)) ||
        (sDefaultFrameLoader.isEmpty()       )
       )
    {
        sLog.appendAscii("error\t:\t"                                );
        sLog.appendAscii("There is no valid default frame loader!?\n");
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
        // Because we replace its registration later completly with all
        // types, which are not referenced by any other loader.
        // So we can avaoid our code against the complexity of a diff!
        ::rtl::OUString sLoader = pIt->first;
        if (sLoader.equals(sDefaultFrameLoader))
            continue;

        CacheItem&     rLoader   = pIt->second;
        css::uno::Any& rTypesReg = rLoader[PROPNAME_TYPES];
        OUStringList   lTypesReg (rTypesReg);

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
    rDefaultLoader[PROPNAME_TYPES] <<= lTypes.getAsConstList();

    ::rtl::OUString sLogOut = sLog.makeStringAndClear();
    OSL_ENSURE(!nErrors, ::rtl::OUStringToOString(sLogOut,RTL_TEXTENCODING_UTF8).getStr());
    if (nErrors>0)
        throw css::document::CorruptedFilterConfigurationException(
                "filter configuration: " + sLogOut,
                css::uno::Reference< css::uno::XInterface >(),
                sLogOut);
    OSL_ENSURE(!nWarnings, ::rtl::OUStringToOString(sLogOut,RTL_TEXTENCODING_UTF8).getStr());

    // <- SAFE
}



void FilterCache::impl_addItem2FlushList(      EItemType        eType,
                                         const ::rtl::OUString& sItem)
    throw(css::uno::Exception)
{
    OUStringList* pList = 0;
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

        case E_DETECTSERVICE :
                pList = &m_lChangedDetectServices;
                break;

        default : throw css::uno::Exception(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "unsupported item type" )), 0);
    }

    OUStringList::const_iterator pItem = ::std::find(pList->begin(), pList->end(), sItem);
    if (pItem == pList->end())
        pList->push_back(sItem);
}



FilterCache::EItemFlushState FilterCache::impl_specifyFlushOperation(const css::uno::Reference< css::container::XNameAccess >& xSet ,
                                                                     const CacheItemList&                                      rList,
                                                                     const ::rtl::OUString&                                    sItem)
    throw(css::uno::Exception)
{
    sal_Bool bExistsInConfigLayer = xSet->hasByName(sItem);
    sal_Bool bExistsInMemory      = (rList.find(sItem) != rList.end());

    EItemFlushState eState( E_ITEM_UNCHANGED );

    // !? ... such situation can occur, if an item was added and(!) removed before it was flushed :-)
    if (!bExistsInConfigLayer && !bExistsInMemory)
        eState = E_ITEM_UNCHANGED;
    else
    if (!bExistsInConfigLayer && bExistsInMemory)
        eState = E_ITEM_ADDED;
    else
    if (bExistsInConfigLayer && bExistsInMemory)
        eState = E_ITEM_CHANGED;
    else
    if (bExistsInConfigLayer && !bExistsInMemory)
        eState = E_ITEM_REMOVED;

    return eState;
}



void FilterCache::impl_resolveItem4TypeRegistration(      CacheItemList*   pList,
                                                    const ::rtl::OUString& sItem,
                                                    const ::rtl::OUString& sType)
    throw(css::uno::Exception)
{
    CacheItem& rItem = (*pList)[sItem];
    // In case its a new created entry (automaticly done by the boost::unordered_map index operator!)
    // we must be shure, that this entry has its own name as property available.
    // Its needed later at our container interface!
    rItem[PROPNAME_NAME] <<= sItem;

    OUStringList lTypeRegs(rItem[PROPNAME_TYPES]);
    if (::std::find(lTypeRegs.begin(), lTypeRegs.end(), sType) == lTypeRegs.end())
    {
        lTypeRegs.push_back(sType);
        rItem[PROPNAME_TYPES] <<= lTypeRegs.getAsConstList();
    }
}



void FilterCache::impl_load(EFillState eRequiredState)
    throw(css::uno::Exception)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // Attention: Detect services are part of the standard set!
    // So there is no need to handle it seperatly.

    // ------------------------------------------
    // a) The standard set of config value is needed.
    if (
        ((eRequiredState & E_CONTAINS_STANDARD) == E_CONTAINS_STANDARD) &&
        ((m_eFillState   & E_CONTAINS_STANDARD) != E_CONTAINS_STANDARD)
       )
    {
        // Attention! If config couldnt be opened successfully
        // and exception os thrown automaticly and must be forwarded
        // to our calli ...
        css::uno::Reference< css::container::XNameAccess > xTypes(impl_openConfig(E_PROVIDER_TYPES), css::uno::UNO_QUERY_THROW);
        {
            RTL_LOGFILE_CONTEXT( aLog, "framework (as96863) ::FilterCache::load std");
            impl_loadSet(xTypes, E_TYPE, E_READ_STANDARD, &m_lTypes);
        }
    }

    // ------------------------------------------
    // b) We need all type informations ...
    if (
        ((eRequiredState & E_CONTAINS_TYPES) == E_CONTAINS_TYPES) &&
        ((m_eFillState   & E_CONTAINS_TYPES) != E_CONTAINS_TYPES)
       )
    {
        // Attention! If config couldnt be opened successfully
        // and exception os thrown automaticly and must be forwarded
        // to our calli ...
        css::uno::Reference< css::container::XNameAccess > xTypes(impl_openConfig(E_PROVIDER_TYPES), css::uno::UNO_QUERY_THROW);
        {
            RTL_LOGFILE_CONTEXT( aLog, "framework (as96863) ::FilterCache::load all types");
            impl_loadSet(xTypes, E_TYPE, E_READ_UPDATE, &m_lTypes);
        }
    }

    // ------------------------------------------
    // c) We need all filter informations ...
    if (
        ((eRequiredState & E_CONTAINS_FILTERS) == E_CONTAINS_FILTERS) &&
        ((m_eFillState   & E_CONTAINS_FILTERS) != E_CONTAINS_FILTERS)
       )
    {
        // Attention! If config couldnt be opened successfully
        // and exception os thrown automaticly and must be forwarded
        // to our calli ...
        css::uno::Reference< css::container::XNameAccess > xFilters(impl_openConfig(E_PROVIDER_FILTERS), css::uno::UNO_QUERY_THROW);
        {
            RTL_LOGFILE_CONTEXT( aLog, "framework (as96863) ::FilterCache::load all filters");
            impl_loadSet(xFilters, E_FILTER, E_READ_ALL, &m_lFilters);
        }
    }

    // ------------------------------------------
    // c) We need all frame loader informations ...
    if (
        ((eRequiredState & E_CONTAINS_FRAMELOADERS) == E_CONTAINS_FRAMELOADERS) &&
        ((m_eFillState   & E_CONTAINS_FRAMELOADERS) != E_CONTAINS_FRAMELOADERS)
       )
    {
        // Attention! If config couldnt be opened successfully
        // and exception os thrown automaticly and must be forwarded
        // to our calli ...
        css::uno::Reference< css::container::XNameAccess > xLoaders(impl_openConfig(E_PROVIDER_OTHERS), css::uno::UNO_QUERY_THROW);
        {
            RTL_LOGFILE_CONTEXT( aLog, "framework (as96863) ::FilterCache::load all frame loader");
            impl_loadSet(xLoaders, E_FRAMELOADER, E_READ_ALL, &m_lFrameLoaders);
        }
    }

    // ------------------------------------------
    // d) We need all content handler informations ...
    if (
        ((eRequiredState & E_CONTAINS_CONTENTHANDLERS) == E_CONTAINS_CONTENTHANDLERS) &&
        ((m_eFillState   & E_CONTAINS_CONTENTHANDLERS) != E_CONTAINS_CONTENTHANDLERS)
       )
    {
        // Attention! If config couldnt be opened successfully
        // and exception os thrown automaticly and must be forwarded
        // to our calli ...
        css::uno::Reference< css::container::XNameAccess > xHandlers(impl_openConfig(E_PROVIDER_OTHERS), css::uno::UNO_QUERY_THROW);
        {
            RTL_LOGFILE_CONTEXT( aLog, "framework (as96863) ::FilterCache::load all content handler");
            impl_loadSet(xHandlers, E_CONTENTHANDLER, E_READ_ALL, &m_lContentHandlers);
        }
    }

    // update fill state. Note: its a bit field, which combines different parts.
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
    ::rtl::OUString sSetName;
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
    css::uno::Sequence< ::rtl::OUString >              lItems;

    try
    {
        css::uno::Any aVal = xConfig->getByName(sSetName);
        if (!(aVal >>= xSet) || !xSet.is())
        {
            ::rtl::OUStringBuffer sMsg(256);
            sMsg.appendAscii("Could not open configuration set \"");
            sMsg.append     (sSetName                             );
            sMsg.appendAscii("\"."                                );
            throw css::uno::Exception(
                    sMsg.makeStringAndClear(),
                    css::uno::Reference< css::uno::XInterface >());
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

    // But dont update optimized structures like e.g. hash
    // for mapping extensions to its types!

    const ::rtl::OUString* pItems = lItems.getConstArray();
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
                    ::rtl::OUStringBuffer sMsg(256);
                    sMsg.appendAscii("item \""                 );
                    sMsg.append     (pItems[i]                 );
                    sMsg.appendAscii("\" not found for update!");
                    throw css::uno::Exception(sMsg.makeStringAndClear()                    ,
                                              css::uno::Reference< css::uno::XInterface >());
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
    ::rtl::OUString sFormatName    = m_sFormatName   ;
    ::rtl::OUString sFormatVersion = m_sFormatVersion;
    ::rtl::OUString sActLocale     = m_sActLocale    ;
    aLock.clear();
    // <- SAFE ----------------------------------

    css::uno::Any aVal = xNode->getByName(PROPNAME_UINAME);
    css::uno::Reference< css::container::XNameAccess > xUIName;
    if (!(aVal >>= xUIName) && !xUIName.is())
        return;

    const ::comphelper::SequenceAsVector< ::rtl::OUString >                 lLocales(xUIName->getElementNames());
          ::comphelper::SequenceAsVector< ::rtl::OUString >::const_iterator pLocale ;
          ::comphelper::SequenceAsHashMap                                   lUINames;

    const char FORMATNAME_VAR[] = "%productname%";
    const char FORMATVERSION_VAR[] = "%formatversion%";
    // patch %PRODUCTNAME and %FORMATNAME
    for (  pLocale  = lLocales.begin();
           pLocale != lLocales.end()  ;
         ++pLocale                    )
    {
        const ::rtl::OUString& sLocale = *pLocale;

        ::rtl::OUString sValue;
        xUIName->getByName(sLocale) >>= sValue;

        // replace %productname%
        sal_Int32 nIndex = sValue.indexOf(FORMATNAME_VAR);
        while(nIndex != -1)
        {
            sValue = sValue.replaceAt(nIndex, RTL_CONSTASCII_LENGTH(FORMATNAME_VAR), sFormatName);
            nIndex = sValue.indexOf(FORMATNAME_VAR, nIndex);
        }
        // replace %formatversion%
        nIndex = sValue.indexOf(FORMATVERSION_VAR);
        while(nIndex != -1)
        {
            sValue = sValue.replaceAt(nIndex, RTL_CONSTASCII_LENGTH(FORMATVERSION_VAR), sFormatVersion);
            nIndex = sValue.indexOf(FORMATVERSION_VAR, nIndex);
        }

        lUINames[sLocale] <<= sValue;
    }

    aVal <<= lUINames.getAsConstPropertyValueList();
    rItem[PROPNAME_UINAMES] = aVal;

    // find right UIName for current office locale
    // Use fallbacks too!
    pLocale = ::comphelper::Locale::getFallback(lLocales, sActLocale);
    if (pLocale == lLocales.end())
    {
#if OSL_DEBUG_LEVEL > 0
        if ( sActLocale == "en-US" )
            return;
        ::rtl::OUString sName = rItem.getUnpackedValueOrDefault(PROPNAME_NAME, ::rtl::OUString());

        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("Fallback scenario for filter or type '"           );
        sMsg.append     (sName                                              );
        sMsg.appendAscii("' and locale '"                                   );
        sMsg.append     (sActLocale                                         );
        sMsg.appendAscii("' failed. Please check your filter configuration.");

        OSL_FAIL(_FILTER_CONFIG_TO_ASCII_(sMsg.makeStringAndClear()));
#endif
        return;
    }

    const ::rtl::OUString& sLocale = *pLocale;
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
                                     const ::rtl::OUString&                                    sItem  ,
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
            ::rtl::OUStringBuffer sMsg(256);
            sMsg.appendAscii("found corrupted item \"");
            sMsg.append     (sItem                    );
            sMsg.appendAscii("\"."                    );
            throw css::uno::Exception(sMsg.makeStringAndClear()                    ,
                                      css::uno::Reference< css::uno::XInterface >());
        }
    #ifdef WORKAROUND_EXCEPTION_PROBLEM
    }
    catch(const css::container::NoSuchElementException&)
    {
        throw;
    }
    #endif

    // The internal name of an item must(!) be part of the property
    // set too. Of course its already used as key into the e.g. outside
    // used hash map ... but some of our API methods provide
    // this property set as result only. But the user of this CacheItem
    // should know, which value the key names has :-) ITS IMPORTANT!
    CacheItem aItem;
    aItem[PROPNAME_NAME] = css::uno::makeAny(sItem);
    switch(eType)
    {
        //---------------------------------------
        case E_TYPE :
        {
            // read standard properties of a type
            if (
                (eOption == E_READ_STANDARD) ||
                (eOption == E_READ_ALL     )
               )
            {
                aItem[PROPNAME_PREFERREDFILTER] = xItem->getByName(PROPNAME_PREFERREDFILTER);
                aItem[PROPNAME_DETECTSERVICE  ] = xItem->getByName(PROPNAME_DETECTSERVICE  );
                aItem[PROPNAME_URLPATTERN     ] = xItem->getByName(PROPNAME_URLPATTERN     );
                aItem[PROPNAME_EXTENSIONS     ] = xItem->getByName(PROPNAME_EXTENSIONS     );
                aItem[PROPNAME_PREFERRED      ] = xItem->getByName(PROPNAME_PREFERRED      );
                aItem[PROPNAME_CLIPBOARDFORMAT] = xItem->getByName(PROPNAME_CLIPBOARDFORMAT);
            }
            // read optional properties of a type
            // no else here! Is an additional switch ...
            if (
                (eOption == E_READ_UPDATE) ||
                (eOption == E_READ_ALL   )
               )
            {
                aItem[PROPNAME_MEDIATYPE      ] = xItem->getByName(PROPNAME_MEDIATYPE      );
                impl_readPatchUINames(xItem, aItem);
            }
        }
        break;

        //---------------------------------------
        case E_FILTER :
        {
            // read standard properties of a filter
            if (
                (eOption == E_READ_STANDARD) ||
                (eOption == E_READ_ALL     )
               )
            {
                aItem[PROPNAME_TYPE             ] = xItem->getByName(PROPNAME_TYPE             );
                aItem[PROPNAME_FILEFORMATVERSION] = xItem->getByName(PROPNAME_FILEFORMATVERSION);
                aItem[PROPNAME_UICOMPONENT      ] = xItem->getByName(PROPNAME_UICOMPONENT      );
                aItem[PROPNAME_FILTERSERVICE    ] = xItem->getByName(PROPNAME_FILTERSERVICE    );
                aItem[PROPNAME_DOCUMENTSERVICE  ] = xItem->getByName(PROPNAME_DOCUMENTSERVICE  );
                aItem[PROPNAME_EXPORTEXTENSION  ] = xItem->getByName(PROPNAME_EXPORTEXTENSION  );

                // special handling for flags! Convert it from a list of names to its
                // int representation ...
                css::uno::Sequence< ::rtl::OUString > lFlagNames;
                if (xItem->getByName(PROPNAME_FLAGS) >>= lFlagNames)
                    aItem[PROPNAME_FLAGS] <<= FilterCache::impl_convertFlagNames2FlagField(lFlagNames);
            }
            // read optional properties of a filter
            // no else here! Is an additional switch ...
            if (
                (eOption == E_READ_UPDATE) ||
                (eOption == E_READ_ALL   )
               )
            {
                aItem[PROPNAME_USERDATA    ] = xItem->getByName(PROPNAME_USERDATA    );
                aItem[PROPNAME_TEMPLATENAME] = xItem->getByName(PROPNAME_TEMPLATENAME);
//TODO remove it if moving of filter uinames to type uinames
//       will be finished realy
#ifdef AS_ENABLE_FILTER_UINAMES
                impl_readPatchUINames(xItem, aItem);
#endif // AS_ENABLE_FILTER_UINAMES
            }
        }
        break;

        //---------------------------------------
        case E_FRAMELOADER :
        case E_CONTENTHANDLER :
        {
            aItem[PROPNAME_TYPES] = xItem->getByName(PROPNAME_TYPES);
        }
        break;
        default: break;
    }

    return aItem;
}



CacheItemList::iterator FilterCache::impl_loadItemOnDemand(      EItemType        eType,
                                                           const ::rtl::OUString& sItem)
    throw(css::uno::Exception)
{
    CacheItemList*                              pList   = 0;
    css::uno::Reference< css::uno::XInterface > xConfig    ;
    ::rtl::OUString                             sSet       ;

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

        case E_DETECTSERVICE :
        {
            OSL_FAIL("Cant load detect services on demand. Who use this unsupported feature?");
        }
        break;
    }

    css::uno::Reference< css::container::XNameAccess > xRoot(xConfig, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNameAccess > xSet ;
    xRoot->getByName(sSet) >>= xSet;

    CacheItemList::iterator pItemInCache  = pList->find(sItem);
    sal_Bool                bItemInConfig = xSet->hasByName(sItem);

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
                                const CacheItem&                                           aItem)
    throw(css::uno::Exception)
{
    CacheItem::const_iterator pIt;
    switch(eType)
    {
        //---------------------------------------
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

        //---------------------------------------
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
            // its not allowd to change the value here. We must work on a copy!
            pIt = aItem.find(PROPNAME_FLAGS);
            if (pIt != aItem.end())
            {
                sal_Int32 nFlags = 0;
                pIt->second >>= nFlags;
                css::uno::Any aFlagNameList;
                aFlagNameList <<= FilterCache::impl_convertFlagField2FlagNames(nFlags);
                xItem->replaceByName(PROPNAME_FLAGS, aFlagNameList);
            }

//TODO remove it if moving of filter uinames to type uinames
//       will be finished realy
#ifdef AS_ENABLE_FILTER_UINAMES
            css::uno::Reference< css::container::XNameReplace > xUIName;
            xItem->getByName(PROPNAME_UINAME) >>= xUIName;
            impl_savePatchUINames(xUIName, aItem);
#endif //  AS_ENABLE_FILTER_UINAMES
        }
        break;

        //---------------------------------------
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
    static! => no locks neccessary
-----------------------------------------------*/
css::uno::Sequence< ::rtl::OUString > FilterCache::impl_convertFlagField2FlagNames(sal_Int32 nFlags)
{
    OUStringList lFlagNames;

    if ((nFlags & FLAGVAL_3RDPARTYFILTER   ) == FLAGVAL_3RDPARTYFILTER   ) lFlagNames.push_back(FLAGNAME_3RDPARTYFILTER   );
    if ((nFlags & FLAGVAL_ALIEN            ) == FLAGVAL_ALIEN            ) lFlagNames.push_back(FLAGNAME_ALIEN            );
    if ((nFlags & FLAGVAL_ASYNCHRON        ) == FLAGVAL_ASYNCHRON        ) lFlagNames.push_back(FLAGNAME_ASYNCHRON        );
    if ((nFlags & FLAGVAL_BROWSERPREFERRED ) == FLAGVAL_BROWSERPREFERRED ) lFlagNames.push_back(FLAGNAME_BROWSERPREFERRED );
    if ((nFlags & FLAGVAL_CONSULTSERVICE   ) == FLAGVAL_CONSULTSERVICE   ) lFlagNames.push_back(FLAGNAME_CONSULTSERVICE   );
    if ((nFlags & FLAGVAL_DEFAULT          ) == FLAGVAL_DEFAULT          ) lFlagNames.push_back(FLAGNAME_DEFAULT          );
    if ((nFlags & FLAGVAL_ENCRYPTION       ) == FLAGVAL_ENCRYPTION       ) lFlagNames.push_back(FLAGNAME_ENCRYPTION       );
    if ((nFlags & FLAGVAL_EXPORT           ) == FLAGVAL_EXPORT           ) lFlagNames.push_back(FLAGNAME_EXPORT           );
    if ((nFlags & FLAGVAL_IMPORT           ) == FLAGVAL_IMPORT           ) lFlagNames.push_back(FLAGNAME_IMPORT           );
    if ((nFlags & FLAGVAL_INTERNAL         ) == FLAGVAL_INTERNAL         ) lFlagNames.push_back(FLAGNAME_INTERNAL         );
    if ((nFlags & FLAGVAL_NOTINCHOOSER     ) == FLAGVAL_NOTINCHOOSER     ) lFlagNames.push_back(FLAGNAME_NOTINCHOOSER     );
    if ((nFlags & FLAGVAL_NOTINFILEDIALOG  ) == FLAGVAL_NOTINFILEDIALOG  ) lFlagNames.push_back(FLAGNAME_NOTINFILEDIALOG  );
    if ((nFlags & FLAGVAL_NOTINSTALLED     ) == FLAGVAL_NOTINSTALLED     ) lFlagNames.push_back(FLAGNAME_NOTINSTALLED     );
    if ((nFlags & FLAGVAL_OWN              ) == FLAGVAL_OWN              ) lFlagNames.push_back(FLAGNAME_OWN              );
    if ((nFlags & FLAGVAL_PACKED           ) == FLAGVAL_PACKED           ) lFlagNames.push_back(FLAGNAME_PACKED           );
    if ((nFlags & FLAGVAL_PASSWORDTOMODIFY ) == FLAGVAL_PASSWORDTOMODIFY ) lFlagNames.push_back(FLAGNAME_PASSWORDTOMODIFY );
    if ((nFlags & FLAGVAL_PREFERRED        ) == FLAGVAL_PREFERRED        ) lFlagNames.push_back(FLAGNAME_PREFERRED        );
    if ((nFlags & FLAGVAL_STARTPRESENTATION) == FLAGVAL_STARTPRESENTATION) lFlagNames.push_back(FLAGNAME_STARTPRESENTATION);
    if ((nFlags & FLAGVAL_READONLY         ) == FLAGVAL_READONLY         ) lFlagNames.push_back(FLAGNAME_READONLY         );
    if ((nFlags & FLAGVAL_SUPPORTSSELECTION) == FLAGVAL_SUPPORTSSELECTION) lFlagNames.push_back(FLAGNAME_SUPPORTSSELECTION);
    if ((nFlags & FLAGVAL_TEMPLATE         ) == FLAGVAL_TEMPLATE         ) lFlagNames.push_back(FLAGNAME_TEMPLATE         );
    if ((nFlags & FLAGVAL_TEMPLATEPATH     ) == FLAGVAL_TEMPLATEPATH     ) lFlagNames.push_back(FLAGNAME_TEMPLATEPATH     );
    if ((nFlags & FLAGVAL_USESOPTIONS      ) == FLAGVAL_USESOPTIONS      ) lFlagNames.push_back(FLAGNAME_USESOPTIONS      );
    if ((nFlags & FLAGVAL_COMBINED         ) == FLAGVAL_COMBINED         ) lFlagNames.push_back(FLAGNAME_COMBINED         );

    return lFlagNames.getAsConstList();
}

/*-----------------------------------------------
    static! => no locks neccessary
-----------------------------------------------*/
sal_Int32 FilterCache::impl_convertFlagNames2FlagField(const css::uno::Sequence< ::rtl::OUString >& lNames)
{
    sal_Int32 nField = 0;

    const ::rtl::OUString* pNames = lNames.getConstArray();
          sal_Int32        c      = lNames.getLength();
    for (sal_Int32 i=0; i<c; ++i)
    {
        if (pNames[i].equals(FLAGNAME_3RDPARTYFILTER))
        {
            nField |= FLAGVAL_3RDPARTYFILTER;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_ALIEN))
        {
            nField |= FLAGVAL_ALIEN;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_ASYNCHRON))
        {
            nField |= FLAGVAL_ASYNCHRON;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_BROWSERPREFERRED))
        {
            nField |= FLAGVAL_BROWSERPREFERRED;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_CONSULTSERVICE))
        {
            nField |= FLAGVAL_CONSULTSERVICE;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_DEFAULT))
        {
            nField |= FLAGVAL_DEFAULT;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_ENCRYPTION))
        {
            nField |= FLAGVAL_ENCRYPTION;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_EXPORT))
        {
            nField |= FLAGVAL_EXPORT;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_IMPORT))
        {
            nField |= FLAGVAL_IMPORT;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_INTERNAL))
        {
            nField |= FLAGVAL_INTERNAL;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_NOTINCHOOSER))
        {
            nField |= FLAGVAL_NOTINCHOOSER;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_NOTINFILEDIALOG))
        {
            nField |= FLAGVAL_NOTINFILEDIALOG;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_NOTINSTALLED))
        {
            nField |= FLAGVAL_NOTINSTALLED;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_OWN))
        {
            nField |= FLAGVAL_OWN;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_PACKED))
        {
            nField |= FLAGVAL_PACKED;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_PASSWORDTOMODIFY))
        {
            nField |= FLAGVAL_PASSWORDTOMODIFY;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_PREFERRED))
        {
            nField |= FLAGVAL_PREFERRED;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_STARTPRESENTATION))
        {
            nField |= FLAGVAL_STARTPRESENTATION;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_READONLY))
        {
            nField |= FLAGVAL_READONLY;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_SUPPORTSSELECTION))
        {
            nField |= FLAGVAL_SUPPORTSSELECTION;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_TEMPLATE))
        {
            nField |= FLAGVAL_TEMPLATE;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_TEMPLATEPATH))
        {
            nField |= FLAGVAL_TEMPLATEPATH;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_USESOPTIONS))
        {
            nField |= FLAGVAL_USESOPTIONS;
            continue;
        }
        if (pNames[i].equals(FLAGNAME_COMBINED))
        {
            nField |= FLAGVAL_COMBINED;
            continue;
        }
    }

    return nField;
}



void FilterCache::impl_interpretDataVal4Type(const ::rtl::OUString& sValue,
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
        case 3:     rItem[PROPNAME_URLPATTERN] <<= impl_tokenizeString(sValue, (sal_Unicode)';').getAsConstList();
                    break;
        // Extensions
        case 4:     rItem[PROPNAME_EXTENSIONS] <<= impl_tokenizeString(sValue, (sal_Unicode)';').getAsConstList();
                    break;
    }
}



void FilterCache::impl_interpretDataVal4Filter(const ::rtl::OUString& sValue,
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
                            OSL_FAIL("FilterCache::impl_interpretDataVal4Filter()\nCant move Order value from filter to type on demand!\n");
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
        case 5:     rItem[PROPNAME_USERDATA] <<= impl_tokenizeString(sValue, (sal_Unicode)';').getAsConstList();
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
         That would be usefully to gurantee a consistent cache.
-----------------------------------------------*/
void FilterCache::impl_readOldFormat()
    throw(css::uno::Exception)
{
    // Attention: Opening/Reading of this old configuration format has to be handled gracefully.
    // Its optional and shouldnt disturb our normal work!
    // E.g. we must check, if the package exists ...

    css::uno::Reference< css::container::XNameAccess > xCfg;
    try
    {
        css::uno::Reference< css::uno::XInterface > xInt = impl_openConfig(E_PROVIDER_OLD);
        xCfg = css::uno::Reference< css::container::XNameAccess >(xInt, css::uno::UNO_QUERY_THROW);
    }
    /* corrupt filter addon ? because it's external (optional) code .. we can ignore it. Addon wont work then ...
       but that seams to be acceptable.
       see #139088# for further informations
    */
    catch(const css::uno::Exception&)
        { return; }

    ::rtl::OUString TYPES_SET("Types");

    // May be there is no type set ...
    if (xCfg->hasByName(TYPES_SET))
    {
        css::uno::Reference< css::container::XNameAccess > xSet;
        xCfg->getByName(TYPES_SET) >>= xSet;
        const css::uno::Sequence< ::rtl::OUString > lItems = xSet->getElementNames();
        const ::rtl::OUString*                      pItems = lItems.getConstArray();
        for (sal_Int32 i=0; i<lItems.getLength(); ++i)
            m_lTypes[pItems[i]] = impl_readOldItem(xSet, E_TYPE, pItems[i]);
    }

    ::rtl::OUString FILTER_SET("Filters");
    // May be there is no filter set ...
    if (xCfg->hasByName(FILTER_SET))
    {
        css::uno::Reference< css::container::XNameAccess > xSet;
        xCfg->getByName(FILTER_SET) >>= xSet;
        const css::uno::Sequence< ::rtl::OUString > lItems = xSet->getElementNames();
        const ::rtl::OUString*                      pItems = lItems.getConstArray();
        for (sal_Int32 i=0; i<lItems.getLength(); ++i)
            m_lFilters[pItems[i]] = impl_readOldItem(xSet, E_FILTER, pItems[i]);
    }
}



CacheItem FilterCache::impl_readOldItem(const css::uno::Reference< css::container::XNameAccess >& xSet ,
                                              EItemType                                           eType,
                                        const ::rtl::OUString&                                    sItem)
    throw(css::uno::Exception)
{
    css::uno::Reference< css::container::XNameAccess > xItem;
    xSet->getByName(sItem) >>= xItem;
    if (!xItem.is())
        throw css::uno::Exception(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Cant read old item." )),
                css::uno::Reference< css::uno::XInterface >());

    CacheItem aItem;
    aItem[PROPNAME_NAME] <<= sItem;

    // Installed flag ...
    // Isnt used any longer!

    // UIName
    impl_readPatchUINames(xItem, aItem);

    // Data
    ::rtl::OUString sData;
    OUStringList    lData;
    xItem->getByName(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Data" ))) >>= sData;
    lData = impl_tokenizeString(sData, (sal_Unicode)',');
    if (
        (sData.isEmpty()) ||
        (lData.size()<1    )
       )
    {
        throw css::uno::Exception(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Cant read old item property DATA." )),
                css::uno::Reference< css::uno::XInterface >());
    }

    sal_Int32 nProp = 0;
    for (OUStringList::const_iterator pProp  = lData.begin();
                                      pProp != lData.end()  ;
                                    ++pProp                 )
    {
        const ::rtl::OUString& sProp = *pProp;
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



OUStringList FilterCache::impl_tokenizeString(const ::rtl::OUString& sData     ,
                                                    sal_Unicode      cSeperator)
{
    OUStringList lData  ;
    sal_Int32    nToken = 0;
    do
    {
        ::rtl::OUString sToken = sData.getToken(0, cSeperator, nToken);
        lData.push_back(sToken);
    }
    while(nToken >= 0);
    return lData;
}

#if OSL_DEBUG_LEVEL > 0


::rtl::OUString FilterCache::impl_searchFrameLoaderForType(const ::rtl::OUString& sType) const
{
    CacheItemList::const_iterator pIt;
    for (  pIt  = m_lFrameLoaders.begin();
           pIt != m_lFrameLoaders.end()  ;
         ++pIt                           )
    {
        const ::rtl::OUString& sItem = pIt->first;
        ::comphelper::SequenceAsHashMap lProps(pIt->second);
        OUStringList                    lTypes(lProps[PROPNAME_TYPES]);

        if (::std::find(lTypes.begin(), lTypes.end(), sType) != lTypes.end())
            return sItem;
    }

    return ::rtl::OUString();
}



::rtl::OUString FilterCache::impl_searchContentHandlerForType(const ::rtl::OUString& sType) const
{
    CacheItemList::const_iterator pIt;
    for (  pIt  = m_lContentHandlers.begin();
           pIt != m_lContentHandlers.end()  ;
         ++pIt                              )
    {
        const ::rtl::OUString& sItem = pIt->first;
        ::comphelper::SequenceAsHashMap lProps(pIt->second);
        OUStringList                    lTypes(lProps[PROPNAME_TYPES]);

        if (::std::find(lTypes.begin(), lTypes.end(), sType) != lTypes.end())
            return sItem;
    }

    return ::rtl::OUString();
}
#endif



sal_Bool FilterCache::impl_isModuleInstalled(const ::rtl::OUString& sModule)
{
    css::uno::Reference< css::container::XNameAccess > xCfg;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (! m_xModuleCfg.is())
    {
        m_xModuleCfg = css::uno::Reference< css::container::XNameAccess >(
                            ::comphelper::ConfigurationHelper::openConfig(
                                m_xSMGR,
                                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Setup/Office/Factories" )),
                                ::comphelper::ConfigurationHelper::E_READONLY),
                            css::uno::UNO_QUERY_THROW);
    }

    xCfg = m_xModuleCfg;
    aLock.clear();
    // <- SAFE

    if (xCfg.is())
        return xCfg->hasByName(sModule);

    return sal_False;
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
