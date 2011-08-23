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
#ifndef _SCH_CELLRANGES_HXX_
#define _SCH_CELLRANGES_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#include <vector>
namespace binfilter {

// Structures for CellRangeAddresses from container documents (Writer/Calc)
// ========================================================================

struct SchSingleCell
{
    sal_Int32 mnColumn, mnRow;
    sal_Bool  mbRelativeColumn : 1;     // if true the $-sign before the address is skipped in the XML format
    sal_Bool  mbRelativeRow    : 1;     // see above

    SchSingleCell() :
            mnColumn( -1 ),
            mnRow( -1 ),
            mbRelativeColumn( sal_False ),
            mbRelativeRow( sal_False )
        {}
};

struct SchCellAddress
{
    ::std::vector< SchSingleCell > maCells;      // to handle subtables (Writer)
};

struct SchCellRangeAddress
{
    SchCellAddress maUpperLeft;
    SchCellAddress maLowerRight;    // range is one cell if this vector is empty (or equal to aUpperLeft)
    ::rtl::OUString msTableName;
    sal_Int32 mnTableNumber;        // to reconstruct old address string in Calc

    SchCellRangeAddress() :
            mnTableNumber( -1 )
        {}
};

struct SchChartRange
{
    ::std::vector< SchCellRangeAddress > maRanges;    // a number of ranges describing the complete source data for the chart
    sal_Bool mbFirstColumnContainsLabels;
    sal_Bool mbFirstRowContainsLabels;
    sal_Bool mbKeepCopyOfData;                        // if this is true, the chart will export a table containing the current data

    SchChartRange() :
            mbFirstColumnContainsLabels( sal_False ),
            mbFirstRowContainsLabels( sal_False ),
            mbKeepCopyOfData( sal_True )
        {}
};

} //namespace binfilter
#endif	// _SCH_CELLRANGES_HXX_
