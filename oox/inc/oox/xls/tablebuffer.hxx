/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablebuffer.hxx,v $
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

#ifndef OOX_XLS_TABLEBUFFER_HXX
#define OOX_XLS_TABLEBUFFER_HXX

#include <com/sun/star/table/CellRangeAddress.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

struct OoxTableData
{
    ::com::sun::star::table::CellRangeAddress
                        maRange;            /// Range of the table in the worksheet.
    ::rtl::OUString     maProgName;         /// Programmatical name.
    ::rtl::OUString     maDisplayName;      /// Display name.
    sal_Int32           mnId;               /// Unique table identifier.
    sal_Int32           mnType;             /// Table type (worksheet, query, etc.).
    sal_Int32           mnHeaderRows;       /// Number of header rows.
    sal_Int32           mnTotalsRows;       /// Number of totals rows.

    explicit            OoxTableData();
};

// ----------------------------------------------------------------------------

class Table : public WorkbookHelper
{
public:
    explicit            Table( const WorkbookHelper& rHelper );

    /** Imports a table definition from the passed attributes. */
    void                importTable( const AttributeList& rAttribs, sal_Int16 nSheet );
    /** Imports a table definition from a TABLE record. */
    void                importTable( RecordInputStream& rStrm, sal_Int16 nSheet );

    /** Creates a database range from this tables. */
    void                finalizeImport();

    /** Returns the table identifier. */
    inline sal_Int32    getTableId() const { return maOoxData.mnId; }
    /** Returns the token index used in API token arrays (com.sun.star.sheet.FormulaToken). */
    inline sal_Int32    getTokenIndex() const { return mnTokenIndex; }

    /** Returns the cell range of this table. */
    inline const ::com::sun::star::table::CellRangeAddress& getRange() const { return maOoxData.maRange; }
    /** Returns the number of columns of this table. */
    inline sal_Int32    getWidth() const { return maOoxData.maRange.EndColumn - maOoxData.maRange.StartColumn + 1; }
    /** Returns the number of rows of this table. */
    inline sal_Int32    getHeight() const { return maOoxData.maRange.EndRow - maOoxData.maRange.StartRow + 1; }
    /** Returns the number of header rows in the table range. */
    inline sal_Int32    getHeaderRows() const { return maOoxData.mnHeaderRows; }
    /** Returns the number of totals rows in the table range. */
    inline sal_Int32    getTotalsRows() const { return maOoxData.mnTotalsRows; }

private:
    OoxTableData        maOoxData;
    sal_Int32           mnTokenIndex;       /// Token index used in API token array.
};

typedef ::boost::shared_ptr< Table > TableRef;

// ============================================================================

class TableBuffer : public WorkbookHelper
{
public:
    explicit            TableBuffer( const WorkbookHelper& rHelper );

    /** Imports a table definition from the passed attributes. */
    TableRef            importTable( const AttributeList& rAttribs, sal_Int16 nSheet );
    /** Imports a table definition from a TABLE record. */
    TableRef            importTable( RecordInputStream& rStrm, sal_Int16 nSheet );

    /** Creates database ranges from all imported tables. */
    void                finalizeImport();

    /** Returns a table by its identifier. */
    TableRef            getTable( sal_Int32 nTableId ) const;

private:
    void                insertTable( TableRef xTable );

private:
    typedef RefMap< sal_Int32, Table > TableMap;
    TableMap            maTables;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

