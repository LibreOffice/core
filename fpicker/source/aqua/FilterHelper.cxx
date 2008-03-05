/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FilterHelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:35:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <functional>
#include <algorithm>

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

// #ifndef _OSL_DIAGNOSE_H_
// #include <osl/diagnose.h>
// #endif

#ifndef _CFSTRINGUTILITIES_HXX_
#include "CFStringUtilities.hxx"
#endif

#ifndef _NSSTRING_OOOADDITIONS_HXX_
#include "NSString_OOoAdditions.hxx"
#endif

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

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

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

    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    //add a separator if this is not the first group to be added
    sal_Bool bPrependSeparator = m_pFilterList != NULL;

    // ensure that we have a filter list
    ::rtl::OUString sInitialCurrentFilter;
    if( aFilters.getLength() > 0)
        sInitialCurrentFilter = aFilters[0].First;
    ensureFilterList( sInitialCurrentFilter );

    // append the filter
    if (bPrependSeparator) {
        rtl::OUString dash = rtl::OUString::createFromAscii("-");
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

//    OSL_TRACE("filter event handler called");

    if (m_aCurrentFilter == NULL) {
        OSL_TRACE("filter name is null");
        return sal_True;
    }

    NSFileManager *manager = [NSFileManager defaultManager];
    MacOSBOOL bDir = NO;
    if ([manager fileExistsAtPath:sFilename isDirectory:&bDir] && bDir == YES) {
//        OSL_TRACE(" folder");
        return sal_True;
    }
//    OSL_TRACE(" file");

    FilterList::iterator filter = ::std::find_if(m_pFilterList->begin(), m_pFilterList->end(), FilterTitleMatch(m_aCurrentFilter));
    if (filter == m_pFilterList->end()) {
        OSL_TRACE("filter not found in list");
        return sal_True;
    }

    OUStringList suffixList = filter->getFilterSuffixList();

    {
//        OSL_TRACE(" starting to work");
        rtl::OUString aName = [sFilename OUString];
        rtl::OUString allMatcher = rtl::OUString::createFromAscii(".*");
        for(OUStringList::iterator iter = suffixList.begin(); iter != suffixList.end(); iter++) {
            if (aName.matchIgnoreAsciiCase(*iter, aName.getLength() - (*iter).getLength()) || ((*iter).equals(allMatcher))) {
                return sal_True;
            }
        }
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
