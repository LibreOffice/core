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

#include <functional>
#include <algorithm>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

#include "CFStringUtilities.hxx"
#include "NSString_OOoAdditions.hxx"
#include "NSURL_OOoAdditions.hxx"

#include "FilterHelper.hxx"

#pragma mark DEFINES
#define CLASS_NAME "FilterEntry"

#pragma mark FilterEntry
//---------------------------------------------------------------------
FilterEntry::FilterEntry( const rtl::OUString& _rTitle, const UnoFilterList& _rSubFilters )
:m_sTitle( _rTitle )
,m_aSubFilters( _rSubFilters )
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", _rTitle);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

//---------------------------------------------------------------------
sal_Bool FilterEntry::hasSubFilters() const
{
//    OSL_TRACE(">>> FilterEntry::%s", __func__);
    sal_Bool bReturn = ( 0 < m_aSubFilters.getLength() );
//    OSL_TRACE("<<< FilterEntry::%s retVal: %d", __func__, bReturn);
    return bReturn;
}

//---------------------------------------------------------------------
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

//=====================================================================

static rtl::OUString
shrinkFilterName( const rtl::OUString aFilterName, bool bAllowNoStar = false )
{
    // DBG_PRINT_ENTRY(CLASS_NAME, "shrinkFilterName", "filterName", aFilterName);

    int i;
    int nBracketLen = -1;
    int nBracketEnd = -1;
    rtl::OUString rFilterName = aFilterName;
    const sal_Unicode *pStr = rFilterName;
    rtl::OUString aRealName = rFilterName;

    for( i = aRealName.getLength() - 1; i > 0; i-- )
    {
        if( pStr[i] == ')' )
            nBracketEnd = i;
        else if( pStr[i] == '(' )
        {
            nBracketLen = nBracketEnd - i;
            if( nBracketEnd <= 0 )
                continue;
            if( isFilterString( rFilterName.copy( i + 1, nBracketLen - 1 ), "*." ) )
                aRealName = aRealName.replaceAt( i, nBracketLen + 1, rtl::OUString() );
            else if (bAllowNoStar)
            {
                if( isFilterString( rFilterName.copy( i + 1, nBracketLen - 1 ), ".") )
                    aRealName = aRealName.replaceAt( i, nBracketLen + 1, rtl::OUString() );
            }
        }
    }

    return aRealName;
}

//------------------------------------------------------------------------------------
namespace {
    //................................................................................
    struct FilterTitleMatch : public ::std::unary_function< FilterEntry, bool >
    {
protected:
        const rtl::OUString rTitle;

public:
        FilterTitleMatch( const rtl::OUString _rTitle ) : rTitle( _rTitle ) { }

        //............................................................................
        bool operator () ( const FilterEntry& _rEntry )
        {
            sal_Bool bMatch;
            if( !_rEntry.hasSubFilters() ) {
                //first try the complete filter name
                rtl::OUString title = _rEntry.getTitle();
                bMatch = ( title.equals(rTitle) );
                if (!bMatch) {
                    //we didn't find a match using the full name, let's give it another
                    //try using the shrunk version
                    rtl::OUString aShrunkName = shrinkFilterName( _rEntry.getTitle() ).trim();
                    bMatch = ( aShrunkName.equals(rTitle) );
                }
            }
            else
                // a filter group -> search the sub filters
                bMatch =
                    _rEntry.endSubFilters() != ::std::find_if(
                                                              _rEntry.beginSubFilters(),
                                                              _rEntry.endSubFilters(),
                                                              *this
                                                              );

            return bMatch ? true : false;
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
: m_pFilterList(NULL)
, m_pFilterNames(NULL)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

FilterHelper::~FilterHelper()
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    NSAutoreleasePool *pool = [NSAutoreleasePool new];

    if (NULL != m_pFilterList) {
        delete m_pFilterList;
    }

    if (NULL != m_pFilterNames) {
        //we called retain when we added the strings to the list, so we should release them now
        for (NSStringList::iterator iter = m_pFilterNames->begin(); iter != m_pFilterNames->end(); iter++) {
            [*iter release];
        }
        delete m_pFilterNames;
    }

    [pool release];

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

//------------------------------------------------------------------------------------
sal_Bool FilterHelper::FilterNameExists( const rtl::OUString rTitle )
{
    sal_Bool bRet = sal_False;

    if( m_pFilterList )
        bRet =
            m_pFilterList->end() != ::std::find_if(
                                                   m_pFilterList->begin(),
                                                   m_pFilterList->end(),
                                                   FilterTitleMatch( rTitle )
                                                   );

    return bRet;
}

//------------------------------------------------------------------------------------
sal_Bool FilterHelper::FilterNameExists( const UnoFilterList& _rGroupedFilters )
{
    sal_Bool bRet = sal_False;

    if( m_pFilterList )
    {
        const UnoFilterEntry* pStart = _rGroupedFilters.getConstArray();
        const UnoFilterEntry* pEnd = pStart + _rGroupedFilters.getLength();
        for( ; pStart != pEnd; ++pStart )
            if( m_pFilterList->end() != ::std::find_if(
                                                        m_pFilterList->begin(),
                                                        m_pFilterList->end(),
                                                        FilterTitleMatch( pStart->First ) ) )
                break;

        bRet = (pStart != pEnd);
    }

    return bRet;
}

//------------------------------------------------------------------------------------
void FilterHelper::ensureFilterList( const ::rtl::OUString& _rInitialCurrentFilter )
{
    //OSL_TRACE(">>> FilterHelper::%s", __func__);
    if( NULL == m_pFilterList )
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

    if(m_aCurrentFilter.equals(rFilter) == false)
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
throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException ) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", aTitle, "filter", aFilterString);

    SolarMutexGuard aGuard;

    if( FilterNameExists( aTitle ) ) {
        throw com::sun::star::lang::IllegalArgumentException();
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
throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException ) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "aTitle", OUStringToOString(aTitle, RTL_TEXTENCODING_UTF8).getStr());

    SetCurFilter(aTitle);

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

::rtl::OUString SAL_CALL FilterHelper::getCurrentFilter(  )
throw( ::com::sun::star::uno::RuntimeException ) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    ::rtl::OUString sReturn = (m_aCurrentFilter);

    DBG_PRINT_EXIT(CLASS_NAME, __func__, OUStringToOString(sReturn, RTL_TEXTENCODING_UTF8).getStr());

    return sReturn;
}

void SAL_CALL FilterHelper::appendFilterGroup( const ::rtl::OUString& sGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aFilters )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) {

    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "title", OUStringToOString(sGroupTitle, RTL_TEXTENCODING_UTF8).getStr());

    SolarMutexGuard aGuard;

    //add a separator if this is not the first group to be added
    sal_Bool bPrependSeparator = m_pFilterList != NULL;

    // ensure that we have a filter list
    ::rtl::OUString sInitialCurrentFilter;
    if( aFilters.getLength() > 0)
        sInitialCurrentFilter = aFilters[0].First;
    ensureFilterList( sInitialCurrentFilter );

    // append the filter
    if (bPrependSeparator) {
        rtl::OUString dash(RTL_CONSTASCII_USTRINGPARAM("-"));
        OUStringList emptyList;
        m_pFilterList->push_back(FilterEntry(dash, emptyList));
    }

    const com::sun::star::beans::StringPair* pSubFilters   = aFilters.getConstArray();
    const com::sun::star::beans::StringPair* pSubFiltersEnd = pSubFilters + aFilters.getLength();
    for( ; pSubFilters != pSubFiltersEnd; ++pSubFilters ) {
        appendFilter(pSubFilters->First, pSubFilters->Second);
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);
}

sal_Bool FilterHelper::filenameMatchesFilter(NSString* sFilename)
{
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (m_aCurrentFilter == NULL) {
        OSL_TRACE("filter name is null");
        return sal_True;
    }

    NSFileManager *manager = [NSFileManager defaultManager];
    NSDictionary* pAttribs = [manager fileAttributesAtPath: sFilename traverseLink: NO];
    if( pAttribs )
    {
        NSObject* pType = [pAttribs objectForKey: NSFileType];
        if( pType && [pType isKindOfClass: [NSString class]] )
        {
            NSString* pT = (NSString*)pType;
            if( [pT isEqualToString: NSFileTypeDirectory]    ||
                [pT isEqualToString: NSFileTypeSymbolicLink] )
                return sal_True;
        }
    }

    FilterList::iterator filter = ::std::find_if(m_pFilterList->begin(), m_pFilterList->end(), FilterTitleMatch(m_aCurrentFilter));
    if (filter == m_pFilterList->end()) {
        OSL_TRACE("filter not found in list");
        return sal_True;
    }

    OUStringList suffixList = filter->getFilterSuffixList();

    {
        rtl::OUString aName = [sFilename OUString];
        rtl::OUString allMatcher(RTL_CONSTASCII_USTRINGPARAM(".*"));
        for(OUStringList::iterator iter = suffixList.begin(); iter != suffixList.end(); iter++) {
            if (aName.matchIgnoreAsciiCase(*iter, aName.getLength() - (*iter).getLength()) || ((*iter).equals(allMatcher))) {
                return sal_True;
            }
        }
    }

    // might be an alias
    NSString* pResolved = resolveAlias( sFilename );
    if( pResolved )
    {
        sal_Bool bResult = filenameMatchesFilter( pResolved );
        [pResolved autorelease];
        if( bResult )
            return sal_True;
    }

    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return sal_False;
}

FilterList* FilterHelper::getFilterList() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);
    DBG_PRINT_EXIT(CLASS_NAME, __func__);

    return m_pFilterList;
}

NSStringList* FilterHelper::getFilterNames() {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__);

    if (NULL == m_pFilterList)
        return NULL;
    if (NULL == m_pFilterNames) {
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

void FilterHelper::fillSuffixList(OUStringList& aSuffixList, const ::rtl::OUString& suffixString) {
    DBG_PRINT_ENTRY(CLASS_NAME, __func__, "aSuffixList", suffixString);

    sal_Int32 nIndex = 0;
    do {
        rtl::OUString aToken = suffixString.getToken( 0, ';', nIndex );
        aSuffixList.push_back(aToken.copy(1));
    } while ( nIndex >= 0 );

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
