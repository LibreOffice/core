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

#include "document.hxx"
#include <memory>

class ScFlatBoolRowSegments;
class ScFlatBoolColSegments;
class ScTable;

class FilterData
{
private:
    ScTable& mrTable;

    ScDocument& getDocument();
    ScDocument const& getDocument() const;

public:
    FilterData(ScTable& rTable);

    std::unique_ptr<ScFlatBoolColSegments> mpFilteredCols;
    std::unique_ptr<ScFlatBoolRowSegments> mpFilteredRows;
    std::unique_ptr<ScFlatBoolColSegments> mpHiddenCols;
    std::unique_ptr<ScFlatBoolRowSegments> mpHiddenRows;

    void makeReady();

    OString dumpRowsAsString(OString const& rDefault);
    OString dumpColsAsString(OString const& rDefault);

    OString dumpHiddenRowsAsString(OString const& rDefault);
    OString dumpHiddenColsAsString(OString const& rDefault);

    bool setColFiltered(SCCOL nStartCol, SCCOL nEndCol, bool bFiltered);
    bool setRowFiltered(SCROW nStartRow, SCROW nEndRow, bool bFiltered);

    bool rowFiltered(SCROW nRow, SCROW* pFirstRow = nullptr, SCROW* pLastRow = nullptr) const;
    bool columnFiltered(SCCOL nCol, SCCOL* pFirstCol = nullptr, SCCOL* pLastCol = nullptr) const;
    bool hasFilteredRows(SCROW nStartRow, SCROW nEndRow) const;
    void copyColFiltered(FilterData const& rFilter, SCCOL nStartCol, SCCOL nEndCol);
    void copyRowFiltered(FilterData const& rFilter, SCROW nStartRow, SCROW nEndRow);
    SCROW firstNonFilteredRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW lastNonFilteredRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW countNonFilteredRows(SCROW nStartRow, SCROW nEndRow) const;

    bool rowHidden(SCROW nRow, SCROW* pFirstRow = nullptr, SCROW* pLastRow = nullptr) const;
    bool rowHiddenLeaf(SCROW nRow, SCROW* pFirstRow = nullptr, SCROW* pLastRow = nullptr) const;
    bool hasHiddenRows(SCROW nStartRow, SCROW nEndRow) const;
    bool colHidden(SCCOL nCol, SCCOL* pFirstCol = nullptr, SCCOL* pLastCol = nullptr) const;
    bool setColHidden(SCROW nStartCol, SCROW nEndCol, bool bHidden);
    bool setRowHidden(SCROW nStartRow, SCROW nEndRow, bool bHidden);
    void copyColHidden(FilterData const& rFilter, SCCOL nStartCol, SCCOL nEndCol);
    void copyRowHidden(FilterData const& rFilter, SCROW nStartRow, SCROW nEndRow);

    SCROW firstVisibleRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW lastVisibleRow(SCROW nStartRow, SCROW nEndRow) const;
    SCROW countVisibleRows(SCROW nStartRow, SCROW nEndRow) const;
    SCCOL countVisibleCols(SCCOL nStartCol, SCCOL nEndCol) const;
    SCCOLROW lastHiddenColRow(SCCOLROW nPos, bool bCol) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
