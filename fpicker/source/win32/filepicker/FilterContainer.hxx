/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FilterContainer.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _FILTER_CONTAINER_HXX_
#define _FILTER_CONTAINER_HXX_

#include <sal/types.h>
#include <rtl/ustring.hxx>

#include <vector>

//------------------------------------------------------
// helper class, only useable by OFilterContainer
//------------------------------------------------------

class CFilterContainer
{
public:
    // defines a filter entry which is made of a name and a filter value
    // e.g. 'Text *.txt'
    typedef std::pair< rtl::OUString, rtl::OUString > FILTER_ENTRY_T;

public:
    explicit CFilterContainer( sal_Int32 initSize = 0 );

    // add a new filter
    // returns true if the filter was successfully added
    // returns false if duplicates are not allowed and
    // the filter is already in the container
    sal_Bool SAL_CALL addFilter(
        const ::rtl::OUString& aName,
        const ::rtl::OUString& aFilter,
        sal_Bool bAllowDuplicates = sal_False );

    // delete the specified filter returns true on
    // success and false if the filter was not found
    sal_Bool SAL_CALL delFilter( const ::rtl::OUString& aName );

    // the number of filter already added
    sal_Int32 SAL_CALL numFilter( );

    // clear all entries
    void SAL_CALL empty( );

    // retrieve a filter from the container both methods
    // return true on success and false if the specified
    // filter was not found
    sal_Bool SAL_CALL getFilter( const ::rtl::OUString& aName, ::rtl::OUString& theFilter ) const;
    sal_Bool SAL_CALL getFilter( sal_Int32 aIndex, ::rtl::OUString& theFilter ) const;

    // returns the position of the specified filter or -1
    // if the filter was not found
    sal_Int32 SAL_CALL getFilterPos( const ::rtl::OUString& aName ) const;

    // starts enumerating the filter in the container
    void SAL_CALL beginEnumFilter( );

    // returns true if another filter has been retrieved
    sal_Bool SAL_CALL getNextFilter( FILTER_ENTRY_T& nextFilterEntry );

    // cache current filter
    void SAL_CALL setCurrentFilter( const ::rtl::OUString& aName );

    // returns cached current filter
    ::rtl::OUString SAL_CALL getCurrentFilter() const;

protected:
    typedef std::vector< FILTER_ENTRY_T > FILTER_VECTOR_T;

private:
    // prevent copy and assignment
    CFilterContainer( const CFilterContainer& );
    CFilterContainer& SAL_CALL operator=( const CFilterContainer& );

    sal_Int32 SAL_CALL getFilterTagPos( const ::rtl::OUString& aName ) const;

private:
    FILTER_VECTOR_T                 m_vFilters;
    FILTER_VECTOR_T::const_iterator m_iter;
    sal_Bool                        m_bIterInitialized;
    ::rtl::OUString                 m_sCurrentFilter;
};

//----------------------------------------------------------------
// a helper function to create a filter buffer in the format
// the Win32 API requires, e.g. "Text\0*.txt\0Doc\0*.doc;*xls\0\0"
//----------------------------------------------------------------

rtl::OUString SAL_CALL makeWinFilterBuffer( CFilterContainer& aFilterContainer );

#endif
