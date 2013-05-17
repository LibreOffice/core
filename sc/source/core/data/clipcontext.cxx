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

ClipContextBase::ClipContextBase(ScDocument& rDoc) :
    mrDoc(rDoc), mnTabStart(-1), mnTabEnd(-1) {}

ClipContextBase::~ClipContextBase() {}

void ClipContextBase::setTabRange(SCTAB nStart, SCTAB nEnd)
{
    mnTabStart = nStart;
    mnTabEnd = nEnd;
}

SCTAB ClipContextBase::getTabStart() const
{
    return mnTabStart;
}

SCTAB ClipContextBase::getTabEnd() const
{
    return mnTabEnd;
}

ColumnBlockPosition* ClipContextBase::getBlockPosition(SCTAB nTab, SCCOL nCol)
{
    if (mnTabStart < 0 || mnTabEnd < 0 || mnTabStart > mnTabEnd)
        return NULL;

    size_t nTabIndex = nTab - mnTabStart;
    if (nTabIndex >= maTables.size())
        maTables.resize(nTabIndex+1);

    ColumnsType& rCols = maTables[nTabIndex];

    ColumnsType::iterator it = rCols.find(nCol);
    if (it != rCols.end())
        // Block position for this column has already been fetched.
        return &it->second;

    std::pair<ColumnsType::iterator,bool> r =
        rCols.insert(
            ColumnsType::value_type(nCol, ColumnBlockPosition()));

    if (!r.second)
        // insertion failed.
        return NULL;

    it = r.first;

    if (!mrDoc.InitColumnBlockPosition(it->second, nTab, nCol))
        return NULL;

    return &it->second;
}

CopyFromClipContext::CopyFromClipContext(ScDocument& rDoc,
    ScDocument* pRefUndoDoc, ScDocument* pClipDoc, sal_uInt16 nInsertFlag,
    bool bAsLink, bool bSkipAttrForEmptyCells) :
    ClipContextBase(rDoc),
    mpRefUndoDoc(pRefUndoDoc), mpClipDoc(pClipDoc), mnInsertFlag(nInsertFlag),
    mbAsLink(bAsLink), mbSkipAttrForEmptyCells(bSkipAttrForEmptyCells) {}

CopyFromClipContext::~CopyFromClipContext()
{
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

bool CopyFromClipContext::isAsLink() const
{
    return mbAsLink;
}

bool CopyFromClipContext::isSkipAttrForEmptyCells() const
{
    return mbSkipAttrForEmptyCells;
}

CopyToClipContext::CopyToClipContext(ScDocument& rDoc) : ClipContextBase(rDoc) {}
CopyToClipContext::~CopyToClipContext() {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
