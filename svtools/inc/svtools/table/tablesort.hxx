/*************************************************************************
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

#ifndef SVTOOLS_TABLESORT_HXX
#define SVTOOLS_TABLESORT_HXX

#include "svtools/table/tabletypes.hxx"

//......................................................................................................................
namespace svt { namespace table
{
//......................................................................................................................

    //==================================================================================================================
    //= ColumnSortDirection
    //==================================================================================================================
    enum ColumnSortDirection
    {
        ColumnSortAscending,
        ColumnSortDescending
    };

    //==================================================================================================================
    //= ColumnSort
    //==================================================================================================================
    struct ColumnSort
    {
        ColPos              nColumnPos;
        ColumnSortDirection eSortDirection;

        ColumnSort()
            :nColumnPos( COL_INVALID )
            ,eSortDirection( ColumnSortAscending )
        {
        }

        ColumnSort( ColPos const i_columnPos, ColumnSortDirection const i_sortDirection )
            :nColumnPos( i_columnPos )
            ,eSortDirection( i_sortDirection )
        {
        }
    };

    //==================================================================================================================
    //= ITableDataSort
    //==================================================================================================================
    /** provides sorting functionality for the datta underlying an ITableModel
    */
    class SAL_NO_VTABLE ITableDataSort
    {
    public:
        /** sorts the rows in the model by the given column's data, in the given direction.
        */
        virtual void        sortByColumn( ColPos const i_column, ColumnSortDirection const i_sortDirection ) = 0;

        /** retrieves the current sort order of the data

            If the <code>nColumnIndex</code> member of the returned srtructure is <code>COL_INVALID</code>, then
            the data is currently not sorted.
        */
        virtual ColumnSort  getCurrentSortOrder() const = 0;
    };

//......................................................................................................................
} } // namespace svt::table
//......................................................................................................................

#endif // SVTOOLS_TABLESORT_HXX
