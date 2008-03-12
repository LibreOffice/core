/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: celltypes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:01:27 $
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

