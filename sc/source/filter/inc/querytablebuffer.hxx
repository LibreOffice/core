/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once

#include "stylesbuffer.hxx"
#include "worksheethelper.hxx"

namespace oox::xls {

struct QueryTableModel : public AutoFormatModel
{
    OUString     maDefName;          /// Defined name containing the target cell range.
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

class QueryTable : public WorksheetHelper
{
public:
    explicit            QueryTable( const WorksheetHelper& rHelper );

    /** Imports query table settings from the queryTable element. */
    void                importQueryTable( const AttributeList& rAttribs );
    /** Imports query table settings from the QUERYTABLE record. */
    void                importQueryTable( SequenceInputStream& rStrm );

    /** Inserts a web query into the sheet. */
    void                finalizeImport();

private:
    QueryTableModel     maModel;
};

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

} // namespace oox::xls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
