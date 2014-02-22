/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


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
   SAL_INFO( "filter.config", "{ (as96863) FilterCache lifetime");
}



FilterCache::~FilterCache()
{
   SAL_INFO( "filter.config", "} (as96863) FilterCache lifetime");
    if (m_xTypesChglisteners.is())
        m_xTypesChglisteners->stopListening();
    if (m_xFiltersChgListener.is())
        m_xFiltersChgListener->stopListening();
}



FilterCache* FilterCache::clone() const
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    FilterCache* pClone = new FilterCache();

    
    
    

    pClone->m_lTypes                     = m_lTypes;
    pClone->m_lDetectServices            = m_lDetectServices;
    pClone->m_lFilters                   = m_lFilters;
    pClone->m_lFrameLoaders              = m_lFrameLoaders;
    pClone->m_lContentHandlers           = m_lContentHandlers;
    pClone->m_lExtensions2Types          = m_lExtensions2Types;
    pClone->m_lURLPattern2Types          = m_lURLPattern2Types;

    pClone->m_sActLocale                 = m_sActLocale;

    pClone->m_eFillState                 = m_eFillState;

    pClone->m_lChangedTypes              = m_lChangedTypes;
    pClone->m_lChangedFilters            = m_lChangedFilters;
    pClone->m_lChangedDetectServices     = m_lChangedDetectServices;
    pClone->m_lChangedFrameLoaders       = m_lChangedFrameLoaders;
    pClone->m_lChangedContentHandlers    = m_lChangedContentHandlers;

    return pClone;
    
}



void FilterCache::takeOver(const FilterCache& rClone)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    

    
    
    

    
    
    
    
    
    
    
    

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

    m_eFillState     = rClone.m_eFillState;

    
    
    
    
    impl_validateAndOptimize();
    
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
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    if ((m_eFillState & eRequired) == eRequired)
        return;

#if OSL_DEBUG_LEVEL > 1
    if (!bByThread && ((eRequired & E_CONTAINS_ALL) == E_CONTAINS_ALL))
    {
        SAL_WARN( "filter.config", "Who disturb our \"fill cache on demand\" feature and force loading of ALL data during office startup? Please optimize your code, so a full filled filter cache is not really needed here!");
    }
#endif

    

    
    
    
    
    
    
    
    if (m_eFillState == E_CONTAINS_NOTHING)
    {
        impl_getDirectCFGValue(CFGDIRECTKEY_OFFICELOCALE) >>= m_sActLocale;
        if (m_sActLocale.isEmpty())
        {
            _FILTER_CONFIG_LOG_1_("FilterCache::ctor() ... could not specify office locale => use default \"%s\"\n", _FILTER_CONFIG_TO_ASCII_(DEFAULT_OFFICELOCALE));
            m_sActLocale = DEFAULT_OFFICELOCALE;
        }

        
        impl_readOldFormat();

        
        
        
        
        
        /* LateInitListener* pLateInit = */ new LateInitListener(comphelper::getProcessComponentContext());
    }

    
    
    
    
    impl_load(eRequired);
    
}



sal_Bool FilterCache::isFillState(FilterCache::EFillState eState) const
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    return ((m_eFillState & eState) == eState);
    
}



OUStringList FilterCache::getMatchingItemsByProps(      EItemType  eType  ,
                                                  const CacheItem& lIProps,
                                                  const CacheItem& lEProps) const
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    
    const CacheItemList& rList = impl_getItemList(eType);

    OUStringList lKeys;

    
    
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
    
}



sal_Bool FilterCache::hasItems(EItemType eType) const
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    
    const CacheItemList& rList = impl_getItemList(eType);

    return !rList.empty();
    
}



OUStringList FilterCache::getItemNames(EItemType eType) const
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    
    const CacheItemList& rList = impl_getItemList(eType);

    OUStringList lKeys;
    for (CacheItemList::const_iterator pIt  = rList.begin();
                                       pIt != rList.end()  ;
                                     ++pIt                 )
    {
        lKeys.push_back(pIt->first);
    }
    return lKeys;
    
}



sal_Bool FilterCache::hasItem(      EItemType        eType,
                              const OUString& sItem)
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    
    const CacheItemList& rList = impl_getItemList(eType);

    
    
    
    CacheItemList::const_iterator pIt = rList.find(sItem);
    if (pIt != rList.end())
        return sal_True;

    try
    {
        impl_loadItemOnDemand(eType, sItem);
        
        return sal_True;
    }
    catch(const css::container::NoSuchElementException&)
    {}

    return sal_False;
    
}



CacheItem FilterCache::getItem(      EItemType        eType,
                               const OUString& sItem)
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    
    CacheItemList& rList = impl_getItemList(eType);

    
    CacheItemList::iterator pIt = rList.find(sItem);
    if (pIt == rList.end())
    {
        
        
        
        
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
        OUString sDocService;
        rFilter[PROPNAME_DOCUMENTSERVICE] >>= sDocService;

        
        
        bool bIsHelpFilter = sItem == "writer_web_HTML_help";

        if ( !bIsHelpFilter && !impl_isModuleInstalled(sDocService) )
        {
            OUString sMsg("The requested filter '" + sItem +
                          "' exists ... but it should not; because the corresponding LibreOffice module was not installed.");
            throw css::container::NoSuchElementException(sMsg, css::uno::Reference< css::uno::XInterface >());
        }
    }

    return pIt->second;
    
}



void FilterCache::removeItem(      EItemType        eType,
                             const OUString& sItem)
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    
    CacheItemList& rList = impl_getItemList(eType);

    CacheItemList::iterator pItem = rList.find(sItem);
    if (pItem == rList.end())
        pItem = impl_loadItemOnDemand(eType, sItem); 
    rList.erase(pItem);

    impl_addItem2FlushList(eType, sItem);
}



void FilterCache::setItem(      EItemType        eType ,
                          const OUString& sItem ,
                          const CacheItem&       aValue)
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    
    CacheItemList& rList = impl_getItemList(eType);

    
    
    CacheItem aItem = aValue;
    aItem[PROPNAME_NAME] <<= sItem;
    aItem.validateUINames(m_sActLocale);

    
    
    removeStatePropsFromItem(aItem);

    rList[sItem] = aItem;

    impl_addItem2FlushList(eType, sItem);
}


void FilterCache::refreshItem(      EItemType        eType,
                              const OUString& sItem)
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    impl_loadItemOnDemand(eType, sItem);
}



void FilterCache::addStatePropsToItem(      EItemType        eType,
                                      const OUString& sItem,
                                            CacheItem&       rItem)
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
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
        sal_Bool bMandatory = ((aDescription.Attributes & css::beans::PropertyAttribute::REMOVABLE) != css::beans::PropertyAttribute::REMOVABLE);

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
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
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

                
                
                if (!xItem.is())
                    throw css::uno::Exception("Cant add item. Set is finalized or mandatory!",
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

                
                
                if (!xItem.is())
                    throw css::uno::Exception("Cant change item. Its finalized or mandatory!",
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
    
    
    
    INetURLObject   aParser    (aURL.Main);
    OUString sExtension = aParser.getExtension(INetURLObject::LAST_SEGMENT       ,
                                                      true                          ,
                                                      INetURLObject::DECODE_WITH_CHARSET);
    sExtension = sExtension.toAsciiLowerCase();

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    
    
    
    
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

        }
    }

    
    
    
    
    
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
    
}

const CacheItemList& FilterCache::impl_getItemList(EItemType eType) const
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    switch(eType)
    {
        case E_TYPE           : return m_lTypes          ;
        case E_FILTER         : return m_lFilters        ;
        case E_FRAMELOADER    : return m_lFrameLoaders   ;
        case E_CONTENTHANDLER : return m_lContentHandlers;
        case E_DETECTSERVICE  : return m_lDetectServices ;

    }

    throw css::uno::Exception("unknown sub container requested.",
                                            css::uno::Reference< css::uno::XInterface >());
    
}

CacheItemList& FilterCache::impl_getItemList(EItemType eType)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    switch(eType)
    {
        case E_TYPE           : return m_lTypes          ;
        case E_FILTER         : return m_lFilters        ;
        case E_FRAMELOADER    : return m_lFrameLoaders   ;
        case E_CONTENTHANDLER : return m_lContentHandlers;
        case E_DETECTSERVICE  : return m_lDetectServices ;

    }

    throw css::uno::Exception("unknown sub container requested.",
                                            css::uno::Reference< css::uno::XInterface >());
    
}

css::uno::Reference< css::uno::XInterface > FilterCache::impl_openConfig(EConfigProvider eProvider)
    throw(css::uno::Exception)
{
    ::osl::ResettableMutexGuard aLock(m_aLock);

    OUString                              sPath      ;
    css::uno::Reference< css::uno::XInterface >* pConfig = 0;
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
            sRtlLog         = "framework (as96863) ::FilterCache::impl_openconfig(E_PROVIDER_TYPES)";
        }
        break;

        case E_PROVIDER_FILTERS :
        {
            if (m_xConfigFilters.is())
                return m_xConfigFilters;
            sPath           = CFGPACKAGE_TD_FILTERS;
            pConfig         = &m_xConfigFilters;
            sRtlLog         = "framework (as96863) ::FilterCache::impl_openconfig(E_PROVIDER_FILTERS)";
        }
        break;

        case E_PROVIDER_OTHERS :
        {
            if (m_xConfigOthers.is())
                return m_xConfigOthers;
            sPath   = CFGPACKAGE_TD_OTHERS;
            pConfig = &m_xConfigOthers;
            sRtlLog = "framework (as96863) ::FilterCache::impl_openconfig(E_PROVIDER_OTHERS)";
        }
        break;

        case E_PROVIDER_OLD :
        {
            
            
            sPath   = CFGPACKAGE_TD_OLD;
            pConfig = &xOld;
            sRtlLog = "framework (as96863) ::FilterCache::impl_openconfig(E_PROVIDER_OLD)";
        }
        break;

        default : throw css::uno::Exception("These configuration node isnt supported here for open!", 0);
    }

    {
        SAL_INFO( "filter.config", "" << sRtlLog.getStr());
        *pConfig = impl_createConfigAccess(sPath    ,
                                           sal_False,   
                                           sal_True );  
    }


    
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
                                                                               sal_True ,  
                                                                               sal_False); 
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
                                                                                       sal_Bool         bReadOnly   ,
                                                                                       sal_Bool         bLocalesMode)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    css::uno::Reference< css::uno::XInterface > xCfg;

    try
    {
        css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
            css::configuration::theDefaultProvider::get( comphelper::getProcessComponentContext() ) );

        ::comphelper::SequenceAsVector< css::uno::Any > lParams;
        css::beans::NamedValue aParam;

        
        aParam.Name    = _FILTER_CONFIG_FROM_ASCII_("nodepath");
        aParam.Value <<= sRoot;
        lParams.push_back(css::uno::makeAny(aParam));

        
        if (bLocalesMode)
        {
            aParam.Name    = _FILTER_CONFIG_FROM_ASCII_("locale");
            aParam.Value <<= _FILTER_CONFIG_FROM_ASCII_("*"     );
            lParams.push_back(css::uno::makeAny(aParam));
        }

        
        if (bReadOnly)
            xCfg = xConfigProvider->createInstanceWithArguments(SERVICE_CONFIGURATIONACCESS, lParams.getAsConstList());
        else
            xCfg = xConfigProvider->createInstanceWithArguments(SERVICE_CONFIGURATIONUPDATEACCESS, lParams.getAsConstList());

        
        
        
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
    
}



void FilterCache::impl_validateAndOptimize()
    throw(css::uno::Exception)
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    SAL_INFO( "filter.config", "framework (as96863) ::FilterCache::impl_validateAndOptimize");

    
    
    sal_Bool bSomeTypesShouldExist   = ((m_eFillState & E_CONTAINS_STANDARD       ) == E_CONTAINS_STANDARD       );
    sal_Bool bAllFiltersShouldExist  = ((m_eFillState & E_CONTAINS_FILTERS        ) == E_CONTAINS_FILTERS        );

#if OSL_DEBUG_LEVEL > 0

    sal_Int32             nWarnings = 0;


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
                "The list of types or filters is empty." );
    }

    
    
    
    
    sal_Int32             nErrors   = 0;
    OUStringBuffer sLog(256);

    CacheItemList::iterator pIt;

    for (pIt = m_lTypes.begin(); pIt != m_lTypes.end(); ++pIt)
    {
        OUString sType = pIt->first;
        CacheItem       aType = pIt->second;

        
        
        OUString sDetectService;
        aType[PROPNAME_DETECTSERVICE ] >>= sDetectService;
        if (!sDetectService.isEmpty())
            impl_resolveItem4TypeRegistration(&m_lDetectServices, sDetectService, sType);

        
        
        
        
        
        
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

        
        
        
        
        
        
        
        
        sal_Bool bPreferred = sal_False;
        aType[PROPNAME_PREFERRED] >>= bPreferred;

        const OUString* pExtensions = lExtensions.getConstArray();
        for (sal_Int32 e=0; e<ce; ++e)
        {
            
            
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

        
        
        
        
        if (!bAllFiltersShouldExist)
            continue;

        OUString sPrefFilter;
        aType[PROPNAME_PREFERREDFILTER] >>= sPrefFilter;
        if (sPrefFilter.isEmpty())
        {
            
            
            
            
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
                sLog.append("Warning\t:\t" "The type \"" + sType + "\" isnt used by any filter, loader or content handler.\n");
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
                    ++nWarnings; 
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
            if ((nFlags & FLAGVAL_IMPORT) != FLAGVAL_IMPORT)
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

    
    
    
    
    OUStringList lTypes = getItemNames(E_TYPE);
    for (  pIt  = m_lFrameLoaders.begin();
           pIt != m_lFrameLoaders.end()  ;
         ++pIt                           )
    {
        
        
        
        
        OUString sLoader = pIt->first;
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

    OUString sLogOut = sLog.makeStringAndClear();
    OSL_ENSURE(!nErrors, OUStringToOString(sLogOut,RTL_TEXTENCODING_UTF8).getStr());
    if (nErrors>0)
        throw css::document::CorruptedFilterConfigurationException(
                "filter configuration: " + sLogOut,
                css::uno::Reference< css::uno::XInterface >(),
                sLogOut);
    OSL_ENSURE(!nWarnings, OUStringToOString(sLogOut,RTL_TEXTENCODING_UTF8).getStr());

    
}



void FilterCache::impl_addItem2FlushList(      EItemType        eType,
                                         const OUString& sItem)
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

        default : throw css::uno::Exception("unsupported item type", 0);
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
    sal_Bool bExistsInConfigLayer = xSet->hasByName(sItem);
    sal_Bool bExistsInMemory      = (rList.find(sItem) != rList.end());

    EItemFlushState eState( E_ITEM_UNCHANGED );

    
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



void FilterCache::impl_resolveItem4TypeRegistration(      CacheItemList*   pList,
                                                    const OUString& sItem,
                                                    const OUString& sType)
    throw(css::uno::Exception)
{
    CacheItem& rItem = (*pList)[sItem];
    
    
    
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
    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    

    
    
    if (
        ((eRequiredState & E_CONTAINS_STANDARD) == E_CONTAINS_STANDARD) &&
        ((m_eFillState   & E_CONTAINS_STANDARD) != E_CONTAINS_STANDARD)
       )
    {
        
        
        
        css::uno::Reference< css::container::XNameAccess > xTypes(impl_openConfig(E_PROVIDER_TYPES), css::uno::UNO_QUERY_THROW);
        {
            SAL_INFO( "filter.config", "framework (as96863) ::FilterCache::load std");
            impl_loadSet(xTypes, E_TYPE, E_READ_STANDARD, &m_lTypes);
        }
    }

    
    
    if (
        ((eRequiredState & E_CONTAINS_TYPES) == E_CONTAINS_TYPES) &&
        ((m_eFillState   & E_CONTAINS_TYPES) != E_CONTAINS_TYPES)
       )
    {
        
        
        
        css::uno::Reference< css::container::XNameAccess > xTypes(impl_openConfig(E_PROVIDER_TYPES), css::uno::UNO_QUERY_THROW);
        {
            SAL_INFO( "filter.config", "framework (as96863) ::FilterCache::load all types");
            impl_loadSet(xTypes, E_TYPE, E_READ_UPDATE, &m_lTypes);
        }
    }

    
    
    if (
        ((eRequiredState & E_CONTAINS_FILTERS) == E_CONTAINS_FILTERS) &&
        ((m_eFillState   & E_CONTAINS_FILTERS) != E_CONTAINS_FILTERS)
       )
    {
        
        
        
        css::uno::Reference< css::container::XNameAccess > xFilters(impl_openConfig(E_PROVIDER_FILTERS), css::uno::UNO_QUERY_THROW);
        {
            SAL_INFO( "filter.config", "framework (as96863) ::FilterCache::load all filters");
            impl_loadSet(xFilters, E_FILTER, E_READ_ALL, &m_lFilters);
        }
    }

    
    
    if (
        ((eRequiredState & E_CONTAINS_FRAMELOADERS) == E_CONTAINS_FRAMELOADERS) &&
        ((m_eFillState   & E_CONTAINS_FRAMELOADERS) != E_CONTAINS_FRAMELOADERS)
       )
    {
        
        
        
        css::uno::Reference< css::container::XNameAccess > xLoaders(impl_openConfig(E_PROVIDER_OTHERS), css::uno::UNO_QUERY_THROW);
        {
            SAL_INFO( "filter.config", "framework (as96863) ::FilterCache::load all frame loader");
            impl_loadSet(xLoaders, E_FRAMELOADER, E_READ_ALL, &m_lFrameLoaders);
        }
    }

    
    
    if (
        ((eRequiredState & E_CONTAINS_CONTENTHANDLERS) == E_CONTAINS_CONTENTHANDLERS) &&
        ((m_eFillState   & E_CONTAINS_CONTENTHANDLERS) != E_CONTAINS_CONTENTHANDLERS)
       )
    {
        
        
        
        css::uno::Reference< css::container::XNameAccess > xHandlers(impl_openConfig(E_PROVIDER_OTHERS), css::uno::UNO_QUERY_THROW);
        {
            SAL_INFO( "filter.config", "framework (as96863) ::FilterCache::load all content handler");
            impl_loadSet(xHandlers, E_CONTENTHANDLER, E_READ_ALL, &m_lContentHandlers);
        }
    }

    
    m_eFillState = (EFillState) ((sal_Int32)m_eFillState | (sal_Int32)eRequiredState);

    
    
    impl_validateAndOptimize();

    
}



void FilterCache::impl_loadSet(const css::uno::Reference< css::container::XNameAccess >& xConfig,
                                     EItemType                                           eType  ,
                                     EReadOption                                         eOption,
                                     CacheItemList*                                      pCache )
    throw(css::uno::Exception)
{
    
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

    
    

    
    

    const OUString* pItems = lItems.getConstArray();
          sal_Int32        c      = lItems.getLength();
    for (sal_Int32 i=0; i<c; ++i)
    {
        CacheItemList::iterator pItem = pCache->find(pItems[i]);
        switch(eOption)
        {
            
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

    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    OUString sActLocale     = m_sActLocale    ;
    aLock.clear();
    

    css::uno::Any aVal = xNode->getByName(PROPNAME_UINAME);
    css::uno::Reference< css::container::XNameAccess > xUIName;
    if (!(aVal >>= xUIName) && !xUIName.is())
        return;

    const ::comphelper::SequenceAsVector< OUString >                 lLocales(xUIName->getElementNames());
          ::comphelper::SequenceAsVector< OUString >::const_iterator pLocale ;
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
    
    
    
    
    css::uno::Reference< css::container::XNameAccess > xItem;
    #ifdef WORKAROUND_EXCEPTION_PROBLEM
    try
    {
    #endif
        css::uno::Any aVal = xSet->getByName(sItem);
        if (!(aVal >>= xItem) || !xItem.is())
        {
            OUString sMsg("found corrupted item \"" + sItem + "\".");
            throw css::uno::Exception(sMsg, css::uno::Reference< css::uno::XInterface >());
        }
    #ifdef WORKAROUND_EXCEPTION_PROBLEM
    }
    catch(const css::container::NoSuchElementException&)
    {
        throw;
    }
    #endif

    
    
    
    
    
    CacheItem aItem;
    aItem[PROPNAME_NAME] = css::uno::makeAny(sItem);
    switch(eType)
    {
        
        case E_TYPE :
        {
            
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

        
        case E_FILTER :
        {
            
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

                
                
                css::uno::Sequence< OUString > lFlagNames;
                if (xItem->getByName(PROPNAME_FLAGS) >>= lFlagNames)
                    aItem[PROPNAME_FLAGS] <<= FilterCache::impl_convertFlagNames2FlagField(lFlagNames);
            }
            
            
            if (
                (eOption == E_READ_UPDATE) ||
                (eOption == E_READ_ALL   )
               )
            {
                aItem[PROPNAME_USERDATA    ] = xItem->getByName(PROPNAME_USERDATA    );
                aItem[PROPNAME_TEMPLATENAME] = xItem->getByName(PROPNAME_TEMPLATENAME);


#ifdef AS_ENABLE_FILTER_UINAMES
                impl_readPatchUINames(xItem, aItem);
#endif 
            }
        }
        break;

        
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
                                                           const OUString& sItem)
    throw(css::uno::Exception)
{
    CacheItemList*                              pList   = 0;
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

        case E_DETECTSERVICE :
        {
            SAL_WARN( "filter.config", "Cant load detect services on demand. Who use this unsupported feature?");
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

            
            
            
            
            pIt = aItem.find(PROPNAME_FLAGS);
            if (pIt != aItem.end())
            {
                sal_Int32 nFlags = 0;
                pIt->second >>= nFlags;
                css::uno::Any aFlagNameList;
                aFlagNameList <<= FilterCache::impl_convertFlagField2FlagNames(nFlags);
                xItem->replaceByName(PROPNAME_FLAGS, aFlagNameList);
            }



#ifdef AS_ENABLE_FILTER_UINAMES
            css::uno::Reference< css::container::XNameReplace > xUIName;
            xItem->getByName(PROPNAME_UINAME) >>= xUIName;
            impl_savePatchUINames(xUIName, aItem);
#endif 
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
css::uno::Sequence< OUString > FilterCache::impl_convertFlagField2FlagNames(sal_Int32 nFlags)
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
    static! => no locks necessary
-----------------------------------------------*/
sal_Int32 FilterCache::impl_convertFlagNames2FlagField(const css::uno::Sequence< OUString >& lNames)
{
    sal_Int32 nField = 0;

    const OUString* pNames = lNames.getConstArray();
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



void FilterCache::impl_interpretDataVal4Type(const OUString& sValue,
                                                   sal_Int32        nProp ,
                                                   CacheItem&       rItem )
{
    switch(nProp)
    {
        
        case 0:     {
                        if (sValue.toInt32() == 1)
                            rItem[PROPNAME_PREFERRED] = css::uno::makeAny(sal_True);
                        else
                            rItem[PROPNAME_PREFERRED] = css::uno::makeAny(sal_False);
                    }
                    break;
        
        case 1:     rItem[PROPNAME_MEDIATYPE] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        
        case 2:     rItem[PROPNAME_CLIPBOARDFORMAT] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        
        case 3:     rItem[PROPNAME_URLPATTERN] <<= impl_tokenizeString(sValue, (sal_Unicode)';').getAsConstList();
                    break;
        
        case 4:     rItem[PROPNAME_EXTENSIONS] <<= impl_tokenizeString(sValue, (sal_Unicode)';').getAsConstList();
                    break;
    }
}



void FilterCache::impl_interpretDataVal4Filter(const OUString& sValue,
                                                     sal_Int32        nProp ,
                                                     CacheItem&       rItem )
{
    switch(nProp)
    {
        
        case 0:     {
                        sal_Int32 nOrder = sValue.toInt32();
                        if (nOrder > 0)
                        {
                            SAL_WARN( "filter.config", "FilterCache::impl_interpretDataVal4Filter()\nCant move Order value from filter to type on demand!");
                            _FILTER_CONFIG_LOG_2_("impl_interpretDataVal4Filter(%d, \"%s\") ... OK", (int)eType, _FILTER_CONFIG_TO_ASCII_(rItem).getStr())
                        }
                    }
                    break;
        
        case 1:     rItem[PROPNAME_TYPE] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        
        case 2:     rItem[PROPNAME_DOCUMENTSERVICE] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        
        case 3:     rItem[PROPNAME_FILTERSERVICE] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        
        case 4:     rItem[PROPNAME_FLAGS] <<= sValue.toInt32();
                    break;
        
        case 5:     rItem[PROPNAME_USERDATA] <<= impl_tokenizeString(sValue, (sal_Unicode)';').getAsConstList();
                    break;
        
        case 6:     rItem[PROPNAME_FILEFORMATVERSION] <<= sValue.toInt32();
                    break;
        
        case 7:     rItem[PROPNAME_TEMPLATENAME] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
        
        case 8:     rItem[PROPNAME_UICOMPONENT] <<= ::rtl::Uri::decode(sValue, rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8);
                    break;
    }
}

/*-----------------------------------------------
    TODO work on a cache copy first, which can be flushed afterwards
         That would be useful to guarantee a consistent cache.
-----------------------------------------------*/
void FilterCache::impl_readOldFormat()
    throw(css::uno::Exception)
{
    
    
    

    css::uno::Reference< css::container::XNameAccess > xCfg;
    try
    {
        css::uno::Reference< css::uno::XInterface > xInt = impl_openConfig(E_PROVIDER_OLD);
        xCfg = css::uno::Reference< css::container::XNameAccess >(xInt, css::uno::UNO_QUERY_THROW);
    }
    /* corrupt filter addon ? because it's external (optional) code .. we can ignore it. Addon wont work then ...
       but that seems to be acceptable.
       see #139088# for further information
    */
    catch(const css::uno::Exception&)
        { return; }

    OUString TYPES_SET("Types");

    
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



CacheItem FilterCache::impl_readOldItem(const css::uno::Reference< css::container::XNameAccess >& xSet ,
                                              EItemType                                           eType,
                                        const OUString&                                    sItem)
    throw(css::uno::Exception)
{
    css::uno::Reference< css::container::XNameAccess > xItem;
    xSet->getByName(sItem) >>= xItem;
    if (!xItem.is())
        throw css::uno::Exception("Cant read old item.", css::uno::Reference< css::uno::XInterface >());

    CacheItem aItem;
    aItem[PROPNAME_NAME] <<= sItem;

    
    

    
    impl_readPatchUINames(xItem, aItem);

    
    OUString sData;
    OUStringList    lData;
    xItem->getByName( "Data" ) >>= sData;
    lData = impl_tokenizeString(sData, (sal_Unicode)',');
    if (
        (sData.isEmpty()) ||
        (lData.size()<1    )
       )
    {
        throw css::uno::Exception( "Cant read old item property DATA.", css::uno::Reference< css::uno::XInterface >());
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
        OUStringList                    lTypes(lProps[PROPNAME_TYPES]);

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
        OUStringList                    lTypes(lProps[PROPNAME_TYPES]);

        if (::std::find(lTypes.begin(), lTypes.end(), sType) != lTypes.end())
            return sItem;
    }

    return OUString();
}
#endif



sal_Bool FilterCache::impl_isModuleInstalled(const OUString& sModule)
{
    css::uno::Reference< css::container::XNameAccess > xCfg;

    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (! m_xModuleCfg.is())
    {
        m_xModuleCfg = officecfg::Setup::Office::Factories::get();
    }

    xCfg = m_xModuleCfg;
    aLock.clear();
    

    if (xCfg.is())
        return xCfg->hasByName(sModule);

    return sal_False;
}

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
