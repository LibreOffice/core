/*************************************************************************
 *
 *  $RCSfile: filterfactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-29 13:41:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "filterfactory.hxx"
#include "macros.hxx"
#include "constant.hxx"
#include "versions.hxx"

//_______________________________________________
// includes

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

/** @short  can be used to query for filters related to its application module.
 */
#define BASE_QUERY_ALL          "_query_all"
#define BASE_QUERY_WRITER       "_query_Writer"
#define BASE_QUERY_WEB          "_query_web"
#define BASE_QUERY_GLOBAL       "_query_global"
#define BASE_QUERY_CHART        "_query_chart"
#define BASE_QUERY_CALC         "_query_calc"
#define BASE_QUERY_IMPRESS      "_query_impress"
#define BASE_QUERY_DRAW         "_query_draw"
#define BASE_QUERY_MATH         "_query_math"

//_______________________________________________

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
#define SEPERATOR_QUERYPARAM                            ((sal_Unicode)':')
#define SEPERATOR_QUERYPARAMVALUE                       ((sal_Unicode)'=')

#define QUERYPARAM_IFLAGS                               ::rtl::OUString::createFromAscii("iflags")
#define QUERYPARAM_EFLAGS                               ::rtl::OUString::createFromAscii("eflags")
#define QUERYPARAM_SORT_PROP                            ::rtl::OUString::createFromAscii("sort_prop")

#define QUERYPARAM_DESCENDING                           ::rtl::OUString::createFromAscii("descending")
#define QUERYPARAM_USE_ORDER                            ::rtl::OUString::createFromAscii("use_order")
#define QUERYPARAM_DEFAULT_FIRST                        ::rtl::OUString::createFromAscii("default_first")
#define QUERYPARAM_CASE_SENSITIVE                       ::rtl::OUString::createFromAscii("case_sensitive")

#define QUERYPARAMVALUE_SORT_PROP_NAME                  ::rtl::OUString::createFromAscii("name")
#define QUERYPARAMVALUE_SORT_PROP_UINAME                ::rtl::OUString::createFromAscii("uiname")

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
FilterFactory::FilterFactory(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    BaseContainer::init(xSMGR                                         ,
                        FilterFactory::impl_getImplementationName()   ,
                        FilterFactory::impl_getSupportedServiceNames(),
                        FilterCache::E_FILTER                         );
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
FilterFactory::~FilterFactory()
{
}

/*-----------------------------------------------
    16.07.2003 13:43
-----------------------------------------------*/
css::uno::Reference< css::uno::XInterface > SAL_CALL FilterFactory::createInstance(const ::rtl::OUString& sFilter)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    return createInstanceWithArguments(sFilter, css::uno::Sequence< css::uno::Any >());
}

/*-----------------------------------------------
    17.07.2003 08:56
-----------------------------------------------*/
css::uno::Reference< css::uno::XInterface > SAL_CALL FilterFactory::createInstanceWithArguments(const ::rtl::OUString&                     sFilter   ,
                                                                                                const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    ::rtl::OUString sRealFilter = sFilter;

    #ifdef _FILTER_CONFIG_MIGRATION_Q_

        /* -> TODO - HACK
            check if the given filter name realy exist ...
            Because our old implementation worked with an internal
            type name instead of a filter name. For a small migration time
            we must simulate this old feature :-( */

        if (!m_rCache->hasItem(FilterCache::E_FILTER, sFilter) && m_rCache->hasItem(FilterCache::E_TYPE, sFilter))
        {
            OSL_ENSURE(sal_False, "Who use this deprecated functionality?");
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
    ::rtl::OUString sFilterService;
    aFilter[PROPNAME_FILTERSERVICE] >>= sFilterService;

    // create service instance
    css::uno::Reference< css::uno::XInterface > xFilter;
    if (sFilterService.getLength())
        xFilter = m_xSMGR->createInstance(sFilterService);

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

/*-----------------------------------------------
    18.02.2004 14:21
-----------------------------------------------*/
css::uno::Sequence< ::rtl::OUString > SAL_CALL FilterFactory::getAvailableServiceNames()
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
    lEProps[PROPNAME_FILTERSERVICE] <<= ::rtl::OUString();

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

/*-----------------------------------------------
    11.03.2004 08:37
-----------------------------------------------*/
css::uno::Reference< css::container::XEnumeration > SAL_CALL FilterFactory::createSubSetEnumerationByQuery(const ::rtl::OUString& sQuery)
    throw (css::uno::RuntimeException)
{
    // reject old deprecated queries ...
    if (sQuery.matchAsciiL("_filterquery_",13,0))
        throw css::uno::RuntimeException(
                    _FILTER_CONFIG_FROM_ASCII_("Use of deprecated and now unsupported query!"),
                    static_cast< css::container::XContainerQuery* >(this));

    // convert "_query_xxx:..." to "getByDocService=xxx:..."
    ::rtl::OUString sNewQuery(sQuery);
    sal_Int32 pos = sNewQuery.indexOf(::rtl::OUString::createFromAscii("_query_"),0);
    if (pos != -1)
    {
        OSL_ENSURE(sal_False, "DEPRECATED!\nPlease use new query format: 'matchByDocumentService=...'");
        ::rtl::OUStringBuffer sPatchedQuery(256);
        sPatchedQuery.appendAscii("matchByDocumentService=");
        sPatchedQuery.append     (sNewQuery.copy(7)        );
        sNewQuery = sPatchedQuery.makeStringAndClear();
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
            OSL_ENSURE(sal_False, "DEPRECATED!\nPlease use prop search at the TypeDetection container!");
//            lEnumSet = impl_queryGetPreferredFilterForType(lTokens);
        else
        if (lTokens.find(QUERY_IDENTIFIER_MATCHBYDOCUMENTSERVICE) != lTokens.end())
            lEnumSet = impl_queryMatchByDocumentService(lTokens);
    }

    // pack list of item names as an enum list
    // Attention: Do not return empty reference for empty list!
    // The outside check "hasMoreElements()" should be enough, to detect this state :-)
    size_t c = lEnumSet.size();
    css::uno::Sequence< ::rtl::OUString > lSet = lEnumSet.getAsConstList();
    ::comphelper::OEnumerationByName* pEnum = new ::comphelper::OEnumerationByName(this, lSet);
    return css::uno::Reference< css::container::XEnumeration >(static_cast< css::container::XEnumeration* >(pEnum), css::uno::UNO_QUERY);
}
/*
        if (lEnumSet.empty())
        {
            //-------------------------------------------
            // 1) getDefaultFilterForType=<internal_typename>

            pIt = lTokens.find(::rtl::OUString::createFromAscii("getDefaultFilterForType"));
            if (pIt != lTokens.end())
            {
                // SAFE ->
                ::osl::ResettableMutexGuard aLock(m_aLock);

                // might not all types was loaded till now!
                impl_loadOnDemand();

                ::rtl::OUString sType  = pIt->second;
                FilterCache*    pCache = impl_getWorkingCache();
                if (pCache->hasItem(FilterCache::E_TYPE, sType))
                {
                    CacheItem aType = pCache->getItem(FilterCache::E_TYPE, sType);
                    ::rtl::OUString sPreferredFilter;
                    aType[PROPNAME_PREFERREDFILTER] >>= sPreferredFilter;

                    if (
                        (sPreferredFilter.getLength()                              ) &&
                        (pCache->hasItem(FilterCache::E_FILTER, sPreferredFilter))
                       )
                    {
                        lEnumSet.push_back(sPreferredFilter);
                    }
                }

                aLock.clear();
                // <- SAFE
            }
        }
*/

/*-----------------------------------------------
    11.03.2004 08:33
-----------------------------------------------*/
OUStringList FilterFactory::impl_queryMatchByDocumentService(const QueryTokenizer& lTokens) const
{
    // analyze query
    QueryTokenizer::const_iterator pIt;

    ::rtl::OUString sDocumentService;
    sal_Int32       nIFlags = 0;
    sal_Int32       nEFlags = 0;

    pIt = lTokens.find(QUERY_IDENTIFIER_MATCHBYDOCUMENTSERVICE);
    if (pIt != lTokens.end())
        sDocumentService = pIt->second;

#define COMP_HACK
#ifdef COMP_HACK
    if (sDocumentService.equalsAscii("writer"))
    {
        OSL_ENSURE(sal_False, "DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = ::rtl::OUString::createFromAscii("com.sun.star.text.TextDocument");
    }
    else
    if (sDocumentService.equalsAscii("web"))
    {
        OSL_ENSURE(sal_False, "DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = ::rtl::OUString::createFromAscii("com.sun.star.text.WebDocument");
    }
    else
    if (sDocumentService.equalsAscii("global"))
    {
        OSL_ENSURE(sal_False, "DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = ::rtl::OUString::createFromAscii("com.sun.star.text.GlobalDocument");
    }
    else
    if (sDocumentService.equalsAscii("calc"))
    {
        OSL_ENSURE(sal_False, "DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = ::rtl::OUString::createFromAscii("com.sun.star.sheet.SpreadsheetDocument");
    }
    else
    if (sDocumentService.equalsAscii("draw"))
    {
        OSL_ENSURE(sal_False, "DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = ::rtl::OUString::createFromAscii("com.sun.star.drawing.DrawingDocument");
    }
    else
    if (sDocumentService.equalsAscii("impress"))
    {
        OSL_ENSURE(sal_False, "DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = ::rtl::OUString::createFromAscii("com.sun.star.presentation.PresentationDocument");
    }
    else
    if (sDocumentService.equalsAscii("math"))
    {
        OSL_ENSURE(sal_False, "DEPRECATED!\nPlease use right document service for filter query!");
        sDocumentService = ::rtl::OUString::createFromAscii("com.sun.star.formula.FormulaProperties");
    }
#endif

    pIt = lTokens.find(QUERY_PARAM_IFLAGS);
    if (pIt != lTokens.end())
        nIFlags = ::rtl::OUString(pIt->second).toInt32();

    pIt = lTokens.find(QUERY_PARAM_EFLAGS);
    if (pIt != lTokens.end())
        nEFlags = ::rtl::OUString(pIt->second).toInt32();

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
        const ::rtl::OUString&          sName   = *pName;
        const CacheItem                 aFilter = pCache->getItem(FilterCache::E_FILTER, sName);
              CacheItem::const_iterator pProp   ;

        // "matchByDocumentService="                    => any filter will be adressed here
        // "matchByDocumentService=all"                 => any filter will be adressed here
        // "matchByDocumentService=com.sun.star..."     => only filter matching this document service will be adressed
        ::rtl::OUString sCheckValue = aFilter.getUnpackedValueOrDefault(PROPNAME_DOCUMENTSERVICE, ::rtl::OUString());
        if (
            ( sDocumentService.getLength()                 ) &&
            (!sDocumentService.equals(QUERY_CONSTVALUE_ALL)) &&
            (!sCheckValue.equals(sDocumentService)         )
           )
        {
            continue; // ignore filter -> try next one!
        }

        // "iflags="        => not allowed
        // "iflags=-1"      => not allowed
        // "iflags=0"       => not usefull
        // "iflags=283648"  => only filter, which has set these flag field will be adressed
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
        // "eflags=0"       => not usefull
        // "eflags=283648"  => only filter, which has not set these flag field will be adressed
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

    aLock.clear();
    // <- SAFE ----------------------

    return lResult;
 }

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
::rtl::OUString FilterFactory::impl_getImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.filter.config.FilterFactory");
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
css::uno::Sequence< ::rtl::OUString > FilterFactory::impl_getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lServiceNames(1);
    lServiceNames[0] = ::rtl::OUString::createFromAscii("com.sun.star.document.FilterFactory");
    return lServiceNames;
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
css::uno::Reference< css::uno::XInterface > SAL_CALL FilterFactory::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    FilterFactory* pNew = new FilterFactory(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::lang::XMultiServiceFactory* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter
