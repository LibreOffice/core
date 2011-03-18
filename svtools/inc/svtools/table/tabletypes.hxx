/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef SVTOOLS_INC_TABLE_TABLETYPES_HXX
#define SVTOOLS_INC_TABLE_TABLETYPES_HXX

#include <sal/types.h>

//........................................................................
namespace svt { namespace table
{
//........................................................................
    /// a value denoting the size of a table
    typedef sal_Int32   TableSize;

    /// a value denoting a column position within a table
    typedef sal_Int32   ColPos;
    /// a value denoting a row position within a table
    typedef sal_Int32   RowPos;

    /** a value denoting an arbitrary coordinate value of a position within
        a table

        Values of this type are guaranteed to be large enough to hold column
        positions as well as row positions.
    */
    typedef sal_Int32   AnyPos;

    typedef sal_Int32   TableMetrics;

/// denotes the column containing the row headers
#define COL_ROW_HEADERS         ((::svt::table::ColPos)-1)
/// denotes the row containing the column headers
#define ROW_COL_HEADERS         ((::svt::table::RowPos)-1)

/// denotes an invalid column index
#define COL_INVALID             ((::svt::table::ColPos)-2)
/// denotes an invalid row index
#define ROW_INVALID             ((::svt::table::RowPos)-2)


//........................................................................
} } // namespace svt::table
//........................................................................

#endif // SVTOOLS_INC_TABLE_TABLETYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
