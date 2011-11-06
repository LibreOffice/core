/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
