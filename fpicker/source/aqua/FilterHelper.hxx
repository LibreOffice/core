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

#ifndef INCLUDED_FPICKER_SOURCE_AQUA_FILTERHELPER_HXX
#define INCLUDED_FPICKER_SOURCE_AQUA_FILTERHELPER_HXX

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <com/sun/star/uno/RuntimeException.hpp>

#include <list>
#include <vector>

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

typedef css::beans::StringPair UnoFilterEntry;
typedef css::uno::Sequence< UnoFilterEntry >   UnoFilterList;  // can be transported more effectively
typedef ::std::list<NSString *> NSStringList;
typedef ::std::list<OUString> OUStringList;

struct FilterEntry
{
protected:
    OUString            m_sTitle;
    OUStringList        m_sFilterSuffixList;
    UnoFilterList       m_aSubFilters;

public:
    FilterEntry( const OUString& _rTitle, const OUStringList _rFilter )
    : m_sTitle( _rTitle )
    , m_sFilterSuffixList( _rFilter )
    {
    }

    FilterEntry( const OUString& _rTitle, const UnoFilterList& _rSubFilters );

    OUString const & getTitle() const { return m_sTitle; }
    OUStringList const & getFilterSuffixList() const { return m_sFilterSuffixList; }

    /// determines if the filter has sub filter (i.e., the filter is a filter group in real)
    bool        hasSubFilters( ) const;

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
    void SAL_CALL appendFilter( const OUString& aTitle, const OUString& aFilter )
        /*throw( css::lang::IllegalArgumentException, css::uno::RuntimeException )*/ ;

    void SAL_CALL setCurrentFilter( const OUString& aTitle )
        /*throw( css::lang::IllegalArgumentException, css::uno::RuntimeException )*/ ;

    OUString SAL_CALL getCurrentFilter(  )
        /*throw( css::uno::RuntimeException )*/ ;

    //XFilterGroupManager delegates
    void SAL_CALL appendFilterGroup( const OUString& sGroupTitle, const css::uno::Sequence< css::beans::StringPair >& aFilters )
        /*throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)*/ ;

    //accessor
    FilterList* getFilterList();
    NSStringList* getFilterNames();

    //misc
    void SetCurFilter( const OUString& rFilter );
    void SetFilterAtIndex(unsigned index);
    OUStringList getCurrentFilterSuffixList();
    int getCurrentFilterIndex();
    void SetFilters();
    bool filenameMatchesFilter(NSString * sFilename);

private:
    FilterList *m_pFilterList;
    OUString m_aCurrentFilter;
    NSStringList *m_pFilterNames;

    bool FilterNameExists( const OUString& rTitle );
    bool FilterNameExists( const UnoFilterList& _rGroupedFilters );

    void ensureFilterList( const OUString& _rInitialCurrentFilter );
};

#endif // INCLUDED_FPICKER_SOURCE_AQUA_FILTERHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
