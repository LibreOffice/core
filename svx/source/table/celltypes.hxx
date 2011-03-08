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

#ifndef _SVX_CELLTYPES_HXX_
#define _SVX_CELLTYPES_HXX_

#include <rtl/ref.hxx>
#include <vector>

namespace sdr { namespace table {

class Cell;
class TableModel;
class TableRow;
class TableColumn;
class TableRows;
class TableColumns;
typedef rtl::Reference< Cell > CellRef;
typedef rtl::Reference< TableModel > TableModelRef;
typedef rtl::Reference< TableRow > TableRowRef;
typedef rtl::Reference< TableColumn > TableColumnRef;
typedef rtl::Reference< TableRows > TableRowsRef;
typedef rtl::Reference< TableColumns > TableColumnsRef;
typedef std::vector< CellRef > CellVector;
typedef std::vector< TableRowRef > RowVector;
typedef std::vector< TableColumnRef > ColumnVector;

class TableDesignUser
{
public:
    virtual bool isInUse() = 0;
};

template< typename T >
class RangeIterator
{
public:
    /** creates an iterator from rStart (including) to rEnd (excluding) if
        bForeward is true or from nEnd (excluding to nStart (including).
        rStart must be <= rEnd.
    */
    RangeIterator( const T& rStart, const T& rEnd, bool bForeward = true )
    {
        if( bForeward )
        {
            maIter = rStart;
            maEnd = rEnd;
        }
        else
        {
            maIter = rEnd-1;
            maEnd = rStart-1;
        }
    }

    /* returns true if the next call to next() will return true also. */
    bool hasNext() const
    {
        return maIter != maEnd;
    }

    /* iterates in the configured direction and returns true if rValue
       now contains a valid positon in the range of this iterator */
    bool next( T& rValue )
    {
        if( maIter == maEnd )
            return false;

        rValue = maIter;
        if( maIter < maEnd )
            maIter++;
        else
            maIter--;
        return true;
    }

private:
    T maEnd;
    T maIter;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
