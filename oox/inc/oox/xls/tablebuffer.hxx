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

#ifndef OOX_XLS_TABLEBUFFER_HXX
#define OOX_XLS_TABLEBUFFER_HXX

#include <com/sun/star/table/CellRangeAddress.hpp>
#include "oox/helper/containerhelper.hxx"
#include "oox/xls/workbookhelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

struct TableModel
{
    ::com::sun::star::table::CellRangeAddress
                        maRange;            /// Original (unchecked) range of the table.
    ::rtl::OUString     maProgName;         /// Programmatical name.
    ::rtl::OUString     maDisplayName;      /// Display name.
    sal_Int32           mnId;               /// Unique table identifier.
    sal_Int32           mnType;             /// Table type (worksheet, query, etc.).
    sal_Int32           mnHeaderRows;       /// Number of header rows.
    sal_Int32           mnTotalsRows;       /// Number of totals rows.

    explicit            TableModel();
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
    inline sal_Int32    getTableId() const { return maModel.mnId; }
    /** Returns the token index used in API token arrays (com.sun.star.sheet.FormulaToken). */
    inline sal_Int32    getTokenIndex() const { return mnTokenIndex; }
    /** Returns the display name of the table. */
    inline const ::rtl::OUString& getDisplayName() const { return maModel.maDisplayName; }

    /** Returns the original (unchecked) total range of the table. */
    inline const ::com::sun::star::table::CellRangeAddress& getOriginalRange() const { return maModel.maRange; }
    /** Returns the cell range of this table. */
    inline const ::com::sun::star::table::CellRangeAddress& getRange() const { return maDestRange; }
    /** Returns the number of columns of this table. */
    inline sal_Int32    getWidth() const { return maDestRange.EndColumn - maDestRange.StartColumn + 1; }
    /** Returns the number of rows of this table. */
    inline sal_Int32    getHeight() const { return maDestRange.EndRow - maDestRange.StartRow + 1; }
    /** Returns the number of header rows in the table range. */
    inline sal_Int32    getHeaderRows() const { return maModel.mnHeaderRows; }
    /** Returns the number of totals rows in the table range. */
    inline sal_Int32    getTotalsRows() const { return maModel.mnTotalsRows; }

private:
    TableModel          maModel;
    ::com::sun::star::table::CellRangeAddress
                        maDestRange;        /// Validated range of the table in the worksheet.
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
    /** Returns a table by its display name. */
    TableRef            getTable( const ::rtl::OUString& rDispName ) const;

private:
    void                insertTable( TableRef xTable );

private:
    typedef RefMap< sal_Int32, Table >          TableIdMap;
    typedef RefMap< ::rtl::OUString, Table >    TableNameMap;

    TableIdMap          maIdTables;
    TableNameMap        maNameTables;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif

