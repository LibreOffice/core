/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sheetcellrangemap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

#ifndef OOX_XLS_SHEETCELLRANGEMAP_HXX
#define OOX_XLS_SHEETCELLRANGEMAP_HXX

#include <hash_map>
#include <map>
#include <vector>
#include <sal/types.h>

namespace com { namespace sun { namespace star { namespace table {
    struct CellRangeAddress;
    struct CellAddress;
}}}}

namespace oox {
namespace xls {

/** This class is used to simulate a cell address "hit" against a set of
    cell range addresses.  All addresses are 3-dimensional (sheet, row,
    column). */
class SheetCellRangeMap
{
public:
    explicit SheetCellRangeMap();
    ~SheetCellRangeMap() throw();

    /** Add a new cell range to the map. */
    void addCellRange( const ::com::sun::star::table::CellRangeAddress& aRangeAddr );

    /** Test if a given cell address overlaps one of the registered cell
        ranges. */
    bool isOverlapping( const ::com::sun::star::table::CellAddress& aCellAddr ) const;

public:
    typedef ::std::vector<size_t>                       AreaIdSetType;
    typedef ::std::map<sal_Int32, AreaIdSetType>        EndAreaIdMapType;
    typedef ::std::map<sal_Int32, EndAreaIdMapType>     StartEndMapType;

private:
    struct SheetSet
    {
        SheetCellRangeMap::StartEndMapType maColRanges;
        SheetCellRangeMap::StartEndMapType maRowRanges;
    };
    typedef ::std::hash_map< sal_Int16, SheetSet > SheetMapType;

    void addRange( StartEndMapType& rRangeMap, sal_Int32 nStart, sal_Int32 nEnd, size_t nAreaId );
    bool expandSearch( const EndAreaIdMapType& rEndMap, const ::com::sun::star::table::CellAddress& aCellAddr, bool bColumn ) const;
    bool searchColumns( const SheetSet& rSheet, const ::com::sun::star::table::CellAddress& aCellAddr ) const;

private:
    SheetMapType  maSheetMap;
    ::std::vector< ::com::sun::star::table::CellRangeAddress > maAreas;
};

} // namespace xls
} // namespace oox

#endif
