/*************************************************************************
 *
 *  $RCSfile: FilterContainer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:52:26 $
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

#ifndef _FILTER_CONTAINER_HXX_
#define _FILTER_CONTAINER_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

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
};

//----------------------------------------------------------------
// a helper function to create a filter buffer in the format
// the Win32 API requires, e.g. "Text\0*.txt\0Doc\0*.doc;*xls\0\0"
//----------------------------------------------------------------

rtl::OUString SAL_CALL makeWinFilterBuffer( CFilterContainer& aFilterContainer );

#endif
