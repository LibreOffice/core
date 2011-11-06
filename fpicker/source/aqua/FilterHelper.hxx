/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _FILTERHELPER_HXX_
#define _FILTERHELPER_HXX_

// #ifndef _COM_SUN_STAR_UNO_TYPE_HXX_
// #include <com/sun/star/uno/Type.hxx>
// #endif

// #ifndef INCLUDED_CPPU_UNOTYPE_HXX
// #include <cppu/unotype.hxx>
// #endif

// #ifndef _TYPELIB_TYPECLASS_H_
// #include "typelib/typeclass.h"
// #endif

// #ifndef _TYPELIB_TYPEDESCRIPTION_H_
// #include "typelib/typedescription.h"
// #endif

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HXX_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HXX_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

// #ifndef _RTL_USTRING_H_
// #include <rtl/ustring.hxx>
// #endif

#include <list>
#include <vector>

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

typedef ::com::sun::star::beans::StringPair UnoFilterEntry;
typedef ::com::sun::star::uno::Sequence< UnoFilterEntry >   UnoFilterList;  // can be transported more effectively
typedef ::std::list<NSString *> NSStringList;
typedef ::std::list<rtl::OUString> OUStringList;

struct FilterEntry
{
protected:
    rtl::OUString       m_sTitle;
    OUStringList        m_sFilterSuffixList;
    UnoFilterList       m_aSubFilters;

public:
    FilterEntry( const rtl::OUString& _rTitle, const OUStringList _rFilter )
    : m_sTitle( _rTitle )
    , m_sFilterSuffixList( _rFilter )
    {
    }

    FilterEntry( const rtl::OUString& _rTitle, const UnoFilterList& _rSubFilters );

    rtl::OUString       getTitle() const { return m_sTitle; }
    OUStringList    getFilterSuffixList() const { return m_sFilterSuffixList; }

    /// determines if the filter has sub filter (i.e., the filter is a filter group in real)
    sal_Bool        hasSubFilters( ) const;

    /** retrieves the filters belonging to the entry
        @return
        the number of sub filters
        */
    sal_Int32       getSubFilters( UnoFilterList& _rSubFilterList );

    // helpers for iterating the sub filters
    const UnoFilterEntry*   beginSubFilters() const { return m_aSubFilters.getConstArray(); }
    const UnoFilterEntry*   endSubFilters() const { return m_aSubFilters.getConstArray() + m_aSubFilters.getLength(); }
};

typedef ::std::vector < FilterEntry >       FilterList;

class FilterHelper {

public:
    FilterHelper();
    virtual ~FilterHelper();

    //XFilterManager delegates
    void SAL_CALL appendFilter( const ::rtl::OUString& aTitle, const ::rtl::OUString& aFilter )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    void SAL_CALL setCurrentFilter( const ::rtl::OUString& aTitle )
        throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

    ::rtl::OUString SAL_CALL getCurrentFilter(  )
        throw( ::com::sun::star::uno::RuntimeException );

    //XFilterGroupManager delegates
    void SAL_CALL appendFilterGroup( const ::rtl::OUString& sGroupTitle, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair >& aFilters )
        throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);


    //accessor
    FilterList* getFilterList();
    NSStringList* getFilterNames();

    //misc
    void SetCurFilter( const rtl::OUString& rFilter );
    void SetFilterAtIndex(unsigned index);
    OUStringList getCurrentFilterSuffixList();
    int getCurrentFilterIndex();
    void SetFilters();
    sal_Bool filenameMatchesFilter(NSString * sFilename);

private:
    FilterList *m_pFilterList;
    rtl::OUString m_aCurrentFilter;
    NSStringList *m_pFilterNames;

    int implAddFilter( const rtl::OUString rFilter, const OUStringList rSuffixList);
    int implAddFilterGroup( const rtl::OUString rFilter,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::StringPair>& _rFilters );

    sal_Bool FilterNameExists( const rtl::OUString rTitle );
    sal_Bool FilterNameExists( const UnoFilterList& _rGroupedFilters );

    void ensureFilterList( const ::rtl::OUString& _rInitialCurrentFilter );

    void fillSuffixList(OUStringList& aSuffixList, const ::rtl::OUString& suffixString);

};

#endif //_FILTERHELPER_HXX_
