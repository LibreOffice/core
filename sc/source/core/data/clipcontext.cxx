/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "clipcontext.hxx"
#include "document.hxx"

namespace sc {

CopyFromClipContext::CopyFromClipContext(
    ScDocument* pRefUndoDoc, ScDocument* pClipDoc, sal_uInt16 nInsertFlag,
    bool bAsLink, bool bSkipAttrForEmptyCells) :
    mpRefUndoDoc(pRefUndoDoc), mpClipDoc(pClipDoc), mnInsertFlag(nInsertFlag),
    mnTabStart(-1), mnTabEnd(-1),
    mbAsLink(bAsLink), mbSkipAttrForEmptyCells(bSkipAttrForEmptyCells) {}

CopyFromClipContext::~CopyFromClipContext()
{
}

bool CopyFromClipContext::initBlockPositions(ScDocument& rDoc, SCCOL nCol1, SCCOL nCol2)
{
    if (mnTabStart < 0 || mnTabEnd < 0 || mnTabStart > mnTabEnd)
        return false;

    size_t nSize = mnTabEnd - mnTabStart + 1;
    if (maTables.size() < nSize)
        maTables.resize(nSize);

    for (size_t i = 0; i < nSize; ++i)
    {
        SCTAB nTab = i + mnTabStart;
        ColumnsType& rCols = maTables[i];
        for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        {
            ColumnsType::iterator it = rCols.find(nCol);
            if (it != rCols.end())
                // This column has already been initialized. Skip it.
                continue;

            std::pair<ColumnsType::iterator,bool> r =
                rCols.insert(
                    ColumnsType::value_type(nCol, ColumnBlockPosition()));

            if (!r.second)
                // insertion failed.
                return false;

            it = r.first;

            if (!rDoc.InitColumnBlockPosition(it->second, nTab, nCol))
                return false;
        }
    }

    return true;
}

void CopyFromClipContext::setTabRange(SCTAB nStart, SCTAB nEnd)
{
    mnTabStart = nStart;
    mnTabEnd = nEnd;
}

ColumnBlockPosition* CopyFromClipContext::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    size_t nTabIndex = nTab - mnTabStart;
    if (nTabIndex >= maTables.size())
        return NULL;

    ColumnsType& rCols = maTables[nTabIndex];
    ColumnsType::iterator it = rCols.find(nCol);

    return it == rCols.end() ? NULL : &it->second;
}

ScDocument* CopyFromClipContext::getUndoDoc()
{
    return mpRefUndoDoc;
}

ScDocument* CopyFromClipContext::getClipDoc()
{
    return mpClipDoc;
}

sal_uInt16 CopyFromClipContext::getInsertFlag() const
{
    return mnInsertFlag;
}

SCTAB CopyFromClipContext::getTabStart() const
{
    return mnTabStart;
}

SCTAB CopyFromClipContext::getTabEnd() const
{
    return mnTabEnd;
}

bool CopyFromClipContext::isAsLink() const
{
    return mbAsLink;
}

bool CopyFromClipContext::isSkipAttrForEmptyCells() const
{
    return mbSkipAttrForEmptyCells;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
