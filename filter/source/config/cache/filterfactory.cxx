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


#include "filterfactory.hxx"
#include "macros.hxx"
#include "constant.hxx"
#include "versions.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/enumhelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <officecfg/Setup.hxx>
#include <officecfg/TypeDetection/UISort.hxx>


namespace filter{
    namespace config{

/** @short  define all possible parts of a filter query.

    @descr  syntax: "<query>[:<param>[=<value>]]"
            e.g.:   "_query_writer:default_first:use_order:sort_prop=uiname"

            argument                        description                                     default
            -----------------------------------------------------------------------------------------------
            iflags=<mask>                   include filters by given mask                   0
            eflags=<mask>                   exclude filters by given mask                   0
            sort_prop=<[name,uiname]>       sort by internal name or uiname                 name
            descending                      sort descending                                 false
            use_order                       use order flag of filters for sorting           false
            default_first                   set default filter on top of return list        false
            case_sensitive                  compare "sort_prop" case sensitive              false
 */

FilterFactory::FilterFactory(const css::uno::Reference< css::uno::XComponentContext >& rxContext)
    : m_xContext(rxContext)
{
    BaseContainer::init(rxContext                                         ,
                        FilterFactory::impl_getImplementationName()   ,
                        FilterFactory::impl_getSupportedServiceNames(),
                        FilterCache::E_FILTER                         );
}



FilterFactory::~FilterFactory()
{
}



css::uno::Reference< css::uno::XInterface > SAL_CALL FilterFactory::createInstance(const OUString& sFilter)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    return createInstanceWithArguments(sFilter, css::uno::Sequence< css::uno::Any >());
}



css::uno::Reference< css::uno::XInterface > SAL_CALL FilterFactory::createInstanceWithArguments(const OUString&                     sFilter   ,
                                                                                                const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    OUString sRealFilter = sFilter;

    #ifdef _FILTER_CONFIG_MIGRATION_Q_

        /* -> TODO - HACK
            check if the given filter name realy exist ...
            Because our old implementation worked with an internal
            type name instead of a filter name. For a small migration time
            we must simulate this old feature :-( */

        if (!m_rCache->hasItem(FilterCache::E_FILTER, sFilter) && m_rCache->hasItem(FilterCache::E_TYPE, sFilter))
        {
            OSL_FAIL("Who use this deprecated functionality?");
            _FILTER_CONFIG_LOG_("FilterFactory::createInstanceWithArguments() ... simulate old type search functionality!\n");

            css::uno::Sequence< css::beans::NamedValue > lQuery(1);
            lQuery[0].Name    = PROPNAME_TYPE;
            lQuery[0].Value <<= sFilter;

            css::uno::Reference< css::container::XEnumeration > xSet = createSubSetEnumerationByProperties(lQuery);
            while(xSet->hasMoreElements())
            {
                ::comphelper::SequenceAsHashMap lHandlerProps(xSet->nextElement());
                if (!(lHandlerProps[PROPNAME_NAME] >>= sRealFilter))
                    continue;
            }

            // prevent outside code against NoSuchElementException!
            // But dont implement such defensive strategy for our new create handling :-)
            if (!m_rCache->hasItem(FilterCache::E_FILTER, sRealFilter))
                return css::uno::Reference< css::uno::XInterface>();
        }

        /* <- HACK */

    #endif // _FILTER_CONFIG_MIGRATION_Q_

    // search filter on cache
    CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, sRealFilter);
    OUString sFilterService;
    aFilter[PROPNAME_FILTERSERVICE] >>= sFilterService;

    // create service instance
    css::uno::Reference< css::uno::XInterface > xFilter;
    if (!sFilterService.isEmpty())
        xFilter = m_xContext->getServiceManager()->createInstanceWithContext(sFilterService, m_xContext);

    // initialize filter
    css::uno::Reference< css::lang::XInitialization > xInit(xFilter, css::uno::UNO_QUERY);
    if (xInit.is())
    {
        // format: lInitData[0] = seq<PropertyValue>, which contains all configuration properties of this filter
        //         lInitData[1] = lArguments[0]
        //         ...
        //         lInitData[n] = lArguments[n-1]
        css::uno::Sequence< css::beans::PropertyValue > lConfig;
        aFilter >> lConfig;

        ::comphelper::SequenceAsVector< css::uno::Any > stlArguments(lArguments);
        stlArguments.insert(stlArguments.begin(), css::uno::makeAny(lConfig));

        css::uno::Sequence< css::uno::Any > lInitData;
        stlArguments >> lInitData;

        xInit->initialize(lInitData);
    }

    return xFilter;
    // <- SAFE
}



css::uno::Sequence< OUString > SAL_CALL FilterFactory::getAvailableServiceNames()
    throw(css::uno::RuntimeException)
{
    /* Attention: Instead of getElementNames() this method have to return only filter names,
                  which can be created as UNO Services realy. Thats why we search for filters,
                  which dont have a valid value for the property "FilterService".
                  Of course we cant check for corrupted service names here. We can check
                  for empty strings only ...
    */
    CacheItem lIProps;
    CacheItem lEProps;
    lEProps[PROPNAME_FILTERSERVICE] <<= OUString();

    OUStringList lUNOFilters;
    try
    {
        lUNOFilters = m_rCache->getMatchingItemsByProps(FilterCache::E_FILTER, lIProps, lEProps);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { lUNOFilters.clear(); }

    return lUNOFilters.getAsConstList();
}



css::uno::Reference< css::container::XEnumeration > SAL_CALL FilterFactory::createSubSetEnumerationByQuery(const OUString& sQuery)
    throw (css::uno::RuntimeException)
{
    // reject old deprecated queries ...
    if (sQuery.matchAsciiL("_filterquery_",13,0))
        throw css::uno::RuntimeException(
                    _FILTER_CONFIG_FROM_ASCII_("Use of deprecated and now unsupported query!"),
                    static_cast< css::container::XContainerQuery* >(this));

    // convert "_query_xxx:..." to "getByDocService=xxx:..."
    OUString sNewQuery(sQuery);
    sal_Int32 pos = sNewQuery.indexOf("_query_");
    if (pos != -1)
    {
        OSL_FAIL("DEPRECATED!\nPlease use new query format: 'matchByDocumentService=...'");
        OUString sPatchedQuery("matchByDocumentService=" + sNewQuery.copy(7));
        sNewQuery = sPatchedQuery;
    }

    // analyze query and split it into its tokens
    QueryTokenizer                  lTokens(sNewQuery);
    QueryTokenizer::const_iterator  pIt;
    OUStringList                    lEnumSet;

    // start query
    // (see attention comment below!)
    if (lTokens.valid())
    {
        // SAFE -> ----------------------
        ::osl::ResettableMutexGuard aLock(m_aLock);
        // May be not all filters was loaded ...
        // But we need it now!
        impl_loadOnDemand();
        aLock.clear();
        // <- SAFE ----------------------

        if (lTokens.find(QUERY_IDENTIFIER_GETPREFERREDFILTERFORTYPE) != lTokens.end())
            OSL_FAIL("DEPRECATED!\nPlease use prop search at the TypeDetection container!");
        else
        if (lTokens.find(QUERY_IDENTIFIER_MATCHBYDOCUMENTSERVICE) != lTokens.end())
            lEnumSet = impl_queryMatchByDocumentService(lTokens);
        else
        if (lTokens.find(QUERY_IDENTIFIER_GET_SORTED_FILTERLIST) != lTokens.end())
            lEnumSet = impl_getSortedFilterList(lTokens);
    }

    // pack list of item names as an enum list
    // Attention: Do not return empty reference for empty list!
    // The outside check "hasMoreElements()" should be enough, to detect this state :-)
    css::uno::Sequence< OUString > lSet = lEnumSet.getAsConstList();
    ::comphelper::OEnumerationByName* pEnum = new ::comphelper::OEnumerationByName(this, lSet);
    return css::uno::Reference< css::container::XEnumeration >(static_cast< css::container::XEnumeration* >(pEnum), css::uno::UNO_QUERY);
}



OUStringList FilterFactory::impl_queryMatchByDocumentService(const QueryTokenizer& lTokens) const
{
    // analyze query
    QueryTokenizer::const_iterator pIt;

    OUString sDocumentService;
    sal_Int32       nIFlags = 0;
    sal_Int32       nEFlags = 0;

    pIt = lTokens.find(QUERY_IDENTIFIER_MATCHBYDOCUMENTSERVICE);
    if (pIt != lTokens.end())
        sDocumentService = pIt->second;

#define COMP_HACK
#ifdef COMP_HACK
    if ( sDocumentService == "writer" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = OUString( "com.sun.star.text.TextDocument" );
    }
    else if ( sDocumentService == "web" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = OUString( "com.sun.star.text.WebDocument" );
    }
    else if ( sDocumentService == "global" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = OUString( "com.sun.star.text.GlobalDocument" );
    }
    else if ( sDocumentService == "calc" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = OUString( "com.sun.star.sheet.SpreadsheetDocument" );
    }
    else if ( sDocumentService == "draw" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = OUString( "com.sun.star.drawing.DrawingDocument" );
    }
    else if ( sDocumentService == "impress" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = OUString( "com.sun.star.presentation.PresentationDocument" );
    }
    else if ( sDocumentService == "math" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = OUString( "com.sun.star.formula.FormulaProperties" );
    }
#endif

    pIt = lTokens.find(QUERY_PARAM_IFLAGS);
    if (pIt != lTokens.end())
        nIFlags = OUString(pIt->second).toInt32();

    pIt = lTokens.find(QUERY_PARAM_EFLAGS);
    if (pIt != lTokens.end())
        nEFlags = OUString(pIt->second).toInt32();

    // SAFE -> ----------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    // search suitable filters
    FilterCache* pCache       = impl_getWorkingCache();
    OUStringList lFilterNames = pCache->getItemNames(FilterCache::E_FILTER);
    OUStringList lResult      ;

    for (OUStringList::const_iterator pName  = lFilterNames.begin();
                                      pName != lFilterNames.end()  ;
                                    ++pName                        )
    {
        try
        {
            const OUString&          sName   = *pName;
            const CacheItem                 aFilter = pCache->getItem(FilterCache::E_FILTER, sName);
                CacheItem::const_iterator pProp   ;

            // "matchByDocumentService="                    => any filter will be addressed here
            // "matchByDocumentService=all"                 => any filter will be addressed here
            // "matchByDocumentService=com.sun.star..."     => only filter matching this document service will be addressed
            OUString sCheckValue = aFilter.getUnpackedValueOrDefault(PROPNAME_DOCUMENTSERVICE, OUString());
            if (
                (!sDocumentService.isEmpty()                   ) &&
                (!sDocumentService.equals(QUERY_CONSTVALUE_ALL)) &&
                (!sCheckValue.equals(sDocumentService)         )
            )
            {
                continue; // ignore filter -> try next one!
            }

            // "iflags="        => not allowed
            // "iflags=-1"      => not allowed
            // "iflags=0"       => not useful
            // "iflags=283648"  => only filter, which has set these flag field will be addressed
            sal_Int32 nCheckValue = aFilter.getUnpackedValueOrDefault(PROPNAME_FLAGS, (sal_Int32)0);
            if (
                (nIFlags > 0                       ) &&
                ((nCheckValue & nIFlags) != nIFlags)
            )
            {
                continue; // ignore filter -> try next one!
            }

            // "eflags="        => not allowed
            // "eflags=-1"      => not allowed
            // "eflags=0"       => not useful
            // "eflags=283648"  => only filter, which has not set these flag field will be addressed
            if (
                (nEFlags > 0                       ) &&
                ((nCheckValue & nEFlags) == nEFlags)
            )
            {
                continue; // ignore filter -> try next one!
            }

            // OK - this filter passed all checks.
            // It match the query ...
            lResult.push_back(sName);
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }
    }

    aLock.clear();
    // <- SAFE ----------------------

    return lResult;
}



class stlcomp_removeIfMatchFlags
{
    private:
        FilterCache* m_pCache ;
        sal_Int32    m_nFlags ;
        sal_Bool     m_bIFlags;

    public:
        stlcomp_removeIfMatchFlags(FilterCache* pCache ,
                                   sal_Int32    nFlags ,
                                   sal_Bool     bIFlags)
            : m_pCache (pCache )
            , m_nFlags (nFlags )
            , m_bIFlags(bIFlags)
        {}

        bool operator() (const OUString& sFilter) const
        {
            try
            {
                const CacheItem aFilter = m_pCache->getItem(FilterCache::E_FILTER, sFilter);
                        sal_Int32 nFlags  = aFilter.getUnpackedValueOrDefault(PROPNAME_FLAGS, ((sal_Int32)0));

                bool bMatch = false;
                if (m_bIFlags)
                    // IFlags are interpeted as ALL_FLAGS_MUST_MATCH !
                    bMatch = ((nFlags & m_nFlags) == m_nFlags);
                else
                    // EFlags are interpreted as ATE_LEAST_ONE_FLAG_MUST_MATCH !
                    bMatch = !(nFlags & m_nFlags);
                // We are asked for bRemove ! And bMatch = !bRemove => so bRemove = !bMatch .-)
                return !bMatch;
            }
            catch(const css::container::NoSuchElementException &)
            {
                return true;
            }
        }
};



OUStringList FilterFactory::impl_getSortedFilterList(const QueryTokenizer& lTokens) const
{
    // analyze the given query parameter
    QueryTokenizer::const_iterator pIt1;

    OUString sModule;
    sal_Int32       nIFlags = -1;
    sal_Int32       nEFlags = -1;

    pIt1 = lTokens.find(QUERY_PARAM_MODULE);
    if (pIt1 != lTokens.end())
        sModule = pIt1->second;
    pIt1 = lTokens.find(QUERY_PARAM_IFLAGS);
    if (pIt1 != lTokens.end())
        nIFlags = OUString(pIt1->second).toInt32();
    pIt1 = lTokens.find(QUERY_PARAM_EFLAGS);
    if (pIt1 != lTokens.end())
        nEFlags = OUString(pIt1->second).toInt32();

    // simple search for filters of one specific module.
    OUStringList lFilterList;
    if (!sModule.isEmpty())
        lFilterList = impl_getSortedFilterListForModule(sModule, nIFlags, nEFlags);
    else
    {
        // more complex search for all filters
        // We check first, which office modules are installed ...
        OUStringList lModules = impl_getListOfInstalledModules();
        OUStringList::const_iterator pIt2;
        for (  pIt2  = lModules.begin();
               pIt2 != lModules.end()  ;
             ++pIt2                    )
        {
            sModule = *pIt2;
            OUStringList lFilters4Module = impl_getSortedFilterListForModule(sModule, nIFlags, nEFlags);
            OUStringList::const_iterator pIt3;
            for (  pIt3  = lFilters4Module.begin();
                   pIt3 != lFilters4Module.end()  ;
                 ++pIt3                           )
            {
                const OUString& sFilter = *pIt3;
                lFilterList.push_back(sFilter);
            }
        }
    }

    return lFilterList;
}



OUStringList FilterFactory::impl_getListOfInstalledModules() const
{
    // SAFE -> ----------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aLock.clear();
    // <- SAFE ----------------------

    css::uno::Reference< css::container::XNameAccess > xModuleConfig = officecfg::Setup::Office::Factories::get(xContext);
    OUStringList lModules(xModuleConfig->getElementNames());
    return lModules;
}



OUStringList FilterFactory::impl_getSortedFilterListForModule(const OUString& sModule,
                                                                    sal_Int32        nIFlags,
                                                                    sal_Int32        nEFlags) const
{
    OUStringList lSortedFilters = impl_readSortedFilterListFromConfig(sModule);

    // get all filters for the requested module
    CacheItem lIProps;
    lIProps[PROPNAME_DOCUMENTSERVICE] <<= sModule;

    // SAFE -> ----------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);
    FilterCache* pCache        = impl_getWorkingCache();
    OUStringList lOtherFilters = pCache->getMatchingItemsByProps(FilterCache::E_FILTER, lIProps);
    aLock.clear();
    // <- SAFE ----------------------

    // bring "other" filters in an alphabeticly order
    // It's needed below.
    ::std::sort(lOtherFilters.begin(), lOtherFilters.end());

    // merge both lists together
    OUStringList           lMergedFilters = lSortedFilters;
    const OUStringList::const_iterator itlOtherFiltersEnd = lOtherFilters.end();
    const OUStringList::const_iterator itlSortedFiltersEnd = lSortedFilters.end();
    for (OUStringList::const_iterator  pIt  = lOtherFilters.begin();
           pIt != itlOtherFiltersEnd  ;
         ++pIt                         )
    {
        if (::std::find(lSortedFilters.begin(), lSortedFilters.end(), *pIt) == itlSortedFiltersEnd)
            lMergedFilters.push_back(*pIt);
    }

    OUStringList::iterator pItToErase;

    // remove all filters from this merged list, which does not fit the flag specification
    if (nIFlags != -1)
    {
        pItToErase = ::std::remove_if(lMergedFilters.begin(), lMergedFilters.end(), stlcomp_removeIfMatchFlags(pCache, nIFlags, sal_True));
        lMergedFilters.erase(pItToErase, lMergedFilters.end());
    }
    if (nEFlags != -1)
    {
        pItToErase = ::std::remove_if(lMergedFilters.begin(), lMergedFilters.end(), stlcomp_removeIfMatchFlags(pCache, nEFlags, sal_False));
        lMergedFilters.erase(pItToErase, lMergedFilters.end());
    }

    // sort the default filter to the front of this list
    // TODO

    return lMergedFilters;
}



OUStringList FilterFactory::impl_readSortedFilterListFromConfig(const OUString& sModule) const
{
    // SAFE -> ----------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    aLock.clear();
    // <- SAFE ----------------------

    try
    {
        css::uno::Reference< css::container::XNameAccess > xUISortConfig = officecfg::TypeDetection::UISort::ModuleDependendFilterOrder::get(xContext);
        // dont check the module name here. If it does not exists, an exception is thrown and catched below.
        // We return an empty list as result then.
        css::uno::Reference< css::container::XNameAccess > xModule;
        xUISortConfig->getByName(sModule) >>= xModule;
        if (xModule.is()) // only to be on the safe side of life if the exception was not thrown .-)
        {
            // Note: convertion of the returned Any to OUStringList throws
            // an IllegalArgumentException if the type does not match ...
            // but it resets the OUStringList to a length of 0 if the Any is empty!
            OUStringList lSortedFilters(xModule->getByName(PROPNAME_SORTEDFILTERLIST));
            return lSortedFilters;
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}

    return OUStringList();
}



OUString FilterFactory::impl_getImplementationName()
{
    return OUString( "com.sun.star.comp.filter.config.FilterFactory" );
}



css::uno::Sequence< OUString > FilterFactory::impl_getSupportedServiceNames()
{
    css::uno::Sequence< OUString > lServiceNames(1);
    lServiceNames[0] = OUString( "com.sun.star.document.FilterFactory" );
    return lServiceNames;
}



css::uno::Reference< css::uno::XInterface > SAL_CALL FilterFactory::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    FilterFactory* pNew = new FilterFactory( comphelper::getComponentContext(xSMGR) );
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::lang::XMultiServiceFactory* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
