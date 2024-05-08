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
#include "constant.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/enumhelper.hxx>
#include <comphelper/sequence.hxx>
#include <officecfg/Setup.hxx>
#include <officecfg/TypeDetection/UISort.hxx>


namespace filter::config{

FilterCache& GetTheFilterCache()
{
    static FilterCache CACHE;
    return CACHE;
}

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
    static const css::uno::Sequence<OUString> sServiceNames { u"com.sun.star.document.FilterFactory"_ustr };
    BaseContainer::init(u"com.sun.star.comp.filter.config.FilterFactory"_ustr   ,
                         sServiceNames,
                        FilterCache::E_FILTER                         );
}


FilterFactory::~FilterFactory()
{
}


css::uno::Reference< css::uno::XInterface > SAL_CALL FilterFactory::createInstance(const OUString& sFilter)
{
    return createInstanceWithArguments(sFilter, css::uno::Sequence< css::uno::Any >());
}


css::uno::Reference< css::uno::XInterface > SAL_CALL FilterFactory::createInstanceWithArguments(const OUString&                     sFilter   ,
                                                                                                const css::uno::Sequence< css::uno::Any >& lArguments)
{
    // SAFE ->
    std::unique_lock aLock(m_aMutex);

    auto & cache = GetTheFilterCache();

    // search filter on cache
    CacheItem aFilter = cache.getItem(FilterCache::E_FILTER, sFilter);
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

        ::std::vector< css::uno::Any > stlArguments(comphelper::sequenceToContainer< ::std::vector< css::uno::Any > >(lArguments));
        stlArguments.insert(stlArguments.begin(), css::uno::Any(lConfig));

        xInit->initialize(comphelper::containerToSequence(stlArguments));
    }

    return xFilter;
    // <- SAFE
}


css::uno::Sequence< OUString > SAL_CALL FilterFactory::getAvailableServiceNames()
{
    /* Attention: Instead of getElementNames() this method have to return only filter names,
                  which can be created as UNO Services really. That's why we search for filters,
                  which don't have a valid value for the property "FilterService".
                  Of course we can't check for corrupted service names here. We can check
                  for empty strings only...
    */
    css::beans::NamedValue lEProps[] {
        { PROPNAME_FILTERSERVICE, css::uno::Any(OUString()) } };

    std::vector<OUString> lUNOFilters;
    try
    {
        lUNOFilters = GetTheFilterCache().getMatchingItemsByProps(FilterCache::E_FILTER, {}, lEProps);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { lUNOFilters.clear(); }

    return comphelper::containerToSequence(lUNOFilters);
}


css::uno::Reference< css::container::XEnumeration > SAL_CALL FilterFactory::createSubSetEnumerationByQuery(const OUString& sQuery)
{
    // reject old deprecated queries ...
    if (sQuery.startsWith("_filterquery_"))
        throw css::uno::RuntimeException(
                    u"Use of deprecated and now unsupported query!"_ustr,
                    static_cast< css::container::XContainerQuery* >(this));

    // convert "_query_xxx:..." to "getByDocService=xxx:..."
    OUString sNewQuery(sQuery);
    sal_Int32 pos = sNewQuery.indexOf("_query_");
    if (pos != -1)
    {
        OSL_FAIL("DEPRECATED!\nPlease use new query format: 'matchByDocumentService=...'");
        sNewQuery = OUString::Concat("matchByDocumentService=") + sNewQuery.subView(7);
    }

    // analyze query and split it into its tokens
    QueryTokenizer                  lTokens(sNewQuery);
    std::vector<OUString>           lEnumSet;

    // start query
    // (see attention comment below!)
    if (lTokens.valid())
    {
        // SAFE -> ----------------------
        {
            std::unique_lock aLock(m_aMutex);
            // May be not all filters was loaded ...
            // But we need it now!
            impl_loadOnDemand(aLock);
        }
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
    return new ::comphelper::OEnumerationByName(this, std::move(lEnumSet));
}


std::vector<OUString> FilterFactory::impl_queryMatchByDocumentService(const QueryTokenizer& lTokens) const
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
        sDocumentService = "com.sun.star.text.TextDocument";
    }
    else if ( sDocumentService == "web" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = "com.sun.star.text.WebDocument";
    }
    else if ( sDocumentService == "global" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = "com.sun.star.text.GlobalDocument";
    }
    else if ( sDocumentService == "calc" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = "com.sun.star.sheet.SpreadsheetDocument";
    }
    else if ( sDocumentService == "draw" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = "com.sun.star.drawing.DrawingDocument";
    }
    else if ( sDocumentService == "impress" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = "com.sun.star.presentation.PresentationDocument";
    }
    else if ( sDocumentService == "math" )
    {
        OSL_FAIL("DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = "com.sun.star.formula.FormulaProperties";
    }
#endif

    pIt = lTokens.find(QUERY_PARAM_IFLAGS);
    if (pIt != lTokens.end())
        nIFlags = pIt->second.toInt32();

    pIt = lTokens.find(QUERY_PARAM_EFLAGS);
    if (pIt != lTokens.end())
        nEFlags = pIt->second.toInt32();

    // SAFE -> ----------------------
    std::unique_lock aLock(m_aMutex);

    // search suitable filters
    FilterCache* pCache       = impl_getWorkingCache(aLock);
    std::vector<OUString> lFilterNames = pCache->getItemNames(FilterCache::E_FILTER);
    std::vector<OUString> lResult      ;

    for (auto const& filterName : lFilterNames)
    {
        try
        {
            const CacheItem aFilter = pCache->getItem(FilterCache::E_FILTER, filterName);

            // "matchByDocumentService="                    => any filter will be addressed here
            // "matchByDocumentService=all"                 => any filter will be addressed here
            // "matchByDocumentService=com.sun.star..."     => only filter matching this document service will be addressed
            OUString sCheckValue = aFilter.getUnpackedValueOrDefault(PROPNAME_DOCUMENTSERVICE, OUString());
            if (
                (!sDocumentService.isEmpty()                   ) &&
                (sDocumentService != QUERY_CONSTVALUE_ALL      ) &&
                (sCheckValue != sDocumentService         )
            )
            {
                continue; // ignore filter -> try next one!
            }

            // "iflags="        => not allowed
            // "iflags=-1"      => not allowed
            // "iflags=0"       => not useful
            // "iflags=283648"  => only filter, which has set these flag field will be addressed
            sal_Int32 nCheckValue = aFilter.getUnpackedValueOrDefault(PROPNAME_FLAGS, sal_Int32(0));
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
            lResult.push_back(filterName);
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            { continue; }
    }

    aLock.unlock();
    // <- SAFE ----------------------

    return lResult;
}

namespace {

class stlcomp_removeIfMatchFlags
{
    private:
        FilterCache* m_pCache ;
        sal_Int32    m_nFlags ;
        bool     m_bIFlags;

    public:
        stlcomp_removeIfMatchFlags(FilterCache* pCache ,
                                   sal_Int32    nFlags ,
                                   bool     bIFlags)
            : m_pCache (pCache )
            , m_nFlags (nFlags )
            , m_bIFlags(bIFlags)
        {}

        bool operator() (const OUString& sFilter) const
        {
            try
            {
                const CacheItem aFilter = m_pCache->getItem(FilterCache::E_FILTER, sFilter);
                sal_Int32 nFlags  = aFilter.getUnpackedValueOrDefault(PROPNAME_FLAGS, (sal_Int32(0)));

                bool bMatch = false;
                if (m_bIFlags)
                    // IFlags are interpreted as ALL_FLAGS_MUST_MATCH !
                    bMatch = ((nFlags & m_nFlags) == m_nFlags);
                else
                    // EFlags are interpreted as AT_LEAST_ONE_FLAG_MUST_MATCH !
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

}

std::vector<OUString> FilterFactory::impl_getSortedFilterList(const QueryTokenizer& lTokens) const
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
        nIFlags = pIt1->second.toInt32();
    pIt1 = lTokens.find(QUERY_PARAM_EFLAGS);
    if (pIt1 != lTokens.end())
        nEFlags = pIt1->second.toInt32();

    // simple search for filters of one specific module.
    std::vector<OUString> lFilterList;
    if (!sModule.isEmpty())
        lFilterList = impl_getSortedFilterListForModule(sModule, nIFlags, nEFlags);
    else
    {
        // more complex search for all filters
        // We check first, which office modules are installed...
        const css::uno::Sequence<OUString> lModules = impl_getListOfInstalledModules();
        for (auto const& module : lModules)
        {
            std::vector<OUString> lFilters4Module = impl_getSortedFilterListForModule(module, nIFlags, nEFlags);
            for (auto const& filter4Module : lFilters4Module)
            {
                lFilterList.push_back(filter4Module);
            }
        }
    }

    return lFilterList;
}


css::uno::Sequence<OUString> FilterFactory::impl_getListOfInstalledModules()
{
    css::uno::Reference< css::container::XNameAccess > xModuleConfig = officecfg::Setup::Office::Factories::get();
    return xModuleConfig->getElementNames();
}


std::vector<OUString> FilterFactory::impl_getSortedFilterListForModule(const OUString& sModule,
                                                                    sal_Int32        nIFlags,
                                                                    sal_Int32        nEFlags) const
{
    std::vector<OUString> lSortedFilters = impl_readSortedFilterListFromConfig(sModule);

    // get all filters for the requested module
    css::beans::NamedValue lIProps[] { { PROPNAME_DOCUMENTSERVICE, css::uno::Any(sModule) } };

    // SAFE -> ----------------------
    std::unique_lock aLock(m_aMutex);
    FilterCache* pCache        = impl_getWorkingCache(aLock);
    std::vector<OUString> lOtherFilters = pCache->getMatchingItemsByProps(FilterCache::E_FILTER, lIProps);
    aLock.unlock();
    // <- SAFE ----------------------

    // bring "other" filters in an alphabetical order
    // It's needed below.
    ::std::sort(lOtherFilters.begin(), lOtherFilters.end());

    // merge both lists together
    std::vector<OUString> lMergedFilters = lSortedFilters;
    const auto itlSortedFiltersEnd = lSortedFilters.cend();
    for (auto const& otherFilter : lOtherFilters)
    {
        if (::std::find(lSortedFilters.cbegin(), lSortedFilters.cend(), otherFilter) == itlSortedFiltersEnd)
            lMergedFilters.push_back(otherFilter);
    }

    // remove all filters from this merged list, which does not fit the flag specification
    if (nIFlags != -1)
    {
        std::erase_if(lMergedFilters, stlcomp_removeIfMatchFlags(pCache, nIFlags, true));
    }
    if (nEFlags != -1)
    {
        std::erase_if(lMergedFilters, stlcomp_removeIfMatchFlags(pCache, nEFlags, false));
    }

    // sort the default filter to the front of this list
    // TODO

    return lMergedFilters;
}


std::vector<OUString> FilterFactory::impl_readSortedFilterListFromConfig(const OUString& sModule)
{
    try
    {
        css::uno::Reference< css::container::XNameAccess > xUISortConfig = officecfg::TypeDetection::UISort::ModuleDependendFilterOrder::get();
        // don't check the module name here. If it does not exists, an exception is thrown and caught below.
        // We return an empty list as result then.
        css::uno::Reference< css::container::XNameAccess > xModule;
        xUISortConfig->getByName(sModule) >>= xModule;
        if (xModule.is()) // only to be on the safe side of life if the exception was not thrown .-)
        {
            // Note: conversion of the returned Any to std::vector<OUString> throws
            // an IllegalArgumentException if the type does not match ...
            // but it resets the std::vector<OUString> to a length of 0 if the Any is empty!
            std::vector<OUString> lSortedFilters(
                    comphelper::sequenceToContainer< std::vector<OUString> >(xModule->getByName(PROPNAME_SORTEDFILTERLIST).get<css::uno::Sequence<OUString> >()));
            return lSortedFilters;
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}

    return std::vector<OUString>();
}

} // namespace filter

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_FilterFactory_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new filter::config::FilterFactory(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
