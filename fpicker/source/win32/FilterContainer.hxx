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

#pragma once

#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <vector>


// helper class, only usable by OFilterContainer


class CFilterContainer
{
public:
    // defines a filter entry which is made of a name and a filter value
    // e.g. 'Text *.txt'
    typedef std::pair< OUString, OUString > FILTER_ENTRY_T;

public:
    explicit CFilterContainer( sal_Int32 initSize = 0 );

    // add a new filter
    // returns true if the filter was successfully added
    // returns false if duplicates are not allowed and
    // the filter is already in the container
    bool addFilter(
        const OUString& aName,
        const OUString& aFilter,
        bool bAllowDuplicates = false );

    // delete the specified filter returns true on
    // success and false if the filter was not found
    bool delFilter( const OUString& aName );

    // the number of filter already added
    sal_Int32 numFilter( );

    // clear all entries
    void empty( );

    // retrieve a filter from the container. These methods
    // return true on success and false if the specified
    // filter was not found
    bool getFilterByName(const OUString& aName, OUString& theFilter) const;
    bool getFilterByIndex(sal_Int32 aIndex, OUString& theFilter) const;
    bool getFilterNameByIndex(sal_Int32 aIndex, OUString& theName) const;

    // returns the position of the specified filter or -1
    // if the filter was not found
    sal_Int32 getFilterPos( const OUString& aName ) const;

    // starts enumerating the filter in the container
    void beginEnumFilter( );

    // returns true if another filter has been retrieved
    bool getNextFilter( FILTER_ENTRY_T& nextFilterEntry );

    // cache current filter
    void setCurrentFilter( const OUString& aName );

    // returns cached current filter
    OUString getCurrentFilter() const;

protected:
    typedef std::vector< FILTER_ENTRY_T > FILTER_VECTOR_T;

private:
    // prevent copy and assignment
    CFilterContainer( const CFilterContainer& );
    CFilterContainer& SAL_CALL operator=( const CFilterContainer& );

    sal_Int32 getFilterTagPos( const OUString& aName ) const;

private:
    FILTER_VECTOR_T                 m_vFilters;
    FILTER_VECTOR_T::const_iterator m_iter;
    bool                            m_bIterInitialized;
    OUString                 m_sCurrentFilter;
};


// a helper function to create a filter buffer in the format
// the Win32 API requires, e.g. "Text\0*.txt\0Doc\0*.doc;*xls\0\0"


OUString makeWinFilterBuffer( CFilterContainer& aFilterContainer );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
