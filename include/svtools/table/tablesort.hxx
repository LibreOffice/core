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

#ifndef INCLUDED_SVTOOLS_TABLE_TABLESORT_HXX
#define INCLUDED_SVTOOLS_TABLE_TABLESORT_HXX

#include <svtools/table/tabletypes.hxx>


namespace svt { namespace table
{



    //= ColumnSortDirection

    enum ColumnSortDirection
    {
        ColumnSortAscending,
        ColumnSortDescending
    };


    //= ColumnSort

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


    //= ITableDataSort

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

    protected:
        ~ITableDataSort() {}
    };


} } // namespace svt::table


#endif // INCLUDED_SVTOOLS_TABLE_TABLESORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
