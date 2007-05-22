/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLRangeHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 16:33:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

//!!
//!! This file is an exact copy of the same file in chart2 project
//!!

#ifndef XMLRANGEHELPER_HXX
#define XMLRANGEHELPER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//namespace chart
//{
namespace XMLRangeHelper
{

struct Cell
{
    sal_Int32 nColumn;
    sal_Int32 nRow;
    bool bRelativeColumn;
    bool bRelativeRow;
    bool bIsEmpty;

    Cell() :
            nColumn(0),
            nRow(0),
            bRelativeColumn(false),
            bRelativeRow(false),
            bIsEmpty(true)
    {}

    inline bool empty() const { return bIsEmpty; }
};

struct CellRange
{
    Cell aUpperLeft;
    Cell aLowerRight;
    ::rtl::OUString aTableName;
};

CellRange getCellRangeFromXMLString( const ::rtl::OUString & rXMLString );

::rtl::OUString getXMLStringFromCellRange( const CellRange & rRange );


} //  namespace XMLRangeHelper
//} //  namespace chart

// XMLRANGEHELPER_HXX
#endif
