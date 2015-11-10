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

#include "sal/config.h"

#include <functional>
#include <algorithm>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include "CFStringUtilities.hxx"
#include "NSString_OOoAdditions.hxx"
#include "NSURL_OOoAdditions.hxx"

#include "FilterHelper.hxx"

namespace {

void fillSuffixList(OUStringList& aSuffixList, const ::rtl::OUString& suffixString) {
    sal_Int32 nIndex = 0;
    do {
        rtl::OUString aToken = suffixString.getToken( 0, ';', nIndex );
        aSuffixList.push_back(aToken.copy(1));
    } while ( nIndex >= 0 );
}

}

#pragma mark DEFINES
#define CLASS_NAME "FilterEntry"

#pragma mark FilterEntry

FilterEntry::FilterEntry( const rtl::OUString& _rTitle, const UnoFilterList& _rSubFilters )
:m_sTitle( _rTitle )
,m_aSubFilters( _rSubFilters )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", _rTitle);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}


bool FilterEntry::hasSubFilters() const
{
//    OSL_TRACE(">>> FilterEntry::%s", __func__);
    bool bReturn = ( 0 < m_aSubFilters.getLength() );
//    OSL_TRACE("<<< FilterEntry::%s retVal: %d", __func__, bReturn);
    return bReturn;
}


sal_Int32 FilterEntry::getSubFilters( UnoFilterList& _rSubFilterList )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    _rSubFilterList = m_aSubFilters;
    sal_Int32 nReturn = m_aSubFilters.getLength();

    DBG_PRINT_EXIT(CLASS_NAME, __func__, nReturn);

    return nReturn;
}

#pragma mark statics
static bool
isFilterString( const rtl::OUString& rFilterString, const char *pMatch )
{
    sal_Int32 nIndex = 0;
    rtl::OUString aToken;
    bool bIsFilter = true;

    rtl::OUString aMatch(rtl::OUString::createFromAscii(pMatch));

    do
    {
        aToken = rFilterString.getToken( 0, ';', nIndex );
        if( !aToken.match( aMatch ) )
        {
            bIsFilter = false;
            break;
        }
    }
    while( nIndex >= 0 );

    return bIsFilter;
}



static rtl::OUString
shrinkFilterName( const rtl::OUString& aFilterName, bool bAllowNoStar = false )
{
    // DBG_PRINT_ENTRY(CLASS_NAME, "shrinkFilterName", "filterName", aFilterName);

    sal_Int32 nBracketEnd = -1;
    rtl::OUString aRealName(aFilterName);

    for( sal_Int32 i = aRealName.getLength() - 1; i > 0; i-- )
    {
        if( aFilterName[i] == ')' )
            nBracketEnd = i;
        else if( aFilterName[i] == '(' )
        {
            sal_Int32 nBracketLen = nBracketEnd - i;
            if( nBracketEnd <= 0 )
                continue;
            if( isFilterString( aFilterName.copy( i + 1, nBracketLen - 1 ), "*." ) )
                aRealName = aRealName.replaceAt( i, nBracketLen + 1, rtl::OUString() );
            else if (bAllowNoStar)
            {
                if( isFilterString( aFilterName.copy( i + 1, nBracketLen - 1 ), ".") )
                    aRealName = aRealName.replaceAt( i, nBracketLen + 1, rtl::OUString() );
            }
        }
    }

    return aRealName;
}


namespace {

    struct FilterTitleMatch : public ::std::unary_function< FilterEntry, bool >
    {
protected:
        const rtl::OUString rTitle;

public:
        FilterTitleMatch( const rtl::OUString& _rTitle ) : rTitle( _rTitle ) { }


        bool operator () ( const FilterEntry& _rEntry )
        {
            bool bMatch;
            if( !_rEntry.hasSubFilters() ) {
                //first try the complete filter name
                rtl::OUString title = _rEntry.getTitle();
                bMatch = title.equals(rTitle);
                if (!bMatch) {
                    //we didn't find a match using the full name, let's give it another
                    //try using the shrunk version
                    rtl::OUString aShrunkName = shrinkFilterName( _rEntry.getTitle() ).trim();
                    bMatch = aShrunkName.equals(rTitle);
                }
            }
            else
                // a filter group -> search the sub filters
                bMatch =
                    ::std::any_of(_rEntry.beginSubFilters(),
                                  _rEntry.endSubFilters(),
                                  *this);

            return bMatch;
        }

        bool operator () ( const UnoFilterEntry& _rEntry )
        {
            rtl::OUString aShrunkName = shrinkFilterName( _rEntry.First );
            bool retVal = aShrunkName.equals(rTitle);
            return retVal;
        }
    };
}

#undef CLASS_NAME
#define CLASS_NAME "FilterHelper"

FilterHelper::FilterHelper()
: m_pFilterList(nullptr)
, m_pFilterNames(nullptr)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

FilterHelper::~FilterHelper()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (nullptr != m_pFilterList) {
        delete m_pFilterList;
    }

    if (nullptr != m_pFilterNames) {
        //we called retain when we added the strings to the list, so we should release them now
        for (NSStringList::iterator iter = m_pFilterNames->begin(); iter != m_pFilterNames->end(); iter++) {
            [*iter release];
        }
        delete m_pFilterNames;
    }

    [pool release];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}


bool FilterHelper::FilterNameExists( const rtl::OUString& rTitle )
{
    bool bRet = false;

    if( m_pFilterList )
        bRet =
            ::std::any_of(m_pFilterList->begin(),
                          m_pFilterList->end(),
                          FilterTitleMatch( rTitle ));

    return bRet;
}


bool FilterHelper::FilterNameExists( const UnoFilterList& _rGroupedFilters )
{
    bool bRet = false;

    if( m_pFilterList )
    {
        const UnoFilterEntry* pStart = _rGroupedFilters.getConstArray();
        const UnoFilterEntry* pEnd = pStart + _rGroupedFilters.getLength();
        for( ; pStart != pEnd; ++pStart )
            if( ::std::any_of(m_pFilterList->begin(),
                              m_pFilterList->end(),
                              FilterTitleMatch( pStart->First ) ) )
                break;

        bRet = (pStart != pEnd);
    }

    return bRet;
}


void FilterHelper::ensureFilterList( const ::rtl::OUString& _rInitialCurrentFilter )
{
    //OSL_TRACE(">>> FilterHelper::%s", __func__);
    if( nullptr == m_pFilterList )
    {
        m_pFilterList = new FilterList;

        // set the first filter to the current filter
        m_aCurrentFilter = _rInitialCurrentFilter;
        OSL_TRACE("ensureFilterList filter:%s", OUStringToOString(m_aCurrentFilter, RTL_TEXTENCODING_UTF8).getStr());
    }
    //OSL_TRACE("<<< FilterHelper::%s", __func__);
}

void FilterHelper::SetCurFilter( const rtl::OUString& rFilter )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "filter", rFilter);

    SolarMutexGuard aGuard;

    if(!m_aCurrentFilter.equals(rFilter))
    {
        m_aCurrentFilter = rFilter;
    }

    //only for output purposes
#if OSL_DEBUG_LEVEL > 1
    FilterList::iterator aFilter = ::std::find_if(m_pFilterList->begin(), m_pFilterList->end(), FilterTitleMatch(m_aCurrentFilter));
    if (aFilter != m_pFilterList->end()) {
        OUStringList suffixes = aFilter->getFilterSuffixList();
        if (!suffixes.empty()) {
            OSL_TRACE("Current active suffixes: ");
            OUStringList::iterator suffIter = suffixes.begin();
            while(suffIter != suffixes.end()) {
                OSL_TRACE("%s", OUStringToOString((*suffIter), RTL_TEXTENCODING_UTF8).getStr());
                suffIter++;
            }
        }
    } else {
        OSL_TRACE("No filter entry was found for that name!");
    }
#endif

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void FilterHelper::SetFilters()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    // set the default filter
    if( m_aCurrentFilter.getLength() > 0 )
    {
        OSL_TRACE( "Setting current filter to %s", OUStringToOString(m_aCurrentFilter, RTL_TEXTENCODING_UTF8).getStr());

        SetCurFilter( m_aCurrentFilter );
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void FilterHelper::appendFilter(const ::rtl::OUString& aTitle, const ::rtl::OUString& aFilterString)
throw( css::lang::IllegalArgumentException, css::uno::RuntimeException ) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", aTitle, "filter", aFilterString);

    SolarMutexGuard aGuard;

    if( FilterNameExists( aTitle ) ) {
        throw css::lang::IllegalArgumentException();
    }

    // ensure that we have a filter list
    ensureFilterList( aTitle );

    // append the filter
    OUStringList suffixList;
    fillSuffixList(suffixList, aFilterString);
    m_pFilterList->push_back(FilterEntry( aTitle, suffixList ) );

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

void FilterHelper::setCurrentFilter( const ::rtl::OUString& aTitle )
throw( css::lang::IllegalArgumentException, css::uno::RuntimeException ) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "aTitle", OUStringToOString(aTitle, RTL_TEXTENCODING_UTF8).getStr());

    SetCurFilter(aTitle);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

::rtl::OUString SAL_CALL FilterHelper::getCurrentFilter(  )
throw( css::uno::RuntimeException ) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::rtl::OUString sReturn = (m_aCurrentFilter);

    DBG_PRINT_EXIT(CLASS_NAME, __func__, OUStringToOString(sReturn, RTL_TEXTENCODING_UTF8).getStr());

    return sReturn;
}

void SAL_CALL FilterHelper::appendFilterGroup( const ::rtl::OUString& sGroupTitle, const css::uno::Sequence< css::beans::StringPair >& aFilters )
throw (css::lang::IllegalArgumentException, css::uno::RuntimeException) {

    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", OUStringToOString(sGroupTitle, RTL_TEXTENCODING_UTF8).getStr());

    SolarMutexGuard aGuard;

    //add a separator if this is not the first group to be added
    bool bPrependSeparator = m_pFilterList != nullptr;

    // ensure that we have a filter list
    ::rtl::OUString sInitialCurrentFilter;
    if( aFilters.getLength() > 0)
        sInitialCurrentFilter = aFilters[0].First;
    ensureFilterList( sInitialCurrentFilter );

    // append the filter
    if (bPrependSeparator) {
        rtl::OUString dash("-");
        OUStringList emptyList;
        m_pFilterList->push_back(FilterEntry(dash, emptyList));
    }

    const css::beans::StringPair* pSubFilters   = aFilters.getConstArray();
    const css::beans::StringPair* pSubFiltersEnd = pSubFilters + aFilters.getLength();
    for( ; pSubFilters != pSubFiltersEnd; ++pSubFilters ) {
        appendFilter(pSubFilters->First, pSubFilters->Second);
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

bool FilterHelper::filenameMatchesFilter(NSString* sFilename)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (m_aCurrentFilter == nullptr) {
        OSL_TRACE("filter name is null");
        return true;
    }

    NSFileManager *manager = [NSFileManager defaultManager];
    NSDictionary* pAttribs = [manager attributesOfItemAtPath: sFilename error: nil];
    if( pAttribs )
    {
        NSObject* pType = [pAttribs objectForKey: NSFileType];
        if( pType && [pType isKindOfClass: [NSString class]] )
        {
            NSString* pT = (NSString*)pType;
            if( [pT isEqualToString: NSFileTypeDirectory]    ||
                [pT isEqualToString: NSFileTypeSymbolicLink] )
                return true;
        }
    }

    FilterList::iterator filter = ::std::find_if(m_pFilterList->begin(), m_pFilterList->end(), FilterTitleMatch(m_aCurrentFilter));
    if (filter == m_pFilterList->end()) {
        OSL_TRACE("filter not found in list");
        return true;
    }

    OUStringList suffixList = filter->getFilterSuffixList();

    {
        rtl::OUString aName = [sFilename OUString];
        rtl::OUString allMatcher(".*");
        for(OUStringList::iterator iter = suffixList.begin(); iter != suffixList.end(); iter++) {
            if (aName.matchIgnoreAsciiCase(*iter, aName.getLength() - (*iter).getLength()) || ((*iter).equals(allMatcher))) {
                return true;
            }
        }
    }

    // might be an alias
    NSString* pResolved = resolveAlias( sFilename );
    if( pResolved )
    {
        bool bResult = filenameMatchesFilter( pResolved );
        [pResolved autorelease];
        if( bResult )
            return true;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return false;
}

FilterList* FilterHelper::getFilterList() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return m_pFilterList;
}

NSStringList* FilterHelper::getFilterNames() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (nullptr == m_pFilterList)
        return nullptr;
    if (nullptr == m_pFilterNames) {
        //build filter names list
        m_pFilterNames = new NSStringList;
        for (FilterList::iterator iter = m_pFilterList->begin(); iter != m_pFilterList->end(); iter++) {
            m_pFilterNames->push_back([[NSString stringWithOUString:iter->getTitle()] retain]);
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return m_pFilterNames;
}

void FilterHelper::SetFilterAtIndex(unsigned index) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "index", index);

    if (m_pFilterList->size() <= index) {
        index = 0;
    }
    FilterEntry entry = m_pFilterList->at(index);
    SetCurFilter(entry.getTitle());

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

int FilterHelper::getCurrentFilterIndex() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    int result = 0;//default to first filter
    if (m_aCurrentFilter.getLength() > 0) {
        int i = 0;
        for (FilterList::iterator iter = m_pFilterList->begin(); iter != m_pFilterList->end(); iter++, i++) {
            rtl::OUString aTitle = iter->getTitle();
            if (m_aCurrentFilter.equals(aTitle)) {
                result = i;
                break;
            } else {
                aTitle = shrinkFilterName(aTitle).trim();
                if (m_aCurrentFilter.equals(aTitle)) {
                    result = i;
                    break;
                }
            }
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__, result);

    return result;
}

OUStringList FilterHelper::getCurrentFilterSuffixList() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    OUStringList retVal;
    if (m_aCurrentFilter.getLength() > 0) {
        for (FilterList::iterator iter = m_pFilterList->begin(); iter != m_pFilterList->end(); iter++) {
            rtl::OUString aTitle = iter->getTitle();
            if (m_aCurrentFilter.equals(aTitle)) {
                retVal = iter->getFilterSuffixList();
                break;
            } else {
                aTitle = shrinkFilterName(aTitle).trim();
                if (m_aCurrentFilter.equals(aTitle)) {
                    retVal = iter->getFilterSuffixList();
                    break;
                }
            }
        }
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return retVal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
