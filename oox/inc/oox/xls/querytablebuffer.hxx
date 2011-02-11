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

#ifndef OOX_XLS_QUERYTABLEBUFFER_HXX
#define OOX_XLS_QUERYTABLEBUFFER_HXX

#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/worksheethelper.hxx"

namespace oox {
namespace xls {

// ============================================================================

struct QueryTableModel : public AutoFormatModel
{
    ::rtl::OUString     maDefName;          /// Defined name containing the target cell range.
    sal_Int32           mnConnId;           /// Identifier of the external connection used to query the data.
    sal_Int32           mnGrowShrinkType;   /// Behaviour when source data size changes.
    bool                mbHeaders;          /// True = source data contains a header row.
    bool                mbRowNumbers;       /// True = first column contains row numbers.
    bool                mbDisableRefresh;   /// True = refreshing data disabled.
    bool                mbBackground;       /// True = refresh asynchronously.
    bool                mbFirstBackground;  /// True = first background refresh not yet finished.
    bool                mbRefreshOnLoad;    /// True = refresh table after import.
    bool                mbFillFormulas;     /// True = expand formulas next to range when source data grows.
    bool                mbRemoveDataOnSave; /// True = remove querried data before saving.
    bool                mbDisableEdit;      /// True = connection locked for editing.
    bool                mbPreserveFormat;   /// True = use existing formatting for new rows.
    bool                mbAdjustColWidth;   /// True = adjust column widths after refresh.
    bool                mbIntermediate;     /// True = query table defined but not built yet.

    explicit            QueryTableModel();
};

// ----------------------------------------------------------------------------

class QueryTable : public WorksheetHelper
{
public:
    explicit            QueryTable( const WorksheetHelper& rHelper );

    /** Imports query table settings from the queryTable element. */
    void                importQueryTable( const AttributeList& rAttribs );
    /** Imports query table settings from the QUERYTABLE record. */
    void                importQueryTable( SequenceInputStream& rStrm );

    /** Imports query table settings from the QUERYTABLE record. */
    void                importQueryTable( BiffInputStream& rStrm );
    /** Imports query table settings from the QUERYTABLEREFRESH record. */
    void                importQueryTableRefresh( BiffInputStream& rStrm );
    /** Imports query table settings from the QUERYTABLESETTINGS record. */
    void                importQueryTableSettings( BiffInputStream& rStrm );

    /** Inserts a web query into the sheet. */
    void                finalizeImport();

private:
    QueryTableModel     maModel;
};

// ============================================================================

class QueryTableBuffer : public WorksheetHelper
{
public:
    explicit            QueryTableBuffer( const WorksheetHelper& rHelper );

    /** Creates a new query table and stores it into the internal vector. */
    QueryTable&         createQueryTable();

    /** Inserts all web queries into the sheet. */
    void                finalizeImport();

private:
    typedef RefVector< QueryTable > QueryTableVector;
    QueryTableVector    maQueryTables;
};

// ============================================================================

} // namespace xls
} // namespace oox

#endif
