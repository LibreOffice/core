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

#ifndef _FILTERHELPER_HXX_
#define _FILTERHELPER_HXX_

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <com/sun/star/uno/RuntimeException.hpp>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
