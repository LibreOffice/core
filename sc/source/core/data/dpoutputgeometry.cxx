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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "dpoutputgeometry.hxx"
#include "address.hxx"

#include <vector>

using ::std::vector;

ScDPOutputGeometry::ScDPOutputGeometry(const ScRange& rOutRange, bool bShowFilter, ImportType eImportType) :
    maOutRange(rOutRange),
    mnRowFields(0),
    mnColumnFields(0),
    mnPageFields(0),
    mnDataFields(0),
    meImportType(eImportType),
    mbShowFilter(bShowFilter)
{
}

ScDPOutputGeometry::~ScDPOutputGeometry()
{
}

void ScDPOutputGeometry::setRowFieldCount(sal_uInt32 nCount)
{
    mnRowFields = nCount;
}

void ScDPOutputGeometry::setColumnFieldCount(sal_uInt32 nCount)
{
    mnColumnFields = nCount;
}

void ScDPOutputGeometry::setPageFieldCount(sal_uInt32 nCount)
{
    mnPageFields = nCount;
}

void ScDPOutputGeometry::setDataFieldCount(sal_uInt32 nCount)
{
    mnDataFields = nCount;
}

void ScDPOutputGeometry::getColumnFieldPositions(vector<ScAddress>& rAddrs) const
{
    vector<ScAddress> aAddrs;
    if (!mnColumnFields)
    {
        rAddrs.swap(aAddrs);
        return;
    }

    bool bDataLayout = mnDataFields > 1;

    SCROW nCurRow = maOutRange.aStart.Row();

    if (mnPageFields)
    {
        SCROW nRowStart = maOutRange.aStart.Row() + mbShowFilter;
        SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);
        nCurRow = nRowEnd + 2;
    }
    else if (mbShowFilter)
        nCurRow += 2;

    SCROW nRow = nCurRow;
    SCTAB nTab = maOutRange.aStart.Tab();
    SCCOL nColStart = static_cast<SCCOL>(maOutRange.aStart.Col() + mnRowFields + (bDataLayout ? 1 : 0));
    SCCOL nColEnd = nColStart + static_cast<SCCOL>(mnColumnFields-1);

    for (SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        aAddrs.push_back(ScAddress(nCol, nRow, nTab));
    rAddrs.swap(aAddrs);
}

void ScDPOutputGeometry::getRowFieldPositions(vector<ScAddress>& rAddrs) const
{
    vector<ScAddress> aAddrs;
    if (!mnRowFields)
    {
        rAddrs.swap(aAddrs);
        return;
    }

    SCROW nRow = getRowFieldHeaderRow();
    SCTAB nTab = maOutRange.aStart.Tab();
    SCCOL nColStart = maOutRange.aStart.Col();
    SCCOL nColEnd = nColStart + static_cast<SCCOL>(mnRowFields-1);

    for (SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        aAddrs.push_back(ScAddress(nCol, nRow, nTab));
    rAddrs.swap(aAddrs);
}

void ScDPOutputGeometry::getPageFieldPositions(vector<ScAddress>& rAddrs) const
{
    vector<ScAddress> aAddrs;
    if (!mnPageFields)
    {
        rAddrs.swap(aAddrs);
        return;
    }

    SCTAB nTab = maOutRange.aStart.Tab();
    SCCOL nCol = maOutRange.aStart.Col();

    SCROW nRowStart = maOutRange.aStart.Row() + mbShowFilter;
    SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);

    for (SCROW nRow = nRowStart; nRow <= nRowEnd; ++nRow)
        aAddrs.push_back(ScAddress(nCol, nRow, nTab));
    rAddrs.swap(aAddrs);
}

SCROW ScDPOutputGeometry::getRowFieldHeaderRow() const
{
    SCROW nCurRow = maOutRange.aStart.Row();

    if (mnPageFields)
    {
        SCROW nRowStart = maOutRange.aStart.Row() + mbShowFilter;
        SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);
        nCurRow = nRowEnd + 2;
    }
    else if (mbShowFilter)
        nCurRow += 2;

    if (mnColumnFields)
        nCurRow += static_cast<SCROW>(mnColumnFields);
    else if (mnRowFields)
        ++nCurRow;

    return nCurRow;
}

ScDPOutputGeometry::FieldType ScDPOutputGeometry::getFieldButtonType(const ScAddress& rPos) const
{
    // We will ignore the table position for now.

    bool bExtraTitleRow = (mnColumnFields == 0 && meImportType == ScDPOutputGeometry::XLS);
    bool bDataLayout = mnDataFields > 1;

    SCROW nCurRow = maOutRange.aStart.Row();

    if (mnPageFields)
    {
        SCCOL nCol = maOutRange.aStart.Col();
        SCROW nRowStart = maOutRange.aStart.Row() + mbShowFilter;
        SCROW nRowEnd   = nRowStart + static_cast<SCCOL>(mnPageFields-1);
        if (rPos.Col() == nCol && nRowStart <= rPos.Row() && rPos.Row() <= nRowEnd)
            return Page;

        nCurRow = nRowEnd + 2;
    }
    else if (mbShowFilter)
        nCurRow += 2;

    if (mnColumnFields)
    {
        SCROW nRow = nCurRow;
        SCCOL nColStart = static_cast<SCCOL>(maOutRange.aStart.Col() + mnRowFields + (bDataLayout ? 1 : 0));
        SCCOL nColEnd = nColStart + static_cast<SCCOL>(mnColumnFields-1);
        if (rPos.Row() == nRow && nColStart <= rPos.Col() && rPos.Col() <= nColEnd)
            return Column;

        nCurRow += static_cast<SCROW>(mnColumnFields);
    }

    if (bExtraTitleRow)
        ++nCurRow;

    if (mnRowFields)
    {
        SCCOL nColStart = maOutRange.aStart.Col();
        SCCOL nColEnd = nColStart + static_cast<SCCOL>(mnRowFields-1);
        if (rPos.Row() == nCurRow && nColStart <= rPos.Col() && rPos.Col() <= nColEnd)
            return Row;
    }

    return None;
}
