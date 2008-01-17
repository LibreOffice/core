/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pivottablebuffer.hxx,v $
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

#ifndef OOX_XLS_PIVOTTABLEBUFFER_HXX
#define OOX_XLS_PIVOTTABLEBUFFER_HXX

#include <hash_map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include "oox/xls/sheetcellrangemap.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

struct PivotCacheField
{
    ::rtl::OUString                     maName;
    ::std::vector< ::rtl::OUString >    maItems;
};

typedef ::std::vector< PivotCacheField >    PivotCacheFields;

// ----------------------------------------------------------------------------

struct PivotCacheData
{
    enum SourceType
    {
        WORKSHEET,
        EXTERNAL
    };
    SourceType              meSourceType;

    struct BaseSource {};

    struct WorksheetSource : public BaseSource
    {
        ::rtl::OUString maSrcRange;
        ::rtl::OUString maSheetName;
    };

    struct ExternalSource : public BaseSource
    {
    };

    typedef ::boost::shared_ptr< BaseSource >   SourceRef;
    SourceRef                                   mpSourceProp;

    PivotCacheFields                            maFields;

    PivotCacheData();

    WorksheetSource* getWorksheetSource() const;
    ExternalSource*  getExternalSource() const;
};

// ============================================================================

struct PivotTableField
{
    enum AxisType
    {
        COLUMN,
        ROW,
        PAGE,
        VALUES
    };

    AxisType    meAxis;

    ::std::vector< sal_uInt32 > maItems;    /// IDs pointing to corresponding items in pivot cache field.
    bool mbDataField;

    PivotTableField();
};

typedef ::std::vector< PivotTableField >    PivotTableFields;

// ----------------------------------------------------------------------------

struct PivotTableData
{
    ::com::sun::star::table::CellRangeAddress   maRange;    /// destination cell range
    sal_uInt32          mnCacheId;                          /// cache ID

    PivotTableFields    maFields;

    PivotTableData();
};

// ----------------------------------------------------------------------------

class PivotTableBuffer : public WorkbookHelper
{
public:
    explicit            PivotTableBuffer( const WorkbookHelper& rHelper );

    /** Returns a pointer to the pivot cache instance by ID. */
    const PivotCacheData*     getPivotCache( sal_uInt32 nCacheId ) const;

    /** Sets a new pivot cache data.  It will overwrite an existing cache data if
        any exists at the specified ID location. */
    void                setPivotCache( sal_uInt32 nCacheId, const PivotCacheData& aData );

    PivotTableData*     getPivotTable( const ::rtl::OUString& aName );

    void                setPivotTable( const ::rtl::OUString& aName, const PivotTableData& aData );

    /** Check if a given cell address overlaps one of pivot table destination
        ranges. */
    bool                isOverlapping( const ::com::sun::star::table::CellAddress& aCellAddress ) const;

    void                finalizeImport() const;

private:

    typedef ::std::hash_map< sal_uInt32, PivotCacheData >   PivotCacheMapType;
    typedef ::std::hash_map< ::rtl::OUString, PivotTableData, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > PivotTableMapType;

    void                writePivotTable( const ::rtl::OUString& aName, const PivotTableData& aData ) const;

    bool                getSourceRange( const ::rtl::OUString& aSheetName, const ::rtl::OUString& aRefName,
                                        ::com::sun::star::table::CellRangeAddress& rRange ) const;

private:

    PivotCacheMapType       maPivotCacheMap;    /// A collection of pivot cache data organized by IDs.
    PivotTableMapType       maPivotTableMap;    /// A collection of pivot table data organized by IDs.
    SheetCellRangeMap       maCellRangeMap;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
