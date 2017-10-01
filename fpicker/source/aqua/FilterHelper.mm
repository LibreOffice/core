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

#pragma mark FilterEntry

FilterEntry::FilterEntry( const rtl::OUString& _rTitle, const UnoFilterList& _rSubFilters )
:m_sTitle( _rTitle )
,m_aSubFilters( _rSubFilters )
{
}


bool FilterEntry::hasSubFilters() const
{
    bool bReturn = ( 0 < m_aSubFilters.getLength() );

    return bReturn;
}


sal_Int32 FilterEntry::getSubFilters( UnoFilterList& _rSubFilterList )
{
    _rSubFilterList = m_aSubFilters;
    sal_Int32 nReturn = m_aSubFilters.getLength();

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

    struct FilterTitleMatch
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

FilterHelper::FilterHelper()
: m_pFilterList(nullptr)
, m_pFilterNames(nullptr)
{
}

FilterHelper::~FilterHelper()
{
    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (nullptr != m_pFilterList) {
        delete m_pFilterList;
    }

    if (nullptr != m_pFilterNames) {
        //we called retain when we added the strings to the list, so we should release them now
        for (NSStringList::iterator iter = m_pFilterNames->begin(); iter != m_pFilterNames->end(); ++iter) {
            [*iter release];
        }
        delete m_pFilterNames;
    }

    [pool release];
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
    if( nullptr == m_pFilterList )
    {
        m_pFilterList = new FilterList;

        // set the first filter to the current filter
        m_aCurrentFilter = _rInitialCurrentFilter;
    }
}

void FilterHelper::SetCurFilter( const rtl::OUString& rFilter )
{
    SolarMutexGuard aGuard;

    if(!m_aCurrentFilter.equals(rFilter))
    {
        m_aCurrentFilter = rFilter;
    }

}

void FilterHelper::SetFilters()
{
    // set the default filter
    if( m_aCurrentFilter.getLength() > 0 )
    {
        SetCurFilter( m_aCurrentFilter );
    }
}

void FilterHelper::appendFilter(const ::rtl::OUString& aTitle, const ::rtl::OUString& aFilterString)
{
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
}

void FilterHelper::setCurrentFilter( const ::rtl::OUString& aTitle )
{
    SetCurFilter(aTitle);
}

::rtl::OUString SAL_CALL FilterHelper::getCurrentFilter(  )
{
    ::rtl::OUString sReturn = (m_aCurrentFilter);

    return sReturn;
}

void SAL_CALL FilterHelper::appendFilterGroup( const ::rtl::OUString& /* sGroupTitle */, const css::uno::Sequence< css::beans::StringPair >& aFilters )
{
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
        OUStringList emptyList;
        m_pFilterList->push_back(FilterEntry("-", emptyList));
    }

    const css::beans::StringPair* pSubFilters   = aFilters.getConstArray();
    const css::beans::StringPair* pSubFiltersEnd = pSubFilters + aFilters.getLength();
    for( ; pSubFilters != pSubFiltersEnd; ++pSubFilters ) {
        appendFilter(pSubFilters->First, pSubFilters->Second);
    }
}

bool FilterHelper::filenameMatchesFilter(NSString* sFilename)
{
    if (m_aCurrentFilter.isEmpty()) {
        SAL_WARN("fpicker", "filter name is empty");
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
        SAL_WARN("fpicker", "filter not found in list");
        return true;
    }

    OUStringList suffixList = filter->getFilterSuffixList();

    {
        rtl::OUString aName = [sFilename OUString];
        for(OUStringList::iterator iter = suffixList.begin(); iter != suffixList.end(); ++iter) {
            if (*iter == ".*" || aName.endsWithIgnoreAsciiCase(*iter)) {
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

    return false;
}

FilterList* FilterHelper::getFilterList()
{
    return m_pFilterList;
}

NSStringList* FilterHelper::getFilterNames()
{
    if (nullptr == m_pFilterList)
        return nullptr;
    if (nullptr == m_pFilterNames) {
        //build filter names list
        m_pFilterNames = new NSStringList;
        for (FilterList::iterator iter = m_pFilterList->begin(); iter != m_pFilterList->end(); ++iter) {
            m_pFilterNames->push_back([[NSString stringWithOUString:iter->getTitle()] retain]);
        }
    }

    return m_pFilterNames;
}

void FilterHelper::SetFilterAtIndex(unsigned index)
{
    if (m_pFilterList->size() <= index) {
        index = 0;
    }
    FilterEntry entry = m_pFilterList->at(index);
    SetCurFilter(entry.getTitle());
}

int FilterHelper::getCurrentFilterIndex()
{
    int result = 0;//default to first filter
    if (m_aCurrentFilter.getLength() > 0) {
        int i = 0;
        for (FilterList::iterator iter = m_pFilterList->begin(); iter != m_pFilterList->end(); ++iter, ++i) {
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

    return result;
}

OUStringList FilterHelper::getCurrentFilterSuffixList()
{
    OUStringList retVal;
    if (m_aCurrentFilter.getLength() > 0) {
        for (FilterList::iterator iter = m_pFilterList->begin(); iter != m_pFilterList->end(); ++iter) {
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

    return retVal;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
